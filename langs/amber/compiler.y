
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
	#define GRAMMAR_STROP 9
	#define GRAMMAR_RETURN 12
	#define GRAMMAR_STRING 13
	#define GRAMMAR_LOGIC 14
	#define GRAMMAR_ULOGIC 15
	
	typedef struct node_s {
		#define MAX_CHILDREN 16
		
		int child_count;
		struct node_s* children[MAX_CHILDREN];
		
		char* ref, *ref_code, *data;
		int data_bytes, type, line;
		
	} node_t;
	
	void compile(node_t* self);
	
	node_t* new_node(int line, int type, int data_bytes, char* data, int child_count, ...) {
		node_t* self = (node_t*) malloc(sizeof(node_t));
		
		self->type = type;
		self->data = data;
		self->data_bytes = data_bytes;
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
	struct { char* data; int bytes; } data;
	struct node_s* ast;
}

%token IF GOTO LAB RETURN VAR PRINT
%nonassoc IFX
%nonassoc ELSE

%token <str> IDENTIFIER NUMBER
%token <data> STRING
%token NONTOKEN ERROR ENDFILE

%nonassoc UBDEREF UDEREF UREF UMINUS UPLUS

%left STROP_CAT STROP_FOR
%left '+' '-'
%left '*' '/'
%left '?' '&'

%type<ast> program statement statement_list expression

%start program
%%
program
	: statement_list { $$ = new_node(yylineno, GRAMMAR_PROGRAM, 0, "", 1, $1); compile($$); }
	;

statement
	: ';' { $$ = new_node(yylineno, GRAMMAR_STATEMENT, 0, "", 0); }
	| expression ';' { $$ = $1; }
	| PRINT expression ';' { $$ = new_node(yylineno, GRAMMAR_PRINT, 0, "", 1, $2); }
	| RETURN expression ';' { $$ = new_node(yylineno, GRAMMAR_RETURN, 0, "", 1, $2); }
	| VAR IDENTIFIER '=' expression ';' { $$ = new_node(yylineno, GRAMMAR_VAR_DECLARATION, 0, $2, 1, $4); }
	| '{' '}' { $$ = new_node(yylineno, GRAMMAR_STATEMENT_LIST, 0, "", 0); }
	| '{' statement_list '}' { $$ = $2; }
	;

statement_list
	: statement { $$ = $1; }
	| statement_list statement { $$ = new_node(yylineno, GRAMMAR_STATEMENT_LIST, 0, "", 2, $1, $2); }
	;

expression
	: NUMBER { $$ = new_node(yylineno, GRAMMAR_NUMBER, 0, $1, 0); }
	| STRING { $$ = new_node(yylineno, GRAMMAR_STRING, $1.bytes, $1.data, 0); }
	| IDENTIFIER { $$ = new_node(yylineno, GRAMMAR_IDENTIFIER, 0, $1, 0); }
	| '*' expression %prec UBDEREF { $$ = new_node(yylineno, GRAMMAR_UNARY, 0, "*", 1, $2); }
	| '?' expression %prec UDEREF { $$ = new_node(yylineno, GRAMMAR_UNARY, 0, "?", 1, $2); }
	| '&' expression %prec UREF { $$ = new_node(yylineno, GRAMMAR_UNARY, 0, "&", 1, $2); }
	| '-' expression %prec UMINUS { $$ = new_node(yylineno, GRAMMAR_UNARY, 0, "-", 1, $2); }
	| '+' expression %prec UPLUS { $$ = $2; }
	| expression STROP_CAT expression { $$ = new_node(yylineno, GRAMMAR_STROP, 0, "+", 2, $1, $3); }
	| expression STROP_FOR expression { $$ = new_node(yylineno, GRAMMAR_STROP, 0, "%", 2, $1, $3); }
	| expression '+' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, 0, "+", 2, $1, $3); }
	| expression '-' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, 0, "-", 2, $1, $3); }
	| expression '*' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, 0, "*", 2, $1, $3); }
	| expression '/' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, 0, "/", 2, $1, $3); }
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

static int has_defined_internal_send = 0;

void compile(node_t* self) {
	depth++;
	//~ printf("\t# %d -> line = %d, type = %d, data = %s, children = %d\n", depth, self->line, self->type, self->data, self->child_count);
	
	if (self->type == GRAMMAR_PROGRAM) {
		fprintf(yyout, ":main:\tmov bp sp\tsub bp 1024\n");
		
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
		
		fprintf(yyout, "%%%s", self->ref);
		for (int i = 0; i < self->data_bytes; i++) fprintf(yyout, " x%x", self->data[i]);
		fprintf(yyout, "%%\n");
		
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
		fprintf(yyout, "%smov g0 %s\tcad bp sub %ld\tmov ?ad g0\n", self->children[0]->ref_code, self->children[0]->ref, references[reference_count].stack_pointer = stack_pointer);
		
		stack_pointer += 8;
		reference_count++;
		
	} else if (self->type == GRAMMAR_PRINT) {
		if (self->child_count) fprintf(yyout, "%smov a0 %s\tcal print\n", self->children[0]->ref_code, self->children[0]->ref);
		else fprintf(yyout, "mov a0 0\tcal print\n");
		
	} else if (self->type == GRAMMAR_RETURN) {
		if (self->child_count) fprintf(yyout, "%smov g0 %s\tret\n", self->children[0]->ref_code, self->children[0]->ref);
		else fprintf(yyout, "mov g0 0\nret\n");
		
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
		
		fprintf(yyout, "%smov g0 %s\t%s%s g0 %s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, operator_instruction, self->children[1]->ref, self->ref_code, self->ref);
		stack_pointer += 8;
		
	} else if (self->type == GRAMMAR_UNARY) {
		self->ref_code = (char*) malloc(64);
		sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer);
		self->ref = "?ad";
		
		char* unary_code = "nop g0";
		
		if (self->data[0] == '-') unary_code = "xor g0 x8000000000000000\tadd g0 1";
		else if (self->data[0] == '~') unary_code = "not g0";
		else if (self->data[0] == '*') unary_code = "mov g0 1?g0";
		else if (self->data[0] == '?') unary_code = "mov g0 8?g0";
		else if (self->data[0] == '&') unary_code = "mov g0 ad";
		
		fprintf(yyout, "%smov g0 %s\t%s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, unary_code, self->ref_code, self->ref);
		stack_pointer += 8;
		
	} else if (self->type == GRAMMAR_STROP) {
		self->ref_code = (char*) malloc(64);
		sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer);
		self->ref = "?ad";
		
		char* strop_code = "";
		
		if (self->data[0] == '+') {
			if (!has_defined_internal_send) {
				has_defined_internal_send = 1;
				fprintf(yyout,
					"jmp $amber_internal_send_end\t:$amber_internal_send:\n"
					"\tjmp $amber_internal_send_cond\t:$amber_internal_send_loop:\n"
					"\t\tadd a0 1\n"
					"\t\t:$amber_internal_send_cond:\tcnd 1?a0\tjmp $amber_internal_send_loop\tret\t:$amber_internal_send_end:\n");
				
			}
			
			strop_code =
				"mov g2 g0\tmov a0 g2\tcal $amber_internal_send\tmov g3 a0\tsub g3 g0\n"
				"mov a0 g1\tcal $amber_internal_send\tmov a3 a0\tsub a3 g1\tadd a3 1\n"
				"mov a0 g3\tadd a0 a3\tcal malloc\n"
				"mov a0 g0\tmov a1 g2\tmov a2 g3\tcal mcpy\n"
				"add a0 g3\tmov a1 g1\tmov a2 a3\tcal mcpy\n"
				"mov g0 a0\tsub g0 g3\n";
			
		}
		
		fprintf(yyout, "%smov g0 %s\t%smov g1 %s\n%s%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, strop_code, self->ref_code, self->ref);
		stack_pointer += 8;
		
	}
	
	// misc
	
	else if (self->type == GRAMMAR_PROGRAM) {
		fprintf(yyout, "mov g0 0\tret\n");
		
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
		yyout = stdout;
		
		if (argc > 2) {
			FILE* file = fopen(argv[2], "w");
			if (!file) {
				fprintf(stderr, "failed open");
				exit(1);
				
			}
			
			yyout = file;
			
		}
		
	}
	
	yyparse();
	fclose(yyout);
	system("geany main.asm");
	return 0; 
}
