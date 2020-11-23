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
    OP_RETURN
} OpCode;

#endif