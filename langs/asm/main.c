
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DATA_LABEL_TOKEN '%'
#define RES_POS_LABEL_TOKEN ':'

#define IS_WHITE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == RES_POS_LABEL_TOKEN || (x) == DATA_LABEL_TOKEN || (x) == ',' || (x) == ';' || (x) == '#' || (x) == '\r' || (x) == '\0')
#define MAX_TOKEN_LENGTH 63

typedef struct {
	char data[MAX_TOKEN_LENGTH + 1];
	uint8_t bytes;
	
	uint64_t data_label_bytes;
	uint8_t* data_label_array;
	
	uint64_t reserved_position;
	
} token_t;

#include "database.h"

static FILE* input = (FILE*) 0;
static FILE* output = (FILE*) 0;

static char* code = (char*) 0;
static uint64_t code_bytes = 0;

static uint8_t assembler_verbose = 0;
static uint8_t assembler_warnings = 1;
static uint8_t assembler_extra_checks = 1;

static char* input_path = "code.asm";
static char* output_path = "rom.zed";

#define ASSEMBLER_TARGET_ZED 0
#define ASSEMBLER_TARGET_X86 1

static uint8_t assembler_target = ASSEMBLER_TARGET_ZED;

static uint64_t res_pos_label_count = 0;
static uint64_t data_label_count = 0;

static token_t* res_pos_label_identifiers = (token_t*) 0;
static token_t* data_label_identifiers = (token_t*) 0;

static uint64_t text_section_bytes = 0;
static uint8_t* text_section_data = (uint8_t*) 0;

static uint64_t rom_bytes = 0;
static uint8_t* rom_data = (uint8_t*) 0;

static uint64_t current_line_number;

static inline int64_t assembler_store_token(token_t* self, char* string) {
	memset(self, 0, sizeof(*self));
	uint8_t warn_too_big = 0;
	for (; !(self->bytes >= MAX_TOKEN_LENGTH && (warn_too_big = 1) /* look at this beauty */) && !IS_WHITE(*string); self->bytes++) self->data[self->bytes] = *string++;
	if (warn_too_big && assembler_warnings) printf("WARNING Line %ld, token %s has surpassed the maximum length (%d)\n", current_line_number, self->data, MAX_TOKEN_LENGTH);
	return self->bytes - 1;
	
} static inline int64_t assembler_token_index(uint64_t count, token_t* list, token_t* comparator) { // search through list of tokens and find index of matching token (returns -1 if no match)
	for (uint64_t i = 0; i < count; i++) if (strcmp(list[i].data, comparator->data) == 0) return i;
	return -1;
	
} static inline uint8_t assembler_token_to_number(token_t* self, int64_t* value_reference) { // returns 0 on success
	char* endptr = (char*) 0;
	
	if      (*self->data == 'x') *value_reference = strtoll(self->data + 1, &endptr, 16); // hexadecimal
	else if (*self->data == 'b') *value_reference = strtoll(self->data + 1, &endptr, 2);  // binary
	else                         *value_reference = strtoll(self->data,     &endptr, 10); // decimal
	
	return endptr == self->data || strlen(endptr);
	
}

static inline void assembler_add_8bit_value(uint8_t value) {
	text_section_data = (uint8_t*) realloc(text_section_data, text_section_bytes + 2);
	text_section_data[text_section_bytes++] = value;
	
} static inline void assembler_add_64bit_value(uint64_t value) {
	text_section_bytes = ((text_section_bytes - 1) / sizeof(uint64_t) + 1) * sizeof(uint64_t);
	text_section_data = (uint8_t*) realloc(text_section_data, ((text_section_bytes - 1) / sizeof(uint64_t) + 1) * sizeof(uint64_t) + 1);
	((uint64_t*) text_section_data)[(text_section_bytes - 1) / sizeof(uint64_t) + 1] = value;
	text_section_bytes += sizeof(uint64_t);
	
}

static inline void assembler_add_token(uint8_t type, uint64_t data) {
	type = type == TOKEN_NUMBER && data < 0x100 ? TOKEN_BYTE : type;
	assembler_add_8bit_value(type);
	
	if (type == TOKEN_NUMBER || type == TOKEN_RES_POS || type == TOKEN_RESERVED) assembler_add_64bit_value(data);
	else                                                                         assembler_add_8bit_value (data);
	
}

static int assemble(void) {
	token_t* res_pos_label_identifiers = (token_t*) malloc(sizeof(token_t));
	token_t* data_label_identifiers = (token_t*) malloc(sizeof(token_t));
	
	res_pos_label_count = 0;
	data_label_count = 0;
	
	token_t* current_data_label = (token_t*) 0;
	uint8_t in_comment = 0;
	
	if (assembler_verbose) printf("Indexing all the label names and parsing data sections ...\n");
	current_line_number = 1;
	for (uint64_t i = 0; i < code_bytes; i++) {
		char* current = code + i;
		
		if (*current == '\n') {
			current_line_number++;
			in_comment = 0;
			
		} else if (*current == '#') {
			in_comment = !in_comment;
			
		} else if (!in_comment) {
			if (current_data_label) {
				if (*current == DATA_LABEL_TOKEN) { // found end of data label
					current_data_label = (token_t*) 0;
					
				} else if (!IS_WHITE(*current)) {
					token_t token;
					i += assembler_store_token(&token, current);
					
					int64_t byte = 0;
					if (assembler_token_to_number(&token, &byte)) {
						printf("WARNING Line %ld, found unknown token %s in data label %s\n", current_line_number, token.data, current_data_label->data);
						
					} if (byte > 0xFF) {
						printf("WARNING Line %ld, value %ld does not fit in a byte\n", current_line_number, byte);
						byte %= 0x100;
						
					}
					
					current_data_label->data_label_array = (uint8_t*) realloc(current_data_label->data_label_array, (current_data_label->data_label_bytes + 2) * sizeof(uint8_t));
					current_data_label->data_label_array[current_data_label->data_label_bytes++] = byte;
					
				}
				
			} else if (*current == RES_POS_LABEL_TOKEN) { // found reserved position label
				res_pos_label_identifiers = (token_t*) realloc(res_pos_label_identifiers, (res_pos_label_count + 2) * sizeof(token_t));
				i += 2 + assembler_store_token(&res_pos_label_identifiers[res_pos_label_count++], current + 1);
				if (assembler_extra_checks && assembler_token_index(res_pos_label_count, res_pos_label_identifiers, &res_pos_label_identifiers[res_pos_label_count - 1]) < res_pos_label_count - 1) printf("WARNING Line %ld, reserved position label %s declared multiple times\n", current_line_number, res_pos_label_identifiers[res_pos_label_count - 1].data);
				
			} else if (*current == DATA_LABEL_TOKEN) { // found data label
				data_label_identifiers = (token_t*) realloc(data_label_identifiers, (data_label_count + 2) * sizeof(token_t));
				i += 1 + assembler_store_token(&data_label_identifiers[data_label_count], current + 1);
				if (assembler_extra_checks && assembler_token_index(data_label_count, data_label_identifiers, &data_label_identifiers[data_label_count]) < data_label_count) printf("WARNING Line %ld, data label %s declared multiple times\n", current_line_number, data_label_identifiers[data_label_count].data);
				data_label_identifiers[data_label_count].data_label_array = (uint8_t*) malloc(1);
				current_data_label = &data_label_identifiers[data_label_count++];
				
			}
			
		}
		
	}
	
	in_comment = 0;
	uint8_t in_data_section = 0;
	uint8_t found_main_label = 0;
	
	text_section_bytes = 0;
	text_section_data = (uint8_t*) malloc(1);
	*text_section_data = 0;
	
	if (assembler_verbose) printf("Reading assembly and building text section ...\n");
	current_line_number = 1;
	for (uint64_t i = 0; i < code_bytes; i++) {
		char* current = code + i;
		
		if (*current == '\n') {
			current_line_number++;
			in_comment = 0;
			
		} else if (*current == '#') {
			in_comment = !in_comment;
			
		} else if (!in_comment) {
			if (in_data_section) { // stall text section reading if inside data section
				if (*current == DATA_LABEL_TOKEN) { // found end of data label
					in_data_section = 0;
					
				}
				
			} else if (*current == DATA_LABEL_TOKEN) { // found data label
				in_data_section = 1;
				
			} else if (*current == RES_POS_LABEL_TOKEN) { // found reserved position label
				token_t identifier;
				i += 2 + assembler_store_token(&identifier, current + 1);
				res_pos_label_identifiers[assembler_token_index(res_pos_label_count, res_pos_label_identifiers, &identifier)].reserved_position = 1337; /// TODO
				
				if (strcmp(identifier.data, "main") == 0) {
					found_main_label = 1;
					/// TODO set main reserved position
					
				}
				
			} else if (!IS_WHITE(*current)) {
				token_t token;
				i += 1 + assembler_store_token(&token, current);
				int64_t instruction, _register, res_pos_label, data_label, prereserved;
				
				if      ((instruction   = assembler_token_index(sizeof(assembler_instructions) / sizeof(*assembler_instructions), assembler_instructions, &token)) >= 0) assembler_add_token(TOKEN_INSTRUCTION, instruction);
				else if ((_register     = assembler_token_index(sizeof(assembler_registers   ) / sizeof(*assembler_registers   ), assembler_registers,    &token)) >= 0) assembler_add_token(TOKEN_REGISTER, _register);
				else if ((res_pos_label = assembler_token_index(res_pos_label_count, res_pos_label_identifiers,                                           &token)) >= 0) assembler_add_token(TOKEN_RES_POS, res_pos_label);
				else if ((data_label    = assembler_token_index(data_label_count, data_label_identifiers,                                                 &token)) >= 0) assembler_add_token(TOKEN_RESERVED, sizeof(assembler_prereserved) / sizeof(*assembler_prereserved) + data_label);
				else if ((prereserved   = assembler_token_index(sizeof(assembler_prereserved ) / sizeof(*assembler_prereserved ), assembler_prereserved,  &token)) >= 0) assembler_add_token(TOKEN_RESERVED, prereserved);
					
				} else if (token.data[1] == '[' || token.data[0] == '[') { /// TODO addresses
					if (token.data[0] == '8' || token.data[0] == '[') { // 64 bit addressing
						printf("ADDRESS 8 byte %s\n", token.data);
						
					} else if (token.data[0] == '1') { // 8 bit addressing
						printf("ADDRESS 1 byte %s\n", token.data);
						
					} else {
						printf("WARNING Line %ld, unknown addressing type %c\n", current_line_number, *current);
						
					}
					
				} else { // test if maybe this is a number literal
					int64_t number = 0;
					if (assembler_token_to_number(&token, &number)) {
						printf("WARNING Line %ld, unknown token or identifier %s\n", current_line_number, token.data);
						
					} else {
						//~ printf("NUMBER %ld\n", number);
						assembler_add_token(TOKEN_NUMBER, number);
						
					}
					
				}
				
			}
			
		}
		
	}
	
	for (uint64_t i = 0; i < text_section_bytes; i++) {
		printf("%x\t", text_section_data[i]);
		if (!((i + 1) % 8)) printf("\n");
		
	}
	
	if (!found_main_label) {
		printf("WARNING Could not find the main reserved position label\n");
		
	}
	
	return 0;
	
}

static int build_rom(void) {
	fprintf(stderr, "TODO %s\n", __func__);
	return 1;
	
}

static void assembler_free(void) {
	if (input) fclose(input);
	if (output) fclose(output);
	
	if (code) free(code);
	if (rom_data) free(rom_data);
	
	for (uint64_t i = 0; i < data_label_count; i++) if (data_label_identifiers[i].data_label_array) free(data_label_identifiers[i].data_label_array);
	
	if (res_pos_label_identifiers) free(res_pos_label_identifiers);
	if (data_label_identifiers) free(data_label_identifiers);
	
	if (text_section_data) free(text_section_data);
	
}

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "verbose") == 0) {
			assembler_verbose = 1;
			printf("Verbose flag set\n");
			
		} else if (strcmp(argv[i], "suppress") == 0) {
			assembler_warnings = 0;
			if (assembler_verbose) printf("Disabled warnings\n");
			
		} else if (strcmp(argv[i], "no-checks") == 0) { /// TODO find "assembler_extra_checks" and see if this is actually used. Otherwise, remove it, obv
			assembler_extra_checks = 0;
			if (assembler_verbose) printf("Disabled extra checks\n");
			
		} else if (strcmp(argv[i], "in") == 0) {
			input_path = argv[++i];
			if (assembler_verbose) printf("Set input path to %s\n", input_path);
			
		} else if (strcmp(argv[i], "out") == 0) {
			output_path = argv[++i];
			if (assembler_verbose) printf("Set output path to %s\n", output_path);
			
		} else if (strcmp(argv[i], "target") == 0) { /// TODO not sure if this will even be in C version, or if ill wait until rewriting this assembler in Amber
			char* target = argv[++i];
			
			if      (strcmp(target, "zed") == 0) assembler_target = ASSEMBLER_TARGET_ZED;
			else if (strcmp(target, "x86") == 0) assembler_target = ASSEMBLER_TARGET_X86;
			
			else if (assembler_verbose) {
				printf("WARNING Unknown target %s\n", target);
				
			}
			
		} else if (assembler_verbose) {
			printf("WARNING Unknown flag %s\n", argv[i]);
			
		}
		
	}
	
	input = fopen(input_path, "rb");
	if (!input) {
		fprintf(stderr, "ERROR Could not load %s file (as input)\n", input_path);
		assembler_free();
		return 1;
		
	}
	
	output = fopen(output_path, "wb");
	if (!output) {
		fprintf(stderr, "ERROR Could not load %s file (as output)\n", output_path);
		assembler_free();
		return 1;
		
	}
	
	fseek(input, 0, SEEK_END);
	code_bytes = ftell(input);
	if (assembler_verbose) printf("ASM File is %ld bytes long\n", code_bytes);
	rewind(input);
	
	code = (char*) malloc(code_bytes);
	fread(code, sizeof(char), code_bytes, input);
	
	if (assemble()) {
		fprintf(stderr, "ERROR Failed to assemble\n");
		assembler_free();
		return 1;
		
	} if (build_rom()) {
		fprintf(stderr, "ERROR Failed to build ROM\n");
		assembler_free();
		return 1;
		
	}
	
	fwrite(rom_data, sizeof(uint8_t), rom_bytes, output);
	if (assembler_verbose) printf("Assembler finished with success\n");
	
	assembler_free();
	return 0;
	
}
