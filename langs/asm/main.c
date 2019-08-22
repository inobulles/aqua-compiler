
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

static FILE* fp = (FILE*) 0;
static char* code = (char*) 0;

static uint8_t assembler_verbose = 0;
static char* input_path = "code.asm";
static char* output_path = "rom.zed";

static void assembler_free(void) {
	if (fp) fclose(fp);
	if (code) free(code);
	
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
	
	fp = fopen(input_path, "rb");
	if (!fp) {
		fprintf(stderr, "ERROR Could not load main.asm file\n");
		assembler_free();
		return 1;
		
	}
	
	fseek(fp, 0, SEEK_END);
	uint64_t file_bytes = ftell(fp);
	if (assembler_verbose) printf("ASM File is %ld bytes long\n", file_bytes);
	rewind(fp);
	
	code = (char*) malloc(file_bytes);
	fread(code, sizeof(char), file_bytes, fp);
	
	//~ printf("%s\n", code);
	
	assembler_free();
	return 0;
	
}
