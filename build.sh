
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

echo "AQUA compiler terminated with no errors"
exit 0
