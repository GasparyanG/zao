#ifndef ZAO_VALUE_H
#define ZAO_VALUE_H

#include "object.h"

#define AS_NUMBER(value)    (value->as.number)
#define AS_BOOL(value)      ((value.type == VAL_FALSE) ? false : true)

#define IS_NUMBER(value)    (value.type == VAL_NUMBER)
#define IS_BOOL(value)      (value.type == VAL_FALSE || value.type == VAL_TRUE)

typedef enum {
    VAL_NIL,
    VAL_FALSE,
    VAL_TRUE,
    VAL_NUMBER,
    VAL_STRING,
    VAL_FUNCTION
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        Obj* obj;
    } as;
} Value;

#endif