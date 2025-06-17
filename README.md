# MLang

MLang is my strictly typed programming language with full type inference that compiles to a custom byte code. This project is hand crafted with zero dependencies. The language uses a custom recursive descent parser as the front-end and targets a custom byte code vm as the backend. This is a multiplatform project targeting both Linux and Windows.

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
- [x] Structs (on the heap)
- [x] C-calls (FFI) to dynamic libraries
- [x] Strings
- [x] Printing
- [ ] Arrays
- [ ] Closures
- [ ] Garbage collection

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

```
# FFI to dynamic c libraries (libtest.so)
let mul = extern test::mul(a: Int, b: Int): Int;
let result = mul(5, 3);
```

```
# Structs

struct Point {
    let x: Int;
    let y: Int;
}

struct Line {
    let begin: Point;
    let someInt: Int;
    let end: Point;
}

let l: Line;
l.begin.x = 3;
l.begin.y = 4;

l.end.x = 8;
l.end.y = 9;

ret l.begin.x + l.end.y;
```

```
# Strings
let print = extern test::print(s: String): Void;
let str = "Hello, World!";
print(str);
```

```
# Using external library (raylib)
let str = "Hello, World!";

let InitWindow = extern raylib::InitWindow(w: Int, h: Int, s: String): Void;
InitWindow(800, 600, str);

let SetTargetFPS = extern raylib::SetTargetFPS(fps: Int): Void;
SetTargetFPS(60);

let WindowShouldClose = extern raylib::WindowShouldClose(): Bool;
let CloseWindow = extern raylib::CloseWindow(): Void;

let BeginDrawing = extern raylib::BeginDrawing(): Void;
let ClearBackground = extern raylib::ClearBackground(color: Int): Void;
let EndDrawing = extern raylib::EndDrawing(): Void;

let DrawText = extern raylib::DrawText(text: String, x: Int, y: Int, fontSize: Int, color: Int): Void;

while(WindowShouldClose() == false) {
    BeginDrawing();
    ClearBackground(1);
    DrawText(str, 190, 200, 20, 1);
    EndDrawing();
}

CloseWindow();
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

None

## Getting started

If you want to build this yourself, refere to this guide [INSTALL.md](/INSTALL.md)
