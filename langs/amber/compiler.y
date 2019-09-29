
%{
	#include <stdio.h>
	#include <stdarg.h>
	#include <stdint.h>
	#include <stdlib.h>
	#include <string.h>
	
	extern int yylex(void);
	extern int yyparse(void);
	
	extern FILE* yyin;
	extern FILE* yyout;
	
	extern int yylineno;
	
	void yyerror(const char* message) {
		fflush(stdout);
		fprintf(stderr, "ERROR (%s) on line %d\n", message, yylineno);
		
	}
	
	enum grammar_e {
		GRAMM_PROGRAM,
		
		GRAMM_STATEMENT, GRAMM_EXPRESSION, GRAMM_ARGUMENT, GRAMM_ATTRIBUTE, // big syntax elements
		GRAMM_LIST_STATEMENT, GRAMM_LIST_EXPRESSION, GRAMM_LIST_ARGUMENT, GRAMM_LIST_ATTRIBUTE, // lists
		
		GRAMM_CALL, // expressions
		GRAMM_VAR_DECL, // statements
		GRAMM_IDENTIFIER, GRAMM_NUMBER, GRAMM_STRING, // literals
	};
	
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
	
	static uint64_t data_section_count = 0;
	static uint64_t stack_pointer = 0;
	
	static uint64_t reference_count = 0;
	static reference_t* references = (reference_t*) 0;
	
	reference_t* create_reference(char* identifier, uint64_t bytes) {
		if (references) references = (reference_t*) realloc(references, (reference_count + 1) * sizeof(reference_t));
		else references = (reference_t*) malloc((reference_count + 1) * sizeof(reference_t));
		
		memset(&references[reference_count], 0, sizeof(reference_t));
		strncpy(references[reference_count].identifier, identifier, sizeof(references[reference_count].identifier));
		
		//~ references[reference_count].scope_depth = depth;
		references[reference_count].bytes = bytes;
		references[reference_count].stack_pointer = (stack_pointer += bytes - (stack_pointer - 1) % bytes + bytes - 1);
		
		return &references[reference_count++];
		
	}
	
	void compile(node_t* self) {
		if (!self) return;
		self->ref = self->data;
		
		printf("node = %p\tline = %d\ttype = %d\tdata = %s\n", self, self->line, self->type, self->data);
		
		// big syntax elements
		
		if (self->type == GRAMM_LIST_STATEMENT) {
			compile(self->children[0]);
			compile(self->children[1]);
			
		}
		
		// expressions
		
		else if (self->type == GRAMM_CALL) {
			if (strcmp(self->children[0]->data, "ret") == 0) { // return
				compile(self->children[1]);
				fprintf(yyout, "%smov g0 %s\tret\n", self->children[1]->ref_code, self->children[1]->ref);
				
			} else {
				compile(self->children[0]);
				
				node_t* expression_list_root = self->children[1];
				uint64_t argument = 0;
				
				while (expression_list_root) {
					node_t* argument_node = expression_list_root;
					
					if (expression_list_root->type == GRAMM_LIST_EXPRESSION) {
						argument_node = expression_list_root->children[0];
						expression_list_root = expression_list_root->children[1];
						
					}
					
					compile(argument_node);
					fprintf(yyout, "%smov a%ld %s\t", argument_node->ref_code, argument, argument_node->ref);
					argument++;
					
					if (expression_list_root->type != GRAMM_LIST_EXPRESSION) {
						break;
						
					}
					
				}
				
				fprintf(yyout, "%scal %s\n", self->children[0]->ref_code, self->children[0]->ref);
				
			}
			
		}
		
		// statements
		
		else if (self->type == GRAMM_VAR_DECL) {
			char* ref_code = "";
			char* ref = "0";
			
			if (self->child_count > 1) { // is also assignment?
				compile(self->children[1]);
				
				ref_code = self->children[1]->ref_code;
				ref = self->children[1]->ref;
				
			}
			
			fprintf(yyout, "%smov g0 %s\tcad bp sub %ld\tmov ?ad g0\n", ref_code, ref, create_reference(self->data, (uint64_t) self->children[0])->stack_pointer);
			
		}
		
		// literals
		
		else if (self->type == GRAMM_IDENTIFIER) {
			/// TODO check for references *FIRST*
			
		} else if (self->type == GRAMM_STRING) {
			self->ref = (char*) malloc(32);
			sprintf(self->ref, "$amber_data_%ld", data_section_count++);
			
			fprintf(yyout, "%%%s ", self->ref);
			for (uint64_t i = 0; i < self->data_bytes; i++) fprintf(yyout, "x%x ", self->data[i]);
			fprintf(yyout, "0%%\n");
			
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
%}

%code requires {}

%union {
	struct { char* data; uint64_t bytes; } data;
	struct node_s* abstract_syntax_tree;
}

%token FUNC IF WHILE
%nonassoc IFX
%nonassoc ELSE

%token VAR BYTE

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

%type <abstract_syntax_tree> program data_type statement expression argument list_statement list_expression list_argument list_attribute

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
	: VAR { $$ = 8; }
	| BYTE { $$ = 1; }
	;

statement
	: ';' { $$ = new_node(GRAMM_STATEMENT, 0, "", 0); }
	| expression ';' { $$ = $1; }
	
	| '{' '}' { $$ = new_node(GRAMM_LIST_STATEMENT, 0, "", 0); }
	| '{' list_statement '}' { $$ = $2; }
	
	| data_type IDENTIFIER '=' expression ';' { $$ = new_node(GRAMM_VAR_DECL, 0, $2.data, 2, $1, $4); }
	| data_type IDENTIFIER ';' { $$ = new_node(GRAMM_VAR_DECL, 0, $2.data, 1, $1); }
	;

expression
	: '(' expression ')' { $$ = $2; }
	
	| IDENTIFIER { $$ = new_node(GRAMM_IDENTIFIER, 0, $1.data, 0); }
	| NUMBER { $$ = new_node(GRAMM_NUMBER, 0, $1.data, 0); }
	| STRING { $$ = new_node(GRAMM_STRING, $1.bytes, $1.data, 0); }
	
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
