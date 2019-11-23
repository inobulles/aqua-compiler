
#!/bin/sh
set -e
echo "Compiling with Amber 1.1.0 (beta) ..."

update=""
git_prefix="https://github.com"

while test $# -gt 0; do
	if   [ "$1" = "update"     ]; then update="update"
	elif [ "$1" = "git-prefix" ]; then git_prefix="$2"; shift
	fi
	
	shift
done

if [ ! -d "lib" ]; then
	echo "Downloading lib ..."
	git clone $git_prefix/inobulles/aqua-amber-lib --depth 1 -b master
	mv aqua-amber-lib lib
fi

if [ "$update" = "update" ]; then
	echo "Updating lib ..."
	cd lib
	git pull origin master
	cd ..
fi

rm -rf src
mkdir -p src
cp -r ../../code/* src
cp -rf lib src

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
