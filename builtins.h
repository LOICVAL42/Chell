#ifndef CSHELL_BUILTINS_H
#define CSHELL_BUILTINS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "libs/scplib/scp/containers/hashmap.h"
#include "libs/scplib/scp/utils.h"

int cd(int argc, char** args) {
    printf("cd moi");
    fflush(stdout);
	if (argc == 1) {
		chdir(getenv("HOME")); // Can glitch sometimes but fuck it hey i'll deal with it later
		return EXIT_SUCCESS;
	}
	if (argc > 2) {
		fprintf(stderr, "cd: Too many arguments");
		return EXIT_FAILURE;
	}
	if (chdir(args[1]) == 0) return EXIT_SUCCESS;
	switch (errno) {
		case EACCES:
			fprintf(stderr, "cd: Acces denied");
			return EXIT_FAILURE;
		case ENOENT:
			fprintf(stderr, "cd: File or directory does not exist");
			return EXIT_FAILURE;
		case ENOTDIR:
			fprintf(stderr, "cd: Not a directory");
			return EXIT_FAILURE;
		default:
			fprintf(stderr, "cd: Error");
			return EXIT_FAILURE;
	}
}

typedef int (*func)(int argc, char** args);

typedef struct func_pair {
	const char* key;
	func value;
} func_pair;

static func_pair builtins[] = {
	{ "cd", cd }
};

struct scpHashMap* generateHashmap(void) {
	struct scpHashMap* builtinsHashMap = scpHashMap_new(scpHash_stringA, scpHash_stringB, scpCmp_string);
	for (size_t i = 0; i < sizeof builtins / sizeof *builtins; ++i)
		scpHashMap_insert(builtinsHashMap, builtins[i].key, builtins + i);
	return builtinsHashMap;
}

#endif // CSHELL_BUILTINS_H
