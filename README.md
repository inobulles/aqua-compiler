# aqua-compiler
This repo contains the source for the AQUA compiler (command-line) and the core language and target source files.

## Command-line arguments
Here is a list of all command-line arguments that can be passed to the compiler and how to use them:

## --lang [language name]
Choose a programming language.
Defaults to the latest stable version of Amber (`amber`).

## --targ [target name]
Choose a target.
Defaults to targetting AQUA with a ZPK (`aqua`).

## --input [path to project directory]
Specify the path to the projet you want to compile.
Will be the current working directory by default.

## --output [path to output directory]
Specify where you want the output to be.
Will be the current working directory by default.

## --debug
Enable debugging.
