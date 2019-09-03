
/// HEAVILY BASED ON https://github.com/yihui-he/c0-compiler

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
	
	#define STR(VAR) (#VAR)
	
	char* none = "none";
	char* integer = "int";
	char* assign = "=";
	
	typedef struct node_s {
		#define MAX_CHILDREN 16
		
		int child_count;
		struct node_s* children[MAX_CHILDREN];
		
		char* type, *string, *value, *data_type;
		int line;
		
	} node_t;
	
	int indent = 0;
	void print_node(node_t* node) {
		printf("%d<Tree lineNo=\"%d\" nodeType=\"%s\" string=\"%s\" value=\"%s\" dataType=\"%s\">\n", 
		indent,
		node->line,
		node->type,
		node->string,
		node->value, 
		node->data_type);
	int i;
	if (node->child_count > 0){
		indent++;
		printf("%d<Child>\n", indent);
		for (i=0;i<node->child_count;i++){
			print_node(node->children[i]);
		}
		indent--;
		printf("%d</Child>\n", indent);
	}
	printf("%d</Tree>\n", indent);
		
	}
	
	node_t* new_node(int line, char* type, char* string, char* value, char* data_type, int child_count, ...) {
		node_t* self = (node_t*) malloc(sizeof(node_t));
		
		self->type = type;
		self->string = string;
		self->value = value;
		self->data_type = data_type;
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

%token IF ELSE WHILE GOTO LAB RETURN VOID
%token INT
%token PLUS MINUS STAR SLASH LT LE GT GE EQUAL NEQUAL ASSIGN
%token LSQUAR RSQUAR LBRACE RBRACE
%token SEMI COMMA LPAREN RPAREN
%token <str> ID NUMBER
%token LETTER DIGIT
%token NONTOKEN ERROR ENDFILE

%left PLUS MINUS
%left STAR SLASH

%type<ast> atree program external_declaration var_declaration init_declarator_list fun_declaration params_list compound_stmt declarator params block_item_list block_item factor call term additive_expression simple_expression /*unary_expression postfix_expression*/ assignment_expression return_stmt while_stmt if_stmt expression statement args expression_stmt
%type<str> relop declaration_specifiers

%start atree
%%

atree:program { print_node($1); }

program 
	: external_declaration {$$=$1;}
	| program external_declaration {$$=new_node(yylineno, STR(program), none, none, none, 2, $1, $2); }
	;

external_declaration
	: var_declaration {$$=$1;}
	| fun_declaration {$$=$1;}
	;

var_declaration
	: declaration_specifiers init_declarator_list SEMI 
	{$$=new_node(yylineno, "var_declaration", none, none, $1, 1, $2); }
	;

init_declarator_list
	: ID {$$ = new_node(yylineno, "init_declarator_list", $1, none, none, 0);}
	| ID ASSIGN expression {$$ = new_node(yylineno,"init_declarator_list", $1, none, none,  1, $3);}
	| init_declarator_list COMMA ID {$$ = new_node(yylineno,"init_declarator_list", $3, none, none,  1, $1);}
	;

declarator
	: LPAREN RPAREN {$$ = new_node(yylineno,"declarator", none, none, none,  0);}
	| LPAREN params RPAREN {$$ = new_node(yylineno,"declarator", none, none, none,  1, $2);}
	;

fun_declaration
	: declaration_specifiers ID declarator compound_stmt {$$=new_node(yylineno,STR(fun_declaration), $2, none, $1,  1, $4);}
	;

declaration_specifiers
	: INT {$$=integer;}
	;

params_list
	: INT ID {$$ = new_node(yylineno,"params_list", $2, none, integer,  0);}
	| params_list COMMA INT ID {$$ = new_node(yylineno,"params_list", $4, none, integer,  1, $1);}
	;

params
	: params_list {$$=$1;}
	| VOID {$$ = new_node(yylineno,"params", none, none, "VOID",  0);}
	;
	
compound_stmt
	: LBRACE RBRACE {$$ = new_node(yylineno,"compound_stmt", none, none, none,  0);}
	| LBRACE block_item_list RBRACE {$$ = $2;}
	;

block_item_list
	: block_item {$$ = $1;}
	| block_item_list block_item {$$ = new_node(yylineno,"block_item_list", none, none, none,  2, $1, $2);}
	;

block_item
	: var_declaration {$$=$1;}
	| statement {$$=$1;}
	;

statement
	: expression_stmt {$$=$1;}
	| compound_stmt {$$=$1;}
	| if_stmt {$$=$1;}
	| while_stmt {$$=$1;}
	| return_stmt {$$=$1;}
	;

expression_stmt
	: SEMI {$$ = new_node(yylineno,"expression_stmt", none, none, none,  0);}
	| expression SEMI {$$=$1;}
	;

if_stmt
	: IF LPAREN expression RPAREN statement ELSE statement {$$ = new_node(yylineno,"if_stmt", none, none, none,  3, $3, $5, $7);}
	| IF LPAREN expression RPAREN statement {$$ = new_node(yylineno,"if_stmt", none, none, none,  2, $3, $5);}
	;

while_stmt
	: WHILE LPAREN expression RPAREN statement {$$ = new_node(yylineno,"while_stmt", none, none, none,  2, $3, $5);}
	;

return_stmt
	: RETURN SEMI {$$ = new_node(yylineno,"return_stmt", none, none, none,  0);}
	| RETURN expression SEMI {$$ = new_node(yylineno,"return_stmt", none, none, none,  1, $2);}
	;
	
expression
	: assignment_expression {$$=$1;}
	| simple_expression {$$=$1;}
	;

/*assignment_expression
	: ID ASSIGN expression {$$ = new_node(yylineno,"assignment_expression", $1, none, none,  1, $3);}
	| unary_expression  {$$=$1;}    ;*/

assignment_expression
	: ID ASSIGN expression {$$ = new_node(yylineno,"assignment_expression", $1, none, none,  1, $3);}
	;

/*unary_expression 
	: INC_OP ID {$$ = new_node(yylineno,"unary_expression", $2, none, "++",  0);}
	| DEC_OP ID {$$ = new_node(yylineno,"unary_expression", $2, none, "--",  0);}
	| postfix_expression {$$=$1;}
	;

postfix_expression
	: ID INC_OP {$$ = new_node(yylineno,"postfix_expression", $1, none, "++",  0);}
	| ID DEC_OP {$$ = new_node(yylineno,"postfix_expression", $1, none, "--",  0);}
	;*/

simple_expression
	: additive_expression {$$=$1;}
	| additive_expression relop additive_expression {$$ = new_node(yylineno,"simple_expression", none, none, $2,  2, $1, $3);}
	;

relop 
	: LT     {$$ = "<";}
	| LE     {$$ = "<=";}
	| GT     {$$ = ">";}
	| GE     {$$ = ">=";}
	| EQUAL  {$$ = "==";}
	| NEQUAL {$$ = "!=";}
	;

additive_expression
	: term {$$=$1;}
	| additive_expression PLUS term {$$ = new_node(yylineno,"additive_expression", none, none, "+",  2, $1, $3);}
	| additive_expression MINUS term {$$ = new_node(yylineno,"additive_expression", none, none, "-",  2, $1, $3);}
	| PLUS additive_expression %prec STAR {$$ = new_node(yylineno,"additive_expression", none, none, "+",  1, $2);}
	| MINUS additive_expression %prec STAR {$$ = new_node(yylineno,"additive_expression", none, none, "-",  1, $2);}
	;

term
	: factor {$$=$1;}
	| term STAR factor {$$ = new_node(yylineno,"term", none, none, "*",  2, $1, $3);}
	| term SLASH factor {$$ = new_node(yylineno,"term", none, none, "/",  2, $1, $3);}
	;

factor
	: LPAREN expression RPAREN {$$=$2;}
	| ID {$$ = new_node(yylineno,"factor", $1, none, none,  0);}
	| call {$$=$1;}
	| NUMBER {$$ = new_node(yylineno,"factor", none, $1, none,  0);}
	;
	
call
	: ID LPAREN RPAREN {$$ = new_node(yylineno,"call", $1, none, none,  0);}
	| ID LPAREN args RPAREN {$$ = new_node(yylineno,"call", $1, none, none,  1, $3);}
	;

args
	: expression {$$=$1;}
	| expression COMMA args {$$ = new_node(yylineno,"args", none, none, none,  2, $1, $3);}
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
