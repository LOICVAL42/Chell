#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "builtins.h"

#define BUFFER_SIZE 256

char* appendPath(const char* s1, const char* s2){
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

char** splitInput(const char* input){
    char** ans = malloc(0);
    char* arg = malloc(0);
    size_t j = 0;
    ulong k = 0;
    for (int i = 0; !input[i] || input[i] != '\n'; ++i) {
        if (input[i] == ' '){
            ans = realloc(ans, (k+1)*sizeof(char*));
            ans[k] = arg;
            ++k;
            j = 0;
            arg = malloc(0);
            continue;
        }
        arg = realloc(arg, j+1);
        arg[j] = input[i];
        ++j;
    }
    ans = realloc(ans, k+2);
    ans[k] = arg;
    ans[k+1] = NULL;
    return ans;
}

char** generatePath(const char* rawPath){
    char** paths = malloc(0);
    char* path = malloc(0);
    unsigned long pathSize = 0;
    unsigned long nbPaths = 0;
    for (int i = 0; rawPath[i] != '\0'; ++i) {
        if (rawPath[i] == ':'){
            paths = realloc(paths, (nbPaths+1)*sizeof(char*));
            paths[nbPaths] = path;
            path = malloc(0);
            pathSize = 0;
            ++nbPaths;
            continue;
        }
        path = realloc(path, pathSize+1);
        path[pathSize] = rawPath[i];
        ++pathSize;
    }
    paths = realloc(paths, nbPaths+2);
    paths[nbPaths] = path;
    paths[nbPaths+1] = NULL;
    return paths;
}

int execAllPaths(char** args, char** paths){
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

int execBuiltIn(int argc, char** args, func cmd){
    return cmd(argc, args);
}

int main() {
    scpHashMap* builtins = generateHashmap();
    char** paths = generatePath(getenv("PATH"));
    while(1){
        char pwd[BUFFER_SIZE];
        printf("PATH: [%s]\n$ ", getcwd(pwd, BUFFER_SIZE));
        fflush(stdout);
        char userInput[BUFFER_SIZE] = {0};
        fgets(userInput, BUFFER_SIZE, stdin);
        if (feof(stdin)) return EXIT_SUCCESS;
        char** args = splitInput(userInput);
        pid_t pid = fork();
        if (pid == 0){
            func cmd = (func)scpHashMap_search(builtins, *args);
            if (cmd != NULL) execBuiltIn(sizeof args , args, cmd);
            else execAllPaths(args, paths);
        }
        waitpid(-1, &pid, 0);
        printf("\n");
    }
}
