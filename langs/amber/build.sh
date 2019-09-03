
#!/bin/sh
set -e

echo "Compiling code ..."
#~ python compiler.py

flex compiler.l
yacc compiler.y -d

gcc -O -c lex.yy.c -o lex.yy.o
gcc -O -c y.tab.c  -o y.tab.o

gcc lex.yy.o y.tab.o -lfl -lm
./a.out src/main.a
rm -f a.out lex.yy.c y.tab.c lex.yy.o y.tab.o

#~ mv main.asm ../asm/code.asm
#~ cd ../asm
#~ sh build.sh no-checks
#~ cd ../amber

exit 0
