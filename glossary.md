### Questions
- [x] What kind of role does integers play in program exit/termination ?
- [] What does happen with values which is being replaced in array with aother values just via subscript operator ?
- [x] Get comfortable with pointer arithmetics, especially with pointers difference.
- [x] What is the name of DS against which keywords are being compared ?
    - Trie.
- [] Why doesn't ALLOCATE macro worked for Entry allocation ?
- [] How does  CPU cache works ?
- [] What is Valgrind ?
- [] How to handle value update in vm.c ?
    - hint 1: change return value of pop from `Value*` to `Value`.
    - hint 2: change argument value of push from `Value*` to `Value`.

### To Do
- [] memory.c   : exit when pointer is not null.
- [] vm.c       : display `empty stack` error.
- [] vm.c       : display `stack overflow` error.
- [] compiler.c : display `constants table overflow` error.
- [] scanner.c  : display `unknown token` error.
- [~] scanner.c  : deal with other tokens too.
- [~] debug.c    : implement other tokens debugging too.
- [] compiler.c : display `EOF or some kind of error` error
- [x] compiler.c : display `prefix function dosn't exists` error
- [] compiler.c : think this part through.
- [x] table.h    : introduce StrObj which will store key and hash.
- [x] table.c    : handle tombstones.
- [] table.c    : create function in memory.h to handle allocation in one place
- [x] compiler.c : compute hash.
- [] vm.c       : handle string stack overflow error properly.
- [] cm.c       : show line of error.
- [x] compiler   : think about a + b = c + d expression's compilation.
- [] vm.c       : you will feel that Value* is wrong in here, so chagne it to Value.
- [] compiler.c : implement declaration as well (this is just initialization).
- [x] vm.c       : implement bang case.

### In Progress

### Done
