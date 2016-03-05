
`alphyn`: compile-time LR(1) parser.

Ever wanted to write a compiler within a compiler ? Now it's possible with `alphyn` (and some work).

There's some documentation [here](documentation/doc.md).

### note

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

tested with g++ 5.3.1 *(may give strange results or even not compile with clang)*


Made by Timothée Feuillet.
