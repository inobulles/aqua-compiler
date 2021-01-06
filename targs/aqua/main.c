#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <iar.h>

#define DEFAULT_PATH "."
#define DEFAULT_NAME "generic-package"

int main(int argc, char** argv) {
	char* input_path = DEFAULT_PATH;
	char* output_path = (char*) 0;

	char* name = DEFAULT_NAME;

	uint8_t debug = 0;

	printf("[AQUA Target] Parsing arguments ...\n");
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--", 2) == 0) {
			char* option = argv[i] + 2;

			if (strcmp(option, "input") == 0) input_path = argv[++i];
			else if (strcmp(option, "output") == 0) output_path = argv[++i];

			else if (strcmp(option, "name") == 0) name = argv[++i];

			else if (strcmp(option, "debug") == 0) debug = 1;

			else {
				fprintf(stderr, "[AQUA Target] ERROR Option '--%s' is unknown\n", option);
				return 1;
			}

		} else {
			fprintf(stderr, "[AQUA Target] ERROR Unexpected argument '%s'\n", argv[i]);
			return 1;
		}
	}

	// handle input and output paths
	
	if (!output_path) {
		printf("[AQUA Target] Output path not specified, setting it to '%s' ...\n", input_path);
		output_path = input_path;
	}

	printf("[AQUA Target] Creating output directory ...\n");
	if (mkdir(output_path, 0700) < 0 && errno != EEXIST) { // this should really be handled by the compiler, but this is just in case
		fprintf(stderr, "[AQUA Target] ERROR Failed to create output directory at '%s'\n", output_path);
		return 1;
	}

	// create and pack input to iar file

	char* zpk_path = (char*) malloc(strlen(output_path) + 9 /* strlen("/out.zpk") + 1 */); // no risk of memory leak
	sprintf(zpk_path, "%s/out.zpk", output_path);

	iar_file_t iar;
	if (iar_open_write(&iar, zpk_path)) {
		fprintf(stderr, "[AQUA Target] Failed to create ZPK (IAR) file at '%s'\n", zpk_path);
		return 1;
	}

	// iar.header.page_bytes should be left as default (IAR_DEFAULT_PAGE_BYTES)
	// it may be interesting to add an option for "compact" ZPK files, where the page alignment is set to 1
	// this should definitely come with a warning as platforms don't need to guarantee anything other than a 4096 page alignment to conform with the standard

	if (iar_pack(&iar, input_path, name)) {
		fprintf(stderr, "[AQUA Target] Failed to pack ZPK (IAR) file from input at '%s'\n", input_path);
		
		iar_close(&iar);
		return 1;
	}

	iar_write_header(&iar);
	iar_close(&iar);

	printf("[AQUA Target] Done\n");
	return 0;
}