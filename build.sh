
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

if [ -d langs/$code ]; then
	cd langs/$code
	sh build.sh $update git-prefix $git_prefix
	cd ../..
else
	echo "WARNING The language '$code' does not seem to be available"
	exit 1
fi

echo "Creating package ..."
rm -rf package
mkdir package
echo "zed" > package/start
mv rom.zed package/rom.zed

if [ -f "code/unique" ]; then
	echo "Moving unique file to package ..."
else
	echo "No unique file found, automatically creating one ..."
	echo -n "temp`date +%s`$RANDOM" > package/unique
fi

if [ -d "code/meta" ]; then
	echo "Moving meta folder to package ..."
	mv code/meta package/meta
else
	echo "No meta folder found, automatically creating development one ..."
	mkdir package/meta
	
	echo "Development application" > package/meta/name
	echo "Developer" > package/meta/author
	echo "development" > package/meta/version
	echo "Development Company" > package/meta/organization
	echo "Development application." > package/meta/description
fi

if [ -d "code/assets" ]; then
	echo "Moving assets folder to package ..."
	mv code/assets package/assets
fi

iar --pack package --output package.zpk

echo "AQUA compiler terminated with no errors"
exit 0
