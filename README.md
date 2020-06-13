# MLang

MLang is my strictly typed programming language with full type inference that compiles to LLVM. The language is build on top of ANTLR4 as the front-end and targets LLVM JIT as the backend. The project is tested with the google test framework.

This is language is build solely for fun and educational purposes, so maybe you should not use this in production 🐴

## Features

- [x] Type inference
- [x] Strictly typed
- [x] Functions
- [x] Arithmetic operations
- [x] Boolean operations
- [x] Branching (if)
- [x] Loops (while)
- [x] Integer type
- [x] Boolean type
- [x] Float type
- [ ] String type
- [ ] Closures
- [ ] Printing
- [ ] Structs
- [ ] Operator precedence

## Example code

For more examples refer to the [mfiles](/mfiles/) folder

```
# Loops
let i = 0;
let j = 1;
while(i < 10){
    j = j * 2;
    i = i + 1;
}
ret j;
```

```
# Infix notation
let i = 1 + 2;
# Prefix notation
let j = +(1, 2);
```

```
# Functions
let f(x) = x + 2;
ret f(11);
```

```
# Multi-line functions
let f(x) = {
    let y = x + 2;
    ret y + 3;
};
ret f(10);
```

```
# Branching
let x = 1;
if(x < 3){
    ret true;
}
ret false;
```

## Dependencies

- Antlr4
- LLVM
- GTest

## Getting started

You can find an installation guide in [install.md](/install.md)
