#ifndef ZAO_OBJECT_H
#define ZAO_OBJECT_H

#define AS_STRING(obj)    ((ObjString*)(obj))

#define AS_OBJ(obj)       ((Obj*)(obj))

typedef enum {
    OBJ_STRING
} ObjType;


// Mainly meant for polymorphism.
typedef struct {
    ObjType type;
} Obj;

typedef struct {
    Obj obj;            // Type identifier.
    const char* value;  // Payload.
    uint32_t hash;      // Hashed string;
} ObjString;

#endif