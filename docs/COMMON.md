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


lc
---

Command line for lc

lc defines M68000 AMIGA LATTICE LATTICE_50 DEBUG(-d) SPTR(-w) LPTR()

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
