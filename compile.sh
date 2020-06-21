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

while test $# -gt 0; do
    if   [ $1 = --lang   ]; then lang=$2;   shift
    elif [ $1 = --path   ]; then path=$2;   shift
    elif [ $1 = --output ]; then output=$2; shift
    
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
lang_path=`realpath langs/$lang`

if [ ! -f $lang_path ]; then
    echo "[AQUA Compiler] ERROR The language '$lang' is not available"
    exit 1
fi

echo "[AQUA Compiler] Compiling using $lang ..."

# I also need to handle libaries and stuff
# (that's what $compiler_files is for, mixing up source if I need to)

rm -rf $compiler_files
mkdir -p $compiler_files

cd $compiler_files
cp -r $path project

mkdir package
$lang_path --path project --output package/rom.zed

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