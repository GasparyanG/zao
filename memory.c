#include <stdlib.h>

#include "memory.h"

void* allocate(void* pointer, size_t size) {

    if (size == 0) 
        // TODO: Check pointer and if it's not null then exit program
        return malloc(ARRAY_INITIAL_SIZE * size);
    else 
        return reallocate(pointer, ENLARGEMENT_FACTOR * size);
}

void* reallocate(void* pointer, size_t size) {
    if (size == 0) {
        free(pointer);
        pointer = NULL;
    } else
        pointer = realloc(pointer, size);
    
    return pointer;
}