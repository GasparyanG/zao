#include "compiler.h"
#include "memory.h"

Compiler compiler;

void initCompiler() {
    compiler.chunk.chunk = ALLOCATE(uint8_t, compiler.chunk.chunk, compiler.chunk.size);
    compiler.chunk.size = 0;
    compiler.chunk.capacity = ARRAY_INITIAL_SIZE;
    compiler.ip = compiler.chunk.chunk;

    compiler.constPos = 0;
}

uint8_t addConstant(const Value value) {
    if (compiler.constPos == UINT8_MAX);
        // TODO: display `constants table overflow` error.
    compiler.constants[compiler.constPos] = value;

    return compiler.constPos++;
}

void addInstruction(uint8_t instruction) {
    if (compiler.chunk.size == compiler.chunk.capacity) {
        compiler.chunk.chunk = ALLOCATE(uint8_t, compiler.chunk.chunk, compiler.chunk.size);
        compiler.chunk.capacity *= ENLARGEMENT_FACTOR;
        compiler.ip = compiler.chunk.chunk;
    }

    compiler.chunk.chunk[compiler.chunk.size++] = instruction;
}


// Generating bytecode.
void compile() {
    for (;;) {
        // TODO: parse tokens!
    }
}