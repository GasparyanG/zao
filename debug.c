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