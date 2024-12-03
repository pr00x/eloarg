/*
    EloArg - A Lightweight C Library for Command-Line Argument Parsing
    Author: Prox
    
    Description:
    EloArg is a lightweight and versatile command-line argument parser library designed
    to simplify the process of handling and parsing command-line options in C applications with modern and elegant syntax.
    It provides an intuitive API for defining, parsing, and retrieving command-line arguments with 
    support for short and long options, optional and required arguments, and descriptive help messages.

    Features:
    - Short and Long Options: Support for traditional short (`-a`) and long (`--option`) style options.
    - Optional and Required Arguments: Define whether an argument is mandatory, optional, or has no associated value.
    - Descriptive Help Messages: Automatically generate and display a help message with descriptions of all defined options.
    - Dynamic Option Management: Add and parse options dynamically during runtime.
    - Integration with HashTable: Efficient storage and retrieval of parsed options using an underlying hash table for fast lookups.
    - Memory Management Utilities: Provides functions to manage memory safely and prevent leaks.

    Notes:
    - All arguments and their attributes (e.g., description, type) are stored in dynamically allocated structures.
    - Users are responsible for invoking `eloArgFree()` to release all allocated resources after use.
    - The library is designed to integrate seamlessly with other C codebases, leveraging `HashTable` for argument management.

    Functions:
    - Initialization:
        - `eloArgInit`: Creates and initializes a new `EloArg` instance with a specified hash table size.
    - Option Definition:
        - `add`: Registers a new command-line argument with its short and long options, description, and value type.
    - Parsing:
        - `parse`: Processes `argc` and `argv` to identify and store user-provided options.
    - Retrieval:
        - `has`: Checks whether a specific option was provided by the user.
        - `get`: Retrieves the value associated with a specific option.
    - Help:
        - `help`: Displays a user-friendly help message with descriptions of all defined options.
    - Cleanup:
        - `free`: Releases all allocated resources for the `EloArg` instance.
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "eloarg.h"

#define LIBRARY_NAME "EloArg"
#define HELP_PADDING_RIGHT 38
#define HELP_OPTION_LINE_LENGTH 46
#define HELP_MAX_DESCRIPTION_SENTENCE_LENGTH 70

static EloArg eloarg;

static void error(const char *formatStr, ...) {
    va_list args;
    fprintf(stderr, "%s: ", LIBRARY_NAME);

    va_start(args, formatStr);
    vfprintf(stderr, formatStr, args);
    va_end(args);

    fputc('\n', stderr);
    
    eloArgFree();
    exit(EXIT_FAILURE);
}

static void memAllocError(const char *detail) {
    error("Cannot allocate memory for '%s'.", detail);
}

static void freeEloArgOption(EloArgOption *option) {
    if(!option)
        return;

    option->refCount--;

    if(option->refCount == 0) {
        FREE(option->value);
        FREE(option);
    }
}

static void printDescription(const char *description) {
    size_t length = 0;
    const char *wordStart = description;

    while(*description != '\0') {
        if(*description == ' ' || *description == '\t' || *(description + 1) == '\0') {
            int wordLen = description - wordStart;

            if(length + wordLen + (length > 0) <= HELP_MAX_DESCRIPTION_SENTENCE_LENGTH) {
                if(length > 0)
                    putchar(' ');

                printf("%.*s", wordLen, wordStart);
                length += wordLen + (length > 0);
            }
            else {
                // New line
                printf("\n%*s%.*s", HELP_OPTION_LINE_LENGTH, "", wordLen, wordStart);
                length = wordLen;
            }

            wordStart = description + 1;
        }

        description++;
    }

    putchar('\n');
}

static void printHelp(const char *description, const char *footerDescription) {
    HashTable *hashTable = eloarg.hashTable;

    if(hashTable->count(hashTable) == 0)
        return;

    HashTable *exist = initHashTable(eloarg.count * 2);

    if(description)
        puts(description);

    puts("Options:");

    for (size_t i = 0; i < hashTable->getSize(hashTable); i++) {
        if(hashTable->table[i] && hashTable->table[i]->occupied) {
            EloArgOption *option = hashTable->table[i]->value;

            if(exist->has(exist, option->uniqueStrId))
                continue;
            else
                exist->set(exist, option->uniqueStrId, "true");

            if(*option->shortOption && *option->longOption) {
                printf("  -%s, --%s%-*s",
                        option->shortOption,
                        option->longOption,
                        HELP_PADDING_RIGHT - (int)strlen(option->longOption),
                        "");
                printDescription(option->description);
            }
            else if(*option->shortOption) {
                printf("  -%s%-*s", 
                        option->shortOption,
                        HELP_PADDING_RIGHT,
                        "");
                printDescription(option->description);
            }
            else if(*option->longOption) {
                printf("      --%s%-*s",
                        option->longOption,
                        HELP_PADDING_RIGHT - (int)strlen(option->longOption),
                        "");
                printDescription(option->description);
            }
        }
    }

    if(footerDescription)
        printf("\n%s\n", footerDescription);

    exist->free(&exist);
    eloArgFree();
    exit(EXIT_SUCCESS);
}

static void eloArgAdd(char *shortOption, char *longOption, char *description, ArgValueType valueType) {
    if(!shortOption && !longOption)
        error("You must enter either the short or long option.");
    else if(!description)
        error("You must set the description for option '%s'.", longOption ? longOption : shortOption);

    HashTable *hashTable = eloarg.hashTable;

    if(hashTable->has(hashTable, shortOption))
        error("You've already set the short option '%s'.", shortOption);
    else if(hashTable->has(hashTable, longOption))
        error("You've already set the long option '%s'.", longOption);

    EloArgOption *option = malloc(sizeof(EloArgOption));

    if(!option)
        memAllocError("EloArgOption");

    if(shortOption && strlen(shortOption) > ELOARG_SHORT_OPTION_LENGTH)
        error("The maximum length of the short option is %u.", ELOARG_SHORT_OPTION_LENGTH);
    else if(longOption && strlen(longOption) > ELOARG_LONG_OPTION_LENGTH)
        error("The maximum length of the long option is %u.", ELOARG_LONG_OPTION_LENGTH);
    else if(strlen(description) > ELOARG_DESCRIPTION_LENGTH)
        error("The maximum length of the description is %u.", ELOARG_DESCRIPTION_LENGTH);
        
    *option->shortOption = '\0';
    *option->longOption = '\0';
    
    strcpy(option->description, description);
    option->valueType = valueType;
    
    snprintf(option->uniqueStrId, sizeof(option->uniqueStrId), "%u", eloarg.count);

    option->value = NULL;
    option->provided = false;
    option->count = 0;
    option->refCount = 0; // Using a reference counter because two keys can share the same memory

    if(shortOption) {
        strcpy(option->shortOption, shortOption);
        option->refCount++;
        hashTable->set(hashTable, shortOption, option);
    }

    if(longOption) {
        strcpy(option->longOption, longOption);
        option->refCount++;
        hashTable->set(hashTable, longOption, option);
    }

    eloarg.count++;
}

static void eloArgParse(int argc, char **argv) {
    HashTable *hashTable = eloarg.hashTable;

    if(argc == 0 || hashTable->count(hashTable) == 0)
        return;

    // Loop through arguments
    for(size_t i = 1; i < argc; i++) {
        bool optionMatched = false;

        // Terminate options parsing
        if(strcmp(argv[i], "--") == 0)
            return;

        // Check for the long option
        if(argv[i][0] == '-' && argv[i][1] == '-') {
            char *eqPos = strchr(argv[i], '=');

            // If '=' exists, split the option and the value
            if(eqPos) {
                *eqPos = '\0'; // Terminate the option part (--option= -> =)
                EloArgOption *option = (EloArgOption *)hashTable->get(hashTable, argv[i] + 2); // Skip the '--'

                if(!option)
                    error("Unknown option: %s.\nUse option '--help' for more information.", argv[i]);

                if(option->valueType == ARG_OPTIONAL || option->valueType == ARG_REQUIRED) {
                    if(*(eqPos + 1) == '\0')
                        error("Missing value for option: --%s=", option->longOption);

                    option->provided = true;
                    option->value = strdup(eqPos + 1); // Value after '='
                    option->count++;

                    if(!option->value)
                        memAllocError("EloArgOption value");
                }
                else
                    error("option '--%s' doesn't allow an argument.", option->longOption);

                optionMatched = true;
            }
            else { // Long option with space
                EloArgOption *option = (EloArgOption *)hashTable->get(hashTable, argv[i] + 2);

                if(!option)
                    error("Unknown option: %s.\nUse option '--help' for more information.", argv[i]);

                option->provided = true;
                option->count++;
                optionMatched = true;

                // Return if the valueType is ARG_INFO (for --help and --version etc.)
                if(option->valueType == ARG_INFO)
                    return;
                else if(option->valueType == ARG_OPTIONAL || option->valueType == ARG_REQUIRED) { // Handle argument for options that require a value
                    if(i + 1 < argc && argv[i + 1][0] != '-') {
                        option->value = strdup(argv[i + 1]);

                        if(!option->value)
                            memAllocError("EloArgOption value");

                        i++; // Skip the value argument
                    }
                    else
                        error("Missing value for option: --%s", option->longOption);
                }
            }
        }

        // Check for combined short options
        if(!optionMatched && argv[i][0] == '-') {
            char *opt = argv[i] + 1; // Skip the '-'
            char tmpOpt[2] = { *opt, '\0' };

            while(*opt) {
                EloArgOption *option = (EloArgOption *)hashTable->get(hashTable, tmpOpt);

                if(!option)
                    error("Unknown option '%c'.\nUse option '--help' for more information.", *opt);

                option->provided = true;
                option->count++;
                optionMatched = true;

                if(option->valueType == ARG_INFO)
                    return;
                else if(option->valueType == ARG_OPTIONAL || option->valueType == ARG_REQUIRED) {
                    if(*(opt + 1) != '\0') { // -p443
                        option->value = strdup(opt + 1);

                        if(!option->value)
                            memAllocError("EloArgOption value");

                        break;
                    }
                    else if(i + 1 < argc && argv[i + 1][0] != '-') { // -p 443
                        option->value = strdup(argv[i + 1]);

                        if(!option->value)
                            memAllocError("EloArgOption value");

                        i++; // Skip the value argument
                    }
                    else
                        error("Missing value for option: -%c", *opt);
                }

                opt++;
            }
        }
    }

    // Check for missing required arguments
    for(size_t i = 0; i < hashTable->getSize(hashTable); i++) {
        if(hashTable->table[i] && hashTable->table[i]->occupied) {
            EloArgOption *option = (EloArgOption *)hashTable->table[i]->value;

            if(option->valueType == ARG_REQUIRED && option->value == NULL) {
                if(*option->longOption)
                    error("Missing required option: '--%s'\nUse option '--help' for more information.", option->longOption);
                else
                    error("Missing required option: '-%s'\nUse option '--help' for more information.", option->shortOption);
            }
        }
    }
}

static bool eloArgHas(const char *key) {
    EloArgOption *option = (EloArgOption *)eloarg.hashTable->get(eloarg.hashTable, key);
    
    return option && option->provided;
}

static const char *eloArgGet(const char *key) {
    EloArgOption *option = (EloArgOption *)eloarg.hashTable->get(eloarg.hashTable, key);

    return option && option->provided ? option->value : NULL;
}

static size_t eloArgCount(const char *key) {
    EloArgOption *option = (EloArgOption *)eloarg.hashTable->get(eloarg.hashTable, key);

    return option && option->provided ? option->count : 0;
}

static void eloArgFree() {
    if(!eloarg.hashTable)
        return;

    EloArgOption *option;
    HashTable *hashTable = eloarg.hashTable;

    // Free the EloArgOptions
    if(hashTable->count(hashTable) > 0)
        for(size_t i = 0; i < hashTable->getSize(hashTable); i++)
            if(hashTable->table[i] && hashTable->table[i]->occupied) {
                option = (EloArgOption *)hashTable->table[i]->value;
                
                hashTable->table[i]->value = NULL;
                freeEloArgOption(option);
            }

    eloarg.hashTable->free(&eloarg.hashTable);
}

EloArg *eloArgInit(size_t size) {
    eloarg.hashTable = initHashTable(size * 3); // Avoid hash table resizing
    eloarg.count = 0;
    eloarg.help = printHelp;
    eloarg.add = eloArgAdd;
    eloarg.parse = eloArgParse;
    eloarg.has = eloArgHas;
    eloarg.get = eloArgGet;
    eloarg.getCount = eloArgCount;
    eloarg.free = eloArgFree;

    return &eloarg;
}