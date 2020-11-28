#include "table.h"

double hashString(const char* key) {
    uint32_t hash = 2166136261u;

    for (int i=0; i < strlen(key); i++) {
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}