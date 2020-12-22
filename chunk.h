#ifndef ZAO_CHUNK_H
#define ZAO_CHUNK_H

#include <stdint.h>
#include <stdlib.h>

#define JUMP_BYTES 2

typedef struct {
    uint8_t* chunk;
    size_t size;
    size_t capacity;
} Chunk;

typedef enum {
    OP_NONE,        // Don't remove from this position.
    OP_CONSTANT,
    OP_PRINT,
    OP_RETURN,
    OP_CLOSE,
    OP_ADD,
    OP_SUBTRACT,
    OP_DIVIDE,
    OP_MULTIPLY,
    OP_GREATER_THAN,
    OP_LESS_THAN,
    OP_EQUAL_EQUAL,
    OP_BANG,
    OP_NEGATE,
    OP_FALSE,
    OP_TRUE,
    OP_NIL,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_SET_PROPERTY,
    OP_GET_PROPERTY,
    OP_SET_METHOD,
    OP_CLOSURE,
    OP_AND,
    OP_OR,
    OP_JUMP,
    OP_JUMP_BACK,
    OP_JUMP_FOR,
    OP_CALL,
    OP_INVOKE,
    OP_THIS,
    OP_POP
} OpCode;

#endif