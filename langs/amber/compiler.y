
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
	#define GRAMMAR_VAR_DECL 3
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
	#define GRAMMAR_IF 16
	#define GRAMMAR_IFELSE 17
	#define GRAMMAR_ASSIGN 18
	#define GRAMMAR_WHILE 19
	#define GRAMMAR_FUNC 20
	#define GRAMMAR_CALL 21
	#define GRAMMAR_EXPRESSION_LIST 22
	#define GRAMMAR_ARGUMENT 23
	#define GRAMMAR_ARGUMENT_LIST 24
	#define GRAMMAR_CMPOP 25
	#define GRAMMAR_DECL_LIST 26
	#define GRAMMAR_CLASS 27
	
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

%token FUNC CLASS IF WHILE GOTO LAB RETURN VAR PRINT
%nonassoc IFX
%nonassoc ELSE

%token <str> IDENTIFIER NUMBER
%token <data> STRING
%token NONTOKEN ERROR ENDFILE

%nonassoc UBDEREF UDEREF UREF UMINUS UPLUS

%left '='
%left CMPOP_EQ CMPOP_NEQ
%left STROP_EQ STROP_NEQ
%left STROP_CAT STROP_FOR
%left '+' '-'
%left '*' '/'
%left '?' '&'

%type<ast> program statement statement_list declaration declaration_list expression expression_list argument argument_list

%start program
%%
program
	: statement_list { $$ = new_node(yylineno, GRAMMAR_PROGRAM, 0, "", 1, $1); compile($$); }
	;

statement
	: ';' { $$ = new_node(yylineno, GRAMMAR_STATEMENT, 0, "", 0); }
	| expression ';' { $$ = $1; }
	| declaration { $$ = $1; }
	| PRINT expression ';' { $$ = new_node(yylineno, GRAMMAR_PRINT, 0, "", 1, $2); }
	| RETURN expression ';' { $$ = new_node(yylineno, GRAMMAR_RETURN, 0, "", 1, $2); }
	| WHILE '(' expression ')' statement { $$ = new_node(yylineno, GRAMMAR_WHILE, 0, "", 2, $3, $5); }
	| IF '(' expression ')' statement %prec IFX { $$ = new_node(yylineno, GRAMMAR_IF, 0, "", 2, $3, $5); }
	| IF '(' expression ')' statement ELSE statement { $$ = new_node(yylineno, GRAMMAR_IFELSE, 0, "", 3, $3, $5, $7); }
	| '{' '}' { $$ = new_node(yylineno, GRAMMAR_STATEMENT_LIST, 0, "", 0); }
	| '{' statement_list '}' { $$ = $2; }
	;

statement_list
	: statement { $$ = $1; }
	| statement_list statement { $$ = new_node(yylineno, GRAMMAR_STATEMENT_LIST, 0, "", 2, $1, $2); }
	;

declaration
	: CLASS IDENTIFIER '{' declaration_list '}' { $$ = new_node(yylineno, GRAMMAR_CLASS, 0, $2, 1, $4); }
	| FUNC IDENTIFIER statement { $$ = new_node(yylineno, GRAMMAR_FUNC, 0, $2, 1, $3); }
	| FUNC IDENTIFIER '(' argument_list ')' statement { $$ = new_node(yylineno, GRAMMAR_FUNC, 0, $2, 2, $6, $4); }
	| VAR IDENTIFIER '=' expression ';' { $$ = new_node(yylineno, GRAMMAR_VAR_DECL, 0, $2, 1, $4); }
	| VAR IDENTIFIER ';' { $$ = new_node(yylineno, GRAMMAR_VAR_DECL, 0, $2, 0); }
	;

declaration_list
	: declaration { $$ = $1; }
	| declaration_list declaration { $$ = new_node(yylineno, GRAMMAR_DECL_LIST, 0, "", 2, $1, $2); }
	;

expression
	: NUMBER { $$ = new_node(yylineno, GRAMMAR_NUMBER, 0, $1, 0); }
	| STRING { $$ = new_node(yylineno, GRAMMAR_STRING, $1.bytes, $1.data, 0); }
	| IDENTIFIER { $$ = new_node(yylineno, GRAMMAR_IDENTIFIER, 0, $1, 0); }
	| IDENTIFIER '(' ')' { $$ = new_node(yylineno, GRAMMAR_CALL, 0, $1, 0); }
	| '(' expression ')' '(' ')' { $$ = new_node(yylineno, GRAMMAR_CALL, 0, "", 1, $2); }
	| IDENTIFIER '(' expression_list ')' { $$ = new_node(yylineno, GRAMMAR_CALL, 0, $1, 1, $3); }
	| '(' expression ')' '(' expression_list ')' { $$ = new_node(yylineno, GRAMMAR_CALL, 0, "", 2, $2, $5); }
	| '*' expression '=' expression { $$ = new_node(yylineno, GRAMMAR_ASSIGN, 0, "*", 2, $2, $4); }
	| '?' expression '=' expression { $$ = new_node(yylineno, GRAMMAR_ASSIGN, 0, "?", 2, $2, $4); }
	| '*' expression %prec UBDEREF { $$ = new_node(yylineno, GRAMMAR_UNARY, 0, "*", 1, $2); }
	| '?' expression %prec UDEREF { $$ = new_node(yylineno, GRAMMAR_UNARY, 0, "?", 1, $2); }
	| '&' expression %prec UREF { $$ = new_node(yylineno, GRAMMAR_UNARY, 0, "&", 1, $2); }
	| '-' expression %prec UMINUS { $$ = new_node(yylineno, GRAMMAR_UNARY, 0, "-", 1, $2); }
	| '+' expression %prec UPLUS { $$ = $2; }
	| expression '=' expression { $$ = new_node(yylineno, GRAMMAR_ASSIGN, 0, "=", 2, $1, $3); }
	| expression CMPOP_EQ expression { $$ = new_node(yylineno, GRAMMAR_CMPOP, 0, "=", 2, $1, $3); }
	| expression CMPOP_NEQ expression { $$ = new_node(yylineno, GRAMMAR_CMPOP, 0, "!", 2, $1, $3); }
	| expression STROP_CAT expression { $$ = new_node(yylineno, GRAMMAR_STROP, 0, "+", 2, $1, $3); }
	| expression STROP_FOR expression { $$ = new_node(yylineno, GRAMMAR_STROP, 0, "%", 2, $1, $3); }
	| expression '+' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, 0, "+", 2, $1, $3); }
	| expression '-' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, 0, "-", 2, $1, $3); }
	| expression '*' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, 0, "*", 2, $1, $3); }
	| expression '/' expression { $$ = new_node(yylineno, GRAMMAR_OPERATION, 0, "/", 2, $1, $3); }
	| '(' expression ')' { $$ = $2; }
	;

expression_list
	: expression { $$ = $1; }
	| expression ',' expression_list { $$ = new_node(yylineno, GRAMMAR_EXPRESSION_LIST, 0, "", 2, $1, $3); }
	;

argument
	: VAR IDENTIFIER { $$ = new_node(yylineno, GRAMMAR_ARGUMENT, 0, $2, 0); }
	;

argument_list
	: argument { $$ = $1; }
	| argument ',' argument_list { $$ = new_node(yylineno, GRAMMAR_ARGUMENT_LIST, 0, "", 2, $1, $3); }
	;
%%

#include <stdio.h>
#include <stdint.h>

void yyerror(const char* string) {
	fflush(stdout);
	fprintf(stderr, "*** %s, line %d\n", string, yylineno);
	
}

typedef struct {
	char identifier[64];
	uint64_t stack_pointer;
	int scope_depth;
	
} reference_t;

typedef struct class_s {
	char identifier[64];
	
	uint64_t class_count;
	struct class_s* classes;
	
	uint64_t reference_count;
	reference_t* references;
	
} class_t;

static uint64_t stack_pointer = 0;
static class_t main_class = {0};
static class_t* current_class = &main_class;

static uint64_t inline_id = 0;
static uint64_t func_id = 0;

static int data_section_count = 0;
static int depth = 0;

static int has_defined_internal_send = 0;

class_t* create_class(class_t* self, char* identifier) {
	if (self->classes) self->classes = (class_t*) realloc(self->classes, (self->class_count + 1) * sizeof(class_t));
	else self->classes = (class_t*) malloc((self->class_count + 1) * sizeof(class_t));
	
	memset(&self->classes[self->class_count], 0, sizeof(class_t));
	strncpy(self->classes[self->class_count].identifier, identifier, sizeof(self->classes[self->class_count].identifier));
	
	return &self->classes[self->class_count++];
	
}

uint64_t create_reference_in_class(class_t* self, char* identifier) {
	if (self->references) self->references = (reference_t*) realloc(self->references, (self->reference_count + 1) * sizeof(reference_t));
	else self->references = (reference_t*) malloc((self->reference_count + 1) * sizeof(reference_t));
	
	memset(&self->references[self->reference_count], 0, sizeof(reference_t));
	strncpy(self->references[self->reference_count].identifier, identifier, sizeof(self->references[self->reference_count].identifier));
	
	self->references[self->reference_count].scope_depth = depth;
	self->references[self->reference_count].stack_pointer = stack_pointer;
	
	stack_pointer += 8;
	return self->references[self->reference_count++].stack_pointer;
	
} uint64_t create_reference(char* identifier) {
	return create_reference_in_class(current_class, identifier);
	
}

uint64_t generate_stack_entry(node_t* self) {
	self->ref_code = (char*) malloc(64);
	sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer);
	self->ref = "?ad";
	
	uint64_t current_stack_pointer = stack_pointer;
	stack_pointer += 8;
	return current_stack_pointer;
	
} void decrement_depth(void) {
	for (int i = 0; i < current_class->reference_count; i++) if (current_class->references[i].scope_depth > depth + 1) current_class->references[i].scope_depth = -1;
	depth--;
	
}

void compile(node_t* self) {
	depth++;
	//~ printf("\t# %d -> line = %d, type = %d, data = %s, children = %d\n", depth, self->line, self->type, self->data, self->child_count);
	
	if (self->type == GRAMMAR_PROGRAM) {
		fprintf(yyout, ":main:\tmov bp sp\tsub bp 1024\n");
		
	} else if (self->type == GRAMMAR_CLASS) {
		class_t* class = create_class(current_class, self->data);
		
		if (self->child_count > 0) { // has members
			node_t* member_list_root = self->children[0];
			int member = 0;
			
			while (member_list_root) {
				node_t* current_node = member_list_root;
				if (member_list_root->type == GRAMMAR_DECL_LIST) current_node = member_list_root->children[0];
				
				if (current_node->type == GRAMMAR_VAR_DECL) {
					create_reference_in_class(class, current_node->data);
					
				} else if (current_node->type == GRAMMAR_FUNC) {
					class_t* previous_class = current_class;
					current_class = class;
					compile(current_node);
					current_class = previous_class;
					
				} else if (current_node->type == GRAMMAR_CLASS) {
					/// TODO good luck with this one mate
					/// TODO good luck with this one mate
					/// TODO good luck with this one mate
					/// TODO good luck with this one mate
					/// TODO good luck with this one mate
					/// TODO good luck with this one mate
					/// TODO good luck with this one mate
					
				}
				
				if (member_list_root->type == GRAMMAR_DECL_LIST) member_list_root = member_list_root->children[1];
				else break;
				
			}
			
		}
		
		for (int i = 0; i < class->reference_count; i++) {
			printf("CLASS %s REFERENCE %d: %s\n", class->identifier, i, class->references[i].identifier);
			
		}
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_FUNC) {
		uint64_t current_func_id = func_id++;
		fprintf(yyout, "jmp $amber_func_%ld_end\t:$amber_func_%ld:\n", current_func_id, current_func_id);
		depth++;
		
		if (self->child_count > 1) { // has arguments
			node_t* argument_list_root = self->children[1];
			int argument = 0;
			
			while (argument_list_root) {
				fprintf(yyout, "cad bp sub %ld\tmov ?ad a%d\n", create_reference(argument_list_root->type == GRAMMAR_ARGUMENT_LIST ? argument_list_root->children[0]->data : argument_list_root->data), argument++);
				
				if (argument_list_root->type == GRAMMAR_ARGUMENT_LIST) argument_list_root = argument_list_root->children[1];
				else break;
				
			}
			
		}
		
		decrement_depth();
		compile(self->children[0]); // compile statement
		
		fprintf(yyout, "mov g0 0\tret\t:$amber_func_%d_end:\n", current_func_id);
		fprintf(yyout, "cad bp sub %ld\tmov ?ad $amber_func_%d_end\n", create_reference(self->data), current_func_id);
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_WHILE) {
		uint64_t current_inline_id = inline_id++;
		fprintf(yyout, "jmp $amber_inline_%ld_condition\t:$amber_inline_%ld:\n", current_inline_id, current_inline_id);
		
		compile(self->children[1]); // compile statement
		fprintf(yyout, ":$amber_inline_%ld_condition:", current_inline_id);
		
		compile(self->children[0]); // compile expression
		fprintf(yyout, "%scnd %s\tjmp $amber_inline_%ld\t:$amber_inline_%ld_end:\n", self->children[0]->ref_code, self->children[0]->ref, current_inline_id, current_inline_id);
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_IF) {
		compile(self->children[0]); // compile expression
		
		uint64_t current_inline_id = inline_id++;
		fprintf(yyout, "jmp $amber_inline_%ld_condition\t:$amber_inline_%ld:\n", current_inline_id, current_inline_id);
		
		compile(self->children[1]); // compile statement
		fprintf(yyout, "jmp $amber_inline_%ld_end\t:$amber_inline_%ld_condition:\t%scnd %s\tjmp $amber_inline_%ld\t:$amber_inline_%ld_end:\n", current_inline_id, current_inline_id, self->children[0]->ref_code, self->children[0]->ref, current_inline_id, current_inline_id);
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_IFELSE) {
		compile(self->children[0]); // compile expression
		
		uint64_t current_inline_id = inline_id++;
		fprintf(yyout, "jmp $amber_inline_%ld_condition\t:$amber_inline_%ld:\n", current_inline_id, current_inline_id);
		
		compile(self->children[1]); // compile statement (after if expression)
		fprintf(yyout, "jmp $amber_inline_%ld_end\t:$amber_inline_%ld_condition:\t%scnd %s\tjmp $amber_inline_%ld\n", current_inline_id, current_inline_id, self->children[0]->ref_code, self->children[0]->ref, current_inline_id);
		
		compile(self->children[2]); // compile statement (after else)
		fprintf(yyout, ":$amber_inline_%ld_end:\n", current_inline_id);
		
		depth--;
		return;
		
	}
	
	for (int i = 0; i < self->child_count; i++) compile(self->children[i]);
	decrement_depth();
	
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
		for (int i = 0; i < current_class->reference_count; i++) {
			if (current_class->references[i].scope_depth >= 0 && strncmp(current_class->references[i].identifier, self->data, sizeof(current_class->references[i].identifier)) == 0) {
				self->ref_code = (char*) malloc(64);
				sprintf(self->ref_code, "cad bp sub %ld\t", current_class->references[i].stack_pointer);
				
				self->ref = "?ad";
				break;
				
			}
			
		}
		
	}
	
	// statements
	
	else if (self->type == GRAMMAR_VAR_DECL) {
		uint64_t current_stack_pointer = create_reference(self->data);
		if (self->child_count) fprintf(yyout, "%smov g0 %s\tcad bp sub %ld\tmov ?ad g0\n", self->children[0]->ref_code, self->children[0]->ref, current_stack_pointer);
		
	} else if (self->type == GRAMMAR_PRINT) {
		if (self->child_count) fprintf(yyout, "%smov a0 %s\tcal print\n", self->children[0]->ref_code, self->children[0]->ref);
		else fprintf(yyout, "mov a0 0\tcal print\n");
		
	} else if (self->type == GRAMMAR_RETURN) {
		if (self->child_count) fprintf(yyout, "%smov g0 %s\tret\n", self->children[0]->ref_code, self->children[0]->ref);
		else fprintf(yyout, "mov g0 0\nret\n");
		
	}
	
	// operations
	
	else if (self->type == GRAMMAR_CALL) {
		generate_stack_entry(self);
		
		node_t* expression_list_root = (node_t*) 0;
		int argument = 0;
		
		if (*self->data && self->child_count == 1) expression_list_root = self->children[0];
		else if (!*self->data && self->child_count == 2) expression_list_root = self->children[1];
		
		while (expression_list_root) {
			if (expression_list_root->type == GRAMMAR_EXPRESSION_LIST) {
				fprintf(yyout, "%smov a%d %s\t", expression_list_root->children[0]->ref_code, argument++, expression_list_root->children[0]->ref);
				expression_list_root = expression_list_root->children[1];
				
			} else {
				fprintf(yyout, "%smov a%d %s\t", expression_list_root->ref_code, argument++, expression_list_root->ref);
				break;
				
			}
			
		}
		
		if (*self->data) {
			if (strcmp(self->data, "str") == 0) {
				uint64_t current_inline_id = inline_id++;
				
				fprintf(yyout,
					"%smov g1 a0\tmov a0 16\tcal malloc\tadd g0 a0\tmov 1?g0 0\tsub g0 1\n"
					":$amber_internal_itos_loop_inline_%ld:\tsub g0 1\tdiv g1 %s\tadd a3 48\tmov 1?g0 a3\n"
					"cnd g1\tjmp $amber_internal_itos_loop_inline_%ld\n", argument > 1 ? "mov g3 a1\t" : "", current_inline_id, argument > 1 ? "g3" : "10", current_inline_id);
				
			} else { /// TODO make this work for kos function
				//~ fprintf(yyout, "cal %s\t", self->data);
				fprintf(yyout, "%scal %s\t", self->children[0]->ref_code, self->children[0]->ref);
				
			}
			
		} else {
			fprintf(yyout, "%scal %s\t", self->children[0]->ref_code, self->children[0]->ref);
			
		}
		
		fprintf(yyout, "%smov %s g0\n", self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_ASSIGN) {
		generate_stack_entry(self);
		
		if (self->data[0] == '=') fprintf(yyout, "%smov g0 %s\t%smov %s g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
		else if (self->data[0] == '*') fprintf(yyout, "%smov g0 %s\t%smov g1 %s\tmov 1?g1 g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
		else if (self->data[0] == '?') fprintf(yyout, "%smov g0 %s\t%smov g1 %s\tmov 8?g1 g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_CMPOP) {
		generate_stack_entry(self);
		
		if (self->data[0] == '=') fprintf(yyout, "mov g0 0\t%smov g1 %s\t%smov g2 %s\tcmp g1 g2\tcnd zf\tmov g0 1\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
		else if (self->data[0] == '!') fprintf(yyout, "mov g0 1\t%smov g1 %s\t%smov g2 %s\tcmp g1 g2\tcnd zf\tmov g0 0\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_OPERATION) {
		generate_stack_entry(self);
		char* operator_instruction = "nop";
		
		if (self->data[0] == '+') operator_instruction = "add";
		else if (self->data[0] == '-') operator_instruction = "sub";
		else if (self->data[0] == '*') operator_instruction = "mul";
		else if (self->data[0] == '/') operator_instruction = "div";
		
		fprintf(yyout, "%smov g0 %s\t%s%s g0 %s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, operator_instruction, self->children[1]->ref, self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_UNARY) {
		generate_stack_entry(self);
		char* unary_code = "nop g0";
		
		if (self->data[0] == '-') unary_code = "not g0\tadd g0 1";
		else if (self->data[0] == '~') unary_code = "not g0";
		else if (self->data[0] == '*') unary_code = "mov g0 1?g0";
		else if (self->data[0] == '?') unary_code = "mov g0 8?g0";
		else if (self->data[0] == '&') unary_code = "mov g0 ad";
		
		fprintf(yyout, "%smov g0 %s\t%s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, unary_code, self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_STROP) {
		generate_stack_entry(self);
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
	//~ system("geany main.asm");
	return 0; 
}
