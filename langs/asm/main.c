
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

static FILE* input = (FILE*) 0;
static FILE* output = (FILE*) 0;

static char* code = (char*) 0;
static uint64_t code_bytes = 0;

static uint8_t assembler_verbose = 0;
static uint8_t assembler_warnings = 1;

static char* input_path = "code.asm";
static char* output_path = "rom.zed";

static char* rom_data = (char*) 0;
static char rom_bytes = 0;

static void assembler_free(void) {
	if (input) fclose(input);
	if (output) fclose(output);
	
	if (code) free(code);
	if (rom_data) free(rom_data);
	
}

#define DATA_LABEL_TOKEN '%'
#define RES_POS_LABEL_TOKEN ':'

#define IS_WHITE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == RES_POS_LABEL_TOKEN || (x) == DATA_LABEL_TOKEN || (x) == ',' || (x) == ';' || (x) == '#' || (x) == '\r' || (x) == '\0')
#define MAX_TOKEN_LENGTH 63

typedef struct {
	char data[MAX_TOKEN_LENGTH + 1];
	uint8_t bytes;
	
	uint64_t data_label_bytes;
	uint8_t* data_label_array;
	
} token_t;

static uint64_t current_line_number;

static inline int assembler_compare_token(char* string, char* comparator) {
	while (!IS_WHITE(*string)) if (*string++ != *comparator++) break;
	return *((const uint8_t*) string) - *((const uint8_t*) comparator);
	
} static inline int assembler_store_token(token_t* self, char* string) {
	memset(self, 0, sizeof(*self));
	uint8_t warn_too_big = 0;
	for (; !(self->bytes >= MAX_TOKEN_LENGTH && (warn_too_big = 1) /* look at this beauty */) && !IS_WHITE(*string); self->bytes++) self->data[self->bytes] = *string++;
	if (warn_too_big && assembler_warnings) printf("WARNING Line %ld, token %s has surpassed the maximum length (%d)\n", current_line_number, self->data, MAX_TOKEN_LENGTH);
	return self->bytes - 1;
	
}

static int assemble(void) {
	token_t* res_pos_label_identifiers = (token_t*) malloc(sizeof(token_t));
	token_t* data_label_identifiers = (token_t*) malloc(sizeof(token_t));
	
	uint64_t res_pos_label_count = 0;
	uint64_t data_label_count = 0;
	
	token_t* current_data_label = (token_t*) 0;
	uint8_t in_comment = 0;
	
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
					
					uint64_t byte = 0;
					char* endptr = (char*) 0;
					
					if      (*token.data == 'x') byte = strtoll(token.data + 1, &endptr, 16); // hexadecimal
					else if (*token.data == 'b') byte = strtoll(token.data + 1, &endptr, 2);  // binary
					else                         byte = strtoll(token.data,     &endptr, 10); // decimal
					
					if (endptr == token.data || strlen(endptr)) {
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
				i += 1 + assembler_store_token(&res_pos_label_identifiers[res_pos_label_count++], current + 1);
				
			} else if (*current == DATA_LABEL_TOKEN) { // found data label
				data_label_identifiers = (token_t*) realloc(data_label_identifiers, (data_label_count + 2) * sizeof(token_t));
				i += 1 + assembler_store_token(&data_label_identifiers[data_label_count], current + 1);
				data_label_identifiers[data_label_count].data_label_array = (uint8_t*) malloc(1);
				current_data_label = &data_label_identifiers[data_label_count++];
				
			}
			
		}
		
	}
	
	for (uint64_t i = 0; i < data_label_count; i++) {
		free(data_label_identifiers[i].data_label_array);
		
	}
	
	free(res_pos_label_identifiers);
	free(data_label_identifiers);
	
	return 0;
	
}

static int build_rom(void) {
	fprintf(stderr, "TODO %s\n", __func__);
	return 1;
	
}

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "verbose") == 0) {
			assembler_verbose = 1;
			printf("Verbose flag set\n");
			
		} else if (strcmp(argv[i], "suppress") == 0) {
			assembler_warnings = 0;
			if (assembler_verbose) printf("Disabled warnings\n");
			
		} else if (strcmp(argv[i], "in") == 0) {
			input_path = argv[++i];
			if (assembler_verbose) printf("Set input path to %s\n", input_path);
			
		} else if (strcmp(argv[i], "out") == 0) {
			output_path = argv[++i];
			if (assembler_verbose) printf("Set output path to %s\n", output_path);
			
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
	
	fwrite(rom_data, sizeof(char), rom_bytes, output);
	if (assembler_verbose) printf("Assembler finished with success\n");
	
	assembler_free();
	return 0;
	
}
