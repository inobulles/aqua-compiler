#!/bin/sh
set -e

gcc -O2 main.c -o compiler -Wno-unused-result
exit 0
