.key in
.def in=inputfile.c

; Notes
; Compiler: Auto Linker: lc -Lxcm LIB:lcX.lib LIB:lcC.lib LIB:lcM.lib
; Compiler: Auto Linker: lc -L+lib:mylib.lib+df0:yourlib.lib
; Linker: blink lib:c.oftoc.o to ftoc lib lib:lcm.lib,lib:lc.lib,lib:amiga.lib
; Compiler: lc -O binary
; diff [options] newfile oldfile
; diff -o outputfile

echo "#-------------------#"
echo "# Lattice "
which lc
assign LC: EXISTS
assign INCLUDE: EXISTS
assign LIB: EXISTS
assign QUAD: EXISTS
echo "#-------------------#"
echo " "
echo "#-------------------#"
echo "# Compiling <in> .." 
echo "#-------------------#"
echo " "
lc -L <in>
