#include "compiler.h"
#include "memory.h"


// Declarations.
void expression();
void declaration();
static int resolveLocal(Token* local);

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
    PREC_NONE,          // Interupt when encountered.
    PREC_LITERAL,
    PREC_ASSIGN,        // a + b = c + d should fail.
    PREC_MINUS_PLUS,    // -, +
    PREC_MULT_DIV,      // *,/
    PREC_GROUP,         // (expression)
} Precedence;

typedef void (*ParseFn) (bool canAssign);

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

    if (prefixFunc == NULL)
        error(&parser.previous, "Wrong prefix operation.");
    
    bool canAssign = precedence <= PREC_ASSIGN;
    prefixFunc(canAssign);

    while(precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixFunc = getRule(parser.previous.type)->infix;
        infixFunc(canAssign);
    }

    if (parser.current.type == TOKEN_EQUAL) {
        error(&parser.current, "Wrong assignement expression.");
        advance();
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
        compiler.ip += interval;                                // Add interval to reach desired instruction.
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

static void number(bool canAssign) {
    Value value;
    value.type = VAL_NUMBER;
    value.as.number = parser.previous.number;
    emitConstant(value);
}

static void unary(bool canAssign) {
    advance();
    number(canAssign);
    addInstruction(OP_NEGATE);
}

static void string(bool canAssign) {
    Value value;
    value.type = VAL_STRING;
    value.as.obj = AS_OBJ(copyString(parser.previous.string));
    
    addInstructions(OP_CONSTANT, addConstant(value));
}

static void identifier(bool canAssign) {
    int position;       // Instruction.
    int resLocPos;      // Temporary local.

    Value value;
    value.type = VAL_STRING;
    value.as.obj = AS_OBJ(copyString(parser.previous.string));
    position = addConstant(value);

    OpCode opGet = OP_GET_GLOBAL;
    OpCode opSet = OP_SET_GLOBAL;
    if (compiler.scopeDepth > 0 && 
    (resLocPos = resolveLocal(&parser.previous)) >= 0) {
        opGet = OP_GET_LOCAL;
        opSet = OP_SET_LOCAL;
        position = resLocPos;
    }

    if (parser.current.type == TOKEN_EQUAL) {
        if (!canAssign)
                return;     // Leave = in current position to throw an error.
        advance();
        expression();
        addInstructions(opSet, (uint8_t)position);
    } else {
        addInstructions(opGet, (uint8_t)position);
    }
}

static void literal(bool canAssign) {
    switch(parser.previous.type) {
        case TOKEN_STRING:
            string(canAssign);
            break;
        case TOKEN_NUMBER:
            number(canAssign);
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
            identifier(canAssign);
            break;
        default:
            return; // Unreachable.
    }
}

static void binary(bool canAssign) {
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

static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "')' is required after grouping.");
    advance();
}

static void scopeStart() {
    compiler.scopeDepth++;
}

static void scopeEnd() {
    compiler.scopeDepth--;

    while (compiler.scopeDepth < compiler.locals[compiler.localsCount].scopeDepth)
        compiler.localsCount--;
    
    compiler.localsCount++; // Point to one past the end location.
}

static void block(bool canAssign) {
    scopeStart();
    advance();
    
    while (parser.current.type != TOKEN_RIGHT_CURLY) {
        if (parser.current.type == TOKEN_SEMI_COLON) {
            advance();  // Ignore semicolon (;) and left curly (}).
            continue;
        }
        
        declaration();
    }

    consume(TOKEN_RIGHT_CURLY, "'}' is required at the end of block.");
    advance();
    scopeEnd();
}

ParseRule rules[] = {
    [TOKEN_PLUS]            = {NULL,     binary,     PREC_MINUS_PLUS},
    [TOKEN_MINUS]           = {unary,    binary,     PREC_MINUS_PLUS},
    [TOKEN_STAR]            = {NULL,     binary,     PREC_MULT_DIV},
    [TOKEN_FRD_SLASH]       = {NULL,     binary,     PREC_MULT_DIV},
    [TOKEN_NUMBER]          = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_EOF]             = {NULL,     NULL,       PREC_NONE},
    [TOKEN_LEFT_PAREN]      = {grouping, NULL,       PREC_GROUP},
    [TOKEN_RIGHT_PAREN]     = {NULL,     NULL,       PREC_NONE},
    [TOKEN_LEFT_CURLY]      = {block,    NULL,       PREC_NONE},
    [TOKEN_RIGHT_CURLY]     = {NULL,     NULL,       PREC_NONE},
    [TOKEN_SEMI_COLON]      = {NULL,     NULL,       PREC_NONE},
    [TOKEN_QUESTION]        = {NULL,     NULL,       PREC_NONE}, 
    [TOKEN_COLON]           = {NULL,     NULL,       PREC_NONE},
    [TOKEN_BANG]            = {NULL,     NULL,       PREC_NONE},
    [TOKEN_FALSE]           = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_TRUE]            = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_NIL]             = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_STRING]          = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_IDENTIFIER]      = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_MODULO]          = {literal,  NULL,       PREC_LITERAL}, 
    [TOKEN_BCK_SLASH]       = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_GREATER_THAN]    = {literal,  NULL,       PREC_LITERAL}, 
    [TOKEN_LESS_THAN]       = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_EQUAL]           = {literal,  NULL,       PREC_ASSIGN}
};

ParseRule* getRule(TokenType type) {
    return &rules[type];
}

Compiler compiler;

void initCompiler() {
    compiler.chunk.chunk = ALLOCATE(uint8_t, compiler.chunk.chunk, compiler.chunk.size);
    compiler.chunk.size = 0;
    compiler.panicMode = false;     // There is no error in bytecode.
    compiler.localsCount = 0;
    compiler.scopeDepth = 0;
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
    parsePrecedence(PREC_ASSIGN);
}

void statement() {
    switch(parser.current.type) {
        case TOKEN_EOF:
            // Terminate.
            break;
        case TOKEN_LEFT_CURLY: 
            block(true);
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

static int resolveLocal(Token* local) {
    for (int i = compiler.localsCount - 1; i >= 0; i--) {
        if (strcmp(local->string, compiler.locals[i].name.string) == 0)
            return i;   // Found at index.
    }

    return -1;
}

static int isDeclared(Token* local, size_t depth) {
    for (int i = compiler.localsCount - 1; i >= 0; i--) {
        if (depth == compiler.locals[i].scopeDepth &&
            strcmp(local->string, compiler.locals[i].name.string) == 0)
            return i;   // Found at index.
    }

    return -1;
}

static void declareLocalVariable() {
    int position;
    if ((position = isDeclared(&parser.current, compiler.scopeDepth)) > 0) {
        error(&parser.current, "Double declaration.");
    } else {
        Local* local = &compiler.locals[compiler.localsCount];
        local->name = parser.current;
        local->scopeDepth = compiler.scopeDepth;

        position = compiler.localsCount++;
    }


    advance();      // Consume identifier.
    advance();      // Consume = token as well.

    // TODO: implement declaration as well (this is just initialization).
    expression();   // Semicolon (;) is being consumed in this function.

    addInstructions(OP_SET_LOCAL, (uint8_t)position);
}

static void declareVariable() {
    advance();
    if (parser.current.type != TOKEN_IDENTIFIER) {
        error(&parser.current, "Identifier is expected after 'var'.");
        return;
    }

    if (compiler.scopeDepth > 0)
        return declareLocalVariable(); // Return just meant for termination.
    
    Entry entry;
    entry.key = copyString(parser.current.string);
    entry.tombstone = false;

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
        value.as.obj = AS_OBJ(entry.key);
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
    ObjString* strObj = (ObjString*)malloc(sizeof(ObjString));
    strObj->value = str;
    strObj->hash = hashString(strObj->value);

    return internString(strObj);
}