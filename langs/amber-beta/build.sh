
#!/bin/sh
set -e
echo "Compiling with Amber 1.1.0 (beta) ..."

rm -rf src
mkdir -p src
cp -r ../../code/* src

flex compiler.l
yacc compiler.y -d

gcc -O -c lex.yy.c -o lex.yy.o
gcc -O -c y.tab.c  -o y.tab.o

gcc lex.yy.o y.tab.o -lfl -lm
./a.out src/main.a main.asm
rm -f a.out lex.yy.c y.tab.c lex.yy.o y.tab.o y.tab.h

mv main.asm ../../code/main.asm
cd ../asm
sh build.sh no-checks
cd ../amber-beta

exit 0
