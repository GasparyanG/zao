#ifndef ZAO_MEMORY_H
#define ZAO_MEMORY_H

#include <stdint.h>

#include "object.h"

#define ENLARGEMENT_FACTOR 2
#define ARRAY_INITIAL_SIZE 8

#define ALLOCATE(type, pointer, size) \
    (type*)allocate((void*)pointer, size * sizeof(type))

#define ALLOCATE_WP(type, size) \
    (type*)malloc(size*sizeof(type))

void* allocate(void* pointer, size_t size);
void* reallocate(void* pointer, size_t size);

Obj* allocateObject(ObjType type);

#include "vm.h"
#endif