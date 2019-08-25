
#!/bin/sh
set -e

echo "Compiling code ..."
python compiler.py

mv main.asm ../asm/code.asm
cd ../asm
sh build.sh no-checks
cd ../amber

exit 0
