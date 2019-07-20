
#!/bin/sh
set -e

echo "AQUA compiler"
echo "Parsing arguments ..."

update=""
code=""
git_prefix="https://github.com"

while test $# -gt 0; do
	if   [ "$1" = "update"     ]; then update="update"
	elif [ "$1" = "code"       ]; then code="$2";       shift
	elif [ "$1" = "git-prefix" ]; then git_prefix="$2"; shift
	fi
	
	shift
done

if [ ! -d "lib" ]; then
	echo "Downloading lib ..."
	git clone $git_prefix/inobulles/aqua-lib --depth 1 -b master
	mv aqua-lib lib
fi

if [ "$update" = "update" ]; then
	echo "Updating lib ..."
	cd lib
	git pull origin master
	cd ..
fi

if [ "$code" = "asm" ]; then
	echo "Compiling asm code ..."
	cp code/main.asm langs/asm/code.asm
	cd langs/asm
	sh build.sh
	cd ../..
elif [ "$code" = "c" ]; then
	echo "Compiling C code ... "
	rm -rf langs/c/src
	mkdir -p langs/c/src
	cp -r lib langs/c/src
	mv langs/c/src/lib/root.h langs/c/src/root.h
	cp -r code/* langs/c/src
	cd langs/c
	sh build.sh
	cd ../..
else
	echo "WARNING '$code' is not a valid programming language"
	exit 1
fi

echo "AQUA compiler terminated with no errors"
exit 0
