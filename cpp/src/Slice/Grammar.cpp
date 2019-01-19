/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yylval          slice_lval
#define yychar          slice_char
#define yydebug         slice_debug
#define yynerrs         slice_nerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 1 "src/Slice/Grammar.y"


//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Slice/GrammarUtil.h>
#include <IceUtil/InputUtil.h>
#include <IceUtil/UUID.h>
#include <cstring>

#ifdef _MSC_VER
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning(disable:4102)
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning(disable:4065)
// warning C4244: '=': conversion from 'int' to 'yytype_int16', possible loss of data
#   pragma warning(disable:4244)
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


/* Line 371 of yacc.c  */
#line 128 "src/Slice/Grammar.cpp"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
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
   by #include "Grammar.hpp".  */
#ifndef YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED
# define YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int slice_debug;
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
     ICE_VALUE = 287,
     ICE_IDENTIFIER = 288,
     ICE_SCOPED_IDENTIFIER = 289,
     ICE_STRING_LITERAL = 290,
     ICE_INTEGER_LITERAL = 291,
     ICE_FLOATING_POINT_LITERAL = 292,
     ICE_IDENT_OP = 293,
     ICE_KEYWORD_OP = 294,
     ICE_OPTIONAL_OP = 295,
     ICE_METADATA_OPEN = 296,
     ICE_METADATA_CLOSE = 297,
     ICE_GLOBAL_METADATA_OPEN = 298,
     ICE_GLOBAL_METADATA_CLOSE = 299,
     BAD_CHAR = 300
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int slice_parse (void *YYPARSE_PARAM);
#else
int slice_parse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int slice_parse (void);
#else
int slice_parse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 238 "src/Slice/Grammar.cpp"

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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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

# define YYCOPY_NEEDED 1

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
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   869

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  55
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  86
/* YYNRULES -- Number of rules.  */
#define YYNRULES  225
/* YYNRULES -- Number of states.  */
#define YYNSTATES  326

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   300

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    49,    54,     2,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    46,
      52,    50,    53,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    47,     2,    48,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     8,    12,    16,    17,    18,
      22,    23,    28,    29,    30,    34,    35,    39,    41,    42,
      46,    47,    51,    53,    54,    58,    59,    63,    65,    66,
      70,    71,    75,    77,    78,    82,    83,    87,    89,    90,
      94,    96,    97,   101,   102,   106,   108,   111,   112,   119,
     122,   125,   128,   129,   137,   140,   141,   146,   150,   153,
     154,   157,   161,   165,   168,   170,   173,   175,   177,   180,
     183,   186,   187,   194,   199,   203,   206,   207,   209,   212,
     215,   220,   225,   227,   230,   231,   240,   243,   244,   247,
     248,   253,   257,   260,   261,   263,   267,   270,   272,   274,
     278,   281,   286,   289,   291,   294,   296,   298,   301,   305,
     308,   312,   313,   319,   320,   326,   328,   330,   333,   336,
     339,   340,   348,   352,   354,   356,   358,   361,   362,   367,
     371,   374,   375,   377,   381,   383,   385,   387,   395,   403,
     414,   425,   428,   431,   432,   439,   440,   447,   451,   453,
     455,   459,   461,   462,   464,   466,   468,   469,   470,   474,
     480,   485,   492,   496,   502,   505,   506,   508,   510,   512,
     514,   516,   518,   520,   522,   524,   526,   528,   531,   533,
     535,   537,   540,   543,   545,   549,   551,   553,   554,   556,
     558,   560,   562,   564,   566,   573,   579,   581,   583,   585,
     587,   589,   591,   593,   595,   597,   599,   601,   603,   605,
     607,   609,   611,   613,   615,   617,   619,   621,   623,   625,
     627,   629,   631,   633,   635,   637
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      56,     0,    -1,    60,    -1,    46,    -1,    -1,    43,   136,
      44,    -1,    41,   136,    42,    -1,    -1,    -1,    58,    61,
      60,    -1,    -1,    59,    63,    62,    60,    -1,    -1,    -1,
      77,    64,    57,    -1,    -1,    97,    65,    46,    -1,    97,
      -1,    -1,    98,    66,    57,    -1,    -1,   112,    67,    46,
      -1,   112,    -1,    -1,   113,    68,    57,    -1,    -1,    80,
      69,    46,    -1,    80,    -1,    -1,    81,    70,    57,    -1,
      -1,    90,    71,    46,    -1,    90,    -1,    -1,    91,    72,
      57,    -1,    -1,   121,    73,    46,    -1,   121,    -1,    -1,
     122,    74,    46,    -1,   122,    -1,    -1,   124,    75,    57,
      -1,    -1,   139,    76,    46,    -1,   139,    -1,     1,    46,
      -1,    -1,     3,    33,    78,    47,    60,    48,    -1,     6,
      33,    -1,     6,   140,    -1,   137,    79,    -1,    -1,   137,
      79,    83,    82,    47,    84,    48,    -1,    12,   133,    -1,
      -1,    59,    88,    46,    84,    -1,     1,    46,    84,    -1,
      59,    88,    -1,    -1,   134,    33,    -1,    40,    36,    49,
      -1,    40,   133,    49,    -1,    40,    49,    -1,    31,    -1,
      86,    85,    -1,    85,    -1,   103,    -1,     7,    33,    -1,
       7,   140,    -1,   137,    89,    -1,    -1,   137,    89,    92,
      47,    93,    48,    -1,    59,    94,    46,    93,    -1,     1,
      46,    93,    -1,    59,    94,    -1,    -1,   104,    -1,     4,
      33,    -1,     4,   140,    -1,     4,    38,    36,    49,    -1,
       4,    38,   133,    49,    -1,    95,    -1,   137,    95,    -1,
      -1,   137,    96,   100,   101,    99,    47,   102,    48,    -1,
      12,   133,    -1,    -1,    13,   115,    -1,    -1,    59,   110,
      46,   102,    -1,     1,    46,   102,    -1,    59,   110,    -1,
      -1,    87,    -1,    87,    50,   138,    -1,   134,   140,    -1,
     134,    -1,    85,    -1,    85,    50,   138,    -1,    86,    85,
      -1,    86,    85,    50,   138,    -1,   134,   140,    -1,   134,
      -1,    86,   134,    -1,   134,    -1,    15,    -1,   105,    38,
      -1,    30,   105,    38,    -1,   105,    39,    -1,    30,   105,
      39,    -1,    -1,   106,   131,    49,   108,   132,    -1,    -1,
     106,     1,    49,   109,   132,    -1,   103,    -1,   107,    -1,
       5,    33,    -1,     5,   140,    -1,   137,   111,    -1,    -1,
     137,   111,   116,   114,    47,   117,    48,    -1,   133,    51,
     115,    -1,   133,    -1,    24,    -1,    32,    -1,    12,   115,
      -1,    -1,    59,   118,    46,   117,    -1,     1,    46,   117,
      -1,    59,   118,    -1,    -1,   107,    -1,   120,    51,   119,
      -1,   120,    -1,   133,    -1,   140,    -1,   137,     8,    52,
      59,   134,    53,    33,    -1,   137,     8,    52,    59,   134,
      53,   140,    -1,   137,     9,    52,    59,   134,    51,    59,
     134,    53,    33,    -1,   137,     9,    52,    59,   134,    51,
      59,   134,    53,   140,    -1,    10,    33,    -1,    10,   140,
      -1,    -1,   137,   123,   125,    47,   127,    48,    -1,    -1,
     137,    10,   126,    47,   127,    48,    -1,   128,    51,   127,
      -1,   128,    -1,    33,    -1,    33,    50,   129,    -1,   140,
      -1,    -1,    36,    -1,   133,    -1,    11,    -1,    -1,    -1,
     130,    59,    87,    -1,   131,    51,   130,    59,    87,    -1,
     130,    59,   134,   140,    -1,   131,    51,   130,    59,   134,
     140,    -1,   130,    59,   134,    -1,   131,    51,   130,    59,
     134,    -1,    14,   119,    -1,    -1,    33,    -1,    34,    -1,
      16,    -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,
      21,    -1,    22,    -1,    23,    -1,    24,    -1,    24,    54,
      -1,    25,    -1,    32,    -1,   133,    -1,   133,    54,    -1,
      35,   135,    -1,    35,    -1,   136,    51,   135,    -1,   135,
      -1,    26,    -1,    -1,    36,    -1,    37,    -1,   133,    -1,
      35,    -1,    28,    -1,    29,    -1,    27,    59,   134,    33,
      50,   138,    -1,    27,    59,   134,    50,   138,    -1,     3,
      -1,     4,    -1,     5,    -1,     6,    -1,     7,    -1,     8,
      -1,     9,    -1,    10,    -1,    11,    -1,    12,    -1,    13,
      -1,    14,    -1,    15,    -1,    16,    -1,    17,    -1,    18,
      -1,    19,    -1,    20,    -1,    21,    -1,    22,    -1,    23,
      -1,    24,    -1,    25,    -1,    26,    -1,    27,    -1,    28,
      -1,    29,    -1,    30,    -1,    31,    -1,    32,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   115,   115,   123,   127,   134,   143,   148,   157,   156,
     166,   165,   176,   184,   183,   189,   188,   193,   198,   197,
     203,   202,   207,   212,   211,   217,   216,   221,   226,   225,
     231,   230,   235,   240,   239,   245,   244,   249,   254,   253,
     258,   263,   262,   268,   267,   272,   276,   286,   285,   319,
     323,   334,   345,   344,   371,   380,   388,   397,   400,   405,
     412,   425,   445,   538,   546,   559,   567,   582,   588,   592,
     603,   614,   613,   655,   664,   667,   672,   679,   685,   689,
     700,   725,   827,   839,   853,   852,   892,   927,   935,   940,
     948,   957,   960,   965,   972,   994,  1021,  1043,  1069,  1078,
    1089,  1098,  1107,  1117,  1131,  1137,  1145,  1157,  1181,  1206,
    1230,  1261,  1260,  1283,  1282,  1305,  1306,  1312,  1316,  1327,
    1342,  1341,  1376,  1411,  1446,  1451,  1461,  1466,  1474,  1483,
    1486,  1491,  1498,  1504,  1511,  1523,  1535,  1546,  1555,  1570,
    1581,  1598,  1602,  1614,  1613,  1646,  1645,  1664,  1670,  1678,
    1690,  1710,  1718,  1727,  1731,  1770,  1777,  1788,  1790,  1806,
    1822,  1834,  1846,  1857,  1873,  1878,  1886,  1889,  1897,  1901,
    1905,  1909,  1913,  1917,  1921,  1925,  1929,  1933,  1937,  1941,
    1945,  1964,  2005,  2011,  2019,  2026,  2038,  2045,  2055,  2068,
    2081,  2128,  2139,  2150,  2166,  2175,  2189,  2192,  2195,  2198,
    2201,  2204,  2207,  2210,  2213,  2216,  2219,  2222,  2225,  2228,
    2231,  2234,  2237,  2240,  2243,  2246,  2249,  2252,  2255,  2258,
    2261,  2264,  2267,  2270,  2273,  2276
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
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
  "ICE_IDEMPOTENT", "ICE_OPTIONAL", "ICE_VALUE", "ICE_IDENTIFIER",
  "ICE_SCOPED_IDENTIFIER", "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENT_OP", "ICE_KEYWORD_OP",
  "ICE_OPTIONAL_OP", "ICE_METADATA_OPEN", "ICE_METADATA_CLOSE",
  "ICE_GLOBAL_METADATA_OPEN", "ICE_GLOBAL_METADATA_CLOSE", "BAD_CHAR",
  "';'", "'{'", "'}'", "')'", "'='", "','", "'<'", "'>'", "'*'", "$accept",
  "start", "opt_semicolon", "global_meta_data", "meta_data", "definitions",
  "$@1", "$@2", "definition", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8",
  "$@9", "$@10", "$@11", "$@12", "$@13", "$@14", "$@15", "module_def",
  "@16", "exception_id", "exception_decl", "exception_def", "@17",
  "exception_extends", "exception_exports", "type_id", "optional",
  "optional_type_id", "exception_export", "struct_id", "struct_decl",
  "struct_def", "@18", "struct_exports", "struct_export", "class_name",
  "class_id", "class_decl", "class_def", "@19", "class_extends",
  "implements", "class_exports", "data_member", "struct_data_member",
  "return_type", "operation_preamble", "operation", "@20", "@21",
  "class_export", "interface_id", "interface_decl", "interface_def", "@22",
  "interface_list", "interface_extends", "interface_exports",
  "interface_export", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_id", "enum_def", "@23", "@24", "enumerator_list",
  "enumerator", "enumerator_initializer", "out_qualifier", "parameters",
  "throws", "scoped_name", "type", "string_literal", "string_list",
  "local_qualifier", "const_initializer", "const_def", "keyword", YY_NULL
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
     295,   296,   297,   298,   299,   300,    59,   123,   125,    41,
      61,    44,    60,    62,    42
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    55,    56,    57,    57,    58,    59,    59,    61,    60,
      62,    60,    60,    64,    63,    65,    63,    63,    66,    63,
      67,    63,    63,    68,    63,    69,    63,    63,    70,    63,
      71,    63,    63,    72,    63,    73,    63,    63,    74,    63,
      63,    75,    63,    76,    63,    63,    63,    78,    77,    79,
      79,    80,    82,    81,    83,    83,    84,    84,    84,    84,
      85,    86,    86,    86,    86,    87,    87,    88,    89,    89,
      90,    92,    91,    93,    93,    93,    93,    94,    95,    95,
      96,    96,    96,    97,    99,    98,   100,   100,   101,   101,
     102,   102,   102,   102,   103,   103,   103,   103,   104,   104,
     104,   104,   104,   104,   105,   105,   105,   106,   106,   106,
     106,   108,   107,   109,   107,   110,   110,   111,   111,   112,
     114,   113,   115,   115,   115,   115,   116,   116,   117,   117,
     117,   117,   118,   119,   119,   120,   120,   121,   121,   122,
     122,   123,   123,   125,   124,   126,   124,   127,   127,   128,
     128,   128,   128,   129,   129,   130,   130,   131,   131,   131,
     131,   131,   131,   131,   132,   132,   133,   133,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   135,   135,   136,   136,   137,   137,   138,   138,
     138,   138,   138,   138,   139,   139,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     0,     0,     3,
       0,     4,     0,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       1,     0,     3,     0,     3,     1,     2,     0,     6,     2,
       2,     2,     0,     7,     2,     0,     4,     3,     2,     0,
       2,     3,     3,     2,     1,     2,     1,     1,     2,     2,
       2,     0,     6,     4,     3,     2,     0,     1,     2,     2,
       4,     4,     1,     2,     0,     8,     2,     0,     2,     0,
       4,     3,     2,     0,     1,     3,     2,     1,     1,     3,
       2,     4,     2,     1,     2,     1,     1,     2,     3,     2,
       3,     0,     5,     0,     5,     1,     1,     2,     2,     2,
       0,     7,     3,     1,     1,     1,     2,     0,     4,     3,
       2,     0,     1,     3,     1,     1,     1,     7,     7,    10,
      10,     2,     2,     0,     6,     0,     6,     3,     1,     1,
       3,     1,     0,     1,     1,     1,     0,     0,     3,     5,
       4,     6,     3,     5,     2,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       1,     2,     2,     1,     3,     1,     1,     0,     1,     1,
       1,     1,     1,     1,     6,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       7,     0,     0,     0,     8,     0,     2,   183,   185,     0,
       0,     1,     7,     0,     0,   186,     7,    10,    13,    27,
      28,    32,    33,    17,    18,    22,    23,    37,    40,    41,
       0,    45,   182,     6,     0,     5,     9,    46,    47,     0,
       7,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     0,     4,     0,     0,     0,     0,     0,     0,   145,
      51,    70,    83,    87,   119,   143,     0,   184,     0,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   178,   179,
     166,   167,   180,     0,    11,     3,    14,    26,    29,    31,
      34,    16,    19,    21,    24,    36,    39,    42,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,    78,     0,
      79,   117,   118,    49,    50,    68,    69,     7,     7,   141,
       0,   142,     0,    52,     0,     0,    89,     0,   120,     0,
      44,     7,   177,   181,     0,     0,     0,     0,     0,     0,
     152,    54,     0,     0,    86,     0,    84,   124,   125,   126,
     123,     0,   152,     0,     0,   192,   193,   191,   188,   189,
     190,   195,    80,    81,     0,     0,   149,     0,   148,   151,
       0,     0,     0,     0,    88,     0,     0,     0,     0,    48,
     194,     0,     7,     0,   146,   152,     0,     0,     0,     0,
      64,     0,    98,     0,    75,    77,   103,    72,     0,   122,
       0,     0,     0,   144,   137,   138,     0,   153,   150,   154,
     147,     0,    66,     0,    94,    58,    67,    97,    53,    74,
       0,    63,     0,     0,   100,     0,     0,    60,   102,     0,
       0,     0,     0,   106,     0,     0,     0,     0,   132,   130,
     105,   121,     0,    57,    65,     0,     0,    96,    61,    62,
      99,     0,    73,     0,     0,   115,   116,    92,    97,    85,
     129,     0,   104,   107,   109,     0,   155,     7,     0,     0,
       0,    95,    56,   101,    91,   104,     0,   108,   110,   113,
       0,   111,   156,   128,   139,   140,    90,   165,   158,   162,
     165,     7,     0,   114,   160,   112,     0,   164,   134,   135,
     136,   159,   163,     0,   161,   133
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,    86,     4,     5,     6,    12,    40,    17,    41,
      46,    47,    48,    49,    42,    43,    44,    45,    50,    51,
      52,    66,    18,    68,    60,    19,    20,   162,   143,   208,
     232,   233,   234,   235,    61,    21,    22,   144,   193,   214,
      62,    63,    23,    24,   195,   146,   166,   251,   236,   215,
     256,   257,   258,   310,   307,   277,    64,    25,    26,   171,
     169,   148,   222,   259,   317,   318,    27,    28,    65,    29,
     149,   140,   187,   188,   228,   287,   288,   313,    82,   245,
       8,     9,    30,   181,    31,   189
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -278
static const yytype_int16 yypact[] =
{
      13,     1,     1,    42,  -278,   156,  -278,     1,  -278,    -7,
      -3,  -278,    10,     4,    33,  -278,     6,  -278,  -278,    28,
    -278,    30,  -278,    32,  -278,    39,  -278,    52,    68,  -278,
     200,    77,  -278,  -278,     1,  -278,  -278,  -278,  -278,   835,
      10,    84,    85,    84,    87,    84,    89,    84,    92,    84,
      93,    94,    84,   455,   523,   554,   585,    80,    96,   616,
      12,    35,    16,    76,    14,  -278,   100,  -278,    95,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,    82,  -278,  -278,
    -278,  -278,   101,   -12,  -278,  -278,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,    59,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,     6,     6,  -278,
     109,  -278,    37,  -278,   120,    37,   155,    57,  -278,   123,
    -278,    36,  -278,  -278,   121,   116,   126,   127,   835,   835,
     647,  -278,   125,   338,  -278,    57,  -278,  -278,  -278,  -278,
     128,   130,   647,   132,   116,  -278,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,   135,   134,   139,   143,   146,  -278,
     372,   153,   810,   163,  -278,   131,    57,    86,   164,  -278,
    -278,   678,     6,    63,  -278,   647,   167,   810,   166,   338,
    -278,    31,   168,   835,   171,  -278,   709,  -278,   304,  -278,
     173,   759,   174,  -278,  -278,  -278,   835,  -278,  -278,  -278,
    -278,   372,  -278,   835,   175,   177,  -278,   709,  -278,  -278,
     152,  -278,   178,   116,   176,   191,   338,  -278,  -278,   182,
     759,   181,    86,  -278,   785,   835,    74,   267,  -278,   186,
    -278,  -278,   180,  -278,  -278,   116,   372,  -278,  -278,  -278,
    -278,   116,  -278,   304,   835,  -278,  -278,   188,   418,  -278,
    -278,    90,  -278,  -278,  -278,   189,  -278,     6,   -17,    86,
     740,  -278,  -278,  -278,  -278,   191,   304,  -278,  -278,  -278,
     810,  -278,   224,  -278,  -278,  -278,  -278,   223,  -278,   709,
     223,     6,   491,  -278,  -278,  -278,   810,  -278,   190,  -278,
    -278,  -278,   709,   491,  -278,  -278
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -278,  -278,   151,  -278,   -16,   -10,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -278,  -209,
    -188,  -181,  -277,  -278,  -278,  -278,  -278,  -278,  -194,  -278,
    -278,  -278,  -278,  -278,  -278,  -278,  -278,  -253,   -11,  -278,
      -9,  -278,    -8,  -278,  -278,  -278,  -278,  -278,  -278,  -278,
    -134,  -278,  -234,  -278,   -79,  -278,  -278,  -278,  -278,  -278,
    -278,  -278,  -156,  -278,  -278,   -56,  -278,   -63,  -128,   -34,
      26,   246,  -278,  -171,  -278,   -47
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -188
static const yytype_int16 yytable[] =
{
      39,   157,    36,   200,   212,    83,   130,   132,   134,   136,
     -12,   213,   141,   -12,   161,   239,   198,   164,   280,   170,
     294,   154,   263,   308,   142,   244,   147,   180,   -82,   -82,
      84,   194,   301,    32,   302,    33,     7,   170,   155,   321,
     255,    35,    11,   306,    34,   264,   180,     1,    34,   230,
      37,     1,   272,     2,     1,   303,     2,   292,   -12,   -55,
      67,  -127,   219,   -82,    80,    81,    38,   240,   170,   274,
      80,    81,   270,   255,   -25,   229,   -30,     1,   -15,     2,
     241,   167,   -71,   242,   -12,   -20,   264,   220,   145,   168,
      80,    81,    80,    81,   291,   156,    80,    81,   -35,   227,
     293,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,   283,   284,   -38,   180,    -7,    -7,    -7,    -7,
      -7,   158,   159,   -43,   184,   185,    -7,     1,   297,   298,
      85,    87,   137,    89,  -131,    91,   152,   180,    93,    95,
      96,   173,   151,   180,   175,   176,   150,   192,   138,    80,
      81,   177,   178,   179,   225,   153,   160,    13,   216,    14,
    -187,  -187,  -187,  -187,  -187,  -187,  -187,   163,   165,   248,
     172,   174,   190,   237,   207,   182,   183,   197,   218,   196,
     199,   221,    15,    16,   319,   202,   226,   260,   201,   203,
     267,   204,   262,   192,    88,   319,    90,   205,    92,   209,
      94,   268,   250,    97,    53,    54,    55,    56,    57,    58,
      59,   217,   223,   231,   238,   207,   278,   246,   243,   252,
     260,   282,   261,   266,   247,   265,   271,   269,   273,   279,
     192,   267,   289,   290,   296,   286,   221,   312,   299,   275,
     295,   323,   276,   305,   325,   281,   311,   315,    10,     0,
     207,     0,     0,     0,     0,     0,     0,   250,     0,     0,
       0,     0,   314,     0,     0,   320,   309,     0,   285,     0,
       0,   300,     0,   221,     0,   324,   320,     0,   286,     0,
     250,     0,   322,  -156,  -156,  -156,  -156,  -156,  -156,  -156,
    -156,  -156,  -156,     0,     0,   316,     0,     0,  -156,  -156,
    -156,  -156,     0,     0,     0,   249,     0,  -156,  -156,     0,
       0,     0,     0,     0,     0,     0,  -157,     0,  -157,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,
       0,     0,     0,     0,    -7,    -7,    -7,    -7,    -7,   191,
       0,     0,     0,     0,    -7,     1,     0,     0,     0,     0,
       0,     0,   -93,     0,    -7,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,     0,     0,     0,     0,     0,    -7,
      -7,    -7,    -7,   206,     0,     0,     0,     0,    -7,     1,
       0,     0,     0,     0,     0,     0,   -76,     0,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,     0,     0,
       0,     0,     0,    -7,    -7,    -7,    -7,     0,     0,     0,
       0,     0,    -7,     1,     0,     0,     0,     0,     0,     0,
     -59,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   247,     0,     0,     0,     0,  -105,  -105,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   128,     0,
       0,     0,     0,   129,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,    80,    81,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   131,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   133,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   126,   127,   135,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   139,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     186,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,   224,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   247,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   304,   253,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,     0,     0,     0,     0,   254,
     210,    79,    80,    81,     0,     0,     0,     0,     0,   211,
     253,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,   210,    79,    80,    81,
       0,     0,     0,     0,     0,   211,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,   210,    79,    80,    81,     0,     0,     0,     0,     0,
     211,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     0,    79,    80,    81
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-278)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      16,   129,    12,   174,   192,    39,    53,    54,    55,    56,
       0,   192,    59,     0,   142,   209,   172,   145,   252,   147,
     273,    33,   231,   300,    12,   213,    12,   155,    12,    13,
      40,   165,    49,     7,    51,    42,    35,   165,    50,   316,
     221,    44,     0,   296,    51,   233,   174,    41,    51,   205,
      46,    41,   246,    43,    41,   289,    43,   266,    48,    47,
      34,    47,   196,    47,    33,    34,    33,    36,   196,   250,
      33,    34,   243,   254,    46,   203,    46,    41,    46,    43,
      49,    24,    47,   211,    48,    46,   274,     1,    12,    32,
      33,    34,    33,    34,   265,    36,    33,    34,    46,    36,
     271,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    38,    39,    46,   243,    30,    31,    32,    33,
      34,   137,   138,    46,   158,   159,    40,    41,    38,    39,
      46,    46,    52,    46,    48,    46,    54,   265,    46,    46,
      46,   151,    47,   271,    28,    29,    46,   163,    52,    33,
      34,    35,    36,    37,   201,    54,    47,     1,   192,     3,
       4,     5,     6,     7,     8,     9,    10,    47,    13,   216,
      47,    50,    47,   207,   190,    49,    49,    47,    47,    51,
      48,   197,    26,    27,   312,    51,   202,   221,    53,    50,
     237,    48,   226,   209,    43,   323,    45,    51,    47,    46,
      49,    49,   218,    52,     4,     5,     6,     7,     8,     9,
      10,    48,    48,    46,    48,   231,   250,    46,    50,    46,
     254,   255,    48,    46,    33,    50,    50,    49,    46,    48,
     246,   278,    46,    53,    46,    11,   252,    14,    49,   250,
     274,    51,   250,   290,   323,   254,   302,   310,     2,    -1,
     266,    -1,    -1,    -1,    -1,    -1,    -1,   273,    -1,    -1,
      -1,    -1,   309,    -1,    -1,   312,   300,    -1,     1,    -1,
      -1,   287,    -1,   289,    -1,   322,   323,    -1,    11,    -1,
     296,    -1,   316,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    -1,   311,    -1,    -1,    31,    32,
      33,    34,    -1,    -1,    -1,     1,    -1,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    51,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      -1,    -1,    -1,    -1,    30,    31,    32,    33,    34,     1,
      -1,    -1,    -1,    -1,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      32,    33,    34,     1,    -1,    -1,    -1,    -1,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    -1,    -1,    31,    32,    33,    34,    -1,    -1,    -1,
      -1,    -1,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      48,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    -1,    -1,    -1,    38,    39,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
      -1,    -1,    -1,    38,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,    30,
      31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,    40,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,    34,
      -1,    -1,    -1,    -1,    -1,    40,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,
      -1,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      40,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    -1,    -1,    -1,    32,    33,    34
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    41,    43,    56,    58,    59,    60,    35,   135,   136,
     136,     0,    61,     1,     3,    26,    27,    63,    77,    80,
      81,    90,    91,    97,    98,   112,   113,   121,   122,   124,
     137,   139,   135,    42,    51,    44,    60,    46,    33,    59,
      62,    64,    69,    70,    71,    72,    65,    66,    67,    68,
      73,    74,    75,     4,     5,     6,     7,     8,     9,    10,
      79,    89,    95,    96,   111,   123,    76,   135,    78,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    32,
      33,    34,   133,   134,    60,    46,    57,    46,    57,    46,
      57,    46,    57,    46,    57,    46,    46,    57,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    38,
     140,    33,   140,    33,   140,    33,   140,    52,    52,    33,
     126,   140,    12,    83,    92,    12,   100,    12,   116,   125,
      46,    47,    54,    54,    33,    50,    36,   133,    59,    59,
      47,   133,    82,    47,   133,    13,   101,    24,    32,   115,
     133,   114,    47,    60,    50,    28,    29,    35,    36,    37,
     133,   138,    49,    49,   134,   134,    33,   127,   128,   140,
      47,     1,    59,    93,   115,    99,    51,    47,   127,    48,
     138,    53,    51,    50,    48,    51,     1,    59,    84,    46,
      31,    40,    85,    86,    94,   104,   134,    48,    47,   115,
       1,    59,   117,    48,    33,   140,    59,    36,   129,   133,
     127,    46,    85,    86,    87,    88,   103,   134,    48,    93,
      36,    49,   133,    50,    85,   134,    46,    33,   140,     1,
      59,   102,    46,    15,    30,    86,   105,   106,   107,   118,
     134,    48,   134,    84,    85,    50,    46,   140,    49,    49,
     138,    50,    93,    46,    86,   103,   107,   110,   134,    48,
     117,   105,   134,    38,    39,     1,    11,   130,   131,    46,
      53,   138,    84,   138,   102,   134,    46,    38,    39,    49,
      59,    49,    51,   117,    33,   140,   102,   109,    87,   134,
     108,   130,    14,   132,   140,   132,    59,   119,   120,   133,
     140,    87,   134,    51,   140,   119
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

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
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
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
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
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
/* The lookahead symbol.  */
int yychar;


#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

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
/* Line 1792 of yacc.c  */
#line 116 "src/Slice/Grammar.y"
    {
}
    break;

  case 3:
/* Line 1792 of yacc.c  */
#line 124 "src/Slice/Grammar.y"
    {
}
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 127 "src/Slice/Grammar.y"
    {
}
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 135 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
}
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 144 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (3)]);
}
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 148 "src/Slice/Grammar.y"
    {
    (yyval) = new StringListTok;
}
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 157 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 166 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (2)]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 176 "src/Slice/Grammar.y"
    {
}
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 184 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ModulePtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 189 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 194 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after class forward declaration");
}
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 198 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDefPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 203 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 208 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after interface forward declaration");
}
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 212 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ClassDefPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 217 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0);
}
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 222 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after exception forward declaration");
}
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 226 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ExceptionPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 231 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0);
}
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 236 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after struct forward declaration");
}
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 240 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || StructPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 245 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || SequencePtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 250 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after sequence definition");
}
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 254 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || DictionaryPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 259 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after dictionary definition");
}
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 263 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || EnumPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 268 "src/Slice/Grammar.y"
    {
    assert((yyvsp[(1) - (1)]) == 0 || ConstPtr::dynamicCast((yyvsp[(1) - (1)])));
}
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 273 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after const definition");
}
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 277 "src/Slice/Grammar.y"
    {
    yyerrok;
}
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 286 "src/Slice/Grammar.y"
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
}
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 303 "src/Slice/Grammar.y"
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
}
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 320 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 324 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
}
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 335 "src/Slice/Grammar.y"
    {
    unit->error("exceptions cannot be forward declared");
    (yyval) = 0;
}
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 345 "src/Slice/Grammar.y"
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
}
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 359 "src/Slice/Grammar.y"
    {
    if((yyvsp[(4) - (7)]))
    {
        unit->popContainer();
    }
    (yyval) = (yyvsp[(4) - (7)]);
}
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 372 "src/Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    (yyval) = contained;
}
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 380 "src/Slice/Grammar.y"
    {
    (yyval) = 0;
}
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 389 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 398 "src/Slice/Grammar.y"
    {
}
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 401 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
}
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 405 "src/Slice/Grammar.y"
    {
}
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 413 "src/Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    (yyval) = typestring;
}
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 426 "src/Slice/Grammar.y"
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
}
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 446 "src/Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(2) - (3)]));

    ContainerPtr cont = unit->currentContainer();
    assert(cont);
    ContainedList cl = cont->lookupContained(scoped->v, false);
    if(cl.empty())
    {
        EnumeratorList enumerators = cont->enumerators(scoped->v);
        if(enumerators.size() == 1)
        {
            // Found
            cl.push_back(enumerators.front());
            scoped->v = enumerators.front()->scoped();
            unit->warning(Deprecated, string("referencing enumerator `") + scoped->v
                          + "' without its enumeration's scope is deprecated");
        }
        else if(enumerators.size() > 1)
        {
            ostringstream os;
            os << "enumerator `" << scoped->v << "' could designate";
            bool first = true;
            for(EnumeratorList::iterator p = enumerators.begin(); p != enumerators.end(); ++p)
            {
                if(first)
                {
                    first = false;
                }
                else
                {
                    os << " or";
                }

                os << " `" << (*p)->scoped() << "'";
            }
            unit->error(os.str());
        }
        else
        {
            unit->error(string("`") + scoped->v + "' is not defined");
        }
    }

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
}
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 539 "src/Slice/Grammar.y"
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 547 "src/Slice/Grammar.y"
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 560 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    (yyval) = m;
}
    break;

  case 66:
/* Line 1792 of yacc.c  */
#line 568 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 589 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 69:
/* Line 1792 of yacc.c  */
#line 593 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
}
    break;

  case 70:
/* Line 1792 of yacc.c  */
#line 604 "src/Slice/Grammar.y"
    {
    unit->error("structs cannot be forward declared");
    (yyval) = 0; // Dummy
}
    break;

  case 71:
/* Line 1792 of yacc.c  */
#line 614 "src/Slice/Grammar.y"
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
}
    break;

  case 72:
/* Line 1792 of yacc.c  */
#line 633 "src/Slice/Grammar.y"
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
}
    break;

  case 73:
/* Line 1792 of yacc.c  */
#line 656 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
    break;

  case 74:
/* Line 1792 of yacc.c  */
#line 665 "src/Slice/Grammar.y"
    {
}
    break;

  case 75:
/* Line 1792 of yacc.c  */
#line 668 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
}
    break;

  case 76:
/* Line 1792 of yacc.c  */
#line 672 "src/Slice/Grammar.y"
    {
}
    break;

  case 78:
/* Line 1792 of yacc.c  */
#line 686 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 79:
/* Line 1792 of yacc.c  */
#line 690 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
}
    break;

  case 80:
/* Line 1792 of yacc.c  */
#line 701 "src/Slice/Grammar.y"
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
}
    break;

  case 81:
/* Line 1792 of yacc.c  */
#line 726 "src/Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(3) - (4)]));

    ContainerPtr cont = unit->currentContainer();
    assert(cont);
    ContainedList cl = cont->lookupContained(scoped->v, false);
    if(cl.empty())
    {
        EnumeratorList enumerators = cont->enumerators(scoped->v);
        if(enumerators.size() == 1)
        {
            // Found
            cl.push_back(enumerators.front());
            scoped->v = enumerators.front()->scoped();
            unit->warning(Deprecated, string("referencing enumerator `") + scoped->v
                          + "' without its enumeration's scope is deprecated");
        }
        else if(enumerators.size() > 1)
        {
            ostringstream os;
            os << "enumerator `" << scoped->v << "' could designate";
            bool first = true;
            for(EnumeratorList::iterator p = enumerators.begin(); p != enumerators.end(); ++p)
            {
                if(first)
                {
                    first = false;
                }
                else
                {
                    os << " or";
                }

                os << " `" << (*p)->scoped() << "'";
            }
            unit->error(os.str());
        }
        else
        {
            unit->error(string("`") + scoped->v + "' is not defined");
        }
    }

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

}
    break;

  case 82:
/* Line 1792 of yacc.c  */
#line 828 "src/Slice/Grammar.y"
    {
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]))->v;
    classId->t = -1;
    (yyval) = classId;
}
    break;

  case 83:
/* Line 1792 of yacc.c  */
#line 840 "src/Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false, local->v);
    (yyval) = cl;
}
    break;

  case 84:
/* Line 1792 of yacc.c  */
#line 853 "src/Slice/Grammar.y"
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
}
    break;

  case 85:
/* Line 1792 of yacc.c  */
#line 876 "src/Slice/Grammar.y"
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
}
    break;

  case 86:
/* Line 1792 of yacc.c  */
#line 893 "src/Slice/Grammar.y"
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
}
    break;

  case 87:
/* Line 1792 of yacc.c  */
#line 927 "src/Slice/Grammar.y"
    {
    (yyval) = 0;
}
    break;

  case 88:
/* Line 1792 of yacc.c  */
#line 936 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 89:
/* Line 1792 of yacc.c  */
#line 940 "src/Slice/Grammar.y"
    {
    (yyval) = new ClassListTok;
}
    break;

  case 90:
/* Line 1792 of yacc.c  */
#line 949 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
    break;

  case 91:
/* Line 1792 of yacc.c  */
#line 958 "src/Slice/Grammar.y"
    {
}
    break;

  case 92:
/* Line 1792 of yacc.c  */
#line 961 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
}
    break;

  case 93:
/* Line 1792 of yacc.c  */
#line 965 "src/Slice/Grammar.y"
    {
}
    break;

  case 94:
/* Line 1792 of yacc.c  */
#line 973 "src/Slice/Grammar.y"
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
}
    break;

  case 95:
/* Line 1792 of yacc.c  */
#line 995 "src/Slice/Grammar.y"
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
}
    break;

  case 96:
/* Line 1792 of yacc.c  */
#line 1022 "src/Slice/Grammar.y"
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
}
    break;

  case 97:
/* Line 1792 of yacc.c  */
#line 1044 "src/Slice/Grammar.y"
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
}
    break;

  case 98:
/* Line 1792 of yacc.c  */
#line 1070 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
}
    break;

  case 99:
/* Line 1792 of yacc.c  */
#line 1079 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, value->v.value,
                                            value->v.valueAsString, value->v.valueAsLiteral);
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
}
    break;

  case 100:
/* Line 1792 of yacc.c  */
#line 1090 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
}
    break;

  case 101:
/* Line 1792 of yacc.c  */
#line 1099 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[(2) - (4)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
}
    break;

  case 102:
/* Line 1792 of yacc.c  */
#line 1108 "src/Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (2)]));
    string name = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]))->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
    break;

  case 103:
/* Line 1792 of yacc.c  */
#line 1118 "src/Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[(1) - (1)]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("missing data member name");
}
    break;

  case 104:
/* Line 1792 of yacc.c  */
#line 1132 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    m->v.type = TypePtr::dynamicCast((yyvsp[(2) - (2)]));
    (yyval) = m;
}
    break;

  case 105:
/* Line 1792 of yacc.c  */
#line 1138 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok();
    m->v.type = TypePtr::dynamicCast((yyvsp[(1) - (1)]));
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
    break;

  case 106:
/* Line 1792 of yacc.c  */
#line 1146 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
    break;

  case 107:
/* Line 1792 of yacc.c  */
#line 1158 "src/Slice/Grammar.y"
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
}
    break;

  case 108:
/* Line 1792 of yacc.c  */
#line 1182 "src/Slice/Grammar.y"
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
}
    break;

  case 109:
/* Line 1792 of yacc.c  */
#line 1207 "src/Slice/Grammar.y"
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
}
    break;

  case 110:
/* Line 1792 of yacc.c  */
#line 1231 "src/Slice/Grammar.y"
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
}
    break;

  case 111:
/* Line 1792 of yacc.c  */
#line 1261 "src/Slice/Grammar.y"
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
}
    break;

  case 112:
/* Line 1792 of yacc.c  */
#line 1273 "src/Slice/Grammar.y"
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[(4) - (5)]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
    break;

  case 113:
/* Line 1792 of yacc.c  */
#line 1283 "src/Slice/Grammar.y"
    {
    if((yyvsp[(1) - (3)]))
    {
        unit->popContainer();
    }
    yyerrok;
}
    break;

  case 114:
/* Line 1792 of yacc.c  */
#line 1291 "src/Slice/Grammar.y"
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[(4) - (5)]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
    break;

  case 117:
/* Line 1792 of yacc.c  */
#line 1313 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 118:
/* Line 1792 of yacc.c  */
#line 1317 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
}
    break;

  case 119:
/* Line 1792 of yacc.c  */
#line 1328 "src/Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    cont->checkIntroduced(ident->v, cl);
    (yyval) = cl;
}
    break;

  case 120:
/* Line 1792 of yacc.c  */
#line 1342 "src/Slice/Grammar.y"
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
}
    break;

  case 121:
/* Line 1792 of yacc.c  */
#line 1360 "src/Slice/Grammar.y"
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
}
    break;

  case 122:
/* Line 1792 of yacc.c  */
#line 1377 "src/Slice/Grammar.y"
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
}
    break;

  case 123:
/* Line 1792 of yacc.c  */
#line 1412 "src/Slice/Grammar.y"
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
}
    break;

  case 124:
/* Line 1792 of yacc.c  */
#line 1447 "src/Slice/Grammar.y"
    {
    unit->error("illegal inheritance from type Object");
    (yyval) = new ClassListTok; // Dummy
}
    break;

  case 125:
/* Line 1792 of yacc.c  */
#line 1452 "src/Slice/Grammar.y"
    {
    unit->error("illegal inheritance from type Value");
    (yyval) = new ClassListTok; // Dummy
}
    break;

  case 126:
/* Line 1792 of yacc.c  */
#line 1462 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 127:
/* Line 1792 of yacc.c  */
#line 1466 "src/Slice/Grammar.y"
    {
    (yyval) = new ClassListTok;
}
    break;

  case 128:
/* Line 1792 of yacc.c  */
#line 1475 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(1) - (4)]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[(2) - (4)]));
    if(contained && !metaData->v.empty())
    {
    contained->setMetaData(metaData->v);
    }
}
    break;

  case 129:
/* Line 1792 of yacc.c  */
#line 1484 "src/Slice/Grammar.y"
    {
}
    break;

  case 130:
/* Line 1792 of yacc.c  */
#line 1487 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
}
    break;

  case 131:
/* Line 1792 of yacc.c  */
#line 1491 "src/Slice/Grammar.y"
    {
}
    break;

  case 133:
/* Line 1792 of yacc.c  */
#line 1505 "src/Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[(1) - (3)]));
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
}
    break;

  case 134:
/* Line 1792 of yacc.c  */
#line 1512 "src/Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[(1) - (1)]));
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
}
    break;

  case 135:
/* Line 1792 of yacc.c  */
#line 1524 "src/Slice/Grammar.y"
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
}
    break;

  case 136:
/* Line 1792 of yacc.c  */
#line 1536 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
}
    break;

  case 137:
/* Line 1792 of yacc.c  */
#line 1547 "src/Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (7)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(7) - (7)]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (7)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (7)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v);
}
    break;

  case 138:
/* Line 1792 of yacc.c  */
#line 1556 "src/Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (7)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(7) - (7)]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (7)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (7)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
    break;

  case 139:
/* Line 1792 of yacc.c  */
#line 1571 "src/Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (10)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(10) - (10)]));
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast((yyvsp[(4) - (10)]));
    TypePtr keyType = TypePtr::dynamicCast((yyvsp[(5) - (10)]));
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast((yyvsp[(7) - (10)]));
    TypePtr valueType = TypePtr::dynamicCast((yyvsp[(8) - (10)]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v);
}
    break;

  case 140:
/* Line 1792 of yacc.c  */
#line 1582 "src/Slice/Grammar.y"
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
}
    break;

  case 141:
/* Line 1792 of yacc.c  */
#line 1599 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 142:
/* Line 1792 of yacc.c  */
#line 1603 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    (yyval) = (yyvsp[(2) - (2)]); // Dummy
}
    break;

  case 143:
/* Line 1792 of yacc.c  */
#line 1614 "src/Slice/Grammar.y"
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    if(en)
    {
        cont->checkIntroduced(ident->v, en);
    }
    else
    {
        en = cont->createEnum(IceUtil::generateUUID(), local->v, Dummy);
    }
    unit->pushContainer(en);
    (yyval) = en;
}
    break;

  case 144:
/* Line 1792 of yacc.c  */
#line 1631 "src/Slice/Grammar.y"
    {
    EnumPtr en = EnumPtr::dynamicCast((yyvsp[(3) - (6)]));
    if(en)
    {
        EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast((yyvsp[(5) - (6)]));
        if(enumerators->v.empty())
        {
            unit->error("enum `" + en->name() + "' must have at least one enumerator");
        }
        unit->popContainer();
    }
    (yyval) = (yyvsp[(3) - (6)]);
}
    break;

  case 145:
/* Line 1792 of yacc.c  */
#line 1646 "src/Slice/Grammar.y"
    {
    unit->error("missing enumeration name");
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), local->v, Dummy);
    unit->pushContainer(en);
    (yyval) = en;
}
    break;

  case 146:
/* Line 1792 of yacc.c  */
#line 1655 "src/Slice/Grammar.y"
    {
    unit->popContainer();
    (yyval) = (yyvsp[(2) - (6)]);
}
    break;

  case 147:
/* Line 1792 of yacc.c  */
#line 1665 "src/Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast((yyvsp[(3) - (3)]))->v);
    (yyval) = ens;
}
    break;

  case 148:
/* Line 1792 of yacc.c  */
#line 1671 "src/Slice/Grammar.y"
    {
}
    break;

  case 149:
/* Line 1792 of yacc.c  */
#line 1679 "src/Slice/Grammar.y"
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
}
    break;

  case 150:
/* Line 1792 of yacc.c  */
#line 1691 "src/Slice/Grammar.y"
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
            ens->v.push_front(en);
        }
    }
    (yyval) = ens;
}
    break;

  case 151:
/* Line 1792 of yacc.c  */
#line 1711 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    (yyval) = ens;
}
    break;

  case 152:
/* Line 1792 of yacc.c  */
#line 1718 "src/Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    (yyval) = ens; // Dummy
}
    break;

  case 153:
/* Line 1792 of yacc.c  */
#line 1728 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(1) - (1)]);
}
    break;

  case 154:
/* Line 1792 of yacc.c  */
#line 1732 "src/Slice/Grammar.y"
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
}
    break;

  case 155:
/* Line 1792 of yacc.c  */
#line 1771 "src/Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = true;
    (yyval) = out;
}
    break;

  case 156:
/* Line 1792 of yacc.c  */
#line 1777 "src/Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = false;
    (yyval) = out;
}
    break;

  case 157:
/* Line 1792 of yacc.c  */
#line 1788 "src/Slice/Grammar.y"
    {
}
    break;

  case 158:
/* Line 1792 of yacc.c  */
#line 1791 "src/Slice/Grammar.y"
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
}
    break;

  case 159:
/* Line 1792 of yacc.c  */
#line 1807 "src/Slice/Grammar.y"
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
}
    break;

  case 160:
/* Line 1792 of yacc.c  */
#line 1823 "src/Slice/Grammar.y"
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
}
    break;

  case 161:
/* Line 1792 of yacc.c  */
#line 1835 "src/Slice/Grammar.y"
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
}
    break;

  case 162:
/* Line 1792 of yacc.c  */
#line 1847 "src/Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(3) - (3)]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        unit->error("missing parameter name");
    }
}
    break;

  case 163:
/* Line 1792 of yacc.c  */
#line 1858 "src/Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[(3) - (5)]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[(5) - (5)]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        unit->error("missing parameter name");
    }
}
    break;

  case 164:
/* Line 1792 of yacc.c  */
#line 1874 "src/Slice/Grammar.y"
    {
    (yyval) = (yyvsp[(2) - (2)]);
}
    break;

  case 165:
/* Line 1792 of yacc.c  */
#line 1878 "src/Slice/Grammar.y"
    {
    (yyval) = new ExceptionListTok;
}
    break;

  case 166:
/* Line 1792 of yacc.c  */
#line 1887 "src/Slice/Grammar.y"
    {
}
    break;

  case 167:
/* Line 1792 of yacc.c  */
#line 1890 "src/Slice/Grammar.y"
    {
}
    break;

  case 168:
/* Line 1792 of yacc.c  */
#line 1898 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindByte);
}
    break;

  case 169:
/* Line 1792 of yacc.c  */
#line 1902 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindBool);
}
    break;

  case 170:
/* Line 1792 of yacc.c  */
#line 1906 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindShort);
}
    break;

  case 171:
/* Line 1792 of yacc.c  */
#line 1910 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindInt);
}
    break;

  case 172:
/* Line 1792 of yacc.c  */
#line 1914 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindLong);
}
    break;

  case 173:
/* Line 1792 of yacc.c  */
#line 1918 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindFloat);
}
    break;

  case 174:
/* Line 1792 of yacc.c  */
#line 1922 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindDouble);
}
    break;

  case 175:
/* Line 1792 of yacc.c  */
#line 1926 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindString);
}
    break;

  case 176:
/* Line 1792 of yacc.c  */
#line 1930 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindObject);
}
    break;

  case 177:
/* Line 1792 of yacc.c  */
#line 1934 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindObjectProxy);
}
    break;

  case 178:
/* Line 1792 of yacc.c  */
#line 1938 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindLocalObject);
}
    break;

  case 179:
/* Line 1792 of yacc.c  */
#line 1942 "src/Slice/Grammar.y"
    {
    (yyval) = unit->builtin(Builtin::KindValue);
}
    break;

  case 180:
/* Line 1792 of yacc.c  */
#line 1946 "src/Slice/Grammar.y"
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
}
    break;

  case 181:
/* Line 1792 of yacc.c  */
#line 1965 "src/Slice/Grammar.y"
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
}
    break;

  case 182:
/* Line 1792 of yacc.c  */
#line 2006 "src/Slice/Grammar.y"
    {
    StringTokPtr str1 = StringTokPtr::dynamicCast((yyvsp[(1) - (2)]));
    StringTokPtr str2 = StringTokPtr::dynamicCast((yyvsp[(2) - (2)]));
    str1->v += str2->v;
}
    break;

  case 183:
/* Line 1792 of yacc.c  */
#line 2012 "src/Slice/Grammar.y"
    {
}
    break;

  case 184:
/* Line 1792 of yacc.c  */
#line 2020 "src/Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[(3) - (3)]));
    StringListTokPtr stringList = StringListTokPtr::dynamicCast((yyvsp[(1) - (3)]));
    stringList->v.push_back(str->v);
    (yyval) = stringList;
}
    break;

  case 185:
/* Line 1792 of yacc.c  */
#line 2027 "src/Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    (yyval) = stringList;
}
    break;

  case 186:
/* Line 1792 of yacc.c  */
#line 2039 "src/Slice/Grammar.y"
    {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    (yyval) = local;
}
    break;

  case 187:
/* Line 1792 of yacc.c  */
#line 2045 "src/Slice/Grammar.y"
    {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    (yyval) = local;
}
    break;

  case 188:
/* Line 1792 of yacc.c  */
#line 2056 "src/Slice/Grammar.y"
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
}
    break;

  case 189:
/* Line 1792 of yacc.c  */
#line 2069 "src/Slice/Grammar.y"
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
}
    break;

  case 190:
/* Line 1792 of yacc.c  */
#line 2082 "src/Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ConstDefTokPtr def = new ConstDefTok;
    ContainedList cl = unit->currentContainer()->lookupContained(scoped->v, false);
    if(cl.empty())
    {
        // Could be an enumerator
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
}
    break;

  case 191:
/* Line 1792 of yacc.c  */
#line 2129 "src/Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = literal->v;
    def->v.valueAsLiteral = literal->literal;
    (yyval) = def;
}
    break;

  case 192:
/* Line 1792 of yacc.c  */
#line 2140 "src/Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "false";
    def->v.valueAsLiteral = "false";
    (yyval) = def;
}
    break;

  case 193:
/* Line 1792 of yacc.c  */
#line 2151 "src/Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[(1) - (1)]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "true";
    def->v.valueAsLiteral = "true";
    (yyval) = def;
}
    break;

  case 194:
/* Line 1792 of yacc.c  */
#line 2167 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (6)]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[(3) - (6)]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[(4) - (6)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(6) - (6)]));
    (yyval) = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral);
}
    break;

  case 195:
/* Line 1792 of yacc.c  */
#line 2176 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[(2) - (5)]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[(3) - (5)]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[(5) - (5)]));
    unit->error("missing constant name");
    (yyval) = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral, Dummy); // Dummy
}
    break;

  case 196:
/* Line 1792 of yacc.c  */
#line 2190 "src/Slice/Grammar.y"
    {
}
    break;

  case 197:
/* Line 1792 of yacc.c  */
#line 2193 "src/Slice/Grammar.y"
    {
}
    break;

  case 198:
/* Line 1792 of yacc.c  */
#line 2196 "src/Slice/Grammar.y"
    {
}
    break;

  case 199:
/* Line 1792 of yacc.c  */
#line 2199 "src/Slice/Grammar.y"
    {
}
    break;

  case 200:
/* Line 1792 of yacc.c  */
#line 2202 "src/Slice/Grammar.y"
    {
}
    break;

  case 201:
/* Line 1792 of yacc.c  */
#line 2205 "src/Slice/Grammar.y"
    {
}
    break;

  case 202:
/* Line 1792 of yacc.c  */
#line 2208 "src/Slice/Grammar.y"
    {
}
    break;

  case 203:
/* Line 1792 of yacc.c  */
#line 2211 "src/Slice/Grammar.y"
    {
}
    break;

  case 204:
/* Line 1792 of yacc.c  */
#line 2214 "src/Slice/Grammar.y"
    {
}
    break;

  case 205:
/* Line 1792 of yacc.c  */
#line 2217 "src/Slice/Grammar.y"
    {
}
    break;

  case 206:
/* Line 1792 of yacc.c  */
#line 2220 "src/Slice/Grammar.y"
    {
}
    break;

  case 207:
/* Line 1792 of yacc.c  */
#line 2223 "src/Slice/Grammar.y"
    {
}
    break;

  case 208:
/* Line 1792 of yacc.c  */
#line 2226 "src/Slice/Grammar.y"
    {
}
    break;

  case 209:
/* Line 1792 of yacc.c  */
#line 2229 "src/Slice/Grammar.y"
    {
}
    break;

  case 210:
/* Line 1792 of yacc.c  */
#line 2232 "src/Slice/Grammar.y"
    {
}
    break;

  case 211:
/* Line 1792 of yacc.c  */
#line 2235 "src/Slice/Grammar.y"
    {
}
    break;

  case 212:
/* Line 1792 of yacc.c  */
#line 2238 "src/Slice/Grammar.y"
    {
}
    break;

  case 213:
/* Line 1792 of yacc.c  */
#line 2241 "src/Slice/Grammar.y"
    {
}
    break;

  case 214:
/* Line 1792 of yacc.c  */
#line 2244 "src/Slice/Grammar.y"
    {
}
    break;

  case 215:
/* Line 1792 of yacc.c  */
#line 2247 "src/Slice/Grammar.y"
    {
}
    break;

  case 216:
/* Line 1792 of yacc.c  */
#line 2250 "src/Slice/Grammar.y"
    {
}
    break;

  case 217:
/* Line 1792 of yacc.c  */
#line 2253 "src/Slice/Grammar.y"
    {
}
    break;

  case 218:
/* Line 1792 of yacc.c  */
#line 2256 "src/Slice/Grammar.y"
    {
}
    break;

  case 219:
/* Line 1792 of yacc.c  */
#line 2259 "src/Slice/Grammar.y"
    {
}
    break;

  case 220:
/* Line 1792 of yacc.c  */
#line 2262 "src/Slice/Grammar.y"
    {
}
    break;

  case 221:
/* Line 1792 of yacc.c  */
#line 2265 "src/Slice/Grammar.y"
    {
}
    break;

  case 222:
/* Line 1792 of yacc.c  */
#line 2268 "src/Slice/Grammar.y"
    {
}
    break;

  case 223:
/* Line 1792 of yacc.c  */
#line 2271 "src/Slice/Grammar.y"
    {
}
    break;

  case 224:
/* Line 1792 of yacc.c  */
#line 2274 "src/Slice/Grammar.y"
    {
}
    break;

  case 225:
/* Line 1792 of yacc.c  */
#line 2277 "src/Slice/Grammar.y"
    {
}
    break;


/* Line 1792 of yacc.c  */
#line 4557 "src/Slice/Grammar.cpp"
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


/* Line 2055 of yacc.c  */
#line 2281 "src/Slice/Grammar.y"

