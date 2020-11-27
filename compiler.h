#ifndef ZAO_COMPILER_H
#define ZAO_COMPILER_H

#include <stdint.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "scanner.h"
#include "value.h"

typedef struct {
    Token previous;
    Token current;
} Parser;

extern Parser parser;

typedef struct {
    Chunk chunk;
    uint8_t* ip;
    Value constants[UINT8_MAX];

    bool panicMode;

    size_t constPos;
} Compiler;

void initCompiler();
void freeCompiler();

uint8_t addConstant(Value value);
void addInstruction(uint8_t instruction);
void advance();
void recover();     // Recover from error state.

extern Compiler compiler;

// Generating bytecode.
void compile();

#endif