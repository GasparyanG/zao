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

Obj* allocateObject(ObjType type) {
    Obj* obj;
    switch(type) {
        case OBJ_STRING:
            obj = AS_OBJ(ALLOCATE_WP(ObjString, 1));
            break;
        case OBJ_FUNCTION:
            obj = AS_OBJ(ALLOCATE_WP(ObjFunction, 1));
            break;
        case OBJ_CLOSURE:
            obj = AS_OBJ(ALLOCATE_WP(ObjClosure, 1));
            break;
        case OBJ_UPVALUE:
            obj = AS_OBJ(ALLOCATE_WP(ObjUpValue, 1));
            break;
    }

    // Prepare object and chain in singly-linked list.
    obj->type = type;
    obj->next = vm.objects;
    vm.objects = obj;

    return obj;
}