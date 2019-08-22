
#!/bin/sh
set -e

echo "Compiling code ..."
python compiler.py

echo "Assembling code ..."
mv main.asm ../asm/code.asm
cd ../asm
sh build.sh
cd ../amber

exit 0
