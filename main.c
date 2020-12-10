#include <stdio.h>

#include "compiler.h"
#include "vm.h"
#include "scanner.h"
#include "object.h"


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

static void repl() {
    for(;;) {
        printf("> ");
        initScanner(cmdInput());
        
        compile(true);
        
        if (compiler->panicMode)
            recover();
        else
            if (!runtimeResult(run())) break;

        freeScanner();
    }
}

int main(int argc, char* argv[]) {
    ObjFunction* function = (ObjFunction*)malloc(sizeof(ObjFunction));

    initCompiler(function);
    initVM();

    if (argc > 1) {
        initScanner(fileInput(argv[1]));
        
        compile(false);
        run();

        freeScanner();
    } else
        repl();

    freeCompiler();

    return 0;
}