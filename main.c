#include "compiler.h"
#include "vm.h"
#include "scanner.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    initCompiler();
    initVM();

    if (argc > 1)
        initScanner(fileInput(argv[1]));
    else
        initScanner(cmdInput());


    return 0;
}