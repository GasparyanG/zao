#ifndef ZAO_TABLE_H
#define ZAO_TABLE_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "object.h"
#include "value.h"

#define LOAD_FACTOR 0.75    // Enlarge Table if size/capacity > LOAD_FACTOR.

typedef struct {
    ObjString* key;     // Key to search with.
    Value value;
    bool tombstone;     // Are we deleted or not ?
} Entry;

typedef struct {
    Entry* entries;     // Array of entires.
    size_t size;        // Amount of entires.
    size_t capacity;    // Amount of space to store entires.
} Table;


// Helper functions.
// Resource management.
void initTable(Table* table);
void freeTable(Table* table);

// Table and Entry manipulation.
uint32_t hashString(const char* key);
Entry* findEntry(Table* table, ObjString* key);
bool addEntry(Table* table, Entry* entry);
void deleteEntry(Table* table, ObjString* key);

#endif