
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yylval          slice_lval
#define yychar          slice_char
#define yydebug         slice_debug
#define yynerrs         slice_nerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
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



/* Line 189 of yacc.c  */
#line 127 "Grammar.tab.c"

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



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 213 "Grammar.tab.c"

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
# if YYENABLE_NLS
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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   893

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  71
/* YYNRULES -- Number of rules.  */
#define YYNRULES  199
/* YYNRULES -- Number of states.  */
#define YYNSTATES  295

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
     199,   200,   202,   206,   209,   211,   213,   217,   220,   225,
     228,   230,   233,   235,   237,   240,   244,   247,   251,   252,
     258,   259,   265,   267,   269,   272,   275,   278,   279,   287,
     291,   293,   295,   298,   299,   304,   308,   311,   312,   314,
     318,   320,   322,   324,   332,   340,   351,   362,   365,   368,
     369,   376,   382,   386,   388,   390,   394,   396,   397,   399,
     401,   403,   404,   405,   409,   415,   420,   427,   431,   437,
     440,   441,   443,   446,   450,   452,   454,   456,   458,   460,
     462,   464,   466,   468,   471,   473,   475,   478,   481,   483,
     487,   489,   491,   492,   494,   496,   498,   500,   502,   504,
     511,   517,   519,   521,   523,   525,   527,   529,   531,   533,
     535,   537,   539,   541,   543,   545,   547,   549,   551,   553,
     555,   557,   559,   561,   563,   565,   567,   569,   571,   573
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      55,     0,    -1,    58,    -1,    42,   120,    43,    -1,    40,
     120,    41,    -1,    -1,    -1,    56,    59,    58,    -1,    -1,
      57,    62,    60,    45,    58,    -1,    -1,     1,    45,    61,
      58,    -1,    57,    62,    -1,    -1,    63,    -1,    82,    -1,
      83,    -1,    97,    -1,    98,    -1,    66,    -1,    67,    -1,
      76,    -1,    77,    -1,   106,    -1,   107,    -1,   109,    -1,
     123,    -1,    -1,     3,    33,    64,    46,    58,    47,    -1,
       6,    33,    -1,     6,   124,    -1,   121,    65,    -1,    -1,
     121,    65,    69,    68,    46,    70,    47,    -1,    12,   117,
      -1,    -1,    57,    74,    45,    70,    -1,     1,    45,    70,
      -1,    57,    74,    -1,    -1,   118,    33,    -1,    39,    35,
      48,    -1,    39,   117,    48,    -1,    39,    48,    -1,    31,
      -1,    72,    71,    -1,    71,    -1,    88,    -1,     7,    33,
      -1,     7,   124,    -1,   121,    75,    -1,    -1,   121,    75,
      78,    46,    79,    47,    -1,    57,    80,    45,    79,    -1,
       1,    45,    79,    -1,    57,    80,    -1,    -1,    89,    -1,
       4,    33,    -1,     4,   124,    -1,   121,    81,    -1,    -1,
     121,    81,    85,    86,    84,    46,    87,    47,    -1,    12,
     117,    -1,    -1,    13,   100,    -1,    -1,    57,    95,    45,
      87,    -1,     1,    45,    87,    -1,    57,    95,    -1,    -1,
      73,    -1,    73,    49,   122,    -1,   118,   124,    -1,   118,
      -1,    71,    -1,    71,    49,   122,    -1,    72,    71,    -1,
      72,    71,    49,   122,    -1,   118,   124,    -1,   118,    -1,
      72,   118,    -1,   118,    -1,    15,    -1,    90,    37,    -1,
      30,    90,    37,    -1,    90,    38,    -1,    30,    90,    38,
      -1,    -1,    91,   115,    48,    93,   116,    -1,    -1,    91,
       1,    48,    94,   116,    -1,    88,    -1,    92,    -1,     5,
      33,    -1,     5,   124,    -1,   121,    96,    -1,    -1,   121,
      96,   101,    99,    46,   102,    47,    -1,   117,    50,   100,
      -1,   117,    -1,    24,    -1,    12,   100,    -1,    -1,    57,
     103,    45,   102,    -1,     1,    45,   102,    -1,    57,   103,
      -1,    -1,    92,    -1,   105,    50,   104,    -1,   105,    -1,
     117,    -1,   124,    -1,   121,     8,    51,    57,   118,    52,
      33,    -1,   121,     8,    51,    57,   118,    52,   124,    -1,
     121,     9,    51,    57,   118,    50,    57,   118,    52,    33,
      -1,   121,     9,    51,    57,   118,    50,    57,   118,    52,
     124,    -1,    10,    33,    -1,    10,   124,    -1,    -1,   121,
     108,   110,    46,   111,    47,    -1,   121,    10,    46,   111,
      47,    -1,   112,    50,   111,    -1,   112,    -1,    33,    -1,
      33,    49,   113,    -1,   124,    -1,    -1,    35,    -1,   117,
      -1,    11,    -1,    -1,    -1,   114,    57,    73,    -1,   115,
      50,   114,    57,    73,    -1,   114,    57,   118,   124,    -1,
     115,    50,   114,    57,   118,   124,    -1,   114,    57,   118,
      -1,   115,    50,   114,    57,   118,    -1,    14,   104,    -1,
      -1,    33,    -1,    32,    33,    -1,   117,    32,    33,    -1,
      16,    -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,
      21,    -1,    22,    -1,    23,    -1,    24,    -1,    24,    53,
      -1,    25,    -1,   117,    -1,   117,    53,    -1,    34,   119,
      -1,    34,    -1,   120,    50,   119,    -1,   119,    -1,    26,
      -1,    -1,    35,    -1,    36,    -1,   117,    -1,    34,    -1,
      28,    -1,    29,    -1,    27,    57,   118,    33,    49,   122,
      -1,    27,    57,   118,    49,   122,    -1,     3,    -1,     4,
      -1,     5,    -1,     6,    -1,     7,    -1,     8,    -1,     9,
      -1,    10,    -1,    11,    -1,    12,    -1,    13,    -1,    14,
      -1,    15,    -1,    16,    -1,    17,    -1,    18,    -1,    19,
      -1,    20,    -1,    21,    -1,    22,    -1,    23,    -1,    24,
      -1,    25,    -1,    26,    -1,    27,    -1,    28,    -1,    29,
      -1,    30,    -1,    31,    -1
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
     742,   749,   771,   798,   820,   846,   855,   866,   875,   884,
     894,   908,   914,   922,   934,   958,   983,  1007,  1038,  1037,
    1060,  1059,  1082,  1083,  1089,  1093,  1104,  1119,  1118,  1153,
    1188,  1223,  1233,  1238,  1246,  1255,  1258,  1263,  1270,  1276,
    1283,  1295,  1307,  1318,  1327,  1342,  1353,  1370,  1374,  1386,
    1385,  1409,  1424,  1430,  1438,  1450,  1473,  1481,  1490,  1494,
    1533,  1540,  1551,  1553,  1569,  1585,  1597,  1609,  1620,  1636,
    1641,  1649,  1652,  1658,  1671,  1675,  1679,  1683,  1687,  1691,
    1695,  1699,  1703,  1707,  1711,  1715,  1734,  1775,  1781,  1789,
    1796,  1808,  1815,  1825,  1838,  1851,  1897,  1908,  1919,  1935,
    1944,  1958,  1961,  1964,  1967,  1970,  1973,  1976,  1979,  1982,
    1985,  1988,  1991,  1994,  1997,  2000,  2003,  2006,  2009,  2012,
    2015,  2018,  2021,  2024,  2027,  2030,  2033,  2036,  2039,  2042
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
  "start", "global_meta_data", "meta_data", "definitions", "$@1", "$@2",
  "$@3", "definition", "module_def", "@4", "exception_id",
  "exception_decl", "exception_def", "@5", "exception_extends",
  "exception_exports", "type_id", "optional", "optional_type_id",
  "exception_export", "struct_id", "struct_decl", "struct_def", "@6",
  "struct_exports", "struct_export", "class_id", "class_decl", "class_def",
  "@7", "class_extends", "implements", "class_exports", "data_member",
  "struct_data_member", "return_type", "operation_preamble", "operation",
  "@8", "@9", "class_export", "interface_id", "interface_decl",
  "interface_def", "@10", "interface_list", "interface_extends",
  "interface_exports", "interface_export", "exception_list", "exception",
  "sequence_def", "dictionary_def", "enum_id", "enum_def", "@11",
  "enumerator_list", "enumerator", "enumerator_initializer",
  "out_qualifier", "parameters", "throws", "scoped_name", "type",
  "string_literal", "string_list", "local_qualifier", "const_initializer",
  "const_def", "keyword", 0
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
      87,    88,    88,    88,    88,    89,    89,    89,    89,    89,
      89,    90,    90,    90,    91,    91,    91,    91,    93,    92,
      94,    92,    95,    95,    96,    96,    97,    99,    98,   100,
     100,   100,   101,   101,   102,   102,   102,   102,   103,   104,
     104,   105,   105,   106,   106,   107,   107,   108,   108,   110,
     109,   109,   111,   111,   112,   112,   112,   112,   113,   113,
     114,   114,   115,   115,   115,   115,   115,   115,   115,   116,
     116,   117,   117,   117,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   119,   119,   120,
     120,   121,   121,   122,   122,   122,   122,   122,   122,   123,
     123,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124
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
       0,     1,     3,     2,     1,     1,     3,     2,     4,     2,
       1,     2,     1,     1,     2,     3,     2,     3,     0,     5,
       0,     5,     1,     1,     2,     2,     2,     0,     7,     3,
       1,     1,     2,     0,     4,     3,     2,     0,     1,     3,
       1,     1,     1,     7,     7,    10,    10,     2,     2,     0,
       6,     5,     3,     1,     1,     3,     1,     0,     1,     1,
       1,     0,     0,     3,     5,     4,     6,     3,     5,     2,
       0,     1,     2,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     1,     2,     2,     1,     3,
       1,     1,     0,     1,     1,     1,     1,     1,     1,     6,
       5,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     6,   162,     2,    10,   158,
     160,     0,     0,     1,     0,     0,   161,     5,    12,    14,
      19,    20,    21,    22,    15,    16,    17,    18,    23,    24,
      25,     0,    26,     0,   157,     4,     0,     3,     7,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
      50,    60,    96,   119,    11,   159,     0,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   154,     0,   141,   155,
       0,     0,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,    58,    59,    94,    95,    29,    30,    48,    49,     5,
       5,   117,   127,   118,     0,    32,     0,     0,    66,     0,
      97,     0,     0,   153,   142,     0,   156,     0,     0,     9,
       0,     0,   124,     0,   123,   126,    34,     0,     0,    63,
       0,    61,   101,   102,   100,     0,   127,     0,   143,     0,
     167,   168,   166,   163,   164,   165,   170,     0,     0,     0,
     121,   127,     0,     0,     0,     0,    65,     0,     0,     0,
       0,    28,   169,     0,     5,   128,   125,   129,   122,     0,
       0,     0,     0,    44,     0,    75,     0,    55,    57,    80,
      52,     0,    99,     0,     0,     0,   120,   113,   114,     0,
       0,    46,     0,    71,    38,    47,    74,    33,    54,     0,
      43,     0,     0,    77,     0,     0,    40,    79,     0,     0,
       0,     0,    83,     0,     0,     0,     0,   108,   106,    82,
      98,     0,    37,    45,     0,     0,    73,    41,    42,    76,
       0,    53,     0,     0,    92,    93,    69,    74,    62,   105,
       0,    81,    84,    86,     0,   130,     5,     0,     0,     0,
      72,    36,    78,    68,    81,     0,    85,    87,    90,     0,
      88,   131,   104,   115,   116,    67,   140,   133,   137,   140,
       5,     0,    91,   135,    89,     0,   139,   110,   111,   112,
     134,   138,     0,   136,   109
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,     6,     7,    14,    41,    33,    18,    19,
      56,    49,    20,    21,   137,   115,   181,   201,   202,   203,
     204,    50,    22,    23,   116,   165,   187,    51,    24,    25,
     167,   118,   141,   220,   205,   188,   225,   226,   227,   279,
     276,   246,    52,    26,    27,   145,   143,   120,   195,   228,
     286,   287,    28,    29,    53,    30,   121,   133,   134,   176,
     256,   257,   282,    69,   214,    10,    11,    31,   156,    32,
     135
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -218
static const yytype_int16 yypact[] =
{
     536,   -11,    -3,    -3,    63,  -218,    29,  -218,  -218,    -3,
    -218,    24,   -14,  -218,   299,    38,  -218,    36,    51,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,
    -218,   154,  -218,   299,  -218,  -218,    -3,  -218,  -218,  -218,
     533,    60,   643,   674,   705,   736,    56,    57,   502,    -9,
      34,     6,     4,  -218,  -218,  -218,    76,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,    70,  -218,    73,  -218,     7,
      26,   299,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,    36,
      36,  -218,   767,  -218,    53,  -218,    78,    53,   112,     9,
    -218,    83,   457,  -218,  -218,    97,  -218,    82,    55,  -218,
     533,   533,    84,    89,    90,  -218,   107,    96,   379,   107,
       9,  -218,  -218,  -218,     3,    98,   767,    94,  -218,    55,
    -218,  -218,  -218,  -218,  -218,   107,  -218,    91,    99,    35,
    -218,   767,   412,   101,    95,   100,  -218,   102,     9,   312,
     103,  -218,  -218,   798,    36,  -218,  -218,   107,  -218,   109,
      95,   108,   379,  -218,    -5,   117,   533,   111,  -218,   829,
    -218,   346,  -218,   124,   455,   123,  -218,  -218,  -218,   533,
     412,  -218,   533,   122,   128,  -218,   829,  -218,  -218,   127,
    -218,    30,    55,   129,   143,   379,  -218,  -218,   132,   455,
     133,   312,  -218,   213,   533,    61,   248,  -218,   136,  -218,
    -218,   130,  -218,  -218,    55,   412,  -218,  -218,  -218,  -218,
      55,  -218,   346,   533,  -218,  -218,   139,   576,  -218,  -218,
      64,  -218,  -218,  -218,   137,  -218,    36,    47,   312,   860,
    -218,  -218,  -218,  -218,   143,   346,  -218,  -218,  -218,    95,
    -218,   175,  -218,  -218,  -218,  -218,   173,  -218,   829,   173,
      36,   612,  -218,  -218,  -218,    95,  -218,   138,   107,  -218,
    -218,   829,   612,  -218,  -218
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -218,  -218,  -218,   -17,   -13,  -218,  -218,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -189,  -162,  -150,  -208,
    -218,  -218,  -218,  -218,  -218,  -161,  -218,  -218,  -218,  -218,
    -218,  -218,  -218,  -217,   -30,  -218,   -32,  -218,   -26,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -117,  -218,  -213,  -218,
     -98,  -218,  -218,  -218,  -218,  -218,  -218,   -82,  -218,  -218,
     -76,  -218,   -80,  -102,   -27,    13,   197,  -218,  -140,  -218,
     -38
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -133
static const yytype_int16 yytable[] =
{
      40,    38,   185,   114,   102,   104,   106,   108,   249,   172,
     113,   232,   136,    70,   186,   139,   119,   144,   117,   -64,
      54,   208,    34,   166,   213,   263,   155,    67,    68,    37,
     209,     9,    15,   142,     8,   125,    36,   -35,   144,   125,
     233,    67,    68,   210,   224,   272,   261,   155,   275,    55,
    -103,   192,   -64,   168,   241,    16,    17,   177,   129,   127,
     126,   277,   125,    13,   170,    35,   144,    67,    68,   243,
     175,    39,   239,   224,    36,   128,     2,   290,   238,   178,
     -51,   233,   211,   150,   151,    67,    68,    67,    68,   152,
     153,   154,   130,   131,   260,   270,    -8,   271,   252,   253,
     262,   266,   267,   157,   158,    71,   124,   109,   110,   147,
     155,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,   164,   122,   123,   138,   140,   183,    67,    68,   146,
     148,   149,   155,   159,   184,   198,   160,   189,   155,   125,
     161,   171,   162,   173,   169,   180,   182,   190,   191,   174,
     196,   217,   194,   206,   200,   207,   215,   199,    42,    43,
      44,    45,    46,    47,    48,   164,   212,   229,   236,   221,
     230,   234,   231,   235,   219,   237,   216,   242,   240,   288,
     248,   258,   259,   180,   265,   268,   255,   281,   292,   244,
     288,   250,   247,   245,   294,   280,   229,   251,   164,   284,
      12,     0,     0,     0,   194,     0,     0,     0,     0,   236,
       0,     0,     0,     0,     0,     0,   264,     0,   180,     0,
       0,   274,     0,     0,     0,   219,     0,     0,   222,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,   269,
     283,   194,   278,   289,   183,    67,    68,     0,   219,   254,
       0,     0,   184,   293,   289,     0,     0,     0,   291,   255,
       0,     0,     0,   285,  -131,  -131,  -131,  -131,  -131,  -131,
    -131,  -131,  -131,  -131,     0,     0,     0,     0,     0,  -131,
    -131,  -131,     0,     0,     0,     0,     0,  -131,  -131,     0,
       0,     0,     0,     0,     0,     0,  -132,     0,  -132,   -13,
       1,     0,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
       0,     0,     0,   193,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,     0,     2,
       0,     3,    -5,    -5,    -5,    -5,   -13,   218,     0,     0,
       0,    -5,     2,     0,     0,     0,     0,     0,     0,  -107,
       0,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,     0,     0,     0,     0,    -5,    -5,    -5,    -5,
     163,     0,     0,     0,     0,    -5,     2,     0,     0,     0,
       0,     0,     0,   -70,     0,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,     0,     0,     0,     0,     0,
      -5,    -5,    -5,   179,     0,     0,     0,     0,    -5,     2,
       0,     0,     0,     0,     0,     0,   -56,     0,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,     0,     0,
       0,     0,     0,    -5,    -5,    -5,     0,     0,     0,     0,
       0,    -5,     2,     0,     0,     0,     0,     0,     1,   -39,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,     0,     0,
     222,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,     0,     0,    -5,    -5,   223,   183,    67,    68,     0,
       0,     0,     0,     0,   184,     0,     0,     2,     0,     3,
       0,     0,     0,     0,   -13,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,     0,   111,   -13,     1,     0,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,     0,   112,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,     0,
       0,     0,    -5,    -5,     0,    67,    68,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     2,     0,     3,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,     0,   216,
       0,     0,     0,   -82,   -82,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,    67,    68,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,     0,   101,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,     0,   103,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,     0,   105,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,     0,   107,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,     0,
     132,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
       0,   197,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,     0,   216,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,     0,   273
};

static const yytype_int16 yycheck[] =
{
      17,    14,   164,    12,    42,    43,    44,    45,   221,   149,
      48,   200,   114,    40,   164,   117,    12,   119,    12,    13,
      33,   182,     9,   140,   186,   242,   128,    32,    33,    43,
      35,    34,     3,    24,    45,    32,    50,    46,   140,    32,
     202,    32,    33,    48,   194,   258,   235,   149,   265,    36,
      46,   168,    46,    50,   215,    26,    27,   159,    71,    33,
      53,   269,    32,     0,   146,    41,   168,    32,    33,   219,
      35,    33,   212,   223,    50,    49,    40,   285,    48,   161,
      46,   243,   184,    28,    29,    32,    33,    32,    33,    34,
      35,    36,   109,   110,   234,    48,    45,    50,    37,    38,
     240,    37,    38,   130,   131,    45,    33,    51,    51,   122,
     212,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   138,    46,    53,    46,    13,    31,    32,    33,    46,
      33,    49,   234,    49,    39,   173,    47,   164,   240,    32,
      50,    47,    46,    52,    46,   162,    45,    47,    46,    50,
      47,   189,   169,   180,    45,    47,    45,   174,     4,     5,
       6,     7,     8,     9,    10,   182,    49,   194,   206,    45,
      47,    49,   199,    45,   191,    48,    33,    45,    49,   281,
      47,    45,    52,   200,    45,    48,    11,    14,    50,   219,
     292,   223,   219,   219,   292,   271,   223,   224,   215,   279,
       3,    -1,    -1,    -1,   221,    -1,    -1,    -1,    -1,   247,
      -1,    -1,    -1,    -1,    -1,    -1,   243,    -1,   235,    -1,
      -1,   259,    -1,    -1,    -1,   242,    -1,    -1,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,   256,
     278,   258,   269,   281,    31,    32,    33,    -1,   265,     1,
      -1,    -1,    39,   291,   292,    -1,    -1,    -1,   285,    11,
      -1,    -1,    -1,   280,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      32,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    50,     0,
       1,    -1,     3,     4,     5,     6,     7,     8,     9,    10,
      -1,    -1,    -1,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    27,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    40,
      -1,    42,    30,    31,    32,    33,    47,     1,    -1,    -1,
      -1,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,    -1,    30,    31,    32,    33,
       1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,
      31,    32,    33,     1,    -1,    -1,    -1,    -1,    39,    40,
      -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    -1,    -1,    31,    32,    33,    -1,    -1,    -1,    -1,
      -1,    39,    40,    -1,    -1,    -1,    -1,    -1,     1,    47,
       3,     4,     5,     6,     7,     8,     9,    10,    -1,    -1,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    -1,    -1,    26,    27,    30,    31,    32,    33,    -1,
      -1,    -1,    -1,    -1,    39,    -1,    -1,    40,    -1,    42,
      -1,    -1,    -1,    -1,    47,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    33,     0,     1,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    46,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      -1,    -1,    26,    27,    -1,    32,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    40,    -1,    42,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    33,
      -1,    -1,    -1,    37,    38,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,     3,     4,     5,     6,
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
      -1,    33,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    33,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    33
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    40,    42,    55,    56,    57,    58,    45,    34,
     119,   120,   120,     0,    59,     3,    26,    27,    62,    63,
      66,    67,    76,    77,    82,    83,    97,    98,   106,   107,
     109,   121,   123,    61,   119,    41,    50,    43,    58,    33,
      57,    60,     4,     5,     6,     7,     8,     9,    10,    65,
      75,    81,    96,   108,    58,   119,    64,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    32,    33,   117,
     118,    45,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    33,   124,    33,   124,    33,   124,    33,   124,    51,
      51,    33,    46,   124,    12,    69,    78,    12,    85,    12,
     101,   110,    46,    53,    33,    32,    53,    33,    49,    58,
      57,    57,    33,   111,   112,   124,   117,    68,    46,   117,
      13,    86,    24,   100,   117,    99,    46,    58,    33,    49,
      28,    29,    34,    35,    36,   117,   122,   118,   118,    49,
      47,    50,    46,     1,    57,    79,   100,    84,    50,    46,
     111,    47,   122,    52,    50,    35,   113,   117,   111,     1,
      57,    70,    45,    31,    39,    71,    72,    80,    89,   118,
      47,    46,   100,     1,    57,   102,    47,    33,   124,    57,
      45,    71,    72,    73,    74,    88,   118,    47,    79,    35,
      48,   117,    49,    71,   118,    45,    33,   124,     1,    57,
      87,    45,    15,    30,    72,    90,    91,    92,   103,   118,
      47,   118,    70,    71,    49,    45,   124,    48,    48,   122,
      49,    79,    45,    72,    88,    92,    95,   118,    47,   102,
      90,   118,    37,    38,     1,    11,   114,   115,    45,    52,
     122,    70,   122,    87,   118,    45,    37,    38,    48,    57,
      48,    50,   102,    33,   124,    87,    94,    73,   118,    93,
     114,    14,   116,   124,   116,    57,   104,   105,   117,   124,
      73,   118,    50,   124,   104
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
# if YYLTYPE_IS_TRIVIAL
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
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





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
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
  int yytoken;
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

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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

/* Line 1455 of yacc.c  */
#line 108 "../Slice/Grammar.y"
    {
;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 116 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 125 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 129 "../Slice/Grammar.y"
    {
    (yyval) = new StringListTok;
;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 157 "../Slice/Grammar.y"
    {
    yyerrok;
;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 162 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 166 "../Slice/Grammar.y"
    {
;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 174 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ModulePtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 178 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 182 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDefPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 186 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 190 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDefPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 194 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0);
;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 198 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ExceptionPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 202 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0);
;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 206 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || StructPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 210 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || SequencePtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 214 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || DictionaryPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 218 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || EnumPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 222 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ConstPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 265 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 269 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 280 "../Slice/Grammar.y"
    {
    unit->error("exceptions cannot be forward declared");
    (yyval) = 0;
;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 325 "../Slice/Grammar.y"
    {
    (yyval) = 0;
;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 343 "../Slice/Grammar.y"
    {
;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 346 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 350 "../Slice/Grammar.y"
    {
;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 511 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 515 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 526 "../Slice/Grammar.y"
    {
    unit->error("structs cannot be forward declared");
    (yyval) = 0; // Dummy
;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 581 "../Slice/Grammar.y"
    {
;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 584 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 588 "../Slice/Grammar.y"
    {
;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 602 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 606 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 704 "../Slice/Grammar.y"
    {
    (yyval) = 0;
;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 713 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 717 "../Slice/Grammar.y"
    {
    (yyval) = new ClassListTok;
;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 735 "../Slice/Grammar.y"
    {
;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 738 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 742 "../Slice/Grammar.y"
    {
;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
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

/* Line 1455 of yacc.c  */
#line 847 "../Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 856 "../Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, value->v.value,
                                            value->v.valueAsString, value->v.valueAsLiteral);
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 867 "../Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 876 "../Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(2) - (4)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 885 "../Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (2)]));
    string name = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]))->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("keyword `" + name + "' cannot be used as data member name");
;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 895 "../Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (1)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("missing data member name");
;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 909 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    m->v.type = TypePtr::dynamicCast((yyvsp[(2) - (2)]));
    (yyval) = m;
;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 915 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok();
    m->v.type = TypePtr::dynamicCast((yyvsp[(1) - (1)]));
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 923 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 935 "../Slice/Grammar.y"
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

  case 85:

/* Line 1455 of yacc.c  */
#line 959 "../Slice/Grammar.y"
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

  case 86:

/* Line 1455 of yacc.c  */
#line 984 "../Slice/Grammar.y"
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

  case 87:

/* Line 1455 of yacc.c  */
#line 1008 "../Slice/Grammar.y"
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

  case 88:

/* Line 1455 of yacc.c  */
#line 1038 "../Slice/Grammar.y"
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

  case 89:

/* Line 1455 of yacc.c  */
#line 1050 "../Slice/Grammar.y"
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

  case 90:

/* Line 1455 of yacc.c  */
#line 1060 "../Slice/Grammar.y"
    {
    if((yyvsp[(1) - (3)]))
    {
	unit->popContainer();
    }
    yyerrok;
;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1068 "../Slice/Grammar.y"
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

  case 94:

/* Line 1455 of yacc.c  */
#line 1090 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1094 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1105 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    cont->checkIntroduced(ident->v, cl);
    (yyval) = cl;
;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1119 "../Slice/Grammar.y"
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

  case 98:

/* Line 1455 of yacc.c  */
#line 1137 "../Slice/Grammar.y"
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

  case 99:

/* Line 1455 of yacc.c  */
#line 1154 "../Slice/Grammar.y"
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

  case 100:

/* Line 1455 of yacc.c  */
#line 1189 "../Slice/Grammar.y"
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

  case 101:

/* Line 1455 of yacc.c  */
#line 1224 "../Slice/Grammar.y"
    {
    unit->error("illegal inheritance from type Object");
    (yyval) = new ClassListTok; // Dummy
;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1234 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1238 "../Slice/Grammar.y"
    {
    (yyval) = new ClassListTok;
;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1247 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1256 "../Slice/Grammar.y"
    {
;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1259 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1263 "../Slice/Grammar.y"
    {
;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1277 "../Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[(1) - (3)]));
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1284 "../Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[(1) - (1)]));
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1296 "../Slice/Grammar.y"
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

  case 112:

/* Line 1455 of yacc.c  */
#line 1308 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1319 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (7)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(7) - (7)]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (7)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (7)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v);
;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1328 "../Slice/Grammar.y"
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

  case 115:

/* Line 1455 of yacc.c  */
#line 1343 "../Slice/Grammar.y"
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

  case 116:

/* Line 1455 of yacc.c  */
#line 1354 "../Slice/Grammar.y"
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

  case 117:

/* Line 1455 of yacc.c  */
#line 1371 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1375 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1386 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    cont->checkIntroduced(ident->v, en);
    (yyval) = en;
;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1395 "../Slice/Grammar.y"
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

  case 121:

/* Line 1455 of yacc.c  */
#line 1410 "../Slice/Grammar.y"
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

  case 122:

/* Line 1455 of yacc.c  */
#line 1425 "../Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast((yyvsp[(3) - (3)]))->v);
    (yyval) = ens;
;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1431 "../Slice/Grammar.y"
    {
;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1439 "../Slice/Grammar.y"
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

  case 125:

/* Line 1455 of yacc.c  */
#line 1451 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    ContainerPtr cont = unit->currentContainer();
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    if(intVal)
    {
        if(intVal->v < 0 || intVal->v > Int32Max)
        {
            unit->error("value for enumerator `" + ident->v + "' is out of range");
        }
        else
        {
            EnumeratorPtr en = cont->createEnumerator(ident->v, static_cast<int>(intVal->v));
            if(en)
            {
                ens->v.push_front(en);
            }
        }
    }
    (yyval) = ens;
;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1474 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    (yyval) = ens;
;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1481 "../Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    (yyval) = ens; // Dummy
;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1491 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(1) - (1)]);
;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1495 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ContainedList cl = unit->currentContainer()->lookupContained(scoped->v);
    IntegerTokPtr tok;
    if(!cl.empty())
    {
        ConstPtr constant = ConstPtr::dynamicCast(cl.front());
        if(constant)
        {
            unit->currentContainer()->checkIntroduced(scoped->v, constant);
            BuiltinPtr b = BuiltinPtr::dynamicCast(constant->type());
            if(b && (b->kind() == Builtin::KindByte || b->kind() == Builtin::KindShort ||
                     b->kind() == Builtin::KindInt || b->kind() == Builtin::KindLong))
            {
                IceUtil::Int64 v;
                if(IceUtilInternal::stringToInt64(constant->value(), v))
                {
                    tok = new IntegerTok;
                    tok->v = v;
                    tok->literal = constant->value();
                }
            }
        }
    }

    if(!tok)
    {
        string msg = "illegal initializer: `" + scoped->v + "' is not an integer constant";
        unit->error(msg); // $$ is dummy
    }

    (yyval) = tok;
;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1534 "../Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = true;
    (yyval) = out;
;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1540 "../Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = false;
    (yyval) = out;
;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1551 "../Slice/Grammar.y"
    {
;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1554 "../Slice/Grammar.y"
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

  case 134:

/* Line 1455 of yacc.c  */
#line 1570 "../Slice/Grammar.y"
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

  case 135:

/* Line 1455 of yacc.c  */
#line 1586 "../Slice/Grammar.y"
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

  case 136:

/* Line 1455 of yacc.c  */
#line 1598 "../Slice/Grammar.y"
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

  case 137:

/* Line 1455 of yacc.c  */
#line 1610 "../Slice/Grammar.y"
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

  case 138:

/* Line 1455 of yacc.c  */
#line 1621 "../Slice/Grammar.y"
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

  case 139:

/* Line 1455 of yacc.c  */
#line 1637 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1641 "../Slice/Grammar.y"
    {
    (yyval) = new ExceptionListTok;
;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1650 "../Slice/Grammar.y"
    {
;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1653 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ident->v = "::" + ident->v;
    (yyval) = ident;
;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1659 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    scoped->v += "::";
    scoped->v += ident->v;
    (yyval) = scoped;
;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1672 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindByte);
;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1676 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindBool);
;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1680 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindShort);
;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1684 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindInt);
;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1688 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindLong);
;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1692 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindFloat);
;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1696 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindDouble);
;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1700 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindString);
;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1704 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindObject);
;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1708 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindObjectProxy);
;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1712 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindLocalObject);
;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1716 "../Slice/Grammar.y"
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

  case 156:

/* Line 1455 of yacc.c  */
#line 1735 "../Slice/Grammar.y"
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

  case 157:

/* Line 1455 of yacc.c  */
#line 1776 "../Slice/Grammar.y"
    {
    StringTokPtr str1 = StringTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr str2 = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    str1->v += str2->v;
;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1782 "../Slice/Grammar.y"
    {
;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1790 "../Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    StringListTokPtr stringList = StringListTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    stringList->v.push_back(str->v);
    (yyval) = stringList;
;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1797 "../Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    (yyval) = stringList;
;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1809 "../Slice/Grammar.y"
    {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    (yyval) = local;
;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1815 "../Slice/Grammar.y"
    {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    (yyval) = local;
;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1826 "../Slice/Grammar.y"
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

  case 164:

/* Line 1455 of yacc.c  */
#line 1839 "../Slice/Grammar.y"
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

  case 165:

/* Line 1455 of yacc.c  */
#line 1852 "../Slice/Grammar.y"
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

  case 166:

/* Line 1455 of yacc.c  */
#line 1898 "../Slice/Grammar.y"
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

  case 167:

/* Line 1455 of yacc.c  */
#line 1909 "../Slice/Grammar.y"
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

  case 168:

/* Line 1455 of yacc.c  */
#line 1920 "../Slice/Grammar.y"
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

  case 169:

/* Line 1455 of yacc.c  */
#line 1936 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (6)]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[(3) - (6)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(4) - (6)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(6) - (6)]));
    (yyval) = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral);
;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1945 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (5)]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[(3) - (5)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    unit->error("missing constant name");
    (yyval) = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral, Dummy); // Dummy
;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1959 "../Slice/Grammar.y"
    {
;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1962 "../Slice/Grammar.y"
    {
;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1965 "../Slice/Grammar.y"
    {
;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1968 "../Slice/Grammar.y"
    {
;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1971 "../Slice/Grammar.y"
    {
;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1974 "../Slice/Grammar.y"
    {
;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1977 "../Slice/Grammar.y"
    {
;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1980 "../Slice/Grammar.y"
    {
;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1983 "../Slice/Grammar.y"
    {
;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1986 "../Slice/Grammar.y"
    {
;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1989 "../Slice/Grammar.y"
    {
;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1992 "../Slice/Grammar.y"
    {
;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1995 "../Slice/Grammar.y"
    {
;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1998 "../Slice/Grammar.y"
    {
;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 2001 "../Slice/Grammar.y"
    {
;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 2004 "../Slice/Grammar.y"
    {
;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 2007 "../Slice/Grammar.y"
    {
;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 2010 "../Slice/Grammar.y"
    {
;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 2013 "../Slice/Grammar.y"
    {
;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 2016 "../Slice/Grammar.y"
    {
;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 2019 "../Slice/Grammar.y"
    {
;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 2022 "../Slice/Grammar.y"
    {
;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 2025 "../Slice/Grammar.y"
    {
;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 2028 "../Slice/Grammar.y"
    {
;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 2031 "../Slice/Grammar.y"
    {
;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 2034 "../Slice/Grammar.y"
    {
;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 2037 "../Slice/Grammar.y"
    {
;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 2040 "../Slice/Grammar.y"
    {
;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 2043 "../Slice/Grammar.y"
    {
;}
    break;



/* Line 1455 of yacc.c  */
#line 4425 "Grammar.tab.c"
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
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



/* Line 1675 of yacc.c  */
#line 2047 "../Slice/Grammar.y"


