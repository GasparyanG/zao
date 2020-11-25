#include "scanner.h"


// Declarations
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

bool advance(int n) {
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

        // TODO: deal with other tokens too.
        default:
            // TODO: display `unknown token` error
            return prepToken(TOKEN_EOF);
    }
}