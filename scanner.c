#include "scanner.h"

Scanner scanner;

void initScanner(const char* sourceCode) {
    scanner.sourceCode = sourceCode;
    scanner.position = 0;
    scanner.size = strlen(sourceCode);
}

const char* fileInput(const char* fileName) {
    FILE* fp;
    fp = fopen(fileName, "r");

    // Compute file size.
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);

    rewind(fp);                         // Rewind back to the beginning.
    char* sourceCode = (char*)malloc(sz + 1);  // Allocate memory for file's content.

    // Read to sourceCode
    fread(sourceCode, sizeof(char), sz, fp);
    sourceCode[sz] = '\0';

    fclose(fp);                         // Return resources back by closing file.

    return sourceCode;
}

const char* cmdInput() {

}