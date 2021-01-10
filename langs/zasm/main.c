// pretty horrendous code, I know (although it used to be worse), I need to rewrite this some day

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <compiler_common.h>

#define DATA_LABEL_TOKEN '%'
#define POSITION_LABEL_TOKEN ':'

#define IS_WHITE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == DATA_LABEL_TOKEN || (x) == POSITION_LABEL_TOKEN || (x) == ',' || (x) == ';' || (x) == '#' || (x) == '\r' || (x) == '\0')

static FILE* input = (FILE*) 0;
static FILE* output = (FILE*) 0;

static char* code = (char*) 0;
static uint64_t code_bytes = 0;

static uint8_t assembler_extra_checks = 1;

static char* input_path = "main.zasm";
static char* output_path = ".";

static uint64_t current_line_number;

// assembler helper functions (previously ungodly)

static inline int64_t assembler_name_token(zed_token_t* self, char* string) {
	uint64_t length = 0;
	for (; !IS_WHITE(*string); length++, string++); // get name length

	self->name = (char*) malloc(length + 1);

	memcpy(self->name, string - length, length);
	self->name[length] = 0; // null terminate
	
	return length - 1;
}

static inline int64_t assembler_add_token(zed_token_t** list, uint64_t* count, char* string) {
	*list = (zed_token_t*) realloc(*list, (*count + 1) * sizeof(zed_token_t));
	return assembler_name_token(&(*list)[(*count)++], string); // *lol*
}

static inline uint8_t assembler_token_to_number(zed_token_t* self, int64_t* value_reference) { // returns 0 on success
	char* endptr = (char*) 0;
	
	if      (*self->name == 'x') *value_reference = strtoll(self->name + 1, &endptr, 16); // hexadecimal
	else if (*self->name == 'b') *value_reference = strtoll(self->name + 1, &endptr, 2);  // binary
	else                         *value_reference = strtoll(self->name,     &endptr, 10); // decimal
	
	return endptr == self->name || strlen(endptr);
}

// actual nice, juicy, meaty functions

static int assemble(void) {
	// first pass (get position and data labels)

	zed_token_t* current_data_label = (zed_token_t*) 0;

	uint8_t in_comment = 0;
	uint8_t in_string = 0;
	
	current_line_number = 1;
	uint64_t last_line_number_for_string_outside_data_label_warning = -1;

	for (char* current = code; current - code < code_bytes; current++) {
		// the order of all these if's is very important

		if      (!in_string  && *current == '\n') current_line_number++, in_comment = 0;
		else if (!in_comment && *current == '"' ) in_string  = !in_string;
		else if (!in_string  && *current == '#' ) in_comment = !in_comment;
		
		else if (!in_comment) {
			if (current_data_label) {
				if (in_string) {
					zed_token_add_data(current_data_label, *current);
					if (*current == '\n') current_line_number++, in_comment = 0; // this is necessary

				} else if (*current == DATA_LABEL_TOKEN) { // found end of data label
					current_data_label = (zed_token_t*) 0;
					
				} else if (!IS_WHITE(*current)) {
					zed_token_t dummy_token = { 0 };
					current += assembler_name_token(&dummy_token, current);

					int64_t byte = 0;
					if (assembler_token_to_number(&dummy_token, &byte)) {
						printf("[ZASM Language] WARNING Line %ld, found unknown token '%s' in data label '%s'\n", current_line_number, dummy_token.name, current_data_label->name);
					}

					zed_token_free(&dummy_token); // important because memory leak otherwise

					if (byte > 0xFF) {
						printf("[ZASM Language] WARNING Line %ld, value '0x%lx' does not fit in a byte, cropping to least significant byte (0x%lx) ...\n", current_line_number, byte, byte % 0x100);
						byte %= 0x100;
					}

					zed_token_add_data(current_data_label, (uint8_t) byte);
				}
				
			} else if (!in_string && *current == POSITION_LABEL_TOKEN) { // found position label
				current += 2 + assembler_add_token(&position_labels, &position_label_count, current + 1);

				if (assembler_extra_checks && zed_token_find(position_labels, position_label_count - 1, position_labels[position_label_count - 1].name) >= 0) {
					printf("[ZASM Language] WARNING Line %ld, position label '%s' declared multiple times\n", current_line_number, position_labels[position_label_count - 1].name);
				}

				if (*current != POSITION_LABEL_TOKEN) {
					printf("[ZASM Language] WARNING Line %ld, unterminated position label '%s'\n", current_line_number, position_labels[position_label_count - 1].name);
				}

			} else if (!in_string && *current == DATA_LABEL_TOKEN) { // found data label
				current += 1 + assembler_add_token(&data_labels, &data_label_count, current + 1);

				if (assembler_extra_checks && zed_token_find(data_labels, data_label_count - 1, data_labels[data_label_count - 1].name) >= 0) {
					printf("[ZASM Language] WARNING Line %ld, data label '%s' declared multiple times\n", current_line_number, data_labels[data_label_count - 1].name);
				}

				if (*current != DATA_LABEL_TOKEN) {
					current_data_label = &data_labels[data_label_count - 1];
				}

			} else if (in_string) { // illegal state
				if (*current == '\n') { // even criminals need to keep track of their lines 😎
					current_line_number++;
					in_comment = 0;
				}

				if (current_line_number != last_line_number_for_string_outside_data_label_warning) { // issue warning only once per line
					printf("[ZASM Language] WARNING Line %ld, string outside of data label\n", current_line_number);
					last_line_number_for_string_outside_data_label_warning = current_line_number;
				}
			}
		}
	}

	if (in_string) printf("[ZASM Language] WARNING Unterminated string\n");
	if (current_data_label) printf("[ZASM Language] WARNING Unterminated data label '%s'\n", current_data_label->name);

	// second pass (parse tokens and create logic section)
	
	in_comment = 0;
	in_string = 0;
	
	uint8_t in_data_section = 0;
	uint8_t found_main_label = 0;
	
	current_line_number = 1;

	for (char* current = code; current - code < code_bytes; current++) {
		// the order of all these if's is very important
		
		if      (               *current == '\n') current_line_number++, in_comment = 0;
		else if (!in_comment && *current == '"' ) in_string  = !in_string;
		else if (!in_string  && *current == '#' ) in_comment = !in_comment;
		
		else if (!in_comment && !in_string) {
			if (in_data_section) { // stall logic section reading if inside data section
				if (*current == DATA_LABEL_TOKEN) { // found end of data label
					in_data_section = 0;
				}
				
			} else if (*current == DATA_LABEL_TOKEN) { // found data label
				rom_validate_instruction();
				in_data_section = 1;
				
			} else if (*current == POSITION_LABEL_TOKEN) { // found position label
				rom_validate_instruction();

				zed_token_t dummy_token = { 0 };
				current += 2 + assembler_name_token(&dummy_token, current + 1);

				if (strcmp(dummy_token.name, "main") == 0) {
					found_main_label = 1;
					position_labels[0].position = logic_section_words;

				} else {
					position_labels[zed_token_find(position_labels, position_label_count, dummy_token.name)].position = logic_section_words;
				}

				zed_token_free(&dummy_token);
				
			} else if (!IS_WHITE(*current)) {
				int64_t opcode, _register, position_ptr, data_ptr, kfunc_ptr;

				zed_token_t token = { 0 };
				current += 1 + assembler_name_token(&token, current);

				// go though all opcodes, registers, &c to try and find a match

				if      ((opcode       = zed_token_find(zed_opcodes,     sizeof(zed_opcodes  ) / sizeof(*zed_opcodes  ), token.name)) >= 0) rom_add(ZED_TYPE_OPCODE, opcode);
				else if ((_register    = zed_token_find(zed_registers,   sizeof(zed_registers) / sizeof(*zed_registers), token.name)) >= 0) rom_add(ZED_OPERAND_16_TYPE_REGISTER, _register);
				else if ((position_ptr = zed_token_find(position_labels, position_label_count,                           token.name)) >= 0) rom_add(ZED_OPERAND_16_TYPE_POSITION_INDEX, position_ptr);
				else if ((data_ptr     = zed_token_find(data_labels,     data_label_count,                               token.name)) >= 0) rom_add(ZED_OPERAND_16_TYPE_DATA_INDEX, data_ptr);
				else if ((kfunc_ptr    = zed_token_find(zed_kfuncs,      sizeof(zed_kfuncs   ) / sizeof(*zed_kfuncs   ), token.name)) >= 0) rom_add(ZED_OPERAND_16_TYPE_KFUNC_INDEX, kfunc_ptr);
				
				else if (token.name[1] == '?' || token.name[0] == '?') {
					char* string = (char*) 0;
					uint8_t token_type = ZED_OPERAND_16_TYPE_ADDRESS_64;
					
					if (token.name[0] == '8' || token.name[0] == '?') { // 64 bit addressing
						token_type = ZED_OPERAND_16_TYPE_ADDRESS_64;
						string = token.name[0] == '8' ? token.name + 2 : token.name + 1;
						
					} else if (token.name[0] == '1') { // 8 bit addressing
						token_type = ZED_OPERAND_16_TYPE_ADDRESS_8;
						string = token.name + 2;
						
					} else {
						printf("[ZASM Language] WARNING Line %ld, unknown addressing type '%c'\n", current_line_number, *current);
					}
					
					if (string) { // create the cla instruction chain (or not if is a single register)
						if ((_register = zed_token_find(zed_registers, sizeof(zed_registers) / sizeof(*zed_registers), string)) >= 0) {
							rom_add(token_type, _register);

						} else {
							printf("[ZASM Language] WARNING Line %ld, address '%s' is not a register\n", current_line_number, string);
						}
					}
					
				} else { // test if maybe this is a number literal
					int64_t number = 0;
					
					if (assembler_token_to_number(&token, &number)) {
						printf("[ZASM Language] WARNING Line %ld, unknown token or identifier '%s'\n", current_line_number, token.name);

					} else {
						rom_add(ZED_OPERAND_16_TYPE_CONSTANT, number);
					}
				}

				zed_token_free(&token);
			}
		}
	}
	
	if (!found_main_label) {
		printf("[ZASM Language] WARNING Couldn't find the main position label\n");
	}

	rom_validate_instruction(); // make sure we validate the last instruction
	return 0;
}

static void assembler_free(void) {
	if (input) fclose(input);
	if (output) fclose(output);
}

int main(int argc, char** argv) {
	printf("[ZASM Language] Compiling with ZASM 1.0.0 (stable) ...\n");
	
	printf("[ZASM Language] Parsing arguments ...\n");
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--", 2) == 0) { // argument is option
			char* option = argv[i] + 2;

			if (strcmp(option, "input") == 0) input_path = argv[++i];
			else if (strcmp(option, "output") == 0) output_path = argv[++i];

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
	rewind(input);
	
	code = (char*) malloc(code_bytes);
	fread(code, sizeof(char), code_bytes, input);
	
	printf("[ZASM Language] Creating ROM ...\n");
	rom_create();

	printf("[ZASM Language] Assembling code ...\n");
	if (assemble()) {
		fprintf(stderr, "[ZASM Language] ERROR Failed to assemble\n");

		assembler_free();
		return 1;
	}
	
	printf("[ZASM Language] Building ROM ...\n");
	rom_build();

	printf("[ZASM Language] Outputting ROM ...\n");

	char* rom_path = (char*) malloc(strlen(output_path) + 9 /* strlen("/rom.zed") + 1 */); // no risk of memory leak
	sprintf(rom_path, "%s/rom.zed", output_path);
	
	output = fopen(rom_path, "wb");
	if (!output) {
		fprintf(stderr, "[ZASM Language] ERROR Couldn't open '%s' for writing\n", rom_path);

		assembler_free();
		return 1;
	}
	
	if (rom_data) {
		fwrite(rom_data, sizeof(uint8_t), rom_bytes, output);
	}
	
	assembler_free();
	printf("[ZASM Language] Done\n");

	return 0;
}
