
#!/bin/bash
set -e
echo "Compiling with ZASM 1.0.0 (stable) ..."

cp ../../code/main.asm code.asm
gcc main.c #-DVERBOSE_MODE=$1 -DDEBUGGING_MODE=$2 -D__ASM_ZVM_V__=$3
./a.out verbose in code.asm out rom.zed $*
cp code.asm ../../rom.asm
mv rom.zed ../../rom.zed

exit 0
