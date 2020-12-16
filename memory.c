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

static void markObject(Obj* object) {
    if (object->isMarked) return;
    object->isMarked = true;

    if (vm.greyCapacity == 0) {
        vm.greyStack = ALLOCATE(Obj*, vm.greyStack, 0);
        vm.greyCapacity = ARRAY_INITIAL_SIZE;
    } else if (vm.greyCapacity < vm.greyCount + 1) {
        vm.greyStack = ALLOCATE(Obj*, vm.greyStack, vm.greyCapacity);
        vm.greyCapacity *= ENLARGEMENT_FACTOR;
    }

    vm.greyStack[vm.greyCount++] = object;
}

static void markArray(Value* arr, ssize_t size) {
    if (size <= 0) return;
    for (size_t i = 0; i < size; i++) {
        if (arr[i].type == VAL_FUNCTION || arr[i].type == VAL_STRING) {
            markObject(arr[i].as.obj);
        }
    }
}

static void markCompilers() {
    Compiler* cmpl = compiler;
    while(cmpl) {
        markObject(AS_OBJ(cmpl->function));
        cmpl = cmpl->enclosedCompiler;
    }
}

static void mark() {
    markArray(vm.stack, (vm.stackTop - vm.stack));
    markCompilers();
}

static void blacken(Obj* object) {
    switch(object->type) {
        case OBJ_FUNCTION: {
            ObjFunction* function = AS_FUNCTION(object);
            markArray(function->constants, function->constPos);
            if (function->name)
                markObject(AS_OBJ(function->name));
            break;
        }

        case OBJ_STRING:
            markObject(AS_OBJ(object));
            break;
    }
}

static void processGrey() {
    while (vm.greyCount > 0)
        blacken(vm.greyStack[--vm.greyCount]);
    
    // Initialize grey stack.
    free(vm.greyStack);
    vm.greyStack = NULL;
    vm.greyCapacity = 0;
}


static void sweep() {

}

static void collectGarbage() {
    printf("--GC-Start\n");

    mark();
    processGrey();
    sweep();

    printf("--GC-End\n");
}

Obj* allocateObject(ObjType type) {
#ifdef ZAO_GC
    collectGarbage();
#endif

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