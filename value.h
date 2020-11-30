#ifndef ZAO_VALUE_H
#define ZAO_VALUE_H

#define AS_NUMBER(value)    (value->as.number)

#define IS_NUMBER(value)    (value.type == VAL_NUMBER)

typedef enum {
    VAL_NIL,
    VAL_FALSE,
    VAL_TRUE,
    VAL_NUMBER
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
    } as;
} Value;

#endif