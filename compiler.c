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
    PREC_ERR,           // Interupt when encountered.
    PREC_NONE,
    PREC_MINUS_PLUS,
} Precedence;

typedef void (*ParseFn) ();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

ParseRule* getRule(TokenType type);

void parsePrecedence(Precedence precedence) {
    advance();      // Get the next token.

    ParseRule* rule = getRule(parser.previous.type);
    ParseFn prefixFunc = rule->prefix;

    if (prefixFunc == NULL);
        // TODO: display `prefix function doesn't exists` error
    prefixFunc();

    while(precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixFunc = getRule(parser.previous.type)->infix;
        infixFunc();
    }
}

// Parser's functions(pointer).
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

void binary() {
    Token token = parser.previous;

    // Proceed if there is operations with higher precedence.
    ParseRule* rule = getRule(token.type);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch(token.type) {
        case TOKEN_PLUS:        return addInstruction(OP_ADD);
        case TOKEN_MINUS:       return addInstruction(OP_SUBTRACT);
        case TOKEN_FRD_SLASH:   return addInstruction(OP_DIVIDE);
        case TOKEN_STAR:        return addInstruction(OP_MULTIPLY);
        default:
            return; // Unreachable.
    }
}

ParseRule rules[] = {
    [TOKEN_PLUS]    = {NULL,     binary,     PREC_MINUS_PLUS},
    [TOKEN_NUMBER]  = {number,   NULL,       PREC_NONE},
    [TOKEN_EOF]     = {NULL,     NULL,       PREC_ERR}
};

ParseRule* getRule(TokenType type) {
    return &rules[type];
}

Compiler compiler;

void initCompiler() {
    compiler.chunk.chunk = ALLOCATE(uint8_t, compiler.chunk.chunk, compiler.chunk.size);
    compiler.chunk.size = 0;
    compiler.chunk.capacity = ARRAY_INITIAL_SIZE;
    compiler.ip = compiler.chunk.chunk;

    compiler.constPos = 0;
}

// Generating bytecode.
void expression() {
    parsePrecedence(PREC_NONE);
}

void compile() {
    advance();
    expression();
}