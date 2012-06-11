/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse slice_parse
#define yylex   slice_lex
#define yyerror slice_error
#define yylval  slice_lval
#define yychar  slice_char
#define yydebug slice_debug
#define yynerrs slice_nerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ICE_MODULE = 258,
     ICE_CLASS = 259,
     ICE_INTERFACE = 260,
     ICE_EXCEPTION = 261,
     ICE_STRUCT = 262,
     ICE_SEQUENCE = 263,
     ICE_DICTIONARY = 264,
     ICE_ENUM = 265,
     ICE_OUT = 266,
     ICE_EXTENDS = 267,
     ICE_IMPLEMENTS = 268,
     ICE_THROWS = 269,
     ICE_VOID = 270,
     ICE_BYTE = 271,
     ICE_BOOL = 272,
     ICE_SHORT = 273,
     ICE_INT = 274,
     ICE_LONG = 275,
     ICE_FLOAT = 276,
     ICE_DOUBLE = 277,
     ICE_STRING = 278,
     ICE_OBJECT = 279,
     ICE_LOCAL_OBJECT = 280,
     ICE_LOCAL = 281,
     ICE_CONST = 282,
     ICE_FALSE = 283,
     ICE_TRUE = 284,
     ICE_IDEMPOTENT = 285,
     ICE_OPTIONAL = 286,
     ICE_SCOPE_DELIMITER = 287,
     ICE_IDENTIFIER = 288,
     ICE_STRING_LITERAL = 289,
     ICE_INTEGER_LITERAL = 290,
     ICE_FLOATING_POINT_LITERAL = 291,
     ICE_IDENT_OP = 292,
     ICE_KEYWORD_OP = 293,
     ICE_OPTIONAL_OP = 294,
     ICE_METADATA_OPEN = 295,
     ICE_METADATA_CLOSE = 296,
     ICE_GLOBAL_METADATA_OPEN = 297,
     ICE_GLOBAL_METADATA_CLOSE = 298,
     BAD_CHAR = 299
   };
#endif
/* Tokens.  */
#define ICE_MODULE 258
#define ICE_CLASS 259
#define ICE_INTERFACE 260
#define ICE_EXCEPTION 261
#define ICE_STRUCT 262
#define ICE_SEQUENCE 263
#define ICE_DICTIONARY 264
#define ICE_ENUM 265
#define ICE_OUT 266
#define ICE_EXTENDS 267
#define ICE_IMPLEMENTS 268
#define ICE_THROWS 269
#define ICE_VOID 270
#define ICE_BYTE 271
#define ICE_BOOL 272
#define ICE_SHORT 273
#define ICE_INT 274
#define ICE_LONG 275
#define ICE_FLOAT 276
#define ICE_DOUBLE 277
#define ICE_STRING 278
#define ICE_OBJECT 279
#define ICE_LOCAL_OBJECT 280
#define ICE_LOCAL 281
#define ICE_CONST 282
#define ICE_FALSE 283
#define ICE_TRUE 284
#define ICE_IDEMPOTENT 285
#define ICE_OPTIONAL 286
#define ICE_SCOPE_DELIMITER 287
#define ICE_IDENTIFIER 288
#define ICE_STRING_LITERAL 289
#define ICE_INTEGER_LITERAL 290
#define ICE_FLOATING_POINT_LITERAL 291
#define ICE_IDENT_OP 292
#define ICE_KEYWORD_OP 293
#define ICE_OPTIONAL_OP 294
#define ICE_METADATA_OPEN 295
#define ICE_METADATA_CLOSE 296
#define ICE_GLOBAL_METADATA_OPEN 297
#define ICE_GLOBAL_METADATA_CLOSE 298
#define BAD_CHAR 299




/* Copy the first part of user declarations.  */
#line 1 "../Slice/Grammar.y"


// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/GrammarUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/UUID.h>
#include <cstring>

#ifdef _MSC_VER
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
#endif

using namespace std;
using namespace Slice;

void
slice_error(const char* s)
{
    // yacc and recent versions of Bison use "syntax error" instead
    // of "parse error".

    if (strcmp(s, "parse error") == 0)
    {
	unit->error("syntax error");
    }
    else
    {
	unit->error(s);
    }
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 247 "Grammar.tab.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   875

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  69
/* YYNRULES -- Number of rules.  */
#define YYNRULES  190
/* YYNRULES -- Number of states.  */
#define YYNSTATES  282

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   299

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    48,    53,     2,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    45,
      51,    49,    52,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    46,     2,    47,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     9,    13,    14,    15,    19,    20,
      26,    27,    32,    35,    36,    38,    40,    42,    44,    46,
      48,    50,    52,    54,    56,    58,    60,    62,    63,    70,
      73,    76,    79,    80,    88,    91,    92,    97,   101,   104,
     105,   108,   112,   116,   119,   121,   124,   126,   128,   131,
     134,   137,   138,   145,   150,   154,   157,   158,   160,   163,
     166,   169,   170,   179,   182,   183,   186,   187,   192,   196,
     199,   200,   202,   206,   209,   211,   214,   216,   218,   221,
     225,   228,   232,   233,   239,   240,   246,   248,   250,   253,
     256,   259,   260,   268,   272,   274,   276,   279,   280,   285,
     289,   292,   293,   295,   299,   301,   303,   305,   313,   321,
     332,   343,   346,   349,   350,   357,   363,   367,   369,   371,
     373,   374,   376,   377,   378,   382,   388,   393,   400,   404,
     410,   413,   414,   416,   419,   423,   425,   427,   429,   431,
     433,   435,   437,   439,   441,   444,   446,   448,   451,   454,
     456,   460,   462,   464,   465,   467,   469,   471,   473,   475,
     477,   484,   490,   492,   494,   496,   498,   500,   502,   504,
     506,   508,   510,   512,   514,   516,   518,   520,   522,   524,
     526,   528,   530,   532,   534,   536,   538,   540,   542,   544,
     546
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      55,     0,    -1,    58,    -1,    42,   118,    43,    -1,    40,
     118,    41,    -1,    -1,    -1,    56,    59,    58,    -1,    -1,
      57,    62,    60,    45,    58,    -1,    -1,     1,    45,    61,
      58,    -1,    57,    62,    -1,    -1,    63,    -1,    82,    -1,
      83,    -1,    96,    -1,    97,    -1,    66,    -1,    67,    -1,
      76,    -1,    77,    -1,   105,    -1,   106,    -1,   108,    -1,
     121,    -1,    -1,     3,    33,    64,    46,    58,    47,    -1,
       6,    33,    -1,     6,   122,    -1,   119,    65,    -1,    -1,
     119,    65,    69,    68,    46,    70,    47,    -1,    12,   115,
      -1,    -1,    57,    74,    45,    70,    -1,     1,    45,    70,
      -1,    57,    74,    -1,    -1,   116,    33,    -1,    39,    35,
      48,    -1,    39,   115,    48,    -1,    39,    48,    -1,    31,
      -1,    72,    71,    -1,    71,    -1,    88,    -1,     7,    33,
      -1,     7,   122,    -1,   119,    75,    -1,    -1,   119,    75,
      78,    46,    79,    47,    -1,    57,    80,    45,    79,    -1,
       1,    45,    79,    -1,    57,    80,    -1,    -1,    88,    -1,
       4,    33,    -1,     4,   122,    -1,   119,    81,    -1,    -1,
     119,    81,    85,    86,    84,    46,    87,    47,    -1,    12,
     115,    -1,    -1,    13,    99,    -1,    -1,    57,    94,    45,
      87,    -1,     1,    45,    87,    -1,    57,    94,    -1,    -1,
      73,    -1,    73,    49,   120,    -1,   116,   122,    -1,   116,
      -1,    72,   116,    -1,   116,    -1,    15,    -1,    89,    37,
      -1,    30,    89,    37,    -1,    89,    38,    -1,    30,    89,
      38,    -1,    -1,    90,   113,    48,    92,   114,    -1,    -1,
      90,     1,    48,    93,   114,    -1,    88,    -1,    91,    -1,
       5,    33,    -1,     5,   122,    -1,   119,    95,    -1,    -1,
     119,    95,   100,    98,    46,   101,    47,    -1,   115,    50,
      99,    -1,   115,    -1,    24,    -1,    12,    99,    -1,    -1,
      57,   102,    45,   101,    -1,     1,    45,   101,    -1,    57,
     102,    -1,    -1,    91,    -1,   104,    50,   103,    -1,   104,
      -1,   115,    -1,   122,    -1,   119,     8,    51,    57,   116,
      52,    33,    -1,   119,     8,    51,    57,   116,    52,   122,
      -1,   119,     9,    51,    57,   116,    50,    57,   116,    52,
      33,    -1,   119,     9,    51,    57,   116,    50,    57,   116,
      52,   122,    -1,    10,    33,    -1,    10,   122,    -1,    -1,
     119,   107,   109,    46,   110,    47,    -1,   119,    10,    46,
     110,    47,    -1,   111,    50,   110,    -1,   111,    -1,    33,
      -1,   122,    -1,    -1,    11,    -1,    -1,    -1,   112,    57,
      73,    -1,   113,    50,   112,    57,    73,    -1,   112,    57,
     116,   122,    -1,   113,    50,   112,    57,   116,   122,    -1,
     112,    57,   116,    -1,   113,    50,   112,    57,   116,    -1,
      14,   103,    -1,    -1,    33,    -1,    32,    33,    -1,   115,
      32,    33,    -1,    16,    -1,    17,    -1,    18,    -1,    19,
      -1,    20,    -1,    21,    -1,    22,    -1,    23,    -1,    24,
      -1,    24,    53,    -1,    25,    -1,   115,    -1,   115,    53,
      -1,    34,   117,    -1,    34,    -1,   118,    50,   117,    -1,
     117,    -1,    26,    -1,    -1,    35,    -1,    36,    -1,   115,
      -1,    34,    -1,    28,    -1,    29,    -1,    27,    57,   116,
      33,    49,   120,    -1,    27,    57,   116,    49,   120,    -1,
       3,    -1,     4,    -1,     5,    -1,     6,    -1,     7,    -1,
       8,    -1,     9,    -1,    10,    -1,    11,    -1,    12,    -1,
      13,    -1,    14,    -1,    15,    -1,    16,    -1,    17,    -1,
      18,    -1,    19,    -1,    20,    -1,    21,    -1,    22,    -1,
      23,    -1,    24,    -1,    25,    -1,    26,    -1,    27,    -1,
      28,    -1,    29,    -1,    30,    -1,    31,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   107,   107,   115,   124,   129,   138,   137,   147,   146,
     157,   156,   161,   166,   173,   177,   181,   185,   189,   193,
     197,   201,   205,   209,   213,   217,   221,   231,   230,   264,
     268,   279,   290,   289,   316,   325,   333,   342,   345,   350,
     357,   370,   390,   460,   468,   481,   489,   504,   510,   514,
     525,   536,   535,   571,   580,   583,   588,   595,   601,   605,
     616,   630,   629,   669,   704,   712,   717,   725,   734,   737,
     742,   749,   771,   798,   820,   846,   852,   860,   872,   896,
     921,   945,   976,   975,   998,   997,  1020,  1021,  1027,  1031,
    1042,  1057,  1056,  1091,  1126,  1161,  1171,  1176,  1184,  1193,
    1196,  1201,  1208,  1214,  1221,  1233,  1245,  1256,  1265,  1280,
    1291,  1308,  1312,  1324,  1323,  1347,  1362,  1368,  1376,  1388,
    1396,  1405,  1412,  1423,  1425,  1441,  1457,  1469,  1481,  1492,
    1508,  1513,  1521,  1524,  1530,  1543,  1547,  1551,  1555,  1559,
    1563,  1567,  1571,  1575,  1579,  1583,  1587,  1606,  1647,  1653,
    1661,  1668,  1680,  1687,  1697,  1710,  1723,  1769,  1780,  1791,
    1807,  1816,  1830,  1833,  1836,  1839,  1842,  1845,  1848,  1851,
    1854,  1857,  1860,  1863,  1866,  1869,  1872,  1875,  1878,  1881,
    1884,  1887,  1890,  1893,  1896,  1899,  1902,  1905,  1908,  1911,
    1914
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ICE_MODULE", "ICE_CLASS",
  "ICE_INTERFACE", "ICE_EXCEPTION", "ICE_STRUCT", "ICE_SEQUENCE",
  "ICE_DICTIONARY", "ICE_ENUM", "ICE_OUT", "ICE_EXTENDS", "ICE_IMPLEMENTS",
  "ICE_THROWS", "ICE_VOID", "ICE_BYTE", "ICE_BOOL", "ICE_SHORT", "ICE_INT",
  "ICE_LONG", "ICE_FLOAT", "ICE_DOUBLE", "ICE_STRING", "ICE_OBJECT",
  "ICE_LOCAL_OBJECT", "ICE_LOCAL", "ICE_CONST", "ICE_FALSE", "ICE_TRUE",
  "ICE_IDEMPOTENT", "ICE_OPTIONAL", "ICE_SCOPE_DELIMITER",
  "ICE_IDENTIFIER", "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENT_OP", "ICE_KEYWORD_OP",
  "ICE_OPTIONAL_OP", "ICE_METADATA_OPEN", "ICE_METADATA_CLOSE",
  "ICE_GLOBAL_METADATA_OPEN", "ICE_GLOBAL_METADATA_CLOSE", "BAD_CHAR",
  "';'", "'{'", "'}'", "')'", "'='", "','", "'<'", "'>'", "'*'", "$accept",
  "start", "global_meta_data", "meta_data", "definitions", "@1", "@2",
  "@3", "definition", "module_def", "@4", "exception_id", "exception_decl",
  "exception_def", "@5", "exception_extends", "exception_exports",
  "type_id", "optional", "optional_type_id", "exception_export",
  "struct_id", "struct_decl", "struct_def", "@6", "struct_exports",
  "struct_export", "class_id", "class_decl", "class_def", "@7",
  "class_extends", "implements", "class_exports", "data_member",
  "return_type", "operation_preamble", "operation", "@8", "@9",
  "class_export", "interface_id", "interface_decl", "interface_def", "@10",
  "interface_list", "interface_extends", "interface_exports",
  "interface_export", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_id", "enum_def", "@11", "enumerator_list",
  "enumerator", "out_qualifier", "parameters", "throws", "scoped_name",
  "type", "string_literal", "string_list", "local_qualifier",
  "const_initializer", "const_def", "keyword", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,    59,   123,   125,    41,    61,
      44,    60,    62,    42
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    54,    55,    56,    57,    57,    59,    58,    60,    58,
      61,    58,    58,    58,    62,    62,    62,    62,    62,    62,
      62,    62,    62,    62,    62,    62,    62,    64,    63,    65,
      65,    66,    68,    67,    69,    69,    70,    70,    70,    70,
      71,    72,    72,    72,    72,    73,    73,    74,    75,    75,
      76,    78,    77,    79,    79,    79,    79,    80,    81,    81,
      82,    84,    83,    85,    85,    86,    86,    87,    87,    87,
      87,    88,    88,    88,    88,    89,    89,    89,    90,    90,
      90,    90,    92,    91,    93,    91,    94,    94,    95,    95,
      96,    98,    97,    99,    99,    99,   100,   100,   101,   101,
     101,   101,   102,   103,   103,   104,   104,   105,   105,   106,
     106,   107,   107,   109,   108,   108,   110,   110,   111,   111,
     111,   112,   112,   113,   113,   113,   113,   113,   113,   113,
     114,   114,   115,   115,   115,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   117,   117,
     118,   118,   119,   119,   120,   120,   120,   120,   120,   120,
     121,   121,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   122,   122,   122,   122,   122,
     122
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     3,     0,     0,     3,     0,     5,
       0,     4,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     6,     2,
       2,     2,     0,     7,     2,     0,     4,     3,     2,     0,
       2,     3,     3,     2,     1,     2,     1,     1,     2,     2,
       2,     0,     6,     4,     3,     2,     0,     1,     2,     2,
       2,     0,     8,     2,     0,     2,     0,     4,     3,     2,
       0,     1,     3,     2,     1,     2,     1,     1,     2,     3,
       2,     3,     0,     5,     0,     5,     1,     1,     2,     2,
       2,     0,     7,     3,     1,     1,     2,     0,     4,     3,
       2,     0,     1,     3,     1,     1,     1,     7,     7,    10,
      10,     2,     2,     0,     6,     5,     3,     1,     1,     1,
       0,     1,     0,     0,     3,     5,     4,     6,     3,     5,
       2,     0,     1,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     2,     2,     1,
       3,     1,     1,     0,     1,     1,     1,     1,     1,     1,
       6,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     6,   153,     2,    10,   149,
     151,     0,     0,     1,     0,     0,   152,     5,    12,    14,
      19,    20,    21,    22,    15,    16,    17,    18,    23,    24,
      25,     0,    26,     0,   148,     4,     0,     3,     7,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
      50,    60,    90,   113,    11,   150,     0,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   145,     0,   132,   146,
       0,     0,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,    58,    59,    88,    89,    29,    30,    48,    49,     5,
       5,   111,   120,   112,     0,    32,     0,     0,    66,     0,
      91,     0,     0,   144,   133,     0,   147,     0,     0,     9,
       0,     0,   118,     0,   117,   119,    34,     0,     0,    63,
       0,    61,    95,    96,    94,     0,   120,     0,   134,     0,
     158,   159,   157,   154,   155,   156,   161,     0,     0,   115,
     120,     0,     0,     0,     0,    65,     0,     0,     0,     0,
      28,   160,     0,     5,   116,     0,     0,     0,     0,    44,
       0,    46,     0,    71,    55,    57,    74,    52,     0,    93,
       0,     0,     0,   114,   107,   108,     0,     0,    38,    47,
      33,    54,     0,    43,     0,    45,     0,     0,     0,    40,
      73,     0,     0,     0,     0,    77,     0,     0,     0,     0,
     102,   100,    76,    92,     0,    37,     0,    41,    42,    72,
      53,     0,     0,    86,    87,    69,    74,    62,    99,     0,
      75,    78,    80,     0,   121,     5,     0,     0,     0,    36,
      68,    75,     0,    79,    81,    84,     0,    82,   122,    98,
     109,   110,    67,   131,   124,   128,   131,     5,     0,    85,
     126,    83,     0,   130,   104,   105,   106,   125,   129,     0,
     127,   103
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,     6,     7,    14,    41,    33,    18,    19,
      56,    49,    20,    21,   137,   115,   177,   181,   182,   183,
     198,    50,    22,    23,   116,   164,   184,    51,    24,    25,
     166,   118,   141,   213,   185,   218,   219,   220,   266,   263,
     235,    52,    26,    27,   145,   143,   120,   192,   221,   273,
     274,    28,    29,    53,    30,   121,   133,   134,   245,   246,
     269,    69,   186,    10,    11,    31,   156,    32,   135
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -195
static const yytype_int16 yypact[] =
{
     494,   -19,     0,     0,    89,  -195,    33,  -195,  -195,     0,
    -195,   -18,   -21,  -195,    38,    61,  -195,    56,    65,  -195,
    -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,   118,  -195,    38,  -195,  -195,     0,  -195,  -195,  -195,
     116,    68,   601,   632,   663,   694,    64,    66,   460,     5,
      70,     3,     6,  -195,  -195,  -195,    72,  -195,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,    76,  -195,    86,  -195,    30,
      41,    38,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,    56,
      56,  -195,   725,  -195,    37,  -195,    84,    37,   107,    67,
    -195,    96,   415,  -195,  -195,   110,  -195,    97,    73,  -195,
     116,   116,  -195,   100,   102,  -195,   121,   104,   337,   121,
      67,  -195,  -195,  -195,    36,   108,   725,   111,  -195,    73,
    -195,  -195,  -195,  -195,  -195,   121,  -195,   105,   112,  -195,
     725,   370,   115,   836,   119,  -195,   117,    67,   271,   120,
    -195,  -195,   756,    56,  -195,   123,   836,   125,   337,  -195,
      40,  -195,   116,   124,   129,  -195,   787,  -195,   304,  -195,
     130,   413,   131,  -195,  -195,  -195,   116,   370,   132,  -195,
    -195,  -195,   128,  -195,    47,  -195,   146,    73,   337,  -195,
    -195,   136,   413,   135,   271,  -195,   492,   116,    39,   235,
    -195,   138,  -195,  -195,   113,  -195,   370,  -195,  -195,  -195,
    -195,   304,   116,  -195,  -195,   139,   534,  -195,  -195,    74,
    -195,  -195,  -195,   140,  -195,    56,    34,   271,   818,  -195,
    -195,   146,   304,  -195,  -195,  -195,   836,  -195,   175,  -195,
    -195,  -195,  -195,   173,  -195,   787,   173,    56,   570,  -195,
    -195,  -195,   836,  -195,   142,   121,  -195,  -195,   787,   570,
    -195,  -195
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -195,  -195,  -195,   -17,    -8,  -195,  -195,  -195,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,  -195,  -170,  -171,  -179,  -175,
    -195,  -195,  -195,  -195,  -195,  -154,  -195,  -195,  -195,  -195,
    -195,  -195,  -195,  -165,  -162,   -23,  -195,   -16,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,  -112,  -195,  -194,  -195,   -85,
    -195,  -195,  -195,  -195,  -195,  -195,  -125,  -195,   -60,  -195,
     -67,  -109,   -27,    21,   197,  -195,  -140,  -195,   -41
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -124
static const yytype_int16 yytable[] =
{
      40,   102,   104,   106,   108,   136,    38,   113,   139,   171,
     144,   205,   217,    70,   199,   117,   -64,   114,   119,   155,
     238,   169,    37,    35,   201,    54,     8,   225,   165,    36,
      34,   144,    36,   232,     9,   174,    15,   217,   -13,     1,
     155,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,   -64,
     233,   -35,   -97,   259,   230,   189,   249,    55,   144,    16,
      17,   205,   125,   129,    -5,    -5,   250,   229,   125,    67,
      68,   204,    67,    68,   127,   202,   241,   242,     2,   125,
       3,   264,   257,   126,   258,   -13,   167,   262,   203,    13,
     128,   142,   130,   131,    39,   228,     2,   277,   155,    67,
      68,   150,   151,   157,   158,    67,    68,   152,   153,   154,
      -8,   253,   254,    71,   147,   109,   -51,   110,   122,   124,
     140,   163,    42,    43,    44,    45,    46,    47,    48,   123,
     138,   195,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,   146,   148,   176,   210,   149,   159,    67,    68,
     161,   191,   160,   125,   168,   206,   196,   172,   170,   275,
     178,   163,   173,   188,   222,   248,   187,   193,   197,   224,
     275,   212,   200,   207,   208,   214,   227,   226,   223,   209,
     176,   231,   237,   247,   252,   236,   244,   268,   255,   222,
     240,   163,   279,   239,   281,   210,   234,   191,   267,   271,
      12,     0,     0,     0,     0,   251,     0,   261,     0,   176,
       0,     0,     0,     0,   212,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,     0,     0,   276,   256,   265,
     191,     0,     0,     0,     0,   212,   243,   280,   276,     0,
       0,     0,     0,     0,     0,   278,   244,     0,     0,     0,
     272,  -122,  -122,  -122,  -122,  -122,  -122,  -122,  -122,  -122,
    -122,     0,     0,     0,     0,     0,  -122,  -122,  -122,     0,
       0,     0,   190,     0,  -122,  -122,     0,     0,     0,     0,
       0,     0,     0,  -123,     0,  -123,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,     0,     0,     0,
       0,    -5,    -5,    -5,    -5,   211,     0,     0,     0,     0,
      -5,     2,     0,     0,     0,     0,     0,     0,  -101,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
       0,     0,     0,     0,    -5,    -5,    -5,    -5,   162,     0,
       0,     0,     0,    -5,     2,     0,     0,     0,     0,     0,
       0,   -70,     0,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,     0,     0,     0,     0,     0,    -5,    -5,
      -5,   175,     0,     0,     0,     0,    -5,     2,     0,     0,
       0,     0,     0,     0,   -56,     0,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,     0,     0,     0,     0,
       0,    -5,    -5,    -5,     0,     0,     0,     0,     0,    -5,
       2,     0,     0,     0,     0,     0,     1,   -39,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,     0,     0,   215,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,     0,
       0,    -5,    -5,   216,   179,    67,    68,     0,     0,     0,
       0,     0,   180,     0,     0,     2,     0,     3,     0,     0,
       0,     0,   -13,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,     0,   111,   -13,     1,     0,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,     0,   112,   215,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,     0,     0,
      -5,    -5,     0,   179,    67,    68,     0,     0,     0,     0,
       0,   180,     0,     0,     2,     0,     3,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,     0,   209,     0,     0,
       0,   -76,   -76,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,    67,    68,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,     0,   101,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,     0,   103,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,     0,   105,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,     0,   107,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,     0,   132,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,     0,   194,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,     0,
     209,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
       0,   260,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,     0,     0,     0,     0,     0,   179,    67,    68,
       0,     0,     0,     0,     0,   180
};

static const yytype_int16 yycheck[] =
{
      17,    42,    43,    44,    45,   114,    14,    48,   117,   149,
     119,   182,   191,    40,   176,    12,    13,    12,    12,   128,
     214,   146,    43,    41,   178,    33,    45,   197,   140,    50,
       9,   140,    50,   212,    34,   160,     3,   216,     0,     1,
     149,     3,     4,     5,     6,     7,     8,     9,    10,    46,
     212,    46,    46,   247,   208,   167,   226,    36,   167,    26,
      27,   232,    32,    71,    26,    27,   231,   207,    32,    32,
      33,   180,    32,    33,    33,    35,    37,    38,    40,    32,
      42,   256,    48,    53,    50,    47,    50,   252,    48,     0,
      49,    24,   109,   110,    33,    48,    40,   272,   207,    32,
      33,    28,    29,   130,   131,    32,    33,    34,    35,    36,
      45,    37,    38,    45,   122,    51,    46,    51,    46,    33,
      13,   138,     4,     5,     6,     7,     8,     9,    10,    53,
      46,   172,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    46,    33,   161,   186,    49,    47,    32,    33,
      46,   168,    50,    32,    46,   182,   173,    52,    47,   268,
      45,   178,    50,    46,   191,    52,    47,    47,    45,   196,
     279,   188,    47,    49,    45,    45,    48,    45,    47,    33,
     197,    45,    47,    45,    45,   212,    11,    14,    48,   216,
     217,   208,    50,   216,   279,   236,   212,   214,   258,   266,
       3,    -1,    -1,    -1,    -1,   232,    -1,   248,    -1,   226,
      -1,    -1,    -1,    -1,   231,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   265,    -1,    -1,   268,   245,   256,
     247,    -1,    -1,    -1,    -1,   252,     1,   278,   279,    -1,
      -1,    -1,    -1,    -1,    -1,   272,    11,    -1,    -1,    -1,
     267,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,    -1,
      -1,    -1,     1,    -1,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    -1,    50,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    -1,    -1,    -1,
      -1,    30,    31,    32,    33,     1,    -1,    -1,    -1,    -1,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    47,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      -1,    -1,    -1,    -1,    30,    31,    32,    33,     1,    -1,
      -1,    -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    47,    -1,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,
      33,     1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    47,    -1,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,
      -1,    31,    32,    33,    -1,    -1,    -1,    -1,    -1,    39,
      40,    -1,    -1,    -1,    -1,    -1,     1,    47,     3,     4,
       5,     6,     7,     8,     9,    10,    -1,    -1,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      -1,    26,    27,    30,    31,    32,    33,    -1,    -1,    -1,
      -1,    -1,    39,    -1,    -1,    40,    -1,    42,    -1,    -1,
      -1,    -1,    47,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    33,     0,     1,    -1,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    46,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      26,    27,    -1,    31,    32,    33,    -1,    -1,    -1,    -1,
      -1,    39,    -1,    -1,    40,    -1,    42,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    33,    -1,    -1,
      -1,    37,    38,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    33,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    33,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    33,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    33,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    33,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    33,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      33,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    33,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,
      -1,    -1,    -1,    -1,    -1,    39
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    40,    42,    55,    56,    57,    58,    45,    34,
     117,   118,   118,     0,    59,     3,    26,    27,    62,    63,
      66,    67,    76,    77,    82,    83,    96,    97,   105,   106,
     108,   119,   121,    61,   117,    41,    50,    43,    58,    33,
      57,    60,     4,     5,     6,     7,     8,     9,    10,    65,
      75,    81,    95,   107,    58,   117,    64,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    32,    33,   115,
     116,    45,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    33,   122,    33,   122,    33,   122,    33,   122,    51,
      51,    33,    46,   122,    12,    69,    78,    12,    85,    12,
     100,   109,    46,    53,    33,    32,    53,    33,    49,    58,
      57,    57,    33,   110,   111,   122,   115,    68,    46,   115,
      13,    86,    24,    99,   115,    98,    46,    58,    33,    49,
      28,    29,    34,    35,    36,   115,   120,   116,   116,    47,
      50,    46,     1,    57,    79,    99,    84,    50,    46,   110,
      47,   120,    52,    50,   110,     1,    57,    70,    45,    31,
      39,    71,    72,    73,    80,    88,   116,    47,    46,    99,
       1,    57,   101,    47,    33,   122,    57,    45,    74,    88,
      47,    79,    35,    48,   115,    71,   116,    49,    45,    33,
     122,     1,    57,    87,    45,    15,    30,    72,    89,    90,
      91,   102,   116,    47,   116,    70,    45,    48,    48,   120,
      79,    45,    72,    88,    91,    94,   116,    47,   101,    89,
     116,    37,    38,     1,    11,   112,   113,    45,    52,    70,
      87,   116,    45,    37,    38,    48,    57,    48,    50,   101,
      33,   122,    87,    93,    73,   116,    92,   112,    14,   114,
     122,   114,    57,   103,   104,   115,   122,    73,   116,    50,
     122,   103
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

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
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
     `$$ = $1'.

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
#line 108 "../Slice/Grammar.y"
    {
;}
    break;

  case 3:
#line 116 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
;}
    break;

  case 4:
#line 125 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
;}
    break;

  case 5:
#line 129 "../Slice/Grammar.y"
    {
    (yyval) = new StringListTok;
;}
    break;

  case 6:
#line 138 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
;}
    break;

  case 8:
#line 147 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (2)]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
;}
    break;

  case 10:
#line 157 "../Slice/Grammar.y"
    {
    yyerrok;
;}
    break;

  case 12:
#line 162 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 13:
#line 166 "../Slice/Grammar.y"
    {
;}
    break;

  case 14:
#line 174 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ModulePtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 15:
#line 178 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 16:
#line 182 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDefPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 17:
#line 186 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 18:
#line 190 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDefPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 19:
#line 194 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0);
;}
    break;

  case 20:
#line 198 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ExceptionPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 21:
#line 202 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0);
;}
    break;

  case 22:
#line 206 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || StructPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 23:
#line 210 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || SequencePtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 24:
#line 214 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || DictionaryPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 25:
#line 218 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || EnumPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 26:
#line 222 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ConstPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 27:
#line 231 "../Slice/Grammar.y"
    {
    unit->setSeenDefinition();
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ModulePtr module = cont->createModule(ident->v);
    if(module)
    {
	cont->checkIntroduced(ident->v, module);
	unit->pushContainer(module);
	(yyval) = module;
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 28:
#line 248 "../Slice/Grammar.y"
    {
    if((yyvsp[(3) - (6)]))
    {
	unit->popContainer();
	(yyval) = (yyvsp[(3) - (6)]);
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 29:
#line 265 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 30:
#line 269 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 31:
#line 280 "../Slice/Grammar.y"
    {
    unit->error("exceptions cannot be forward declared");
    (yyval) = 0;
;}
    break;

  case 32:
#line 290 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (3)]));
    ExceptionPtr base = ExceptionPtr::dynamicCast((yyvsp[(3) - (3)]));
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base, local->v);
    if(ex)
    {
	cont->checkIntroduced(ident->v, ex);
	unit->pushContainer(ex);
    }
    (yyval) = ex;
;}
    break;

  case 33:
#line 304 "../Slice/Grammar.y"
    {
    if((yyvsp[(4) - (7)]))
    {
	unit->popContainer();
    }
    (yyval) = (yyvsp[(4) - (7)]);
;}
    break;

  case 34:
#line 317 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    (yyval) = contained;
;}
    break;

  case 35:
#line 325 "../Slice/Grammar.y"
    {
    (yyval) = 0;
;}
    break;

  case 36:
#line 334 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
;}
    break;

  case 37:
#line 343 "../Slice/Grammar.y"
    {
;}
    break;

  case 38:
#line 346 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 39:
#line 350 "../Slice/Grammar.y"
    {
;}
    break;

  case 40:
#line 358 "../Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    (yyval) = typestring;
;}
    break;

  case 41:
#line 371 "../Slice/Grammar.y"
    {
    IntegerTokPtr i = IntegerTokPtr::dynamicCast((yyvsp[(2) - (3)]));

    int tag;
    if(i->v < 0 || i->v > Int32Max)
    {
        unit->error("tag for optional is out of range");
        tag = -1;
    }
    else
    {
        tag = static_cast<int>(i->v);
    }

    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = tag >= 0;
    m->v.tag = tag;
    (yyval) = m;
;}
    break;

  case 42:
#line 391 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(2) - (3)]));

    ContainerPtr cont = unit->currentContainer();
    assert(cont);
    ContainedList cl = cont->lookupContained(scoped->v);
    if(cl.empty())
    {
        YYERROR; // Can't continue, jump to next yyerrok
    }
    cont->checkIntroduced(scoped->v);

    int tag = -1;
    EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
    ConstPtr constant = ConstPtr::dynamicCast(cl.front());
    if(constant)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(constant->type());
        if(b)
        {
            switch(b->kind())
            {
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                IceUtil::Int64 l = IceUtilInternal::strToInt64(constant->value().c_str(), 0, 0);
                if(l < 0 || l > Int32Max)
                {
                    unit->error("tag for optional is out of range");
                }
                tag = static_cast<int>(l);
                break;
            }
            default:
                break;
            }
        }
    }
    else if(enumerator)
    {
        //
        // TODO: When this code is merged with ICE-4619, we need to fix the
        // loop below to consider the enumerator's value instead of its ordinal
        // position.
        //
        EnumeratorList el = enumerator->type()->getEnumerators();
        int i = 0;
        for(EnumeratorList::iterator p = el.begin(); p != el.end(); ++p, ++i)
        {
            if(enumerator == *p)
            {
                break;
            }
        }
        tag = i;
    }

    if(tag < 0)
    {
        unit->error("invalid tag `" + scoped->v + "' for optional");
    }

    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = tag >= 0;
    m->v.tag = tag;
    (yyval) = m;
;}
    break;

  case 43:
#line 461 "../Slice/Grammar.y"
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 44:
#line 469 "../Slice/Grammar.y"
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 45:
#line 482 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    (yyval) = m;
;}
    break;

  case 46:
#line 490 "../Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 48:
#line 511 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 49:
#line 515 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 50:
#line 526 "../Slice/Grammar.y"
    {
    unit->error("structs cannot be forward declared");
    (yyval) = 0; // Dummy
;}
    break;

  case 51:
#line 536 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    StructPtr st = cont->createStruct(ident->v, local->v);
    if(st)
    {
	cont->checkIntroduced(ident->v, st);
	unit->pushContainer(st);
    }
    (yyval) = st;
;}
    break;

  case 52:
#line 549 "../Slice/Grammar.y"
    {
    if((yyvsp[(3) - (6)]))
    {
	unit->popContainer();
    }
    (yyval) = (yyvsp[(3) - (6)]);

    //
    // Empty structures are not allowed
    //
    StructPtr st = StructPtr::dynamicCast((yyval));
    assert(st);
    if(st->dataMembers().empty())
    {
    	unit->error("struct `" + st->name() + "' must have at least one member"); // $$ is a dummy
    }
;}
    break;

  case 53:
#line 572 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
;}
    break;

  case 54:
#line 581 "../Slice/Grammar.y"
    {
;}
    break;

  case 55:
#line 584 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 56:
#line 588 "../Slice/Grammar.y"
    {
;}
    break;

  case 58:
#line 602 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 59:
#line 606 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 60:
#line 617 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false, local->v);
    (yyval) = cl;
;}
    break;

  case 61:
#line 630 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (4)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast((yyvsp[(3) - (4)]));
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast((yyvsp[(4) - (4)]));
    if(base)
    {
	bases->v.push_front(base);
    }
    ClassDefPtr cl = cont->createClassDef(ident->v, false, bases->v, local->v);
    if(cl)
    {
	cont->checkIntroduced(ident->v, cl);
	unit->pushContainer(cl);
	(yyval) = cl;
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 62:
#line 653 "../Slice/Grammar.y"
    {
    if((yyvsp[(5) - (8)]))
    {
	unit->popContainer();
	(yyval) = (yyvsp[(5) - (8)]);
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 63:
#line 670 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    (yyval) = 0;
    if(!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if(!cl || cl->isInterface())
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not a class";
	    unit->error(msg);
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if(!def)
	    {
		string msg = "`";
		msg += scoped->v;
		msg += "' has been declared but not defined";
		unit->error(msg);
	    }
	    else
	    {
	    	cont->checkIntroduced(scoped->v);
		(yyval) = def;
	    }
	}
    }
;}
    break;

  case 64:
#line 704 "../Slice/Grammar.y"
    {
    (yyval) = 0;
;}
    break;

  case 65:
#line 713 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 66:
#line 717 "../Slice/Grammar.y"
    {
    (yyval) = new ClassListTok;
;}
    break;

  case 67:
#line 726 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
;}
    break;

  case 68:
#line 735 "../Slice/Grammar.y"
    {
;}
    break;

  case 69:
#line 738 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 70:
#line 742 "../Slice/Grammar.y"
    {
;}
    break;

  case 71:
#line 750 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr def = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    DataMemberPtr dm;
    if(cl)
    {
        dm = cl->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, 0, "", "");
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        dm = st->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, 0, "", "");
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        dm = ex->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, 0, "", "");
    }
    unit->currentContainer()->checkIntroduced(def->v.name, dm);
    (yyval) = dm;
;}
    break;

  case 72:
#line 772 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr def = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(3) - (3)]));

    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    DataMemberPtr dm;
    if(cl)
    {
        dm = cl->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, value->v.value,
                                  value->v.valueAsString, value->v.valueAsLiteral);
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        dm = st->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, value->v.value,
                                  value->v.valueAsString, value->v.valueAsLiteral);
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        dm = ex->createDataMember(def->v.name, def->v.type, def->v.optional, def->v.tag, value->v.value,
                                  value->v.valueAsString, value->v.valueAsLiteral);
    }
    unit->currentContainer()->checkIntroduced(def->v.name, dm);
    (yyval) = dm;
;}
    break;

  case 73:
#line 799 "../Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (2)]));
    string name = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        (yyval) = cl->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        (yyval) = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        (yyval) = ex->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    assert((yyval));
    unit->error("keyword `" + name + "' cannot be used as data member name");
;}
    break;

  case 74:
#line 821 "../Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (1)]));
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        (yyval) = cl->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        (yyval) = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        (yyval) = ex->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    assert((yyval));
    unit->error("missing data member name");
;}
    break;

  case 75:
#line 847 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    m->v.type = TypePtr::dynamicCast((yyvsp[(2) - (2)]));
    (yyval) = m;
;}
    break;

  case 76:
#line 853 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok();
    m->v.type = TypePtr::dynamicCast((yyvsp[(1) - (1)]));
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 77:
#line 861 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 78:
#line 873 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    string name = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag);
	if(op)
	{
	    cl->checkIntroduced(name, op);
	    unit->pushContainer(op);
	    (yyval) = op;
	}
	else
	{
	    (yyval) = 0;
	}
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 79:
#line 897 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast((yyvsp[(2) - (3)]));
    string name = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag,
                                              Operation::Idempotent);
	if(op)
	{
	    cl->checkIntroduced(name, op);
	    unit->pushContainer(op);
	    (yyval) = op;
	}
	else
	{
	    (yyval) = 0;
	}
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 80:
#line 922 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    string name = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag);
	if(op)
	{
	    unit->pushContainer(op);
	    unit->error("keyword `" + name + "' cannot be used as operation name");
	    (yyval) = op; // Dummy
	}
	else
	{
	    (yyval) = 0;
	}
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 81:
#line 946 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast((yyvsp[(2) - (3)]));
    string name = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
	OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag,
                                              Operation::Idempotent);
	if(op)
	{
	    unit->pushContainer(op);
	    unit->error("keyword `" + name + "' cannot be used as operation name");
	    (yyval) = op; // Dummy
	}
	else
	{
	    return 0;
	}
    }
    else
    {
    	(yyval) = 0;
    }
;}
    break;

  case 82:
#line 976 "../Slice/Grammar.y"
    {
    if((yyvsp[(1) - (3)]))
    {
	unit->popContainer();
	(yyval) = (yyvsp[(1) - (3)]);
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 83:
#line 988 "../Slice/Grammar.y"
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[(4) - (5)]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
;}
    break;

  case 84:
#line 998 "../Slice/Grammar.y"
    {
    if((yyvsp[(1) - (3)]))
    {
	unit->popContainer();
    }
    yyerrok;
;}
    break;

  case 85:
#line 1006 "../Slice/Grammar.y"
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[(4) - (5)]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
;}
    break;

  case 88:
#line 1028 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 89:
#line 1032 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 90:
#line 1043 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    cont->checkIntroduced(ident->v, cl);
    (yyval) = cl;
;}
    break;

  case 91:
#line 1057 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (3)]));
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    ClassDefPtr cl = cont->createClassDef(ident->v, true, bases->v, local->v);
    if(cl)
    {
	cont->checkIntroduced(ident->v, cl);
	unit->pushContainer(cl);
	(yyval) = cl;
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 92:
#line 1075 "../Slice/Grammar.y"
    {
    if((yyvsp[(4) - (7)]))
    {
	unit->popContainer();
	(yyval) = (yyvsp[(4) - (7)]);
    }
    else
    {
	(yyval) = 0;
    }
;}
    break;

  case 93:
#line 1092 "../Slice/Grammar.y"
    {
    ClassListTokPtr intfs = ClassListTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if(!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if(!cl || !cl->isInterface())
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not an interface";
	    unit->error(msg);
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if(!def)
	    {
		string msg = "`";
		msg += scoped->v;
		msg += "' has been declared but not defined";
		unit->error(msg);
	    }
	    else
	    {
	    	cont->checkIntroduced(scoped->v);
		intfs->v.push_front(def);
	    }
	}
    }
    (yyval) = intfs;
;}
    break;

  case 94:
#line 1127 "../Slice/Grammar.y"
    {
    ClassListTokPtr intfs = new ClassListTok;
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if(!types.empty())
    {
	ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
	if(!cl || !cl->isInterface())
	{
	    string msg = "`";
	    msg += scoped->v;
	    msg += "' is not an interface";
	    unit->error(msg); // $$ is a dummy
	}
	else
	{
	    ClassDefPtr def = cl->definition();
	    if(!def)
	    {
		string msg = "`";
		msg += scoped->v;
		msg += "' has been declared but not defined";
		unit->error(msg); // $$ is a dummy
	    }
	    else
	    {
	    	cont->checkIntroduced(scoped->v);
		intfs->v.push_front(def);
	    }
	}
    }
    (yyval) = intfs;
;}
    break;

  case 95:
#line 1162 "../Slice/Grammar.y"
    {
    unit->error("illegal inheritance from type Object");
    (yyval) = new ClassListTok; // Dummy
;}
    break;

  case 96:
#line 1172 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 97:
#line 1176 "../Slice/Grammar.y"
    {
    (yyval) = new ClassListTok;
;}
    break;

  case 98:
#line 1185 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
;}
    break;

  case 99:
#line 1194 "../Slice/Grammar.y"
    {
;}
    break;

  case 100:
#line 1197 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 101:
#line 1201 "../Slice/Grammar.y"
    {
;}
    break;

  case 103:
#line 1215 "../Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[(1) - (3)]));
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
;}
    break;

  case 104:
#line 1222 "../Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[(1) - (1)]));
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
;}
    break;

  case 105:
#line 1234 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if(!exception)
    {
	exception = cont->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    (yyval) = exception;
;}
    break;

  case 106:
#line 1246 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
;}
    break;

  case 107:
#line 1257 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (7)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(7) - (7)]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (7)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (7)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v);
;}
    break;

  case 108:
#line 1266 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (7)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(7) - (7)]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (7)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (7)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
;}
    break;

  case 109:
#line 1281 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (10)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(10) - (10)]));
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (10)]));
    TypePtr keyType = TypePtr::dynamicCast((yyvsp[(5) - (10)]));
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast((yyvsp[(7) - (10)]));
    TypePtr valueType = TypePtr::dynamicCast((yyvsp[(8) - (10)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v);
;}
    break;

  case 110:
#line 1292 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (10)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(10) - (10)]));
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (10)]));
    TypePtr keyType = TypePtr::dynamicCast((yyvsp[(5) - (10)]));
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast((yyvsp[(7) - (10)]));
    TypePtr valueType = TypePtr::dynamicCast((yyvsp[(8) - (10)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
;}
    break;

  case 111:
#line 1309 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 112:
#line 1313 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 113:
#line 1324 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    cont->checkIntroduced(ident->v, en);
    (yyval) = en;
;}
    break;

  case 114:
#line 1333 "../Slice/Grammar.y"
    {
    EnumPtr en = EnumPtr::dynamicCast((yyvsp[(3) - (6)]));
    if(en)
    {
	EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast((yyvsp[(5) - (6)]));
	if(enumerators->v.empty())
	{
	    unit->error("enum `" + en->name() + "' must have at least one enumerator");
	}
	en->setEnumerators(enumerators->v); // Dummy
    }
    (yyval) = (yyvsp[(3) - (6)]);
;}
    break;

  case 115:
#line 1348 "../Slice/Grammar.y"
    {
    unit->error("missing enumeration name");
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (5)]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), local->v, Dummy); // Dummy
    EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast((yyvsp[(4) - (5)]));
    en->setEnumerators(enumerators->v); // Dummy
    (yyval) = en;
;}
    break;

  case 116:
#line 1363 "../Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast((yyvsp[(3) - (3)]))->v);
    (yyval) = ens;
;}
    break;

  case 117:
#line 1369 "../Slice/Grammar.y"
    {
;}
    break;

  case 118:
#line 1377 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    ContainerPtr cont = unit->currentContainer();
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if(en)
    {
	ens->v.push_front(en);
    }
    (yyval) = ens;
;}
    break;

  case 119:
#line 1389 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    (yyval) = ens;
;}
    break;

  case 120:
#line 1396 "../Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    (yyval) = ens; // Dummy
;}
    break;

  case 121:
#line 1406 "../Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = true;
    (yyval) = out;
;}
    break;

  case 122:
#line 1412 "../Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = false;
    (yyval) = out;
;}
    break;

  case 123:
#line 1423 "../Slice/Grammar.y"
    {
;}
    break;

  case 124:
#line 1426 "../Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    OptionalDefTokPtr tsp = OptionalDefTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	ParamDeclPtr pd = op->createParamDecl(tsp->v.name, tsp->v.type, isOutParam->v, tsp->v.optional, tsp->v.tag);
	unit->currentContainer()->checkIntroduced(tsp->v.name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (3)]));
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
;}
    break;

  case 125:
#line 1442 "../Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[(3) - (5)]));
    OptionalDefTokPtr tsp = OptionalDefTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	ParamDeclPtr pd = op->createParamDecl(tsp->v.name, tsp->v.type, isOutParam->v, tsp->v.optional, tsp->v.tag);
	unit->currentContainer()->checkIntroduced(tsp->v.name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (5)]));
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
;}
    break;

  case 126:
#line 1458 "../Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(3) - (4)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(4) - (4)]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
	unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
;}
    break;

  case 127:
#line 1470 "../Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[(3) - (6)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (6)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(6) - (6)]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
	unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
;}
    break;

  case 128:
#line 1482 "../Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(3) - (3)]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
	unit->error("missing parameter name");
    }
;}
    break;

  case 129:
#line 1493 "../Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[(3) - (5)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (5)]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
	unit->error("missing parameter name");
    }
;}
    break;

  case 130:
#line 1509 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 131:
#line 1513 "../Slice/Grammar.y"
    {
    (yyval) = new ExceptionListTok;
;}
    break;

  case 132:
#line 1522 "../Slice/Grammar.y"
    {
;}
    break;

  case 133:
#line 1525 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ident->v = "::" + ident->v;
    (yyval) = ident;
;}
    break;

  case 134:
#line 1531 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    scoped->v += "::";
    scoped->v += ident->v;
    (yyval) = scoped;
;}
    break;

  case 135:
#line 1544 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindByte);
;}
    break;

  case 136:
#line 1548 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindBool);
;}
    break;

  case 137:
#line 1552 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindShort);
;}
    break;

  case 138:
#line 1556 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindInt);
;}
    break;

  case 139:
#line 1560 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindLong);
;}
    break;

  case 140:
#line 1564 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindFloat);
;}
    break;

  case 141:
#line 1568 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindDouble);
;}
    break;

  case 142:
#line 1572 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindString);
;}
    break;

  case 143:
#line 1576 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindObject);
;}
    break;

  case 144:
#line 1580 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindObjectProxy);
;}
    break;

  case 145:
#line 1584 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindLocalObject);
;}
    break;

  case 146:
#line 1588 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ContainerPtr cont = unit->currentContainer();
    if(cont)
    {
	TypeList types = cont->lookupType(scoped->v);
	if(types.empty())
	{
	    YYERROR; // Can't continue, jump to next yyerrok
	}
	cont->checkIntroduced(scoped->v);
	(yyval) = types.front();
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 147:
#line 1607 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    if(cont)
    {
	TypeList types = cont->lookupType(scoped->v);
	if(types.empty())
	{
	    YYERROR; // Can't continue, jump to next yyerrok
	}
	for(TypeList::iterator p = types.begin(); p != types.end(); ++p)
	{
	    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(*p);
	    if(!cl)
	    {
		string msg = "`";
		msg += scoped->v;
		msg += "' must be class or interface";
		unit->error(msg);
		YYERROR; // Can't continue, jump to next yyerrok
	    }
	    cont->checkIntroduced(scoped->v);
	    if(cl->isLocal())
	    {
		unit->error("cannot create proxy for " + cl->kindOf() + " `" + cl->name() + "'"); // $$ is dummy
	    }
	    *p = new Proxy(cl);
	}
	(yyval) = types.front();
    }
    else
    {
        (yyval) = 0;
    }
;}
    break;

  case 148:
#line 1648 "../Slice/Grammar.y"
    {
    StringTokPtr str1 = StringTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr str2 = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    str1->v += str2->v;
;}
    break;

  case 149:
#line 1654 "../Slice/Grammar.y"
    {
;}
    break;

  case 150:
#line 1662 "../Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    StringListTokPtr stringList = StringListTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    stringList->v.push_back(str->v);
    (yyval) = stringList;
;}
    break;

  case 151:
#line 1669 "../Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    (yyval) = stringList;
;}
    break;

  case 152:
#line 1681 "../Slice/Grammar.y"
    {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    (yyval) = local;
;}
    break;

  case 153:
#line 1687 "../Slice/Grammar.y"
    {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    (yyval) = local;
;}
    break;

  case 154:
#line 1698 "../Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = sstr.str();
    def->v.valueAsLiteral = intVal->literal;
    (yyval) = def;
;}
    break;

  case 155:
#line 1711 "../Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = sstr.str();
    def->v.valueAsLiteral = floatVal->literal;
    (yyval) = def;
;}
    break;

  case 156:
#line 1724 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ConstDefTokPtr def = new ConstDefTok;
    ContainedList cl = unit->currentContainer()->lookupContained(scoped->v);
    if(cl.empty())
    {
        def->v.type = TypePtr(0);
        def->v.value = SyntaxTreeBasePtr(0);
        def->v.valueAsString = scoped->v;
        def->v.valueAsLiteral = scoped->v;
    }
    else
    {
	EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
        ConstPtr constant = ConstPtr::dynamicCast(cl.front());
	if(enumerator)
        {
            unit->currentContainer()->checkIntroduced(scoped->v, enumerator);
            def->v.type = enumerator->type();
            def->v.value = enumerator;
            def->v.valueAsString = scoped->v;
            def->v.valueAsLiteral = scoped->v;
        }
        else if(constant)
        {
            unit->currentContainer()->checkIntroduced(scoped->v, constant);
            def->v.value = constant;
            def->v.valueAsString = constant->value();
            def->v.valueAsLiteral = constant->value();
        }
        else
	{
	    string msg = "illegal initializer: `" + scoped->v + "' is a";
	    static const string vowels = "aeiou";
	    string kindOf = cl.front()->kindOf();
	    if(vowels.find_first_of(kindOf[0]) != string::npos)
	    {
	    	msg += "n";
	    }
	    msg += " " + kindOf;
	    unit->error(msg); // $$ is dummy
	}
    }
    (yyval) = def;
;}
    break;

  case 157:
#line 1770 "../Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = literal->v;
    def->v.valueAsLiteral = literal->literal;
    (yyval) = def;
;}
    break;

  case 158:
#line 1781 "../Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "false";
    def->v.valueAsLiteral = "false";
    (yyval) = def;
;}
    break;

  case 159:
#line 1792 "../Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "true";
    def->v.valueAsLiteral = "true";
    (yyval) = def;
;}
    break;

  case 160:
#line 1808 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (6)]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[(3) - (6)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(4) - (6)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(6) - (6)]));
    (yyval) = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral);
;}
    break;

  case 161:
#line 1817 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (5)]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[(3) - (5)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    unit->error("missing constant name");
    (yyval) = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral, Dummy); // Dummy
;}
    break;

  case 162:
#line 1831 "../Slice/Grammar.y"
    {
;}
    break;

  case 163:
#line 1834 "../Slice/Grammar.y"
    {
;}
    break;

  case 164:
#line 1837 "../Slice/Grammar.y"
    {
;}
    break;

  case 165:
#line 1840 "../Slice/Grammar.y"
    {
;}
    break;

  case 166:
#line 1843 "../Slice/Grammar.y"
    {
;}
    break;

  case 167:
#line 1846 "../Slice/Grammar.y"
    {
;}
    break;

  case 168:
#line 1849 "../Slice/Grammar.y"
    {
;}
    break;

  case 169:
#line 1852 "../Slice/Grammar.y"
    {
;}
    break;

  case 170:
#line 1855 "../Slice/Grammar.y"
    {
;}
    break;

  case 171:
#line 1858 "../Slice/Grammar.y"
    {
;}
    break;

  case 172:
#line 1861 "../Slice/Grammar.y"
    {
;}
    break;

  case 173:
#line 1864 "../Slice/Grammar.y"
    {
;}
    break;

  case 174:
#line 1867 "../Slice/Grammar.y"
    {
;}
    break;

  case 175:
#line 1870 "../Slice/Grammar.y"
    {
;}
    break;

  case 176:
#line 1873 "../Slice/Grammar.y"
    {
;}
    break;

  case 177:
#line 1876 "../Slice/Grammar.y"
    {
;}
    break;

  case 178:
#line 1879 "../Slice/Grammar.y"
    {
;}
    break;

  case 179:
#line 1882 "../Slice/Grammar.y"
    {
;}
    break;

  case 180:
#line 1885 "../Slice/Grammar.y"
    {
;}
    break;

  case 181:
#line 1888 "../Slice/Grammar.y"
    {
;}
    break;

  case 182:
#line 1891 "../Slice/Grammar.y"
    {
;}
    break;

  case 183:
#line 1894 "../Slice/Grammar.y"
    {
;}
    break;

  case 184:
#line 1897 "../Slice/Grammar.y"
    {
;}
    break;

  case 185:
#line 1900 "../Slice/Grammar.y"
    {
;}
    break;

  case 186:
#line 1903 "../Slice/Grammar.y"
    {
;}
    break;

  case 187:
#line 1906 "../Slice/Grammar.y"
    {
;}
    break;

  case 188:
#line 1909 "../Slice/Grammar.y"
    {
;}
    break;

  case 189:
#line 1912 "../Slice/Grammar.y"
    {
;}
    break;

  case 190:
#line 1915 "../Slice/Grammar.y"
    {
;}
    break;


/* Line 1267 of yacc.c.  */
#line 3924 "Grammar.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
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

  /* Else will try to reuse look-ahead token after shifting the error
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 1919 "../Slice/Grammar.y"


