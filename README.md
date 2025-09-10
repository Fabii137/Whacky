
# Whacky

This is a basic compiler written c++.
For language grammar see ./docs/grammar.md


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
