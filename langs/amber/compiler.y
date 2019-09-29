
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
		GRAMM_VAR_DECL, GRAMM_IF, GRAMM_WHILE, GRAMM_CONTROL, // statements
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
		
		uint64_t stack_pointer;
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
	static uint64_t inline_count = 0;
	
	static uint64_t stack_pointer = 0;
	static uint64_t depth = 0;
	
	static uint64_t reference_count = 0;
	static reference_t* references = (reference_t*) 0;
	
	uint64_t generate_stack_entry(node_t* self) {
		self->ref_code = (char*) malloc(32);
		sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer += 8 - (stack_pointer - 1) % 8 + 8 - 1);
		self->ref = "?ad";
		return stack_pointer;
		
	} reference_t* create_reference(char* identifier, uint64_t bytes) {
		if (references) references = (reference_t*) realloc(references, (reference_count + 1) * sizeof(reference_t));
		else references = (reference_t*) malloc((reference_count + 1) * sizeof(reference_t));
		
		memset(&references[reference_count], 0, sizeof(reference_t));
		strncpy(references[reference_count].identifier, identifier, sizeof(references[reference_count].identifier));
		
		references[reference_count].scope_depth = depth;
		references[reference_count].bytes = bytes;
		references[reference_count].stack_pointer = (stack_pointer += bytes - (stack_pointer - 1) % bytes + bytes - 1);
		
		return &references[reference_count++];
		
	} void decrement_depth(void) {
		for (uint64_t i = 0; i < reference_count; i++) if (references[i].scope_depth > depth) references[i].scope_depth = -1;
		depth--;
		
	}
	
	/// TODO see if this is feasable
	
	//~ static uint64_t* current_recorded_allocation_count = (uint64_t*) 0;
	
	//~ void record_allocation(const char* pointer_ref, const char* bytes_ref) {
		//~ fprintf(yyout, "psh %s\tpsh %s\t", pointer_ref, bytes_ref);
		//~ (*current_recorded_allocation_count)++;
		
	//~ } void free_recorded_allocations(uint64_t recorded_allocation_count) {
		//~ while (recorded_allocation_count--) {
			//~ fprintf(yyout, "pop a1\tpop a0\tcal mfree\n");
			
		//~ }
		
	//~ }
	
	void compile(node_t* self) {
		if (!self) return;
		
		depth++;
		self->ref = self->data;
		
		printf("node = %p\tline = %d\ttype = %d\tdata = %s\n", self, self->line, self->type, self->data);
		
		// big syntax elements
		
		if (self->type == GRAMM_LIST_STATEMENT) {
			//~ uint64_t recorded_allocation_count = 0;
			//~ current_recorded_allocation_count = &recorded_allocation_count;
			
			compile(self->children[0]);
			compile(self->children[1]);
			
			//~ free_recorded_allocations(recorded_allocation_count);
			
		}
		
		// expressions
		
		else if (self->type == GRAMM_CALL) {
			if (strcmp(self->children[0]->data, "ret") == 0) { // return
				compile(self->children[1]);
				fprintf(yyout, "%smov g0 %s\tret\n", self->children[1]->ref_code, self->children[1]->ref);
				
			} else {
				if (strcmp(self->children[0]->data, "new") == 0) { // new
					compile(self->children[1]);
					fprintf(yyout, "%smov a0 %s\t", self->children[1]->ref_code, self->children[1]->ref);
					fprintf(yyout, "cal malloc\t");
					//~ record_allocation("g0", "a0");
					
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
					
					if (strcmp(self->children[0]->data, "str") == 0) { // string builtin
						uint64_t current = inline_count++;
						
						fprintf(yyout,
							"%smov g1 a0\tmov a0 16\tcal malloc\tadd g0 a0\tmov 1?g0 0\tsub g0 1\n"
							":$amber_internal_itos_loop_inline_%ld:\tsub g0 1\tdiv g1 %s\tadd a3 48\tmov 1?g0 a3\n"
							"cnd g1\tjmp $amber_internal_itos_loop_inline_%ld\n", argument > 1 ? "mov g3 a1\t" : "", current, argument > 1 ? "g3" : "10", current);
						
					} else {
						fprintf(yyout, "%scal %s\t", self->children[0]->ref_code, self->children[0]->ref);
						
					}
					
				}
				
				generate_stack_entry(self);
				fprintf(yyout, "%smov %s g0\n", self->ref_code, self->ref);
				
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
			
		} else if (self->type == GRAMM_IF) {
			compile(self->children[0]); // compile expression
			
			uint64_t current = inline_count++;
			fprintf(yyout, "jmp $amber_inline_%ld_condition\t:$amber_inline_%ld:\n", current, current);
			
			compile(self->children[1]); // compile statement (after if expression)
			
			if (self->child_count > 2) { // has "else" statement
				fprintf(yyout, "jmp $amber_inline_%ld_end\t:$amber_inline_%ld_condition:\t%scnd %s\tjmp $amber_inline_%ld\n", current, current, self->children[0]->ref_code, self->children[0]->ref, current);
				compile(self->children[2]); // compile statement (after else)
				fprintf(yyout, ":$amber_inline_%ld_end:\n", current);
				
			} else {
				fprintf(yyout, "jmp $amber_inline_%ld_end\t:$amber_inline_%ld_condition:\t%scnd %s\tjmp $amber_inline_%ld\t:$amber_inline_%ld_end:\n", current, current, self->children[0]->ref_code, self->children[0]->ref, current, current);
				
			}
			
		} else if (self->type == GRAMM_WHILE) {
			uint64_t current = inline_count++;
			fprintf(yyout, "jmp $amber_inline_%ld_condition\t:$amber_inline_%ld:\n", current, current);
			
			compile(self->children[1]); // compile statement
			fprintf(yyout, ":$amber_inline_%ld_condition:", current);
			
			compile(self->children[0]); // compile expression
			fprintf(yyout, "%scnd %s\tjmp $amber_inline_%ld\t:$amber_inline_%ld_end:\n", self->children[0]->ref_code, self->children[0]->ref, current, current);
			
		}
		
		// literals
		
		else if (self->type == GRAMM_IDENTIFIER) {
			for (uint64_t i = 0; i < reference_count; i++) {
				if (references[i].scope_depth >= 0 && strcmp(self->data, references[i].identifier) == 0) {
					self->ref_code = (char*) malloc(32);
					sprintf(self->ref_code, "cad bp sub %ld\t", self->stack_pointer = references[i].stack_pointer);
					
					self->ref = "?ad";
					break;
					
				}
				
			}
			
		} else if (self->type == GRAMM_STRING) {
			self->ref = (char*) malloc(32);
			sprintf(self->ref, "$amber_data_%ld", data_section_count++);
			
			fprintf(yyout, "%%%s ", self->ref);
			for (uint64_t i = 0; i < self->data_bytes; i++) fprintf(yyout, "x%x ", self->data[i]);
			fprintf(yyout, "0%%\n");
			
		}
		
		decrement_depth();
		
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

%token <data> CONTROL ATTRIBUTE IDENTIFIER NUMBER STRING
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
	: VAR { $$ = (node_t*) 8; }
	| BYTE { $$ = (node_t*) 1; }
	;

statement
	: ';' { $$ = new_node(GRAMM_STATEMENT, 0, "", 0); }
	| expression ';' { $$ = $1; }
	
	| '{' '}' { $$ = new_node(GRAMM_LIST_STATEMENT, 0, "", 0); }
	| '{' list_statement '}' { $$ = $2; }
	
	| data_type IDENTIFIER '=' expression ';' { $$ = new_node(GRAMM_VAR_DECL, 0, $2.data, 2, $1, $4); }
	| data_type IDENTIFIER ';' { $$ = new_node(GRAMM_VAR_DECL, 0, $2.data, 1, $1); }
	
	| IF '(' expression ')' statement %prec IFX { $$ = new_node(GRAMM_IF, 0, "", 2, $3, $5); }
	| IF '(' expression ')' statement ELSE statement { $$ = new_node(GRAMM_IF, 0, "", 3, $3, $5, $7); }
	
	| WHILE '(' expression ')' statement { $$ = new_node(GRAMM_WHILE, 0, "", 2, $3, $5); }
	| CONTROL { $$ = new_node(GRAMM_CONTROL, 0, $1.data, 0); }
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
