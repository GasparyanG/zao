#include "compiler.h"
#include "vm.h"
#include "scanner.h"
#include <stdio.h>

static bool isEof() {
    if (parser.current.type == TOKEN_SEMI_COLON) {
        advance();
        if (parser.current.type == TOKEN_EOF)
            return true;
    }

    return false;
}

int main(int argc, char* argv[]) {
    initCompiler();
    initVM();

    if (argc > 1) {
        initScanner(fileInput(argv[1]));
        for(;;) {
            compile();
            
            if (compiler.panicMode)
                recover();
            else
                run();

            if (isEof()) break;
        }

        freeScanner();
    } else {
        for(;;) {
            printf("> ");
            initScanner(cmdInput());
            
            compile();
            
            if (compiler.panicMode)
                recover();
            else
                run();

            freeScanner();
        }
    }

    freeCompiler();

    return 0;
}