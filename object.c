#include "object.h"

ObjClosure* newClosure(ObjFunction* function) {
    ObjClosure* closure = (ObjClosure*)malloc(sizeof(ObjClosure));

    closure->function = function;

    // Prepare UpValues.
    ObjUpValue** upvalues = 
        (ObjUpValue**)malloc(sizeof(ObjUpValue*) * function->upvaluesCount);

    for (size_t i = 0; i < function->upvaluesCount; i++)
        upvalues[i] = NULL;
    closure->upvalues = upvalues;

    return closure;
}