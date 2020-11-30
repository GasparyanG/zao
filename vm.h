#ifndef ZAO_VM_H
#define ZAO_VM_H

#include <stdint.h>

#include "common.h"
#include "debug.h"
#include "compiler.h"
#include "value.h"
#include "table.h"

typedef struct {
    Value stack[UINT8_MAX];
    Value* stackTop;
} VM;

extern VM vm;

// VM operations.
void initVM();

// Stack operations.
Value* pop();
void push(Value* value);

// Interpreter operations.
void run();

#endif