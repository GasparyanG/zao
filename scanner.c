#include "scanner.h"

Scanner scanner;

void initScanner(const char* sourceCode) {
    scanner.sourceCode = sourceCode;
    scanner.position = 0;
    scanner.size = strlen(sourceCode);
}