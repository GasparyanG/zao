#include "object.h"
#include "memory.h"

ObjClosure* newClosure(ObjFunction* function) {
    ObjClosure* closure = (ObjClosure*)allocateObject(OBJ_CLOSURE);

    closure->function = function;

    // Prepare UpValues.
    ObjUpValue** upvalues = 
        (ObjUpValue**)malloc(sizeof(ObjUpValue*) * function->upvaluesCount);

    for (size_t i = 0; i < function->upvaluesCount; i++)
        upvalues[i] = NULL;
    closure->upvalues = upvalues;

    return closure;
}

ObjInstance* newObjInstance(ObjClass* objClass) {
    ObjInstance* instance = (ObjInstance*)allocateObject(OBJ_INSTANCE);
    instance->blueprint = objClass;

    // Initialize properties container.
    Table properties;
    initTable(&properties);
    instance->properties = properties;

    return instance;
}

ObjClass* newClass() {
    ObjClass* objClass = (ObjClass*)allocateObject(OBJ_CLASS);

    Table methods;
    initTable(&methods);
    objClass->methods = methods;
    objClass->parent = NULL;

    return objClass;
}