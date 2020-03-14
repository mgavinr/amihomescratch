Top Issues
======
14.mar.2020 the sasc does not bla bla bla


Compilers
======

lc
---

Command line for lc

```
lc -L helloworld                    - compile and link, link with the lattice runtime library for exe
lc -Lm helloworld                   - compile and link, link with lcm.lib
lc -fi -Lm helloworld               - compile and link, with lcmieee.lib amiga.lib
lc -f -Lm helloworld                - compile and link, with ffp

-Lxpm                               - lcx.lib lcp.lib lcm.lib
-L+lib:mylib.lib+dh0:yourlib.lib    - mylib.lib and yourlib.lib
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
