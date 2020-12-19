#ifndef ZAO_OBJECT_H
#define ZAO_OBJECT_H

#include <stdint.h>
#include <stdlib.h>

#include "chunk.h"
#include "value.h"

#define AS_STRING(obj)    ((ObjString*)(obj))
#define AS_FUNCTION(obj)  ((ObjFunction*)(obj))
#define AS_CLOSURE(obj)   ((ObjClosure*)(obj))
#define AS_UPVALUE(obj)   ((ObjUpValue*)(obj))

#define AS_OBJ(obj)       ((Obj*)(obj))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_CLOSURE,
    OBJ_UPVALUE
} ObjType;

typedef struct {
    bool isLocal;           // Whether value is from parent function or not.
    uint8_t index;          // Index to make upvalue chaining possible.
} UpValue;

// Mainly meant for polymorphism.
typedef struct Obj {
    ObjType type;
    bool isMarked;
    Obj* next;
} Obj;

typedef struct {
    Obj obj;            // Type identifier.
    const char* value;  // Payload.
    uint32_t hash;      // Hashed string;
} ObjString;

typedef struct {
    Obj obj;
    ObjString* name;
    Chunk chunk;
    uint8_t* ip;
    size_t arity;   // Amount of arguments.

    Value constants[UINT8_MAX];
    size_t constPos;

    UpValue* upvalues[UINT8_MAX];
    uint8_t upvaluesCount;
} ObjFunction;

typedef struct {
    Obj obj;
    Value* location;
    Value value;
} ObjUpValue;

typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpValue** upvalues;
} ObjClosure;

typedef struct {
    Obj obj;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* blueprint;
    Table properties;
} ObjInstance;

ObjClosure* newClosure(ObjFunction* function);

#endif