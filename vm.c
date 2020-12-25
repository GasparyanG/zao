#include <stdio.h>

#include "vm.h"

VM vm;

void initVM() {
    vm.stackTop = vm.stack;
    vm.stringCount = 0;
    
    vm.callFrame = NULL;
    vm.objects = NULL;
    
    vm.nextGC = GC_INITIAL_THRESHOLD;
    vm.amountOfObjs = 0;

    vm.greyStack = NULL;
    vm.greyCount = 0;
    vm.greyCapacity = 0;
    
    Table glob;
    initTable(&glob);
}

void freeVM() {
    free(vm.callFrame);
    free(vm.stackTop);
}

CallFrame* initCallFrame() {
    return (CallFrame*)malloc(sizeof(CallFrame));
}

void freeCallFrame(CallFrame* callFrame) {
    free(callFrame);
}

ObjString* internString(ObjString* strToCmp) {
    for (uint32_t i = 0; i < vm.stringCount; i++) {
        if (strToCmp->hash == vm.internedStrings[i]->hash)
            return vm.internedStrings[i];
    }

    if ((vm.stringCount + 1) >= UINT8_MAX)
        fatalError("String stack overflow.");

    vm.internedStrings[vm.stringCount++] = strToCmp;
    return strToCmp;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // Show row.
    uint16_t currentPos 
        = vm.callFrame->closure->function->ip - vm.callFrame->closure->function->chunk.chunk;
    uint16_t lineNumber 
        = vm.callFrame->closure->function->chunk.lines[currentPos];
    fprintf(stderr, "[Line %d].\n", lineNumber);

    // Reset stack and call frame to be able to continue.
    vm.stackTop = vm.stack;
    vm.callFrame->closure->function->ip 
        = &vm.callFrame->closure->function->chunk.chunk[vm.callFrame->closure->function->chunk.size];
}

static bool isStackEmpty() {
    if ((vm.stackTop - vm.stack) == 0) return true;
    return false;
}

Value* pop() {
    if (isStackEmpty())
        runtimeError("Stack is empty.");
    return --vm.stackTop;
}

Value* peek(size_t n) {
    return vm.stackTop - n;
}

void push(Value* value) {
    if ((vm.stackTop - vm.stack) > UINT8_MAX)
        runtimeError("Stack limit (%d) is reached.", UINT8_MAX);
    *vm.stackTop++ = *value;
}


static void printValue(Value* value) {
    switch(value->type) {
        case VAL_NUMBER:
            printf("%g\n", value->as.number);
            break;
        case VAL_FALSE:
            printf("false\n");
            break;
        case VAL_TRUE:
            printf("true\n");
            break;
        case VAL_NIL:
            printf("NIL\n");
            break;
        case VAL_STRING:
            printf("%s\n", AS_STRING(value->as.obj)->value);
            break;
        case VAL_FUNCTION:
            printf("<function> %s\n", AS_FUNCTION(value->as.obj)->name->value);
            break;
        case VAL_CLASS:
            printf("<class>\n");
            break;
        case VAL_INSTANCE:
            printf("<instace>\n");
            break;
        default:
            return; // Unreachable.
    }
}


static inline ValueType boolValue(bool value) {
    if (value) return VAL_TRUE;
    return VAL_FALSE;
}

static void boolOperator(OpCode op) {
    Value b = *pop();
    Value a = *pop();

    if (IS_BOOL(b) && IS_BOOL(a)) {
        bool bb = (b.type == VAL_FALSE) ? false: true;
        bool ba = (a.type == VAL_TRUE) ? true: false;

        Value c;
        if (op == OP_AND)
            c.type = boolValue(bb && ba);
        else
            c.type = boolValue(bb || ba);
        push(&c);
    } else 
        runtimeError("Can't use %s for not bool values.", 
            (op == OP_AND) ? "and": "or");
}


// Fcuntion call operations.
static CallFrame* initCallFrameFromCompiler() {
    CallFrame* callFrame = initCallFrame();
    callFrame->closure = newClosure(compiler->function);

    callFrame->nextFrame = NULL;
    callFrame->functionLocals = vm.locals;
    callFrame->position = 0;

    return callFrame;
}

static void updatePosition(uint8_t pos) {
    vm.callFrame->position = 
        (vm.callFrame->position < pos) 
        ? pos : vm.callFrame->position;
}

static CallFrame* updateCallFrame(uint8_t argCount) {
    CallFrame* callFrame = initCallFrame();

    callFrame->closure = AS_CLOSURE(peek(argCount + 1)->as.obj);
    callFrame->nextFrame = vm.callFrame;
    callFrame->position = (vm.callFrame->position == 0) ? 0: (vm.callFrame->position + 1);
    callFrame->functionLocals = &vm.locals[callFrame->position];
    callFrame->closure->function->ip = callFrame->closure->function->chunk.chunk;

    return callFrame;
}

static void updateVariables(uint8_t arity) {
    for (int16_t j = arity; j > 0; j--)
        vm.callFrame->functionLocals[j] = *pop();
    
    // Don't handle local variable setting.
    vm.callFrame->closure->function->ip += 2*arity;
}

static void closeUpValues(ObjClosure* closure) {
    for (uint8_t i = 0; i < closure->function->upvaluesCount; i++) {
        // Copy Value from locals to upvalue.
        closure->upvalues[i]->value 
            = *closure->upvalues[i]->location;

        // Change location pointer.
        closure->upvalues[i]->location = 
            &closure->upvalues[i]->value;
    }
}

static void changeCallFrameBack() {
    CallFrame* callFrameToFree = vm.callFrame;
    vm.callFrame = vm.callFrame->nextFrame;
    freeCallFrame(callFrameToFree);
}

static void opReturn(bool nilReturned) {
    if (!nilReturned) return;

    changeCallFrameBack();
    
    Value value;
    value.type = VAL_NIL;
    push(&value);
}

static ObjUpValue* captureUpValue(Value* value) {
    ObjUpValue* upvalue = (ObjUpValue*)allocateObject(OBJ_UPVALUE);
    upvalue->location = value;

    return upvalue;
}

static Entry* initCall(ObjClass* objClass, ObjString* string) {
    if (objClass == NULL) return NULL;

    Entry* entry = findEntry(&objClass->methods, string);
    if (entry->key == NULL)
        return initCall(objClass->parent, string);
    
    return entry;
}

static void call(uint8_t arity) {
    Obj* obj = peek(arity + 1)->as.obj;

    switch(obj->type) {
        case OBJ_CLOSURE: {
            vm.callFrame = updateCallFrame(arity);
            updateVariables(arity);
            pop();      // Pop function from stack.

            // Be prepared to handle 'this' keyword.
            if (!isStackEmpty() && peek(1)->type == VAL_INSTANCE &&
                peek(1)->as.obj->type == OBJ_INSTANCE)
                vm.callFrame->functionLocals[FUNC_LOCALS_OFFSET] = *pop();

            break;
        }

        case OBJ_CLASS: {
            Value classObj = *peek(arity + 1);
            ObjInstance* instance = newObjInstance(AS_CLASS(classObj.as.obj));
            Value value;
            value.type = VAL_INSTANCE;
            value.as.obj = AS_OBJ(instance);

            ObjString* string = copyString("init");
            Entry* entry = initCall(AS_CLASS(classObj.as.obj), string);

            if ((entry == NULL || entry->key == NULL) && arity == 0) {
                pop();                          // Get rid of class object
                push(&value);                   // Push instance to stack.
            } else {
                *peek(arity + 1) = value;       // Replace class with instance.
                
                // Make room for closure.
                memcpy((vm.stackTop - arity + 1), (vm.stackTop - arity), sizeof(Value) * arity);
                vm.stackTop++;
                *peek(arity + 1) = entry->value; // Put closure to that room.

                call(arity);                     // Instance will be poped here.
            }

            break;
        }
    }

}

static bool invoke(ObjClass* objClass, ObjString* string) {
    if (objClass == NULL) return false; // Method doesn't exist.
    
    Entry* entry = findEntry(&objClass->methods, string);
    if (entry->key == NULL)
        return invoke(objClass->parent, string);

    push(&entry->value);
    return true;
}

ExecutionResult run() {
    vm.callFrame = initCallFrameFromCompiler();

    for (;;) {
#define READ_BYTE()         *vm.callFrame->closure->function->ip++
#define READ_STRING() \
    AS_STRING(vm.callFrame->closure->function->constants[(*vm.callFrame->closure->function->ip++)].as.obj)
#define READ_FUNCTION() \
    AS_FUNCTION(vm.callFrame->closure->function->constants[(*vm.callFrame->closure->function->ip++)].as.obj)
#define BOOL_BINARY_OP(op) \
    do { \
        Value* b = pop(); \
        Value* a = pop(); \
        Value c; \
        if (b->type == VAL_NUMBER) \
            c.type = boolValue((AS_NUMBER(a) op AS_NUMBER(b))); \
        else if (b->type == VAL_STRING) \
            c.type = boolValue((AS_STRING(a) op AS_STRING(b))); \
        push(&c); \
    } while(false)

#define BINARY_OP(op) \
    do { \
        Value* b = pop(); \
        Value* a = pop(); \
        Value c; \
        c.type = VAL_NUMBER; \
        c.as.number = (AS_NUMBER(a) op AS_NUMBER(b)); \
        push(&c); \
    } while(false)

#ifdef ZAO_DEBUGGER_MODE_ON
    displayInstruction(vm.callFrame->closure->function->ip);
#endif 

        if (*vm.callFrame->closure->function->ip == OP_NONE) {
            if (vm.callFrame->nextFrame != NULL)
                opReturn(true);
            else
                return EXECUTION_SUCCESS;
        }

        switch(*vm.callFrame->closure->function->ip++) {
            case OP_CONSTANT:
                push(&vm.callFrame->closure->function->constants[READ_BYTE()]);     
                break;
            case OP_FALSE: {
                Value value;
                value.type = VAL_FALSE;
                push(&value);
                break;
            }
            case OP_TRUE: {
                Value value;
                value.type = VAL_TRUE;
                push(&value);
                break;
            }
            case OP_NIL: {
                Value value;
                value.type = VAL_NIL;
                push(&value);
                break;
            }
            
            case OP_ADD:            BINARY_OP(+); break;
            case OP_SUBTRACT:       BINARY_OP(-); break;
            case OP_MULTIPLY:       BINARY_OP(*); break;
            case OP_DIVIDE:         BINARY_OP(/); break;
            case OP_GREATER_THAN:   BOOL_BINARY_OP(>); break;
            case OP_LESS_THAN:      BOOL_BINARY_OP(<); break;
            case OP_EQUAL_EQUAL:    BOOL_BINARY_OP(==); break;
            case OP_BANG: {
                Value value = *pop();
                if (IS_BOOL(value)) {
                    value.type = (value.type == VAL_FALSE) ? VAL_TRUE: VAL_FALSE;
                    push(&value);
                } else {
                    runtimeError("Can't convert not bool to bool.");
                    return INTERPRETER_RUNTIME_ERROR;
                }
                
                break;
            }

            case OP_AND: {
                boolOperator(OP_AND);
                break;
            }

            case OP_OR: {
                boolOperator(OP_OR);
                break;
            }

            case OP_PRINT: {
                Value* value = pop();
                printValue(value);
                break;
            }
            
            case OP_RETURN: {
                opReturn(false);
                break;
            }

            case OP_CLOSE: {
                Value* value = peek(1);
                if (value->type == VAL_FUNCTION)
                    closeUpValues(AS_CLOSURE(value->as.obj));
                
                changeCallFrameBack();
                break;
            }    
            
            case OP_NEGATE: {
                Value value;
                value.type = VAL_NUMBER;
                value.as.number = -(AS_NUMBER(pop()));
                push(&value);
                break;
            }

            case OP_DEFINE_GLOBAL: {
                // At this point we only need to set value, because
                // entry was already prepared at compile time.
                Entry* entry = findEntry(&vm.globals, READ_STRING());
                entry->value = *pop();
                break;
            }

            case OP_GET_GLOBAL: {
                ObjString* str = READ_STRING();
                if (vm.globals.size == 0) {
                    runtimeError("Undefined variable '%s'.", str->value);
                    return INTERPRETER_RUNTIME_ERROR;
                }

                Entry* entry = findEntry(&vm.globals, str);

                if (entry->key == NULL) {
                    runtimeError("Undefined variable '%s'.", str->value);
                    return INTERPRETER_RUNTIME_ERROR;
                }
                
                push(&entry->value);
                break;
            }
            
            case OP_SET_GLOBAL: {
                ObjString* str = READ_STRING();
                if (vm.globals.size == 0) {
                    runtimeError("Undefined variable '%s'.", str->value);
                    return INTERPRETER_RUNTIME_ERROR;
                }

                Entry* entry = findEntry(&vm.globals, str);

                if (entry->key == NULL) {
                    runtimeError("Undefined variable '%s'.", str->value);
                    return INTERPRETER_RUNTIME_ERROR;
                }

                entry->value = *pop();
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t pos = READ_BYTE();
                vm.callFrame->functionLocals[pos] = *pop();
                updatePosition(pos);
                break;
            }

            case OP_GET_LOCAL: {
                uint8_t pos = READ_BYTE();
                push(&vm.callFrame->functionLocals[pos]);
                updatePosition(pos);
                break; 
            }

            case OP_SET_UPVALUE: {
                *vm.callFrame->closure->upvalues[READ_BYTE()]->location = *pop();
                break;
            }

            case OP_GET_UPVALUE: {
                push(vm.callFrame->closure->upvalues[READ_BYTE()]->location);
                break;
            }

            case OP_GET_PROPERTY: {
                ObjInstance* instance = AS_INSTANCE(pop()->as.obj);
                ObjString* string = READ_STRING();

                if (instance->properties.size == 0) {
                    runtimeError("Instance don't have property named '%s'.", string->value);
                    return INTERPRETER_RUNTIME_ERROR;
                }

                Entry entry = *findEntry(&instance->properties, string);
                push(&entry.value);
                break;
            }

            case OP_SET_PROPERTY: {
                Value value = *pop();
                ObjInstance* instance = AS_INSTANCE(peek(1)->as.obj);
                ObjString* key = READ_STRING();
                
                Entry* entry = NULL;
                if (instance->properties.capacity > 0)
                    entry = findEntry(&instance->properties, key);
                if (entry == NULL) {
                    Entry newEntry;
                    newEntry.key = key;
                    newEntry.value = value;
                    addEntry(&instance->properties, &newEntry);
                } else {
                    entry->key = key;
                    entry->value = value;                
                }

                break;
            }

            case OP_SET_METHOD: {
                ObjClass* objClass = AS_CLASS(pop()->as.obj);
                Entry* entry = findEntry(&objClass->methods, READ_STRING());
                entry->value = *pop();
                break;
            }

            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(pop()->as.obj);
                ObjClosure* closure = newClosure(function);
                
                Value value;
                value.type = VAL_FUNCTION;
                value.as.obj = AS_OBJ(closure);
                push(&value);
                
                for (uint8_t i = 0; i < function->upvaluesCount; i++) {
                    UpValue* upvalue = function->upvalues[i];

                    if (upvalue->isLocal) {
                        closure->upvalues[i] = captureUpValue(vm.callFrame->functionLocals + upvalue->index);
                    } else {
                        closure->upvalues[i] = vm.callFrame->closure->upvalues[i];
                    }
                }

                break;
            }

            case OP_JUMP: {
                if (AS_BOOL((*peek(1))))
                    vm.callFrame->closure->function->ip += JUMP_BYTES;       // Go straight to instruction.
                else
                    vm.callFrame->closure->function->ip += bytesFusion(READ_BYTE(), READ_BYTE()) + 1;
                break;
            }

            case OP_JUMP_BACK: {
                vm.callFrame->closure->function->ip 
                    = &vm.callFrame->closure->function->chunk.chunk[bytesFusion(READ_BYTE(), READ_BYTE())];
                break;
            }

            case OP_JUMP_FOR: {
                if (AS_BOOL((*peek(1))))
                    vm.callFrame->closure->function->ip += bytesFusion(READ_BYTE(), READ_BYTE()) + 1;
                else {
                    vm.callFrame->closure->function->ip += JUMP_BYTES;   // Ignore assignement bytes.
                    vm.callFrame->closure->function->ip += bytesFusion(READ_BYTE(), READ_BYTE()) + 1;
                }
                break;
            }

            case OP_JUMP_IF_TRUE: {
                if (!AS_BOOL((*peek(1))))
                    vm.callFrame->closure->function->ip += JUMP_BYTES;       // Go straight to instruction.
                else
                    vm.callFrame->closure->function->ip += bytesFusion(READ_BYTE(), READ_BYTE()) + 1;
                break;
            }

            case OP_CALL: {
                call(READ_BYTE());
                break;
            }

            case OP_INVOKE: {
                // Add closure to stack.
                ObjInstance* instance = AS_INSTANCE(peek(1)->as.obj);
                ObjClass* objClass = instance->blueprint;

                ObjString* string = READ_STRING();
                if (!invoke(objClass, string)) {
                    runtimeError("Methods '%s' doesn't exists.", string->value);
                    return INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }

            case OP_THIS:
                // Change 'this' keyword with actual instance. 
                push(&vm.callFrame->functionLocals[FUNC_LOCALS_OFFSET]);
                break;
            
            case OP_INHERIT: {
                Value value = *pop();
                if (!IS_OBJ(value) || value.as.obj->type != OBJ_CLASS) {
                    runtimeError("Can't inherit from non class object.");
                    return INTERPRETER_RUNTIME_ERROR;
                }

                ObjClass* objClass = AS_CLASS(value.as.obj);
                ObjClass* childClass = AS_CLASS(pop()->as.obj);
                childClass->parent = objClass;

                break;
            }

            case OP_SUPER: {
                // Take instance and create new one with the same properties, 
                // but with parent's methods.
                ObjInstance* instance = AS_INSTANCE(vm.callFrame->functionLocals[FUNC_LOCALS_OFFSET].as.obj);
                ObjClass* parent = instance->blueprint->parent;
                if (parent == NULL) {
                    runtimeError("This instance doesn't have parent.");
                    return INTERPRETER_RUNTIME_ERROR;
                }

                ObjInstance* parentInstance = AS_INSTANCE(allocateObject(OBJ_INSTANCE));
                parentInstance->blueprint = parent;
                parentInstance->properties = instance->properties;

                Value value;
                value.type = VAL_INSTANCE;
                value.as.obj = AS_OBJ(parentInstance);

                push(&value);
                break;
            }

            case OP_POP:
                pop();
                break;

            default: {
                if (vm.callFrame->nextFrame != NULL)
                    opReturn(true);
                else
                    return EXECUTION_SUCCESS;            
            }
        }
#undef READ_BYTE
#undef READ_STRING
#undef READ_FUNCTION
#undef BOOL_BINARY_OP
#undef BINARY_OP
    }
}