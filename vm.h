#ifndef ZAO_VM_H
#define ZAO_VM_H

#include <stdint.h>

#include "common.h"
#include "debug.h"
#include "compiler.h"
#include "object.h"
#include "value.h"
#include "table.h"

typedef struct {
    Value stack[UINT8_MAX];
    Value* stackTop;

    ObjString* internedStrings[UINT8_MAX];
    size_t stringCount;
} VM;

extern VM vm;

// String operations.
ObjString* internString(ObjString* strToCmp);

// VM operations.
void initVM();

// Stack operations.
Value* pop();
void push(Value* value);

// Interpreter operations.
void run();

#endif