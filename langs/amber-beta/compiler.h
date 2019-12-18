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
extern char* current_file;

void yyerror(const char* message) {
	fflush(stdout);
	fprintf(stderr, "ERROR (%s) on line %d\n", message, yylineno);
}

enum grammar_e {
	GRAMM_PROGRAM,
	
	GRAMM_STATEMENT, GRAMM_EXPRESSION, GRAMM_ARGUMENT, GRAMM_ATTRIBUTE, // big syntax elements
	GRAMM_LIST_IDENTIFIER, GRAMM_LIST_STATEMENT, GRAMM_LIST_EXPRESSION, GRAMM_LIST_ARGUMENT, GRAMM_LIST_ATTRIBUTE, // lists
	
	GRAMM_CALL, // expressions
	GRAMM_ASSIGN, GRAMM_CAST, // class stuff
	GRAMM_COMPARE, GRAMM_STR_COMPARE, GRAMM_LOGIC, GRAMM_OPERATION, GRAMM_STR_OPERATION, GRAMM_UNARY, GRAMM_ACCESS, // arithmetic expressions
	GRAMM_VAR_DECL, GRAMM_FUNC, GRAMM_CLASS, // declaration statements
	GRAMM_IF, GRAMM_WHILE, GRAMM_CONTROL, // statements
	GRAMM_IDENTIFIER, GRAMM_NUMBER, GRAMM_STRING, // literals
};

typedef struct node_s {
	uint8_t child_count;
	struct node_s* children[16];
	
	struct node_s* parent;
	void* class;
	
	uint8_t type;
	int line;
	
	uint8_t is_raw_class_name;
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
	memset(self, 0, sizeof(node_t));
	
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
static uint64_t function_count = 0;

static uint64_t stack_pointer = 0;
static uint64_t depth = 0;

static uint64_t reference_count = 0;
static node_t** references = (node_t**) 0;

static uint8_t has_defined_internal_send = 0;
static const char* argument_registers[] = {"a0", "a1", "a2", "a3", "g0", "g1", "g2"};

uint64_t generate_stack_entry(node_t* self) {
	self->ref_code = (char*) malloc(32);
	sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer += 8 - (stack_pointer - 1) % 8 + 8 - 1);
	self->ref = "?ad";
	return stack_pointer;
} node_t* create_reference(node_t* self, uint8_t width) {
	self->width = width;
	self->scope_depth = depth;
	self->stack_pointer = (stack_pointer += self->width - (stack_pointer - 1) % self->width + self->width - 1);
	
	if (references) references = (node_t**) realloc(references, (reference_count + 1) * sizeof(node_t*));
	else references = (node_t**) malloc((reference_count + 1) * sizeof(node_t*));
	
	return references[reference_count++] = self;
} void decrement_depth(void) {
	for (uint64_t i = 0; i < reference_count; i++) if (references[i]->scope_depth > depth) references[i]->scope_depth = -1;
	depth--;
}

// class helper functions

typedef struct {
	const char* name;
	uint64_t bytes;
	uint64_t offset;
	void* class;
	
	const char* initial_value_ref_code;
	const char* initial_value_ref;
} class_variable_t;

typedef struct class_s {
	const char* name;
	uint64_t depth;
	uint64_t bytes;
	
	struct class_s* parent;
	//~ struct class_s* classes;
	struct class_s* classes[512];
	uint64_t class_count;
	
	//~ node_t** functions;
	node_t* functions[512];
	uint64_t function_count;
	
	//~ class_variable_t* variables;
	class_variable_t variables[512];
	uint64_t variable_count;
} class_t;

static class_t main_class = { .name = "Main" };
static uint8_t compiling_class = 0;

static class_t* class_stack[16] = {&main_class}; // 16 is the maximum nesting number for classes
static uint64_t class_stack_index = 0;

void create_class(const char* name) {
	compiling_class = 1;
	class_t* parent = class_stack[class_stack_index];
	
	//~ if (parent->class_count) parent->classes = (class_t*) realloc(parent->classes, (parent->class_count + 1) * sizeof(class_t));
	//~ else parent->classes = (class_t*) malloc(sizeof(class_t));
	
	parent->classes[parent->class_count] = (class_t*) malloc(sizeof(class_t));
	class_t* self = parent->classes[parent->class_count++];
	memset(self, 0, sizeof(*self));
	self->parent = parent;
	
	self->name = name;
	self->depth = depth;
	
	class_stack[++class_stack_index] = self;
} void class_add_function(node_t* function) {
	class_t* self = class_stack[class_stack_index];
	
	//~ if (self->function_count) self->functions = (node_t**) realloc(self->functions, (self->function_count + 1) * sizeof(node_t*));
	//~ else self->functions = (node_t**) malloc(sizeof(node_t*));
	
	self->functions[self->function_count++] = function;
} void class_add_variable(node_t* declaration, uint8_t bytes, uint64_t times, const char* value_ref_code, const char* value_ref) {
	class_t* self = class_stack[class_stack_index];
	
	//~ if (self->variable_count) self->variables = (class_variable_t*) realloc(self->variables, (self->variable_count + 1) * sizeof(class_variable_t));
	//~ else self->variables = (class_variable_t*) malloc(sizeof(class_variable_t));
	
	self->variables[self->variable_count].name = declaration->data;
	self->variables[self->variable_count].bytes = bytes * times;
	self->variables[self->variable_count].class = declaration->class;
	
	self->variables[self->variable_count].offset = self->bytes;
	self->bytes += (bytes - (self->bytes - 1) % bytes + bytes - 1) * times;
	
	self->variables[self->variable_count].initial_value_ref_code = value_ref_code;
	self->variables[self->variable_count++].initial_value_ref = value_ref;
} void exit_class(void) {
	class_stack_index--;
}

void compile(node_t* self) {
	if (!self) return;
	
	depth++;
	self->ref = self->data;
	
	if (self->parent) self->class = self->parent->class;
	else self->class = compiling_class ? class_stack[class_stack_index] : &main_class;
	
	//~ printf("node = %p\tline = %d\ttype = %d\tdata = %s\n", self, self->line, self->type, self->data);
	
	// big syntax elements
	
	if (self->type == GRAMM_LIST_STATEMENT) {
		compile(self->children[0]);
		compile(self->children[1]);
	}
	
	// expressions
	
	else if (self->type == GRAMM_ACCESS) {
		compile(self->children[0]);
		self->children[1]->parent = self->children[0];
		compile(self->children[1]);
		self->class = self->children[1]->class;
		
		self->ref = self->children[1]->ref;
		self->ref_code = self->children[1]->ref_code;
	} else if (self->type == GRAMM_CAST) {
		compile(self->children[0]);
		compile(self->children[1]);
		
		self->class = self->children[0]->class;
		self->ref = self->children[1]->ref;
		self->ref_code = self->children[1]->ref_code;
	} else if (self->type == GRAMM_OPERATION) {
		compile(self->children[0]);
		compile(self->children[1]);
		
		generate_stack_entry(self);
		char* instruction = "nop";
		
		if      (*self->data == '|') instruction = "or";
		else if (*self->data == '^') instruction = "xor";
		else if (*self->data == '&') instruction = "and";
		else if (*self->data == '<') instruction = "shl";
		else if (*self->data == '>') instruction = "shr";
		else if (*self->data == 'r') instruction = "ror";
		else if (*self->data == '+') instruction = "add";
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
		else if (*self->data == '[') fprintf(yyout, "mov g0 0\t%smov g1 %s\t%scmp g1 %s\tcnd zf\tmov g0 1\tcmp sf of\txor zf 1\tcnd zf\tmov g0 1\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
		else if (*self->data == '>') fprintf(yyout, "mov g0 1\t%smov g1 %s\t%scmp g1 %s\tcnd zf\tmov g0 0\tcmp sf of\txor zf 1\tcnd zf\tmov g0 0\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
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
			else if (*self->data == '|') code = "cmp g0 0\txor zf 1\tmov g0 zf";
			else if (*self->data == '!') code = "cmp g0 0\tmov g0 zf";
			
			fprintf(yyout, "%smov g0 %s\t%s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, code, self->ref_code, self->ref);
		}
	} else if (self->type == GRAMM_CALL) {
		if (strcmp(self->children[0]->data, "return") == 0) { // return
			compile(self->children[1]);
			fprintf(yyout, "%smov g0 %s\tret\n", self->children[1]->ref_code, self->children[1]->ref);
		} else {
			if (strcmp(self->children[0]->data, "classof") == 0) { // classof
				compile(self->children[1]);
				fprintf(yyout, "%%$amber_class_name_%ld \"%s<%p>\" 0%%\tmov g0 $amber_class_name_%ld\t", data_section_count, ((class_t*) self->children[1]->class)->name, self->children[1]->class, data_section_count);
				data_section_count++;
			} else if (strcmp(self->children[0]->data, "new") == 0) { // new
				compile(self->children[1]);
				fprintf(yyout, "%smov a0 %s\tcal malloc\tmov a2 a0\tmov a0 g0\tmov a1 0\tcal mset\tmov g0 a0\t", self->children[1]->ref_code, self->children[1]->ref);
				self->class = self->children[1]->class;
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
					
					fprintf(yyout, "%smov %s %s\t", argument_node->ref_code, argument_registers[argument++], argument_node->ref);
					
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
					uint8_t statically_called = 1;
					if (self->children[0]->child_count && !self->children[0]->children[1]->parent->is_raw_class_name) {
						statically_called = 0;
						fprintf(yyout, "%smov g3 %s\t", self->children[0]->children[1]->parent->ref_code, self->children[0]->children[1]->parent->ref);
					}
					
					if (statically_called) fprintf(yyout, "mov g3 0\t");
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
		
		uint8_t class = self->data_bytes;
		if (class) {
			compile(self->children[self->child_count - 1]);
			self->class = self->children[self->child_count - 1]->class;
		}
		
		if ((self->child_count > 2 && !class) || (self->child_count > 3 && class)) { // is also assignment?
			compile(self->children[2]);
			if (!class) self->class = self->children[2]->class;
			
			ref_code = self->children[2]->ref_code;
			ref = self->children[2]->ref;
		}
		
		uint8_t bytes = (uint8_t) (uint64_t) self->children[1];
		uint64_t times = (uint64_t) self->children[0];
		
		if (compiling_class) {
			class_add_variable(self, bytes, times, ref_code, ref);
		} else {
			create_reference(self, bytes);
			fprintf(yyout, "%smov g0 %s\tcad bp sub %ld\tmov ?ad g0\n", ref_code, ref, self->stack_pointer);
		}
	} else if (self->type == GRAMM_FUNC) {
		uint8_t local = 1; /// TODO global attribute
		uint64_t current_func_id;
		depth++;
		
		if (local) {
			current_func_id = function_count++;
			fprintf(yyout, "jmp $amber_func_%ld$end\t:$amber_func_%ld:\n", current_func_id, current_func_id);
		} else {
			fprintf(yyout, "jmp %s$end\t:%s:\n", self->data, self->data);
		}
		
		uint8_t prev_compiling_class = compiling_class;
		compiling_class = 0;
		
		uint64_t argument = 0;
		if (self->child_count > 1) { // has arguments?
			node_t* argument_list_root = self->children[1];
			while (argument_list_root) {
				node_t* argument_node = argument_list_root->type == GRAMM_LIST_ARGUMENT ? argument_list_root->children[0] : argument_list_root;
				create_reference(argument_node, (uint8_t) (uint64_t) argument_node->children[0]);
				fprintf(yyout, "cad bp sub %ld\tmov ?ad %s\n", argument_node->stack_pointer, argument_registers[argument++]);
				
				if (argument_node->child_count > 1) { // has a specific class
					compile(argument_node->children[1]);
					argument_node->class = argument_node->children[1]->class;
				}
				
				if (argument_list_root->type == GRAMM_LIST_ARGUMENT) argument_list_root = argument_list_root->children[1];
				else break;
			}
		}
		
		node_t* imaginary_self_node = (node_t*) malloc(sizeof(node_t));
		memset(imaginary_self_node, 0, sizeof(node_t));
		
		imaginary_self_node->type = GRAMM_VAR_DECL;
		imaginary_self_node->data = "self";
		
		imaginary_self_node->class = class_stack[class_stack_index];
		create_reference(imaginary_self_node, 8);
		fprintf(yyout, "cad bp sub %ld\tmov ?ad g3\n", imaginary_self_node->stack_pointer);
		
		compile(self->children[0]);
		compiling_class = prev_compiling_class;
		
		decrement_depth();
		node_t* function_reference = create_reference(self, 8);
		
		if (local) {
			fprintf(yyout, "mov g0 0\tret\t:$amber_func_%ld$end:\n", current_func_id);
			fprintf(yyout, "cad bp sub %ld\tmov ?ad $amber_func_%ld\n", self->stack_pointer, current_func_id);
		} else {
			fprintf(yyout, "mov g0 0\tret\t:%s$end:\n", self->data);
			fprintf(yyout, "cad bp sub %ld\tmov ?ad %s\n", self->stack_pointer, self->data);
		}
		
		if (compiling_class) {
			class_add_function(function_reference);
		}
	} else if (self->type == GRAMM_CLASS) {
		uint8_t prev_compiling_class = compiling_class;
		create_class(self->data);
		depth++;
		compile(self->children[0]); // compile statement
		exit_class();
		compiling_class = prev_compiling_class;
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
		
		compile(self->children[0]); // compile statement
		fprintf(yyout, ":$amber_loop_%ld_condition:", current);
		
		if (self->child_count > 1) { // has condition expression?
			compile(self->children[1]); // compile expression
			fprintf(yyout, "%scnd %s\t", self->children[1]->ref_code, self->children[1]->ref);
		}
		
		fprintf(yyout, "jmp $amber_loop_%ld\t:$amber_loop_%ld_end:\n", current, current);
	} else if (self->type == GRAMM_CONTROL) {
		if (strcmp(self->data, "break") == 0) fprintf(yyout, "jmp $amber_loop_%ld_end\n", loop_count - 1);
		else if (strcmp(self->data, "continue") == 0) fprintf(yyout, "jmp $amber_loop_%ld_condition\n", loop_count - 1);
	}
	
	// literals
	
	else if (self->type == GRAMM_IDENTIFIER) {
		uint8_t stop = 0;
		
		for (int64_t i = ((class_t*) self->class)->variable_count - 1; !stop && i >= 0; i--) if (strcmp(self->data, ((class_t*) self->class)->variables[i].name) == 0) {
			self->ref_code = (char*) malloc(strlen(self->parent->ref_code) + 32);
			sprintf(self->ref_code, "%scad %s add %ld\t", self->parent->ref_code, self->parent->ref, ((class_t*) self->class)->variables[i].offset);
			self->class = ((class_t*) self->class)->variables[i].class;
			
			self->ref = "?ad";
			stop = 1;
			break;
		} for (int64_t i = ((class_t*) self->class)->function_count - 1; !stop && i >= 0; i--) if (strcmp(self->data, ((class_t*) self->class)->functions[i]->data) == 0) {
			self->ref_code = (char*) malloc(32);
			sprintf(self->ref_code, "cad bp sub %ld\t", self->stack_pointer = ((class_t*) self->class)->functions[i]->stack_pointer);
			
			self->ref = "?ad";
			stop = 1;
			break;
		}
		
		class_t* current = class_stack[class_stack_index];
		uint8_t first_loop = 1;
		
		while (!stop) {
			for (int64_t i = current->class_count - 1; !stop && i >= 0; i--) if (strcmp(self->data, current->classes[i]->name) == 0) {
				self->ref_code = "";
				self->ref = (char*) malloc(16);
				
				self->is_raw_class_name = 1;
				self->class = current->classes[i];
				sprintf(self->ref, "%ld", (uint64_t) current->classes[i]->bytes);
				
				stop = 1;
				break;
			}
			
			if (first_loop) {
				first_loop = 0;
				current = (class_t*) self->class;
			} else {
				if (current->parent) current = current->parent;
				else break;
			}
		}
		
		for (int64_t i = reference_count - 1; !stop && i >= 0; i--) if (references[i]->scope_depth >= 0 && strcmp(self->data, references[i]->data) == 0) {
			self->class = references[i]->class;
			self->ref_code = (char*) malloc(32);
			sprintf(self->ref_code, "cad bp sub %ld\t", self->stack_pointer = references[i]->stack_pointer);
			
			self->ref = "?ad";
			stop = 1;
			break;
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

extern char import_prefix[1024];

int main(int argc, char* argv[]) {
	if (argc > 1) {
		uint64_t last_slash = 0;
		for (uint64_t i = 0; i < strlen(argv[1]); i++) if (argv[1][i] == '/') last_slash = i;
		memcpy(import_prefix, argv[1], last_slash + 1);
		
		current_file = argv[1];
		
		FILE* file = fopen(argv[1], "r");
		if (!file) {
			fprintf(stderr, "WARNING Could not open file %s for reading\n", argv[1]);
			exit(1);
		}
		
		yyin = file;
		yyout = stdout;
		
		if (argc > 2) {
			FILE* file = fopen(argv[2], "w");
			if (!file) {
				fprintf(stderr, "WARNING Could not open file %s for writing\n", argv[2]);
				exit(1);
			}
			
			yyout = file;
		}
	}
	
	yyparse();
	fclose(yyout);
	return 0; 
}
