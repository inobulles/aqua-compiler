
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static FILE* input = (FILE*) 0;
static FILE* output = (FILE*) 0;

static char* code = (char*) 0;
static uint64_t code_bytes = 0;

static uint8_t assembler_verbose = 0;
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

#define IS_WHITE(x) ((x) == ' ' || (x) == '\t' || (x) == '\n' || (x) == ',' || (x) == ';' || (x) == '#' || (x) == '\r' || (x) == '\0')
#define MAX_IDENTIFIER_LENGTH 64

typedef struct {
	char data[MAX_IDENTIFIER_LENGTH];
	uint8_t bytes;
	
} identifier_t;

static int assembler_compare_token(char* string, char* comparator) {
	while (!IS_WHITE(*string)) if (*string++ != *comparator++) break;
	return *((const uint8_t*) string) - *((const uint8_t*) comparator);
	
} static void assembler_store_identifier(identifier_t* self, char* string) {
	memset(self, 0, sizeof(*self));
	for (; self->bytes < MAX_IDENTIFIER_LENGTH - 1 && !IS_WHITE(*string); self->bytes++) self->data[self->bytes] = *string++;
	
}

static int assemble(void) {
	uint64_t line_number = 1;
	uint8_t in_comment = 0;
	
	for (uint64_t i = 0; i < code_bytes; i++) {
		char* current = code + i;
		
		if (*current == '\n') {
			line_number++;
			in_comment = 0;
			
		} else if (*current == '#') {
			in_comment = !in_comment;
			
		} else if (!in_comment && (*current == ':' || *current == '-')) { // found label
			identifier_t identifier;
			assembler_store_identifier(&identifier, current + 1);
			printf("%s %d\n", identifier.data, identifier.bytes);
			i += identifier.bytes;
			
		}
		
	}
	
	return 0;
	
}

static int build_rom(void) {
	fprintf(stderr, "TODO %s\n", __func__);
	return 1;
	
}

int main(int argc, char* argv[]) {
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "verbose") == 0) {
			printf("Verbose flag set\n");
			assembler_verbose = 1;
			
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
