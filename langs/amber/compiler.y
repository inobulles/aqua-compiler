
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
	#define GRAMMAR_STATEMENT
	#define GRAMMAR_PRINT
	#define GRAMMAR_VAR_DECLARATION
	#define GRAMMAR_STATEMENT_LIST
	#define GRAMMAR_NUMBER
	#define GRAMMAR_IDENTIFIER
	#define GRAMMAR_UNARY_MINUS
	#define GRAMMAR_
	#define GRAMMAR_
	#define GRAMMAR_
	#define GRAMMAR_
	
	typedef struct node_s {
		#define MAX_CHILDREN 16
		
		int child_count;
		struct node_s* children[MAX_CHILDREN];
		
		int type;
		char* data;
		int line;
		
	} node_t;
	
	void compile(node_t* node) {
		
		
		if (node->child_count > 0) for (int i = 0; i < node->child_count; i++) {
			compile(node->children[i]);
			
		}
		
	}
	
	node_t* new_node(int line, int type, char* data, int child_count, ...) {
		node_t* self = (node_t*) malloc(sizeof(node_t));
		
		self->type = type;
		self->data = data;
		self->line = line;
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

%token <str> IDENTIFIER NUMBER
%token NONTOKEN ERROR ENDFILE

%nonassoc UMINUS

%left '+' '-'
%left '*' '/'

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
	| IDENTIFIER { $$ = new_node(yylineno, GRAMMAR_IDENTIFIER, $1, 0); }
	| '-' expression %prec UMINUS { $$ = new_node(yylineno, GRAMMAR_UNARY_MINUS, "", 1, $2); }
	| expression '+' expression { $$ = new_node(yylineno, GRAMMAR_PLUS, "", 2, $1, $3); }
	| expression '-' expression { $$ = new_node(yylineno, GRAMMAR_MINUS, "", 2, $1, $3); }
	| expression '*' expression { $$ = new_node(yylineno, GRAMMAR_TIMES, "", 2, $1, $3); }
	| expression '/' expression { $$ = new_node(yylineno, GRAMMAR_DIVIDE, "", 2, $1, $3); }
	| '(' expression ')' { $$ = $2; }
	;
%%

#include <stdio.h>

void yyerror(const char* string) {
	fflush(stdout);
	fprintf(stderr, "*** %s\n", string);
	
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
