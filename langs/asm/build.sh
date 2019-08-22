
#!/bin/bash
set -e

gcc main.c #-DVERBOSE_MODE=$1 -DDEBUGGING_MODE=$2 -D__ASM_ZVM_V__=$3
./a.out in code.asm out rom.zed
mv rom.zed ../../rom.zed

exit 0
