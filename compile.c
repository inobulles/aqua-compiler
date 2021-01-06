
// TODO create aqua target which creates .ZPK file (so remove the DEFAULT_TARGET stuff)

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/wait.h>

#include <compiler_common.h> // useless include

#if !defined(COMPILER_DIR_PATH)
	#error "COMPILER_DIR_PATH was not supplied"
#endif

#define DEFAULT_PATH "."

#define DEFAULT_LANGUAGE "amber"
#define DEFAULT_TARGET "aqua"

int main(int argc, char** argv) {
	char* input_path = DEFAULT_PATH;
	char* output_path = (char*) 0;

	char* lang = DEFAULT_LANGUAGE;
	char* targ = DEFAULT_TARGET;

	uint8_t debug = 0;

	printf("[AQUA Compiler] Parsing arguments ...\n");
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--", 2) == 0) {
			char* option = argv[i] + 2;

			if (strcmp(option, "input") == 0) input_path = argv[++i];
			else if (strcmp(option, "output") == 0) output_path = argv[++i];

			else if (strcmp(option, "lang") == 0) lang = argv[++i];
			else if (strcmp(option, "targ") == 0) targ = argv[++i];

			else if (strcmp(option, "debug") == 0) debug = 1;

			else {
				fprintf(stderr, "[AQUA Compiler] ERROR Option '--%s' is unknown\n", option);
				return 1;
			}

		} else {
			fprintf(stderr, "[AQUA Compiler] ERROR Unexpected argument '%s'\n", argv[i]);
			return 1;
		}
	}

	uint8_t none_targ = strcmp(targ, "none") == 0;

	// handle input and output paths

	if (!output_path) {
		printf("[AQUA Compiler] Output path not specified, setting it to '%s/out/' ...\n", input_path);
		
		output_path = (char*) malloc(strlen(input_path) + 6 /* strlen("/out/") + 1 */); // no risk of memory leak
		sprintf(output_path, "%s/out/", input_path);
	}

	char* real_input_path = realpath(input_path, (char*) 0); // no risk of memory leak

	// it's necessary to chdir to the input path since the output path could possibly not yet exist
	// that's also why the realpath(output_path, ...) call is after the creation of output_path; realpath works only on pre-existing paths

	printf("[AQUA Compiler] Creating output directory ...\n");
	if (mkdir(output_path, 0700) < 0 && errno != EEXIST) {
		fprintf(stderr, "[AQUA Compiler] ERROR Failed to create output directory at '%s'\n", output_path);
		return 1;
	}

	char* real_output_path = realpath(output_path, (char*) 0); // no risk of memory leak

	printf("[AQUA Compiler] Changing directory to input path ...\n");
	if (chdir(real_input_path) < 0) {
		fprintf(stderr, "[AQUA Compiler] Input path '%s' doesn't seem to exist\n", input_path);
		return 1;
	}

	// handle the intermediate path

	char* intermediate_path;

	if (none_targ) {
		printf("[AQUA Compiler] Target is none, intermediate output path can be set to output\n");
		intermediate_path = real_output_path;

	} else {
		printf("[AQUA Compiler] Creating intermediate output directory ...\n");
		
		intermediate_path = (char*) malloc(strlen(real_output_path) + 15 /* strlen("intermediate/") + 1 */); // no risk of memory leak
		sprintf(intermediate_path, "%s/intermediate/", real_output_path);

		if (mkdir(intermediate_path, 0700) < 0 && errno != EEXIST) {
			fprintf(stderr, "[AQUA Compiler] ERROR Failed to create intermediate output directory at '%s'\n", intermediate_path);
			return 1;
		}
	}

	printf("[AQUA Compiler] Changing directory to intermediate output path ...\n");
	chdir(intermediate_path);

	printf("[AQUA Compiler] Generating start node ...\n");
	FILE* start_fp = fopen("start", "wb");
	fprintf(start_fp, "zed");
	fclose(start_fp);

	FILE* feature_set_fp = fopen("feature_set", "wb");
	fprintf(feature_set_fp, "");
	fclose(feature_set_fp);

	printf("[AQUA Compiler] Changing directory back to input path ...\n");
	chdir(real_input_path);

	// run the lang and targ processes

	printf("[AQUA Compiler] Running lang process ...\n");

	if (!fork()) { // child process
		char* lang_path = (char*) malloc(strlen(COMPILER_DIR_PATH) + strlen(lang) + 8 /* strlen("/langs/") + 1 */); // no risk of memory leak
		sprintf(lang_path, "%s/langs/%s", COMPILER_DIR_PATH, lang);

		execl(lang_path, debug ? "--debug" : "", "--output", intermediate_path, NULL);

		fprintf(stderr, "[AQUA Compiler] ERROR Replacing the child process with the lang process (%s) failed for some reason (errno = %d)\n", lang_path, errno);
		return 1;
	}

	printf("[AQUA Compiler] Waiting for child language process to finish ...\n");

	int wstatus = 0;
	while (wait(&wstatus) > 0);

	if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus)) {
		fprintf(stderr, "[AQUA Compiler] ERROR lang process failed\n");
		return 1;
	}

	if (none_targ) {
		printf("[AQUA Compiler] Target is none, no need to go further\n");
		goto done;
	}

	printf("[AQUA Compiler] Running targ process ...\n");

	if (!fork()) { // child process
		char* targ_path = (char*) malloc(strlen(COMPILER_DIR_PATH) + strlen(targ) + 8 /* strlen("/targs/") + 1 */); // no risk of memory leak
		sprintf(targ_path, "%s/targs/%s", COMPILER_DIR_PATH, targ);

		execl(targ_path, debug ? "--debug" : "", "--input", intermediate_path, "--output", real_output_path, NULL);

		fprintf(stderr, "[AQUA Compiler] ERROR Replacing the child process with the targ process (%s) failed for some reason (errno = %d)\n", targ_path, errno);
		return 1;
	}

	wstatus = 0;
	while (wait(&wstatus) > 0);

	if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus)) {
		fprintf(stderr, "[AQUA Compiler] ERROR targ process failed\n");
		return 1;
	}
	
done:

	printf("[AQUA Compiler] Done\n");
	return 0;
}
