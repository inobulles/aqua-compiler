#include <stdio.h>

#define COMPILER_MNEMONICS
#include <compiler_common.h>

int main(int argc, char* argv[]) {
	printf("[TEST] Compiling with test language ...\n");

	char* input_path; // not used
	char* output_path;

	printf("[TEST] Parsing arguments ...\n");
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--", 2) == 0) {
			char* option = argv[i] + 2;

			if (strcmp(option, "input") == 0) input_path = argv[++i];
			if (strcmp(option, "output") == 0) output_path = argv[++i];
			
			else {
				fprintf(stderr, "[TEST] ERROR Option '--%s' is unknown\n", option);
				return 1;
			}

		} else {
			fprintf(stderr, "[TEST] ERROR Unexpected argument '%s'\n", argv[i]);
			return 1;
		}
	}

	// create rom

	printf("[TEST] Creating ROM ...\n");
	zed_rom_create();

	// assembler

	DATA_LABEL(hello) {
		DATA_LABEL_SET_STR("Hello world!\n");
	}

	POS_LABEL(return) {
		MOV REG(A0), CONST16(0xDEAD);
		CAL REG(G0), KFUNC(EXIT);
	}

	MAIN {
		MOV REG(A0), _DATA(hello);
		CAL REG(G0), KFUNC(PRINT);

		JMP POS(return);
		
		MOV REG(G0), CONST16(1337);
	}

	// build rom & output

	printf("[TEST] Building ROM ...\n");
	zed_rom_build();

	printf("[TEST] Outputting ROM ...\n");

	char* rom_path = (char*) malloc(strlen(output_path) + 9 /* strlen("/rom.zed") + 1 */); // no risk of memory leak :sweat_drops::sweat_drops::sweat_drops:
	sprintf(rom_path, "%s/rom.zed", output_path);
	
	FILE* output = fopen(rom_path, "wb");
	if (!output) {
		fprintf(stderr, "[TEST] ERROR Couldn't open '%s' for writing\n", rom_path);
		return 1;
	}
	
	if (zed_rom_data) {
		fwrite(zed_rom_data, sizeof(uint8_t), zed_rom_bytes, output);
	}
	
	printf("[TEST] Done\n");

	return 0;
}