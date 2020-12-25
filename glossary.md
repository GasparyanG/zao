### Questions
- [x] What kind of role does integers play in program exit/termination ?
- [x] What does happen with values which is being replaced in array with aother values just via subscript operator ?
- [x] Get comfortable with pointer arithmetics, especially with pointers difference.
- [x] What is the name of DS against which keywords are being compared ?
    - Trie.
- [] Why doesn't ALLOCATE macro worked for Entry allocation ?
- [] How does  CPU cache works ?
- [x] What is Valgrind ?
- [x] How to handle value update in vm.c ?
- [] Why I couldn't use pointers explicitly in debug.c (jumpInstruction) ?

### To Do
- [x] memory.c   : exit when pointer is not null.
- [x] vm.c       : display `empty stack` error.
- [x] vm.c       : display `stack overflow` error.
- [x] compiler.c : display `constants table overflow` error.
- [] scanner.c  : display `unknown token` error.
- [x] scanner.c  : deal with other tokens too.
- [x] debug.c    : implement other tokens debugging too.
- [] compiler.c : display `EOF or some kind of error` error
- [x] compiler.c : display `prefix function dosn't exists` error
- [] compiler.c : think this part through.
- [x] table.h    : introduce StrObj which will store key and hash.
- [x] table.c    : handle tombstones.
- [~] table.c    : create function in memory.h to handle allocation in one place
- [x] compiler.c : compute hash.
- [] vm.c       : handle string stack overflow error properly.
- [] vm.c       : show line of error.
- [x] compiler   : think about a + b = c + d expression's compilation.
- [x] vm.c       : you will feel that Value* is wrong in here, so chagne it to Value.
- [x] compiler.c : implement declaration as well (this is just initialization).
- [x] vm.c       : implement bang case.
- [] compiler.c : why does 1 is being subtracted ?
- [] compiler.c : why does 0 is being converted to 8 ?
- [x] compiler.c : for optimization same, implement short circuit.
- [] compiler.c : pass identifier error message.
- [] compiler.c : extract method initCompiler -> initFunction.
- [x] vm.c       : think about initialization arguments.
- [x] table.c    : entries are repeating. To reproduce run test/table.zao
- [x] vm.c       : handle method absence.

### In Progress

### Done
