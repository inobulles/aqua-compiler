
/// HEAVILY BASED ON https://github.com/yihui-he/c0-compiler

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
"/*" { comment(); } // comments
"//".* {}

"if"     { return IF; } // statement keywords
"else"   { return ELSE; }
"while"  { return WHILE; }
"goto"   { return GOTO; }
"lab"    { return LAB; }
"return" { return RETURN; }
"void"   { return VOID; }

"int"    { yylval.str = strdup(yytext); return INT; } // type keywords

"+"  { return PLUS; } // operators
"-"  { return MINUS; } 
"*"  { return STAR; } 
"/"  { return SLASH; } 
"<"  { return LT; } 
"<=" { return LE; } 
">"  { return GT; } 
">=" { return GE; } 
"==" { return EQUAL; } 
"!=" { return NEQUAL; } 
"="  { return ASSIGN; } 

("[") { return LSQUAR; } // tree control
("]") { return RSQUAR; }
("{") { return LBRACE; }
("}") { return RBRACE; }

";"   { return SEMI; }
","   { return COMMA; }
"("   { return LPAREN; }
")"   { return RPAREN; }

{L}{A}* { return ID; } // literals
{D}+ { yylval.str = strdup(yytext); return NUMBER; }

{WS}+ {} // white spaces and other tokens
.     {}
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
