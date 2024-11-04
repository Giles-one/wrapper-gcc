## wrapper-gcc

wrapper-gcc is a compiler wrapper similar to alf-gcc, which can hijack compiler options without interfering with compilation scripts.

## Usage

```
$ make
$ make link
```

- `make` will generate `bin/wrapper-gcc`, `sbin/bcsdcc`.
- `make link` will create symbolic links `bin/cc -> ./wapper-gcc` .. `sbin/bcsdcc -> ../bin/wrapper-gc`.
- `make clean` is used to remove compiled files.

### step1

make bcsdcc in your PATH environment.

```
$ export PATH=$(pwd)/sbin${PATH:+:${PATH}}
```

### step2

specify if debug, which optimization, what to append. And compile your project.

```
$ cd test/cmake/build
$ bcsdcc -g -o O0 -a '-DMACRO=0' -- bash
# do anything you need to compile
$ cmake .. && make
...
[DEBUG]: c++
[DEBUG]: -DNDEBUG
[DEBUG]: -MD
[DEBUG]: -MT
[DEBUG]: CMakeFiles/a.out.dir/data/lgy/Projects/wrapper-gcc/test/a.cpp.o
[DEBUG]: -MF
[DEBUG]: CMakeFiles/a.out.dir/data/lgy/Projects/wrapper-gcc/test/a.cpp.o.d
[DEBUG]: -o
[DEBUG]: CMakeFiles/a.out.dir/data/lgy/Projects/wrapper-gcc/test/a.cpp.o
[DEBUG]: -c
[DEBUG]: /data/lgy/Projects/wrapper-gcc/test/a.cpp
[DEBUG]: -g
[DEBUG]: -O0
[DEBUG]: -DMACRO=0
...
```

see? Without modifying `CMakeLists.txt`, it can hijack compiler options according to the options of bcsdcc.

### step3

When done, don't forget to exit current shell.
```
$ exit
```

### another example

```
$ cd test/make
$ make
gcc -o a.out -s -Os ../a.c
$ rm a.out 
$ bcsdcc -g -o O0 --append '-D MACRO' -- make
Debug mode enabled.
Optimization level set to: O0
Append `-D MACRO` to compile options.
gcc -o a.out -s -Os ../a.c
[DEBUG]: Hijack compiler ** gcc **
[DEBUG]: cmd argv
[DEBUG]: gcc
[DEBUG]: -o
[DEBUG]: a.out
[DEBUG]: -O0
[DEBUG]: ../a.c
[DEBUG]: -g
[DEBUG]: -D MACRO
$ strings ./a.out | grep GNU
GNU C17 11.4.0 -mtune=generic -march=x86-64 -g -O0 -fasynchronous-unwind-tables -fstack-protector-strong -fstack-clash-protection -fcf-protection
```