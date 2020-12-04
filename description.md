## Value
* properties
    - `type`                - value type (e.g. number, string, bool, etc.).
    - `as`                  - value gor given type.
        - to be able to store values in the same memory location union is used.

## VM
* properties
    - `stack`               - meant for maintaining implementation flow.
    - `stackTop`            - meant for keeping track on one-past the end element.
    - `internedStrings`     - ObjString array.
    - `stringCount`         - amount of interned strings. Serves as index and size indicator.
    - `locals`              - scope local values.

* [WIP]void runtimeError(const char* format, ...)
    - description
        - deal with runtime errors.

* ObjString* internString(ObjString*)
    - description
        - Keep track on strings to not duplicate them.
        - Based on this comparitions of strings can be done effortlessly.
        - if string exists delete passed one, else add new one and return it.

* ExecutionResult
    - description
        - indicator (enum) of execution result.

* ExecutionResult run()
    - Iterates over compiled instructions and handles them correspondingly.
    - To dispatch to required handler it considers instruction/op code.


* BINARY_OP(op)
    - description
        - takes operator and does changes to stack by computing binary expressions.
    - design notes
        - why macro?        - To be able to deal with different operators.
        - why `do while` ?  - First of all to have block expression and at the same time
                            being able to use semicolon (;) afterwards.
* BOOL_BINARY_OP(op)
    - description
        - the same as BINARY_OP, except this one handles binary operatos (>, <, ==).

## Compiler + Chunk
* Chunk properties
    - `chunk`       - instructions.
    - `size`       - amount of bytes.
    - `capacity`   - space to hold bytes.

* Compiler properties
    - `Chunk`       - Chunk (see above).
    - `ip`          - instruction pointer.
    - `constants`   - some instructions require values, so this DS will hold it.
    - `panicMode`   - should be true when compiler encounters error.
    - `table`       - hash table for variables.
    - `locals`      - Local objects array. Meant for local variables (neasted in scopes).
    - `scopeDepth`  - in which scope are we right now ?
    - `localsCount` - amount of locals currently being stored in _locals_.

* Local
    - `name`        - Token, which stores identifier's name.
    - `scopeDepth`  - current variables scope.

* void scopeStart()
    - description: increments compiler::scopeDepth.

* void scopeEnd()
    - description
        - decrements compiler::scopeDepth.
        - 'removes' locals - actually its just decrements compiler::localsCount 
        to let next locals to replace them at their location.

* void recover()
    - description
        - consume all tokens until EOF or semicolon (;).
        - change ip to point to the end of instructions (chunk.chunk).
        - set `panicMode` to true.

## Debugger
- Just prints info about:
    - instructions while instructions are being processed by `run()` function (a.k.a. interpreter).
    - tokens while tokens are being processed by `compile()` function (a.k.a parser).


## Memory
* ALLOCATE
    - arguments
        - type
        - pointer
        - amount
    - functionality
        - allocate required amount of space (being calculated based on type).
        - convert void* to type*.

* allocate
    - arguments
        - pointer
        - size
    - functionlity
        - either allocate 8 * size or 2 * size.

* reallocate
    - resize space:
        1) free space       size == 0
        2) make smaller     size < space
        3) make bigger      size > space


### Instruction effect on Stack (vm)
* OP_FALSE, OP_TRUE, OP_NIL, OP_CONSTANT
    - effect: +1
    - description: add constant to stack either from _constants_ or explicitly from _bytecode_.

* OP_PRINT
    - effect: -1
    - description: pop value from stack and print it.

* OP_ADD, OP_SUBTRACT, OP_DIVIDE, OP_MULTIPY
    - effect: -1
    - description: pop 2 values from stack, 
    implement required operation and push result back.

* OP_NEGATE
    - effect: 0
    - pop value from stack, negate it, push back to stack.

* OP_DEFINE_GLOBAL
    - effect: -1 || 0
    - pop value (if any) from stack and initialize variable in hash table.

* OP_GET_GLOBAL
    - effect: +1
    - push value to stack, for including in expressions.

* OP_SET_GLOBAL
    - effect: -1
    - pop value from stack to change (assign) existing variables value.

* OP_AND, OP_OR, OP_GREATER_THAN, OP_LESS_THAN, OP_EQUAL_EQUAL
    - effect: -1
    - _pop two_ values from stack to _combine_ their values via _boolean opeator_,
    then _push result_.

### Scanner
* protperties
    - `sourceCode`      - self-explanatory.
    - `position`        - index to current character.
    - `size`            - length of source code.
    - `lineNumber`      - character's line nubmer. This is meant for errors.
    
#### Token
* properties
    - `type`            - enum.
    - `string`          - meant for strings, identifiers, etc.
    - `number`          - meant for numbers.
    - `lineNumber`      - meant for erros.

* bool advance(int)
    - description
        - traverse scanner via position.
    - retrn value       
        - Are we at the end of sourceCode or not.
    - arguments
        - amount to go forward of backward.

* char getChar(bool)
    - description
        - get next character from scanner and get rid of whitespaces.
    - arguments
        - do we need to consume whitespaces or not.
    - return value
        - desired character.
    
### Parser
* properties
    - `previous`        - previous Token.
    - `current`         - current Token.

* Precedence
    - description: meant for traversing tokens based on priority (a.k.a tight binding).

* void parsePrecedence(Precedence)
    - description
        - parse expression based on precedence (see above).
    - arguments
        - precedence of token.
    - return
        - nothing.

* ParseFn
    - description
        - pointer to parser functions (e.g. binary, number, etc.).

* ParseRule: properties
    - `prefix`          - such as "unary minus", "number", etc..
    - `infix`           - such as "binary", "grouping", etc..
    - `precedence`      - Precedence to apply traversing sequence.

* void unary(bool canAssign)
    - description
        - negare number.

* void binary(bool canAssign)
    - description
        - call parsePrecedence with +1 precedence to search for higher operations to handle.
        - emit binary operators to bytecode.
    
* void grouping(bool canAssign)
    - description
        - handles (expression), while being prioritized againset some operations 
        (e.g. litterals, arithmetics, etc.)

* expression()
    - description
        - imply grammer on expressions.
    - examples
        - addition, multiplication, grouping, unary, etc..

* statment()
    - description
        - imply grammer on statements.
    - examples
        - print, if-then, while, etc..

* declaration()
    - description
        - imply grammer on declarations.
    - examples
        - function, variable, class, etc..

* void block(bool canAssign)
    - description
        - handle { statements }.
        - statements in curly braces ({}) are being handled via this 'grammer rule'.
    - implementation flow:
        1) start scope by calling `scopeStart()`.
        2) handle all statements by terating through them.
        3) stop if end of scope is reached (i.e. '}' is found).
        4) end scope by calling `scopeEnd()`.

### Table and Entry
* Entry::properties
    - `key`             - seach names via this.
    - `value`           - payload.
    - `hash`            - make search faster via hashed key.
    - `tombstone`       - meant for deletion.

* Table::properties
    - `entries`         - array of Entry.
    - `size`            - amount of entries.
    - `capacity`        - amount of space for entries.

* double hashString(const char* key)
    - description
        - hash string by considering every character in it.

* Entry* findEntry(Table* table, const char* key, uint32_t hash)
    - description
        - seach Entry in table in _linear probing_ style.

* void addEntry(Table* table, Entry* entry)
    - description
        - use _findEntry_ function to find position.
        - add entry to table.
        - grow table if it's near to max load space (in our case 0.75).

## Objects
* Obj::properties
    - `type`            - type of object.

* ObjString::properties
    - `obj`             - to be able to use polymorphism.
    - `string`          - payload.
    - `hash`            - string hash.