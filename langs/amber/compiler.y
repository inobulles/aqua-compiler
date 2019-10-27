
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
		GRAMM_ASSIGN, GRAMM_COMPARE, GRAMM_STR_COMPARE, GRAMM_LOGIC, GRAMM_OPERATION, GRAMM_STR_OPERATION, GRAMM_UNARY, // arithmetic expressions
		GRAMM_VAR_DECL, GRAMM_FUNC, // declaration statements
		GRAMM_IF, GRAMM_WHILE, GRAMM_CONTROL, // statements
		GRAMM_IDENTIFIER, GRAMM_NUMBER, GRAMM_STRING, // literals
	};
	
	typedef struct node_s {
		uint8_t child_count;
		struct node_s* children[16];
		
		uint8_t type;
		int line;
		
		char* ref_code;
		char* ref;
		
		uint8_t width;
		uint64_t stack_pointer;
		int64_t scope_depth;
		
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
	static uint64_t loop_count = 0;
	
	static uint64_t stack_pointer = 0;
	static uint64_t depth = 0;
	
	static uint64_t reference_count = 0;
	static node_t** references = (node_t**) 0;
	
	static uint8_t has_defined_internal_send = 0;
	
	uint64_t generate_stack_entry(node_t* self) {
		self->ref_code = (char*) malloc(32);
		sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer += 8 - (stack_pointer - 1) % 8 + 8 - 1);
		self->ref = "?ad";
		return stack_pointer;
		
	} uint64_t create_reference(node_t* self, uint8_t width) {
		self->width = width;
		self->scope_depth = depth;
		self->stack_pointer = (stack_pointer += self->width - (stack_pointer - 1) % self->width + self->width - 1);
		
		if (references) references = (node_t**) realloc(references, (reference_count + 1) * sizeof(node_t*));
		else references = (node_t**) malloc((reference_count + 1) * sizeof(node_t*));
		
		references[reference_count] = self;
		return reference_count++;
		
	} void decrement_depth(void) {
		for (uint64_t i = 0; i < reference_count; i++) if (references[i]->scope_depth > depth) references[i]->scope_depth = -1;
		depth--;
		
	}
	
	void compile(node_t* self) {
		if (!self) return;
		
		depth++;
		self->ref = self->data;
		
		//~ printf("node = %p\tline = %d\ttype = %d\tdata = %s\n", self, self->line, self->type, self->data);
		
		// big syntax elements
		
		if (self->type == GRAMM_LIST_STATEMENT) {
			compile(self->children[0]);
			compile(self->children[1]);
			
		}
		
		// expressions
		
		else if (self->type == GRAMM_OPERATION) {
			compile(self->children[0]);
			compile(self->children[1]);
			
			generate_stack_entry(self);
			char* instruction = "nop";
			
			if (*self->data == '+') instruction = "add";
			else if (*self->data == '-') instruction = "sub";
			else if (*self->data == '*') instruction = "mul";
			else if (*self->data == '/') instruction = "div";
			else if (*self->data == '%') instruction = "div";
			
			fprintf(yyout, "%smov g0 %s\t%s%s g0 %s\t%smov %s %s\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, instruction, self->children[1]->ref, self->ref_code, self->ref, *self->data == '%' ? "a3" : "g0");
			
		} else if (self->type == GRAMM_STR_OPERATION) {
			compile(self->children[0]);
			compile(self->children[1]);
			
			generate_stack_entry(self);
			if (*self->data == '+') {
				if (!has_defined_internal_send) {
					has_defined_internal_send = 1;
					fprintf(yyout,
						"jmp $amber_internal_send_end\t:$amber_internal_send:\n"
						"\tjmp $amber_internal_send_cond\t:$amber_internal_send_loop:\n"
						"\t\tadd a0 1\n"
						"\t\t:$amber_internal_send_cond:\tcnd 1?a0\tjmp $amber_internal_send_loop\tret\t:$amber_internal_send_end:\n");
					
				}
				
				fprintf(yyout,
					"%smov g0 %s\t%smov g1 %s\n"
					"mov g2 g0\tmov a0 g2\tcal $amber_internal_send\tmov g3 a0\tsub g3 g0\n"
					"mov a0 g1\tcal $amber_internal_send\tmov a3 a0\tsub a3 g1\tadd a3 1\n"
					"mov a0 g3\tadd a0 a3\tcal malloc\n"
					"mov a0 g0\tmov a1 g2\tmov a2 g3\tcal mcpy\n"
					"add a0 g3\tmov a1 g1\tmov a2 a3\tcal mcpy\n"
					"mov g0 a0\tsub g0 g3\n"
					"%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
				
			}
			
		} else if (self->type == GRAMM_LOGIC) {
			compile(self->children[0]);
			compile(self->children[1]);
			
			generate_stack_entry(self);
			char* instruction = "nop";
			
			if (*self->data == '&') instruction = "and";
			else if (*self->data == '^') instruction = "xor";
			else if (*self->data == '|') instruction = "or";
			
			fprintf(yyout, /* normalize left */ "mov g0 0\t%scnd %s\tmov g0 1\t" /* normalize right */ "mov g1 0\t%scnd %s\tmov g1 1\t" /* operation */ "%s g0 g1\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, instruction, self->ref_code, self->ref);
			
		} else if (self->type == GRAMM_COMPARE) {
			compile(self->children[0]);
			compile(self->children[1]);
			
			generate_stack_entry(self);
			
			if (*self->data == '=') fprintf(yyout, "mov g0 0\t%smov g1 %s\t%smov g2 %s\tcmp g1 g2\tcnd zf\tmov g0 1\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
			else if (*self->data == '!') fprintf(yyout, "mov g0 1\t%smov g1 %s\t%smov g2 %s\tcmp g1 g2\tcnd zf\tmov g0 0\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
			
			else if (*self->data == '<') fprintf(yyout, "mov g0 1\t%smov g1 %s\t%scmp g1 %s\tcmp sf of\tcnd zf\tmov g0 0\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
			else if (*self->data == ']') fprintf(yyout, "mov g0 0\t%smov g1 %s\t%scmp g1 %s\tcmp sf of\tcnd zf\tmov g0 1\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
			else if (*self->data == '[') fprintf(yyout, "mov g0 0\t%smov g1 %s\t%scmp g1 %s\tcnd zf\tmov g0 1\tcmp sf of\tnot zf\tcnd zf\tmov g0 1\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
			else if (*self->data == '>') fprintf(yyout, "mov g0 0\t%smov g1 %s\t%scmp g1 %s\tnot zf\tmov g2 zf\tcmp sf of\tcmp zf g1\tcnd zf\tmov g0 1\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
			
		} else if (self->type == GRAMM_STR_COMPARE) {
			compile(self->children[0]);
			compile(self->children[1]);
			
			generate_stack_entry(self);
			uint64_t current = inline_count++;
			
			fprintf(yyout,
				"mov g0 %d\t%smov g1 %s\t%smov g2 %s\t:$amber_internal_seq_loop_inline_%ld:\n"
				"cmp 1?g1 1?g2\txor zf 1\tcnd zf\tmov g0 %d\tcnd zf\tjmp $amber_internal_seq_loop_inline_%ld_end\n"
				"cmp 1?g1 0\tcnd zf\tjmp $amber_internal_seq_loop_inline_%ld_end\n"
				"cmp 1?g2 0\tcnd zf\tjmp $amber_internal_seq_loop_inline_%ld_end\n"
				"add g1 1\tadd g2 1\tjmp $amber_internal_seq_loop_inline_%ld\n"
				":$amber_internal_seq_loop_inline_%ld_end:\t%smov %s g0\n", *self->data == '=', self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, current, *self->data != '=', current, current, current, current, current, self->ref_code, self->ref);
			
		} else if (self->type == GRAMM_ASSIGN) {
			compile(self->children[0]);
			compile(self->children[1]);
			
			generate_stack_entry(self);
			
			if (*self->data == '=') fprintf(yyout, "%smov g0 %s\t%smov %s g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
			else if (*self->data == '*') fprintf(yyout, "%smov g0 %s\t%smov g1 %s\tmov 1?g1 g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
			else if (*self->data == '?') fprintf(yyout, "%smov g0 %s\t%smov g1 %s\tmov 8?g1 g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
			
		} else if (self->type == GRAMM_UNARY) {
			compile(self->children[0]);
			generate_stack_entry(self);
			
			if (*self->data == '&') {
				fprintf(yyout, "%smov g0 ad\t%smov %s g0\n", self->children[0]->ref_code, self->ref_code, self->ref);
				
			} else {
				char* code = "nop g0";
				
				if (*self->data == '-') code = "not g0\tadd g0 1";
				else if (*self->data == '~') code = "not g0";
				else if (*self->data == '*') code = "mov g0 1?g0";
				else if (*self->data == '?') code = "mov g0 8?g0";
				
				fprintf(yyout, "%smov g0 %s\t%s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, code, self->ref_code, self->ref);
				
			}
			
		} else if (self->type == GRAMM_CALL) {
			if (strcmp(self->children[0]->data, "return") == 0) { // return
				compile(self->children[1]);
				fprintf(yyout, "%smov g0 %s\tret\n", self->children[1]->ref_code, self->children[1]->ref);
				
			} else {
				if (strcmp(self->children[0]->data, "new") == 0) { // new
					compile(self->children[1]);
					fprintf(yyout, "%smov a0 %s\tcal malloc\tmov a2 a0\tmov a0 g0\tmov a1 0\tcal mset\tmov g0 a0\t", self->children[1]->ref_code, self->children[1]->ref);
					
				} else {
					compile(self->children[0]);
					
					node_t* expression_list_root = self->children[1];
					while (expression_list_root) {
						node_t* argument_node = expression_list_root;
						
						node_t* previous_expression_list_root = expression_list_root;
						if (expression_list_root->type == GRAMM_LIST_EXPRESSION) {
							argument_node = expression_list_root->children[0];
							expression_list_root = expression_list_root->children[1];
							
						}
						
						compile(argument_node);
						
						if (previous_expression_list_root->type != GRAMM_LIST_EXPRESSION) {
							break;
							
						}
						
					}
					
					expression_list_root = self->children[1];
					uint64_t argument = 0;
					
					while (expression_list_root) {
						node_t* argument_node = expression_list_root;
						
						node_t* previous_expression_list_root = expression_list_root;
						if (expression_list_root->type == GRAMM_LIST_EXPRESSION) {
							argument_node = expression_list_root->children[0];
							expression_list_root = expression_list_root->children[1];
							
						}
						
						fprintf(yyout, "%smov a%ld %s\t", argument_node->ref_code, argument++, argument_node->ref);
						
						if (previous_expression_list_root->type != GRAMM_LIST_EXPRESSION) {
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
		
		// declaration statements
		
		else if (self->type == GRAMM_VAR_DECL) {
			char* ref_code = "";
			char* ref = "0";
			
			if (self->child_count > 1) { // is also assignment?
				compile(self->children[1]);
				
				ref_code = self->children[1]->ref_code;
				ref = self->children[1]->ref;
				
			}
			
			create_reference(self, (uint8_t) self->children[0]);
			fprintf(yyout, "%smov g0 %s\tcad bp sub %ld\tmov ?ad g0\n", ref_code, ref, self->stack_pointer);
			
		} else if (self->type == GRAMM_FUNC) {
			depth++;
			fprintf(yyout, "jmp %s$end\t:%s:\n", self->data, self->data);
			
			uint64_t argument = 0;
			if (self->child_count > 1) { // has arguments?
				node_t* argument_list_root = self->children[1];
				while (argument_list_root) {
					node_t* argument_node = argument_list_root->type == GRAMM_LIST_ARGUMENT ? argument_list_root->children[0] : argument_list_root;
					create_reference(argument_node, (uint8_t) argument_node->children[0]);
					fprintf(yyout, "cad bp sub %ld\tmov ?ad a%d\n", argument_node->stack_pointer, argument++);
					
					if (argument_list_root->type == GRAMM_LIST_ARGUMENT) argument_list_root = argument_list_root->children[1];
					else break;
					
				}
				
			}
			
			compile(self->children[0]);
			create_reference(self, 8);
			decrement_depth();
			
			fprintf(yyout, "mov g0 0\tret\t:%s$end:\n", self->data);
			fprintf(yyout, "cad bp sub %ld\tmov ?ad %s\n", self->stack_pointer, self->data);
			
		}
		
		// statements
		
		else if (self->type == GRAMM_IF) {
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
			uint64_t current = loop_count++;
			fprintf(yyout, "jmp $amber_loop_%ld_condition\t:$amber_loop_%ld:\n", current, current);
			
			compile(self->children[1]); // compile statement
			fprintf(yyout, ":$amber_loop_%ld_condition:", current);
			
			compile(self->children[0]); // compile expression
			fprintf(yyout, "%scnd %s\tjmp $amber_loop_%ld\t:$amber_loop_%ld_end:\n", self->children[0]->ref_code, self->children[0]->ref, current, current);
			
		} else if (self->type == GRAMM_CONTROL) {
			if (strcmp(self->data, "break") == 0) fprintf(yyout, "jmp $amber_loop_%ld_end\n", loop_count - 1);
			else if (strcmp(self->data, "cont") == 0) fprintf(yyout, "jmp $amber_loop_%ld_condition\n", loop_count - 1);
			
		}
		
		// literals
		
		else if (self->type == GRAMM_IDENTIFIER) {
			for (uint64_t i = 0; i < reference_count; i++) {
				if (references[i]->scope_depth >= 0 && strcmp(self->data, references[i]->data) == 0) {
					self->ref_code = (char*) malloc(32);
					sprintf(self->ref_code, "cad bp sub %ld\t", self->stack_pointer = references[i]->stack_pointer);
					
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
		//~ system("geany main.asm");
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

%left '='

%left LOG_AND
%left LOG_XOR
%left LOG_OR

%left CMP_EQ CMP_NEQ
%left CMP_GTE CMP_LTE CMP_GT CMP_LT

%left '+' '-'
%left '*' '/' '%'

%left STR_CMP_EQ STR_CMP_NEQ
%left STR_CAT STR_FORMAT

%nonassoc UNARY_BYTE_DEREF UNARY_DEREF UNARY_REF UNARY_COMPL UNARY_MINUS UNARY_PLUS
%type <abstract_syntax_tree> program data_type left_pointer1 left_pointer8 statement expression argument list_statement list_expression list_argument list_attribute

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
	: VAR  { $$ = (node_t*) 8; }
	| BYTE { $$ = (node_t*) 1; }
	;

statement
	: ';' { $$ = new_node(GRAMM_STATEMENT, 0, "", 0); }
	| expression ';' { $$ = $1; }
	
	| '{' '}' { $$ = new_node(GRAMM_LIST_STATEMENT, 0, "", 0); }
	| '{' list_statement '}' { $$ = $2; }
	
	| data_type IDENTIFIER ';' { $$ = new_node(GRAMM_VAR_DECL, 0, $2.data, 1, $1); }
	| data_type IDENTIFIER '=' expression ';' { $$ = new_node(GRAMM_VAR_DECL, 0, $2.data, 2, $1, $4); }
	
	| FUNC IDENTIFIER statement { $$ = new_node(GRAMM_FUNC, 0, $2.data, 1, $3); }
	| FUNC IDENTIFIER '(' list_argument ')' statement { $$ = new_node(GRAMM_FUNC, 0, $2.data, 2, $6, $4); }
	
	| IF '(' expression ')' statement %prec IFX { $$ = new_node(GRAMM_IF, 0, "", 2, $3, $5); }
	| IF '(' expression ')' statement ELSE statement { $$ = new_node(GRAMM_IF, 0, "", 3, $3, $5, $7); }
	
	| WHILE '(' expression ')' statement { $$ = new_node(GRAMM_WHILE, 0, "", 2, $3, $5); }
	| CONTROL { $$ = new_node(GRAMM_CONTROL, 0, $1.data, 0); }
	;

left_pointer1: '*' expression { $$ = $2; }
left_pointer8: '?' expression { $$ = $2; }

expression
	: '(' expression ')' { $$ = $2; }
	
	| left_pointer1 '=' expression { $$ = new_node(GRAMM_ASSIGN, 0, "*", 2, $1, $3); }
	| left_pointer8 '=' expression { $$ = new_node(GRAMM_ASSIGN, 0, "?", 2, $1, $3); }
	
	| '*' expression %prec UNARY_BYTE_DEREF { $$ = new_node(GRAMM_UNARY, 0, "*", 1, $2); }
	| '?' expression %prec UNARY_DEREF { $$ = new_node(GRAMM_UNARY, 0, "?", 1, $2); }
	| '&' expression %prec UNARY_REF { $$ = new_node(GRAMM_UNARY, 0, "&", 1, $2); }
	| '~' expression %prec UNARY_COMPL { $$ = new_node(GRAMM_UNARY, 0, "~", 1, $2); }
	| '-' expression %prec UNARY_MINUS { $$ = new_node(GRAMM_UNARY, 0, "-", 1, $2); }
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
	
	| expression '+' expression { $$ = new_node(GRAMM_OPERATION, 0, "+", 2, $1, $3); }
	| expression '-' expression { $$ = new_node(GRAMM_OPERATION, 0, "-", 2, $1, $3); }
	| expression '*' expression { $$ = new_node(GRAMM_OPERATION, 0, "*", 2, $1, $3); }
	| expression '/' expression { $$ = new_node(GRAMM_OPERATION, 0, "/", 2, $1, $3); }
	| expression '%' expression { $$ = new_node(GRAMM_OPERATION, 0, "%", 2, $1, $3); }
	
	| expression STR_CMP_EQ  expression { $$ = new_node(GRAMM_STR_COMPARE, 0, "=", 2, $1, $3); }
	| expression STR_CMP_NEQ expression { $$ = new_node(GRAMM_STR_COMPARE, 0, "!", 2, $1, $3); }
	
	| expression STR_CAT expression { $$ = new_node(GRAMM_STR_OPERATION, 0, "+", 2, $1, $3); }
	
	| expression '(' list_expression ')' { $$ = new_node(GRAMM_CALL, 0, "", 2, $1, $3); }
	| expression '(' ')' { $$ = new_node(GRAMM_CALL, 0, "", 1, $1); }
	//| expression expression { $$ = new_node(GRAMM_CALL, 0, "", 2, $1, $2); }
	
	| IDENTIFIER { $$ = new_node(GRAMM_IDENTIFIER, 0, $1.data, 0); }
	| NUMBER { $$ = new_node(GRAMM_NUMBER, 0, $1.data, 0); }
	| STRING { $$ = new_node(GRAMM_STRING, $1.bytes, $1.data, 0); }
	;

argument
	: data_type IDENTIFIER { $$ = new_node(GRAMM_ARGUMENT, 0, $2.data, 1, $1); }
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
