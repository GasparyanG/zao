#ifndef ZAO_TABLE_H
#define ZAO_TABLE_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "value.h"

#define LOAD_FACTOR 0.75    // Enlarge Table if size/capacity > LOAD_FACTOR.

struct ObjString;

typedef struct {
    ObjString* key;     // Key to search with.
    Value value;
    bool tombstone;     // Are we deleted or not ?
} Entry;

typedef struct Table {
    Entry* entries;     // Array of entries.
    size_t size;        // Amount of entries.
    size_t capacity;    // Amount of space to store entries.
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

#include "memory.h"
#endif