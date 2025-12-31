# Whacky
This is a basic compiler (without much features :p) written in c++ for learning.
The language grammar can be found at ./docs/grammar.md

## Requirements
- `x86_64 Linux`
- `CMake 3.16+`
- `C++20+`
- `nasm`
- GNU linker (`ld`, provided by the `binutils` package)
## Run:

 1.  create build directory
```shell
mkdir build && cd build
```
 2.  build project
```shell
cmake .. && cmake --build .
```
3. compile and run program
```shell
./whacky <input.wy> && ./out
```
