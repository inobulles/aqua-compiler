#!/bin/sh
set -e

# flags

compiler_files=/tmp/aqua-compiler-files

# parse arguments

echo "[AQUA Compiler] AQUA Compiler"
echo "[AQUA Compiler] Parsing arguments ..."

lang=amber
path=`pwd`
output=$path/package.zpk
update=false

while test $# -gt 0; do
    if   [ $1 = --lang   ]; then lang=$2;   shift
    elif [ $1 = --path   ]; then path=$2;   shift
    elif [ $1 = --output ]; then output=$2; shift
    elif [ $1 = --update ]; then update=true
    
    else
        echo "[AQUA Compiler] ERROR Unknown argument '$1' (read README.md for help)"
        exit 1
    fi

    shift
done

path=`realpath $path`
output=`realpath $output`

# setup everything for compilation

echo "[AQUA Compiler] Setting everything up for compilation ..."

cd `dirname $0`
compiler_path=`realpath .`

if [ ! -f $compiler_path/langs/$lang ]; then
    echo "[AQUA Compiler] ERROR The language '$lang' is not available"
    exit 1
fi

# compilation and library handling

echo "[AQUA Compiler] Compiling using $lang ..."

rm -rf $compiler_files
mkdir -p $compiler_files

cd $compiler_files
cp -r $path project

if [ $update = true ] || [ `echo $lang | grep amber*` ]; then
    if [ ! -d $compiler_path/lib ]; then
        read -p "[AQUA Compiler] Library could not be found in compiler path ($compiler_path/lib). Press enter to install the standard one automatically ... " a
        echo "[AQUA Compiler] Installing library ..."

        ( cd $compiler_path
        sudo git clone https://github.com/inobulles/aqua-amber-lib --depth 1 -b master
        sudo mv aqua-amber-lib lib )
    elif [ $update = true ]; then
        echo "[AQUA Compiler] Updating library ..."

        ( cd $compiler_path/lib
        sudo git pull origin master )
    fi
fi

if [ `echo $lang | grep amber*` ]; then # copy over library
    echo "[AQUA Compiler] Using Amber programming language, so copying over library ..."
    cp -r $compiler_path/lib project
fi

mkdir package
$compiler_path/langs/$lang --path project --output package/rom.zed

if [ `echo $lang | grep amber*` ]; then
    echo "[AQUA Compiler] Using Amber programming language, so running assembly pass on output ..."
    mv package/rom.zed project/main.asm
    $compiler_path/langs/asm --path project --output package/rom.zed
fi

# package creation

echo "[AQUA Compiler] Creating package ..."
echo "zed" > package/start

if [ -f project/unique ]; then
    echo "[AQUA Compiler] Moving unique file to package ..."
    mv project/unique package/unique
else
    echo "[AQUA Compiler] No unique file found, automatically creating one ..."
    unique=temp`date +%s`.`od -An -N2 -i /dev/random | tr -d '[:space:]'`
    echo -n $unique > package/unique
    echo "[AQUA Compiler] Unique is $unique"
fi

if [ -f project/feature_set ]; then
    echo "[AQUA Compiler] Moving feature_set file to package ..."
    mv project/feature_set package/feature_set
fi

if [ -d project/meta ]; then
    echo "[AQUA Compiler] Moving meta folder to package ..."
    mv project/meta package/meta
else
    echo "[AQUA Compiler] No meta folder found, automatically creating development one ..."
    mkdir package/meta

    echo "Development application" > package/meta/name
	echo "Developer" > package/meta/author
	echo "development" > package/meta/version
	echo "Development Company" > package/meta/organization
	echo "Development application." > package/meta/description
fi

if [ -d project/assets ]; then
    echo "[AQUA Compiler] Moving assets folder to package ..."
    mv project/assets package/assets
fi

iar --pack package --output package.zpk
mv package.zpk $output
rm -rf $compiler_files

echo "[AQUA Compiler] Done"
exit 0