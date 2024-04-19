## wrapper-gcc

wrapper-gcc is a compiler wrapper similar to alf-gcc, which can hijack compiler options without interfering with compilation scripts.



## Usage

```
$ make debug
$ make link
```
-  `make debug` will print all the compiler options when compiling, while `make release` will not.
- `make link` is used to create symbolic links.
- `make clean` is used to remove compiled files.


### Test

It can easily take over the behavior of the compiler.

```
$ export PATH=$(pwd)/bin${PATH:+:${PATH}}
$ for COM in wrapper-gcc wrapper-clang; do $COM test/a.c && ./a.out; done 
$ for COM in wrapper-g++ wrapper-clang++; do $COM test/a.cpp && ./a.out; done 
```

Without modifying `CMakeLists.txt`, it can hijack compiler options according to `WRAPPER_OPTI` and `WRAPPER_DEBUG`.

```
$ export CC=$(pwd)/bin/wrapper-gcc
$ export CXX=$(pwd)/bin/wrapper-g++
$ export WRAPPER_OPTI=O3
$ export WRAPPER_DEBUG=1
$ pushd test/cmake
$ cmake . && make
...
[DEBUG]: g++
[DEBUG]: -o
[DEBUG]: CMakeFiles/a.out.dir/home/giles/Project/wrapper-gcc/test/a.cpp.o
[DEBUG]: -c
[DEBUG]: /home/giles/Project/wrapper-gcc/test/a.cpp
[DEBUG]: -O3
[DEBUG]: -g
...
```
