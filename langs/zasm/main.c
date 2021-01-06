// pretty horrendous code, I know, I need to rewrite this some day

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <compiler_common.h>

#define DATA_LABEL_TOKEN '%'
#define POSITION_LABEL_TOKEN ':'

#define IS_WHITE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == DATA_LABEL_TOKEN || (x) == POSITION_LABEL_TOKEN || (x) == ',' || (x) == ';' || (x) == '#' || (x) == '\r' || (x) == '\0')
#define MAX_TOKEN_LENGTH 63 /// TODO maybe make increase this or remove the limit entirely?

typedef struct {
	char data[MAX_TOKEN_LENGTH + 1];
	uint8_t bytes;
	
	uint64_t data_label_bytes;
	uint8_t* data_label_array;
	
	uint64_t position;
} token_t;

#include "database.h"

static FILE* input = (FILE*) 0;
static FILE* output = (FILE*) 0;

static char* code = (char*) 0;
static uint64_t code_bytes = 0;

static uint8_t assembler_verbose = 0;
static uint8_t assembler_warnings = 1;
static uint8_t assembler_extra_checks = 1;

static char* input_path = "main.zasm";
static char* output_path = ".";

static uint64_t position_label_count = 1;
static uint64_t data_label_count = 0;

static token_t* position_label_identifiers = (token_t*) 0;
static token_t* data_label_identifiers = (token_t*) 0;

static uint64_t logic_section_words = 0;
static uint64_t* logic_section_data = (uint64_t*) 0;

static uint64_t rom_bytes = 0;
static uint8_t* rom_data = (uint8_t*) 0;

static uint64_t current_line_number;

// rom helper functions

static inline void rom_add_data(uint64_t data) {
	logic_section_data = (uint64_t*) realloc(logic_section_data, (logic_section_words + 1) * sizeof(uint64_t));
	logic_section_data[logic_section_words++] = data;
}

static uint8_t instruction_validated = 1;
static uint8_t current_operand;

static uint64_t current_instruction_position;
static zed_instruction_t current_instruction;

static inline void rom_validate_instruction(void) {
	if (!instruction_validated) {
		logic_section_data[current_instruction_position] = *((uint64_t*) &current_instruction);
	}
}

#define TOKEN_TYPE_INSTRUCTION 255

static inline void rom_add_token(uint8_t type, uint64_t data) { /// TODO now that i think of it, this should probably go in compiler_common.h
	if (type == TOKEN_TYPE_INSTRUCTION) {
		rom_validate_instruction();
		instruction_validated = 0;

		current_instruction_position = logic_section_words;
		rom_add_data(0); // leave space for the instruction
		
		memset(&current_instruction, 0, sizeof(current_instruction));
		current_instruction.operation = data & 0xF;
		current_operand = 1;

	} else {
		uint8_t is_64_bit_operand = data > 0xFFFF; // does the data need more than 16 bits to be represented?
		
		if (current_operand == 1) {
			current_instruction.operand1_64_bit = is_64_bit_operand;
			current_instruction.operand1_type = type & 0b111;
			if (!is_64_bit_operand) current_instruction.operand1_data = data & 0xFFFF;

		} else if (current_operand == 2) {
			current_instruction.operand2_64_bit = is_64_bit_operand;
			current_instruction.operand2_type = type & 0b111;
			if (!is_64_bit_operand) current_instruction.operand2_data = data & 0xFFFF;

		} else if (current_operand == 3) {
			current_instruction.operand3_64_bit = is_64_bit_operand;
			current_instruction.operand3_type = type & 0b111;
			if (!is_64_bit_operand) current_instruction.operand3_data = data & 0xFFFF;
		}

		current_operand++;

		if (is_64_bit_operand) {
			rom_add_data(data);
		}
	}
}

// assembler helper functions

static inline int64_t assembler_store_token(token_t* self, char* string) {
	memset(self, 0, sizeof(*self));
	uint8_t warn_too_big = 0;
	for (; !(self->bytes >= MAX_TOKEN_LENGTH && (warn_too_big = 1) /* look at this beauty */) && !IS_WHITE(*string); self->bytes++) self->data[self->bytes] = *string++;
	if (warn_too_big && assembler_warnings) printf("[ZASM Language] WARNING Line %ld, token %s has surpassed the maximum length (%d)\n", current_line_number, self->data, MAX_TOKEN_LENGTH);
	return self->bytes - 1;
	
} static inline int64_t assembler_token_index_with_string(uint64_t count, token_t* list, const char* comparator) { // search through list of tokens and find index of matching token (returns -1 if no match)
	for (uint64_t i = 0; i < count; i++) if (strcmp(list[i].data, comparator) == 0) return i;
	return -1;
	
} static inline int64_t assembler_token_index(uint64_t count, token_t* list, token_t* comparator) {
	return assembler_token_index_with_string(count, list, (const char*) comparator->data);
	
} static inline uint8_t assembler_token_to_number(token_t* self, int64_t* value_reference) { // returns 0 on success
	char* endptr = (char*) 0;
	
	if      (*self->data == 'x') *value_reference = strtoll(self->data + 1, &endptr, 16); // hexadecimal
	else if (*self->data == 'b') *value_reference = strtoll(self->data + 1, &endptr, 2);  // binary
	else                         *value_reference = strtoll(self->data,     &endptr, 10); // decimal
	
	return endptr == self->data || strlen(endptr);
}

// actual nice, juicy, meaty functions

static int assemble(void) {
	position_label_identifiers = (token_t*) malloc((position_label_count + 2) * sizeof(token_t));
	data_label_identifiers = (token_t*) malloc(sizeof(token_t));
	
	position_label_count = 0;
	data_label_count = 0;
	
	token_t* current_data_label = (token_t*) 0;
	uint8_t in_comment = 0;
	uint8_t in_string = 0;
	
	if (assembler_verbose) printf("[ZASM Language] Indexing all the label names and parsing data sections ...\n");
	current_line_number = 1;
	for (uint64_t i = 0; i < code_bytes; i++) {
		char* current = code + i;
		
		if (!in_string && *current == '\n') {
			current_line_number++;
			in_comment = 0;
			
		} else if (!in_comment && *current == '"') {
			in_string = !in_string;
			
		} else if (!in_string && *current == '#') {
			in_comment = !in_comment;
			
		} else if (!in_comment) {
			if (current_data_label) {
				if (in_string) {
					current_data_label->data_label_array = (uint8_t*) realloc(current_data_label->data_label_array, (current_data_label->data_label_bytes + 2) * sizeof(uint8_t));
					current_data_label->data_label_array[current_data_label->data_label_bytes++] = *current;
					
					if (*current == '\n') {
						current_line_number++;
						in_comment = 0;
					}
					
				} else if (*current == DATA_LABEL_TOKEN) { // found end of data label
					current_data_label = (token_t*) 0;
					
				} else if (!IS_WHITE(*current)) {
					token_t token;
					i += assembler_store_token(&token, current);
					
					int64_t byte = 0;
					if (assembler_token_to_number(&token, &byte)) {
						printf("[ZASM Language] WARNING Line %ld, found unknown token %s in data label %s\n", current_line_number, token.data, current_data_label->data);
						
					} if (byte > 0xFF) {
						printf("[ZASM Language] WARNING Line %ld, value %ld does not fit in a byte\n", current_line_number, byte);
						byte %= 0x100;
					}
					
					current_data_label->data_label_array = (uint8_t*) realloc(current_data_label->data_label_array, (current_data_label->data_label_bytes + 2) * sizeof(uint8_t));
					current_data_label->data_label_array[current_data_label->data_label_bytes++] = byte;
				}
				
			} else if (!in_string && *current == POSITION_LABEL_TOKEN) { // found position label
				position_label_identifiers = (token_t*) realloc(position_label_identifiers, (position_label_count + 2) * sizeof(token_t));
				i += 2 + assembler_store_token(&position_label_identifiers[position_label_count++], current + 1);
				if (assembler_extra_checks && assembler_token_index(position_label_count, position_label_identifiers, &position_label_identifiers[position_label_count - 1]) < position_label_count - 1) printf("[ZASM Language] WARNING Line %ld, position label %s declared multiple times\n", current_line_number, position_label_identifiers[position_label_count - 1].data);
				
			} else if (!in_string && *current == DATA_LABEL_TOKEN) { // found data label
				data_label_identifiers = (token_t*) realloc(data_label_identifiers, (data_label_count + 2) * sizeof(token_t));
				i += 1 + assembler_store_token(&data_label_identifiers[data_label_count], current + 1);
				if (assembler_extra_checks && assembler_token_index(data_label_count, data_label_identifiers, &data_label_identifiers[data_label_count]) < data_label_count) printf("[ZASM Language] WARNING Line %ld, data label %s declared multiple times\n", current_line_number, data_label_identifiers[data_label_count].data);
				data_label_identifiers[data_label_count].data_label_array = (uint8_t*) malloc(1);
				current_data_label = &data_label_identifiers[data_label_count++];
			}
		}
	}
	
	in_comment = 0;
	in_string = 0;
	
	uint8_t in_data_section = 0;
	uint8_t found_main_label = 0;
	
	logic_section_words = 0;
	logic_section_data = (uint64_t*) 0;
	
	if (assembler_verbose) printf("[ZASM Language] Reading assembly and building logic section ...\n");
	current_line_number = 1;
	for (uint64_t i = 0; i < code_bytes; i++) {
		char* current = code + i;
		
		if (*current == '\n') {
			current_line_number++;
			in_comment = 0;
			
		} else if (!in_comment && *current == '"') {
			in_string = !in_string;
			
		} else if (!in_string && *current == '#') {
			in_comment = !in_comment;
			
		} else if (!in_comment && !in_string) {
			if (in_data_section) { // stall logic section reading if inside data section
				if (*current == DATA_LABEL_TOKEN) { // found end of data label
					in_data_section = 0;
				}
				
			} else if (*current == DATA_LABEL_TOKEN) { // found data label
				rom_validate_instruction();
				in_data_section = 1;
				
			} else if (*current == POSITION_LABEL_TOKEN) { // found position label
				rom_validate_instruction();

				token_t identifier;
				i += 2 + assembler_store_token(&identifier, current + 1);
				
				if (strcmp(identifier.data, "main") == 0) {
					found_main_label = 1;
					position_label_identifiers[0].position = logic_section_words;

				} else {
					position_label_identifiers[assembler_token_index(position_label_count, position_label_identifiers, &identifier)].position = logic_section_words;
				}
				
			} else if (!IS_WHITE(*current)) {
				token_t token;
				i += 1 + assembler_store_token(&token, current);
				int64_t instruction, _register, position_ptr, data_ptr, kfunc_ptr;

				// go though all instructions, registers, &c to try and find a match

				if      ((instruction  = assembler_token_index(sizeof(assembler_instructions) / sizeof(*assembler_instructions), assembler_instructions, &token)) >= 0) rom_add_token(TOKEN_TYPE_INSTRUCTION, instruction);
				else if ((_register    = assembler_token_index(sizeof(assembler_registers   ) / sizeof(*assembler_registers   ), assembler_registers,    &token)) >= 0) rom_add_token(ZED_OPERAND_16_TYPE_REGISTER, _register);
				else if ((position_ptr = assembler_token_index(position_label_count, position_label_identifiers,                                         &token)) >= 0) rom_add_token(ZED_OPERAND_16_TYPE_POSITION_INDEX, position_ptr);
				else if ((data_ptr     = assembler_token_index(data_label_count, data_label_identifiers,                                                 &token)) >= 0) rom_add_token(ZED_OPERAND_16_TYPE_DATA_INDEX, data_ptr);
				else if ((kfunc_ptr    = assembler_token_index(sizeof(assembler_kfuncs      ) / sizeof(*assembler_kfuncs      ), assembler_kfuncs,       &token)) >= 0) rom_add_token(ZED_OPERAND_16_TYPE_KFUNC_INDEX, kfunc_ptr);
				
				else if (token.data[1] == '?' || token.data[0] == '?') {
					char* string = (char*) 0;
					uint8_t token_type = ZED_OPERAND_16_TYPE_ADDRESS_64;
					
					if (token.data[0] == '8' || token.data[0] == '?') { // 64 bit addressing
						token_type = ZED_OPERAND_16_TYPE_ADDRESS_64;
						string = token.data[0] == '8' ? token.data + 2 : token.data + 1;
						
					} else if (token.data[0] == '1') { // 8 bit addressing
						token_type = ZED_OPERAND_16_TYPE_ADDRESS_8;
						string = token.data + 2;
						
					} else {
						printf("[ZASM Language] WARNING Line %ld, unknown addressing type '%c'\n", current_line_number, *current);
					}
					
					if (string) { // create the cla instruction chain (or not if is a single register)
						if ((_register = assembler_token_index_with_string(sizeof(assembler_registers) / sizeof(*assembler_registers), assembler_registers, string)) >= 0) rom_add_token(token_type, _register);
						else printf("[ZASM Language] WARNING Line %ld, address '%s' is not a register\n", current_line_number, string);
					}
					
				} else { // test if maybe this is a number literal
					int64_t number = 0;
					
					if (assembler_token_to_number(&token, &number)) printf("[ZASM Language] WARNING Line %ld, unknown token or identifier '%s'\n", current_line_number, token.data);
					else rom_add_token(ZED_OPERAND_16_TYPE_CONSTANT, number);
				}
			}
		}
	}
	
	if (!found_main_label) {
		printf("[ZASM Language] WARNING Couldn't find the main position label\n");
	}

	rom_validate_instruction(); // make sure we validate the last instruction
	return 0;
}

static int build_rom(void) {
	// meta section

	zed_meta_section_t meta_section = {
		.magic = ZED_MAGIC,
		.version = ZED_VERSION,

		.data_section_element_count = data_label_count,
		.position_section_element_count = position_label_count,
	};

	rom_bytes = sizeof(meta_section);
	rom_data = (uint8_t*) malloc(rom_bytes);

	// data section
	
	zed_data_section_element_t* data_section_elements = (zed_data_section_element_t*) malloc(data_label_count * sizeof(zed_data_section_element_t));

	for (uint64_t i = 0; i < data_label_count; i++) { // write each data element and record a pointer to it
		data_section_elements[i].bytes = data_label_identifiers[i].data_label_bytes;
		data_section_elements[i].data_offset = rom_bytes;

		rom_data = (uint8_t*) realloc(rom_data, rom_bytes + data_label_identifiers[i].data_label_bytes);
		memcpy(rom_data + rom_bytes, data_label_identifiers[i].data_label_array, data_label_identifiers[i].data_label_bytes);
		rom_bytes += data_label_identifiers[i].data_label_bytes;
	}

	rom_bytes = ((rom_bytes - 1) / sizeof(zed_data_section_element_t) + 1) * sizeof(zed_data_section_element_t); // snap to nearest 128 bit boundary (sizeof(zed_data_section_element_t))
	meta_section.data_section_offset = rom_bytes;

	for (uint64_t i = 0; i < data_label_count; i++) {
		rom_data = (uint8_t*) realloc(rom_data, rom_bytes + sizeof(zed_data_section_element_t));
		memcpy(rom_data + rom_bytes, &data_section_elements[i], sizeof(zed_data_section_element_t));
		rom_bytes += sizeof(zed_data_section_element_t);
	}

	// position section

	meta_section.position_section_offset = rom_bytes;

	for (uint64_t i = 0; i < position_label_count; i++) {
		rom_data = (uint8_t*) realloc(rom_data, rom_bytes + sizeof(uint64_t));
		*((uint64_t*) (rom_data + rom_bytes)) = position_label_identifiers[i].position;
		rom_bytes += sizeof(uint64_t);
	}

	// write logic section
	
	meta_section.logic_section_offset = rom_bytes;
	meta_section.logic_section_words = logic_section_words;

	uint64_t logic_section_bytes = logic_section_words * sizeof(uint64_t);

	rom_data = (uint8_t*) realloc(rom_data, rom_bytes + logic_section_bytes);
	memcpy(rom_data + rom_bytes, logic_section_data, logic_section_bytes);
	
	rom_bytes += logic_section_bytes;
	memcpy(rom_data, &meta_section, sizeof(meta_section)); // write the meta section to the rom

	return 0;
}

static void assembler_free(void) {
	if (input) fclose(input);
	if (output) fclose(output);
	
	if (code) free(code);
	if (rom_data) free(rom_data);
	
	for (uint64_t i = 0; i < data_label_count; i++) if (data_label_identifiers[i].data_label_array) free(data_label_identifiers[i].data_label_array);
	
	if (position_label_identifiers) free(position_label_identifiers);
	if (data_label_identifiers) free(data_label_identifiers);
	
	if (logic_section_data) free(logic_section_data);
}

int main(int argc, char** argv) {
	printf("[ZASM Language] Compiling with ZASM 1.0.0 (stable) ...\n");
	
	printf("[ZASM Language] Parsing arguments ...\n");
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--", 2) == 0) { // argument is option
			char* option = argv[i] + 2;

			if (strcmp(option, "input") == 0) input_path = argv[++i];
			else if (strcmp(option, "output") == 0) output_path = argv[++i];

			else if (strcmp(option, "verbose") == 0) assembler_verbose = 1;
			else if (strcmp(option, "suppress") == 0) assembler_warnings = 0;
			else if (strcmp(option, "no-checks") == 0) assembler_extra_checks = 0;

			else if (strcmp(option, "debug") == 0) printf("[ZASM Language] TODO '--debug' option\n");

			else {
				fprintf(stderr, "[ZASM Language] ERROR Option '--%s' is unknown. TODO Replace the following: ~Run with '--help' to see a list of available options~\n", option);
				return 1;
			}
			
		} else {
			fprintf(stderr, "[ZASM Language] ERROR Unexpected argument '%s'\n", argv[i]);
			return 1;
		}
	}

	if (sizeof(zed_instruction_t) != sizeof(uint64_t)) {
		fprintf(stderr, "[ZASM Language] ERROR ZED instruction structure isn't 64 bits wide\n");
		return 1;
	}

	printf("[ZASM Language] Opening input file ...\n");

	input = fopen(input_path, "rb");
	if (!input) {
		fprintf(stderr, "[ZASM Language] ERROR Couldn't open '%s' for reading\n", input_path);
		
		assembler_free();
		return 1;
	}
	
	fseek(input, 0, SEEK_END);
	code_bytes = ftell(input);
	if (assembler_verbose) printf("[ZASM Language] ASM File is %ld bytes long\n", code_bytes);
	rewind(input);
	
	code = (char*) malloc(code_bytes);
	fread(code, sizeof(char), code_bytes, input);
	
	printf("[ZASM Language] Assembling code ...\n");
	if (assemble()) {
		fprintf(stderr, "[ZASM Language] ERROR Failed to assemble\n");

		assembler_free();
		return 1;
	}
	
	printf("[ZASM Language] Building ROM ...\n");
	if (build_rom()) {
		fprintf(stderr, "[ZASM Language] ERROR Failed to build ROM\n");

		assembler_free();
		return 1;
	}

	printf("[ZASM Language] Outputting ROM ...\n");

	char* rom_path = (char*) malloc(strlen(output_path) + 9 /* strlen("/rom.zed") + 1 */); // no risk of memory leak
	sprintf(rom_path, "%s/rom.zed", output_path);
	
	output = fopen(rom_path, "wb");
	if (!output) {
		fprintf(stderr, "[ZASM Language] ERROR Couldn't open '%s' for writing\n", rom_path);

		assembler_free();
		return 1;
	}
	
	if (assembler_verbose) printf("[ZASM Language] Writing ROM (%ld bytes) to output path (%s) ...\n", rom_bytes, output_path);
	if (rom_data) fwrite(rom_data, sizeof(uint8_t), rom_bytes, output);
	if (assembler_verbose) printf("[ZASM Language] Assembler finished with success\n");
	
	assembler_free();
	printf("[ZASM Language] Done\n");

	return 0;
}
