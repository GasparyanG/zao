#include "compiler.h"
#include "vm.h"
#include "scanner.h"
#include <stdio.h>

static bool isEof() {
    if (parser.current.type == TOKEN_EOF)
        return true;

    return false;
}

static bool runtimeResult(ExecutionResult result) {
    switch(result) {
        case EXECUTION_SUCCESS:
            return true;
        case INTERPRETER_RUNTIME_ERROR:
            return false;
        default:
            // Unreachable.
            return false;
    }
}

int main(int argc, char* argv[]) {
    initCompiler();
    initVM();

    if (argc > 1) {
        initScanner(fileInput(argv[1]));
        for(;;) {
            compile();
            
            if (compiler->panicMode)
                recover();
            else
                if (!runtimeResult(run())) break;

            if (isEof()) break;
        }

        freeScanner();
    } else {
        for(;;) {
            printf("> ");
            initScanner(cmdInput());
            
            compile();
            
            if (compiler->panicMode)
                recover();
            else
                if (!runtimeResult(run())) break;

            freeScanner();
        }
    }

    freeCompiler();

    return 0;
}