#include <stdio.h>

#include "vm.h"

VM vm;

void initVM() {
    vm.stackTop = vm.stack;
    vm.stringCount = 0;
}

ObjString* internString(ObjString* strToCmp) {
    for (uint32_t i = 0; i < vm.stringCount; i++) {
        if (strToCmp->hash == vm.internedStrings[i]->hash) {
            free((char*)strToCmp->value);
            free(strToCmp);
            return vm.internedStrings[i];
        }
    }

    if ((vm.stringCount + 1) >= UINT8_MAX) {
        // TODO: handle string stack overflow error properly.
        printf("String stack overflow.\n");
        exit(1);
    }

    vm.internedStrings[vm.stringCount++] = strToCmp;
    return strToCmp;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // TODO: show line of error.
    // vm.stackTop = vm.stack;
    // compiler.ip = &compiler.chunk.chunk[compiler.chunk.capacity];
}

Value* pop() {
    // TODO: display `empty stack` error.
    return --vm.stackTop;
}

void push(Value* value) {
    if ((vm.stackTop - vm.stack) > UINT8_MAX);
        // TODO: display `stack overflow` error.
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

ExecutionResult run() {
    for (;;) {
#define READ_BYTE()   *compiler.ip++
#define READ_STRING() AS_STRING(compiler.constants[(*compiler.ip++)].as.obj)
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
    displayInstruction(compiler.ip);
#endif 
        if (*compiler.ip == OP_NONE)
            return EXECUTION_SUCCESS;

        switch(*compiler.ip++) {
            case OP_CONSTANT:
                push(&compiler.constants[READ_BYTE()]);     
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
                } else
                    runtimeError("Can't convert not bool to bool.");
                
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
            case OP_RETURN:
                printf("OP_RETURN\n");
                break;
            case OP_NEGATE: {
                Value value;
                value.type = VAL_NUMBER;
                value.as.number = -(AS_NUMBER(pop()));
                push(&value);
                break;
            }

            case OP_DEFINE_GLOBAL: {
                Entry* entry = findEntry(&compiler.table, READ_STRING());
                entry->value = *pop();
                break;
            }

            case OP_GET_GLOBAL: {
                ObjString* str = READ_STRING();
                Entry* entry = findEntry(&compiler.table, str);

                if (entry->key == NULL) {
                    runtimeError("Undefined variable '%s'.", str->value);
                    return INTERPRETER_RUNTIME_ERROR;
                }
                
                push(&entry->value);
                break;
            }
            
            case OP_SET_GLOBAL: {
                ObjString* str = READ_STRING();
                Entry* entry = findEntry(&compiler.table, str);

                if (entry->key == NULL) {
                    runtimeError("Undefined variable '%s'.", str->value);
                    return INTERPRETER_RUNTIME_ERROR;
                }

                entry->value = *pop();
                break;
            }

            case OP_SET_LOCAL:
                vm.locals[READ_BYTE()] = *pop();
                break;

            case OP_GET_LOCAL:
                // TODO: you will feel that Value* is wrong in here, so chagne it to Value.
                push(&vm.locals[READ_BYTE()]);
                break; 

            default:
                return EXECUTION_SUCCESS;
        }
#undef READ_BYTE
#undef READ_STRING
#undef BOOL_BINARY_OP
#undef BINARY_OP
    }
}