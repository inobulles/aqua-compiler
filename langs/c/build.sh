
#!/bin/sh
set -e

if [ ! -d "objconv-src" ]; then
	echo "Downloading objconv ..."
	git clone https://github.com/gitGNU/objconv --depth 1 -b master
	mv objconv objconv-src
fi

if [ ! -f "objconv" ]; then
	echo "Compiling objconv ..."
	g++ -o objconv -O2 -std=c++98 objconv-src/src/*.cpp
fi

echo "Compiling code ..."
rm main.o
gcc -std=gnu11 -s -c src/main.c -o main.o \
	-fdollars-in-identifiers -fextended-identifiers \
	-fno-exceptions -fno-function-cse -Wl,--gc-sections -ffast-math -fomit-frame-pointer -fwhole-program \
	-fshrink-wrap -fcaller-saves -pipe \
	-fno-builtin -fno-builtin-function -nostdlib  \
    -fno-asynchronous-unwind-tables -fno-jump-tables -fno-unwind-tables \
	-Werror-implicit-function-declaration \
	-mstringop-strategy=libcall \
	-masm=intel -march=athlon-fx -maddress-mode=long \
	-msoft-float -mno-sse -m64 -lsoft-fp \

echo "Transpiling code ..."
./objconv -fnasm -ls main.o

echo "Assembling code ..."
mv main.asm ../asm/code.asm
cd ../asm
sh build.sh
cd ../c

exit 0
