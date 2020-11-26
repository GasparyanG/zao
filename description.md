## VM
* properties
    - `stack`     - meant for maintaining implementation flow.
    - `stackTop`  - meant for keeping track on one-past the end element.

* `run()`
    - Iterates over compiled instructions and handles them correspondingly.
    - To dispatch to required handler it considers instruction/op code.


## Compiler + Chunk
* Chunk properties
    - `chunk`       - instructions.
    - `size`       - amount of bytes.
    - `capacity`   - space to hold bytes.

* Compiler properties
    - `Chunk`       - Chunk (see above).
    - `ip`          - instruction pointer.
    - `constants`   - some instructions require values, so this DS will hold it.


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
* OP_PRINT
    - effect: -1
    - description: pop value from stack and print it

* OP_ADD, OP_SUBTRACT, OP_DIVIDE, OP_MULTIPY
    - effect: -1
    - description: pop 2 values from stack, 
    implement required operation and push result back.

### Scanner
* protperties
    - `sourceCode`      - self-explanatory.
    - `position`        - index to current character.
    - `size`            - length of source code.
    
#### Token
* properties
    - `type`            - enum.
    - `string`          - meant for strings, identifiers, etc.
    - `number`          - meant for numbers.

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

* void binary()
    - description
        - call parsePrecedence with +1 precedence to search for higher operations to handle.
        - emit binary operators to bytecode.