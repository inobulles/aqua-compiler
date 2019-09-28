/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 2 "compiler.y" /* yacc.c:339  */

	#include <ctype.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdarg.h>
	#include <stdint.h>
	
	extern void yyerror(const char* string);
	extern int yylex(void);
	
	extern FILE* yyin;
	extern FILE* yyout;
	
	extern int yylineno;
	
	#define GRAMMAR_PROGRAM 0
	#define GRAMMAR_STATEMENT 1
	#define GRAMMAR_PRINT 2
	#define GRAMMAR_VAR_DECL 3
	#define GRAMMAR_STATEMENT_LIST 4
	#define GRAMMAR_NUMBER 5
	#define GRAMMAR_IDENTIFIER 6
	#define GRAMMAR_UNARY 7
	#define GRAMMAR_OPERATION 8
	#define GRAMMAR_STROP 9
	#define GRAMMAR_RETURN 12
	#define GRAMMAR_STRING 13
	#define GRAMMAR_LOGIC 14
	#define GRAMMAR_ULOGIC 15
	#define GRAMMAR_IF 16
	#define GRAMMAR_IFELSE 17
	#define GRAMMAR_ASSIGN 18
	#define GRAMMAR_WHILE 19
	#define GRAMMAR_FUNC 20
	#define GRAMMAR_CALL 21
	#define GRAMMAR_EXPRESSION_LIST 22
	#define GRAMMAR_ARGUMENT 23
	#define GRAMMAR_ARGUMENT_LIST 24
	#define GRAMMAR_CMPOP 25
	#define GRAMMAR_DECL_LIST 26
	#define GRAMMAR_CLASS 27
	#define GRAMMAR_ACCESS 28
	#define GRAMMAR_NEW 29
	#define GRAMMAR_BUILTIN 30
	
	typedef struct {
		uint8_t is_function;
		char identifier[64];
		uint64_t stack_pointer;
		int scope_depth;
		void* class;
		
	} reference_t;
	
	typedef struct class_s {
		char identifier[64];
		int scope_depth;
		int bytes;
		
		uint64_t class_count;
		struct class_s* classes;
		
		uint64_t reference_count;
		reference_t* references;
		
	} class_t;
	
	typedef struct node_s {
		#define MAX_CHILDREN 16
		
		int child_count;
		struct node_s* children[MAX_CHILDREN];
		
		char* ref, *ref_code, *data;
		uint64_t stack_pointer;
		class_t* class;
		uint8_t self_zero;
		struct node_s* access_class_object;
		int data_bytes, type, line;
		
	} node_t;
	
	void compile(node_t* self);
	
	node_t* new_node(int line, int type, int data_bytes, char* data, int child_count, ...) {
		node_t* self = (node_t*) malloc(sizeof(node_t));
		
		self->type = type;
		self->data = data;
		self->data_bytes = data_bytes;
		self->line = line;
		self->ref_code = "";
		self->child_count = child_count;
		
		va_list args;
		va_start(args, child_count);
		
		for (int i = 0; i < self->child_count; i++) {
			self->children[i] = va_arg(args, node_t*);
			
		}
		
		va_end(args);
		return self;
				
	}

#line 175 "y.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
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
#line 111 "compiler.y" /* yacc.c:355  */

	

#line 209 "y.tab.c" /* yacc.c:355  */

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
#line 115 "compiler.y" /* yacc.c:355  */

	char* str;
	struct { char* data; int bytes; } data;
	struct node_s* ast;

#line 293 "y.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 310 "y.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  44
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   513

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  48
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  10
/* YYNRULES -- Number of rules.  */
#define YYNRULES  54
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  116

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   288

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    39,     2,
      43,    44,    36,    34,    47,    35,    41,    37,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
       2,    27,     2,    38,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    45,     2,    46,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    28,    29,    30,    31,    32,    33,    40
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   145,   145,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   162,   163,   167,   168,   169,   170,   171,
     175,   176,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   210,
     211,   215,   216,   220,   221
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FUNC", "CLASS", "IF", "WHILE", "GOTO",
  "LAB", "RETURN", "VAR", "PRINT", "NEW", "IFX", "ELSE", "BUILTIN",
  "IDENTIFIER", "NUMBER", "STRING", "NONTOKEN", "ERROR", "ENDFILE",
  "UBDEREF", "UDEREF", "UREF", "UMINUS", "UPLUS", "'='", "CMPOP_EQ",
  "CMPOP_NEQ", "STROP_EQ", "STROP_NEQ", "STROP_CAT", "STROP_FOR", "'+'",
  "'-'", "'*'", "'/'", "'?'", "'&'", "ARROW", "'.'", "';'", "'('", "')'",
  "'{'", "'}'", "','", "$accept", "program", "statement", "statement_list",
  "declaration", "declaration_list", "expression", "expression_list",
  "argument", "argument_list", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,    61,   282,   283,
     284,   285,   286,   287,    43,    45,    42,    47,    63,    38,
     288,    46,    59,    40,    41,   123,   125,    44
};
# endif

#define YYPACT_NINF -58

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-58)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     133,     1,    11,   -13,    -8,   258,    20,   258,    22,    -6,
     -58,   -58,   -58,   258,   258,   258,   258,   258,   -58,   268,
      89,    40,   133,   -58,   -58,   352,   176,     5,   258,   258,
     369,   -22,   386,   -58,   217,   420,   420,   437,   454,   420,
       7,   298,   -58,    -7,   -58,   -58,   258,   258,   258,   258,
     258,   258,   258,   258,   258,    44,   -58,   228,   188,   -58,
       3,   316,   334,   -58,   258,   -58,   -58,   -58,   281,    17,
     258,   258,   -58,   -58,   -58,   470,    39,    39,    21,    21,
     -15,   -15,   -10,   -10,   -58,   -58,    19,    51,   -12,    23,
      33,     3,    32,   133,   133,   403,   258,   -58,   470,   470,
     -58,   -58,   -58,     9,   133,   -58,   -58,    67,   -58,   -58,
     -58,    68,   -58,   -58,   133,   -58
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      26,    22,    23,     0,     0,     0,     0,     0,     3,     0,
       0,     0,    13,     2,     5,     0,     0,     0,     0,     0,
       0,     0,     0,    24,     0,    37,    36,    33,    34,    35,
      26,     0,    11,     0,     1,    14,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,     0,     0,    16,
       0,     0,     0,     7,     0,    19,     6,    29,    49,     0,
       0,     0,    25,    48,    12,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    27,     0,     0,    26,    53,
       0,    20,     0,     0,     0,     0,     0,    30,    31,    32,
      28,    51,    52,     0,     0,    21,    15,     9,     8,    18,
      50,     0,    54,    17,     0,    10
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -58,   -58,   -25,    -4,   -57,    -3,    -5,   -42,   -58,   -20
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    21,    22,    23,    24,    92,    25,    69,    89,    90
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      30,    59,    32,    91,   102,    64,     1,     2,    35,    36,
      37,    38,    39,     6,    41,    86,    43,    26,    45,    87,
      65,    53,    54,    61,    62,   111,    55,    27,    57,    68,
      28,    55,    72,    57,    91,    29,    31,    34,    33,    74,
      44,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      60,    72,    68,    41,   110,    51,    52,    53,    54,    95,
      84,    97,    55,   100,    57,    98,    99,   101,   107,   108,
     103,    49,    50,    51,    52,    53,    54,   104,   106,   113,
      55,   114,    57,   112,   102,     0,     0,     0,   105,   115,
       0,    68,     1,     2,     3,     4,     0,     0,     5,     6,
       7,     8,     0,     0,     9,    10,    11,    12,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    13,    14,    15,     0,    16,    17,     0,
       0,    18,    19,     0,    20,    42,     1,     2,     3,     4,
       0,     0,     5,     6,     7,     8,     0,     0,     9,    10,
      11,    12,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,    14,    15,
       0,    16,    17,     0,     0,    18,    19,     0,    20,     1,
       2,     3,     4,     0,     0,     5,     6,     7,     8,     0,
       0,     9,    10,    11,    12,     0,     0,     0,    87,     0,
       8,     0,     0,     9,    88,    11,    12,     0,     0,     0,
      13,    14,    15,     0,    16,    17,     0,     0,    18,    58,
       0,    20,    13,    14,    15,     0,    16,    17,     0,     8,
       0,    19,     9,    10,    11,    12,     0,     0,     0,     0,
       8,     0,     0,     9,    10,    11,    12,     0,     0,     0,
       0,    13,    14,    15,     0,    16,    17,     0,     0,     0,
      19,    67,    13,    14,    15,     0,    16,    17,     0,     0,
       8,    19,    85,     9,    10,    11,    12,     0,     0,     0,
       8,     0,     0,     9,    40,    11,    12,     0,     0,     0,
       0,     0,    13,    14,    15,     0,    16,    17,     0,     0,
       0,    19,    13,    14,    15,     0,    16,    17,    46,    47,
      48,    19,     0,    49,    50,    51,    52,    53,    54,     0,
       0,     0,    55,     0,    57,    46,    47,    48,    96,     0,
      49,    50,    51,    52,    53,    54,     0,     0,     0,    55,
       0,    57,    73,    46,    47,    48,     0,     0,    49,    50,
      51,    52,    53,    54,     0,     0,     0,    55,     0,    57,
      93,    46,    47,    48,     0,     0,    49,    50,    51,    52,
      53,    54,     0,     0,     0,    55,     0,    57,    94,    46,
      47,    48,     0,     0,    49,    50,    51,    52,    53,    54,
       0,     0,     0,    55,    56,    57,    46,    47,    48,     0,
       0,    49,    50,    51,    52,    53,    54,     0,     0,     0,
      55,    63,    57,    46,    47,    48,     0,     0,    49,    50,
      51,    52,    53,    54,     0,     0,     0,    55,    66,    57,
      46,    47,    48,     0,     0,    49,    50,    51,    52,    53,
      54,     0,     0,     0,    55,   109,    57,    46,    47,    48,
       0,     0,    49,    50,    51,    52,    53,    54,     0,     0,
       0,    55,     0,    57,    70,    47,    48,     0,     0,    49,
      50,    51,    52,    53,    54,     0,     0,     0,    55,     0,
      57,    71,    47,    48,     0,     0,    49,    50,    51,    52,
      53,    54,     0,     0,     0,    55,     0,    57,    47,    48,
       0,     0,    49,    50,    51,    52,    53,    54,     0,     0,
       0,    55,     0,    57
};

static const yytype_int8 yycheck[] =
{
       5,    26,     7,    60,    16,    27,     3,     4,    13,    14,
      15,    16,    17,    10,    19,    57,    20,    16,    22,    10,
      42,    36,    37,    28,    29,    16,    41,    16,    43,    34,
      43,    41,    44,    43,    91,    43,    16,    43,    16,    46,
       0,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      45,    44,    57,    58,    96,    34,    35,    36,    37,    64,
      16,    44,    41,    44,    43,    70,    71,    16,    93,    94,
      47,    32,    33,    34,    35,    36,    37,    44,    46,   104,
      41,    14,    43,   103,    16,    -1,    -1,    -1,    91,   114,
      -1,    96,     3,     4,     5,     6,    -1,    -1,     9,    10,
      11,    12,    -1,    -1,    15,    16,    17,    18,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    38,    39,    -1,
      -1,    42,    43,    -1,    45,    46,     3,     4,     5,     6,
      -1,    -1,     9,    10,    11,    12,    -1,    -1,    15,    16,
      17,    18,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    38,    39,    -1,    -1,    42,    43,    -1,    45,     3,
       4,     5,     6,    -1,    -1,     9,    10,    11,    12,    -1,
      -1,    15,    16,    17,    18,    -1,    -1,    -1,    10,    -1,
      12,    -1,    -1,    15,    16,    17,    18,    -1,    -1,    -1,
      34,    35,    36,    -1,    38,    39,    -1,    -1,    42,    43,
      -1,    45,    34,    35,    36,    -1,    38,    39,    -1,    12,
      -1,    43,    15,    16,    17,    18,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    15,    16,    17,    18,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    38,    39,    -1,    -1,    -1,
      43,    44,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      12,    43,    44,    15,    16,    17,    18,    -1,    -1,    -1,
      12,    -1,    -1,    15,    16,    17,    18,    -1,    -1,    -1,
      -1,    -1,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      -1,    43,    34,    35,    36,    -1,    38,    39,    27,    28,
      29,    43,    -1,    32,    33,    34,    35,    36,    37,    -1,
      -1,    -1,    41,    -1,    43,    27,    28,    29,    47,    -1,
      32,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      -1,    43,    44,    27,    28,    29,    -1,    -1,    32,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    -1,    43,
      44,    27,    28,    29,    -1,    -1,    32,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    -1,    43,    44,    27,
      28,    29,    -1,    -1,    32,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    41,    42,    43,    27,    28,    29,    -1,
      -1,    32,    33,    34,    35,    36,    37,    -1,    -1,    -1,
      41,    42,    43,    27,    28,    29,    -1,    -1,    32,    33,
      34,    35,    36,    37,    -1,    -1,    -1,    41,    42,    43,
      27,    28,    29,    -1,    -1,    32,    33,    34,    35,    36,
      37,    -1,    -1,    -1,    41,    42,    43,    27,    28,    29,
      -1,    -1,    32,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    -1,    43,    27,    28,    29,    -1,    -1,    32,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    -1,
      43,    27,    28,    29,    -1,    -1,    32,    33,    34,    35,
      36,    37,    -1,    -1,    -1,    41,    -1,    43,    28,    29,
      -1,    -1,    32,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    41,    -1,    43
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     9,    10,    11,    12,    15,
      16,    17,    18,    34,    35,    36,    38,    39,    42,    43,
      45,    49,    50,    51,    52,    54,    16,    16,    43,    43,
      54,    16,    54,    16,    43,    54,    54,    54,    54,    54,
      16,    54,    46,    51,     0,    51,    27,    28,    29,    32,
      33,    34,    35,    36,    37,    41,    42,    43,    43,    50,
      45,    54,    54,    42,    27,    42,    42,    44,    54,    55,
      27,    27,    44,    44,    46,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    16,    44,    55,    10,    16,    56,
      57,    52,    53,    44,    44,    54,    47,    44,    54,    54,
      44,    16,    16,    47,    44,    53,    46,    50,    50,    42,
      55,    16,    57,    50,    14,    50
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    48,    49,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    51,    51,    52,    52,    52,    52,    52,
      53,    53,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    55,
      55,    56,    56,    57,    57
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     3,     3,     5,     5,
       7,     2,     3,     1,     2,     5,     3,     6,     5,     3,
       1,     2,     1,     1,     2,     3,     1,     3,     4,     3,
       4,     4,     4,     2,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     1,
       3,     2,     2,     1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 145 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_PROGRAM, 0, "", 1, (yyvsp[0].ast)); compile((yyval.ast)); }
#line 1539 "y.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 149 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_STATEMENT, 0, "", 0); }
#line 1545 "y.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 150 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[-1].ast); }
#line 1551 "y.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 151 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[0].ast); }
#line 1557 "y.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 152 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_PRINT, 0, "", 1, (yyvsp[-1].ast)); }
#line 1563 "y.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 153 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_RETURN, 0, "", 1, (yyvsp[-1].ast)); }
#line 1569 "y.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 154 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_WHILE, 0, "", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1575 "y.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 155 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_IF, 0, "", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1581 "y.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 156 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_IFELSE, 0, "", 3, (yyvsp[-4].ast), (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1587 "y.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 157 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_STATEMENT_LIST, 0, "", 0); }
#line 1593 "y.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 158 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[-1].ast); }
#line 1599 "y.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 162 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[0].ast); }
#line 1605 "y.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 163 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_STATEMENT_LIST, 0, "", 2, (yyvsp[-1].ast), (yyvsp[0].ast)); }
#line 1611 "y.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 167 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_CLASS, 0, (yyvsp[-3].str), 1, (yyvsp[-1].ast)); }
#line 1617 "y.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 168 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_FUNC, 0, (yyvsp[-1].str), 1, (yyvsp[0].ast)); }
#line 1623 "y.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 169 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_FUNC, 0, (yyvsp[-4].str), 2, (yyvsp[0].ast), (yyvsp[-2].ast)); }
#line 1629 "y.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 170 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_VAR_DECL, 0, (yyvsp[-3].str), 1, (yyvsp[-1].ast)); }
#line 1635 "y.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 171 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_VAR_DECL, 0, (yyvsp[-1].str), 0); }
#line 1641 "y.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 175 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[0].ast); }
#line 1647 "y.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 176 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_DECL_LIST, 0, "", 2, (yyvsp[-1].ast), (yyvsp[0].ast)); }
#line 1653 "y.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 180 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_NUMBER, 0, (yyvsp[0].str), 0); }
#line 1659 "y.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 181 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_STRING, (yyvsp[0].data).bytes, (yyvsp[0].data).data, 0); }
#line 1665 "y.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 182 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_NEW, 0, (yyvsp[0].str), 0); }
#line 1671 "y.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 183 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_IDENTIFIER, 0, (yyvsp[-1].str), 0); }
#line 1677 "y.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 184 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_IDENTIFIER, 0, (yyvsp[0].str), 0); }
#line 1683 "y.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 185 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_CALL, 0, "", 1, (yyvsp[-2].ast)); }
#line 1689 "y.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 186 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_CALL, 0, "", 2, (yyvsp[-3].ast), (yyvsp[-1].ast)); }
#line 1695 "y.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 187 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_BUILTIN, 0, (yyvsp[-2].str), 0); }
#line 1701 "y.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 188 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_BUILTIN, 0, (yyvsp[-3].str), 1, (yyvsp[-1].ast)); }
#line 1707 "y.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 189 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_ASSIGN, 0, "*", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1713 "y.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 190 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_ASSIGN, 0, "?", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1719 "y.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 191 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_UNARY, 0, "*", 1, (yyvsp[0].ast)); }
#line 1725 "y.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 192 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_UNARY, 0, "?", 1, (yyvsp[0].ast)); }
#line 1731 "y.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 193 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_UNARY, 0, "&", 1, (yyvsp[0].ast)); }
#line 1737 "y.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 194 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_UNARY, 0, "-", 1, (yyvsp[0].ast)); }
#line 1743 "y.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 195 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[0].ast); }
#line 1749 "y.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 196 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_ASSIGN, 0, "=", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1755 "y.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 197 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_CMPOP, 0, "=", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1761 "y.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 198 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_CMPOP, 0, "!", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1767 "y.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 199 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_STROP, 0, "+", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1773 "y.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 200 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_STROP, 0, "%", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1779 "y.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 201 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_OPERATION, 0, "+", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1785 "y.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 202 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_OPERATION, 0, "-", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1791 "y.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 203 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_OPERATION, 0, "*", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1797 "y.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 204 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_OPERATION, 0, "/", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1803 "y.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 205 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_ACCESS, 0, (yyvsp[0].str), 1, (yyvsp[-2].ast)); }
#line 1809 "y.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 206 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[-1].ast); }
#line 1815 "y.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 210 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[0].ast); }
#line 1821 "y.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 211 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_EXPRESSION_LIST, 0, "", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1827 "y.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 215 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_ARGUMENT, 0, (yyvsp[0].str), 0); }
#line 1833 "y.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 216 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_ARGUMENT, 0, (yyvsp[0].str), 1, (yyvsp[-1].str)); }
#line 1839 "y.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 220 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = (yyvsp[0].ast); }
#line 1845 "y.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 221 "compiler.y" /* yacc.c:1646  */
    { (yyval.ast) = new_node(yylineno, GRAMMAR_ARGUMENT_LIST, 0, "", 2, (yyvsp[-2].ast), (yyvsp[0].ast)); }
#line 1851 "y.tab.c" /* yacc.c:1646  */
    break;


#line 1855 "y.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 223 "compiler.y" /* yacc.c:1906  */


void yyerror(const char* string) {
	fflush(stdout);
	fprintf(stderr, "*** %s, line %d\n", string, yylineno);
	
}

static uint64_t stack_pointer = 0;
static class_t main_class = {0};
static class_t* current_class = &main_class;

static uint64_t inline_id = 0;
static uint64_t func_id = 0;

static int data_section_count = 0;
static int depth = 0;

static int has_defined_internal_send = 0;

class_t* create_class(class_t* self, char* identifier) {
	if (self->classes) self->classes = (class_t*) realloc(self->classes, (self->class_count + 1) * sizeof(class_t));
	else self->classes = (class_t*) malloc((self->class_count + 1) * sizeof(class_t));
	
	memset(&self->classes[self->class_count], 0, sizeof(class_t));
	strncpy(self->classes[self->class_count].identifier, identifier, sizeof(self->classes[self->class_count].identifier));
	
	self->classes[self->class_count].scope_depth = depth;
	self->classes[self->class_count].bytes = 0;
	
	return &self->classes[self->class_count++];
	
}

class_t* find_class_in_class(class_t* self, char* identifier) {
	for (int i = 0; i < self->class_count; i++) {
		class_t* class = find_class_in_class(&self->classes[i], identifier);
		if (class) return class;
		if (self->classes[i].scope_depth >= 0 && strncmp(self->classes[i].identifier, identifier, sizeof(self->classes[i].identifier)) == 0) return &self->classes[i];
		
	}
	
	return (class_t*) 0;
	
} class_t* find_class(char* identifier) {
	return find_class_in_class(&main_class, identifier);
	
}

uint64_t create_reference_in_class(class_t* self, char* identifier) {
	if (self->references) self->references = (reference_t*) realloc(self->references, (self->reference_count + 1) * sizeof(reference_t));
	else self->references = (reference_t*) malloc((self->reference_count + 1) * sizeof(reference_t));
	
	memset(&self->references[self->reference_count], 0, sizeof(reference_t));
	strncpy(self->references[self->reference_count].identifier, identifier, sizeof(self->references[self->reference_count].identifier));
	
	self->references[self->reference_count].scope_depth = depth;
	self->references[self->reference_count].stack_pointer = stack_pointer;
	
	stack_pointer += 8;
	return self->references[self->reference_count++].stack_pointer;
	
} uint64_t create_reference(char* identifier) {
	return create_reference_in_class(current_class, identifier);
	
}

uint64_t generate_stack_entry(node_t* self) {
	self->ref_code = (char*) malloc(64);
	sprintf(self->ref_code, "cad bp sub %ld\t", stack_pointer);
	self->ref = "?ad";
	
	uint64_t current_stack_pointer = stack_pointer;
	stack_pointer += 8;
	return current_stack_pointer;
	
} void decrement_depth(void) {
	for (int i = 0; i < current_class->reference_count; i++) if (current_class->references[i].scope_depth > depth + 1) current_class->references[i].scope_depth = -1;
	for (int i = 0; i < current_class->class_count; i++) if (current_class->classes[i].scope_depth > depth + 2) current_class->classes[i].scope_depth = -1;
	depth--;
	
}

void compile(node_t* self) {
	depth++;
	printf("\t# %d -> line = %d, type = %d, data = %s, children = %d\n", depth, self->line, self->type, self->data, self->child_count);
	
	if (self->type == GRAMMAR_PROGRAM) {
		fprintf(yyout, ":main:\tmov bp sp\tsub bp 1024\n");
		
	} else if (self->type == GRAMMAR_NEW) {
		self->class = find_class(self->data);
		generate_stack_entry(self);
		fprintf(yyout, "mov a0 %d\tcal malloc\t%smov %s g0\n", self->class->bytes, self->ref_code, self->ref);
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_ACCESS) {
		self->access_class_object = self->children[0];
		self->class = find_class(self->access_class_object->data);
		self->self_zero = 1;
		
		if (!self->class) {
			self->self_zero = 0;
			compile(self->access_class_object);
			self->class = self->access_class_object->class;
			
		}
		
		uint64_t offset = 0;
		for (int i = 0; i < self->class->reference_count; i++) {
			uint8_t found = strcmp(self->class->references[i].identifier, self->data) == 0;
			
			if (found) {
				self->ref_code = (char*) malloc(64);
				
				if (self->class->references[i].is_function) sprintf(self->ref_code, "cad bp sub %ld\t", self->class->references[i].stack_pointer);
				else sprintf(self->ref_code, "%scad %s add %ld\t", self->access_class_object->ref_code, self->access_class_object->ref, offset);
				
			} if (!self->class->references[i].is_function) {
				offset += 8;
				
			} if (found) {
				self->ref = "?ad";
				break;
				
			}
			
		}
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_CLASS) {
		class_t* class = create_class(current_class, self->data);
		
		if (self->child_count > 0) { // has members
			node_t* member_list_root = self->children[0];
			int member = 0;
			
			while (member_list_root) {
				node_t* current_node = member_list_root;
				if (member_list_root->type == GRAMMAR_DECL_LIST) current_node = member_list_root->children[0];
				
				if (current_node->type == GRAMMAR_VAR_DECL) {
					create_reference_in_class(class, current_node->data);
					class->bytes += 8;
					
				} else if (current_node->type == GRAMMAR_FUNC) {
					class_t* previous_class = current_class;
					current_class = class;
					compile(current_node);
					current_class = previous_class;
					
				} else if (current_node->type == GRAMMAR_CLASS) {
					/// TODO good luck with this one mate
					
				}
				
				if (member_list_root->type == GRAMMAR_DECL_LIST) member_list_root = member_list_root->children[1];
				else break;
				
			}
			
		}
		
		for (int i = 0; i < class->reference_count; i++) {
			printf("CLASS %s REFERENCE %d: %s\n", class->identifier, i, class->references[i].identifier);
			
		}
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_FUNC) {
		uint64_t current_func_id = func_id++;
		fprintf(yyout, "jmp $amber_func_%ld_end\t:$amber_func_%ld:\n", current_func_id, current_func_id);
		depth++;
		
		int argument = 0;
		if (current_class != &main_class) fprintf(yyout, "cad bp sub %ld\tmov ?ad a%d\n", create_reference("self"), argument++); // in class (take into account self argument)
		
		if (self->child_count > 1) { // has arguments
			node_t* argument_list_root = self->children[1];
			
			while (argument_list_root) {
				fprintf(yyout, "cad bp sub %ld\tmov ?ad a%d\n", create_reference((argument_list_root->type == GRAMMAR_ARGUMENT_LIST ? argument_list_root->children[0] : argument_list_root)->data), argument++);
				current_class->references[current_class->reference_count - 1].class = find_class((char*) (argument_list_root->type == GRAMMAR_ARGUMENT_LIST ? argument_list_root->children[0] : argument_list_root)->children[0]);
				
				if (argument_list_root->type == GRAMMAR_ARGUMENT_LIST) argument_list_root = argument_list_root->children[1];
				else break;
				
			}
			
		}
		
		compile(self->children[0]); // compile statement
		depth--;
		decrement_depth();
		
		fprintf(yyout, "mov g0 0\tret\t:$amber_func_%ld_end:\n", current_func_id);
		fprintf(yyout, "cad bp sub %ld\tmov ?ad $amber_func_%ld\n", create_reference(self->data), current_func_id);
		current_class->references[current_class->reference_count - 1].is_function = 1;
		
		return;
		
	} else if (self->type == GRAMMAR_WHILE) {
		uint64_t current_inline_id = inline_id++;
		fprintf(yyout, "jmp $amber_inline_%ld_condition\t:$amber_inline_%ld:\n", current_inline_id, current_inline_id);
		
		compile(self->children[1]); // compile statement
		fprintf(yyout, ":$amber_inline_%ld_condition:", current_inline_id);
		
		compile(self->children[0]); // compile expression
		fprintf(yyout, "%scnd %s\tjmp $amber_inline_%ld\t:$amber_inline_%ld_end:\n", self->children[0]->ref_code, self->children[0]->ref, current_inline_id, current_inline_id);
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_IF) {
		compile(self->children[0]); // compile expression
		
		uint64_t current_inline_id = inline_id++;
		fprintf(yyout, "jmp $amber_inline_%ld_condition\t:$amber_inline_%ld:\n", current_inline_id, current_inline_id);
		
		compile(self->children[1]); // compile statement
		fprintf(yyout, "jmp $amber_inline_%ld_end\t:$amber_inline_%ld_condition:\t%scnd %s\tjmp $amber_inline_%ld\t:$amber_inline_%ld_end:\n", current_inline_id, current_inline_id, self->children[0]->ref_code, self->children[0]->ref, current_inline_id, current_inline_id);
		
		depth--;
		return;
		
	} else if (self->type == GRAMMAR_IFELSE) {
		compile(self->children[0]); // compile expression
		
		uint64_t current_inline_id = inline_id++;
		fprintf(yyout, "jmp $amber_inline_%ld_condition\t:$amber_inline_%ld:\n", current_inline_id, current_inline_id);
		
		compile(self->children[1]); // compile statement (after if expression)
		fprintf(yyout, "jmp $amber_inline_%ld_end\t:$amber_inline_%ld_condition:\t%scnd %s\tjmp $amber_inline_%ld\n", current_inline_id, current_inline_id, self->children[0]->ref_code, self->children[0]->ref, current_inline_id);
		
		compile(self->children[2]); // compile statement (after else)
		fprintf(yyout, ":$amber_inline_%ld_end:\n", current_inline_id);
		
		depth--;
		return;
		
	}
	
	for (int i = 0; i < self->child_count; i++) {
		compile(self->children[i]);
		
		if (self->children[i]->class) self->class = self->children[i]->class;
		if (self->children[i]->access_class_object) self->access_class_object = self->children[i]->access_class_object;
		
	}
	
	decrement_depth();
	
	// literals
	
	if (self->type == GRAMMAR_NUMBER) {
		self->ref = self->data;
		
	} else if (self->type == GRAMMAR_STRING) {
		self->ref = (char*) malloc(64);
		memset(self->ref, 0, 64);
		sprintf(self->ref, "$amber_data_%d", data_section_count++);
		
		fprintf(yyout, "%%%s", self->ref);
		for (int i = 0; i < self->data_bytes; i++) fprintf(yyout, " x%x", self->data[i]);
		fprintf(yyout, "%%\n");
		
	} else if (self->type == GRAMMAR_IDENTIFIER) {
		for (int i = 0; i < current_class->reference_count; i++) {
			printf("=== %s %s %d\n", current_class->references[i].identifier, self->data,current_class->references[i].scope_depth);
			
			if (current_class->references[i].scope_depth >= 0 && strncmp(current_class->references[i].identifier, self->data, sizeof(current_class->references[i].identifier)) == 0) {
				self->class = current_class->references[i].class;
				printf("CLASS %s %p\n", current_class->references[i].identifier, self->class);
				
				self->ref_code = (char*) malloc(64);
				sprintf(self->ref_code, "cad bp sub %ld\t", self->stack_pointer = current_class->references[i].stack_pointer);
				
				self->ref = "?ad";
				break;
				
			}
			
		}
		
	}
	
	// statements
	
	else if (self->type == GRAMMAR_VAR_DECL) {
		uint64_t current_stack_pointer = create_reference(self->data);
		
		if (self->child_count) {
			current_class->references[current_class->reference_count - 1].class = self->children[0]->class;
			fprintf(yyout, "%smov g0 %s\tcad bp sub %ld\tmov ?ad g0\n", self->children[0]->ref_code, self->children[0]->ref, current_stack_pointer);
			
		}
		
	} else if (self->type == GRAMMAR_PRINT) {
		if (self->child_count) fprintf(yyout, "%smov a0 %s\tcal print\n", self->children[0]->ref_code, self->children[0]->ref);
		else fprintf(yyout, "mov a0 0\tcal print\n");
		
	} else if (self->type == GRAMMAR_RETURN) {
		if (self->child_count) fprintf(yyout, "%smov g0 %s\tret\n", self->children[0]->ref_code, self->children[0]->ref);
		else fprintf(yyout, "mov g0 0\nret\n");
		
	}
	
	// calls
	
	else if (self->type == GRAMMAR_CALL || self->type == GRAMMAR_BUILTIN) {
		generate_stack_entry(self);
		
		node_t* expression_list_root = (node_t*) 0;
		int argument = 0;
		if (self->children[0]->access_class_object) fprintf(yyout, "%smov a%d %s\t", self->children[0]->access_class_object->ref_code, argument++, self->children[0]->self_zero ? "0" : "ad");
		
		if (*self->data && self->child_count == 1) expression_list_root = self->children[0];
		else if (!*self->data && self->child_count == 2) expression_list_root = self->children[1];
		
		while (expression_list_root) {
			if (expression_list_root->type == GRAMMAR_EXPRESSION_LIST) {
				fprintf(yyout, "%smov a%d %s\t", expression_list_root->children[0]->ref_code, argument++, expression_list_root->children[0]->ref);
				expression_list_root = expression_list_root->children[1];
				
			} else {
				fprintf(yyout, "%smov a%d %s\t", expression_list_root->ref_code, argument++, expression_list_root->ref);
				break;
				
			}
			
		}
		
		if (self->type == GRAMMAR_BUILTIN) {
			if (strcmp(self->data, "str") == 0) {
				uint64_t current_inline_id = inline_id++;
				
				fprintf(yyout,
					"%smov g1 a0\tmov a0 16\tcal malloc\tadd g0 a0\tmov 1?g0 0\tsub g0 1\n"
					":$amber_internal_itos_loop_inline_%ld:\tsub g0 1\tdiv g1 %s\tadd a3 48\tmov 1?g0 a3\n"
					"cnd g1\tjmp $amber_internal_itos_loop_inline_%ld\n", argument > 1 ? "mov g3 a1\t" : "", current_inline_id, argument > 1 ? "g3" : "10", current_inline_id);
				
			} else {
				printf("ERROR ON LINE %d %s\n", __LINE__, self->data);
				
			}
			
		} else {
			printf("%p %d\n", self->children[0]->ref, self->child_count);
			fprintf(yyout, "%scal %s\t", self->children[0]->ref_code, self->children[0]->ref);
			
		}
		
		fprintf(yyout, "%smov %s g0\n", self->ref_code, self->ref);
		
	}
	
	// operations
	
	else if (self->type == GRAMMAR_ASSIGN) {
		generate_stack_entry(self);
		
		if (self->data[0] == '=') fprintf(yyout, "%smov g0 %s\t%smov %s g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
		else if (self->data[0] == '*') fprintf(yyout, "%smov g0 %s\t%smov g1 %s\tmov 1?g1 g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
		else if (self->data[0] == '?') fprintf(yyout, "%smov g0 %s\t%smov g1 %s\tmov 8?g1 g0\t%smov %s g0\n", self->children[1]->ref_code, self->children[1]->ref, self->children[0]->ref_code, self->children[0]->ref, self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_CMPOP) {
		generate_stack_entry(self);
		
		if (self->data[0] == '=') fprintf(yyout, "mov g0 0\t%smov g1 %s\t%smov g2 %s\tcmp g1 g2\tcnd zf\tmov g0 1\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
		else if (self->data[0] == '!') fprintf(yyout, "mov g0 1\t%smov g1 %s\t%smov g2 %s\tcmp g1 g2\tcnd zf\tmov g0 0\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_OPERATION) {
		generate_stack_entry(self);
		char* operator_instruction = "nop";
		
		if (self->data[0] == '+') operator_instruction = "add";
		else if (self->data[0] == '-') operator_instruction = "sub";
		else if (self->data[0] == '*') operator_instruction = "mul";
		else if (self->data[0] == '/') operator_instruction = "div";
		
		fprintf(yyout, "%smov g0 %s\t%s%s g0 %s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, operator_instruction, self->children[1]->ref, self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_UNARY) {
		generate_stack_entry(self);
		char* unary_code = "nop g0";
		
		if (self->data[0] == '-') unary_code = "not g0\tadd g0 1";
		else if (self->data[0] == '~') unary_code = "not g0";
		else if (self->data[0] == '*') unary_code = "mov g0 1?g0";
		else if (self->data[0] == '?') unary_code = "mov g0 8?g0";
		else if (self->data[0] == '&') unary_code = "mov g0 ad";
		
		fprintf(yyout, "%smov g0 %s\t%s\t%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, unary_code, self->ref_code, self->ref);
		
	} else if (self->type == GRAMMAR_STROP) {
		generate_stack_entry(self);
		char* strop_code = "";
		
		if (self->data[0] == '+') {
			if (!has_defined_internal_send) {
				has_defined_internal_send = 1;
				fprintf(yyout,
					"jmp $amber_internal_send_end\t:$amber_internal_send:\n"
					"\tjmp $amber_internal_send_cond\t:$amber_internal_send_loop:\n"
					"\t\tadd a0 1\n"
					"\t\t:$amber_internal_send_cond:\tcnd 1?a0\tjmp $amber_internal_send_loop\tret\t:$amber_internal_send_end:\n");
				
			}
			
			strop_code =
				"mov g2 g0\tmov a0 g2\tcal $amber_internal_send\tmov g3 a0\tsub g3 g0\n"
				"mov a0 g1\tcal $amber_internal_send\tmov a3 a0\tsub a3 g1\tadd a3 1\n"
				"mov a0 g3\tadd a0 a3\tcal malloc\n"
				"mov a0 g0\tmov a1 g2\tmov a2 g3\tcal mcpy\n"
				"add a0 g3\tmov a1 g1\tmov a2 a3\tcal mcpy\n"
				"mov g0 a0\tsub g0 g3\n";
			
		}
		
		fprintf(yyout, "%smov g0 %s\t%smov g1 %s\n%s%smov %s g0\n", self->children[0]->ref_code, self->children[0]->ref, self->children[1]->ref_code, self->children[1]->ref, strop_code, self->ref_code, self->ref);
		
	}
	
	// misc
	
	else if (self->type == GRAMMAR_PROGRAM) {
		fprintf(yyout, "mov g0 0\tret\n");
		
	}
	
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		FILE* file = fopen(argv[1], "r");
		if (!file) {
			fprintf(stderr, "failed open");
			exit(1);
			
		}
		
		yyin = file;
		yyout = stdout;
		
		if (argc > 2) {
			FILE* file = fopen(argv[2], "w");
			if (!file) {
				fprintf(stderr, "failed open");
				exit(1);
				
			}
			
			yyout = file;
			
		}
		
	}
	
	yyparse();
	fclose(yyout);
	system("geany main.asm");
	return 0; 
}
