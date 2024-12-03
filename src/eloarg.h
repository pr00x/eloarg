#ifndef ELOARG_H
#define ELOARG_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "hashtable.h"

#define ELOARG_SHORT_OPTION_LENGTH 1
#define ELOARG_LONG_OPTION_LENGTH 32
#define ELOARG_DESCRIPTION_LENGTH 150
#define ELOARG_UNIQUE_STR_ID 12

#define FREE(ptr) do {  \
    if(ptr) {   \
        free(ptr);  \
        ptr = NULL; \
    }   \
} while(0)

typedef enum {
    ARG_NONE,
    ARG_INFO,
    ARG_OPTIONAL,
    ARG_REQUIRED
} ArgValueType;

typedef struct {
    char shortOption[ELOARG_SHORT_OPTION_LENGTH + 1];
    char longOption[ELOARG_LONG_OPTION_LENGTH + 1];
    char description[ELOARG_DESCRIPTION_LENGTH + 1];
    ArgValueType valueType;
    char uniqueStrId[ELOARG_UNIQUE_STR_ID];
    char *value;
    bool provided;
    size_t count;
    uint8_t refCount;
} EloArgOption;

typedef struct EloArg {
    HashTable *hashTable;
    uint32_t count;

    void (*help)(const char *description, const char *footerDescription);
    void (*add)(char *shortOption, char *longOption, char *description, ArgValueType valueType);
    void (*parse)(int argc, char **argv);
    bool (*has)(const char *key);
    const char *(*get)(const char *key);
    size_t (*getCount)(const char *key);
    void (*free)();
} EloArg;

static void printHelp(const char *description, const char *footerDescription);
static void eloArgAdd(char *shortOption, char *longOption, char *description, ArgValueType valueType);
static void eloArgParse(int argc, char **argv);
static bool eloArgHas(const char *key);
static const char *eloArgGet(const char *key);
static size_t eloArgCount(const char *key);
static void eloArgFree();
EloArg *eloArgInit(size_t size);

#endif