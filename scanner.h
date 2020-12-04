#ifndef ZAO_SCANNER_H
#define ZAO_SCANNER_H

#include "ctype.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory.h"


// Token section.
typedef enum {
    // ARITHMETICS
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_FRD_SLASH,

    TOKEN_NUMBER,
    // EXTREAM CASE TOKENS
    TOKEN_EOF,
    
    // BLOCKS AND GROUPINGS
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_CURLY, TOKEN_RIGHT_CURLY,
    
    // PUNCTUATIONS
    TOKEN_SEMI_COLON, TOKEN_QUESTION, TOKEN_COLON, TOKEN_BANG /* ! */,

    // UNARY TOKENS
    TOKEN_FALSE, TOKEN_TRUE, TOKEN_NIL,

    // MULTICHARACTER TOKENS
    TOKEN_STRING, TOKEN_IDENTIFIER,

    // ARITHMETICS
    TOKEN_MODULO, TOKEN_BCK_SLASH,

    // COMPARISIONS
    TOKEN_GREATER_THAN, TOKEN_LESS_THAN, TOKEN_EQUAL,

    // COMPARISION COMBINATIONS
    TOKEN_EQUAL_EQUAL, TOKEN_GREATER_EQUAL, TOKEN_LESS_EQUAL,
    TOKEN_BANG_EQUAL,

    // BOOL OPERATORS
    TOKEN_AND, TOKEN_OR,

    // UNDERSCORE AND HYPHEN
    TOKEN_UNDERSCORE, TOKEN_HYPHEN,

    // KEYWORDS 
    TOKEN_FOR, TOKEN_FUN, TOKEN_WHILE,
    TOKEN_CLASS, TOKEN_CONTINUE, TOKEN_BREAK, TOKEN_IF, TOKEN_ELSE, 
    TOKEN_PRINT, TOKEN_VAR

} TokenType;

typedef struct {
    TokenType type;
    char* string;
    double number;
    int lineNumber;
} Token;


Token prepToken(TokenType type);

// Scanner section.
typedef struct {
    const char* sourceCode;
    size_t position;
    size_t size;
    int lineNumber;
} Scanner;

extern Scanner scanner;

void initScanner(const char* sourceCode);
void freeScanner();
const char* fileInput(const char* fileName);
const char* cmdInput();


// Token scanning section.
Token scanToken();
char getChar(bool noWS);

#endif