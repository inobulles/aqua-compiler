
/* DOCUMENTATION
 * http://ftp.mozgan.me/Compiler_Manuals/LexAndYaccTutorial.pdf
 * https://github.com/yihui-he/c0-compiler
 */

D [0-9]
X [0-9A-Fa-f]
L [a-zA-Z_]
A [a-zA-Z_0-9]
W [ \t\v\n\f]

%option yylineno

%{  
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	
	#include "y.tab.h"
	
	char string_buffer[100];
	char* string;
	
	int yywrap(void);
	static void comment(void);
%}

%x LEX_STRING

%%
\" { BEGIN LEX_STRING; string = string_buffer; }
<LEX_STRING>\\n { *string++ = '\n'; }
<LEX_STRING>\\t { *string++ = '\t'; }
<LEX_STRING>\\b { *string++ = '\b'; }
<LEX_STRING>\\r { *string++ = '\r'; }
<LEX_STRING>\\0 { *string++ = '\0'; }
<LEX_STRING>\\\\ { *string++ = '\\'; }
<LEX_STRING>\\\" { *string++ = '"'; }
<LEX_STRING>\" {
	BEGIN 0;
	yylval.data.bytes = string - string_buffer;
	yylval.data.data = malloc(yylval.data.bytes);
	memcpy(yylval.data.data, string_buffer, yylval.data.bytes);
	return STRING;
}
<LEX_STRING>. { *string++ = *yytext; }

"/*" comment(); // comments
"//".*

"if"    return IF; // statement keywords
"else"  return ELSE;
"while" return WHILE;
"func"  return FUNC;
"class" return CLASS;
"loop"  return LOOP;

"break"    { yylval.data.data = strdup(yytext); return CONTROL; } // control statement keywords
"continue" { yylval.data.data = strdup(yytext); return CONTROL; }

"var"  return VAR; // data type keywords
"byte" return BYTE;

"freeze" { yylval.data.data = strdup(yytext); return ATTRIBUTE; } // attribute keywords
"manual" { yylval.data.data = strdup(yytext); return ATTRIBUTE; }

"++" return STR_CAT; // string operators
"%%" return STR_FORMAT;

"===" return STR_CMP_EQ; // string comparison operators
"!==" return STR_CMP_NEQ;

"==" return CMP_EQ; // comparison operators
"!=" return CMP_NEQ;
">=" return CMP_GTE;
"<=" return CMP_LTE;
">" return CMP_GT;
"<" return CMP_LT;

("and")|("&&") return LOG_AND; // logical operators
("or")|("||")  return LOG_OR;
("xor")|("^^") return LOG_XOR;

"+" return '+'; // general operators
"-" return '-';
"*" return '*';
"/" return '/';
"%" return '%';
"?" return '?';
"&" return '&';
"=" return '=';
"^" return '^';
("|")|("nrm") return '|';
("!")|("not") return '!';

"<<"  return SHL; // shift operators
">>"  return ROR;
">>>" return SHR; 

";" return ';'; // delimiters
"," return ',';

"{" return '{'; // brackets
"}" return '}';
"[" return '[';
"]" return ']';
"(" return '(';
")" return ')';

{L}{A}* { yylval.data.data = strdup(yytext); return IDENTIFIER; } // literals
"0x"{X}+ { yylval.data.data = strdup(yytext + 1); return NUMBER; }
{D}+ { yylval.data.data = strdup(yytext); return NUMBER; }

{W}+ // white spaces and other tokens
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
