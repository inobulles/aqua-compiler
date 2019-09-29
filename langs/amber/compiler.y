
%{
	#include <stdio.h>
	#include <stdarg.h>
	#include <stdint.h>
	#include <stdlib.h>
	
	extern int yylex(void);
	extern int yyparse(void);
	
	extern FILE* yyin;
	extern FILE* yyout;
	
	extern int yylineno;
	
	void yyerror(const char* message) {
		fflush(stdout);
		fprintf(stderr, "ERROR (%s) on line %d\n", message, yylineno);
		
	}
	
	#define GRAMM_PROGRAM 0
	
	#define GRAMM_STATEMENT 1
	#define GRAMM_EXPRESSION 2
	#define GRAMM_ARGUMENT 3
	#define GRAMM_ATTRIBUTE 4
	
	#define GRAMM_LIST_STATEMENT 5
	#define GRAMM_LIST_EXPRESSION 6
	#define GRAMM_LIST_ARGUMENT 7
	#define GRAMM_LIST_ATTRIBUTE 8
	
	#define GRAMM_IDENTIFIER 9
	#define GRAMM_NUMBER 10
	#define GRAMM_STRING 11
	
	#define GRAMM_CALL 12
	
	typedef struct {
		char identifier[64];
		
		uint8_t function;
		uint8_t bytes;
		
		uint64_t stack_pointer;
		int64_t scope_depth;
		
	} reference_t;
	
	typedef struct node_s {
		uint8_t child_count;
		struct node_s* children[16];
		
		uint8_t type;
		int line;
		
		char* ref_code;
		char* ref;
		
		uint64_t data_bytes;
		char* data;
		
	} node_t;
	
	node_t* new_node(uint8_t type, uint64_t data_bytes, char* data, int child_count, ...) {
		node_t* self = (node_t*) malloc(sizeof(node_t));
		
		self->type = type;
		self->data_bytes = data_bytes;
		self->data = data;
		self->child_count = child_count;
		
		self->line = yylineno;
		self->ref_code = "";
		
		va_list args;
		va_start(args, child_count);
		
		for (int i = 0; i < self->child_count; i++) {
			self->children[i] = va_arg(args, node_t*);
			
		}
		
		va_end(args);
		return self;
		
	}
	
	void compile(node_t* self) {
		if (!self) return;
		
		printf("node = %p\tline = %d\ttype = %d\tdata = %s\n", self, self->line, self->type, self->data);
		
		if (self->type == GRAMM_LIST_STATEMENT) {
			compile(self->children[0]);
			compile(self->children[1]);
			
		} else if (self->type == GRAMM_CALL) {
			
			
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
		//~ system("geany main.asm");
		return 0; 
	}
%}

%code requires {}

%union {
	struct { char* data; uint64_t bytes; } data;
	struct node_s* abstract_syntax_tree;
}

%token FUNC IF WHILE RET VAR
%nonassoc IFX
%nonassoc ELSE

%token <data> ATTRIBUTE IDENTIFIER NUMBER STRING
%token NONTOKEN ERROR ENDFILE

%nonassoc UNARY_BYTE_DEREF UNARY_DEREF UNARY_REF UNARY_MINUS UNARY_PLUS

%left '='
%left CMP_EQ CMP_NEQ
%left STR_CMP_EQ STR_CMP_NEQ
%left STR_CAT STR_FORMAT

%left '+' '-'
%left '*' '/'
%left '?' '&'

%type <abstract_syntax_tree> statement expression argument list_statement list_expression list_argument list_attribute

%start list_statement
%%

statement
	: ';' { $$ = new_node(GRAMM_STATEMENT, 0, "", 0); }
	| expression ';' { $$ = $1; }
	| '{' '}' { $$ = new_node(GRAMM_LIST_STATEMENT, 0, "", 0); }
	| '{' list_statement '}' { $$ = $2; }
	;

expression
	: '(' expression ')' { $$ = $2; }
	
	| IDENTIFIER { $$ = new_node(GRAMM_IDENTIFIER, 0, $1.data, 0); }
	| NUMBER { $$ = new_node(GRAMM_NUMBER, 0, $1.data, 0); }
	| STRING { $$ = new_node(GRAMM_STRING, 0, $1.data, 0); }
	
	| expression expression { $$ = new_node(GRAMM_CALL, 0, "", 2, $1, $2); }
	| expression '(' list_expression ')' { $$ = new_node(GRAMM_CALL, 0, "", 2, $1, $3); }
	;

argument
	:
	;

list_statement
	: statement { $$ = $1; }
	| statement list_statement { $$ = new_node(GRAMM_LIST_STATEMENT, 0, "", 2, $1, $2); }
	;

list_expression
	: expression { $$ = $1; }
	| expression list_expression { $$ = new_node(GRAMM_LIST_EXPRESSION, 0, "", 0); }
	;

list_argument
	: argument { $$ = $1; }
	| argument ',' list_argument { $$ = new_node(GRAMM_LIST_ARGUMENT, 0, "", 2, $1, $3); }
	;

list_attribute
	: ATTRIBUTE { $$ = new_node(GRAMM_ATTRIBUTE, 0, $1.data, 0); }
	| ATTRIBUTE list_attribute { $$ = new_node(GRAMM_LIST_ATTRIBUTE, 0, $1.data, 1, $2); }
	;
