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
        vm.greyCapacity = ARRAY_INITIAL_SIZE;
        size_t size = vm.greyCapacity * sizeof(Obj*);
        vm.greyStack = (Obj**)malloc(size);
    } else if (vm.greyCapacity < vm.greyCount + 1) {
        vm.greyCapacity *= ENLARGEMENT_FACTOR;
        size_t size = vm.greyCapacity * sizeof(Obj*);
        vm.greyStack = (Obj**)realloc(vm.greyStack, size);
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

static void markTable(Table table) {
    for (size_t i = 0; i < table.capacity; i++) {
        Entry entry = table.entries[i];
        if (entry.key == NULL) continue;
        
        markObject(AS_OBJ(entry.key));
        if (IS_OBJ(entry.value))
            markObject(entry.value.as.obj);
    }
}

static void markLocals() {
    for (uint8_t i = 0; i < UINT8_MAX; i++)
        if (IS_OBJ(vm.locals[i]))
            markObject(vm.locals[i].as.obj);
}

static void markCallFrame() {
    CallFrame* callFrame = vm.callFrame;
    while (callFrame != NULL) {
        markObject(AS_OBJ(callFrame->closure));
        callFrame = callFrame->nextFrame;
    }
}

static void mark() {
    markArray(vm.stack, (vm.stackTop - vm.stack));
    markTable(vm.globals);
    markLocals();
    markCallFrame();
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

        case OBJ_CLOSURE: {
            ObjClosure* closure = AS_CLOSURE(object);
            markObject(AS_OBJ(closure->function));

            // for (uint8_t i = 0; i < closure->function->upvaluesCount; i++)
            //     markObject(AS_OBJ(closure->upvalues[i]));
        }
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

static void freeObject(Obj* object) {
    switch(object->type) {
        case OBJ_STRING:
            free(AS_STRING(object));
            break;
        case OBJ_FUNCTION: {
            printf("free function");
            break;
        } case OBJ_CLOSURE: {
            free(AS_CLOSURE(object));
            break;
        } case OBJ_UPVALUE: {
            free(AS_UPVALUE(object));
            break;
        }
    }
}

static void sweep() {
    Obj* previous = NULL;
    Obj* current = vm.objects;

    while (current != NULL) {
        if (current->isMarked) {
            current->isMarked = false;
            previous = current;
            current = current->next;
        } else {
            if (current == vm.objects) {
                vm.objects = current->next;
                freeObject(current);
                current = vm.objects;
            } else {
                previous->next = current->next;
                current->next = NULL;
                freeObject(current);
                current = previous->next;
            }
        }
    }
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
        case OBJ_STRING: {
            obj = AS_OBJ(ALLOCATE_WP(ObjString, 1));
            break;
        }
        case OBJ_FUNCTION: {
            obj = AS_OBJ(ALLOCATE_WP(ObjFunction, 1));
            break;
        }
        case OBJ_CLOSURE: {
            obj = AS_OBJ(ALLOCATE_WP(ObjClosure, 1));
            break;
        }
        case OBJ_UPVALUE: {
            obj = AS_OBJ(ALLOCATE_WP(ObjUpValue, 1));
            break;
        }
    }

    // Prepare object and chain in singly-linked list.
    obj->type = type;
    obj->next = vm.objects;
    obj->isMarked = false;
    vm.objects = obj;

    return obj;
}