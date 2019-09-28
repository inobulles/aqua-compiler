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
#line 111 "compiler.y" /* yacc.c:1909  */

	

#line 48 "y.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    FUNC = 258,
    CLASS = 259,
    IF = 260,
    WHILE = 261,
    GOTO = 262,
    LAB = 263,
    RETURN = 264,
    VAR = 265,
    PRINT = 266,
    NEW = 267,
    IFX = 268,
    ELSE = 269,
    BUILTIN = 270,
    IDENTIFIER = 271,
    NUMBER = 272,
    STRING = 273,
    NONTOKEN = 274,
    ERROR = 275,
    ENDFILE = 276,
    UBDEREF = 277,
    UDEREF = 278,
    UREF = 279,
    UMINUS = 280,
    UPLUS = 281,
    CMPOP_EQ = 282,
    CMPOP_NEQ = 283,
    STROP_EQ = 284,
    STROP_NEQ = 285,
    STROP_CAT = 286,
    STROP_FOR = 287,
    ARROW = 288
  };
#endif
/* Tokens.  */
#define FUNC 258
#define CLASS 259
#define IF 260
#define WHILE 261
#define GOTO 262
#define LAB 263
#define RETURN 264
#define VAR 265
#define PRINT 266
#define NEW 267
#define IFX 268
#define ELSE 269
#define BUILTIN 270
#define IDENTIFIER 271
#define NUMBER 272
#define STRING 273
#define NONTOKEN 274
#define ERROR 275
#define ENDFILE 276
#define UBDEREF 277
#define UDEREF 278
#define UREF 279
#define UMINUS 280
#define UPLUS 281
#define CMPOP_EQ 282
#define CMPOP_NEQ 283
#define STROP_EQ 284
#define STROP_NEQ 285
#define STROP_CAT 286
#define STROP_FOR 287
#define ARROW 288

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 115 "compiler.y" /* yacc.c:1909  */

	char* str;
	struct { char* data; int bytes; } data;
	struct node_s* ast;

#line 132 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
