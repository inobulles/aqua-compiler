
#!/bin/bash
set -e

gcc main.c -Wno-builtin-declaration-mismatch -DVERBOSE_MODE=$1 -DDEBUGGING_MODE=$2 -D__ASM_ZVM_V__=$3
./a.out
exit 0
