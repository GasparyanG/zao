#ifndef ZAO_SCANNER_H
#define ZAO_SCANNER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory.h"


// Token section.
typedef enum {
    // BLOCKS AND GROUPINGS
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_CURLY, TOKEN_RIGHT_CURLY,
    
    // ARITHMETICS
    TOKEN_STAR, TOKEN_PLUS, TOKEN_MINUS, TOKEN_MODULO, TOKEN_FRD_SLASH, TOKEN_BCK_SLASH,

    // COMPARISIONS
    TOKEN_GREATER_THAN, TOKEN_LESS_THAN, TOKEN_EQUAL,

    // COMPARISION COMBINATIONS
    TOKEN_EQUAL_EQUAL, TOKEN_GREATER_EQUAL, TOKEN_LESS_EQUAL,

    // PUNCTUATIONS
    TOKEN_QUESTION, TOKEN_COLON, TOKEN_SEMI_COLON, TOKEN_BANG /* ! */,

    // UNDERSCORE AND HYPHEN
    TOKEN_UNDERSCORE, TOKEN_HYPHEN,

    // EXTREAM CASE TOKENS
    TOKEN_EOF,

    // MULTICHARACTER TOKENS
    TOKEN_STRING, TOKEN_IDENTIFIER, TOKEN_NUMBER

    // KEYWORDS 
    // (coming soon)

} TokenType;

typedef struct {
    TokenType type;
    char* string;
    double number;
} Token;


Token prepToken(TokenType type);

// Scanner section.
typedef struct {
    const char* sourceCode;
    size_t position;
    size_t size;
} Scanner;

extern Scanner scanner;

void initScanner(const char* sourceCode);
const char* fileInput(const char* fileName);
const char* cmdInput();


// Token scanning section.
Token scanToken();
char getChar();
bool advance(int n);

#endif