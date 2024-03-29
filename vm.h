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

typedef struct CallFrame {
    CallFrame* nextFrame;
    Value* functionLocals;
    ObjClosure* closure;
    uint8_t position;
    uint8_t* ip;
} CallFrame;

typedef struct {
    Value stack[UINT8_MAX];
    Value* stackTop;

    CallFrame* callFrame;

    Table globals;              // Hash table for variables.
    Value locals[UINT8_MAX];

    ObjString* internedStrings[UINT8_MAX];
    size_t stringCount;

    Obj* objects;
    size_t amountOfObjs;

    size_t nextGC;

    Obj** greyStack;
    size_t greyCount;
    size_t greyCapacity;
} VM;

extern VM vm;

// String operations.
ObjString* internString(ObjString* strToCmp);

// VM operations.
void initVM();
void freeVM();

CallFrame* initCallFrame();
void freeCallFrame(CallFrame* callFrame);

// Stack operations.
Value* pop();
void push(Value* value);

// Interpreter operations.
ExecutionResult run();

#endif