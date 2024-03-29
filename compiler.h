#ifndef ZAO_COMPILER_H
#define ZAO_COMPILER_H

#include <stdint.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "object.h"
#include "scanner.h"
#include "table.h"
#include "value.h"

typedef struct {
    Token previous;
    Token current;
} Parser;

extern Parser parser;

typedef struct {
    Token name;
    int scopeDepth;
} Local;

typedef struct Compiler {
    ObjFunction* function;
    Compiler* enclosedCompiler;

    Local locals[UINT8_MAX];
    size_t localsCount;
    int scopeDepth;

    bool panicMode;
    bool isInit;
} Compiler;

void initCompiler(ObjFunction* function, bool isInit);
void freeCompiler(Compiler* cmpl);

uint8_t addConstant(Value value);
void addInstruction(uint8_t instruction);
void advance();

extern Compiler* compiler;

// Generating bytecode.
void compile(bool cmd);

ObjString* copyString(const char* str);
#include "vm.h"

#endif