
%{
	#include <ctype.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
	
	extern void yyerror(const char* string);
	extern int yylex(void);
	
	extern FILE* yyin;
	extern FILE* yyout;
	
	extern int yylineno;
	
	#define GRAMMAR_PROGRAM 0
	#define GRAMMAR_STATEMENT 1
	#define GRAMMAR_PRINT 2
	#define GRAMMAR_VAR_DECLARATION 3
	#define GRAMMAR_STATEMENT_LIST 4
	#define GRAMMAR_NUMBER 5
	#define GRAMMAR_IDENTIFIER 6
	#define GRAMMAR_UNARY 7
	#define GRAMMAR_OPERATION 8
	#define GRAMMAR_RETURN 12
	#define GRAMMAR_STRING 13
	#define GRAMMAR_LOGIC 14
	#define GRAMMAR_ULOGIC 15
	
	typedef struct node_s {
		#define MAX_CHILDREN 16
		
		int child_count;
		struct node_s* children[MAX_CHILDREN];
		
		char* ref, *ref_code, *data;
		int type, line;
		
	} node_t;
	
	void compile(node_t* self);
	
	node_t* new_node(int line, int type, char* data, int child_count, ...) {
		node_t* self = (node_t*) malloc(sizeof(node_t));
		
		self->type = type;
		self->data = data;
		self->line = line;
		self->ref_code = "";
		self->child_count = child_count;
		
		va_list args;
		va_start(args, child_count);
		
		for (int i = 0; i < self->child_count; i++) {
			self->children[i] = va_arg(args, node_t*);
			
		}
		
		va_end(args);
		return self;
				
	}
%}

%code requires {
	
}

%union {
	char* str;
	struct node_s* ast;
}

%token IF GOTO LAB RETURN VAR PRINT
%nonassoc IFX
%nonassoc ELSE

%token <str> IDENTIFIER NUMBER STRING
%token NONTOKEN ERROR ENDFILE

%nonassoc UBDEREF UDEREF UREF UMINUS UPLUS

%left '+' '-'
%left '*' '/'
%left '?' '&'

%type<ast> program statement statement_list expression

%start program
%%
program
	: statement_list { $$ = new_node(yylineno, GRAMMAR_PROGRAM, "", 1, $1); compile($$); }
	;

statement
	: ';' { $$ = new_node(yylineno, GRAMMAR_STATEMENT, "", 0); }
	| expression ';' { $$ = $1; }
	| PRINT expression ';' { $$ = new_node(yylineno, GRAMMAR_PRINT, "", 1, $2); }
	| RETURN expression ';' { $$ = new_node(yylineno, GRAMMAR_RETURN,"", 1, $2); }
	| VAR IDENTIFIER '=' expression ';' { $$ = new_node(yylineno, GRAMMAR_VAR_DECLARATION, $2, 1, $4); }
	| '{' '}' { $$ = new_node(yylineno, GRAMMAR_STATEMENT_LIST, "", 0); }
	| '{' statement_list '}' { $$ = $2; }
	;

statement_list
	: statement { $$ = $1; }
	| statement_list statement { $$ = new_node(yylineno, GRAMMAR_STATEMENT_LIST, "", 2, $1, $2); }
	;

expression
	: NUMBER { $$ = new_node(yylineno, GRAMMAR_NUMBER, $1, 0); }
	| STRING { $$ = new_node(yylineno, GRAMMAR_STRING, $1, 0); }
	| IDENTIFIER { $$ = new_node(yylineno, GRAMMAR_IDENTIFIER, $1, 0); }
	| '*' expression %prec UBDEREF { $$ = new_node(yylineno, GRAMMAR_UNARY, "*", 1, $2); }
	| '?' expression %prec UDEREF { $$ = new_node(yylineno, GRAMMAR_UNARY, "?", 1, $2); }
	| '&' expression %prec UREF { $$ = new_node(yylineno, GRAMMAR_UNARY, "&", 1, $2); }
	| '-' expression %prec UMINUS { $$ = new_node(yylineno, GRAMMAR_UNARY, "-", 1, $2); }
	| '+' expression %prec UPLUS { $$ = $2; }
	| expression '+' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, "+", 2, $1, $3); }
	| expression '-' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, "-", 2, $1, $3); }
	| expression '*' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, "*", 2, $1, $3); }
	| expression '/' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, "/", 2, $1, $3); }
	| '(' expression ')' { $$ = $2; }
	;
%%

#include <stdio.h>
#include <stdint.h>

void yyerror(const char* string) {
	fflush(stdout);
	fprintf(stderr, "*** %s\n", string);
	
}

typedef struct {
	char identifier[64];
	uint64_t stack_pointer;
	
} reference_t;

static uint64_t stack_pointer = 0;
static uint64_t reference_count = 0;
static reference_t* references = (reference_t*) 0;

static int data_section_count = 0;
static int depth = 0;

void compile(node_t* self) {
	depth++;
	//~ printf("\t# %d -> line = %d, type = %d, data = %s, children = %d\n", depth, self->line, self->type, self->data, self->child_count);
	
	if (self->type == GRAMMAR_PROGRAM) {
		printf(":main:\tmov bp sp\tsub bp 1024\n");
		
	}
	
	for (int i = 0; i < self->child_count; i++) compile(self->children[i]);
	depth--;
	
	// literals
	
	if (self->type == GRAMMAR_NUMBER) {
		self->ref = self->data;
		
	} else if (self->type == GRAMMAR_STRING) {
		self->ref = (char*) malloc(64);
		memset(self->ref, 0, 64);
		sprintf(self->ref, "$amber_data_%d", data_section_count++);
		
		printf("%%%s", self->ref);
		for (int i = 1; i < strlen(self->data) + 1; i++) printf(" x%x", self->data[i]);
		printf("%%\n");
		
	} else if (self->type == GRAMMAR_IDENTIFIER) {
		for (int i = 0; i < reference_count; i++) {
			if (strncmp(references[i].identifier, self->data, sizeof(references[i].identifier)) == 0) {
				self->ref_code = (char*) malloc(64);
				sprintf(self->ref_code, "cad bp sub %ld\t", references[i].stack_pointer);
				
				self->ref = "?ad";
				break;
				
			}
			
		}
		
	}
	
	// statements
	
	else if (self->type == GRAMMAR_VAR_DECLARATION) {
		if (references) references = (reference_t*) realloc(references, (reference_count + 1) * sizeof(reference_t));
		else references = (reference_t*) malloc((reference_count + 1) * sizeof(reference_t));
		
		memset(&references[reference_count], 0, sizeof(reference_t));
		strncpy(references[reference_count].identifier, self->data, sizeof(references[reference_count].identifier));
		printf("%smov g0 %s\tcad bp sub %ld\tmov ?ad g0\n", self->children[0]->ref_code, self->children[0]->ref, references[reference_count].stack_pointer = stack_pointer);
		
		stack_pointer += 8;
		reference_count++;
		
	} else if (self->type == GRAMMAR_PRINT) {
		if (self->child_count) printf("%smov a0 %s\tcal print\n", self->children[0]->ref_code, self->children[0]->ref);
		else printf("mov a0 0\tcal print\n");
		
	} else if (self->type == GRAMMAR_RETURN) {
		if (self->child_count) printf("%smov g0 %s\tret\n", self->children[0]->ref_code, self->children[0]->ref);
		else printf("mov g0 0\nret\n");
		
	}
	
	// operations
	
	else if (self->type == GRAMMAR_OPERATION) {
		self->ref_code = (char*) malloc(64);
		sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer);
		self->ref = "?ad";
		
		char* operator_instruction = "nop";
		
		if (self->data[0] == '+') operator_instruction = "add";
		else if (self->data[0] == '-') operator_instruction = "sub";
		else if (self->data[0] == '*') operator_instruction = "mul";
		else if (self->data[0] == '/') operator_instruction = "div";
		
		printf("%smov g0 %s\t%s%s g0 %s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, operator_instruction, self->children[1]->ref, self->ref_code, self->ref);
		stack_pointer += 8;
		
	} else if (self->type == GRAMMAR_UNARY) {
		self->ref_code = (char*) malloc(64);
		sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer);
		self->ref = "?ad";
		
		char* unary_code = "nop g0";
		
		if (self->data[0] == '-') unary_code = "xor g0 x8000000000000000";
		else if (self->data[0] == '~') unary_code = "not g0";
		else if (self->data[0] == '*') unary_code = "mov g0 1?g0";
		else if (self->data[0] == '?') unary_code = "mov g0 8?g0";
		else if (self->data[0] == '&') unary_code = "mov g0 ad";
		
		printf("%smov g0 %s\t%s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, unary_code, self->ref_code, self->ref);
		stack_pointer += 8;
		
	}
	
	// misc
	
	else if (self->type == GRAMMAR_PROGRAM) {
		printf("mov g0 0\tret\n");
		
	}
	
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		FILE* file = fopen(argv[1], "r");
		if (!file) {
			fprintf(stderr, "failed open");
			exit(1);
			
		}
		
		yyin = file;
	}
	
	yyparse();
	return 0; 
}
