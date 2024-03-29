# MLang

MLang is my strictly typed programming language with full type inference that compiles to LLVM. The language uses a custom recursive descent parser as the front-end and targets LLVM JIT as the backend. The project is tested with the google test framework. This is a multiplatform project targeting both Linux and Windows.

This language is build solely for fun and educational purposes, so maybe you should not use this in production 🐴

You can download the executables for Windows and Linux in the [release section](https://github.com/MoritzGoeckel/MLang/releases/).

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
- [x] Operator precedence
- [x] Linux support
- [x] Windows support
- [x] Error reporting
- [ ] Heap allocation
- [ ] Arrays
- [ ] Pointers
- [ ] String type
- [ ] Printing
- [ ] Structs
- [ ] Closures

## Examples

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

```
# Operator precedence
let x = 3 + 10 + 6 / 3 - 2 * 5;
ret x; # is 5
```

## Error reporting

Following you find an example on how parsing errors are reported to the user

```
Parsing failed:
Expecting ';' but found 'ret' @3:17

1:  let f(x) = {
2:      let y = x + 2
3:                  ^

Consider adding a semicolon to the end of the statement
```

## Dependencies

- LLVM
- GTest

## Getting started

You can find an installation guide in [INSTALL.md](/INSTALL.md)
