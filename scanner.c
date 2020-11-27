#include "scanner.h"


// Declarations.
static bool isEof();


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
    char* sourceCode;
    size_t size = 0;
    size_t capacity = ARRAY_INITIAL_SIZE;
    sourceCode = ALLOCATE(char, sourceCode, 0);

    char c;
    for(;;) {
        c = fgetc(stdin);
        if (c=='\n') 
            break;
        else {
            if (size == capacity) {
                sourceCode = ALLOCATE(char, sourceCode, size);
                capacity *= ENLARGEMENT_FACTOR;
            }

            sourceCode[size++] = c;
        }
    }

    if (size == capacity)
        sourceCode = (char*)reallocate((void*)sourceCode, size + 1);
    
    sourceCode[size] = '\0';

    return sourceCode;
}

char getChar(bool noWS) {
    char ch;
    ch = scanner.sourceCode[scanner.position];
    scanner.position++;

    // Get rid of whitespaces.
    while (noWS && isspace(ch))
        ch = scanner.sourceCode[scanner.position++];

    return ch;
}

static bool advance(int n) {
    if (scanner.size == 0) return false;
    if (scanner.position > scanner.size) return false;

    if (n < 0 && (scanner.position + n) < 0)
        scanner.position = 0;
    else if (n < 0) {
        scanner.position += n;
    } else if (n > 0 && (scanner.position + n) >= scanner.size)
        return false;
    else if (n > 0)
        scanner.position += n;
    else if (n == 0)
        scanner.position += 1;
    
    return true;
}

bool consume(TokenType type, const char* message) {
    Token token = scanToken();
    if (token.type == type) return true;

    printf("%s\n", message);
    return false;
}


Token prepToken(TokenType type) {
    Token token;
    token.type = type;

    // This two will never be used, 
    // but it's bad practice to leave them uninitialized.
    token.string = NULL;
    token.number = 0;

    return token;
}

static Token prepString() {
    Token token;
    token.type = TOKEN_STRING;
    token.number = 0;           // Isn't required.

    token.string = ALLOCATE(char, token.string, 0);
    size_t size = 0;
    size_t capacity = ARRAY_INITIAL_SIZE;

    // Heavy lifting.
    for (;;) {
        // EOF checking.
        if (!advance(0))
            return prepToken(TOKEN_EOF);
        advance(-1);                        // EOF is checked now go back again.

        char ch;
        if ((ch = getChar(false)) == '"') { // End of string is reached, so terminate.
            if (size == capacity) {
                token.string = (char*)reallocate((void*)token.string, size + 1);
                token.string[size] = '\0';
            }

            return token;
        }
        
        if (size == capacity) {
            token.string = ALLOCATE(char, token.string, size);
            capacity *= ENLARGEMENT_FACTOR;
        }

        token.string[size++] = ch;
    }
}


static void compareForKeyword(
    Token* token, size_t position, const char* str, TokenType type) {
    size_t length = position + strlen(str);
    if (strlen(token->string) != length) return;            // No need for comparision.
    if (strcmp(&token->string[position], str) != 0) return; // This is not a keyword substring.

    // Update token to corresponding TokenType.
    token->type = type;

    // Free resources back.
    free(token->string);
    token->string = NULL;
}

static void lookForKeyword(Token* token) {
    switch(token->string[0]) {
        case 'f': {
            if (strlen(token->string) == 1) return; // Terminate.
            switch(token->string[1]) {
                case 'o': return compareForKeyword(token, 2, "r", TOKEN_FOR);      // for
                case 'a': return compareForKeyword(token, 2, "lse", TOKEN_FALSE);  // false
                case 'u': return compareForKeyword(token, 2, "n", TOKEN_FUN);      // fun as function
                default:
                    return; // Terminate.
            }
            break;
        }

        case 't':
            return compareForKeyword(token, 1, "rue", TOKEN_TRUE);      // true
        case 'w':
            return compareForKeyword(token, 1, "hile", TOKEN_WHILE);    // while
        case 'c': {
            if (strlen(token->string) == 1) return; // Terminate.
            switch(token->string[1]) {
                case 'l': return compareForKeyword(token, 2, "ass", TOKEN_CLASS);        // class
                case 'o': return compareForKeyword(token, 2, "ntinue", TOKEN_CONTINUE);  // continue
                default:
                    return; // Terminate.
            }
        }

        case 'b':
            return compareForKeyword(token, 1, "reak", TOKEN_BREAK);    // break
        case 'i':
            return compareForKeyword(token, 1, "f", TOKEN_IF);          // if
        case 'e':
            return compareForKeyword(token, 1, "lse", TOKEN_ELSE);      // else
        case 'p':
            return compareForKeyword(token, 1, "rint", TOKEN_PRINT);    // print
        default:
            return;         // Terminate.
    }
}

static Token prepIdentifier() {
    advance(-1);                    // Get the previous character as well.
    Token token;
    token.type = TOKEN_IDENTIFIER;
    token.number = 0;               // This field will not be used.

    // Heavy lifting.
    size_t size = 0;
    size_t capcaity = ARRAY_INITIAL_SIZE;
    token.string = ALLOCATE(char, token.string, 0);

    for (;;) {
        if (isEof()) break;
        char ch = getChar(false);

        // Identifiers can only contain letters, numbers, and '_', nothing more.
        if (isspace(ch) || (!isalnum(ch) && ch != '_')) {
            advance(-1);    // Don't lose any chars.
            break;
        }

        if (size == capcaity) {
            token.string = ALLOCATE(char, token.string, size);
            capcaity *= ENLARGEMENT_FACTOR;
        }

        token.string[size++] = ch;
    }

    // Terminating 0.
    if (size == capcaity)
        token.string = (char*)reallocate((void*)token.string, size + 1);

    token.string[size] = '\0';

    // Check whether this identifier is keyword or not.
    // If it is, then update Token to corresponding keyword.
    lookForKeyword(&token);

    return token;
}

static Token prepNumber() {
    advance(-1);                // Get the previous character as well.
    Token token;
    token.type = TOKEN_NUMBER;
    size_t size = 0;
    size_t capacity = ARRAY_INITIAL_SIZE;
    // Collect digits here and then convert to double.
    token.string = ALLOCATE(char, token.string, size);

    // Heavy lifting
    for(;;) {
        if (isEof()) break;
        char ch = getChar(false);

        if (ch != '.' && !isdigit(ch)) {
            advance(-1);        // Don't lose any character.
            break;
        }

        // Make sure that dot (.) is meant for double precision and not for anything else.
        if (ch == '.') {
            if (isEof()) {
                advance(-1);
                break;
            }

            if (!isdigit(getChar(false))) {
                advance(-2);
                break;
            }
        }

        if (size == capacity) {
            token.string = ALLOCATE(char, token.string, size);
            capacity *= ENLARGEMENT_FACTOR;
        }

        token.string[size++] = ch;
    }

    token.number = strtod(token.string, NULL);  // Convert string to double.

    // Free resources back to memory.
    free(token.string);
    token.string = NULL;

    return token;
}

static bool isEof() {
    if (!advance(0)) return true;
    advance(-1);
    return false;
}

static void consumeComment() {
    while(!isEof()) {
        if (getChar(false) != '\n')
            continue;
        else {
            break;
        }
    }
}

// Token scanning section.
Token scanToken() {
    if (!advance(0))
        return prepToken(TOKEN_EOF);
    
    advance(-1);
    char ch = getChar(true);

    // Identifiers and numbers praparation.
    if (isalpha(ch))
        return prepIdentifier();
    else if (isdigit(ch))
        return prepNumber();

    switch(ch) {
        case '"':
            return prepString();
        case '{':
            return prepToken(TOKEN_LEFT_CURLY);
        case '}':
            return prepToken(TOKEN_RIGHT_CURLY);
        case '(':
            return prepToken(TOKEN_LEFT_PAREN);
        case ')':
            return prepToken(TOKEN_RIGHT_PAREN);
        case '/': {
            if(!advance(0))
                return prepToken(TOKEN_EOF);
            advance(-1);
            ch = getChar(true);

            if(ch == '/') {
                consumeComment();
                return scanToken();
            }

            advance(-1);
            return prepToken(TOKEN_FRD_SLASH);   
        }

        case '>': {
            if (isEof()) return prepToken(TOKEN_GREATER_THAN);
            ch = getChar(true);

            if (ch == '=')
                return prepToken(TOKEN_GREATER_EQUAL);
            
            advance(-1);
            return prepToken(TOKEN_GREATER_THAN);
        }

        case '<': {
            if (isEof()) return prepToken(TOKEN_LESS_THAN);
            ch = getChar(true);

            if (ch == '=')
                return prepToken(TOKEN_LESS_EQUAL);
            
            advance(-1);
            return prepToken(TOKEN_LESS_THAN);
        }

        case '=': {
            if (isEof()) return prepToken(TOKEN_EQUAL);
            ch = getChar(true);

            if (ch == '=')
                return prepToken(TOKEN_EQUAL_EQUAL);
            
            advance(-1);
            return prepToken(TOKEN_EQUAL);
        }

        case '+': return prepToken(TOKEN_PLUS);
        case '-': return prepToken(TOKEN_MINUS);
        case '*': return prepToken(TOKEN_STAR);

        // TODO: deal with other tokens too.
        default:
            // TODO: display `unknown token` error
            return prepToken(TOKEN_EOF);
    }
}