# ZAO
ZAO is a multi-paradigm programming language.

## Table of Contents
* [Instalation and Setup](#instalation-and-setup)
* [Syntax](#syntax)
    - [Comments](#comments)
    - [Data Types](#data-types)
        - [Numbers](#numbers)
        - [String](#string)
        - [Bool and `NIL`](#bool-and-nil)
    - [`print`](#print)
    - [Operators](#operators)
        - [Arythmetic](#arythmetic)
        - [Comparision](#comparision)
        - [Logical](#logical)
    - [Variables](#variables)
    - [Scopes](#scopes)
    - [Statements](#statements)
        - [`if` and `else`](#if-and-else)
        - [`while` loop](#while-loop)
        - [`for` loop](#for-loop)
    - [Functions](#functions)
    - [Closures](#closures)
    - [Classes](#classes)
        - [Properties and Methods](#properties-and-methods)
        - [`this` and `init`](#this-and-init)
        - [Inheritance and `super`](#inheritance-and-super)
* [The End](#the-end)

## Instalation and Setup
### Instalation
To clone repository, issue this command:
```sh
git clone https://github.com/GasparyanG/zao.git
```
#### Setup
To setup ZAO, issue this command:
```sh
make
```

#### Usage
To be able to use terminal for ZAO, issue this command:
```sh
./zao
```

To be able to compile and execute file of ZAO, issue this command:
```sh
./zao (file name)
```

Here you should replace `(file name)` with actual file.

## Syntax
### Comments
ZAO only has single-line comments, and to write one we should use double forward slash (`//`). 
```php
// Comments are important part of a good software.
```

The above code will not be _processed_, because it's only required to _describe_ and not to affect the program in any way.
___
### Data Types
#### Numbers
ZAO has two type of numbers:
* **Integers**, which are _whole_ numbers.
```php
-6, -1666 // Negative integers.
0         // You may not see, but this is a zero :D.
5, 1555   // Positive integers.
```
* **Doubles**, which are _floating point_ or _rational_ numbers.
```php
-7.7, -10.10, -100.001 // Negative doubles.
0                      // Again this thing, but don't warry soon you will get used to it.
6.6, 16,16, 166,661    // Positive doubles.
```

#### String
In ZAO string is a collection of characters enclosed in double quotes (`""`).
```php
""                      // Empty string.
"c"                     // String containing single character.
"This is a string."     // String with multiple characters.
```
#### Bool and `NIL`
To represent **boolean logic**, ZAO uses traditional `true` and `false`, while to use **'not a value'** it uses `NIL`.
```php
true
false
NIL
```


### `print`
To be able to output something to standard output ZAO uses `print` keyword.
```php
> print "Hello, World!";

// The above code will output: 
Hello, World!

> print 5;
// This one will output:
5
```

So far we have covered everything about above code except _semicolon_ (`;`). This sign notifies _compiler_ about the end of _an expression_. Once _compiler_ encounters `;` it will evaluate the expression, which comes before that sign.
___
### Operators
#### Arythmetic
* `+` - **Addition** 
```php
> print 16 + 9;
25
```
* `-` - **Subtraction**
```php
> print 169 - 25;
144
```
* `*` - **Multiplication**
```php
> print 11 * 12;
132
```
* `/` - **Division**
```php
> print 19 / 2;
9.5
```
#### Comparision
* `>` and `>=` - **Greater than** and **greater than or equal to**
```php
> print 5 > 5;
false

print 5 >= 5;
true
```
* `<` and `<=` - **Less than** and **less than or equal to**
```php
> print 5 + 6 < 144 / 12;
true

> print (12 + 13) <= 625 / 25;
true
```

* `==` and `!=` - **Equal** and **not equal**
```php
> print (8 + 7)*15 + 2 == 225;
false

> print true != false;
true
```

The most interesting part here is an exclamation sign (`!`) in combination with equal sign (`=`).

As you already understood `==` just checks whether operators are equal to each other or not. To see that they are **not** equal to each other we are using **negation** (i.e. `!` sign).

`!` can also negate a boolean expression.
```php
> print !false;
true

> print !(5 > 4);
true
```

#### Logical
* `and`
```php
> print false and true;
false

// You can remove parentheses if you want.
> print (11 + 22 < 33 + 44) and (55 + 66 < 77 + 88);
true
```

* `or`
```php
> print false or true;
true

// I know you will struggle to find the difference, so let me make your life easier. 
// First expression uses '>' instead of '<' and 'and' is changed to 'or'.
> print (11 + 22 > 33 + 44) or (55 + 66 < 77 + 88);
true
```
___

### Variables
To define a variable we should use `var` keyword.
```php
> var myFirstVariable = "Finally, I'm alive!";

> print myFirstVariable;
Finally, I'm alive!
```

ZAO is a dynamic language, which means that we can store any data type we want in a variable without specifying its type.
```php
> var universal = 15;
> print universal;
15
> universal = "pictures";
> print universal;
pictures
```

**_NOTE_**
> ZAO has three ways to deal with variables:
> 1) **declaration:** just use `var` and variable name. After declaration variabl's value will be `NIL`.
>```php
>> var age;
>> print age;
>NIL
>```
> 2) **definition:** use `var`, variable name, initializer.
>```php
>> var age = 24;
>> print age;
>24
>```
>3) **assignement:** after variable _declaration or defintion_ you can _assign_ new value to it by just using variable name without `var` keyword.
>```php
>> var age = 24;
>> print age;
>24
>> age = 25;
>print age;
>25
>> name = "John";
>Undefined variable 'name'.
>```
> If you will try to use a variable which is not _defined or declared_ yet, then _interperter_ will throw an error.

___
### Scopes
In ZAO scopes are being defined via curly braces (`{}`).
```php
var batMan = "Ben Affleck";

{
    var batMan = "Christian Bale";
    print batMan; // Christian Bale
}

print batMan;     // Ben Affleck

{
    batMan = "Adam West";
}

print batMan;     // Adam West
```

**Let me explain what is going on here.**

At this point scopes worth explaining because of varibles, and to do that lets understand what is a _"life span"_ of a variable.

Variables can _"live forever"_(untill the end of the program) or their _"life"_ can be short depending on the scope they are defined in.

If varible is defined in a **global** scope (i.e. not bounded with `{}`), then it will _"live forever"_, but if it's defined in a **local** scope (i.e. bounded with `{}`), then it will be destroyed at the end of that scope(measured by _scope depth_).

**Now lets have a look at above code.**

First variable declaration is being overriden by **local** scope's variable definition, but fortunately for us that definition is using **local** scope's storage. That's why **global** scope's variable is not changed.
```php
var batMan = "Ben Affleck";

{
    var batMan = "Christian Bale";  // Local definition.
    print batMan; // Christian Bale
}   // This cope's batMen will be destroyed here.

print batMan;     // Ben Affleck
```

The second **local** scope doesn't define its own varaible, but instead it uses the **global** scope's varaible, and for that reason our variable (**global** scope's _batMan_) is being changed.

```php
var batMan = "Ben Affleck"; // Global.

... // Some code here.

{
    batMan = "Adam West";   // Use global batMan.
}

print batMan;     // Adam West
```
___
### Statements
#### `if` and `else`
To be able to control flow of a program we can use conditional statements.
```php
if (true) {
    print "this statement is true";
} else {
    print "this case can't be accessed";
}

this statement is true
```

`if` accepts _expression_, which will be evaluated to `true` or `false`. If that expression is `true` then `if`'s block will be handled, otherwise `else`'s block is accessed.

```php
var teslaSpeed = 250;
if (!(10 < 11) or teslaSpeed != 250) {
    print "if is accessed";
} else {
    print "else is accessed";
}

else is accessed
```

#### `while` loop
Next popular control flow of languages is `whlie` statement, whcih is meant for implementing the same block of code over and over again until `while`'s termination. This termination depends on expression which `while` evaluates at every loop. If expression's result is `true` then implement `while`'s block of code otherwise exit `while` statement.

```php
var limit = 10;
var cycle = 0;
while (cycle < limit) {
    print "hi";
    cycle = cycle + 1;      // Without this line while will not terminate.
}

...
hi  // 'hi' will be printed 10 times.
```


#### `for` loop
`for` statement found its place in almost all programming languges, but I like to think about it as syntactical sugar over `while` statement. Why do I think like that? Well, lets find out together.

```php
var limit = 10;
for (var cycle = 0; cycle < limit; cycle = cycle + 1) {
    print "hi";
}

...
hi  // 'hi' will be printed 10 times.
```

As you can see this code is shorter(3 lines against 5 lines) and more organized than `while`'s code, but the functionality is exactly the same.


**This layout of both statements will make it clear:**

>**declaration**
>
>`while`(**expression**) { **increment** ... }

> `for` (**declaration** | **expression** | **increment**) { ... }

___
### Functions
>If you have to spend effort into looking at a fragment of code to figure out what it's doing, then you should extract it into a function...
>
> --- **Martin Fowler**

Function is a piece of code that once defined can be called later.
```php
fun sum(a, b) {
    return a + b;
}

print sum(5, 6);  // This prints '11'.
```

**To define function we need to use:**
1) **`fun`**            - this keyword will notify compiler that we intended to define function.
2) **fucntion name**    - this is required to be able to refer back to piece of code that this function defines.
3) **argument list**    - function can accept arguments, which will be replaced with parameters, that this function defines in its **local** scope (in our case _a_ and _b_).
4) **function block**   - all of this instructions defined in function's block will be implemented once function is called.
5) **`return`[optional]** - `return` keyword returns result of an expression to caller. If `return` is not used function will return `NIL`.

> `fun` **function name**( **argument list** ) { **function block** }

We can define neasted functions.
```php
fun multAndSum(a, b, c, d) {
    fun mult(e, f) {
        return e * f;
    }

    fun sum(g, h) {
        return g + h;
    }

    print sum(mult(a, b), mult(c,d));
}

multAndSum(1,2,3,4); // This prints '14'.
```

We can assign function to variable.
```php
fun sum(a, b) {
    print a + b;
}

var addition = sum;
addition(1,2);      // This prints '3'.
```

We can call function recursively.
```php
fun fib(n) {
    if (n <= 1) {
        return 1;
    }

    return fib(n-2) + fib(n-1);
}

print fib(5);   // This prints '8'.
print fib(6);   // This prints '13'.
```

___
### Closures
Take a look at this code.
```php
fun foo() {
    var callerName = "foo";
    fun fuu() {
        print callerName;
    }

    print fuu();
}

foo(); // This prints 'foo'.
```

This is a normal case, when we define function inside of another one and call neasted function, which uses "parent" function's **local** variable.


Now imagine that we need to use that same neasted function outside of its "parent".
```php
fun foo() {
    var callerName = "foo";
    fun fuu() {
        print callerName;
    }

    return fuu();
}

var fuu = foo();
fuu(); // This prints 'foo'.
```

Returned function is called **closure**, and as you can see there is no problem to use "parent's" variable outside of its _declaration scope_. We are able to do this because neasted function _"closes"_ (thus **closure**) parents variable within itself (for more info search of **closure upvalues**.).

___
### Classes
Classes are one way to define custom data types and use OOP perks.

#### Properties and Methods
In ZAO classes can have both **properties** and **methods**.
```php
class Bird {
    fun fly() {
        print this.gender;
        print "Bird is flying.";
    }
}

var bird = Bird();
bird.gender = "female";

// This will print:
// female
// Bird is flying.
bird.fly();
```

**Explanation time**
1) **`class`**          - to be able to define _class_ we should say so by using `class` keyword.
2) **class name**       - like in functions we need to be able to reference to class and to do that we have to specify a class name.
3) **class methods[optional]** - classes can have their own methods and to define one just use **function**'s syntax.
4) **Instance propety[optional]**  - although I mentioned that classes can have properties, but actually properties only belong to instances created from that classes.

For `this`'s explanation see [`this`](#this-and-init).

To be able to create instance we just need to use **call** syntax.
```php
class Human {}
var doctor = Human();   // Create 'doctor' instance from Human.
var driver = Human();   // Create 'driver' instance from Human.
```

Once instance is created it can have its own properties by using **dot** syntax.
```php
class Human {
    fun sayName() {
        print this.name;
    }
}

var doctor = Human();
doctor.name = "Doctor Strange";

var driver = Human();
driver.name = "Michael Schumacher";

doctor.sayName();   // This prints 'Doctor Strange'.
driver.sayName();   // This prints 'Michael Schumacher'.
```

As you can see after _instantiation_ these instances are using the same method, but the values of their properties are different.

#### `this` and `init`
Once we define our properties we would like to use them inside methods without having to specify property's instance. Fortunately for us we have `this` keyword which will **point** to desired instance (instance which is calling method).

```php
class Car {
    fun showSpeed() {
        print this.speed;
    }
}

var ferrari = Car();
ferrari.speed = 218;

var lamborghini = Car();
lamborghini.speed = 217;

ferrari.showSpeed();        // 'this' will refer to 'ferrari'.
lamborghini.showSpeed();    // 'this' will refer to 'lamborghini'.
```

Now that we have properties it's logical to be able to initialize them at the instantiation phase rether that after it. For that in ZAO we are using `init` "magic" method, which will be called implicitly once we instantiate some object.

```php
class Teacher {
    fun init(name, salary, subject) {
        this.name = name;
        this.salary = salary;
        this.subject = subject;
    }

    fun introduce() {
        print this.name;
        print this.salary;
        print this.subject;
    }
}

var mathematician = Teacher("Leonhard Euler", 500, "math");
var physicist = Teacher("Albert Ainstein", 700, "physics");
var philosopher = Teacher("Plato", 750, "philosophy");

// For sake of simplicity I will display only subjects,
// but every instance will display its own property's value.
mathematician.introduce();  // This prints 'math'.
physicist.introduce();      // This prints 'physics'.
philosopher.introduce();    // This prints 'philosophy'.
```

#### Inheritance and `super`
Saying that programming language supports OO paradigm, but leaving it without inheritance, is one of 7 sins, so here we are.

To be able to inherit from _parent class_ use this syntax:
> `class` **class name** `inherits` **parent name** { ... }

```php
class Weapon {
    fun init(name, strength) {
        this.name = name;
        this.strength = strength;
    }

    fun introduce() {
        print this.name;
        print this.strength;
    }
}

class Sword inherits Weapon {}
class Gun inherits Weapon {}

var katana = Sward("katana", 5);
var aug = Gun("AUG", 8);

katana.introduce();     // This will print 'katana' '5'.
aug.introduce();        // This will print 'AUG' '8'.
```

As you can see **Sward** and **Gun** are using methods which is defined in **Weapon**. This can be achieved by using `inherits` keyword which will notify compiler that we are dealing with inheritance.

After method _invocation_ current instance will try to find called method and if it's not found in current class then that method will be searched in an _inheritance chain_(classes which is being referenced by their childrens. In this case **Weapon** is being referenced by **Sword** and **Gun**).

**There are two questions left to answer, and after that we are done.**

> What if child class defines method with the same name as its parent does?

Well, this is called **overriding** and in this scenario child's method will be prioritized.

```php
class A {
    fun foo() {
        print "A's foo is called.";
    }
}

class B inherits A {
    fun foo() {
        print "B's foo is called.";
    }
}

var b = B();
// B's 'foo' overrides A's 'foo'.
b.foo();    // This prints "B's foo is called.".
```

> What if we want to use **overriden** method?

To be able to access **overriden** method we should use `super` keyword, which will reference to parent's methods.

```php
class A {
    fun foo() {
        print "A's foo is called.";
    }
}

class B inherits A {
    fun foo() {
        print "B's foo is called.";
        super.foo();
    }
}

var b = B();
// B's 'foo' overrides A's 'foo'.

// This prints 
// "B's foo is called.".
// "A's foo is called.".
b.foo();
```

## The End
Programming languages are very interesting to use but I found myself more exited to implement them, so if you think that this kind of stuff interests you, then you should definitely checkout [CRAFTING INTERPRETERS](http://craftinginterpreters.com/), which was a huge motivation for me to create my own programming language.

I hope, that you learnt something and I wish you the best, bye.