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
        default:
            ip -= offset;
            return;
    } 
}


// TOKENS DEBUGGING SECTION.
static void simpleToken(const char* name) {
    printf("token - %s\n", name);
}

static void stringToken(Token* token) {
    printf("token - %d, string - '%s'\n", token->type, token->string);
}

void displayToken(Token* token) {
    switch(token->type) {
        case TOKEN_STRING:
            stringToken(token);
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