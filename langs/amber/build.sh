#!/bin/sh
set -e

flex compiler.l
yacc compiler.y -d

gcc -O2 -c lex.yy.c -o lex.yy.o -Wno-unused-result
gcc -O2 -c y.tab.c  -o y.tab.o -DYYINITDEPTH=512 -Wno-unused-result

#~ gcc -O0 -c lex.yy.c -o lex.yy.o -g
#~ gcc -O0 -c y.tab.c  -o y.tab.o -g

gcc -O2 lex.yy.o y.tab.o -lm -o compiler
rm -f lex.yy.c y.tab.c lex.yy.o y.tab.o y.tab.h

exit 0
