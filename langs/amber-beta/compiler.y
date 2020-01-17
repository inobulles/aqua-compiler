
%{
	#include "compiler.h"
	static uint64_t next_list_times;
%}

%code requires {}

%union {
	struct { char* data; uint64_t bytes; } data;
	struct node_s* abstract_syntax_tree;
}

%token CLASS FUNC IF WHILE LOOP RETURN
%nonassoc IF_PREC
%nonassoc ELSE

%token VAR BYTE

%token <data> CONTROL ATTRIBUTE IDENTIFIER NUMBER STRING
%token NONTOKEN ERROR ENDFILE

%left '='

%left LOG_OR
%left LOG_XOR
%left LOG_AND

%left '|'
%left '^'
%left '&'

%left CMP_EQ CMP_NEQ
%left CMP_GTE CMP_LTE CMP_GT CMP_LT

%left SHL SHR ROR
%left '+' '-'
%left '*' '/' '%'

%left STR_CMP_EQ STR_CMP_NEQ
%left STR_CAT STR_FORMAT

%nonassoc UNARY_BYTE_DEREF UNARY_DEREF UNARY_REF UNARY_COMPL UNARY_MINUS UNARY_PLUS UNARY_NOT UNARY_NORM
%left CAST
%nonassoc CALL_PREC
%nonassoc '(' ')'
%left '.'
%nonassoc ABS_PREC

%type <abstract_syntax_tree> program data_type statement left_pointer1 left_pointer8 expression argument list_statement list_expression list_argument list_attribute

%start program
%%
program:list_statement {
	fprintf(yyout, "#####################################\n");
	compile($1);
	fprintf(yyout, "mov g0 0\tret\n");
	rewind(yyout);
	fprintf(yyout, ":main:\tmov bp sp\tsub bp %ld", stack_pointer);
};

data_type
	: VAR  { next_list_times = 1; $$ = (node_t*) 8; }
	| BYTE { next_list_times = 1; $$ = (node_t*) 1; }
	| VAR  '[' NUMBER ']' { next_list_times = atoi($3.data); $$ = (node_t*) 8; }
	| BYTE '[' NUMBER ']' { next_list_times = atoi($3.data); $$ = (node_t*) 1; }
	;

statement
	: ';' { $$ = new_node(GRAMM_STATEMENT, 0, "", 0); }
	| expression ';' { $$ = $1; }
	
	| '{' '}' { $$ = new_node(GRAMM_LIST_STATEMENT, 0, "", 0); }
	| '{' list_statement '}' { $$ = $2; }
	
	| data_type IDENTIFIER ';' { $$ = new_node(GRAMM_VAR_DECL, 0, $2.data, 2, (node_t*) next_list_times, $1); }
	| data_type IDENTIFIER '=' expression ';' { $$ = new_node(GRAMM_VAR_DECL, 0, $2.data, 3, (node_t*) next_list_times, $1, $4); }
	
	| expression CAST data_type IDENTIFIER ';' { $$ = new_node(GRAMM_VAR_DECL, 1, $4.data, 3, (node_t*) next_list_times, $3, $1); }
	| expression CAST data_type IDENTIFIER '=' expression ';' { $$ = new_node(GRAMM_VAR_DECL, 1, $4.data, 4, (node_t*) next_list_times, $3, $6, $1); }
	
	| CLASS IDENTIFIER '{' list_statement '}' { $$ = new_node(GRAMM_CLASS, 0, $2.data, 1, $4); }
	
	| FUNC IDENTIFIER statement { $$ = new_node(GRAMM_FUNC, 0, $2.data, 1, $3); }
	| FUNC IDENTIFIER '(' list_argument ')' statement { $$ = new_node(GRAMM_FUNC, 0, $2.data, 2, $6, $4); }
	
	| IF '(' expression ')' statement %prec IF_PREC { $$ = new_node(GRAMM_IF, 0, "", 2, $3, $5); }
	| IF '(' expression ')' statement ELSE statement { $$ = new_node(GRAMM_IF, 0, "", 3, $3, $5, $7); }
	
	| WHILE '(' expression ')' statement { $$ = new_node(GRAMM_WHILE, 0, "", 2, $5, $3); }
	| LOOP statement { $$ = new_node(GRAMM_WHILE, 0, "", 1, $2); }
	| CONTROL { $$ = new_node(GRAMM_CONTROL, 0, $1.data, 0); }
	
	| RETURN expression ';' { $$ = new_node(GRAMM_CALL, 0, "", 2, new_node(GRAMM_IDENTIFIER, 0, "return", 0), $2); }
	| RETURN ';' { $$ = new_node(GRAMM_CALL, 0, "", 2, new_node(GRAMM_IDENTIFIER, 0, "return", 0), new_node(GRAMM_NUMBER, 0, "0", 0)); }
	
	| expression list_expression ';' { $$ = new_node(GRAMM_CALL, 0, "", 2, $1, $2); }
	;

left_pointer1: '*' expression { $$ = $2; }
left_pointer8: '?' expression { $$ = $2; }

expression
	: '(' expression ')' %prec ABS_PREC { $$ = $2; }
	
	| left_pointer1 '=' expression { $$ = new_node(GRAMM_ASSIGN, 0, "*", 2, $1, $3); }
	| left_pointer8 '=' expression { $$ = new_node(GRAMM_ASSIGN, 0, "?", 2, $1, $3); }
	
	| '*' expression %prec UNARY_BYTE_DEREF { $$ = new_node(GRAMM_UNARY, 0, "*", 1, $2); }
	| '?' expression %prec UNARY_DEREF { $$ = new_node(GRAMM_UNARY, 0, "?", 1, $2); }
	| '&' expression %prec UNARY_REF { $$ = new_node(GRAMM_UNARY, 0, "&", 1, $2); }
	| '~' expression %prec UNARY_COMPL { $$ = new_node(GRAMM_UNARY, 0, "~", 1, $2); }
	| '-' expression %prec UNARY_MINUS { $$ = new_node(GRAMM_UNARY, 0, "-", 1, $2); }
	| '|' expression %prec UNARY_NORM { $$ = new_node(GRAMM_UNARY, 0, "|", 1, $2); }
	| '!' expression %prec UNARY_NOT { $$ = new_node(GRAMM_UNARY, 0, "!", 1, $2); }
	| '+' expression %prec UNARY_PLUS { $$ = $2; }
	
	| expression '=' expression { $$ = new_node(GRAMM_ASSIGN, 0, "=", 2, $1, $3); }
	
	| expression LOG_AND expression { $$ = new_node(GRAMM_LOGIC, 0, "&", 2, $1, $3); }
	| expression LOG_XOR expression { $$ = new_node(GRAMM_LOGIC, 0, "^", 2, $1, $3); }
	| expression LOG_OR  expression { $$ = new_node(GRAMM_LOGIC, 0, "|", 2, $1, $3); }
	
	| expression CMP_EQ  expression { $$ = new_node(GRAMM_COMPARE, 0, "=", 2, $1, $3); }
	| expression CMP_NEQ expression { $$ = new_node(GRAMM_COMPARE, 0, "!", 2, $1, $3); }
	| expression CMP_GTE expression { $$ = new_node(GRAMM_COMPARE, 0, "]", 2, $1, $3); }
	| expression CMP_LTE expression { $$ = new_node(GRAMM_COMPARE, 0, "[", 2, $1, $3); }
	| expression CMP_GT  expression { $$ = new_node(GRAMM_COMPARE, 0, ">", 2, $1, $3); }
	| expression CMP_LT  expression { $$ = new_node(GRAMM_COMPARE, 0, "<", 2, $1, $3); }
	
	| expression '|' expression { $$ = new_node(GRAMM_OPERATION, 0, "|", 2, $1, $3); }
	| expression '^' expression { $$ = new_node(GRAMM_OPERATION, 0, "^", 2, $1, $3); }
	| expression '&' expression { $$ = new_node(GRAMM_OPERATION, 0, "&", 2, $1, $3); }
	| expression SHL expression { $$ = new_node(GRAMM_OPERATION, 0, "<", 2, $1, $3); }
	| expression SHR expression { $$ = new_node(GRAMM_OPERATION, 0, ">", 2, $1, $3); }
	| expression ROR expression { $$ = new_node(GRAMM_OPERATION, 0, "r", 2, $1, $3); }
	| expression '+' expression { $$ = new_node(GRAMM_OPERATION, 0, "+", 2, $1, $3); }
	| expression '-' expression { $$ = new_node(GRAMM_OPERATION, 0, "-", 2, $1, $3); }
	| expression '*' expression { $$ = new_node(GRAMM_OPERATION, 0, "*", 2, $1, $3); }
	| expression '/' expression { $$ = new_node(GRAMM_OPERATION, 0, "/", 2, $1, $3); }
	| expression '%' expression { $$ = new_node(GRAMM_OPERATION, 0, "%", 2, $1, $3); }
	
	| expression STR_CMP_EQ  expression { $$ = new_node(GRAMM_STR_COMPARE, 0, "=", 2, $1, $3); }
	| expression STR_CMP_NEQ expression { $$ = new_node(GRAMM_STR_COMPARE, 0, "!", 2, $1, $3); }
	
	| expression STR_CAT expression { $$ = new_node(GRAMM_STR_OPERATION, 0, "+", 2, $1, $3); }
	
	| expression CAST expression { $$ = new_node(GRAMM_CAST, 0, "", 2, $1, $3); }
	| expression '.' expression { $$ = new_node(GRAMM_ACCESS, 0, "", 2, $1, $3); }
	
	| expression '(' list_expression ')' %prec CALL_PREC { $$ = new_node(GRAMM_CALL, 0, "", 2, $1, $3); }
	| expression '(' ')' %prec CALL_PREC { $$ = new_node(GRAMM_CALL, 0, "", 1, $1); }
	
	| IDENTIFIER { $$ = new_node(GRAMM_IDENTIFIER, 0, $1.data, 0); }
	| NUMBER { $$ = new_node(GRAMM_NUMBER, 0, $1.data, 0); }
	| NUMBER '.' NUMBER { $$ = new_node(GRAMM_FIXED, 0, "", 2, $1.data, $3.data); }
	| STRING { $$ = new_node(GRAMM_STRING, $1.bytes, $1.data, 0); }
	;

argument
	: data_type IDENTIFIER { $$ = new_node(GRAMM_ARGUMENT, 0, $2.data, 1, $1); }
	| expression CAST data_type IDENTIFIER { $$ = new_node(GRAMM_ARGUMENT, 0, $4.data, 2, $3, $1); }
	;

list_statement
	: statement { $$ = $1; }
	| statement list_statement { $$ = new_node(GRAMM_LIST_STATEMENT, 0, "", 2, $1, $2); }
	;

list_expression
	: expression { $$ = $1; }
	| expression ',' list_expression { $$ = new_node(GRAMM_LIST_EXPRESSION, 0, "", 2, $1, $3); }
	;

list_argument
	: argument { $$ = $1; }
	| argument ',' list_argument { $$ = new_node(GRAMM_LIST_ARGUMENT, 0, "", 2, $1, $3); }
	;

list_attribute
	: ATTRIBUTE { $$ = new_node(GRAMM_ATTRIBUTE, 0, $1.data, 0); }
	| ATTRIBUTE list_attribute { $$ = new_node(GRAMM_LIST_ATTRIBUTE, 0, $1.data, 1, $2); }
	;
