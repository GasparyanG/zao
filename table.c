#include "table.h"
#include <stdio.h>

void initTable(Table* table) {
    table->size = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table) {
    free(table->entries);
}

double hashString(const char* key) {
    uint32_t hash = 2166136261u;

    for (int i=0; i < strlen(key); i++) {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

Entry* findEntry(Table* table, const char* key, uint32_t hash) {
    uint32_t index = hash % table->capacity;
    uint32_t cycle = 0;
    for (;;) {
        if (table->entries[index].key == NULL || table->entries[index].hash == hash)
            return &table->entries[index];
        
        index = (hash + (++cycle)) % table->capacity;
    }
}

static void growTable(Table* table) {
    if (table->capacity == 0) {
        table->entries = ALLOCATE(Entry, table->entries, 0);
        table->capacity = ARRAY_INITIAL_SIZE;
    } else {
        // TODO: handle tombstones.
        table->entries = ALLOCATE(Entry, table->entries, table->capacity);
        table->capacity *= ENLARGEMENT_FACTOR;
    }
}

void addEntry(Table* table, Entry* entry) {
    if (table->capacity == 0 || table->size >= table->capacity * LOAD_FACTOR) 
        growTable(table);

    Entry* entryToChange = findEntry(table, entry->key, entry->hash);
    *entryToChange = *entry;
    table->size++;
}