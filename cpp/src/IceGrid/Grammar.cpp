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



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ICE_GRID_HELP = 258,
     ICE_GRID_EXIT = 259,
     ICE_GRID_APPLICATION = 260,
     ICE_GRID_NODE = 261,
     ICE_GRID_REGISTRY = 262,
     ICE_GRID_SERVER = 263,
     ICE_GRID_ADAPTER = 264,
     ICE_GRID_PING = 265,
     ICE_GRID_LOAD = 266,
     ICE_GRID_SOCKETS = 267,
     ICE_GRID_ADD = 268,
     ICE_GRID_REMOVE = 269,
     ICE_GRID_LIST = 270,
     ICE_GRID_SHUTDOWN = 271,
     ICE_GRID_STRING = 272,
     ICE_GRID_START = 273,
     ICE_GRID_STOP = 274,
     ICE_GRID_SIGNAL = 275,
     ICE_GRID_STDOUT = 276,
     ICE_GRID_STDERR = 277,
     ICE_GRID_DESCRIBE = 278,
     ICE_GRID_PROPERTIES = 279,
     ICE_GRID_PROPERTY = 280,
     ICE_GRID_STATE = 281,
     ICE_GRID_PID = 282,
     ICE_GRID_ENDPOINTS = 283,
     ICE_GRID_ACTIVATION = 284,
     ICE_GRID_OBJECT = 285,
     ICE_GRID_FIND = 286,
     ICE_GRID_SHOW = 287,
     ICE_GRID_COPYING = 288,
     ICE_GRID_WARRANTY = 289,
     ICE_GRID_DIFF = 290,
     ICE_GRID_UPDATE = 291,
     ICE_GRID_INSTANTIATE = 292,
     ICE_GRID_TEMPLATE = 293,
     ICE_GRID_SERVICE = 294,
     ICE_GRID_ENABLE = 295,
     ICE_GRID_DISABLE = 296
   };
#endif
/* Tokens.  */
#define ICE_GRID_HELP 258
#define ICE_GRID_EXIT 259
#define ICE_GRID_APPLICATION 260
#define ICE_GRID_NODE 261
#define ICE_GRID_REGISTRY 262
#define ICE_GRID_SERVER 263
#define ICE_GRID_ADAPTER 264
#define ICE_GRID_PING 265
#define ICE_GRID_LOAD 266
#define ICE_GRID_SOCKETS 267
#define ICE_GRID_ADD 268
#define ICE_GRID_REMOVE 269
#define ICE_GRID_LIST 270
#define ICE_GRID_SHUTDOWN 271
#define ICE_GRID_STRING 272
#define ICE_GRID_START 273
#define ICE_GRID_STOP 274
#define ICE_GRID_SIGNAL 275
#define ICE_GRID_STDOUT 276
#define ICE_GRID_STDERR 277
#define ICE_GRID_DESCRIBE 278
#define ICE_GRID_PROPERTIES 279
#define ICE_GRID_PROPERTY 280
#define ICE_GRID_STATE 281
#define ICE_GRID_PID 282
#define ICE_GRID_ENDPOINTS 283
#define ICE_GRID_ACTIVATION 284
#define ICE_GRID_OBJECT 285
#define ICE_GRID_FIND 286
#define ICE_GRID_SHOW 287
#define ICE_GRID_COPYING 288
#define ICE_GRID_WARRANTY 289
#define ICE_GRID_DIFF 290
#define ICE_GRID_UPDATE 291
#define ICE_GRID_INSTANTIATE 292
#define ICE_GRID_TEMPLATE 293
#define ICE_GRID_SERVICE 294
#define ICE_GRID_ENABLE 295
#define ICE_GRID_DISABLE 296




/* Copy the first part of user declarations.  */
#line 1 "src/IceGrid/Grammar.y"


//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/Parser.h>

#ifdef _MSC_VER
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning(disable:4102)
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning(disable:4065)
// warning C4244: '=': conversion from 'int' to 'yytype_int16', possible loss of data
#   pragma warning(disable:4244)
// warning C4702: unreachable code
#   pragma warning(disable:4702)
#endif

//
// Avoid old style cast warnings in generated grammar
//
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

//
// Avoid clang conversion warnings
//
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wconversion"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

void
yyerror(const char* s)
{
    parser->invalidCommand(s);
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
#line 235 "src/IceGrid/Grammar.cpp"

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
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2468

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  7
/* YYNRULES -- Number of rules.  */
#define YYNRULES  167
/* YYNRULES -- Number of states.  */
#define YYNSTATES  356

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   296

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
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
      35,    36,    37,    38,    39,    40,    41
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,    10,    13,    14,    17,    20,
      25,    30,    35,    40,    45,    50,    55,    60,    65,    70,
      75,    80,    86,    92,    98,   104,   109,   115,   121,   126,
     131,   136,   141,   146,   151,   156,   161,   166,   171,   176,
     181,   186,   191,   196,   201,   206,   211,   216,   221,   226,
     231,   236,   241,   246,   251,   256,   261,   266,   271,   276,
     281,   286,   291,   296,   301,   306,   311,   316,   321,   326,
     331,   336,   341,   346,   351,   356,   361,   366,   371,   376,
     381,   386,   391,   396,   401,   406,   411,   416,   421,   426,
     431,   436,   441,   446,   451,   456,   461,   466,   471,   476,
     481,   486,   491,   496,   501,   506,   511,   516,   521,   526,
     531,   536,   540,   544,   548,   553,   559,   565,   572,   577,
     581,   585,   590,   594,   598,   601,   603,   606,   609,   613,
     617,   618,   620,   622,   624,   626,   628,   630,   632,   634,
     636,   638,   640,   642,   644,   646,   648,   650,   652,   654,
     656,   658,   660,   662,   664,   666,   668,   670,   672,   674,
     676,   678,   680,   682,   684,   686,   688,   690
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      44,     0,    -1,    45,    -1,    -1,    45,    46,    47,    -1,
      46,    47,    -1,    -1,     3,    42,    -1,     4,    42,    -1,
       5,    13,    48,    42,    -1,     5,    13,     3,    42,    -1,
       5,    14,    48,    42,    -1,     5,    14,     3,    42,    -1,
       5,    35,    48,    42,    -1,     5,    35,     3,    42,    -1,
       5,    36,    48,    42,    -1,     5,    36,     3,    42,    -1,
       5,    23,    48,    42,    -1,     5,    23,     3,    42,    -1,
       5,    15,    48,    42,    -1,     5,    15,     3,    42,    -1,
       8,    38,    23,    48,    42,    -1,     8,    38,    23,     3,
      42,    -1,     8,    38,    37,    48,    42,    -1,     8,    38,
      37,     3,    42,    -1,     8,    38,     3,    42,    -1,    39,
      38,    23,    48,    42,    -1,    39,    38,    23,     3,    42,
      -1,    39,    38,     3,    42,    -1,     6,    23,    48,    42,
      -1,     6,    23,     3,    42,    -1,     6,    10,    48,    42,
      -1,     6,    10,     3,    42,    -1,     6,    11,    48,    42,
      -1,     6,    11,     3,    42,    -1,     6,    12,    48,    42,
      -1,     6,    12,     3,    42,    -1,     6,    16,    48,    42,
      -1,     6,    16,     3,    42,    -1,     6,    15,    48,    42,
      -1,     6,    15,     3,    42,    -1,     6,    32,    48,    42,
      -1,     6,    32,     3,    42,    -1,     7,    23,    48,    42,
      -1,     7,    23,     3,    42,    -1,     7,    10,    48,    42,
      -1,     7,    10,     3,    42,    -1,     7,    16,    48,    42,
      -1,     7,    16,     3,    42,    -1,     7,    15,    48,    42,
      -1,     7,    15,     3,    42,    -1,     7,    32,    48,    42,
      -1,     7,    32,     3,    42,    -1,     8,    14,    48,    42,
      -1,     8,    14,     3,    42,    -1,     8,    23,    48,    42,
      -1,     8,    23,     3,    42,    -1,     8,    18,    48,    42,
      -1,     8,    18,     3,    42,    -1,     8,    19,    48,    42,
      -1,     8,    19,     3,    42,    -1,     8,    20,    48,    42,
      -1,     8,    20,     3,    42,    -1,     8,    21,    48,    42,
      -1,     8,    21,     3,    42,    -1,     8,    22,    48,    42,
      -1,     8,    22,     3,    42,    -1,     8,    26,    48,    42,
      -1,     8,    26,     3,    42,    -1,     8,    27,    48,    42,
      -1,     8,    27,     3,    42,    -1,     8,    24,    48,    42,
      -1,     8,    24,     3,    42,    -1,     8,    25,    48,    42,
      -1,     8,    25,     3,    42,    -1,     8,    40,    48,    42,
      -1,     8,    40,     3,    42,    -1,     8,    41,    48,    42,
      -1,     8,    41,     3,    42,    -1,     8,    15,    48,    42,
      -1,     8,    15,     3,    42,    -1,     8,    32,    48,    42,
      -1,     8,    32,     3,    42,    -1,    39,    18,    48,    42,
      -1,    39,    18,     3,    42,    -1,    39,    19,    48,    42,
      -1,    39,    19,     3,    42,    -1,    39,    23,    48,    42,
      -1,    39,    23,     3,    42,    -1,    39,    24,    48,    42,
      -1,    39,    24,     3,    42,    -1,    39,    25,    48,    42,
      -1,    39,    25,     3,    42,    -1,    39,    15,    48,    42,
      -1,    39,    15,     3,    42,    -1,     9,    28,    48,    42,
      -1,     9,    28,     3,    42,    -1,     9,    14,    48,    42,
      -1,     9,    14,     3,    42,    -1,     9,    15,    48,    42,
      -1,     9,    15,     3,    42,    -1,    30,    13,    48,    42,
      -1,    30,    13,     3,    42,    -1,    30,    14,    48,    42,
      -1,    30,    14,     3,    42,    -1,    30,    31,    48,    42,
      -1,    30,    31,     3,    42,    -1,    30,    15,    48,    42,
      -1,    30,    15,     3,    42,    -1,    30,    23,    48,    42,
      -1,    30,    23,     3,    42,    -1,    32,    33,    42,    -1,
      32,    34,    42,    -1,     3,    49,    42,    -1,     3,    49,
      49,    42,    -1,     3,    49,    17,    48,    42,    -1,     3,
      49,    49,    49,    42,    -1,     3,    49,    49,    17,    48,
      42,    -1,     3,    17,    48,    42,    -1,     3,     1,    42,
      -1,    49,     3,    42,    -1,    49,    17,     1,    42,    -1,
      49,     1,    42,    -1,    17,     1,    42,    -1,     1,    42,
      -1,    42,    -1,    17,    48,    -1,    49,    48,    -1,    17,
       3,    48,    -1,    49,     3,    48,    -1,    -1,     4,    -1,
       5,    -1,     6,    -1,     7,    -1,     8,    -1,     9,    -1,
      10,    -1,    11,    -1,    12,    -1,    13,    -1,    14,    -1,
      15,    -1,    16,    -1,    18,    -1,    19,    -1,    20,    -1,
      23,    -1,    26,    -1,    27,    -1,    24,    -1,    25,    -1,
      28,    -1,    29,    -1,    30,    -1,    31,    -1,    32,    -1,
      33,    -1,    34,    -1,    35,    -1,    36,    -1,    37,    -1,
      38,    -1,    39,    -1,    40,    -1,    41,    -1,    22,    -1,
      21,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   100,   100,   104,   111,   114,   121,   129,   133,   137,
     141,   145,   149,   153,   157,   161,   165,   169,   173,   177,
     181,   185,   189,   193,   197,   201,   205,   209,   213,   217,
     221,   225,   229,   233,   237,   241,   245,   249,   253,   257,
     261,   265,   269,   273,   277,   281,   285,   289,   293,   297,
     301,   305,   309,   313,   317,   321,   325,   329,   333,   337,
     341,   345,   349,   353,   357,   361,   365,   369,   373,   377,
     381,   385,   389,   393,   397,   401,   405,   409,   413,   417,
     421,   425,   429,   433,   437,   441,   445,   449,   453,   457,
     461,   465,   469,   473,   477,   481,   485,   489,   493,   497,
     501,   505,   509,   513,   517,   521,   525,   529,   533,   537,
     541,   545,   549,   553,   557,   568,   572,   583,   594,   598,
     602,   606,   612,   617,   622,   626,   634,   639,   644,   650,
     657,   665,   668,   671,   674,   677,   680,   683,   686,   689,
     692,   695,   698,   701,   704,   707,   710,   713,   716,   719,
     722,   725,   728,   731,   734,   737,   740,   743,   746,   749,
     752,   755,   758,   761,   764,   767,   770,   773
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ICE_GRID_HELP", "ICE_GRID_EXIT",
  "ICE_GRID_APPLICATION", "ICE_GRID_NODE", "ICE_GRID_REGISTRY",
  "ICE_GRID_SERVER", "ICE_GRID_ADAPTER", "ICE_GRID_PING", "ICE_GRID_LOAD",
  "ICE_GRID_SOCKETS", "ICE_GRID_ADD", "ICE_GRID_REMOVE", "ICE_GRID_LIST",
  "ICE_GRID_SHUTDOWN", "ICE_GRID_STRING", "ICE_GRID_START",
  "ICE_GRID_STOP", "ICE_GRID_SIGNAL", "ICE_GRID_STDOUT", "ICE_GRID_STDERR",
  "ICE_GRID_DESCRIBE", "ICE_GRID_PROPERTIES", "ICE_GRID_PROPERTY",
  "ICE_GRID_STATE", "ICE_GRID_PID", "ICE_GRID_ENDPOINTS",
  "ICE_GRID_ACTIVATION", "ICE_GRID_OBJECT", "ICE_GRID_FIND",
  "ICE_GRID_SHOW", "ICE_GRID_COPYING", "ICE_GRID_WARRANTY",
  "ICE_GRID_DIFF", "ICE_GRID_UPDATE", "ICE_GRID_INSTANTIATE",
  "ICE_GRID_TEMPLATE", "ICE_GRID_SERVICE", "ICE_GRID_ENABLE",
  "ICE_GRID_DISABLE", "';'", "$accept", "start", "commands",
  "checkInterrupted", "command", "strings", "keyword", 0
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
     295,   296,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    43,    44,    44,    45,    45,    46,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    48,    48,    48,    48,
      48,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     3,     2,     0,     2,     2,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     5,     5,     5,     5,     4,     5,     5,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     3,     3,     3,     4,     5,     5,     6,     4,     3,
       3,     4,     3,     3,     2,     1,     2,     2,     3,     3,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,     6,     0,     1,     0,     0,     0,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,     0,   144,   145,   146,   167,   166,   147,   150,   151,
     148,   149,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   125,     5,     0,     4,
     124,     0,   131,   132,   133,   134,   135,   136,   130,   154,
     156,   163,     7,     0,     8,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,     0,   130,   130,   130,
     130,   130,     0,   130,   130,   130,   130,   130,     0,     0,
     130,   130,   130,   130,   130,   130,     0,     0,     0,     0,
     119,   130,     0,   130,   130,   113,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     130,   130,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   123,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   130,   122,
     120,     0,   130,   126,   118,   130,   127,     0,   130,   114,
       0,    10,     9,    12,    11,    20,    19,    18,    17,    14,
      13,    16,    15,    32,    31,    34,    33,    36,    35,    40,
      39,    38,    37,    30,    29,    42,    41,    46,    45,    50,
      49,    48,    47,    44,    43,    52,    51,    54,    53,    80,
      79,    58,    57,    60,    59,    62,    61,    64,    63,    66,
      65,    56,    55,    72,    71,    74,    73,    68,    67,    70,
      69,    82,    81,    25,     0,     0,     0,     0,    76,    75,
      78,    77,    98,    97,   100,    99,    96,    95,   102,   101,
     104,   103,   108,   107,   110,   109,   106,   105,    94,    93,
      84,    83,    86,    85,    88,    87,    90,    89,    92,    91,
      28,     0,     0,   121,   128,   129,   115,     0,   116,    22,
      21,    24,    23,    27,    26,   117
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,     3,    47,   122,   123
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -66
static const yytype_int16 yypact[] =
{
      31,    52,    54,    59,   -66,    59,    19,   171,    64,  2345,
    2351,   100,   119,   103,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   108,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   135,   -66,    10,   -66,   -66,   -66,
     -66,   -66,   -66,    89,   -66,   -66,   -66,   -66,   102,   -66,
     -66,    78,   -66,   -66,   -66,   -66,   -66,   -66,  2316,   -66,
     -66,   -66,   -66,   210,   -66,   250,   289,   328,   367,   406,
     445,   484,   523,   562,   601,   640,   679,   718,   757,   796,
     835,   874,   913,   952,   991,  1030,  1069,  1108,  1147,  1186,
    1225,  1264,  1303,  1342,  1381,  1420,    99,  1459,  1498,  1537,
    1576,  1615,    79,  1654,  1693,  1732,  1771,  1810,    82,    86,
    1849,  1888,  1927,  1966,  2005,  2044,    34,    87,    88,   134,
     -66,  2083,   105,  2122,  2316,   -66,  2160,   110,   111,   112,
     113,   114,   120,   122,   123,   126,   127,   129,  2322,  2323,
    2327,  2328,  2329,  2330,  2331,  2333,  2334,  2335,  2336,  2337,
    2340,  2342,  2343,  2344,  2346,  2347,  2348,  2349,  2350,  2352,
    2353,  2354,  2355,  2356,  2357,  2358,  2359,  2360,  2361,  2362,
    2363,  2364,  2365,  2366,  2367,  2368,  2369,  2370,  2371,  2372,
    2373,  2374,  2375,  2376,  2377,  2378,  2379,  2380,  2381,  2382,
    2200,  2239,  2383,  2384,  2385,  2386,  2387,  2388,  2389,  2390,
    2391,  2392,   -66,  2393,  2394,  2395,  2396,  2397,  2398,  2399,
    2400,  2401,  2402,   -66,   -66,  2403,  2404,  2405,  2406,  2407,
    2408,  2409,  2410,  2411,  2412,  2413,  2414,  2415,  2278,   -66,
     -66,  2416,  2316,   -66,   -66,  2316,   -66,  2417,  2316,   -66,
    2418,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,  2419,  2420,  2421,  2422,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,  2423,  2424,   -66,   -66,   -66,   -66,  2425,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -66,   -66,   -66,   159,  2463,   -65,    48
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -4
static const yytype_int16 yytable[] =
{
     128,   130,   132,   134,   136,   138,   140,   142,   144,   146,
     148,   150,   152,   154,   156,   158,   160,   162,   164,   166,
     168,   170,   172,   174,   176,   178,   180,   182,   184,   186,
     188,    -3,   193,   195,   197,   199,   201,   227,   204,   206,
     208,   210,   212,   108,   109,   216,   218,   220,   222,   224,
     226,    48,     4,    48,    -2,    63,   233,   228,   236,   237,
       6,    50,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,   189,   117,   110,   118,    64,   111,   112,   102,
      78,   126,   113,   114,   115,    79,    80,    99,   100,   119,
     120,   202,   190,    81,   213,   305,   307,   116,   214,   229,
     230,   101,    82,    83,    84,   231,   191,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   234,   103,   104,
     105,    95,   241,   242,   243,   244,   245,    96,   106,    97,
      98,     5,   246,   342,   247,   248,   107,   344,   249,   250,
     345,   251,    51,   347,   240,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,    58,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,    62,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   125,   127,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   129,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   131,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     133,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   135,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   137,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   139,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   141,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   143,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   145,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,   121,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   147,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   149,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   151,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     153,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   155,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   157,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   159,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   161,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   163,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   165,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,   121,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   167,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   169,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   171,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     173,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   175,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   177,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   179,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   181,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   183,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   185,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,   121,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   187,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   192,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   194,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     196,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   198,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   200,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   203,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   205,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   207,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   209,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,   121,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   211,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   215,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   217,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     219,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   221,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   223,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   225,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   232,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   235,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   238,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   239,   304,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   306,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   341,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,    65,    66,
      67,    71,    72,    73,   252,   253,    74,    75,    68,   254,
     255,   256,   257,   258,    76,   259,   260,   261,   262,   263,
      69,    70,   264,    77,   265,   266,   267,     0,   268,   269,
     270,   271,   272,     0,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   343,   346,
     348,   349,   350,   351,   352,   353,   354,   355,    49
};

static const yytype_int16 yycheck[] =
{
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     0,    97,    98,    99,   100,   101,     3,   103,   104,
     105,   106,   107,    33,    34,   110,   111,   112,   113,   114,
     115,     3,     0,     5,     0,     7,   121,    23,   123,   124,
       1,    42,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,     3,     1,    15,     3,    42,    18,    19,     1,
      10,    63,    23,    24,    25,    15,    16,    14,    15,    17,
      42,    42,    23,    23,    42,   190,   191,    38,    42,    42,
      42,    28,    32,    14,    15,     1,    37,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    42,    13,    14,
      15,    32,    42,    42,    42,    42,    42,    38,    23,    40,
      41,     2,    42,   228,    42,    42,    31,   232,    42,    42,
     235,    42,     1,   238,   126,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    13,    14,
      15,    10,    11,    12,    42,    42,    15,    16,    23,    42,
      42,    42,    42,    42,    23,    42,    42,    42,    42,    42,
      35,    36,    42,    32,    42,    42,    42,    -1,    42,    42,
      42,    42,    42,    -1,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,     5
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    44,    45,    46,     0,    46,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    47,    49,    47,
      42,     1,     4,     5,     6,     7,     8,     9,    17,    30,
      32,    39,    42,    49,    42,    13,    14,    15,    23,    35,
      36,    10,    11,    12,    15,    16,    23,    32,    10,    15,
      16,    23,    32,    14,    15,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    32,    38,    40,    41,    14,
      15,    28,     1,    13,    14,    15,    23,    31,    33,    34,
      15,    18,    19,    23,    24,    25,    38,     1,     3,    17,
      42,    17,    48,    49,    17,    42,    49,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      23,    37,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    48,    42,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,    42,    42,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    23,    42,
      42,     1,     3,    48,    42,     3,    48,    48,    17,    42,
      49,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,     3,    48,     3,    48,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,     3,    48,    42,    48,    48,    42,    48,    42,    42,
      42,    42,    42,    42,    42,    42
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
#line 101 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 3:
#line 104 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 4:
#line 112 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 5:
#line 115 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 6:
#line 121 "src/IceGrid/Grammar.y"
    {
    parser->checkInterrupted();
;}
    break;

  case 7:
#line 130 "src/IceGrid/Grammar.y"
    {
    parser->usage();
;}
    break;

  case 8:
#line 134 "src/IceGrid/Grammar.y"
    {
    return 0;
;}
    break;

  case 9:
#line 138 "src/IceGrid/Grammar.y"
    {
    parser->addApplication((yyvsp[(3) - (4)]));
;}
    break;

  case 10:
#line 142 "src/IceGrid/Grammar.y"
    {
    parser->usage("application", "add");
;}
    break;

  case 11:
#line 146 "src/IceGrid/Grammar.y"
    {
    parser->removeApplication((yyvsp[(3) - (4)]));
;}
    break;

  case 12:
#line 150 "src/IceGrid/Grammar.y"
    {
    parser->usage("application", "remove");
;}
    break;

  case 13:
#line 154 "src/IceGrid/Grammar.y"
    {
    parser->diffApplication((yyvsp[(3) - (4)]));
;}
    break;

  case 14:
#line 158 "src/IceGrid/Grammar.y"
    {
    parser->usage("application", "diff");
;}
    break;

  case 15:
#line 162 "src/IceGrid/Grammar.y"
    {
    parser->updateApplication((yyvsp[(3) - (4)]));
;}
    break;

  case 16:
#line 166 "src/IceGrid/Grammar.y"
    {
    parser->usage("application", "update");
;}
    break;

  case 17:
#line 170 "src/IceGrid/Grammar.y"
    {
    parser->describeApplication((yyvsp[(3) - (4)]));
;}
    break;

  case 18:
#line 174 "src/IceGrid/Grammar.y"
    {
    parser->usage("application", "describe");
;}
    break;

  case 19:
#line 178 "src/IceGrid/Grammar.y"
    {
    parser->listAllApplications((yyvsp[(3) - (4)]));
;}
    break;

  case 20:
#line 182 "src/IceGrid/Grammar.y"
    {
    parser->usage("application", "list");
;}
    break;

  case 21:
#line 186 "src/IceGrid/Grammar.y"
    {
    parser->describeServerTemplate((yyvsp[(4) - (5)]));
;}
    break;

  case 22:
#line 190 "src/IceGrid/Grammar.y"
    {
    parser->usage("server template", "describe");
;}
    break;

  case 23:
#line 194 "src/IceGrid/Grammar.y"
    {
    parser->instantiateServerTemplate((yyvsp[(4) - (5)]));
;}
    break;

  case 24:
#line 198 "src/IceGrid/Grammar.y"
    {
    parser->usage("server template", "instantiate");
;}
    break;

  case 25:
#line 202 "src/IceGrid/Grammar.y"
    {
    parser->usage("server template");
;}
    break;

  case 26:
#line 206 "src/IceGrid/Grammar.y"
    {
    parser->describeServiceTemplate((yyvsp[(4) - (5)]));
;}
    break;

  case 27:
#line 210 "src/IceGrid/Grammar.y"
    {
    parser->usage("service template", "describe");
;}
    break;

  case 28:
#line 214 "src/IceGrid/Grammar.y"
    {
    parser->usage("service template");
;}
    break;

  case 29:
#line 218 "src/IceGrid/Grammar.y"
    {
    parser->describeNode((yyvsp[(3) - (4)]));
;}
    break;

  case 30:
#line 222 "src/IceGrid/Grammar.y"
    {
    parser->usage("node", "describe");
;}
    break;

  case 31:
#line 226 "src/IceGrid/Grammar.y"
    {
    parser->pingNode((yyvsp[(3) - (4)]));
;}
    break;

  case 32:
#line 230 "src/IceGrid/Grammar.y"
    {
    parser->usage("node", "ping");
;}
    break;

  case 33:
#line 234 "src/IceGrid/Grammar.y"
    {
    parser->printLoadNode((yyvsp[(3) - (4)]));
;}
    break;

  case 34:
#line 238 "src/IceGrid/Grammar.y"
    {
    parser->usage("node", "load");
;}
    break;

  case 35:
#line 242 "src/IceGrid/Grammar.y"
    {
    parser->printNodeProcessorSockets((yyvsp[(3) - (4)]));
;}
    break;

  case 36:
#line 246 "src/IceGrid/Grammar.y"
    {
    parser->usage("node", "sockets");
;}
    break;

  case 37:
#line 250 "src/IceGrid/Grammar.y"
    {
    parser->shutdownNode((yyvsp[(3) - (4)]));
;}
    break;

  case 38:
#line 254 "src/IceGrid/Grammar.y"
    {
    parser->usage("node", "shutdown");
;}
    break;

  case 39:
#line 258 "src/IceGrid/Grammar.y"
    {
    parser->listAllNodes((yyvsp[(3) - (4)]));
;}
    break;

  case 40:
#line 262 "src/IceGrid/Grammar.y"
    {
    parser->usage("node", "list");
;}
    break;

  case 41:
#line 266 "src/IceGrid/Grammar.y"
    {
    parser->show("node", (yyvsp[(3) - (4)]));
;}
    break;

  case 42:
#line 270 "src/IceGrid/Grammar.y"
    {
    parser->usage("node", "show");
;}
    break;

  case 43:
#line 274 "src/IceGrid/Grammar.y"
    {
    parser->describeRegistry((yyvsp[(3) - (4)]));
;}
    break;

  case 44:
#line 278 "src/IceGrid/Grammar.y"
    {
    parser->usage("registry", "describe");
;}
    break;

  case 45:
#line 282 "src/IceGrid/Grammar.y"
    {
    parser->pingRegistry((yyvsp[(3) - (4)]));
;}
    break;

  case 46:
#line 286 "src/IceGrid/Grammar.y"
    {
    parser->usage("registry", "ping");
;}
    break;

  case 47:
#line 290 "src/IceGrid/Grammar.y"
    {
    parser->shutdownRegistry((yyvsp[(3) - (4)]));
;}
    break;

  case 48:
#line 294 "src/IceGrid/Grammar.y"
    {
    parser->usage("registry", "shutdown");
;}
    break;

  case 49:
#line 298 "src/IceGrid/Grammar.y"
    {
    parser->listAllRegistries((yyvsp[(3) - (4)]));
;}
    break;

  case 50:
#line 302 "src/IceGrid/Grammar.y"
    {
    parser->usage("registry", "list");
;}
    break;

  case 51:
#line 306 "src/IceGrid/Grammar.y"
    {
    parser->show("registry", (yyvsp[(3) - (4)]));
;}
    break;

  case 52:
#line 310 "src/IceGrid/Grammar.y"
    {
    parser->usage("registry", "show");
;}
    break;

  case 53:
#line 314 "src/IceGrid/Grammar.y"
    {
    parser->removeServer((yyvsp[(3) - (4)]));
;}
    break;

  case 54:
#line 318 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "remove");
;}
    break;

  case 55:
#line 322 "src/IceGrid/Grammar.y"
    {
    parser->describeServer((yyvsp[(3) - (4)]));
;}
    break;

  case 56:
#line 326 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "describe");
;}
    break;

  case 57:
#line 330 "src/IceGrid/Grammar.y"
    {
    parser->startServer((yyvsp[(3) - (4)]));
;}
    break;

  case 58:
#line 334 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "start");
;}
    break;

  case 59:
#line 338 "src/IceGrid/Grammar.y"
    {
    parser->stopServer((yyvsp[(3) - (4)]));
;}
    break;

  case 60:
#line 342 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "stop");
;}
    break;

  case 61:
#line 346 "src/IceGrid/Grammar.y"
    {
    parser->signalServer((yyvsp[(3) - (4)]));
;}
    break;

  case 62:
#line 350 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "signal");
;}
    break;

  case 63:
#line 354 "src/IceGrid/Grammar.y"
    {
    parser->writeMessage((yyvsp[(3) - (4)]), 1);
;}
    break;

  case 64:
#line 358 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "stdout");
;}
    break;

  case 65:
#line 362 "src/IceGrid/Grammar.y"
    {
    parser->writeMessage((yyvsp[(3) - (4)]), 2);
;}
    break;

  case 66:
#line 366 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "stderr");
;}
    break;

  case 67:
#line 370 "src/IceGrid/Grammar.y"
    {
    parser->stateServer((yyvsp[(3) - (4)]));
;}
    break;

  case 68:
#line 374 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "start");
;}
    break;

  case 69:
#line 378 "src/IceGrid/Grammar.y"
    {
    parser->pidServer((yyvsp[(3) - (4)]));
;}
    break;

  case 70:
#line 382 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "pid");
;}
    break;

  case 71:
#line 386 "src/IceGrid/Grammar.y"
    {
    parser->propertiesServer((yyvsp[(3) - (4)]), false);
;}
    break;

  case 72:
#line 390 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "properties");
;}
    break;

  case 73:
#line 394 "src/IceGrid/Grammar.y"
    {
    parser->propertiesServer((yyvsp[(3) - (4)]), true);
;}
    break;

  case 74:
#line 398 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "property");
;}
    break;

  case 75:
#line 402 "src/IceGrid/Grammar.y"
    {
    parser->enableServer((yyvsp[(3) - (4)]), true);
;}
    break;

  case 76:
#line 406 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "enable");
;}
    break;

  case 77:
#line 410 "src/IceGrid/Grammar.y"
    {
    parser->enableServer((yyvsp[(3) - (4)]), false);
;}
    break;

  case 78:
#line 414 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "disable");
;}
    break;

  case 79:
#line 418 "src/IceGrid/Grammar.y"
    {
    parser->listAllServers((yyvsp[(3) - (4)]));
;}
    break;

  case 80:
#line 422 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "list");
;}
    break;

  case 81:
#line 426 "src/IceGrid/Grammar.y"
    {
    parser->show("server", (yyvsp[(3) - (4)]));
;}
    break;

  case 82:
#line 430 "src/IceGrid/Grammar.y"
    {
    parser->usage("server", "show");
;}
    break;

  case 83:
#line 434 "src/IceGrid/Grammar.y"
    {
    parser->startService((yyvsp[(3) - (4)]));
;}
    break;

  case 84:
#line 438 "src/IceGrid/Grammar.y"
    {
    parser->usage("service", "start");
;}
    break;

  case 85:
#line 442 "src/IceGrid/Grammar.y"
    {
    parser->stopService((yyvsp[(3) - (4)]));
;}
    break;

  case 86:
#line 446 "src/IceGrid/Grammar.y"
    {
    parser->usage("service", "stop");
;}
    break;

  case 87:
#line 450 "src/IceGrid/Grammar.y"
    {
    parser->describeService((yyvsp[(3) - (4)]));
;}
    break;

  case 88:
#line 454 "src/IceGrid/Grammar.y"
    {
    parser->usage("service", "describe");
;}
    break;

  case 89:
#line 458 "src/IceGrid/Grammar.y"
    {
    parser->propertiesService((yyvsp[(3) - (4)]), false);
;}
    break;

  case 90:
#line 462 "src/IceGrid/Grammar.y"
    {
    parser->usage("service", "properties");
;}
    break;

  case 91:
#line 466 "src/IceGrid/Grammar.y"
    {
    parser->propertiesService((yyvsp[(3) - (4)]), true);
;}
    break;

  case 92:
#line 470 "src/IceGrid/Grammar.y"
    {
    parser->usage("service", "property");
;}
    break;

  case 93:
#line 474 "src/IceGrid/Grammar.y"
    {
    parser->listServices((yyvsp[(3) - (4)]));
;}
    break;

  case 94:
#line 478 "src/IceGrid/Grammar.y"
    {
    parser->usage("service", "list");
;}
    break;

  case 95:
#line 482 "src/IceGrid/Grammar.y"
    {
    parser->endpointsAdapter((yyvsp[(3) - (4)]));
;}
    break;

  case 96:
#line 486 "src/IceGrid/Grammar.y"
    {
    parser->usage("adapter", "endpoints");
;}
    break;

  case 97:
#line 490 "src/IceGrid/Grammar.y"
    {
    parser->removeAdapter((yyvsp[(3) - (4)]));
;}
    break;

  case 98:
#line 494 "src/IceGrid/Grammar.y"
    {
    parser->usage("adapter", "remove");
;}
    break;

  case 99:
#line 498 "src/IceGrid/Grammar.y"
    {
    parser->listAllAdapters((yyvsp[(3) - (4)]));
;}
    break;

  case 100:
#line 502 "src/IceGrid/Grammar.y"
    {
    parser->usage("adapter", "list");
;}
    break;

  case 101:
#line 506 "src/IceGrid/Grammar.y"
    {
    parser->addObject((yyvsp[(3) - (4)]));
;}
    break;

  case 102:
#line 510 "src/IceGrid/Grammar.y"
    {
    parser->usage("object", "add");
;}
    break;

  case 103:
#line 514 "src/IceGrid/Grammar.y"
    {
    parser->removeObject((yyvsp[(3) - (4)]));
;}
    break;

  case 104:
#line 518 "src/IceGrid/Grammar.y"
    {
    parser->usage("object", "remove");
;}
    break;

  case 105:
#line 522 "src/IceGrid/Grammar.y"
    {
    parser->findObject((yyvsp[(3) - (4)]));
;}
    break;

  case 106:
#line 526 "src/IceGrid/Grammar.y"
    {
    parser->usage("object", "find");
;}
    break;

  case 107:
#line 530 "src/IceGrid/Grammar.y"
    {
    parser->listObject((yyvsp[(3) - (4)]));
;}
    break;

  case 108:
#line 534 "src/IceGrid/Grammar.y"
    {
    parser->usage("object", "list");
;}
    break;

  case 109:
#line 538 "src/IceGrid/Grammar.y"
    {
    parser->describeObject((yyvsp[(3) - (4)]));
;}
    break;

  case 110:
#line 542 "src/IceGrid/Grammar.y"
    {
    parser->usage("object", "describe");
;}
    break;

  case 111:
#line 546 "src/IceGrid/Grammar.y"
    {
    parser->showCopying();
;}
    break;

  case 112:
#line 550 "src/IceGrid/Grammar.y"
    {
    parser->showWarranty();
;}
    break;

  case 113:
#line 554 "src/IceGrid/Grammar.y"
    {
    parser->usage((yyvsp[(2) - (3)]).front());
;}
    break;

  case 114:
#line 558 "src/IceGrid/Grammar.y"
    {
    if(((yyvsp[(2) - (4)]).front() == "server" || (yyvsp[(2) - (4)]).front() == "service") && (yyvsp[(3) - (4)]).front() == "template")
    {
        parser->usage((yyvsp[(2) - (4)]).front() + " " + (yyvsp[(3) - (4)]).front());
    }
    else
    {
        parser->usage((yyvsp[(2) - (4)]).front(), (yyvsp[(3) - (4)]).front());
    }
;}
    break;

  case 115:
#line 569 "src/IceGrid/Grammar.y"
    {
    parser->usage((yyvsp[(2) - (5)]).front(), (yyvsp[(3) - (5)]).front());
;}
    break;

  case 116:
#line 573 "src/IceGrid/Grammar.y"
    {
    if(((yyvsp[(2) - (5)]).front() == "server" || (yyvsp[(2) - (5)]).front() == "service") && (yyvsp[(3) - (5)]).front() == "template")
    {
        parser->usage((yyvsp[(2) - (5)]).front() + " " + (yyvsp[(3) - (5)]).front(), (yyvsp[(4) - (5)]).front());
    }
    else
    {
        parser->usage((yyvsp[(2) - (5)]).front(), (yyvsp[(3) - (5)]).front());
    }
;}
    break;

  case 117:
#line 584 "src/IceGrid/Grammar.y"
    {
    if(((yyvsp[(2) - (6)]).front() == "server" || (yyvsp[(2) - (6)]).front() == "service") && (yyvsp[(3) - (6)]).front() == "template")
    {
        parser->usage((yyvsp[(2) - (6)]).front() + " " + (yyvsp[(3) - (6)]).front(), (yyvsp[(4) - (6)]).front());
    }
    else
    {
        parser->usage((yyvsp[(2) - (6)]).front(), (yyvsp[(3) - (6)]).front());
    }
;}
    break;

  case 118:
#line 595 "src/IceGrid/Grammar.y"
    {
    parser->usage((yyvsp[(2) - (4)]).front());
;}
    break;

  case 119:
#line 599 "src/IceGrid/Grammar.y"
    {
    parser->usage();
;}
    break;

  case 120:
#line 603 "src/IceGrid/Grammar.y"
    {
    parser->usage((yyvsp[(1) - (3)]).front());
;}
    break;

  case 121:
#line 607 "src/IceGrid/Grammar.y"
    {
    (yyvsp[(1) - (4)]).push_back((yyvsp[(2) - (4)]).front());
    parser->invalidCommand((yyvsp[(1) - (4)]));
    yyerrok;
;}
    break;

  case 122:
#line 613 "src/IceGrid/Grammar.y"
    {
    parser->invalidCommand((yyvsp[(1) - (3)]));
    yyerrok;
;}
    break;

  case 123:
#line 618 "src/IceGrid/Grammar.y"
    {
    parser->invalidCommand((yyvsp[(1) - (3)]));
    yyerrok;
;}
    break;

  case 124:
#line 623 "src/IceGrid/Grammar.y"
    {
    yyerrok;
;}
    break;

  case 125:
#line 627 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 126:
#line 635 "src/IceGrid/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
    (yyval).push_front((yyvsp[(1) - (2)]).front());
;}
    break;

  case 127:
#line 640 "src/IceGrid/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
    (yyval).push_front((yyvsp[(1) - (2)]).front());
;}
    break;

  case 128:
#line 645 "src/IceGrid/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
    (yyval).push_front("help");
    (yyval).push_front((yyvsp[(1) - (3)]).front());
;}
    break;

  case 129:
#line 651 "src/IceGrid/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
    (yyval).push_front("help");
    (yyval).push_front((yyvsp[(1) - (3)]).front());
;}
    break;

  case 130:
#line 657 "src/IceGrid/Grammar.y"
    {
    (yyval) = YYSTYPE();
;}
    break;

  case 131:
#line 666 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 132:
#line 669 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 133:
#line 672 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 134:
#line 675 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 135:
#line 678 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 136:
#line 681 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 137:
#line 684 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 138:
#line 687 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 139:
#line 690 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 140:
#line 693 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 141:
#line 696 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 142:
#line 699 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 143:
#line 702 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 144:
#line 705 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 145:
#line 708 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 146:
#line 711 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 147:
#line 714 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 148:
#line 717 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 149:
#line 720 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 150:
#line 723 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 151:
#line 726 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 152:
#line 729 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 153:
#line 732 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 154:
#line 735 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 155:
#line 738 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 156:
#line 741 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 157:
#line 744 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 158:
#line 747 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 159:
#line 750 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 160:
#line 753 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 161:
#line 756 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 162:
#line 759 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 163:
#line 762 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 164:
#line 765 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 165:
#line 768 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 166:
#line 771 "src/IceGrid/Grammar.y"
    {
;}
    break;

  case 167:
#line 774 "src/IceGrid/Grammar.y"
    {
;}
    break;


/* Line 1267 of yacc.c.  */
#line 3315 "src/IceGrid/Grammar.cpp"
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



