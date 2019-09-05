
/* BASED ON
 * https://github.com/yihui-he/c0-compiler
 * http://ftp.mozgan.me/Compiler_Manuals/LexAndYaccTutorial.pdf
 */

D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
WS  [ \t\v\n\f]

%option yylineno

%{  
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	
	#include "y.tab.h"
	
	int yywrap(void);
	void printToken(char* type) { fprintf(yyout,"  <token line=\"%d\" type=\"%s\" string=\"%s\" />\n", yylineno, type, yytext); }
	static void comment(void);
%}

%%
"/*" comment(); // comments
"//".*

"if"     return IF; // statement keywords
"else"   return ELSE;
"goto"   return GOTO;
"lab"    return LAB;
"print"  return PRINT;
"return" return RETURN;
"var"    return VAR;

"+" return '+';
"-" return '-';
"*" return '*';
"/" return '/';

"=" return '=';
";" return ';';

"{" return '{';
"}" return '}';
"[" return '[';
"]" return ']';
"(" return '(';
")" return ')';

{L}{A}* { yylval.str = strdup(yytext); return IDENTIFIER; } // literals
{D}+ { yylval.str = strdup(yytext); return NUMBER; }
L?\"(\\.|[^\\"])*\" { yylval.str = strndup(yytext, strlen(yytext) - 1); return STRING; }

{WS}+ // white spaces and other tokens
.
%%

#include <stdio.h>

int yywrap(void) {
	return 1;
}


static void comment(void) {
	int c;
	while ((c = input()) != 0) if (c == '*') {
		while ((c = input()) == '*');
		
		if (c == '/') return;
		if (c == 0) break;
	}
	
	printf("unterminated comment");
	exit(-1);
	
}
