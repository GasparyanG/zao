#ifndef ZAO_SCANNER_H
#define ZAO_SCANNER_H

#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* sourceCode;
    size_t position;
    size_t size;
} Scanner;

extern Scanner scanner;

void initScanner(const char* sourceCode);

#endif