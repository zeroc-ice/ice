/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "../IceGrid/Grammar.y" /* yacc.c:339  */


// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Parser.h>

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
using namespace Ice;
using namespace IceGrid;

void
yyerror(const char* s)
{
    parser->invalidCommand(s);
}


#line 107 "Grammar.tab.c" /* yacc.c:339  */

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
   by #include "Grammar.tab.h".  */
#ifndef YY_YY_GRAMMAR_TAB_H_INCLUDED
# define YY_YY_GRAMMAR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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
    ICE_GRID_PATCH = 275,
    ICE_GRID_SIGNAL = 276,
    ICE_GRID_STDOUT = 277,
    ICE_GRID_STDERR = 278,
    ICE_GRID_DESCRIBE = 279,
    ICE_GRID_PROPERTIES = 280,
    ICE_GRID_PROPERTY = 281,
    ICE_GRID_STATE = 282,
    ICE_GRID_PID = 283,
    ICE_GRID_ENDPOINTS = 284,
    ICE_GRID_ACTIVATION = 285,
    ICE_GRID_OBJECT = 286,
    ICE_GRID_FIND = 287,
    ICE_GRID_SHOW = 288,
    ICE_GRID_COPYING = 289,
    ICE_GRID_WARRANTY = 290,
    ICE_GRID_DIFF = 291,
    ICE_GRID_UPDATE = 292,
    ICE_GRID_INSTANTIATE = 293,
    ICE_GRID_TEMPLATE = 294,
    ICE_GRID_SERVICE = 295,
    ICE_GRID_ENABLE = 296,
    ICE_GRID_DISABLE = 297
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (void);

#endif /* !YY_YY_GRAMMAR_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 200 "Grammar.tab.c" /* yacc.c:358  */

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
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2612

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  44
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  7
/* YYNRULES -- Number of rules.  */
#define YYNRULES  172
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  367

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   297

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    43,
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
      35,    36,    37,    38,    39,    40,    41,    42
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    95,    95,    99,   106,   109,   116,   124,   128,   132,
     136,   140,   144,   148,   152,   156,   160,   164,   168,   172,
     176,   180,   184,   188,   192,   196,   200,   204,   208,   212,
     216,   220,   224,   228,   232,   236,   240,   244,   248,   252,
     256,   260,   264,   268,   272,   276,   280,   284,   288,   292,
     296,   300,   304,   308,   312,   316,   320,   324,   328,   332,
     336,   340,   344,   348,   352,   356,   360,   364,   368,   372,
     376,   380,   384,   388,   392,   396,   400,   404,   408,   412,
     416,   420,   424,   428,   432,   436,   440,   444,   448,   452,
     456,   460,   464,   468,   472,   476,   480,   484,   488,   492,
     496,   500,   504,   508,   512,   516,   520,   524,   528,   532,
     536,   540,   544,   548,   552,   556,   560,   564,   568,   579,
     583,   594,   605,   609,   613,   617,   623,   628,   633,   637,
     646,   651,   656,   662,   669,   677,   680,   683,   686,   689,
     692,   695,   698,   701,   704,   707,   710,   713,   716,   719,
     722,   725,   728,   731,   734,   737,   740,   743,   746,   749,
     752,   755,   758,   761,   764,   767,   770,   773,   776,   779,
     782,   785,   788
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ICE_GRID_HELP", "ICE_GRID_EXIT",
  "ICE_GRID_APPLICATION", "ICE_GRID_NODE", "ICE_GRID_REGISTRY",
  "ICE_GRID_SERVER", "ICE_GRID_ADAPTER", "ICE_GRID_PING", "ICE_GRID_LOAD",
  "ICE_GRID_SOCKETS", "ICE_GRID_ADD", "ICE_GRID_REMOVE", "ICE_GRID_LIST",
  "ICE_GRID_SHUTDOWN", "ICE_GRID_STRING", "ICE_GRID_START",
  "ICE_GRID_STOP", "ICE_GRID_PATCH", "ICE_GRID_SIGNAL", "ICE_GRID_STDOUT",
  "ICE_GRID_STDERR", "ICE_GRID_DESCRIBE", "ICE_GRID_PROPERTIES",
  "ICE_GRID_PROPERTY", "ICE_GRID_STATE", "ICE_GRID_PID",
  "ICE_GRID_ENDPOINTS", "ICE_GRID_ACTIVATION", "ICE_GRID_OBJECT",
  "ICE_GRID_FIND", "ICE_GRID_SHOW", "ICE_GRID_COPYING",
  "ICE_GRID_WARRANTY", "ICE_GRID_DIFF", "ICE_GRID_UPDATE",
  "ICE_GRID_INSTANTIATE", "ICE_GRID_TEMPLATE", "ICE_GRID_SERVICE",
  "ICE_GRID_ENABLE", "ICE_GRID_DISABLE", "';'", "$accept", "start",
  "commands", "checkInterrupted", "command", "strings", "keyword", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,    59
};
# endif

#define YYPACT_NINF -67

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-67)))

#define YYTABLE_NINF -4

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      39,    53,    56,    61,   -67,    61,    20,   179,    69,    93,
     142,  2492,   123,    95,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   117,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   146,   -67,    11,   -67,   -67,
     -67,   -67,   -67,   -67,    96,   -67,   -67,   -67,   -67,   122,
     -67,   -67,    76,   -67,   -67,   -67,   -67,   -67,   -67,  2459,
     -67,   -67,   -67,   -67,   219,   -67,   260,   300,   340,   380,
     420,   460,   500,   540,   580,   620,   660,   700,   740,   780,
     820,   860,   900,   940,   980,  1020,  1060,  1100,  1140,  1180,
    1220,  1260,  1300,  1340,  1380,  1420,  1460,  1500,  1540,   102,
    1580,  1620,  1660,  1700,  1740,    84,  1780,  1820,  1860,  1900,
    1940,    85,    90,  1980,  2020,  2060,  2100,  2140,  2180,    30,
      91,   112,   135,   -67,  2220,   120,  2260,  2459,   -67,  2299,
     124,   125,   128,   129,   131,   134,   139,  2460,  2461,  2462,
    2463,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2474,  2475,
    2476,  2477,  2478,  2479,  2480,  2481,  2483,  2484,  2485,  2486,
    2487,  2488,  2489,  2490,  2491,  2493,  2494,  2495,  2496,  2497,
    2498,  2499,  2500,  2501,  2502,  2503,  2504,  2505,  2506,  2507,
    2508,  2509,  2510,  2511,  2512,  2513,  2514,  2515,  2516,  2517,
    2518,  2519,  2520,  2521,  2522,  2523,  2524,  2340,  2380,  2525,
    2526,  2527,  2528,  2529,  2530,  2531,  2532,  2533,  2534,   -67,
    2535,  2536,  2537,  2538,  2539,  2540,  2541,  2542,  2543,  2544,
     -67,   -67,  2545,  2546,  2547,  2548,  2549,  2550,  2551,  2552,
    2553,  2554,  2555,  2556,  2557,  2420,   -67,   -67,  2558,  2459,
     -67,   -67,  2459,   -67,  2559,  2459,   -67,  2560,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,  2561,  2562,  2563,  2564,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,  2565,  2566,   -67,   -67,   -67,   -67,  2567,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,     6,     0,     1,     0,     0,     0,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,     0,   148,   149,   150,   151,   172,   171,   152,   155,
     156,   153,   154,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   129,     5,     0,
       4,   128,     0,   135,   136,   137,   138,   139,   140,   134,
     159,   161,   168,     7,     0,     8,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,     0,
     134,   134,   134,   134,   134,     0,   134,   134,   134,   134,
     134,     0,     0,   134,   134,   134,   134,   134,   134,     0,
       0,     0,     0,   123,   134,     0,   134,   134,   117,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   134,   134,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   127,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     115,   116,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   134,   126,   124,     0,   134,
     130,   122,   134,   131,     0,   134,   118,     0,    10,     9,
      12,    11,    22,    21,    20,    19,    18,    17,    14,    13,
      16,    15,    34,    33,    36,    35,    38,    37,    42,    41,
      40,    39,    32,    31,    44,    43,    48,    47,    52,    51,
      50,    49,    46,    45,    54,    53,    56,    55,    84,    83,
      60,    59,    62,    61,    64,    63,    66,    65,    68,    67,
      70,    69,    58,    57,    76,    75,    78,    77,    72,    71,
      74,    73,    86,    85,    27,     0,     0,     0,     0,    80,
      79,    82,    81,   102,   101,   104,   103,   100,    99,   106,
     105,   108,   107,   112,   111,   114,   113,   110,   109,    98,
      97,    88,    87,    90,    89,    92,    91,    94,    93,    96,
      95,    30,     0,     0,   125,   132,   133,   119,     0,   120,
      24,    23,    26,    25,    29,    28,   121
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -67,   -67,   -67,  2609,  2607,   -66,    52
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,     3,    48,   125,   126
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     131,   133,   135,   137,   139,   141,   143,   145,   147,   149,
     151,   153,   155,   157,   159,   161,   163,   165,   167,   169,
     171,   173,   175,   177,   179,   181,   183,   185,   187,   189,
     191,   193,   195,   234,   200,   202,   204,   206,   208,    -3,
     211,   213,   215,   217,   219,   111,   112,   223,   225,   227,
     229,   231,   233,     4,   235,    49,    -2,    49,   240,    64,
     243,   244,     6,    51,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   196,    66,    67,    68,   102,
     103,   113,    65,    69,   114,   115,   129,    70,   105,   123,
     116,   117,   118,   120,   104,   121,   197,   209,   220,    71,
      72,   316,   318,   221,   236,   119,   238,    85,    86,   122,
     198,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    73,    74,    75,   237,    98,    76,    77,   106,
     107,   108,    99,   241,   100,   101,    78,   248,   249,   353,
     109,   250,   251,   355,   252,    79,   356,   253,   110,   358,
      52,   247,   254,    53,    54,    55,    56,    57,    58,    14,
      15,    16,    17,    18,    19,    20,    59,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      60,    36,    61,    38,    39,    40,    41,    42,    43,    62,
      45,    46,    63,    53,    54,    55,    56,    57,    58,    14,
      15,    16,    17,    18,    19,    20,   127,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      60,    36,    61,    38,    39,    40,    41,    42,    43,    62,
      45,    46,   128,   130,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   132,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   134,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   136,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   138,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   140,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   142,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   144,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   146,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   148,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   150,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   152,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   154,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   156,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   158,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   160,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   162,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   164,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   166,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   168,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   170,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   172,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   174,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   176,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   178,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   180,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   182,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   184,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   186,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   188,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   190,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   192,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   194,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   199,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   201,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   203,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   205,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   207,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   210,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   212,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   214,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   216,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   218,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   222,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   224,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   226,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   228,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   230,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   232,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   239,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   242,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,    53,    54,    55,    56,    57,    58,    14,
      15,    16,    17,    18,    19,    20,   245,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      60,    36,    61,    38,    39,    40,    41,    42,    43,    62,
      45,    46,   246,   315,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   317,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,   352,    53,    54,    55,    56,    57,    58,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    60,    36,    61,    38,    39,    40,    41,    42,    43,
      62,    45,    46,    53,    54,    55,    56,    57,    58,    14,
      15,    16,    17,    18,    19,    20,   124,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      60,    36,    61,    38,    39,    40,    41,    42,    43,    62,
      45,    46,    80,   255,   256,   257,   258,    81,    82,   259,
     260,   261,   262,   263,   264,   265,    83,   266,   267,   268,
     269,   270,   271,   272,   273,    84,   274,   275,   276,   277,
     278,   279,   280,   281,   282,     0,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   354,   357,   359,   360,   361,   362,   363,   364,   365,
     366,     5,    50
};

static const yytype_int16 yycheck[] =
{
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,     3,   100,   101,   102,   103,   104,     0,
     106,   107,   108,   109,   110,    34,    35,   113,   114,   115,
     116,   117,   118,     0,    24,     3,     0,     5,   124,     7,
     126,   127,     1,    43,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,     3,    13,    14,    15,    14,
      15,    15,    43,    20,    18,    19,    64,    24,     1,    43,
      24,    25,    26,     1,    29,     3,    24,    43,    43,    36,
      37,   197,   198,    43,    43,    39,     1,    14,    15,    17,
      38,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    10,    11,    12,    43,    33,    15,    16,    13,
      14,    15,    39,    43,    41,    42,    24,    43,    43,   235,
      24,    43,    43,   239,    43,    33,   242,    43,    32,   245,
       1,   129,    43,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    10,    43,    43,    43,    43,    15,    16,    43,
      43,    43,    43,    43,    43,    43,    24,    43,    43,    43,
      43,    43,    43,    43,    43,    33,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    -1,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,     2,     5
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    45,    46,    47,     0,    47,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    48,    50,
      48,    43,     1,     4,     5,     6,     7,     8,     9,    17,
      31,    33,    40,    43,    50,    43,    13,    14,    15,    20,
      24,    36,    37,    10,    11,    12,    15,    16,    24,    33,
      10,    15,    16,    24,    33,    14,    15,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    33,    39,
      41,    42,    14,    15,    29,     1,    13,    14,    15,    24,
      32,    34,    35,    15,    18,    19,    24,    25,    26,    39,
       1,     3,    17,    43,    17,    49,    50,    17,    43,    50,
       3,    49,     3,    49,     3,    49,     3,    49,     3,    49,
       3,    49,     3,    49,     3,    49,     3,    49,     3,    49,
       3,    49,     3,    49,     3,    49,     3,    49,     3,    49,
       3,    49,     3,    49,     3,    49,     3,    49,     3,    49,
       3,    49,     3,    49,     3,    49,     3,    49,     3,    49,
       3,    49,     3,    49,     3,    49,     3,    49,     3,    49,
       3,    49,     3,    49,     3,    49,     3,    24,    38,     3,
      49,     3,    49,     3,    49,     3,    49,     3,    49,    43,
       3,    49,     3,    49,     3,    49,     3,    49,     3,    49,
      43,    43,     3,    49,     3,    49,     3,    49,     3,    49,
       3,    49,     3,    49,     3,    24,    43,    43,     1,     3,
      49,    43,     3,    49,    49,    17,    43,    50,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,     3,    49,     3,    49,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,     3,    49,    43,    49,    49,    43,    49,    43,
      43,    43,    43,    43,    43,    43,    43
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    44,    45,    45,    46,    46,    47,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      48,    48,    48,    48,    48,    48,    48,    48,    48,    48,
      49,    49,    49,    49,    49,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     3,     2,     0,     2,     2,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     5,     5,     5,     5,     4,     5,     5,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     3,     3,     3,     4,     5,
       5,     6,     4,     3,     3,     4,     3,     3,     2,     1,
       2,     2,     3,     3,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1
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




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

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
      yychar = yylex (&yylval);
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
#line 96 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1971 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 99 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1978 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 107 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1985 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 110 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1992 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 116 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->checkInterrupted();
}
#line 2000 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 125 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage();
}
#line 2008 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 129 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    return 0;
}
#line 2016 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 133 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->addApplication((yyvsp[-1]));
}
#line 2024 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 137 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("application", "add");
}
#line 2032 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 141 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->removeApplication((yyvsp[-1]));
}
#line 2040 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 145 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("application", "remove");
}
#line 2048 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 149 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->diffApplication((yyvsp[-1]));
}
#line 2056 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 153 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("application", "diff");
}
#line 2064 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 157 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->updateApplication((yyvsp[-1]));
}
#line 2072 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 161 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("application", "update");
}
#line 2080 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 165 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->describeApplication((yyvsp[-1]));
}
#line 2088 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 169 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("application", "describe");
}
#line 2096 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 173 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->patchApplication((yyvsp[-1]));
}
#line 2104 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 177 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("application", "patch");
}
#line 2112 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 181 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->listAllApplications((yyvsp[-1]));
}
#line 2120 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 185 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("application", "list");
}
#line 2128 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 189 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->describeServerTemplate((yyvsp[-1]));
}
#line 2136 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 193 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server template", "describe");
}
#line 2144 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 197 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->instantiateServerTemplate((yyvsp[-1]));
}
#line 2152 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 201 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server template", "instantiate");
}
#line 2160 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 205 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server template");
}
#line 2168 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 209 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->describeServiceTemplate((yyvsp[-1]));
}
#line 2176 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 213 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("service template", "describe");
}
#line 2184 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 217 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("service template");
}
#line 2192 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 221 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->describeNode((yyvsp[-1]));
}
#line 2200 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 225 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("node", "describe");
}
#line 2208 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 229 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->pingNode((yyvsp[-1]));
}
#line 2216 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 233 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("node", "ping");
}
#line 2224 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 237 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->printLoadNode((yyvsp[-1]));
}
#line 2232 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 241 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("node", "load");
}
#line 2240 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 245 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->printNodeProcessorSockets((yyvsp[-1]));
}
#line 2248 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 249 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("node", "sockets");
}
#line 2256 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 253 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->shutdownNode((yyvsp[-1]));
}
#line 2264 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 257 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("node", "shutdown");
}
#line 2272 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 261 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->listAllNodes((yyvsp[-1]));
}
#line 2280 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 265 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("node", "list");
}
#line 2288 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 269 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->show("node", (yyvsp[-1]));
}
#line 2296 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 273 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("node", "show");
}
#line 2304 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 277 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->describeRegistry((yyvsp[-1]));
}
#line 2312 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 281 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("registry", "describe");
}
#line 2320 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 285 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->pingRegistry((yyvsp[-1]));
}
#line 2328 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 289 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("registry", "ping");
}
#line 2336 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 293 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->shutdownRegistry((yyvsp[-1]));
}
#line 2344 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 297 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("registry", "shutdown");
}
#line 2352 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 301 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->listAllRegistries((yyvsp[-1]));
}
#line 2360 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 305 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("registry", "list");
}
#line 2368 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 309 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->show("registry", (yyvsp[-1]));
}
#line 2376 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 313 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("registry", "show");
}
#line 2384 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 317 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->removeServer((yyvsp[-1]));
}
#line 2392 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 321 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "remove");
}
#line 2400 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 325 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->describeServer((yyvsp[-1]));
}
#line 2408 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 329 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "describe");
}
#line 2416 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 333 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->startServer((yyvsp[-1]));
}
#line 2424 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 337 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "start");
}
#line 2432 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 341 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->stopServer((yyvsp[-1]));
}
#line 2440 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 345 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "stop");
}
#line 2448 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 349 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->patchServer((yyvsp[-1]));
}
#line 2456 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 353 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "patch");
}
#line 2464 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 357 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->signalServer((yyvsp[-1]));
}
#line 2472 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 361 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "signal");
}
#line 2480 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 365 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->writeMessage((yyvsp[-1]), 1);
}
#line 2488 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 369 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "stdout");
}
#line 2496 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 373 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->writeMessage((yyvsp[-1]), 2);
}
#line 2504 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 377 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "stderr");
}
#line 2512 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 381 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->stateServer((yyvsp[-1]));
}
#line 2520 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 385 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "start");
}
#line 2528 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 389 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->pidServer((yyvsp[-1]));
}
#line 2536 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 393 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "pid");
}
#line 2544 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 397 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->propertiesServer((yyvsp[-1]), false);
}
#line 2552 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 401 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "properties");
}
#line 2560 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 405 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->propertiesServer((yyvsp[-1]), true);
}
#line 2568 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 409 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "property");
}
#line 2576 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 413 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->enableServer((yyvsp[-1]), true);
}
#line 2584 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 417 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "enable");
}
#line 2592 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 421 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->enableServer((yyvsp[-1]), false);
}
#line 2600 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 425 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "disable");
}
#line 2608 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 429 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->listAllServers((yyvsp[-1]));
}
#line 2616 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 433 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "list");
}
#line 2624 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 437 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->show("server", (yyvsp[-1]));
}
#line 2632 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 441 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("server", "show");
}
#line 2640 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 445 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->startService((yyvsp[-1]));
}
#line 2648 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 449 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("service", "start");
}
#line 2656 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 453 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->stopService((yyvsp[-1]));
}
#line 2664 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 457 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("service", "stop");
}
#line 2672 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 461 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->describeService((yyvsp[-1]));
}
#line 2680 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 465 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("service", "describe");
}
#line 2688 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 469 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->propertiesService((yyvsp[-1]), false);
}
#line 2696 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 473 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("service", "properties");
}
#line 2704 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 477 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->propertiesService((yyvsp[-1]), true);
}
#line 2712 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 481 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("service", "property");
}
#line 2720 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 485 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->listServices((yyvsp[-1]));
}
#line 2728 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 489 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("service", "list");
}
#line 2736 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 493 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->endpointsAdapter((yyvsp[-1]));
}
#line 2744 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 497 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("adapter", "endpoints");
}
#line 2752 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 501 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->removeAdapter((yyvsp[-1]));
}
#line 2760 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 505 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("adapter", "remove");
}
#line 2768 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 509 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->listAllAdapters((yyvsp[-1]));
}
#line 2776 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 513 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("adapter", "list");
}
#line 2784 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 517 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->addObject((yyvsp[-1]));
}
#line 2792 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 521 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("object", "add");
}
#line 2800 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 525 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->removeObject((yyvsp[-1]));
}
#line 2808 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 529 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("object", "remove");
}
#line 2816 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 533 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->findObject((yyvsp[-1]));
}
#line 2824 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 537 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("object", "find");
}
#line 2832 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 541 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->listObject((yyvsp[-1]));
}
#line 2840 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 545 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("object", "list");
}
#line 2848 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 549 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->describeObject((yyvsp[-1]));
}
#line 2856 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 553 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage("object", "describe");
}
#line 2864 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 557 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->showCopying();
}
#line 2872 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 561 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->showWarranty();
}
#line 2880 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 565 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage((yyvsp[-1]).front());
}
#line 2888 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 569 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    if(((yyvsp[-2]).front() == "server" || (yyvsp[-2]).front() == "service") && (yyvsp[-1]).front() == "template")
    {
        parser->usage((yyvsp[-2]).front() + " " + (yyvsp[-1]).front());
    }
    else
    {
        parser->usage((yyvsp[-2]).front(), (yyvsp[-1]).front());
    }
}
#line 2903 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 580 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage((yyvsp[-3]).front(), (yyvsp[-2]).front());
}
#line 2911 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 584 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    if(((yyvsp[-3]).front() == "server" || (yyvsp[-3]).front() == "service") && (yyvsp[-2]).front() == "template")
    {
        parser->usage((yyvsp[-3]).front() + " " + (yyvsp[-2]).front(), (yyvsp[-1]).front());
    }
    else
    {
        parser->usage((yyvsp[-3]).front(), (yyvsp[-2]).front());
    }
}
#line 2926 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 595 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    if(((yyvsp[-4]).front() == "server" || (yyvsp[-4]).front() == "service") && (yyvsp[-3]).front() == "template")
    {
        parser->usage((yyvsp[-4]).front() + " " + (yyvsp[-3]).front(), (yyvsp[-2]).front());
    }
    else
    {
        parser->usage((yyvsp[-4]).front(), (yyvsp[-3]).front());
    }
}
#line 2941 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 606 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage((yyvsp[-2]).front());
}
#line 2949 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 610 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage();
}
#line 2957 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 614 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->usage((yyvsp[-2]).front());
}
#line 2965 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 618 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    (yyvsp[-3]).push_back((yyvsp[-2]).front());
    parser->invalidCommand((yyvsp[-3]));
    yyerrok;
}
#line 2975 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 624 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->invalidCommand((yyvsp[-2]));
    yyerrok;
}
#line 2984 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 629 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    parser->invalidCommand((yyvsp[-2]));
    yyerrok;
}
#line 2993 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 634 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    yyerrok;
}
#line 3001 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 638 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3008 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 647 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
    (yyval).push_front((yyvsp[-1]).front());
}
#line 3017 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 652 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
    (yyval).push_front((yyvsp[-1]).front());
}
#line 3026 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 657 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[-1]);
    (yyval).push_front("help");
    (yyval).push_front((yyvsp[-2]).front());
}
#line 3036 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 663 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[-1]);
    (yyval).push_front("help");
    (yyval).push_front((yyvsp[-2]).front());
}
#line 3046 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 669 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = YYSTYPE();
}
#line 3054 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 678 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3061 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 681 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3068 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 684 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3075 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 687 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3082 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 690 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3089 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 693 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3096 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 696 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3103 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 699 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3110 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 702 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3117 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 705 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3124 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 708 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3131 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 711 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3138 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 714 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3145 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 717 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3152 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 720 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3159 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 723 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3166 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 726 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3173 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 729 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3180 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 732 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3187 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 735 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3194 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 738 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3201 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 741 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3208 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 744 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3215 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 747 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3222 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 750 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3229 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 753 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3236 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 756 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3243 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 759 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3250 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 762 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3257 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 765 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3264 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 768 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3271 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 771 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3278 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 774 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3285 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 777 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3292 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 780 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3299 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 783 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3306 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 786 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3313 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 789 "../IceGrid/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3320 "Grammar.tab.c" /* yacc.c:1646  */
    break;


#line 3324 "Grammar.tab.c" /* yacc.c:1646  */
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
