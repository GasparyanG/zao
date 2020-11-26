#include "compiler.h"
#include "memory.h"


// Parser section.
typedef struct {
    Token previous;
    Token current;
} Parser;

Parser parser;

static void advance() {
    parser.previous = parser.current;

    if ((parser.current = scanToken()).type == TOKEN_EOF);
        // TODO: display `EOF or some kind of error` error
}

typedef enum {
    PREC_NONE
} Precedence;

typedef void (*ParseFn) ();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;


// Parser functions.
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

void addInstructions(uint8_t inst1, uint8_t inst2) {
    addInstruction(inst1);
    addInstruction(inst2);
}

void emitConstant(Value value) {
    addInstructions(OP_CONSTANT, addConstant(value));
}

void number() {
    emitConstant(parser.previous.number);
}


ParseRule rules[] = {
    [TOKEN_NUMBER] = {number, NULL, PREC_NONE}
};


Compiler compiler;

void initCompiler() {
    compiler.chunk.chunk = ALLOCATE(uint8_t, compiler.chunk.chunk, compiler.chunk.size);
    compiler.chunk.size = 0;
    compiler.chunk.capacity = ARRAY_INITIAL_SIZE;
    compiler.ip = compiler.chunk.chunk;

    compiler.constPos = 0;
}


// Generating bytecode.
void compile() {
    for (;;) {
        advance();

#ifdef ZAO_TOKEN_DEBUGGER
    displayToken(&parser.previous);
    displayToken(&parser.current);
#endif
    }
}