# MLang

This is the implementation fo a custom language called MLang. The language is build on top of ANTLR4 as the front-end and targets LLVM JIT as the backend. The project is tested with the google test framework. The language is strictly typed with complete type inference.

I build this language solely for fun and education, so maybe you should not use this in production 🐴

# Features

- [x] Arithmetic
- [x] Boolean operations
- [ ] Printing
- [x] Branching (if)
- [x] Loops (while)
- [x] Strictly typed
- [x] Integer type
- [x] Boolean type
- [ ] String type
- [x] Float type
- [x] Function type
- [ ] Closures
- [x] Type inference
- [ ] Structs
- [x] Functions
- [ ] Operator precedence

# Example code

For more examples refere to the [mfiles](/mfiles/) folder

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
# Multi line functions
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

# Dependencies

- Antlr4
- LLVM
- GTest

# Getting started

You can find an installation guide in [install.md](/install.md)

# Some useful links

https://llvm.org/doxygen/classllvm_1_1IRBuilderBase.html#a132a883efbc512ce6325d5f751bb3672
https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl03.html
https://gist.github.com/robstewart57/b11353feb69dc1a6dc30
http://lists.llvm.org/pipermail/llvm-dev/2015-March/083809.html
