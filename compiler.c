#include "compiler.h"
#include "memory.h"


// Declarations.
void expression();

// Parser section.
Parser parser;

void advance() {
    parser.previous = parser.current;

    if ((parser.current = scanToken()).type == TOKEN_EOF);
        // TODO: display `EOF or some kind of error` error
}

static void error(Token* token, const char* message) {
    compiler.panicMode = true;      // Bytecode contains error.
    printf("[Line %d] %s: %s\n",
        token->lineNumber,
        (scanner.lineNumber == token->lineNumber) ? "Error at the end": "Error",
        message);
}

static bool consume(TokenType type, const char* message) {
    if (parser.current.type == type) return true;

    error(&parser.current, message);
    return false;
}

void recover() {
    // Ignore all tokens either until semicolon (;) - a.k.a. end of expression,
    // or until EOF - a.k.a. end of file.
    for (;;) {
        advance();
        if (parser.current.type == TOKEN_EOF 
            || parser.current.type == TOKEN_SEMI_COLON)
            break;
    }

    // Start compiling from new expression.
    compiler.ip += compiler.chunk.size - (compiler.ip - compiler.chunk.chunk);
    compiler.panicMode = false;             // Back to normal.
}

typedef enum {
    PREC_NONE,           // Interupt when encountered.
    PREC_LITERAL,
    PREC_MINUS_PLUS,    // -, +
    PREC_MULT_DIV,      // *,/
    PREC_GROUP,         // (expression)
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
        size_t interval = compiler.ip - compiler.chunk.chunk;   // Don't lose required position to continue.

        compiler.chunk.chunk = ALLOCATE(uint8_t, compiler.chunk.chunk, compiler.chunk.size);
        compiler.chunk.capacity *= ENLARGEMENT_FACTOR;
        compiler.ip = compiler.chunk.chunk;
        compiler.ip += interval ? interval - 1: 0;                             // Add interval to reach desired instruction.
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

static void number() {
    Value value;
    value.type = VAL_NUMBER;
    value.as.number = parser.previous.number;
    emitConstant(value);
}

static void unary() {
    advance();
    number();
    addInstruction(OP_NEGATE);
}

static void string() {
    ObjString* obj = (ObjString*)malloc(sizeof(ObjString));
    obj->obj.type = OBJ_STRING;
    obj->value = parser.previous.string;
    // TODO: compute hash.

    Value value;
    value.type = VAL_STRING;
    value.as.obj = AS_OBJ(obj);
    
    addInstructions(OP_CONSTANT, addConstant(value));
}

static void identifier() {
    Value value;
    ObjString* string = (ObjString*)malloc(sizeof(ObjString));
    string->value = parser.previous.string;
    string->hash = hashString(string->value);
    value.type = VAL_STRING;
    value.as.obj = AS_OBJ(string);
    
    switch (parser.current.type) {
        case TOKEN_EQUAL:
            advance();
            expression();
            addInstructions(OP_SET_GLOBAL, addConstant(value));
            break;
        default:
            addInstructions(OP_GET_GLOBAL, addConstant(value));
    }
}

static void literal() {
    switch(parser.previous.type) {
        case TOKEN_STRING:
            string();
            break;
        case TOKEN_NUMBER:
            number();
            break;
        case TOKEN_FALSE:  // FALSE
            addInstruction(OP_FALSE);
            break;
        case TOKEN_TRUE:   // TRUE
            addInstruction(OP_TRUE);
            break;
        case TOKEN_NIL:    // NIL
            addInstruction(OP_NIL);
            break;
        case TOKEN_IDENTIFIER:
            identifier();
            break;
        default:
            return; // Unreachable.
    }
}

static void binary() {
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

static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "')' is required after grouping.");
    advance();
}

ParseRule rules[] = {
    [TOKEN_PLUS]        = {NULL,     binary,     PREC_MINUS_PLUS},
    [TOKEN_MINUS]       = {unary,    binary,     PREC_MINUS_PLUS},
    [TOKEN_STAR]        = {NULL,     binary,     PREC_MULT_DIV},
    [TOKEN_FRD_SLASH]   = {NULL,     binary,     PREC_MULT_DIV},
    [TOKEN_NUMBER]      = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_EOF]         = {NULL,     NULL,       PREC_NONE},
    [TOKEN_LEFT_PAREN]  = {grouping, NULL,       PREC_GROUP},
    [TOKEN_RIGHT_PAREN] = {NULL,     NULL,       PREC_NONE},
    [TOKEN_LEFT_CURLY]  = {NULL,     NULL,       PREC_NONE},
    [TOKEN_RIGHT_CURLY] = {NULL,     NULL,       PREC_NONE},
    [TOKEN_SEMI_COLON]  = {NULL,     NULL,       PREC_NONE},
    [TOKEN_QUESTION]    = {NULL,     NULL,       PREC_NONE}, 
    [TOKEN_COLON]       = {NULL,     NULL,       PREC_NONE},
    [TOKEN_BANG]        = {NULL,     NULL,       PREC_NONE},
    [TOKEN_FALSE]       = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_TRUE]        = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_NIL]         = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_STRING]      = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_IDENTIFIER]  = {literal,  NULL,       PREC_LITERAL}
};

ParseRule* getRule(TokenType type) {
    return &rules[type];
}

Compiler compiler;

void initCompiler() {
    compiler.chunk.chunk = ALLOCATE(uint8_t, compiler.chunk.chunk, compiler.chunk.size);
    compiler.chunk.size = 0;
    compiler.panicMode = false;     // There is no error in bytecode.
    compiler.chunk.capacity = ARRAY_INITIAL_SIZE;
    compiler.ip = compiler.chunk.chunk;

    Table table;
    initTable(&table);

    compiler.constPos = 0;
}

void freeCompiler() {
    free((void*)compiler.chunk.chunk);
}

// Generating bytecode.
void expression() {
    parsePrecedence(PREC_LITERAL);
}

void statement() {
    switch(parser.current.type) {
        case TOKEN_EOF:
            // Terminate.
            break;
        case TOKEN_PRINT:
            advance();
            expression();
            consume(TOKEN_SEMI_COLON, "';' is required after expression.");
            addInstruction(OP_PRINT);
            break;
        default: {
            expression();
            consume(TOKEN_SEMI_COLON, "';' is required after expression.");
        }
    }
}

static void declareVariable() {
    advance();
    if (parser.current.type != TOKEN_IDENTIFIER) {
        error(&parser.current, "Identifier is expected after 'var'.");
        return;
    }
    
    Entry entry;
    ObjString* key = (ObjString*)malloc(sizeof(ObjString));
    key->value = parser.current.string;
    key->hash = hashString(key->value);
    entry.key = key;
    entry.tombstone = false;
    entry.value = NULL;

    if (addEntry(&compiler.table, &entry)) {
        error(&parser.current, "Variable already exists");
        return; // Terminate.
    }

    advance();

    if (parser.current.type == TOKEN_EQUAL) {
        advance();
        expression();
        
        // Add string to constants' table.
        Value value;
        value.type = VAL_STRING;
        value.as.obj = AS_OBJ(key);
        addInstructions(OP_DEFINE_GLOBAL, addConstant(value));
    } else
        consume(TOKEN_SEMI_COLON, "';' is expected after variable declaration.");
}

void declaration() {
    switch(parser.current.type) {
        case TOKEN_VAR:
            declareVariable();
            break;
        default:
            statement();
    }
}

void compile() {
    advance();
    declaration();
}

ObjString* copyString(const char* str) {
    ObjString* strObj;
    strObj->value = str;
    strObj->hash = hashString(strObj->value);

    return internString(strObj);
}