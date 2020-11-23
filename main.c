#include "compiler.h"
#include "vm.h"

int main() {
    initCompiler();
    initVM();

    addInstruction(OP_CONSTANT);
    uint8_t position = addConstant(8.5);
    addInstruction(position);
    addInstruction(OP_PRINT);

    run();

    return 0;
}