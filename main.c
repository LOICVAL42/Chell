#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "builtins.h"

#define BUFFER_SIZE 256

typedef struct argInput {
	int argc;
	char** args;
} argInput;

char* appendPath(const char* s1, const char* s2) {
	char* ans = malloc(0);
	unsigned long i;
	for (i = 0; s1[i] != '\0' ; ++i) {
		ans = realloc(ans, i+1);
		ans[i] = s1[i];
	}
	ans = realloc(ans, i+1);
	ans[i] = '/';
	unsigned long j;
	for (j = 0; s2[j] != '\0'; ++j) {
		ans = realloc(ans, i+j+2);
		ans[i+j+1] = s2[j];
	}
	ans = realloc(ans, i+j+3);
	ans[i+j+2] = '\0';
	return ans;
}

argInput splitInput(const char* input) {
	char** ans = malloc(0);
	char* arg = malloc(0);
	size_t j = 0;
	unsigned long k = 0;
	for (int i = 0; input[i] != '\0' && input[i] != '\n'; ++i) {
		if (input[i] == ' ') {
            if (j == 0) continue;
			ans = realloc(ans, (k+1)*sizeof(char*));
			ans[k] = arg;
			++k;
			j = 0;
			arg = malloc(0);
			continue;
		}
        arg = realloc(arg, (j + 1) * sizeof(char));
		arg[j] = input[i];
		++j;
	}
	ans = realloc(ans, k + 2);
	ans[k] = arg;
	ans[k + 1] = NULL;
	struct argInput argI;
	argI.argc = (int)k+1;
	argI.args = ans;
	return argI;
}

argInput* splitAllInput(const char* input){
    argInput* ans = malloc(0);
    char* partOfInput = malloc(0);
    unsigned long j = 0;
    unsigned long k = 0;
    for (int i = 0; !input[i] || input[i] != '\n'; ++i) {
        switch(input[i]){
            case ';':
            case '&':
            case '|':
                if (k == 0) {
                    fprintf(stderr, "Syntax Error in commands list : too much %c characters", input[i]);
                    return NULL;
                }
                ans = realloc(ans, (j+2)* sizeof(struct argInput));
                partOfInput = realloc(partOfInput, k+1);
                partOfInput[k] = '\0';
                ans[j] = splitInput(partOfInput + '\0');
                argInput separator;
                char** args = (char **) &input[i];
                separator.args = args;
                separator.argc = 0;
                ans[j+1] = separator;
                k = 0;
                partOfInput = malloc(0);
                j += 2;
                if (input[i] != ';') ++i;
                break;
            default:
                partOfInput = realloc(partOfInput, k+1);
                partOfInput[k] = input[i];
                ++k;
                break;
        }
    }
    ans = realloc(ans, (j+2)* sizeof(argInput));
    partOfInput = realloc(partOfInput, k+1);
    partOfInput[k] = '\0';
    ans[j] = splitInput(partOfInput);
    argInput null;
    null.argc = NULL;
    null.args = NULL;
    ans[j+1] = null;
    return ans;
}

char** generatePath(const char* rawPath) {
	char** paths = malloc(0);
	char* path = malloc(0);
	unsigned long pathSize = 0;
	unsigned long nbPaths = 0;
	for (int i = 0; rawPath[i] != '\0'; ++i) {
		if (rawPath[i] == ':'){
			paths = realloc(paths, (nbPaths + 1) * sizeof(char*));
			paths[nbPaths] = path;
			path = malloc(0);
			pathSize = 0;
			++nbPaths;
			continue;
		}
		path = realloc(path, pathSize + 1);
		path[pathSize] = rawPath[i];
		++pathSize;
	}
	paths = realloc(paths, nbPaths + 2);
	paths[nbPaths] = path;
	paths[nbPaths + 1] = NULL;
	return paths;
}

int execAllPaths(char** args, char** paths) {
	char* firstArg = *args;
	if (*firstArg == '/' || *firstArg == '.') return execve(*args, args, __environ);
	for (int i = 0; paths[i] != NULL; ++i) {
		*args = appendPath(paths[i], firstArg);
		int result = execve(*args, args, __environ);
		if (result != -1) return result;
	}
	*args = firstArg;
	return execve(*args, args, __environ);
}

int execBuiltIn(int argc, char** args, func cmd) {
	return cmd(argc, args);
}

int main(void) {
	struct scpHashMap* builtins = generateHashmap();
	char** paths = generatePath(getenv("PATH"));
	while (true) {
		scpAttribute_unused char pwd[BUFFER_SIZE];
		// printf("PATH: [%s]\n$ ", getcwd(pwd, BUFFER_SIZE));
		printf("$ ");
		fflush(stdout);
		char userInput[BUFFER_SIZE] = {0};
		fgets(userInput, BUFFER_SIZE, stdin);
		if (feof(stdin)) return EXIT_SUCCESS;
		argInput* input = splitAllInput(userInput);
		pid_t pid = fork();
		if (pid == 0){
            if (input == NULL) return EXIT_FAILURE;
            int previousResult = EXIT_SUCCESS;
            for (int i = 0; input[i].args != NULL && input[i].argc != NULL ; ++i) {
                if (input[i].argc == 0){
                    printf("Separator\n");
                    fflush(stdout);
                    switch (**(input[i].args)) {
                        case ';':
                            continue;
                        case '&':
                            if (previousResult == EXIT_FAILURE) return EXIT_FAILURE;
                            continue;
                        case '|':
                            if (previousResult == EXIT_SUCCESS) return EXIT_SUCCESS;
                            continue;
                        default:
                            fprintf(stderr, "Unknown separator for lists. Should NOT happen");
                            break;
                    }
                }
                func_pair* cmd = (func_pair*)scpHashMap_search(builtins, *input[i].args);
                if (cmd != NULL) previousResult = execBuiltIn(input[i].argc, input[i].args, (*cmd).value);
                else previousResult = execAllPaths(input[i].args, paths);
            }
		}
		waitpid(-1, &pid, 0);
	}
}
