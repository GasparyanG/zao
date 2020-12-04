#ifndef ZAO_CHUNK_H
#define ZAO_CHUNK_H

#include <stdint.h>
#include <stdlib.h>

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
    OP_AND,
    OP_OR
} OpCode;

#endif