#include <stdio.h>
#include <stdlib.h>
#include "libs/scplib/scp/containers/hashmap.h"
#include "libs/scplib/scp/utils/hash.h"
#include "libs/scplib/scp/utils/cmp.h"
#ifndef CSHELL_BUILTINS_H
#define CSHELL_BUILTINS_H

#define scpHashMap struct scpHashMap

int cd(int argc, char** args){
    printf("Salut ! cd trop bien");
    return 1;
}

typedef int (*func)(int argc, char** args);
typedef struct func_pair{
    const char* key;
    func value;
} func_pair;

func_pair builtins[] = {
        { "cd", cd},
        {NULL, NULL}
};

scpHashMap* generateHashmap(){
    scpHashMap* builtinsHashMap = scpHashMap_new(scpHash_stringA, scpHash_stringB, scpCmp_string);
    for (int i = 0; builtins[i].key != NULL; ++i) {
        scpHashMap_insert(builtinsHashMap, builtins[i].key, builtins[i].value);
    }
    return builtinsHashMap;
}

#endif //CSHELL_BUILTINS_H
