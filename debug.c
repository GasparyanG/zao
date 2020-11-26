#include "debug.h"

static size_t constantInstruction(const char* name, uint8_t* ip) {
    printf("%-16s %g\n", name, compiler.constants[*ip]);
    return 1;
}

static size_t simpleInstruction(const char* name) {
    printf("%-16s\n", name);
    return 1;
}

void displayInstruction(uint8_t* ip) {
    size_t offset = 0;  // Revert ip changes.

    switch(*ip++) {
        case OP_CONSTANT:
            offset = constantInstruction("OP_CONSTANT", ip);
            break;
        case OP_PRINT:
            offset = simpleInstruction("OP_PRINT");
            break;
        case OP_RETURN:
            offset = simpleInstruction("OP_RETURN");
            break;
        case OP_ADD:
            offset = simpleInstruction("OP_ADD");
            break;
        default:
            ip -= offset;
            return;
    } 
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
        // TODO: implement other tokens debugging too.
        default:
            printf("Token %d is not handled yet.\n", token->type);
            break;

    }
}