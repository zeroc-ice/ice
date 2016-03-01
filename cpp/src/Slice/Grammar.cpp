
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
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

//
// Avoid old style cast warnings in generated grammar
//
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wold-style-cast"
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
#line 134 "Grammar.tab.c"

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
#line 220 "Grammar.tab.c"

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
#define YYLAST   886

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  72
/* YYNRULES -- Number of rules.  */
#define YYNRULES  202
/* YYNRULES -- Number of states.  */
#define YYNSTATES  301

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
     166,   171,   176,   178,   181,   182,   191,   194,   195,   198,
     199,   204,   208,   211,   212,   214,   218,   221,   223,   225,
     229,   232,   237,   240,   242,   245,   247,   249,   252,   256,
     259,   263,   264,   270,   271,   277,   279,   281,   284,   287,
     290,   291,   299,   303,   305,   307,   310,   311,   316,   320,
     323,   324,   326,   330,   332,   334,   336,   344,   352,   363,
     374,   377,   380,   381,   388,   394,   398,   400,   402,   406,
     408,   409,   411,   413,   415,   416,   417,   421,   427,   432,
     439,   443,   449,   452,   453,   455,   458,   462,   464,   466,
     468,   470,   472,   474,   476,   478,   480,   483,   485,   487,
     490,   493,   495,   499,   501,   503,   504,   506,   508,   510,
     512,   514,   516,   523,   529,   531,   533,   535,   537,   539,
     541,   543,   545,   547,   549,   551,   553,   555,   557,   559,
     561,   563,   565,   567,   569,   571,   573,   575,   577,   579,
     581,   583,   585
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      55,     0,    -1,    58,    -1,    42,   121,    43,    -1,    40,
     121,    41,    -1,    -1,    -1,    56,    59,    58,    -1,    -1,
      57,    62,    60,    45,    58,    -1,    -1,     1,    45,    61,
      58,    -1,    57,    62,    -1,    -1,    63,    -1,    83,    -1,
      84,    -1,    98,    -1,    99,    -1,    66,    -1,    67,    -1,
      76,    -1,    77,    -1,   107,    -1,   108,    -1,   110,    -1,
     124,    -1,    -1,     3,    33,    64,    46,    58,    47,    -1,
       6,    33,    -1,     6,   125,    -1,   122,    65,    -1,    -1,
     122,    65,    69,    68,    46,    70,    47,    -1,    12,   118,
      -1,    -1,    57,    74,    45,    70,    -1,     1,    45,    70,
      -1,    57,    74,    -1,    -1,   119,    33,    -1,    39,    35,
      48,    -1,    39,   118,    48,    -1,    39,    48,    -1,    31,
      -1,    72,    71,    -1,    71,    -1,    89,    -1,     7,    33,
      -1,     7,   125,    -1,   122,    75,    -1,    -1,   122,    75,
      78,    46,    79,    47,    -1,    57,    80,    45,    79,    -1,
       1,    45,    79,    -1,    57,    80,    -1,    -1,    90,    -1,
       4,    33,    -1,     4,   125,    -1,     4,    37,    35,    48,
      -1,     4,    37,   118,    48,    -1,    81,    -1,   122,    81,
      -1,    -1,   122,    82,    86,    87,    85,    46,    88,    47,
      -1,    12,   118,    -1,    -1,    13,   101,    -1,    -1,    57,
      96,    45,    88,    -1,     1,    45,    88,    -1,    57,    96,
      -1,    -1,    73,    -1,    73,    49,   123,    -1,   119,   125,
      -1,   119,    -1,    71,    -1,    71,    49,   123,    -1,    72,
      71,    -1,    72,    71,    49,   123,    -1,   119,   125,    -1,
     119,    -1,    72,   119,    -1,   119,    -1,    15,    -1,    91,
      37,    -1,    30,    91,    37,    -1,    91,    38,    -1,    30,
      91,    38,    -1,    -1,    92,   116,    48,    94,   117,    -1,
      -1,    92,     1,    48,    95,   117,    -1,    89,    -1,    93,
      -1,     5,    33,    -1,     5,   125,    -1,   122,    97,    -1,
      -1,   122,    97,   102,   100,    46,   103,    47,    -1,   118,
      50,   101,    -1,   118,    -1,    24,    -1,    12,   101,    -1,
      -1,    57,   104,    45,   103,    -1,     1,    45,   103,    -1,
      57,   104,    -1,    -1,    93,    -1,   106,    50,   105,    -1,
     106,    -1,   118,    -1,   125,    -1,   122,     8,    51,    57,
     119,    52,    33,    -1,   122,     8,    51,    57,   119,    52,
     125,    -1,   122,     9,    51,    57,   119,    50,    57,   119,
      52,    33,    -1,   122,     9,    51,    57,   119,    50,    57,
     119,    52,   125,    -1,    10,    33,    -1,    10,   125,    -1,
      -1,   122,   109,   111,    46,   112,    47,    -1,   122,    10,
      46,   112,    47,    -1,   113,    50,   112,    -1,   113,    -1,
      33,    -1,    33,    49,   114,    -1,   125,    -1,    -1,    35,
      -1,   118,    -1,    11,    -1,    -1,    -1,   115,    57,    73,
      -1,   116,    50,   115,    57,    73,    -1,   115,    57,   119,
     125,    -1,   116,    50,   115,    57,   119,   125,    -1,   115,
      57,   119,    -1,   116,    50,   115,    57,   119,    -1,    14,
     105,    -1,    -1,    33,    -1,    32,    33,    -1,   118,    32,
      33,    -1,    16,    -1,    17,    -1,    18,    -1,    19,    -1,
      20,    -1,    21,    -1,    22,    -1,    23,    -1,    24,    -1,
      24,    53,    -1,    25,    -1,   118,    -1,   118,    53,    -1,
      34,   120,    -1,    34,    -1,   121,    50,   120,    -1,   120,
      -1,    26,    -1,    -1,    35,    -1,    36,    -1,   118,    -1,
      34,    -1,    28,    -1,    29,    -1,    27,    57,   119,    33,
      49,   123,    -1,    27,    57,   119,    49,   123,    -1,     3,
      -1,     4,    -1,     5,    -1,     6,    -1,     7,    -1,     8,
      -1,     9,    -1,    10,    -1,    11,    -1,    12,    -1,    13,
      -1,    14,    -1,    15,    -1,    16,    -1,    17,    -1,    18,
      -1,    19,    -1,    20,    -1,    21,    -1,    22,    -1,    23,
      -1,    24,    -1,    25,    -1,    26,    -1,    27,    -1,    28,
      -1,    29,    -1,    30,    -1,    31,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   114,   114,   122,   131,   136,   145,   144,   154,   153,
     164,   163,   168,   173,   180,   184,   188,   192,   196,   200,
     204,   208,   212,   216,   220,   224,   228,   238,   237,   271,
     275,   286,   297,   296,   323,   332,   340,   349,   352,   357,
     364,   377,   397,   453,   461,   474,   482,   497,   503,   507,
     518,   529,   528,   570,   579,   582,   587,   594,   600,   604,
     615,   640,   705,   717,   731,   730,   770,   805,   813,   818,
     826,   835,   838,   843,   850,   872,   899,   921,   947,   956,
     967,   976,   985,   995,  1009,  1015,  1023,  1035,  1059,  1084,
    1108,  1139,  1138,  1161,  1160,  1183,  1184,  1190,  1194,  1205,
    1220,  1219,  1254,  1289,  1324,  1334,  1339,  1347,  1356,  1359,
    1364,  1371,  1377,  1384,  1396,  1408,  1419,  1428,  1443,  1454,
    1471,  1475,  1487,  1486,  1510,  1525,  1531,  1539,  1551,  1574,
    1582,  1591,  1595,  1634,  1641,  1652,  1654,  1670,  1686,  1698,
    1710,  1721,  1737,  1742,  1750,  1753,  1759,  1772,  1776,  1780,
    1784,  1788,  1792,  1796,  1800,  1804,  1808,  1812,  1816,  1835,
    1876,  1882,  1890,  1897,  1909,  1916,  1926,  1939,  1952,  1998,
    2009,  2020,  2036,  2045,  2059,  2062,  2065,  2068,  2071,  2074,
    2077,  2080,  2083,  2086,  2089,  2092,  2095,  2098,  2101,  2104,
    2107,  2110,  2113,  2116,  2119,  2122,  2125,  2128,  2131,  2134,
    2137,  2140,  2143
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
  "struct_exports", "struct_export", "class_name", "class_id",
  "class_decl", "class_def", "@7", "class_extends", "implements",
  "class_exports", "data_member", "struct_data_member", "return_type",
  "operation_preamble", "operation", "@8", "@9", "class_export",
  "interface_id", "interface_decl", "interface_def", "@10",
  "interface_list", "interface_extends", "interface_exports",
  "interface_export", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_id", "enum_def", "@11", "enumerator_list",
  "enumerator", "enumerator_initializer", "out_qualifier", "parameters",
  "throws", "scoped_name", "type", "string_literal", "string_list",
  "local_qualifier", "const_initializer", "const_def", "keyword", 0
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
      82,    82,    82,    83,    85,    84,    86,    86,    87,    87,
      88,    88,    88,    88,    89,    89,    89,    89,    90,    90,
      90,    90,    90,    90,    91,    91,    91,    92,    92,    92,
      92,    94,    93,    95,    93,    96,    96,    97,    97,    98,
     100,    99,   101,   101,   101,   102,   102,   103,   103,   103,
     103,   104,   105,   105,   106,   106,   107,   107,   108,   108,
     109,   109,   111,   110,   110,   112,   112,   113,   113,   113,
     113,   114,   114,   115,   115,   116,   116,   116,   116,   116,
     116,   116,   117,   117,   118,   118,   118,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     120,   120,   121,   121,   122,   122,   123,   123,   123,   123,
     123,   123,   124,   124,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   125,   125,   125,   125,   125,   125,   125,   125,
     125,   125,   125
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
       4,     4,     1,     2,     0,     8,     2,     0,     2,     0,
       4,     3,     2,     0,     1,     3,     2,     1,     1,     3,
       2,     4,     2,     1,     2,     1,     1,     2,     3,     2,
       3,     0,     5,     0,     5,     1,     1,     2,     2,     2,
       0,     7,     3,     1,     1,     2,     0,     4,     3,     2,
       0,     1,     3,     1,     1,     1,     7,     7,    10,    10,
       2,     2,     0,     6,     5,     3,     1,     1,     3,     1,
       0,     1,     1,     1,     0,     0,     3,     5,     4,     6,
       3,     5,     2,     0,     1,     2,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     2,
       2,     1,     3,     1,     1,     0,     1,     1,     1,     1,
       1,     1,     6,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     6,   165,     2,    10,   161,
     163,     0,     0,     1,     0,     0,   164,     5,    12,    14,
      19,    20,    21,    22,    15,    16,    17,    18,    23,    24,
      25,     0,    26,     0,   160,     4,     0,     3,     7,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
      50,    62,    67,    99,   122,    11,   162,     0,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   157,     0,   144,
     158,     0,     0,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,   200,
     201,   202,    58,     0,    59,    97,    98,    29,    30,    48,
      49,     5,     5,   120,   130,   121,     0,    32,     0,     0,
      69,     0,   100,     0,     0,   156,   145,     0,   159,     0,
       0,     9,     0,     0,     0,     0,   127,     0,   126,   129,
      34,     0,     0,    66,     0,    64,   104,   105,   103,     0,
     130,     0,   146,     0,   170,   171,   169,   166,   167,   168,
     173,    60,    61,     0,     0,     0,   124,   130,     0,     0,
       0,     0,    68,     0,     0,     0,     0,    28,   172,     0,
       5,   131,   128,   132,   125,     0,     0,     0,     0,    44,
       0,    78,     0,    55,    57,    83,    52,     0,   102,     0,
       0,     0,   123,   116,   117,     0,     0,    46,     0,    74,
      38,    47,    77,    33,    54,     0,    43,     0,     0,    80,
       0,     0,    40,    82,     0,     0,     0,     0,    86,     0,
       0,     0,     0,   111,   109,    85,   101,     0,    37,    45,
       0,     0,    76,    41,    42,    79,     0,    53,     0,     0,
      95,    96,    72,    77,    65,   108,     0,    84,    87,    89,
       0,   133,     5,     0,     0,     0,    75,    36,    81,    71,
      84,     0,    88,    90,    93,     0,    91,   134,   107,   118,
     119,    70,   143,   136,   140,   143,     5,     0,    94,   138,
      92,     0,   142,   113,   114,   115,   137,   141,     0,   139,
     112
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,     6,     7,    14,    41,    33,    18,    19,
      57,    49,    20,    21,   141,   117,   187,   207,   208,   209,
     210,    50,    22,    23,   118,   171,   193,    51,    52,    24,
      25,   173,   120,   145,   226,   211,   194,   231,   232,   233,
     285,   282,   252,    53,    26,    27,   149,   147,   122,   201,
     234,   292,   293,    28,    29,    54,    30,   123,   137,   138,
     182,   262,   263,   288,    70,   220,    10,    11,    31,   160,
      32,   139
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -227
static const yytype_int16 yypact[] =
{
     126,   -16,     4,     4,    65,  -227,    21,  -227,  -227,     4,
    -227,   -10,   -11,  -227,    73,    38,  -227,    46,    58,  -227,
    -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,   291,  -227,    73,  -227,  -227,     4,  -227,  -227,  -227,
     853,    61,   559,   625,   656,   687,    59,    63,   479,     6,
      77,     9,    99,    11,  -227,  -227,  -227,    82,  -227,  -227,
    -227,  -227,  -227,  -227,  -227,  -227,    84,  -227,   106,  -227,
       5,    26,    73,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,    29,  -227,  -227,  -227,  -227,  -227,  -227,
    -227,    46,    46,  -227,   718,  -227,     1,  -227,    94,     1,
     129,     3,  -227,   100,   434,  -227,  -227,   112,  -227,    98,
     158,  -227,   101,    37,   853,   853,   105,   103,   110,  -227,
     116,   109,   356,   116,     3,  -227,  -227,  -227,    -6,   115,
     718,   117,  -227,   158,  -227,  -227,  -227,  -227,  -227,   116,
    -227,  -227,  -227,   104,   119,    86,  -227,   718,   389,   120,
     829,   123,  -227,   121,     3,   290,   125,  -227,  -227,   749,
      46,  -227,  -227,   116,  -227,   130,   829,   132,   356,  -227,
      69,   113,   853,   131,  -227,   780,  -227,   323,  -227,   136,
     432,   135,  -227,  -227,  -227,   853,   389,  -227,   853,   128,
     138,  -227,   780,  -227,  -227,   140,  -227,    57,   158,   146,
     151,   356,  -227,  -227,   152,   432,   153,   290,  -227,   219,
     853,    54,   254,  -227,   154,  -227,  -227,   149,  -227,  -227,
     158,   389,  -227,  -227,  -227,  -227,   158,  -227,   323,   853,
    -227,  -227,   160,   523,  -227,  -227,    60,  -227,  -227,  -227,
     159,  -227,    46,    74,   290,   811,  -227,  -227,  -227,  -227,
     151,   323,  -227,  -227,  -227,   829,  -227,   195,  -227,  -227,
    -227,  -227,   194,  -227,   780,   194,    46,   594,  -227,  -227,
    -227,   829,  -227,   161,   116,  -227,  -227,   780,   594,  -227,
    -227
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -227,  -227,  -227,   -17,   -12,  -227,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,  -227,  -227,  -227,  -191,  -162,  -159,  -182,
    -227,  -227,  -227,  -227,  -227,  -168,  -227,  -227,  -227,  -227,
    -227,  -227,  -227,  -227,  -226,   -13,  -227,   -20,  -227,    -9,
    -227,  -227,  -227,  -227,  -227,  -227,  -227,  -119,  -227,  -215,
    -227,   -85,  -227,  -227,  -227,  -227,  -227,  -227,   -83,  -227,
    -227,   -63,  -227,   -68,  -102,   -27,     7,   215,  -227,  -150,
    -227,   -38
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -136
static const yytype_int16 yytable[] =
{
      40,   133,    38,   178,   104,   106,   108,   110,   191,   -63,
     115,   192,   255,    71,   140,   238,    34,   143,   116,   148,
     214,    55,   269,   121,    15,   172,   127,   146,   159,     8,
     219,    35,    37,    68,    69,    68,    69,   127,     9,    36,
      36,   230,   148,    56,   174,   281,   239,    16,    17,   278,
     267,   159,   -35,   247,   -63,   198,   -63,  -106,   128,   129,
     131,    68,    69,   183,   132,    13,   249,   176,   245,   127,
     230,    39,   148,   -13,     1,   130,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,   184,   162,     2,   239,   217,   127,
     266,   258,   259,   283,   134,   135,   268,   272,   273,    -5,
      -5,    68,    69,    -8,   215,   244,    72,   163,   164,   296,
     111,   119,   151,     2,   112,     3,   159,   216,    68,    69,
     -13,   181,   276,   -51,   277,   170,   -13,     1,   124,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,   125,   159,   126,
     142,   204,   144,   195,   159,   152,   150,   153,   127,   161,
     166,   186,    -5,    -5,   165,   168,   179,   223,   200,   212,
     167,   175,   218,   205,   177,   188,     2,   197,     3,   180,
     196,   170,   202,   235,   242,   206,   221,   240,   237,   213,
     225,   227,   236,   241,   222,   294,   154,   155,   243,   186,
      68,    69,   156,   157,   158,   246,   294,   248,   253,   264,
     254,   265,   235,   257,   170,   271,   261,   274,   287,   256,
     200,   298,   250,   300,   286,   242,   251,   290,    12,     0,
       0,     0,   270,     0,   186,     0,     0,   280,     0,     0,
       0,   225,     0,     0,   228,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,   275,   289,   200,   284,   295,
     189,    68,    69,     0,   225,   260,     0,     0,   190,   299,
     295,     0,     0,     0,   297,   261,     0,     0,     0,   291,
    -134,  -134,  -134,  -134,  -134,  -134,  -134,  -134,  -134,  -134,
       0,     0,     0,     0,     0,  -134,  -134,  -134,     0,     0,
       0,   199,     0,  -134,  -134,    42,    43,    44,    45,    46,
      47,    48,  -135,     0,  -135,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,     0,     0,     0,     0,
      -5,    -5,    -5,    -5,   224,     0,     0,     0,     0,    -5,
       2,     0,     0,     0,     0,     0,     0,  -110,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,     0,
       0,     0,     0,    -5,    -5,    -5,    -5,   169,     0,     0,
       0,     0,    -5,     2,     0,     0,     0,     0,     0,     0,
     -73,     0,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,     0,     0,     0,     0,     0,    -5,    -5,    -5,
     185,     0,     0,     0,     0,    -5,     2,     0,     0,     0,
       0,     0,     0,   -56,     0,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,     0,     0,     0,     0,     0,
      -5,    -5,    -5,     0,     0,     0,     0,     0,    -5,     2,
       0,     0,     0,     0,     0,     1,   -39,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,     0,     0,   228,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,     0,
      -5,    -5,   229,   189,    68,    69,     0,     0,     0,     0,
       0,   190,     0,     0,     2,     0,     3,     0,     0,     0,
       0,   -13,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,     0,   113,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   114,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,     0,   222,     0,     0,     0,
     -85,   -85,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,     0,   102,     0,     0,     0,   103,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,    68,    69,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,     0,   105,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,     0,   107,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,     0,
     109,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
       0,   136,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,     0,   203,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,     0,   222,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,     0,   279,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,     0,     0,     0,     0,     0,
     189,    68,    69,     0,     0,     0,     0,     0,   190,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,     0,
       0,     0,     0,     0,     0,    68,    69
};

static const yytype_int16 yycheck[] =
{
      17,   103,    14,   153,    42,    43,    44,    45,   170,     0,
      48,   170,   227,    40,   116,   206,     9,   119,    12,   121,
     188,    33,   248,    12,     3,   144,    32,    24,   130,    45,
     192,    41,    43,    32,    33,    32,    33,    32,    34,    50,
      50,   200,   144,    36,    50,   271,   208,    26,    27,   264,
     241,   153,    46,   221,    45,   174,    47,    46,    53,    33,
      72,    32,    33,   165,    35,     0,   225,   150,   218,    32,
     229,    33,   174,     0,     1,    49,     3,     4,     5,     6,
       7,     8,     9,    10,   167,    48,    40,   249,   190,    32,
     240,    37,    38,   275,   111,   112,   246,    37,    38,    26,
      27,    32,    33,    45,    35,    48,    45,   134,   135,   291,
      51,    12,   124,    40,    51,    42,   218,    48,    32,    33,
      47,    35,    48,    46,    50,   142,     0,     1,    46,     3,
       4,     5,     6,     7,     8,     9,    10,    53,   240,    33,
      46,   179,    13,   170,   246,    33,    46,    49,    32,    48,
      47,   168,    26,    27,    49,    46,    52,   195,   175,   186,
      50,    46,    49,   180,    47,    45,    40,    46,    42,    50,
      47,   188,    47,   200,   212,    45,    45,    49,   205,    47,
     197,    45,    47,    45,    33,   287,    28,    29,    48,   206,
      32,    33,    34,    35,    36,    49,   298,    45,   225,    45,
      47,    52,   229,   230,   221,    45,    11,    48,    14,   229,
     227,    50,   225,   298,   277,   253,   225,   285,     3,    -1,
      -1,    -1,   249,    -1,   241,    -1,    -1,   265,    -1,    -1,
      -1,   248,    -1,    -1,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,   262,   284,   264,   275,   287,
      31,    32,    33,    -1,   271,     1,    -1,    -1,    39,   297,
     298,    -1,    -1,    -1,   291,    11,    -1,    -1,    -1,   286,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      -1,    -1,    -1,    -1,    -1,    31,    32,    33,    -1,    -1,
      -1,     1,    -1,    39,    40,     4,     5,     6,     7,     8,
       9,    10,    48,    -1,    50,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,
      30,    31,    32,    33,     1,    -1,    -1,    -1,    -1,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,    47,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      -1,    -1,    -1,    30,    31,    32,    33,     1,    -1,    -1,
      -1,    -1,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,
       1,    -1,    -1,    -1,    -1,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,
      31,    32,    33,    -1,    -1,    -1,    -1,    -1,    39,    40,
      -1,    -1,    -1,    -1,    -1,     1,    47,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    -1,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      26,    27,    30,    31,    32,    33,    -1,    -1,    -1,    -1,
      -1,    39,    -1,    -1,    40,    -1,    42,    -1,    -1,    -1,
      -1,    47,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    33,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    46,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    33,    -1,    -1,    -1,
      37,    38,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    33,    -1,    -1,    -1,    37,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,     3,     4,
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
      30,    31,    -1,    33,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    33,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,
      31,    32,    33,    -1,    -1,    -1,    -1,    -1,    39,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    -1,
      -1,    -1,    -1,    -1,    -1,    32,    33
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    40,    42,    55,    56,    57,    58,    45,    34,
     120,   121,   121,     0,    59,     3,    26,    27,    62,    63,
      66,    67,    76,    77,    83,    84,    98,    99,   107,   108,
     110,   122,   124,    61,   120,    41,    50,    43,    58,    33,
      57,    60,     4,     5,     6,     7,     8,     9,    10,    65,
      75,    81,    82,    97,   109,    58,   120,    64,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    32,    33,
     118,   119,    45,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    33,    37,   125,    33,   125,    33,   125,    33,
     125,    51,    51,    33,    46,   125,    12,    69,    78,    12,
      86,    12,   102,   111,    46,    53,    33,    32,    53,    33,
      49,    58,    35,   118,    57,    57,    33,   112,   113,   125,
     118,    68,    46,   118,    13,    87,    24,   101,   118,   100,
      46,    58,    33,    49,    28,    29,    34,    35,    36,   118,
     123,    48,    48,   119,   119,    49,    47,    50,    46,     1,
      57,    79,   101,    85,    50,    46,   112,    47,   123,    52,
      50,    35,   114,   118,   112,     1,    57,    70,    45,    31,
      39,    71,    72,    80,    90,   119,    47,    46,   101,     1,
      57,   103,    47,    33,   125,    57,    45,    71,    72,    73,
      74,    89,   119,    47,    79,    35,    48,   118,    49,    71,
     119,    45,    33,   125,     1,    57,    88,    45,    15,    30,
      72,    91,    92,    93,   104,   119,    47,   119,    70,    71,
      49,    45,   125,    48,    48,   123,    49,    79,    45,    72,
      89,    93,    96,   119,    47,   103,    91,   119,    37,    38,
       1,    11,   115,   116,    45,    52,   123,    70,   123,    88,
     119,    45,    37,    38,    48,    57,    48,    50,   103,    33,
     125,    88,    95,    73,   119,    94,   115,    14,   117,   125,
     117,    57,   105,   106,   118,   125,    73,   119,    50,   125,
     105
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
#line 115 "../Slice/Grammar.y"
    {
;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 123 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 132 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 136 "../Slice/Grammar.y"
    {
    (yyval) = new StringListTok;
;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 145 "../Slice/Grammar.y"
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
#line 154 "../Slice/Grammar.y"
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
#line 164 "../Slice/Grammar.y"
    {
    yyerrok;
;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 169 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 173 "../Slice/Grammar.y"
    {
;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 181 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ModulePtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 185 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 189 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDefPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 193 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 197 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDefPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 201 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0);
;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 205 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ExceptionPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 209 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0);
;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 213 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || StructPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 217 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || SequencePtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 221 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || DictionaryPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 225 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || EnumPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 229 "../Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ConstPtr::dynamicCast((yyvsp[(1) - (1)])));
;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 238 "../Slice/Grammar.y"
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
#line 255 "../Slice/Grammar.y"
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
#line 272 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 276 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 287 "../Slice/Grammar.y"
    {
    unit->error("exceptions cannot be forward declared");
    (yyval) = 0;
;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 297 "../Slice/Grammar.y"
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
#line 311 "../Slice/Grammar.y"
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
#line 324 "../Slice/Grammar.y"
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
#line 332 "../Slice/Grammar.y"
    {
    (yyval) = 0;
;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 341 "../Slice/Grammar.y"
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
#line 350 "../Slice/Grammar.y"
    {
;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 353 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 357 "../Slice/Grammar.y"
    {
;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 365 "../Slice/Grammar.y"
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
#line 378 "../Slice/Grammar.y"
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
#line 398 "../Slice/Grammar.y"
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
        tag = enumerator->value();
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
#line 454 "../Slice/Grammar.y"
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
#line 462 "../Slice/Grammar.y"
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
#line 475 "../Slice/Grammar.y"
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
#line 483 "../Slice/Grammar.y"
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
#line 504 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 508 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 519 "../Slice/Grammar.y"
    {
    unit->error("structs cannot be forward declared");
    (yyval) = 0; // Dummy
;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 529 "../Slice/Grammar.y"
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
    else
    {
        st = cont->createStruct(IceUtil::generateUUID(), local->v); // Dummy
        assert(st);
        unit->pushContainer(st);
    }
    (yyval) = st;
;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 548 "../Slice/Grammar.y"
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
#line 571 "../Slice/Grammar.y"
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
#line 580 "../Slice/Grammar.y"
    {
;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 583 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 587 "../Slice/Grammar.y"
    {
;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 601 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 605 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 616 "../Slice/Grammar.y"
    {
    IceUtil::Int64 id = IntegerTokPtr::dynamicCast((yyvsp[(3) - (4)]))->v;
    if(id < 0)
    {
        unit->error("invalid compact id for class: id must be a positive integer");
    }
    else if(id > Int32Max)
    {
        unit->error("invalid compact id for class: value is out of range");
    }
    else 
    {
        string typeId = unit->getTypeId(static_cast<int>(id));
        if(!typeId.empty() && !unit->ignRedefs())
        {
            unit->error("invalid compact id for class: already assigned to class `" + typeId + "'");
        }
    }

    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast((yyvsp[(2) - (4)]))->v;
    classId->t = static_cast<int>(id);
    (yyval) = classId;
;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 641 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(3) - (4)]));

    ContainerPtr cont = unit->currentContainer();
    assert(cont);
    ContainedList cl = cont->lookupContained(scoped->v);
    if(cl.empty())
    {
        YYERROR; // Can't continue, jump to next yyerrok
    }
    cont->checkIntroduced(scoped->v);

    int id = -1;
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
                    unit->error("compact id for class is out of range");
                }
                id = static_cast<int>(l);
                break;
            }
            default:
                break;
            }
        }
    }
    else if(enumerator)
    {
        id = enumerator->value();
    }

    if(id < 0)
    {
        unit->error("invalid compact id for class: id must be a positive integer");
    }
    else 
    {
        string typeId = unit->getTypeId(id);
        if(!typeId.empty() && !unit->ignRedefs())
        {
            unit->error("invalid compact id for class: already assigned to class `" + typeId + "'");
        }
    }

    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast((yyvsp[(2) - (4)]))->v;
    classId->t = id;
    (yyval) = classId;

;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 706 "../Slice/Grammar.y"
    {
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]))->v;
    classId->t = -1;
    (yyval) = classId;
;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 718 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false, local->v);
    (yyval) = cl;
;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 731 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ClassIdTokPtr ident = ClassIdTokPtr::dynamicCast((yyvsp[(2) - (4)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast((yyvsp[(3) - (4)]));
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast((yyvsp[(4) - (4)]));
    if(base)
    {
	bases->v.push_front(base);
    }
    ClassDefPtr cl = cont->createClassDef(ident->v, ident->t, false, bases->v, local->v);
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

  case 65:

/* Line 1455 of yacc.c  */
#line 754 "../Slice/Grammar.y"
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

  case 66:

/* Line 1455 of yacc.c  */
#line 771 "../Slice/Grammar.y"
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

  case 67:

/* Line 1455 of yacc.c  */
#line 805 "../Slice/Grammar.y"
    {
    (yyval) = 0;
;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 814 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 818 "../Slice/Grammar.y"
    {
    (yyval) = new ClassListTok;
;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 827 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 836 "../Slice/Grammar.y"
    {
;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 839 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 843 "../Slice/Grammar.y"
    {
;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 851 "../Slice/Grammar.y"
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

  case 75:

/* Line 1455 of yacc.c  */
#line 873 "../Slice/Grammar.y"
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

  case 76:

/* Line 1455 of yacc.c  */
#line 900 "../Slice/Grammar.y"
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

  case 77:

/* Line 1455 of yacc.c  */
#line 922 "../Slice/Grammar.y"
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

  case 78:

/* Line 1455 of yacc.c  */
#line 948 "../Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 957 "../Slice/Grammar.y"
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

  case 80:

/* Line 1455 of yacc.c  */
#line 968 "../Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 977 "../Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(2) - (4)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 986 "../Slice/Grammar.y"
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

  case 83:

/* Line 1455 of yacc.c  */
#line 996 "../Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (1)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("missing data member name");
;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 1010 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    m->v.type = TypePtr::dynamicCast((yyvsp[(2) - (2)]));
    (yyval) = m;
;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 1016 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok();
    m->v.type = TypePtr::dynamicCast((yyvsp[(1) - (1)]));
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 1024 "../Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 1036 "../Slice/Grammar.y"
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

  case 88:

/* Line 1455 of yacc.c  */
#line 1060 "../Slice/Grammar.y"
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

  case 89:

/* Line 1455 of yacc.c  */
#line 1085 "../Slice/Grammar.y"
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

  case 90:

/* Line 1455 of yacc.c  */
#line 1109 "../Slice/Grammar.y"
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

  case 91:

/* Line 1455 of yacc.c  */
#line 1139 "../Slice/Grammar.y"
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

  case 92:

/* Line 1455 of yacc.c  */
#line 1151 "../Slice/Grammar.y"
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

  case 93:

/* Line 1455 of yacc.c  */
#line 1161 "../Slice/Grammar.y"
    {
    if((yyvsp[(1) - (3)]))
    {
	unit->popContainer();
    }
    yyerrok;
;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1169 "../Slice/Grammar.y"
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

  case 97:

/* Line 1455 of yacc.c  */
#line 1191 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 1195 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1206 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    cont->checkIntroduced(ident->v, cl);
    (yyval) = cl;
;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1220 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (3)]));
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    ClassDefPtr cl = cont->createClassDef(ident->v, -1, true, bases->v, local->v);
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

  case 101:

/* Line 1455 of yacc.c  */
#line 1238 "../Slice/Grammar.y"
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

  case 102:

/* Line 1455 of yacc.c  */
#line 1255 "../Slice/Grammar.y"
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

  case 103:

/* Line 1455 of yacc.c  */
#line 1290 "../Slice/Grammar.y"
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

  case 104:

/* Line 1455 of yacc.c  */
#line 1325 "../Slice/Grammar.y"
    {
    unit->error("illegal inheritance from type Object");
    (yyval) = new ClassListTok; // Dummy
;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1335 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1339 "../Slice/Grammar.y"
    {
    (yyval) = new ClassListTok;
;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1348 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1357 "../Slice/Grammar.y"
    {
;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1360 "../Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1364 "../Slice/Grammar.y"
    {
;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1378 "../Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[(1) - (3)]));
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1385 "../Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[(1) - (1)]));
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1397 "../Slice/Grammar.y"
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

  case 115:

/* Line 1455 of yacc.c  */
#line 1409 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1420 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (7)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(7) - (7)]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (7)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (7)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v);
;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1429 "../Slice/Grammar.y"
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

  case 118:

/* Line 1455 of yacc.c  */
#line 1444 "../Slice/Grammar.y"
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

  case 119:

/* Line 1455 of yacc.c  */
#line 1455 "../Slice/Grammar.y"
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

  case 120:

/* Line 1455 of yacc.c  */
#line 1472 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1476 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1487 "../Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    cont->checkIntroduced(ident->v, en);
    (yyval) = en;
;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1496 "../Slice/Grammar.y"
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

  case 124:

/* Line 1455 of yacc.c  */
#line 1511 "../Slice/Grammar.y"
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

  case 125:

/* Line 1455 of yacc.c  */
#line 1526 "../Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast((yyvsp[(3) - (3)]))->v);
    (yyval) = ens;
;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1532 "../Slice/Grammar.y"
    {
;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1540 "../Slice/Grammar.y"
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

  case 128:

/* Line 1455 of yacc.c  */
#line 1552 "../Slice/Grammar.y"
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

  case 129:

/* Line 1455 of yacc.c  */
#line 1575 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    (yyval) = ens;
;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1582 "../Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    (yyval) = ens; // Dummy
;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1592 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(1) - (1)]);
;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1596 "../Slice/Grammar.y"
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

  case 133:

/* Line 1455 of yacc.c  */
#line 1635 "../Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = true;
    (yyval) = out;
;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1641 "../Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = false;
    (yyval) = out;
;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1652 "../Slice/Grammar.y"
    {
;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1655 "../Slice/Grammar.y"
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

  case 137:

/* Line 1455 of yacc.c  */
#line 1671 "../Slice/Grammar.y"
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

  case 138:

/* Line 1455 of yacc.c  */
#line 1687 "../Slice/Grammar.y"
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

  case 139:

/* Line 1455 of yacc.c  */
#line 1699 "../Slice/Grammar.y"
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

  case 140:

/* Line 1455 of yacc.c  */
#line 1711 "../Slice/Grammar.y"
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

  case 141:

/* Line 1455 of yacc.c  */
#line 1722 "../Slice/Grammar.y"
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

  case 142:

/* Line 1455 of yacc.c  */
#line 1738 "../Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1742 "../Slice/Grammar.y"
    {
    (yyval) = new ExceptionListTok;
;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1751 "../Slice/Grammar.y"
    {
;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1754 "../Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ident->v = "::" + ident->v;
    (yyval) = ident;
;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1760 "../Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    scoped->v += "::";
    scoped->v += ident->v;
    (yyval) = scoped;
;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1773 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindByte);
;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1777 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindBool);
;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1781 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindShort);
;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1785 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindInt);
;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1789 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindLong);
;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1793 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindFloat);
;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1797 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindDouble);
;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1801 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindString);
;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1805 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindObject);
;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1809 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindObjectProxy);
;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1813 "../Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindLocalObject);
;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1817 "../Slice/Grammar.y"
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

  case 159:

/* Line 1455 of yacc.c  */
#line 1836 "../Slice/Grammar.y"
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

  case 160:

/* Line 1455 of yacc.c  */
#line 1877 "../Slice/Grammar.y"
    {
    StringTokPtr str1 = StringTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr str2 = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    str1->v += str2->v;
;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1883 "../Slice/Grammar.y"
    {
;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1891 "../Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    StringListTokPtr stringList = StringListTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    stringList->v.push_back(str->v);
    (yyval) = stringList;
;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1898 "../Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    (yyval) = stringList;
;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1910 "../Slice/Grammar.y"
    {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    (yyval) = local;
;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1916 "../Slice/Grammar.y"
    {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    (yyval) = local;
;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1927 "../Slice/Grammar.y"
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

  case 167:

/* Line 1455 of yacc.c  */
#line 1940 "../Slice/Grammar.y"
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

  case 168:

/* Line 1455 of yacc.c  */
#line 1953 "../Slice/Grammar.y"
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

  case 169:

/* Line 1455 of yacc.c  */
#line 1999 "../Slice/Grammar.y"
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

  case 170:

/* Line 1455 of yacc.c  */
#line 2010 "../Slice/Grammar.y"
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

  case 171:

/* Line 1455 of yacc.c  */
#line 2021 "../Slice/Grammar.y"
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

  case 172:

/* Line 1455 of yacc.c  */
#line 2037 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (6)]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[(3) - (6)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(4) - (6)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(6) - (6)]));
    (yyval) = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral);
;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 2046 "../Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (5)]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[(3) - (5)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    unit->error("missing constant name");
    (yyval) = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral, Dummy); // Dummy
;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 2060 "../Slice/Grammar.y"
    {
;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 2063 "../Slice/Grammar.y"
    {
;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 2066 "../Slice/Grammar.y"
    {
;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 2069 "../Slice/Grammar.y"
    {
;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 2072 "../Slice/Grammar.y"
    {
;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 2075 "../Slice/Grammar.y"
    {
;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 2078 "../Slice/Grammar.y"
    {
;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 2081 "../Slice/Grammar.y"
    {
;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 2084 "../Slice/Grammar.y"
    {
;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 2087 "../Slice/Grammar.y"
    {
;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 2090 "../Slice/Grammar.y"
    {
;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 2093 "../Slice/Grammar.y"
    {
;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 2096 "../Slice/Grammar.y"
    {
;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 2099 "../Slice/Grammar.y"
    {
;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 2102 "../Slice/Grammar.y"
    {
;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 2105 "../Slice/Grammar.y"
    {
;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 2108 "../Slice/Grammar.y"
    {
;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 2111 "../Slice/Grammar.y"
    {
;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 2114 "../Slice/Grammar.y"
    {
;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 2117 "../Slice/Grammar.y"
    {
;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 2120 "../Slice/Grammar.y"
    {
;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 2123 "../Slice/Grammar.y"
    {
;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 2126 "../Slice/Grammar.y"
    {
;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 2129 "../Slice/Grammar.y"
    {
;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 2132 "../Slice/Grammar.y"
    {
;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 2135 "../Slice/Grammar.y"
    {
;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 2138 "../Slice/Grammar.y"
    {
;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 2141 "../Slice/Grammar.y"
    {
;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 2144 "../Slice/Grammar.y"
    {
;}
    break;



/* Line 1455 of yacc.c  */
#line 4542 "Grammar.tab.c"
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
#line 2148 "../Slice/Grammar.y"


