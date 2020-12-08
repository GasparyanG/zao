#ifndef ZAO_VM_H
#define ZAO_VM_H

#include <stdint.h>
#include <stdarg.h>

#include "common.h"
#include "debug.h"
#include "compiler.h"
#include "object.h"
#include "value.h"
#include "table.h"

typedef enum {
    INTERPRETER_RUNTIME_ERROR,
    EXECUTION_SUCCESS
} ExecutionResult;

typedef struct {
    Value stack[UINT8_MAX];
    Value* stackTop;

    Table globals;              // Hash table for variables.
    Value locals[UINT8_MAX];

    Value constants[UINT8_MAX];
    size_t constPos;

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
ExecutionResult run();

#endif