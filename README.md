
`alphyn`: compile-time LR(1) parser.

Ever wanted to write a compiler within a compiler ? <br/>
Ever wanted to generate C++ types from a string ?

Now it's possible with `alphyn`.

### features
- fast LR(1) parser than can both run at compile-time and runtime
  - fast: the compiler is able to optimize directly the parser. There's no dynamic allocation.
- BNF-like + attribute grammar
  - There is a BNF meta-parser that generate a lexer+parser from a string containing a BNF grammar (see the doc)
- two compile-time modes:
  - *standard* mode, alphyn will work like any other parser work (it will return a value -- can be an ASLR if you want)
  - *"meta"* mode, in which the parser will transform the input string into a **type**. In this mode, attributes may even be template classes.
- a fast and flexible lexer

### documentation
There's some documentation [here](documentation/doc.md).

### how to clone/build

to setup the cloned repository:
```
git submodule init
git submodule update
```

to build (on linux / macOS):
```
mkdir build
cd build
cmake ..
make
```

> works great with g++ 5.3.1 <br/> 
> clang++ 3.7.0 does **not** handle the awesomeness of alphyn (because of the template recursion limit. Moreover it performs the template recursion on its stack, and lphyn likes a lot template recursions, more than there's stack available)


Made by Timothée Feuillet.
