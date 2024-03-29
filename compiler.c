#include "compiler.h"
#include "memory.h"


// Declarations.
void expression();
void statement();
static void declareVariable();
void declaration();
static int resolveLocal(Compiler* cmpl, Token* local);
static int resolveUpvalue(Compiler* cmpl, Token* local);
Value prepareValue(Obj* obj, ValueType type);


// Class compiler section.
typedef struct {
    ObjClass* objClass;
} ClassCompiler;

ClassCompiler classCompiler;

void initClassCompiler() {
    classCompiler.objClass = NULL;
}

// Parser section.
Parser parser;

// Currently processed/parsed function's compiler.
Compiler* current = NULL;

void advance() {
    parser.previous = parser.current;
    parser.current = scanToken();
}

static void error(Token* token, const char* message) {
    compiler->panicMode = true;      // Bytecode contains error.
    fprintf(stderr, "[Line %d] %s: %s\n",
        token->lineNumber,
        (scanner.lineNumber == token->lineNumber) ? "Error at the end": "Error",
        message);
}

static bool consume(TokenType type, const char* message) {
    if (parser.current.type == type) return true;

    error(&parser.current, message);
    return false;
}

static void synchronize() {
    if (!compiler->panicMode) return;
    // Ignore all tokens either until semicolon (;) - a.k.a. end of expression,
    // or until EOF - a.k.a. end of file.
    for (;;) {
        advance();
        if (parser.current.type == TOKEN_EOF 
            || parser.current.type == TOKEN_SEMI_COLON) {
            advance();
            break;
        }
    }

    // Start compiling from new expression.
    compiler->function->ip 
        += compiler->function->chunk.size - (compiler->function->ip - compiler->function->chunk.chunk);
    compiler->panicMode = false;             // Back to normal.
}

typedef enum {
    PREC_NONE,          // Interupt when encountered.
    PREC_LITERAL,
    PREC_ASSIGN,        // a + b = c + d should fail.
    PREC_INVOKE,        // .
    PREC_OR,            // or
    PREC_AND,           // and
    PREC_COMPARISION,   // >, <, ==
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
    if (compiler->function->constPos == UINT8_MAX)
        fatalError("Constant table overflow.");    

    compiler->function->constants[compiler->function->constPos] = value;

    return compiler->function->constPos++;
}

void addInstruction(uint8_t instruction) {
    if (compiler->function->chunk.size == compiler->function->chunk.capacity) {
        size_t interval = compiler->function->ip - compiler->function->chunk.chunk;   // Don't lose required position to continue.

        compiler->function->chunk.chunk 
            = ALLOCATE(uint8_t, compiler->function->chunk.chunk, compiler->function->chunk.size);
        compiler->function->chunk.lines 
            = ALLOCATE(uint16_t, compiler->function->chunk.lines, compiler->function->chunk.size);
        compiler->function->chunk.capacity *= ENLARGEMENT_FACTOR;
        compiler->function->ip = compiler->function->chunk.chunk;
        compiler->function->ip += interval;                                // Add interval to reach desired instruction.
    }

    compiler->function->chunk.chunk[compiler->function->chunk.size++] = instruction;
    compiler->function->chunk.lines[compiler->function->chunk.size - 1] = parser.current.lineNumber;
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
    switch(parser.previous.type) {
        case TOKEN_BANG:
            expression();
            addInstruction(OP_BANG);
            break;
        case TOKEN_MINUS:
            advance();
            number(canAssign);
            addInstruction(OP_NEGATE);
            break;
        default:
            return;     // Unreachable.
    }
}

static void string(bool canAssign) {
    Value value;
    value.type = VAL_STRING;
    value.as.obj = AS_OBJ(copyString(parser.previous.string));
    
    addInstructions(OP_CONSTANT, addConstant(value));
}

static void identifier(bool canAssign) {
    int position;       // Instruction.
    int resLocPos;      // Temporary local - this will hold value for `position`.

    OpCode opGet = OP_GET_GLOBAL;
    OpCode opSet = OP_SET_GLOBAL;
    if (compiler->scopeDepth > 0 && 
    (resLocPos = resolveLocal(compiler, &parser.previous)) >= 0) {
        opGet = OP_GET_LOCAL;
        opSet = OP_SET_LOCAL;
        position = resLocPos;
    } else if (compiler->scopeDepth > 0 &&
    (resLocPos = resolveUpvalue(compiler, &parser.previous)) >= 0) {
        opGet = OP_GET_UPVALUE;
        opSet = OP_SET_UPVALUE;
        position = resLocPos;
    } else {
        Value value;
        value.type = VAL_STRING;
        value.as.obj = AS_OBJ(copyString(parser.previous.string));
        position = addConstant(value);
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
        case TOKEN_THIS: {
            if (classCompiler.objClass == NULL)
                error(&parser.previous, "'this' keyword can't be used outside of a class.");
            addInstruction(OP_THIS);
            break;
        }
        case TOKEN_SUPER: {
            if (classCompiler.objClass == NULL)
                error(&parser.previous, "'super' keyword can't be used outside of a class.");
            addInstruction(OP_SUPER);
            break;
        }
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
        case TOKEN_PLUS:            return addInstruction(OP_ADD);
        case TOKEN_MINUS:           return addInstruction(OP_SUBTRACT);
        case TOKEN_FRD_SLASH:       return addInstruction(OP_DIVIDE);
        case TOKEN_STAR:            return addInstruction(OP_MULTIPLY);
        case TOKEN_GREATER_THAN:    return addInstruction(OP_GREATER_THAN);
        case TOKEN_LESS_THAN:       return addInstruction(OP_LESS_THAN);
        case TOKEN_EQUAL_EQUAL:     return addInstruction(OP_EQUAL_EQUAL);
        case TOKEN_GREATER_EQUAL:   return addInstructions(OP_LESS_THAN, OP_BANG);
        case TOKEN_LESS_EQUAL:      return addInstructions(OP_GREATER_THAN, OP_BANG);
        case TOKEN_BANG_EQUAL:      return addInstructions(OP_EQUAL_EQUAL, OP_BANG);
        default:
            return; // Unreachable.
    }
}

static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "')' is required after grouping.");
    advance();
}

static void call(bool canAssign) {
    uint8_t numOfArgs = 0;
    for (;;) {
        if (parser.current.type == TOKEN_RIGHT_PAREN) {
            advance();
            break;
        }
        expression();
        numOfArgs++;
        if (parser.current.type == TOKEN_COMMA)
            advance();
        else if (parser.current.type == TOKEN_RIGHT_PAREN) {
            advance();
            break;
        }
    }

    addInstructions(OP_CALL, numOfArgs);
}

static void dot(bool canAssign) {
    consume(TOKEN_IDENTIFIER, "Identifier is expected after '.'.");

    ObjString* string = copyString(parser.current.string);
    Value value = prepareValue(AS_OBJ(string), VAL_STRING);

    advance();
    switch(parser.current.type) {
        case TOKEN_EQUAL: {
            advance();
            expression();
            addInstructions(OP_SET_PROPERTY, addConstant(value));
            break;
        }

        case TOKEN_SEMI_COLON: {
            addInstructions(OP_GET_PROPERTY, addConstant(value));
            break;
        }

        case TOKEN_DOT: {
            addInstructions(OP_GET_PROPERTY, addConstant(value));
            advance();
            dot(true);
            break;
        }

        case TOKEN_LEFT_PAREN: {
            addInstructions(OP_INVOKE, addConstant(value));
            advance();
            call(true);
            break;
        }
    }
}

static void scopeStart() {
    compiler->scopeDepth++;
}

static void scopeEnd() {
    compiler->scopeDepth--;

    while (compiler->localsCount && 
        compiler->scopeDepth < compiler->locals[compiler->localsCount - 1].scopeDepth) {
        compiler->localsCount--;
    }
}

static void scopeSimpleEnd() {
    compiler->scopeDepth--;
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
    scopeEnd();
    advance();
}

static void addSizeToJumpPos(uint8_t currentPosition, uint16_t jumpingSize) {
    // TODO: why does 0 being converted to 8 ?
    if (jumpingSize == 0) {
        compiler->function->chunk.chunk[currentPosition] = (uint8_t)(jumpingSize);
        compiler->function->chunk.chunk[currentPosition + 1] = (uint8_t)(jumpingSize);
    } else {
        compiler->function->chunk.chunk[currentPosition] = (uint8_t)((8 >> jumpingSize) & 0xff);
        compiler->function->chunk.chunk[currentPosition + 1] = (uint8_t)(jumpingSize & 0xff);
    }
}

static void or_(bool canAssign) {
    addInstruction(OP_JUMP_IF_TRUE);
    size_t currentPosition = compiler->function->chunk.size;   // Position to insert jumping size.
    addInstructions(0x00, 0x00);

    parsePrecedence(PREC_OR);

    uint16_t jumpingSize = compiler->function->chunk.size - currentPosition - JUMP_BYTES;
    addSizeToJumpPos(currentPosition, jumpingSize);

    addInstruction(OP_OR);
}

static void and_(bool canAssign) {
    addInstruction(OP_JUMP);
    size_t currentPosition = compiler->function->chunk.size;   // Position to insert jumping size.
    addInstructions(0x00, 0x00);

    parsePrecedence(PREC_AND);

    uint16_t jumpingSize = compiler->function->chunk.size - currentPosition - JUMP_BYTES;
    addSizeToJumpPos(currentPosition, jumpingSize);

    addInstruction(OP_AND);
}

static size_t emitJump(OpCode jumpCode) {
    addInstruction(jumpCode);
    size_t currentPosition = compiler->function->chunk.size;   // Position to insert jumping size.
    addInstructions(0x00, 0x00);

    return currentPosition;
}

static void patchJump(size_t position) {
    // Add jumping size in bytecode, next to OP_JUMP instruction.
    uint16_t jumpingSize = compiler->function->chunk.size - position - JUMP_BYTES - 1;
    addSizeToJumpPos(position, jumpingSize);   
}

static void else_(bool canAssign) {
    advance();
    statement();
}

static void if_(bool canAssign) {
    advance();
    consume(TOKEN_LEFT_PAREN, "'(' is required after 'if' keyword.");
    expression();   // Prepare bytecode for bool expression.
    
    size_t thenJump = emitJump(OP_JUMP);
    addInstruction(OP_POP);

    statement();

    size_t elseJump = emitJump(OP_JUST_JUMP);
    patchJump(thenJump);
    addInstruction(OP_POP);

    if (parser.current.type == TOKEN_ELSE)
        else_(canAssign);
    patchJump(elseJump);
}

// for statement section.
static void forDecl() {
    advance();
    switch(parser.current.type) {
        case TOKEN_SEMI_COLON:
            break;
        case TOKEN_VAR:
            declareVariable();
            break;
        default:
            error(&parser.current, "Either initialize variable or leave it empty.");
    }
}

static void forExpr() {
    advance();
    switch(parser.current.type) {
        case TOKEN_SEMI_COLON:
            addInstruction(OP_TRUE);    // Always evaluate to true.
            break;
        default:
            expression();
            consume(TOKEN_SEMI_COLON, "';' is required after expression.");
    }
}

static void forAssign() {
    advance();
    switch(parser.current.type) {
        case TOKEN_RIGHT_PAREN:
            break;
        default:
            expression();
    }

    consume(TOKEN_RIGHT_PAREN, "')' is required after 'for' assignement.");
    advance();
}

static void for_(bool canAssign) {
    advance();
    scopeStart();       // Declared variable should be in a scope.

    // Variable declaration/initialization section.
    consume(TOKEN_LEFT_PAREN, "'(' is required after 'if' keyword.");
    forDecl();          // In this section we can either declare vaiable or just leave it empty.

    // Expression section.
    size_t exprPos = compiler->function->chunk.size;
    forExpr();          // In this section we can either wrtire expression or just leave it empty.
    addInstruction(OP_JUMP_FOR);
    size_t exprJumpPos = compiler->function->chunk.size;
    addInstructions(0x00, 0x00);    // Jump over instruction right into block.
    addInstructions(0x00, 0x00);    // Jump out of 'for' statement.

    // Assignement section.
    size_t assignPos = compiler->function->chunk.size;
    forAssign();        // In this section we can either make assignement or just leave it empty.

    // Jump back to expression.
    addInstruction(OP_JUMP_BACK);
    size_t assignJumpPos = compiler->function->chunk.size;
    addInstructions(0x00, 0x00);
    addSizeToJumpPos(compiler->function->chunk.size - JUMP_BYTES, exprPos);

    // Set distance from expression to block.
    addSizeToJumpPos(exprJumpPos, compiler->function->chunk.size - exprJumpPos - JUMP_BYTES);

    // Block statement.
    scopeSimpleEnd();           // This scope will be continued in `statement()`.
    addInstruction(OP_POP);     // Remove bool from stack.
    statement();

    // Go back to assignement, after which in its case will dispatch to expression.
    addInstruction(OP_JUMP_BACK);
    addInstructions(0x00, 0x00);
    addSizeToJumpPos(compiler->function->chunk.size - JUMP_BYTES, assignPos);

    // Set distance from expression to out of 'for' statement, to be able to leave the loop.
    // TODO: why does 1 is subtracted ?
    addSizeToJumpPos(exprJumpPos + JUMP_BYTES, compiler->function->chunk.size - exprJumpPos - (2 * JUMP_BYTES) - 1);
    addInstruction(OP_POP);
}

static void while_(bool canAssign) {
    advance();
    consume(TOKEN_LEFT_PAREN, "'(' is required after 'while' keyword.");

    size_t exprPos = compiler->function->chunk.size;           // Position to jump back after loop.
    expression();                                   // Prepare bytecode for bool expression.

    addInstruction(OP_JUMP);
    size_t currentPosition = compiler->function->chunk.size;   // Position to insert jumping size.
    addInstructions(0x00, 0x00);
    
    statement();                                    // Add bytecode for block statement.
    addInstruction(OP_POP);                         // Remove bool after every loop.

    // Go Back to expression, to see whether to loop again or not.
    addInstruction(OP_JUMP_BACK);
    addInstructions(0x00, 0x00);
    addSizeToJumpPos(compiler->function->chunk.size - JUMP_BYTES, exprPos);

    // Add jumping size in bytecode, next to OP_JUMP instruction.
    // TODO: why does 1 is subtracted ?
    uint16_t jumpingSize = compiler->function->chunk.size - currentPosition - JUMP_BYTES - 1;
    addSizeToJumpPos(currentPosition, jumpingSize);

    addInstruction(OP_POP);                         // Remove last bool from stack.
}

ParseRule rules[] = {
    [TOKEN_PLUS]            = {NULL,     binary,     PREC_MINUS_PLUS},
    [TOKEN_MINUS]           = {unary,    binary,     PREC_MINUS_PLUS},
    [TOKEN_STAR]            = {NULL,     binary,     PREC_MULT_DIV},
    [TOKEN_FRD_SLASH]       = {NULL,     binary,     PREC_MULT_DIV},
    [TOKEN_NUMBER]          = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_EOF]             = {NULL,     NULL,       PREC_NONE},
    [TOKEN_LEFT_PAREN]      = {grouping, call,       PREC_GROUP},
    [TOKEN_RIGHT_PAREN]     = {NULL,     NULL,       PREC_NONE},
    [TOKEN_LEFT_CURLY]      = {block,    NULL,       PREC_NONE},
    [TOKEN_RIGHT_CURLY]     = {NULL,     NULL,       PREC_NONE},
    [TOKEN_COMMA]           = {NULL,     NULL,       PREC_NONE},
    [TOKEN_SEMI_COLON]      = {NULL,     NULL,       PREC_NONE},
    [TOKEN_DOT]             = {NULL,     dot,        PREC_INVOKE}, 
    [TOKEN_COLON]           = {NULL,     NULL,       PREC_NONE},
    [TOKEN_BANG]            = {unary,    NULL,       PREC_NONE},
    [TOKEN_FALSE]           = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_TRUE]            = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_NIL]             = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_STRING]          = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_IDENTIFIER]      = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_MODULO]          = {literal,  NULL,       PREC_LITERAL}, 
    [TOKEN_BCK_SLASH]       = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_GREATER_THAN]    = {NULL,     binary,     PREC_COMPARISION}, 
    [TOKEN_LESS_THAN]       = {NULL,     binary,     PREC_COMPARISION},
    [TOKEN_EQUAL]           = {literal,  NULL,       PREC_ASSIGN},
    [TOKEN_EQUAL_EQUAL]     = {NULL,     binary,     PREC_COMPARISION},
    [TOKEN_GREATER_EQUAL]   = {NULL,     binary,     PREC_COMPARISION},
    [TOKEN_LESS_EQUAL]      = {NULL,     binary,     PREC_COMPARISION},
    [TOKEN_BANG_EQUAL]      = {NULL,     binary,     PREC_COMPARISION},
    [TOKEN_AND]             = {NULL,     and_,       PREC_AND},
    [TOKEN_OR]              = {NULL,     or_,        PREC_OR},
    [TOKEN_IF]              = {if_,      NULL,       PREC_NONE}, 
    [TOKEN_ELSE]            = {else_,    NULL,       PREC_NONE},
    [TOKEN_FOR]             = {for_,     NULL,       PREC_NONE},
    [TOKEN_WHILE]           = {while_,   NULL,       PREC_NONE},
    [TOKEN_THIS]            = {literal,  NULL,       PREC_LITERAL},
    [TOKEN_SUPER]           = {literal,  NULL,       PREC_LITERAL},
};

ParseRule* getRule(TokenType type) {
    return &rules[type];
}

// Compiler section.
Compiler* compiler = NULL;

static void initFunctionUpvalues(ObjFunction* function) {
    function->upvaluesCount = 0;
    for (uint8_t i = 0; i < UINT8_MAX; i++)
        function->upvalues[i] = NULL;
}

static void initChunk(ObjFunction* function) {
    function->chunk.size = 0;
    function->chunk.chunk 
        = ALLOCATE(uint8_t, function->chunk.chunk, function->chunk.size);
    function->chunk.lines
        = ALLOCATE(uint16_t, function->chunk.lines, function->chunk.size);
    function->chunk.size = 0;
    function->chunk.capacity = ARRAY_INITIAL_SIZE;
}

static void initFunction(ObjFunction* function) {
    initChunk(function);
    function->ip = function->chunk.chunk;
    function->arity = 0;
    function->constPos = 0;
}

void initCompiler(ObjFunction* function, bool isInit) {
    Compiler* comp = (Compiler*)malloc(sizeof(Compiler));

    comp->panicMode = false;     // There is no error in bytecode.
    comp->localsCount = 1;       // Function offset.
    comp->scopeDepth = compiler == NULL ? 0: compiler->scopeDepth;
    comp->isInit = isInit;

    // Function section.
    initFunction(function);

    initFunctionUpvalues(function);
    comp->function = function;

    // Compilers' chain.
    if (compiler == NULL) {
        compiler = comp;
        compiler->enclosedCompiler = NULL;
    } else {
        comp->enclosedCompiler = compiler;
        compiler = comp;
        // From now and until global 'compiler' change
        // all compiler's related operations will be applied on new one.
    }
}

void freeCompiler(Compiler* cmpl) {
    free(cmpl);
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
            advance();
            addInstruction(OP_PRINT);
            break;
        case TOKEN_RETURN:
            advance();
            expression();
            consume(TOKEN_SEMI_COLON, "';' is required after expression.");
            advance();
            addInstructions(OP_RETURN, OP_CLOSE);
            break;
        case TOKEN_IF:
            if_(true);
            break;
        case TOKEN_FOR:
            for_(true);
            break;
        case TOKEN_WHILE:
            while_(true);
            break;
        default: {
            expression();
            consume(TOKEN_SEMI_COLON, "';' is required after expression.");
            advance();
        }
    }
}

static int resolveLocal(Compiler* cmpl, Token* local) {
    if (cmpl == NULL) return -1;
    for (int i = cmpl->localsCount - 1; i > 0; i--) {
        if (strcmp(local->string, cmpl->locals[i].name.string) == 0)
            return i;   // Found at index.
    }

    return -1;
}

static int addUpvalue(Compiler* cmpl, uint8_t index, bool isLocal) {
    for (uint8_t i = 0; i < cmpl->function->upvaluesCount; i++) {
        if (cmpl->function->upvalues[i] != NULL 
            && cmpl->function->upvalues[i]->index == index)
            return i;
    }

    UpValue* upvalue = (UpValue*)malloc(sizeof(UpValue));

    upvalue->isLocal = isLocal;
    upvalue->index = index;
    cmpl->function->upvalues[cmpl->function->upvaluesCount] = upvalue;
    return cmpl->function->upvaluesCount++;
} 

static int resolveUpvalue(Compiler* cmpl, Token* local) {
    if (cmpl == NULL) return -1;

    int index = resolveLocal(cmpl->enclosedCompiler, local);
    if (index >= 0)
        return addUpvalue(cmpl, (uint8_t)index, true);

    index = resolveUpvalue(cmpl->enclosedCompiler, local);
    if (index >= 0)
        return addUpvalue(cmpl, (uint8_t)index, false);

    return -1;
}


// Declaration (grammer) section.
static int isDeclared(Token* local, size_t depth) {
    for (int i = compiler->localsCount - 1; i > 0; i--) {
        if (depth == compiler->locals[i].scopeDepth &&
            strcmp(local->string, compiler->locals[i].name.string) == 0)
            return i;   // Found at index.
    }

    return -1;
}

static void declareLocalVariable() {
    int position;
    if ((position = isDeclared(&parser.current, compiler->scopeDepth)) > 0) {
        error(&parser.current, "Double declaration.");
    } else {
        Local* local = &compiler->locals[compiler->localsCount];
        local->name = parser.current;
        local->scopeDepth = compiler->scopeDepth;

        position = compiler->localsCount++;
    }


    advance();          // Consume identifier.
    if (parser.current.type == TOKEN_EQUAL) {
        advance();      // Consume = token as well.
        expression();   // Semicolon (;) is being consumed in this function.
    }

    addInstructions(OP_SET_LOCAL, (uint8_t)position);
}

Value prepareValue(Obj* obj, ValueType type) {
    Value value;
    value.type = type;
    value.as.obj = obj;
    return value;
}

static void declareVariable() {
    advance();
    if (parser.current.type != TOKEN_IDENTIFIER) {
        error(&parser.current, "Identifier is expected after 'var'.");
        return;
    }

    if (compiler->scopeDepth > 0)
        return declareLocalVariable(); // Return just meant for termination.
    
    Entry entry;
    entry.key = copyString(parser.current.string);
    entry.tombstone = false;

    if (addEntry(&vm.globals, &entry)) {
        error(&parser.current, "Variable already exists");
        return; // Terminate.
    }

    advance();

    if (parser.current.type == TOKEN_EQUAL) {
        advance();
        expression();
        
        // Add string to constants' table.
        Value value = prepareValue(AS_OBJ(entry.key), VAL_STRING);
        addInstructions(OP_DEFINE_GLOBAL, addConstant(value));
    }
    
    consume(TOKEN_SEMI_COLON, "';' is expected after variable declaration.");
    advance();
}

static void endFunction() {
    // This condition can be avoided, because first compiler isn't 
    // defined through 'declareFunction()', but extra caution will not hurt.
    if (compiler->enclosedCompiler != NULL) {
        if (classCompiler.objClass && compiler->isInit) {
            addInstruction(OP_THIS);                // Return instance.
            addInstructions(OP_RETURN, OP_CLOSE);   // Implicit return.
        }
            
        Compiler* compilerToFree = compiler;
        compiler = compiler->enclosedCompiler;  // Get back to previous function.
        freeCompiler(compilerToFree);
    }
}

static void argumentList() {
    consume(TOKEN_LEFT_PAREN, "argument list is required after function name.");

    for (;;) {
        advance();
        if (parser.current.type == TOKEN_RIGHT_PAREN) return;
        declareLocalVariable();
        compiler->function->arity++;    // increment amount of arguments by one.
        if (parser.current.type == TOKEN_RIGHT_PAREN) return;
        consume(TOKEN_COMMA, "',' is required after every argument.");
    }
}

static void declareFunctionName(ObjFunction* function) {
    advance();
    if (parser.current.type != TOKEN_IDENTIFIER) {
        error(&parser.current, "Identifier is expected after 'fun'.");
        return;
    }

    if (compiler->scopeDepth > 0)
        return declareLocalVariable(); // Return just meant for termination.

    Entry entry;
    entry.key = copyString(parser.current.string);
    entry.tombstone = false;

    if (classCompiler.objClass != NULL) {
        if (addEntry(&classCompiler.objClass->methods, &entry)) {
            error(&parser.current, "Method name already exists");
            return; // Terminate.
        }
    } else if (addEntry(&vm.globals, &entry)) {
        error(&parser.current, "Variable already exists");
        return; // Terminate.
    }

    advance();

    // Add string to constants' table.
    function->name = entry.key;
    Value value = prepareValue(AS_OBJ(entry.key), VAL_STRING);
    if (classCompiler.objClass != NULL) {
        Value classValue = prepareValue(AS_OBJ(classCompiler.objClass), VAL_CLASS);
        addInstructions(OP_CONSTANT, addConstant(classValue));
        addInstructions(OP_SET_METHOD, addConstant(value));
    } else
        addInstructions(OP_DEFINE_GLOBAL, addConstant(value));
}

static bool isInit(ObjFunction* function) {
    if (function->name == NULL) return false;
    if (strncmp(function->name->value, Init_Method_Name, 4) == 0)
        return true;
    return false;
}

static void declareFunction() {
    ObjFunction* function = (ObjFunction*)allocateObject(OBJ_FUNCTION);

    Value value = prepareValue(AS_OBJ(function), VAL_FUNCTION);
    addInstructions(OP_CONSTANT, addConstant(value));
    addInstruction(OP_CLOSURE);
    declareFunctionName(function);

    initCompiler(function, isInit(function));

    scopeStart();       // Function should have its own scope for arguments and locals.
    argumentList();     // parser argument list.
    scopeSimpleEnd();   // This scope will be handled by function's block.

    consume(TOKEN_RIGHT_PAREN, "')' is required after argument list.");
    advance();

    block(true);

    endFunction();
}

// Class Section.
static void declareClassName() {
    advance();
    consume(TOKEN_IDENTIFIER, "Identifier is expected after 'class'.");

    if (compiler->scopeDepth > 0)
        return declareLocalVariable();
    
    Entry entry;
    entry.key = copyString(parser.current.string);
    entry.tombstone = false;

    if (addEntry(&vm.globals, &entry)) {
        error(&parser.current, "Variable already exists");
        return; // Terminate.
    }

    Value value = prepareValue(AS_OBJ(entry.key), VAL_STRING);
    addInstructions(OP_DEFINE_GLOBAL, addConstant(value));    
    advance();
}

static void classStart(ObjClass* objClass) {
    classCompiler.objClass = objClass;
}

static void classEnd() {
    classCompiler.objClass = NULL;
}

static void declareMethods() {
    consume(TOKEN_LEFT_CURLY, "'{' is required after class name.");
    advance();

    while (parser.current.type != TOKEN_RIGHT_CURLY)
        declareFunction();
    
    consume(TOKEN_RIGHT_CURLY, "'}' is required after class body.");
    advance();
}

static void declareInheritance() {
    switch(parser.current.type) {
        case TOKEN_INHERIT: {
            advance();
            consume(TOKEN_IDENTIFIER, "Identifier is expected after 'inherits'.");
            advance();
            
            Value value = prepareValue(AS_OBJ(classCompiler.objClass), VAL_CLASS);
            addInstructions(OP_CONSTANT, addConstant(value));
            
            identifier(false);
            addInstruction(OP_INHERIT);
            break;
        }
        default: {
            classCompiler.objClass->parent = NULL;
            return;
        }
    }
}

static void declareClass() {
    ObjClass* objClass = newClass();

    classStart(objClass);
    Value value = prepareValue(AS_OBJ(objClass), VAL_CLASS);
    addInstructions(OP_CONSTANT, addConstant(value));
    declareClassName();
    declareInheritance();
    declareMethods();
    classEnd();
}

void declaration() {
    synchronize();

    switch(parser.current.type) {
        case TOKEN_VAR:
            declareVariable();
            break;
        case TOKEN_FUN:
            declareFunction();
            break;
        case TOKEN_CLASS:
            declareClass();
            break;
        default:
            statement();
    }
}

void compile(bool cmd) {
    advance();
    while(parser.current.type != TOKEN_EOF) {
        declaration();
        if (cmd) break;
    }
}

ObjString* copyString(const char* str) {
    ObjString* strObj = (ObjString*)allocateObject(OBJ_STRING);
    strObj->value = str;
    strObj->hash = hashString(strObj->value);

    return internString(strObj);
}