
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
	
	char string_buffer[1024];
	char* string;
	char import_prefix[1024] = {0};
	char import_data;
	
	char* already_imported[4096];
	int already_imported_count = 0;
	
	int yywrap(void);
	static void comment(void);
%}

%x LEX_STRING
%x LEX_IMPORT

%%
\" { BEGIN LEX_STRING; string = string_buffer; } // string
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

"import" { BEGIN LEX_IMPORT; import_data = 0; } // import
"import_data" { BEGIN LEX_IMPORT; import_data = 1; };
<LEX_IMPORT>[ \t]*
<LEX_IMPORT>[^ \t\n);]+ {
	BEGIN 0;
	
	char path[1024] = {0};
	strcpy(path, import_prefix);
	strcpy(path + strlen(path), yytext);
	
	uint8_t import = 1;
	for (int i = 0; i < already_imported_count; i++) {
		if (strcmp(path, already_imported[i])) {
			import = 0;
			break;
		}
	}
	
	if (import) {
		already_imported[already_imported_count++] = path;
		
		if (!(yyin = fopen(path, "r"))) printf("WARNING File %s could not be imported (line %d)\n", path, yylineno);
		else if (!import_data) yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
		
		else {
			fseek(yyin, 0, SEEK_END);
			yylval.data.bytes = ftell(yyin);
			rewind(yyin);
			
			yylval.data.data = malloc(yylval.data.bytes);
			int x = fread(yylval.data.data, 1, yylval.data.bytes, yyin);
			
			return STRING;
		}
	}
}
<<EOF>> {
	yypop_buffer_state();
	if (!YY_CURRENT_BUFFER) yyterminate();
}

"/*" comment(); // comments
"//".*

"if"     return IF; // statement keywords
"else"   return ELSE;
"while"  return WHILE;
"func"   return FUNC;
"class"  return CLASS;
"loop"   return LOOP;

"break"    { yylval.data.data = strdup(yytext); return CONTROL; } // control statement keywords
"continue" { yylval.data.data = strdup(yytext); return CONTROL; }

"var"  return VAR; // data type keywords
"byte" return BYTE;

"freeze" { yylval.data.data = strdup(yytext); return ATTRIBUTE; } // attribute keywords
"global" { yylval.data.data = strdup(yytext); return ATTRIBUTE; }

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

"." return '.'; // class stuff
"->" return CAST;

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
