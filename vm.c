#include <stdio.h>

#include "vm.h"

VM vm;

void initVM() {
    vm.stackTop = vm.stack;
}

Value* pop() {
    // TODO: display `empty stack` error.
    return --vm.stackTop;
}

void push(Value* value) {
    if ((vm.stackTop - vm.stack) > UINT8_MAX);
        // TODO: display `stack overflow` error.
    *vm.stackTop++ = *value;
}

static void printValue(Value* value) {
    printf("%g", *value);
}

void run() {
    for (;;) {
#define READ_BYTE() *compiler.ip++

#ifdef ZAO_DEBUGGER_MODE_ON
    displayInstruction(compiler.ip);
#endif 
        switch(*compiler.ip++) {
            case OP_NONE:
                // Tertminate.
                return;
            case OP_CONSTANT:
                push(&compiler.constants[READ_BYTE()]);     
                break;
            case OP_PRINT: {
                Value* value = pop();
                printValue(value);
                break;
            }
            case OP_RETURN:
                printf("OP_RETURN\n");
                break;
            default:
                // Terminate loop. 
                return;
        }
#undef READ_BYTE
    }
}