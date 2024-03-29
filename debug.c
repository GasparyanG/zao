#include "debug.h"

static size_t constantInstruction(const char* name, uint8_t* ip) {
    switch(vm.callFrame->closure->function->constants[*ip].type) {
        case VAL_NUMBER:
            printf("%-20s %g\n", name, vm.callFrame->closure->function->constants[*ip].as.number);
            break;
        case VAL_STRING:
            printf("%-20s %s\n", name, AS_STRING(vm.callFrame->closure->function->constants[*ip].as.obj)->value);
            break;
        case VAL_FUNCTION:
            printf("%-20s <function>\n", name);
            break;
    }

    return 1;
}

static size_t simpleInstruction(const char* name) {
    printf("%-16s\n", name);
    return 1;
}

static size_t jumpInstruction(const char* name, OpCode type, uint8_t* ip) {
    uint8_t a = *ip++;
    uint8_t b = *ip++;

    switch (type) {
        case OP_JUMP_FOR: {
            uint8_t c = *ip++;
            uint8_t d = *ip; 
            uint16_t size = bytesFusion(a, b);
            uint16_t size2 = bytesFusion(c, d);
            printf("%-20s ASSIGN - %d, BLOCK - %d\n", name, size, size2);
            return 4;
        }
        default: {
            uint16_t size = bytesFusion(a, b);
            printf("%-20s %d\n", name, size);
            return 3;
        }
    }
}

void displayInstruction(uint8_t* ip) {
    size_t offset = 0;  // Revert ip changes.

    switch(*ip++) {
        case OP_CONSTANT:
            offset = constantInstruction("OP_CONSTANT", ip);
            break;
        case OP_FALSE:
            offset = simpleInstruction("OP_FALSE");
            break;
        case OP_TRUE:
            offset = simpleInstruction("OP_TRUE");
            break;
        case OP_NIL:
            offset = simpleInstruction("OP_TRUE");
            break;
        case OP_PRINT:
            offset = simpleInstruction("OP_PRINT");
            break;
        case OP_RETURN:
            offset = simpleInstruction("OP_RETURN");
            break;
        case OP_CLOSE:
            offset = simpleInstruction("OP_CLOSE");
            break;
        case OP_ADD:
            offset = simpleInstruction("OP_ADD");
            break;
        case OP_MULTIPLY:
            offset = simpleInstruction("OP_MULTIPLY");
            break;
        case OP_DIVIDE:
            offset = simpleInstruction("OP_DIVIDE");
            break;
        case OP_SUBTRACT:
            offset = simpleInstruction("OP_SUBTRACT");
            break;
        case OP_NEGATE:
            offset = simpleInstruction("OP_NEGATE");
            break;
        case OP_DEFINE_GLOBAL:
            offset = constantInstruction("OP_DEFINE_GLOBAL", ip);
            break;
        case OP_GET_GLOBAL:
            offset = constantInstruction("OP_GET_GLOBAL", ip);
            break;
        case OP_SET_GLOBAL:
            offset = constantInstruction("OP_SET_GLOBAL", ip);
            break;
        case OP_GET_LOCAL:
            offset = constantInstruction("OP_GET_LOCAL", ip);
            break;
        case OP_SET_LOCAL:
            offset = constantInstruction("OP_SET_LOCAL", ip);
            break;
        case OP_GET_UPVALUE:
            offset = constantInstruction("OP_GET_UPVALUE", ip);
            break;
        case OP_SET_UPVALUE:
            offset = constantInstruction("OP_SET_UPVALUE", ip);
            break;
        case OP_GET_PROPERTY:
            offset = simpleInstruction("OP_GET_PROPERTY");
            break;
        case OP_SET_PROPERTY:
            offset = simpleInstruction("OP_SET_PROPERTY");
            break;
        case OP_SET_METHOD:
            offset = simpleInstruction("OP_SET_METHOD");
            break;
        case OP_CLOSURE:
            offset = simpleInstruction("OP_CLOSURE");
            break;
        case OP_GREATER_THAN:
            offset = simpleInstruction("OP_GREATER_THAN");
            break;
        case OP_LESS_THAN:
            offset = simpleInstruction("OP_LESS_THAN");
            break;
        case OP_BANG:
            offset = simpleInstruction("OP_BANG");
            break;
        case OP_EQUAL_EQUAL:
            offset = simpleInstruction("OP_EQUAL_EQUAL");
            break;
        case OP_AND:
            offset = simpleInstruction("OP_AND");
            break;
        case OP_OR:
            offset = simpleInstruction("OP_OR");
            break;
        case OP_JUMP:
            offset = jumpInstruction("OP_JUMP", OP_JUMP, ip);
            break;
        case OP_JUST_JUMP:
            offset = jumpInstruction("OP_JUST_JUMP", OP_JUST_JUMP, ip);
            break;
        case OP_JUMP_BACK:
            offset = jumpInstruction("OP_JUMP_BACK", OP_JUMP_BACK, ip);
            break;
        case OP_JUMP_FOR:
            offset = jumpInstruction("OP_JUMP_FOR", OP_JUMP_FOR, ip);
            break;
        case OP_JUMP_IF_TRUE:
            offset = jumpInstruction("OP_JUMP_IF_TRUE", OP_JUMP_IF_TRUE, ip);
            break;
        case OP_CALL:
            offset = simpleInstruction("OP_CALL");
            break;
        case OP_INVOKE:
            offset = simpleInstruction("OP_INVOKE");
            break;
        case OP_THIS:
            offset = simpleInstruction("OP_THIS");
            break;
        case OP_INHERIT:
            offset = simpleInstruction("OP_INHERIT");
            break;
        case OP_SUPER:
            offset = simpleInstruction("OP_SUPER");
            break;
        case OP_POP:
            offset = simpleInstruction("OP_POP");
            break;
    } 

    ip -= offset;
}


// TOKENS DEBUGGING SECTION.
static void simpleToken(const char* name) {
    printf("Token - %s\n", name);
}

static void stringToken(Token* token) {
    printf("Token - %d, string - '%s'\n", token->type, token->string);
}

static void numberToken(Token* token) {
    printf("Token - %d, number - '%g'\n", token->type, token->number);
}

static void keywordToken(Token* token) {
    printf("Token - %d, keyword\n", token->type);
}

static void cmpToken(Token* token) {
    printf("Token - %d, comparision\n", token->type);
}

void displayToken(Token* token) {
    switch(token->type) {
        case TOKEN_FUN: case TOKEN_FOR: case TOKEN_FALSE:
        case TOKEN_TRUE: case TOKEN_WHILE: case TOKEN_CLASS:
        case TOKEN_CONTINUE: case TOKEN_BREAK: case TOKEN_IF:
        case TOKEN_ELSE:
            keywordToken(token);
            break;
        case TOKEN_EQUAL: case TOKEN_EQUAL_EQUAL: case TOKEN_GREATER_EQUAL:
        case TOKEN_GREATER_THAN: case TOKEN_LESS_THAN: case TOKEN_LESS_EQUAL:
            cmpToken(token);
            break;
        case TOKEN_STRING:
            stringToken(token);
            break;
        case TOKEN_IDENTIFIER:
            stringToken(token);
            break;
        case TOKEN_NUMBER:
            numberToken(token);
            break;
        case TOKEN_LEFT_CURLY:
            simpleToken("{");
            break;
        case TOKEN_RIGHT_CURLY:
            simpleToken("}");
            break;
        case TOKEN_EOF:
            simpleToken("EOF");
            break;
        default:
            printf("Token %d is not handled yet.\n", token->type);
            break;
    }
}