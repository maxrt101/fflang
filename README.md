# fflang
FF is an interpreted high-level functional programming language.  
This FF implementation is an interpreter-compiler, which supports  
REPL mode and source file parsing.  

## To run:
 - Clone/download repo
 - run `make`
 - run `./ff`

## Basics
FF supports `Null`, `Bool`, `Number` and `String` datatypes.  
`+`, `-`, `*`, `/` and `=` operators are supported.  
Variable declaration is done with `var` keyword.  
Consts are immutable variables. Declared with `const` keyword.  
`if`/`else` control structures are supported. Syntax is the same as in C, C++ or Java.  
`while` and `for` can be used. Syntax is the same as in C.  
`break` and `continue` are supported. Usage is the same as in C.  
User defined functions are supported. They can be defined with `fn` keyword.  

## Extending the capabilities
Native C/C++ functions are supported. All native functions must return `Value` and take `(int, Value*)` as parameters.
Where the first parameter is typically called `argc` for argument count, and the second - `args` for arguments.
Each function must return something, if you don't have anything to return, just return `null` with `Value(VAL_NULL)`.

## Supported features
 - [X] Integral data types: `Null`, `Bool`, `Number`.
 - [X] Built in `String` datatype.
 - [X] Expressions.
 - [X] `+`, `-`, `*`, `/` operators.
 - [X] Global variables, declared with `var` keyword.
 - [X] Scopes and locals.
 - [X] Constants, declared with `const` keyword.
 - [X] `print` statement.
 - [X] `if/else`.
 - [X] `and` and `or` operators.
 - [X] `while` statement.
 - [X] `for` statement.
 - [X] `break` and `continue` keywords.
 - [X] Functions, declared with `fn` keyword.
 - [X] Native functions with suitable api.
 - [ ] Anonimous functions (lambdas)
 - [ ] Classes and instances.
 - [ ] Arrays and lists. (either like native classes or like separate obj types)
 - [ ] Standart library
 - [ ] Operator overloading
 - [ ] Async functions

## Internal works
You can see what the vm is doing by compiling ff with `make debug`. This implementation is divided in 2 parts: compiler and vm.
Compiler parses input, and spits out a `Chunk`. `Chunk` contains bytecode and constants array. Each function has it's own `Chunk` 
The top-level code lives in an implicit `Chunk` called `<script>` The vm runs the `Chunk` that compiler gives it.
FF vm is stack based, and stack frames are of `Value` type. FF has a clear distinction between `Value` and `Object`.
`Value` can store integral types (`Null`, `Number`, `Bool`) and an `Object`. `Object`s are allocated on the heap.Strings
and functions are a good exaple of an `Object`. Strings are represented with `ObjString` object, and functions with `ObjFunction`.
