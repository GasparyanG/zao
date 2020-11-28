#ifndef ZAO_TABLE_H
#define ZAO_TABLE_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "value.h"

#define LOAD_FACTOR 0.75    // Enlarge Table if size/capacity > LOAD_FACTOR.

typedef struct {
    const char* key;    // Key to find value with.
    Value* value;       // Value pointer (points to compiler's constant table).
    uint32_t hash;      // Hashed value.
    bool tombstone;     // Are we deleted or not ?
} Entry;

typedef struct {
    Entry* entries;     // Array of entires.
    size_t size;        // Amount of entires.
    size_t capacity;    // Amount of space to store entires.
} Table;


// Helper functions.
double hashString(const char* key);

#endif