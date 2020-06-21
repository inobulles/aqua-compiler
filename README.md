# aqua-compiler
This repo contains all the build scripts for the AQUA compiler

## Command-line arguments
Here is a list of all command-line arguments that can be passed to the compiler and how to use them.

## --lang [language name]
Choose a programming language.
Defaults to the latest stable version of Amber.

## --path [path to project directory]
Specify the path to the projet you want to compile.
Will be the current working directory by default.

## --output [path to output]
Specify where you want the output package to be.
Will be `package.zpk` in the current working directory by default.

## --update
Update the libraries.