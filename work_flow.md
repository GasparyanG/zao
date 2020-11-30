# ZaO (Zero and One Programming Language)

## Agenda
### 11/20/2020
- [x] 22:50 - 23:50       - Divide work into days
- [x] 23:50 - 00:50       - Drow map of territory
- [x] 00:50 - 01:20       - Think about development manner
    - Design first, but try to keep minimalistic by spending only 10 - 30 minutes
    - Describe what have you done and what should be done next every day,
    and keep that records, to have a look later.
- [x] 01:20 - 01:50       - Think about Git usage
    - Work with git, by trying to explin stuff, but not moer than 2-3 sentences
- [x] 01:50 - 03:45       - Operation's stack effect and check today work's correctness with actual implementation.

### 11/23/2020
- [x] 9:30 - 10:30       - Design
- [x] 10:30 - 12:30       - Compiler and Chunk
    - Dynamic array
- [x] 12:30 - 13:30       - VM
    - Stack
    - Interpreter - OP_PRINT
- [x] 13:30 - 14:30       - Debugger
    - Print opCode

### 11/24 - 25/2020
- [x] 09:10 - 10:00      - Design.
- [x] 10:00 - 11:00      - Read source code from stream.
- [x] 11:00 - 12:00      - Prepare regular tokens.
- [x] 12:00 - 13:00      - Prepare keywords.
- [x] 13:00 - 14:00      - Implement `consume`, and `advance` functions.

### 11/26 - 27/2020
- [x] 09:10 - 10:00      - Design.
- [x] 10:00 - 11:00      - Vaughn Pratt's table.
- [x] 11:00 - 12:00      - Expression handling.
- [x] 12:00 - 13:00      - Error handling.

### 11/28 - 29/2020
- [x] 11:00 - 12:30      - Design.
- [x] 12:30 - 13:30      - Hash table, Entry, hash function.
- [x] 13:30 - 14:30      - Find, insertion, deletion.
- [~] 14:30 - 15:30      - Declaration, definition, assignement.


### ZaO Implementation Path
1) Set Up Virtual Machine
    1.1) instruction pointer
    1.2) constant table
    1.3) Stack processing (single-pass compiler)

2) Recognize Characters
    2.1) Token
    2.2) String
    2.3) Comments
    2.4) etc.

3) Generate Bytecode
    3.1) Deal with grammer
    3.2) Voughn Pratt's parser
        3.2.1) infix, prefix, precedence
        3.2.2) expressions

4) Variables
    4.1) Bytecode litterals (bool, NULL)

5) Hash Table
    5.1) Hash function
    5.2) Linear probbing
    5.3) Tombstons
    5.4) Getter and setter

6) String
    6.1) Tagged union
    6.2) String Interning

7) Scopes
    7.1) Local
        7.1.1) Scope depth
    7.2) Locals

8) Statemtns
    8.1) condition flow
        8.1.1) if, else
    8.2) loops
        8.2.1) while
        8.2.2) for
    
9) Function
    9.1) First-level function
        9.1.1) Chained compilers
    9.2) Function name, function arguments, function body
        9.2.1) Function frame
    9.3) Fucntion call
        9.3) Call stack

10) Closure
    10.1) Upvalues
        10.1.1) Open upvalues (in stack)
        10.1.2) Closed upvalues (in heap and not in stack)

11) Garbage collector
    11.1) mark-and-sweep
        11.1.1) tri-color, work-list approache
    
12) Classes and Inctances
    12.1) Class Object
    12.2) Inctance Object
    12.3) Fields/Prperties
        12.3.1) Getter and setter

13) Initialization and Methods
    13.1) Methods
        13.1.1) Hash table for Class Object to keep methods
    13.2) _init_ method
    13.3) Optimization

14) Inheritance
    14.1) Inherit methods
        14.1.1) Merthod overriding
    14.2) Super
        14.2.1) Access parent methods
    14.3) Optimization
