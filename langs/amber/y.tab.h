/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 81 "compiler.y" /* yacc.c:1909  */

	

#line 48 "y.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IF = 258,
    ELSE = 259,
    WHILE = 260,
    GOTO = 261,
    LAB = 262,
    RETURN = 263,
    VOID = 264,
    INT = 265,
    PLUS = 266,
    MINUS = 267,
    STAR = 268,
    SLASH = 269,
    LT = 270,
    LE = 271,
    GT = 272,
    GE = 273,
    EQUAL = 274,
    NEQUAL = 275,
    ASSIGN = 276,
    LSQUAR = 277,
    RSQUAR = 278,
    LBRACE = 279,
    RBRACE = 280,
    SEMI = 281,
    COMMA = 282,
    LPAREN = 283,
    RPAREN = 284,
    ID = 285,
    NUMBER = 286,
    LETTER = 287,
    DIGIT = 288,
    NONTOKEN = 289,
    ERROR = 290,
    ENDFILE = 291
  };
#endif
/* Tokens.  */
#define IF 258
#define ELSE 259
#define WHILE 260
#define GOTO 261
#define LAB 262
#define RETURN 263
#define VOID 264
#define INT 265
#define PLUS 266
#define MINUS 267
#define STAR 268
#define SLASH 269
#define LT 270
#define LE 271
#define GT 272
#define GE 273
#define EQUAL 274
#define NEQUAL 275
#define ASSIGN 276
#define LSQUAR 277
#define RSQUAR 278
#define LBRACE 279
#define RBRACE 280
#define SEMI 281
#define COMMA 282
#define LPAREN 283
#define RPAREN 284
#define ID 285
#define NUMBER 286
#define LETTER 287
#define DIGIT 288
#define NONTOKEN 289
#define ERROR 290
#define ENDFILE 291

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 85 "compiler.y" /* yacc.c:1909  */

	char* str;
	struct node_s* ast;

#line 137 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
