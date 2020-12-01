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

uint32_t hashString(const char* key) {
    uint32_t hash = 2166136261u;

    for (int i=0; i < strlen(key); i++) {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

Entry* findEntry(Table* table, ObjString* key) {
    uint32_t index = key->hash % table->capacity;
    for (;;) {
        if (table->entries[index].key == NULL || table->entries[index].key->hash == key->hash)
            return &table->entries[index];

        index = (index + 1) % table->capacity;
    }
}

static void growTable(Table* table) {
    if (table->capacity == 0) {
        // TODO: create function in memory.h to handle allocation in one place.
        table->entries = (Entry*)malloc(sizeof(Entry) * ARRAY_INITIAL_SIZE);
        table->capacity = ARRAY_INITIAL_SIZE;

        for (uint32_t i = 0; i < table->capacity; i++) {
            table->entries[i].key = NULL;
        }
    } else {
        // Allocate entries without tombstones - they will be not included.
        Entry* entries = table->entries;
        
        // New Table.
        table->capacity *= ENLARGEMENT_FACTOR;
        table->size = 0;
        table->entries = (Entry*)malloc(sizeof(Entry) * table->capacity * ENLARGEMENT_FACTOR);

        for (uint32_t i = 0; i < table->capacity; i++) {
            table->entries[i].key = NULL;
        }

        // Populate with entries.
        for (uint32_t i = 0, end = table->capacity/ENLARGEMENT_FACTOR; i < end; i++) {
            // Don't include tombstones and empty Entries.
            if (entries[i].tombstone == true || entries[i].key == NULL) continue;
            addEntry(table, &entries[i]);
        }

        free(entries);
    }
}

static void freeEntry(Entry* entry) {
    // free(entry->key);
    // free(entry->value);
}

bool addEntry(Table* table, Entry* entry) {
    if (table->capacity == 0 || table->size >= table->capacity * LOAD_FACTOR) 
        growTable(table);

    Entry* entryToChange = findEntry(table, entry->key);
    if (entryToChange->key != NULL) {
        freeEntry(entry);
        return true;
    }

    *entryToChange = *entry;
    table->size++;
    return false;
}

void deleteEntry(Table* table, ObjString* key) {
    Entry tombstone;
    tombstone.key = NULL;
    tombstone.tombstone = true;

    Entry* entry = findEntry(table, key);
    freeEntry(entry);
    *entry = tombstone;
}