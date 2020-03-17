Top Issues
======
14.mar.2020 the sasc does not bla bla bla


Compilers
======
* far means objects addressed with 32 bit addresses
* near means 16 bit
* chip means chip ..
* extern it exists in some other file or is defined later in the same file, generates a reference that will be resolved by linker
* max size of string constant is 256 bytes

Data types
---
```
char                1           0000 0000     0..255 -128..127
unsigned char       1
short               2
unsigned short      2 
long                4
unsigned long       4
float               4 or 4
double              4 or 8
```

lc
---

Command line for lc

lc defines M68000 AMIGA LATTICE LATTICE_50 DEBUG(-d) SPTR(-w) LPTR()

68k requires that 16bit words and 32bit words are aligned on even addresses.  That means struct misc { char x[3]; char dummy; int y}; has a dummy byte inbetween that sizeof will return.

write 3,4 to a file on Amiga is: 03char 00dummy 00 04int .. and on windows it is 03char 04 00int.

```
[COMPILE/LINK]
lc -L helloworld                    - compile and link, link with the lattice runtime library for exe
lc -Lm helloworld                   - compile and link, link with lcm.lib
lc -fi -Lm helloworld               - compile and link, with lcmieee.lib amiga.lib
lc -f -Lm helloworld                - compile and link, with ffp
[OPTIONS]
-r                                  - make library
-i                                  - include dirs
-dx=y                               - #define
-w                                  - short integers
-cu                                 - unsigned chars
-fi l f 8                           - floatings
-m0 1 2 3 4 a                       - machine codes
-rs r b                             - calling mode
-p <outputname>                     - program name
-Lxpm                               - lcx.lib lcp.lib lcm.lib
-L+lib:mylib.lib+dh0:yourlib.lib    - mylib.lib and yourlib.lib
-Lv                                 - verbose
-Ln                                 - nodebug
[LINK]
lc helloworld                       - compile only helloworld.c to helloworld.o
blink lib:c.o,helloworld.o to helloworld lib lib:lcm.lib,lib:lc.lib,lib:amiga.lib
[OPTIMZE DEBUG]
lc -O helloworld                    - optimize
lc -d -d1 d2 .. d5                  - debug
[VALID]
-cf                                 - check prototypes
-ca                                 - strict ansi
-v                                  - no stack checking

```

- BLINK FROM file1.c file2.c file3.c TO fileout LIB|LIBRARY libs 
- cpr is a code probe debugger
- lc options -c+ lattice c++, -ca ansi, -cc nested comments, -cf function proto checks, -ci skip duplicate includes, -cl pre ansi
- lc options -d0 no debugging, -d1 line numebrs, -d line numbers, -d2345 full and stuff
- lc options -fl lattice floats, -ff 68k, -fi amiga, f8 68881
- lc options -La addsym linker option, -Ln nodebug, -Lt small code, -Lf 68k, -Lm lattice floats, 
- lc options -m0 68k, -m1 68010k, -m2 020, -m3 030

lc libraries
---
```
amiga.lib           - workbench libs maybe, AmigaDOS binding library

lc.lib              - std c
lcr.lib             - std c, registerized params
lcs.lib             - std c, 16bit integers
lcsr.lib            - std c, 16bit registerized params
lcnb.lib            - std c, no base-relative data addressing

lcm.lib             - math
lcm881.lib          - 68881 coprocessor math
lcmieee.lib         - math amiga
lcmr.lib            - math registerized parameters yeah baby
lcms.lib            - math amiga 16bit integers
lcmffp.lib          - math, 68k fast floating point

ddebug.lib          - debug library for // port
debug.lib           - debug library for ser port
```

lc includes
---
```
__builtin_          - some sas routines for the std c ones maybe __builtin_strcpy is the equiv there are #defines in string.h and stdlib.h already defined to use these anyway instead of the bla ones

lcompact <source >dest    - compresses the headers

#include <proto/all.h>    - all amiga functions
```

misc
---
```
files -type f . ./docs      - searches for files in . and docs 
files -pat unix ./docs      - find with unix patterns
files -m -name *.* ./docs   - find with unixey patterns also
files -name #stuff# ./docs  - find with amiga patterns
```

makefiles
---
```lmk```
Example makefile
```
FLAGS=-a -b
myfile: file1.o
  blink FROM LIB:c.o+file1.o TO myfile LIB LIB:lc.lib+LIB:amiga.lib $(FLAGS)

file1.o: file1.c headers.h
  lc -b0 -r0 -o file1.o file1.c
```
Dependencies
```
test.o: test.c test.h
$@ test.o
$? test.c       # based on modification time i think
$* test
$< test.c
$> test

- continue on error
@ silent
~ some escape

lmk.def defines default lattice options

.DEFAULT:
  bla bla is the default option
```
An example lmk file
```
HDRS=stdio.h fcntl.h
OBJ=obj/
LCFLAGS=-iINCLUDE: -iINCLUDE:lattice/ -i/grep/ -o$(OBJ)
LC=LC:lc
LIB=LIB:

.c.o:
  $(LC) $(LCFLAGS) -cc $*

files: $(OBJ)files.o $(OBJ)os.o $(OBJ)strbpl.o
  LINK:blink FROM $(LIB)c.o+$(OBJ)files.o+$(OBJ)os.o+$(OBJ)strbpl.o TO files LIBRARY $(LIB)lc.lib+$(LIB)amiga.lib
  
$(OBJ)files.o: files.c $(HDRS)
$(OBJ)os.o: os.c
```
