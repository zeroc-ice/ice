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
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */
#line 1 "src/Slice/Grammar.y" /* yacc.c:316  */


//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Included first to get 'TokenContext' which we need to define YYLTYPE before flex does.
#include <Slice/GrammarUtil.h>

#line 30 "src/Slice/Grammar.y" /* yacc.c:316  */


// Defines the rule bison uses to reduce token locations. Bison asks that the macro should
// be one-line, and treatable as a single statement when followed by a semi-colon.
// `N` is the number of tokens that are being combined, and (Cur) is their combined location.
#define YYLLOC_DEFAULT(Cur, Rhs, N)                               \
do                                                                \
    if(N == 1)                                                    \
    {                                                             \
        (Cur) = (YYRHSLOC((Rhs), 1));                             \
    }                                                             \
    else                                                          \
    {                                                             \
        if(N)                                                     \
        {                                                         \
            (Cur).firstLine = (YYRHSLOC((Rhs), 1)).firstLine;     \
            (Cur).firstColumn = (YYRHSLOC((Rhs), 1)).firstColumn; \
        }                                                         \
        else                                                      \
        {                                                         \
            (Cur).firstLine = (YYRHSLOC((Rhs), 0)).lastLine;      \
            (Cur).firstColumn = (YYRHSLOC((Rhs), 0)).lastColumn;  \
        }                                                         \
        (Cur).filename = (YYRHSLOC((Rhs), N)).filename;           \
        (Cur).lastLine = (YYRHSLOC((Rhs), N)).lastLine;           \
        (Cur).lastColumn = (YYRHSLOC((Rhs), N)).lastColumn;       \
    }                                                             \
while(0)


#line 104 "src/Slice/Grammar.cpp" /* yacc.c:316  */

/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yydebug         slice_debug
#define yynerrs         slice_nerrs


/* Copy the first part of user declarations.  */
#line 69 "src/Slice/Grammar.y" /* yacc.c:339  */


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
// warning C4127: conditional expression is constant
#   pragma warning(disable:4127)
#endif

// Avoid old style cast warnings in generated grammar
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

// Avoid clang conversion warnings
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wconversion"
#   pragma clang diagnostic ignored "-Wsign-conversion"
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


#line 164 "src/Slice/Grammar.cpp" /* yacc.c:339  */

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
   by #include "Grammar.hpp".  */
#ifndef YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED
# define YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int slice_debug;
#endif
/* "%code requires" blocks.  */
#line 12 "src/Slice/Grammar.y" /* yacc.c:355  */


// Define a custom location type for storing the location (and filename) of tokens.
#define YYLTYPE Slice::TokenContext

// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
#define YYMAXDEPTH  10000
#define YYINITDEPTH YYMAXDEPTH

// Newer bison versions allow to disable stack resizing by defining yyoverflow.
#define yyoverflow(a, b, c, d, e, f, g, h) yyerror(a)


#line 212 "src/Slice/Grammar.cpp" /* yacc.c:355  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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
    ICE_CONST = 280,
    ICE_FALSE = 281,
    ICE_TRUE = 282,
    ICE_IDEMPOTENT = 283,
    ICE_TAG = 284,
    ICE_OPTIONAL = 285,
    ICE_VALUE = 286,
    ICE_STRING_LITERAL = 287,
    ICE_INTEGER_LITERAL = 288,
    ICE_FLOATING_POINT_LITERAL = 289,
    ICE_IDENTIFIER = 290,
    ICE_SCOPED_IDENTIFIER = 291,
    ICE_METADATA_OPEN = 292,
    ICE_METADATA_CLOSE = 293,
    ICE_GLOBAL_METADATA_OPEN = 294,
    ICE_GLOBAL_METADATA_IGNORE = 295,
    ICE_GLOBAL_METADATA_CLOSE = 296,
    ICE_IDENT_OPEN = 297,
    ICE_KEYWORD_OPEN = 298,
    ICE_TAG_OPEN = 299,
    ICE_OPTIONAL_OPEN = 300,
    BAD_CHAR = 301
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef Slice::GrammarBasePtr YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int slice_parse (void);

#endif /* !YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 295 "src/Slice/Grammar.cpp" /* yacc.c:358  */
/* Unqualified %code blocks.  */
#line 61 "src/Slice/Grammar.y" /* yacc.c:359  */


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 305 "src/Slice/Grammar.cpp" /* yacc.c:359  */

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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   938

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  58
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  85
/* YYNRULES -- Number of rules.  */
#define YYNRULES  241
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  351

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   301

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
       2,    50,    57,     2,    53,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    51,    47,
      54,    52,    55,    56,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,     2,    49,     2,     2,     2,     2,
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
      45,    46
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   183,   183,   191,   194,   202,   206,   216,   220,   229,
     237,   246,   255,   254,   260,   259,   264,   269,   268,   274,
     273,   278,   283,   282,   288,   287,   292,   297,   296,   302,
     301,   306,   311,   310,   316,   315,   320,   325,   324,   329,
     334,   333,   339,   338,   343,   347,   357,   356,   389,   393,
     404,   415,   414,   440,   448,   457,   466,   469,   473,   481,
     494,   512,   603,   609,   620,   639,   731,   738,   750,   765,
     775,   788,   794,   798,   809,   820,   819,   860,   869,   872,
     876,   884,   890,   894,   905,   930,  1032,  1044,  1057,  1056,
    1095,  1129,  1138,  1141,  1149,  1153,  1162,  1171,  1174,  1178,
    1186,  1208,  1235,  1257,  1283,  1292,  1303,  1312,  1321,  1330,
    1339,  1349,  1363,  1376,  1384,  1390,  1400,  1424,  1449,  1473,
    1504,  1503,  1526,  1525,  1548,  1549,  1555,  1559,  1570,  1584,
    1583,  1617,  1652,  1687,  1692,  1702,  1706,  1715,  1724,  1727,
    1731,  1739,  1745,  1752,  1764,  1776,  1787,  1795,  1809,  1819,
    1835,  1839,  1851,  1850,  1882,  1881,  1899,  1905,  1913,  1925,
    1945,  1952,  1962,  1966,  2005,  2011,  2022,  2025,  2041,  2057,
    2069,  2081,  2092,  2108,  2112,  2121,  2124,  2132,  2136,  2140,
    2144,  2148,  2152,  2156,  2160,  2164,  2168,  2172,  2176,  2180,
    2184,  2188,  2192,  2196,  2200,  2204,  2209,  2213,  2217,  2236,
    2269,  2297,  2303,  2311,  2318,  2330,  2339,  2348,  2388,  2395,
    2402,  2414,  2423,  2437,  2440,  2443,  2446,  2449,  2452,  2455,
    2458,  2461,  2464,  2467,  2470,  2473,  2476,  2479,  2482,  2485,
    2488,  2491,  2494,  2497,  2500,  2503,  2506,  2509,  2512,  2515,
    2518,  2521
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
  "ICE_CONST", "ICE_FALSE", "ICE_TRUE", "ICE_IDEMPOTENT", "ICE_TAG",
  "ICE_OPTIONAL", "ICE_VALUE", "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENTIFIER", "ICE_SCOPED_IDENTIFIER",
  "ICE_METADATA_OPEN", "ICE_METADATA_CLOSE", "ICE_GLOBAL_METADATA_OPEN",
  "ICE_GLOBAL_METADATA_IGNORE", "ICE_GLOBAL_METADATA_CLOSE",
  "ICE_IDENT_OPEN", "ICE_KEYWORD_OPEN", "ICE_TAG_OPEN",
  "ICE_OPTIONAL_OPEN", "BAD_CHAR", "';'", "'{'", "'}'", "')'", "':'",
  "'='", "','", "'<'", "'>'", "'?'", "'*'", "$accept", "start",
  "opt_semicolon", "global_meta_data", "meta_data", "definitions",
  "definition", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8",
  "$@9", "$@10", "$@11", "$@12", "$@13", "module_def", "@14",
  "exception_id", "exception_decl", "exception_def", "@15",
  "exception_extends", "exception_exports", "type_id", "tag", "optional",
  "tagged_type_id", "exception_export", "struct_id", "struct_decl",
  "struct_def", "@16", "struct_exports", "struct_export", "class_name",
  "class_id", "class_decl", "class_def", "@17", "class_extends", "extends",
  "implements", "class_exports", "data_member", "struct_data_member",
  "return_type", "operation_preamble", "operation", "@18", "@19",
  "class_export", "interface_id", "interface_decl", "interface_def", "@20",
  "interface_list", "interface_extends", "interface_exports",
  "interface_export", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_id", "enum_def", "@21", "@22", "enumerator_list",
  "enumerator", "enumerator_initializer", "out_qualifier", "parameters",
  "throws", "scoped_name", "type", "string_literal", "string_list",
  "const_initializer", "const_def", "keyword", YY_NULLPTR
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
     295,   296,   297,   298,   299,   300,   301,    59,   123,   125,
      41,    58,    61,    44,    60,    62,    63,    42
};
# endif

#define YYPACT_NINF -302

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-302)))

#define YYTABLE_NINF -167

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -302,    37,    25,  -302,   -14,   -14,   -14,  -302,   160,   -14,
    -302,   -15,    33,    39,    -6,    11,   485,   559,   592,   625,
      17,    23,   658,    14,  -302,  -302,    19,    26,  -302,     0,
      57,  -302,    15,    18,    65,  -302,    24,    73,  -302,    80,
      98,  -302,  -302,   100,  -302,  -302,   -14,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,     9,  -302,  -302,  -302,  -302,  -302,  -302,  -302,    14,
      14,  -302,    68,  -302,   902,   112,  -302,  -302,  -302,    75,
     124,   112,    74,   126,   112,   147,    75,   127,   112,   148,
    -302,   128,   112,   130,   139,   142,   112,   146,  -302,   149,
     145,  -302,  -302,   154,   902,   902,   691,   143,   144,   151,
     157,   164,   165,   166,   167,    62,   168,    69,   -13,  -302,
    -302,   179,  -302,  -302,  -302,   360,  -302,  -302,   148,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,   155,   180,  -302,  -302,
    -302,  -302,   691,  -302,  -302,  -302,  -302,  -302,   174,   177,
     181,   153,   178,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,   182,   183,   397,
     185,   872,   188,  -302,   191,   148,   113,   201,   152,   724,
      14,    61,  -302,   691,   183,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,   205,   872,   204,   360,  -302,  -302,    43,    48,
     206,   902,   902,   212,  -302,   757,  -302,   323,  -302,   216,
     811,   215,  -302,  -302,  -302,  -302,   902,  -302,  -302,  -302,
    -302,  -302,   397,  -302,   902,   902,   213,   220,  -302,   757,
    -302,  -302,   221,  -302,   222,   223,  -302,   224,   183,   227,
     241,   228,   360,  -302,  -302,   230,   811,   232,   113,  -302,
     842,   902,   902,   109,   225,  -302,   236,  -302,  -302,   229,
    -302,  -302,  -302,   183,   397,  -302,  -302,  -302,  -302,  -302,
    -302,   183,   183,  -302,   323,   902,   902,  -302,  -302,   238,
     444,  -302,  -302,   111,  -302,  -302,  -302,  -302,   237,  -302,
      14,    -3,   113,   790,  -302,  -302,  -302,  -302,  -302,   241,
     241,   323,  -302,  -302,  -302,   872,  -302,   275,  -302,  -302,
    -302,  -302,   274,  -302,   757,   274,    14,   525,  -302,  -302,
    -302,   872,  -302,   240,  -302,  -302,  -302,   757,   525,  -302,
    -302
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      11,     0,     8,     1,     0,     0,     0,     9,     0,   202,
     204,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   154,     8,    10,    12,    50,    26,    27,    74,
      31,    32,    87,    91,    16,    17,   128,    21,    22,    36,
      39,   152,    40,    44,   201,     7,     0,     5,     6,    45,
      46,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
      82,     0,    83,   126,   127,    48,    49,    72,    73,     8,
       8,   150,     0,   151,     0,     4,    92,    93,    51,     0,
       0,     4,     0,     0,     4,    95,     0,     0,     4,     0,
     129,     0,     4,     0,     0,     0,     4,     0,   203,     0,
       0,   175,   176,     0,     0,     0,   161,   177,   179,   181,
     183,   185,   187,   189,   191,   193,   196,   198,     0,     3,
      13,     0,    53,    25,    28,     0,    30,    33,     0,    88,
      90,    15,    18,   133,   134,   135,   132,     0,    20,    23,
      35,    38,   161,    41,    43,    11,    84,    85,     0,     0,
     158,     0,   157,   160,   178,   180,   182,   184,   186,   188,
     190,   192,   194,   195,   197,   200,   199,     0,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,     8,     0,
       8,     0,   155,   161,     0,   209,   210,   208,   205,   206,
     207,   212,     0,     0,     0,     0,    63,    67,     0,     0,
     104,     0,     0,    79,    81,   111,    76,     0,   131,     0,
       0,     0,   153,    47,   146,   147,     0,   162,   159,   163,
     156,   211,     0,    70,     0,     0,   100,    57,    71,   103,
      52,    78,     0,    62,     0,     0,    66,     0,     0,   106,
       0,   108,     0,    59,   110,     0,     0,     0,     0,   115,
       0,     0,     0,     0,     0,   141,   139,   114,   130,     0,
      56,    68,    69,     0,     0,   102,    60,    61,    64,    65,
     105,     0,     0,    77,     0,     0,     0,   124,   125,    98,
     103,    89,   138,     0,   112,   113,   116,   118,     0,   164,
       8,     0,     0,     0,   101,    55,   107,   109,    97,   112,
     113,     0,   117,   119,   122,     0,   120,   165,   137,   148,
     149,    96,   174,   167,   171,   174,     8,     0,   123,   169,
     121,     0,   173,   143,   144,   145,   168,   172,     0,   170,
     142
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -302,  -302,     1,  -302,    -2,   125,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -230,  -189,  -181,
    -171,  -301,  -302,  -302,  -302,  -302,  -302,  -202,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,    67,  -302,  -278,    28,  -302,
      21,  -302,    29,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -134,  -302,  -259,  -302,   -52,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -145,  -302,  -302,   -30,  -302,   -37,   -80,   -90,
       6,   150,  -201,  -302,   -11
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   140,     7,   191,     2,    24,    95,   107,   108,
     111,   112,   100,   101,   103,   104,   113,   114,   116,   117,
      25,   119,    26,    27,    28,   141,    98,   214,   243,   244,
     245,   246,   247,    29,    30,    31,   102,   192,   223,    32,
      33,    34,    35,   194,   105,    99,   149,   267,   248,   224,
     273,   274,   275,   335,   332,   299,    36,    37,    38,   157,
     155,   110,   231,   276,   342,   343,    39,    40,    41,    42,
     115,    92,   171,   172,   238,   310,   311,   338,   137,   260,
      10,    11,   211,    43,   173
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       8,   123,   220,   241,   138,    82,    84,    86,    88,   302,
     221,    93,   280,   251,   193,    44,   318,   197,     9,   142,
     222,    94,   187,    45,   333,    -2,   150,   -86,   -86,   156,
      96,    96,   259,   261,   168,   169,    96,     3,    46,   188,
     346,    49,   120,   331,   121,   122,    50,   326,   -75,   271,
     327,     4,   118,   328,   315,   281,   282,   290,   240,   272,
     293,   228,     4,   -86,     5,     6,   -86,   -54,   156,    97,
      97,    89,  -136,   -24,    47,    97,   252,    90,   121,   122,
      48,   255,   314,   121,   122,   295,    46,   124,   125,   271,
     316,   317,    46,   253,   237,   296,   121,   122,   256,   272,
     106,   225,   144,   109,   -29,   147,   281,   282,   210,   152,
     121,   122,   -14,   159,   229,   156,   126,   163,   182,   183,
     -19,   239,   145,   249,   210,   185,   186,   -34,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,   254,   257,
     277,    -8,    -8,    -8,    -8,   -37,   279,   -42,    -8,    -8,
       4,   306,   307,   322,   323,    12,    13,    -8,    -8,   139,
     148,    14,  -140,    15,    16,    17,    18,    19,    20,    21,
      22,   143,   153,   146,   151,   158,   300,   160,   210,   154,
     277,   304,   305,   121,   122,    23,   161,   213,   235,     4,
     162,     5,     6,   164,   230,   166,     8,   165,   236,   174,
     175,   233,   202,   210,   167,   319,   320,   176,   195,   205,
     206,   210,   210,   177,   264,   207,   208,   209,   121,   122,
     178,   179,   180,   181,   184,   266,   308,   189,   196,   199,
     200,   203,   215,   201,   204,   334,   309,   226,   285,   227,
     213,  -165,  -165,  -165,  -165,  -165,  -165,  -165,  -165,  -165,
     232,   347,   242,   250,  -165,  -165,  -165,   344,   258,   262,
    -165,  -165,  -165,   268,   278,   283,   230,   284,   344,  -165,
    -165,   286,   287,   288,   289,  -166,   263,   294,  -166,   291,
     292,   301,   213,   312,   313,   321,   309,   324,   337,   285,
     198,   303,   266,   348,   297,   298,   350,   336,   340,     0,
       0,     0,   330,     0,     0,     0,     0,     0,   325,     0,
     230,     0,     0,     0,     0,     0,     0,     0,     0,   266,
       0,     0,     0,   339,   265,     0,   345,     0,     0,     0,
       0,     0,     0,     0,   341,     0,   349,   345,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,     0,     0,
       0,    -8,    -8,    -8,    -8,     0,     0,     0,    -8,    -8,
       4,   190,     0,     0,     0,     0,     0,    -8,    -8,     0,
       0,     0,   -99,     0,     0,     0,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,     0,     0,     0,     0,    -8,
      -8,    -8,     0,     0,     0,    -8,    -8,     4,   212,     0,
       0,     0,     0,     0,    -8,    -8,     0,     0,     0,   -80,
       0,     0,     0,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,     0,     0,     0,     0,    -8,    -8,    -8,     0,
       0,     0,    -8,    -8,     4,     0,     0,     0,     0,     0,
       0,    -8,    -8,     0,     0,     0,   -58,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,     0,     0,     0,   263,
       0,     0,     0,     0,     0,     0,  -114,  -114,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,     0,     0,     0,     0,    81,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
     121,   122,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,     0,     0,     0,    83,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     0,     0,     0,    85,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
      87,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
       0,     0,     0,    91,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,   170,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,     0,     0,     0,   234,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     0,
       0,     0,   263,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,     0,     0,     0,   329,   269,   127,   128,   129,
     130,   131,   132,   133,   134,   135,     0,     0,     0,   270,
     216,   217,   136,     0,     0,     0,   121,   122,     0,     0,
       0,     0,     0,     0,     0,   218,   219,   269,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     0,     0,     0,
       0,   216,   217,   136,     0,     0,     0,   121,   122,     0,
       0,     0,     0,     0,     0,     0,   218,   219,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     0,     0,     0,
       0,   216,   217,   136,     0,     0,     0,   121,   122,     0,
       0,     0,     0,     0,     0,     0,   218,   219,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     0,     0,     0,
       0,     0,     0,   136,     0,     0,     0,   121,   122
};

static const yytype_int16 yycheck[] =
{
       2,    81,   191,   204,    94,    16,    17,    18,    19,   268,
     191,    22,   242,   215,   148,     9,   294,   162,    32,    99,
     191,    23,    35,    38,   325,     0,   106,    12,    13,   109,
      12,    12,   221,   222,   124,   125,    12,     0,    53,    52,
     341,    47,    33,   321,    35,    36,    35,    50,    48,   230,
      53,    37,    46,   312,   284,   244,   245,   258,   203,   230,
     262,   195,    37,    48,    39,    40,    51,    48,   148,    51,
      51,    54,    48,    47,    41,    51,    33,    54,    35,    36,
      41,    33,   283,    35,    36,   266,    53,    89,    90,   270,
     291,   292,    53,    50,    33,   266,    35,    36,    50,   270,
      33,   191,   101,    36,    47,   104,   295,   296,   188,   108,
      35,    36,    47,   112,     1,   195,    48,   116,    56,    57,
      47,   201,    48,   213,   204,    56,    57,    47,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,   218,   219,
     230,    28,    29,    30,    31,    47,   236,    47,    35,    36,
      37,    42,    43,    42,    43,     5,     6,    44,    45,    47,
      13,     1,    49,     3,     4,     5,     6,     7,     8,     9,
      10,    47,    24,    47,    47,    47,   266,    47,   258,    31,
     270,   271,   272,    35,    36,    25,    47,   189,   199,    37,
      48,    39,    40,    47,   196,    50,   198,    48,   200,    56,
      56,    49,    49,   283,    50,   295,   296,    56,    53,    26,
      27,   291,   292,    56,   225,    32,    33,    34,    35,    36,
      56,    56,    56,    56,    56,   227,     1,    48,    48,    55,
      53,    53,    47,    52,    52,   325,    11,    49,   249,    48,
     242,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      49,   341,    47,    49,    29,    30,    31,   337,    52,    47,
      35,    36,    37,    47,    49,    52,   268,    47,   348,    44,
      45,    50,    50,    50,    50,    50,    35,    47,    53,    52,
      52,    49,   284,    47,    55,    47,    11,    50,    14,   300,
     165,   270,   294,    53,   266,   266,   348,   327,   335,    -1,
      -1,    -1,   313,    -1,    -1,    -1,    -1,    -1,   310,    -1,
     312,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   321,
      -1,    -1,    -1,   334,     1,    -1,   337,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   336,    -1,   347,   348,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    28,    29,    30,    31,    -1,    -1,    -1,    35,    36,
      37,     1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    29,
      30,    31,    -1,    -1,    -1,    35,    36,    37,     1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    29,    30,    31,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    49,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      35,    36,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    35,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    35,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      35,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    35,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    35,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    35,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    35,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    35,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    28,
      29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    36
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    59,    63,     0,    37,    39,    40,    61,    62,    32,
     138,   139,   139,   139,     1,     3,     4,     5,     6,     7,
       8,     9,    10,    25,    64,    78,    80,    81,    82,    91,
      92,    93,    97,    98,    99,   100,   114,   115,   116,   124,
     125,   126,   127,   141,   138,    38,    53,    41,    41,    47,
      35,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      35,    42,   142,    35,   142,    35,   142,    35,   142,    54,
      54,    35,   129,   142,    62,    65,    12,    51,    84,   103,
      70,    71,    94,    72,    73,   102,   103,    66,    67,   103,
     119,    68,    69,    74,    75,   128,    76,    77,   138,    79,
      33,    35,    36,   136,    62,    62,    48,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    31,   136,   137,    47,
      60,    83,   136,    47,    60,    48,    47,    60,    13,   104,
     136,    47,    60,    24,    31,   118,   136,   117,    47,    60,
      47,    47,    48,    60,    47,    48,    50,    50,   137,   137,
      35,   130,   131,   142,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    57,    56,    56,    57,    35,    52,    48,
       1,    62,    95,   118,   101,    53,    48,   130,    63,    55,
      53,    52,    49,    53,    52,    26,    27,    32,    33,    34,
     136,   140,     1,    62,    85,    47,    29,    30,    44,    45,
      86,    87,    88,    96,   107,   137,    49,    48,   118,     1,
      62,   120,    49,    49,    35,   142,    62,    33,   132,   136,
     130,   140,    47,    86,    87,    88,    89,    90,   106,   137,
      49,    95,    33,    50,   136,    33,    50,   136,    52,    86,
     137,    86,    47,    35,   142,     1,    62,   105,    47,    15,
      28,    87,    88,   108,   109,   110,   121,   137,    49,   137,
      85,    86,    86,    52,    47,   142,    50,    50,    50,    50,
     140,    52,    52,    95,    47,    87,    88,   106,   110,   113,
     137,    49,   120,   108,   137,   137,    42,    43,     1,    11,
     133,   134,    47,    55,   140,    85,   140,   140,   105,   137,
     137,    47,    42,    43,    50,    62,    50,    53,   120,    35,
     142,   105,   112,    89,   137,   111,   133,    14,   135,   142,
     135,    62,   122,   123,   136,   142,    89,   137,    53,   142,
     122
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    58,    59,    60,    60,    61,    61,    62,    62,    63,
      63,    63,    65,    64,    66,    64,    64,    67,    64,    68,
      64,    64,    69,    64,    70,    64,    64,    71,    64,    72,
      64,    64,    73,    64,    74,    64,    64,    75,    64,    64,
      76,    64,    77,    64,    64,    64,    79,    78,    80,    80,
      81,    83,    82,    84,    84,    85,    85,    85,    85,    86,
      87,    87,    87,    87,    88,    88,    88,    88,    89,    89,
      89,    90,    91,    91,    92,    94,    93,    95,    95,    95,
      95,    96,    97,    97,    98,    98,    98,    99,   101,   100,
     102,   102,   103,   103,   104,   104,   105,   105,   105,   105,
     106,   106,   106,   106,   107,   107,   107,   107,   107,   107,
     107,   107,   108,   108,   108,   108,   109,   109,   109,   109,
     111,   110,   112,   110,   113,   113,   114,   114,   115,   117,
     116,   118,   118,   118,   118,   119,   119,   120,   120,   120,
     120,   121,   122,   122,   123,   123,   124,   124,   125,   125,
     126,   126,   128,   127,   129,   127,   130,   130,   131,   131,
     131,   131,   132,   132,   133,   133,   134,   134,   134,   134,
     134,   134,   134,   135,   135,   136,   136,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   137,   137,   137,   137,   137,   137,   137,   137,   137,
     137,   138,   138,   139,   139,   140,   140,   140,   140,   140,
     140,   141,   141,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     3,     0,     2,
       3,     0,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     1,
       0,     3,     0,     3,     1,     2,     0,     6,     2,     2,
       1,     0,     6,     2,     0,     4,     3,     2,     0,     2,
       3,     3,     2,     1,     3,     3,     2,     1,     2,     2,
       1,     1,     2,     2,     1,     0,     5,     4,     3,     2,
       0,     1,     2,     2,     4,     4,     1,     1,     0,     7,
       2,     0,     1,     1,     2,     0,     4,     3,     2,     0,
       1,     3,     2,     1,     1,     3,     2,     4,     2,     4,
       2,     1,     2,     2,     1,     1,     2,     3,     2,     3,
       0,     5,     0,     5,     1,     1,     2,     2,     1,     0,
       6,     3,     1,     1,     1,     2,     0,     4,     3,     2,
       0,     1,     3,     1,     1,     1,     6,     6,     9,     9,
       2,     2,     0,     5,     0,     5,     3,     1,     1,     3,
       1,     0,     1,     1,     1,     0,     0,     3,     5,     4,
       6,     3,     5,     2,     0,     1,     1,     1,     2,     1,
       2,     1,     2,     1,     2,     1,     2,     1,     2,     1,
       2,     1,     2,     1,     2,     2,     1,     2,     1,     2,
       2,     2,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     6,     5,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
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


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
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
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
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

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

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

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
      yychar = yylex (&yylval, &yylloc);
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
  *++yylsp = yylloc;
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

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 184 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1889 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 192 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1896 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 195 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1903 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 203 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[-1]);
}
#line 1911 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 207 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("global metadata must appear before any definitions");
    (yyval) = (yyvsp[-1]); // Dummy
}
#line 1920 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 217 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[-1]);
}
#line 1928 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 221 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new StringListTok;
}
#line 1936 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 230 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[0]));
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
#line 1948 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 238 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-1]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[0]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1961 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 247 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1968 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 255 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ModulePtr::dynamicCast((yyvsp[0])));
}
#line 1976 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 260 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[0])));
}
#line 1984 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 265 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after class forward declaration");
}
#line 1992 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 269 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDefPtr::dynamicCast((yyvsp[0])));
}
#line 2000 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 274 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[0])));
}
#line 2008 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 279 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after interface forward declaration");
}
#line 2016 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 283 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDefPtr::dynamicCast((yyvsp[0])));
}
#line 2024 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 288 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0);
}
#line 2032 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 293 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after exception forward declaration");
}
#line 2040 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 297 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ExceptionPtr::dynamicCast((yyvsp[0])));
}
#line 2048 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 302 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0);
}
#line 2056 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 307 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after struct forward declaration");
}
#line 2064 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 311 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || StructPtr::dynamicCast((yyvsp[0])));
}
#line 2072 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 316 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || SequencePtr::dynamicCast((yyvsp[0])));
}
#line 2080 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 321 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after sequence definition");
}
#line 2088 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 325 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || DictionaryPtr::dynamicCast((yyvsp[0])));
}
#line 2096 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 330 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after dictionary definition");
}
#line 2104 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 334 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || EnumPtr::dynamicCast((yyvsp[0])));
}
#line 2112 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 339 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ConstPtr::dynamicCast((yyvsp[0])));
}
#line 2120 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 344 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after const definition");
}
#line 2128 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 348 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    yyerrok;
}
#line 2136 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 357 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
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
#line 2156 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 373 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-3]))
    {
        unit->popContainer();
        (yyval) = (yyvsp[-3]);
    }
    else
    {
        (yyval) = 0;
    }
}
#line 2172 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 390 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2180 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 394 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2190 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 405 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("exceptions cannot be forward declared");
    (yyval) = 0;
}
#line 2199 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 415 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[-1]));
    ExceptionPtr base = ExceptionPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base);
    if(ex)
    {
        cont->checkIntroduced(ident->v, ex);
        unit->pushContainer(ex);
    }
    (yyval) = ex;
}
#line 2216 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 428 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-3]))
    {
        unit->popContainer();
    }
    (yyval) = (yyvsp[-3]);
}
#line 2228 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 441 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    (yyval) = contained;
}
#line 2240 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 449 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = 0;
}
#line 2248 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 458 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2261 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 467 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2268 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 470 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 2276 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 474 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2283 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 482 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    (yyval) = typestring;
}
#line 2295 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 495 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    IntegerTokPtr i = IntegerTokPtr::dynamicCast((yyvsp[-1]));

    int tag;
    if(i->v < 0 || i->v > Int32Max)
    {
        unit->error("tag is out of range");
        tag = -1;
    }
    else
    {
        tag = static_cast<int>(i->v);
    }

    TaggedDefTokPtr m = new TaggedDefTok(tag);
    (yyval) = m;
}
#line 2317 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 513 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-1]));

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
            for(const auto& p : enumerators)
            {
                if(first)
                {
                    first = false;
                }
                else
                {
                    os << " or";
                }

                os << " `" << p->scoped() << "'";
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
                    unit->error("tag is out of range");
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
        unit->error("invalid tag `" + scoped->v + "'");
    }

    TaggedDefTokPtr m = new TaggedDefTok(tag);
    (yyval) = m;
}
#line 2412 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 604 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    (yyval) = m;
}
#line 2422 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 610 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    (yyval) = m;
}
#line 2432 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 621 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    IntegerTokPtr i = IntegerTokPtr::dynamicCast((yyvsp[-1]));
    unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));

    int tag;
    if(i->v < 0 || i->v > Int32Max)
    {
        unit->error("tag is out of range");
        tag = -1;
    }
    else
    {
        tag = static_cast<int>(i->v);
    }

    TaggedDefTokPtr m = new TaggedDefTok(tag);
    (yyval) = m;
}
#line 2455 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 640 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-1]));
    unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));

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
            for(const auto& p : enumerators)
            {
                if(first)
                {
                    first = false;
                }
                else
                {
                    os << " or";
                }

                os << " `" << p->scoped() << "'";
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
                    unit->error("tag is out of range");
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
        unit->error("invalid tag `" + scoped->v + "'");
    }

    TaggedDefTokPtr m = new TaggedDefTok(tag);
    (yyval) = m;
}
#line 2551 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 732 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    (yyval) = m;
}
#line 2562 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 739 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    (yyval) = m;
}
#line 2573 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 751 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast((yyvsp[-1]));
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));

//  OptionalPtr opt = OptionalPtr::dynamicCast(ts->v.first);
//  if(!opt)
//  {
//      unit->error("Only optional types can be tagged.");
//  }

    m->type = ts->v.first;
    m->name = ts->v.second;
    (yyval) = m;
}
#line 2592 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 766 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast((yyvsp[-1]));
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));

    // Infer the type to be optional for backwards compatability.
    m->type = new Optional(ts->v.first);
    m->name = ts->v.second;
    (yyval) = m;
}
#line 2606 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 776 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = ts->v.first;
    m->name = ts->v.second;
    (yyval) = m;
}
#line 2618 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 795 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2626 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 799 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2636 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 74:
#line 810 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("structs cannot be forward declared");
    (yyval) = 0; // Dummy
}
#line 2645 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 820 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    StructPtr st = cont->createStruct(ident->v);
    if(st)
    {
        cont->checkIntroduced(ident->v, st);
        unit->pushContainer(st);
    }
    else
    {
        st = cont->createStruct(IceUtil::generateUUID()); // Dummy
        assert(st);
        unit->pushContainer(st);
    }
    (yyval) = st;
}
#line 2667 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 838 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-3]))
    {
        unit->popContainer();
    }
    (yyval) = (yyvsp[-3]);

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
#line 2689 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 861 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2702 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 78:
#line 870 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2709 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 873 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 2717 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 877 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2724 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 891 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2732 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 895 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2742 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 906 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    IceUtil::Int64 id = IntegerTokPtr::dynamicCast((yyvsp[-1]))->v;
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
    classId->v = StringTokPtr::dynamicCast((yyvsp[-2]))->v;
    classId->t = static_cast<int>(id);
    (yyval) = classId;
}
#line 2771 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 931 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-1]));

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
    classId->v = StringTokPtr::dynamicCast((yyvsp[-2]))->v;
    classId->t = id;
    (yyval) = classId;

}
#line 2877 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 1033 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    classId->t = -1;
    (yyval) = classId;
}
#line 2888 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 1045 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false);
    (yyval) = cl;
}
#line 2899 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 1057 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    ClassIdTokPtr ident = ClassIdTokPtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast((yyvsp[-1]));
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast((yyvsp[0]));
    if(base)
    {
        bases->v.push_front(base);
    }
    ClassDefPtr cl = cont->createClassDef(ident->v, ident->t, false, bases->v);
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
#line 2925 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 1079 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-3]))
    {
        unit->popContainer();
        (yyval) = (yyvsp[-3]);
    }
    else
    {
        (yyval) = 0;
    }
}
#line 2941 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 1096 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
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
#line 2979 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 91:
#line 1130 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = 0;
}
#line 2987 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 1139 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2994 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 1142 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3001 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 1150 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3009 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 1154 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ClassListTok;
}
#line 3017 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 1163 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3030 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 1172 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3037 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 1175 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 3045 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 1179 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3052 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 1187 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast((yyvsp[0]));
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    DataMemberPtr dm;
    if(cl)
    {
        dm = cl->createDataMember(def->name, def->type, def->isTagged, def->tag, 0, "", "");
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        dm = st->createDataMember(def->name, def->type, def->isTagged, def->tag, 0, "", "");
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        dm = ex->createDataMember(def->name, def->type, def->isTagged, def->tag, 0, "", "");
    }
    unit->currentContainer()->checkIntroduced(def->name, dm);
    (yyval) = dm;
}
#line 3078 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 1209 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));

    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    DataMemberPtr dm;
    if(cl)
    {
        dm = cl->createDataMember(def->name, def->type, def->isTagged, def->tag, value->v,
                                  value->valueAsString, value->valueAsLiteral);
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        dm = st->createDataMember(def->name, def->type, def->isTagged, def->tag, value->v,
                                  value->valueAsString, value->valueAsLiteral);
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        dm = ex->createDataMember(def->name, def->type, def->isTagged, def->tag, value->v,
                                  value->valueAsString, value->valueAsLiteral);
    }
    unit->currentContainer()->checkIntroduced(def->name, dm);
    (yyval) = dm;
}
#line 3109 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 1236 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
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
#line 3135 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 1258 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[0]));
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
#line 3160 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 1284 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
}
#line 3173 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 1293 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, value->v,
                                            value->valueAsString, value->valueAsLiteral);
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
}
#line 3188 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 1304 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("tagged data members are not supported in structs");
}
#line 3201 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 1313 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[-2]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("tagged data members are not supported in structs");
}
#line 3214 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 1322 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("tagged data members are not supported in structs");
}
#line 3227 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 1331 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[-2]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("tagged data members are not supported in structs");
}
#line 3240 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 1340 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3254 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 1350 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("missing data member name");
}
#line 3267 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 112:
#line 1364 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast((yyvsp[-1]));

//  OptionalPtr opt = OptionalPtr::dynamicCast($2);
//  if(!opt)
//  {
//      unit->error("Only optional types can be tagged.");
//  }

    m->type = TypePtr::dynamicCast((yyvsp[0]));
    (yyval) = m;
}
#line 3284 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 113:
#line 1377 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast((yyvsp[-1]));

    // Infer the type to be optional for backwards compatability.
    m->type = new Optional(TypePtr::dynamicCast((yyvsp[0])));
    (yyval) = m;
}
#line 3296 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 114:
#line 1385 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = TypePtr::dynamicCast((yyvsp[0]));
    (yyval) = m;
}
#line 3306 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 115:
#line 1391 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    (yyval) = m;
}
#line 3315 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 116:
#line 1401 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->type, returnType->isTagged, returnType->tag);
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
#line 3343 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 117:
#line 1425 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->type, returnType->isTagged, returnType->tag,
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
#line 3372 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 118:
#line 1450 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->type, returnType->isTagged, returnType->tag);
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
#line 3400 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 119:
#line 1474 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->type, returnType->isTagged, returnType->tag,
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
#line 3429 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 120:
#line 1504 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-2]))
    {
        unit->popContainer();
        (yyval) = (yyvsp[-2]);
    }
    else
    {
        (yyval) = 0;
    }
}
#line 3445 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 121:
#line 1516 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[-1]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3459 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 122:
#line 1526 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-2]))
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3471 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 123:
#line 1534 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[-1]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3485 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 126:
#line 1556 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3493 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 127:
#line 1560 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 3503 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 128:
#line 1571 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true);
    cont->checkIntroduced(ident->v, cl);
    (yyval) = cl;
}
#line 3515 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 129:
#line 1584 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[-1]));
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast((yyvsp[0]));
    ClassDefPtr cl = cont->createClassDef(ident->v, -1, true, bases->v);
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
#line 3536 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 130:
#line 1601 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-3]))
    {
        unit->popContainer();
        (yyval) = (yyvsp[-3]);
    }
    else
    {
        (yyval) = 0;
    }
}
#line 3552 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 131:
#line 1618 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    ClassListTokPtr intfs = ClassListTokPtr::dynamicCast((yyvsp[0]));
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-2]));
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
#line 3591 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 132:
#line 1653 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    ClassListTokPtr intfs = new ClassListTok;
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
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
#line 3630 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 133:
#line 1688 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("illegal inheritance from type Object");
    (yyval) = new ClassListTok; // Dummy
}
#line 3639 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 134:
#line 1693 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("illegal inheritance from type Value");
    (yyval) = new ClassListTok; // Dummy
}
#line 3648 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 135:
#line 1703 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3656 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 136:
#line 1707 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ClassListTok;
}
#line 3664 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 137:
#line 1716 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3677 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 138:
#line 1725 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3684 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 139:
#line 1728 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 3692 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 140:
#line 1732 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3699 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 142:
#line 1746 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[-2]));
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
}
#line 3710 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 143:
#line 1753 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[0]));
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
}
#line 3721 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 144:
#line 1765 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if(!exception)
    {
        exception = cont->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    (yyval) = exception;
}
#line 3737 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 145:
#line 1777 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3747 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 146:
#line 1788 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v);
}
#line 3759 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 147:
#line 1796 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3772 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 148:
#line 1810 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast((yyvsp[-6]));
    TypePtr keyType = TypePtr::dynamicCast((yyvsp[-5]));
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr valueType = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3786 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 149:
#line 1820 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast((yyvsp[-6]));
    TypePtr keyType = TypePtr::dynamicCast((yyvsp[-5]));
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr valueType = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3801 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 150:
#line 1836 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3809 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 151:
#line 1840 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 3819 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 152:
#line 1851 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v);
    if(en)
    {
        cont->checkIntroduced(ident->v, en);
    }
    else
    {
        en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    }
    unit->pushContainer(en);
    (yyval) = en;
}
#line 3839 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 153:
#line 1867 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    EnumPtr en = EnumPtr::dynamicCast((yyvsp[-3]));
    if(en)
    {
        EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast((yyvsp[-1]));
        if(enumerators->v.empty())
        {
            unit->error("enum `" + en->name() + "' must have at least one enumerator");
        }
        unit->popContainer();
    }
    (yyval) = (yyvsp[-3]);
}
#line 3857 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 154:
#line 1882 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing enumeration name");
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    unit->pushContainer(en);
    (yyval) = en;
}
#line 3869 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 155:
#line 1890 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->popContainer();
    (yyval) = (yyvsp[-4]);
}
#line 3878 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 156:
#line 1900 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast((yyvsp[-2]));
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast((yyvsp[0]))->v);
    (yyval) = ens;
}
#line 3888 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 157:
#line 1906 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3895 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 158:
#line 1914 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    ContainerPtr cont = unit->currentContainer();
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if(en)
    {
        ens->v.push_front(en);
    }
    (yyval) = ens;
}
#line 3911 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 159:
#line 1926 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[-2]));
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    ContainerPtr cont = unit->currentContainer();
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast((yyvsp[0]));
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
#line 3935 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 160:
#line 1946 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    (yyval) = ens;
}
#line 3946 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 161:
#line 1953 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    (yyval) = ens; // Dummy
}
#line 3955 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 162:
#line 1963 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3963 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 163:
#line 1967 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
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
#line 4001 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 164:
#line 2006 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr out = new BoolTok;
    out->v = true;
    (yyval) = out;
}
#line 4011 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 165:
#line 2012 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr out = new BoolTok;
    out->v = false;
    (yyval) = out;
}
#line 4021 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 166:
#line 2023 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4028 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 167:
#line 2026 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[-2]));
    TaggedDefTokPtr tsp = TaggedDefTokPtr::dynamicCast((yyvsp[0]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isTagged, tsp->tag);
        unit->currentContainer()->checkIntroduced(tsp->name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-1]));
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 4048 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 168:
#line 2042 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[-2]));
    TaggedDefTokPtr tsp = TaggedDefTokPtr::dynamicCast((yyvsp[0]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isTagged, tsp->tag);
        unit->currentContainer()->checkIntroduced(tsp->name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-1]));
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 4068 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 169:
#line 2058 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
        unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
#line 4084 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 170:
#line 2070 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
        unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
#line 4100 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 171:
#line 2082 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[-2]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[0]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        unit->error("missing parameter name");
    }
}
#line 4115 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 172:
#line 2093 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[-2]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[0]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        unit->error("missing parameter name");
    }
}
#line 4130 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 173:
#line 2109 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 4138 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 174:
#line 2113 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ExceptionListTok;
}
#line 4146 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 175:
#line 2122 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4153 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 176:
#line 2125 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4160 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 177:
#line 2133 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindByte);
}
#line 4168 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 178:
#line 2137 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindByte);
}
#line 4176 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 179:
#line 2141 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindBool);
}
#line 4184 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 180:
#line 2145 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindBool);
}
#line 4192 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 181:
#line 2149 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindShort);
}
#line 4200 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 182:
#line 2153 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindShort);
}
#line 4208 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 183:
#line 2157 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindInt);
}
#line 4216 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 184:
#line 2161 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindInt);
}
#line 4224 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 185:
#line 2165 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindLong);
}
#line 4232 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 186:
#line 2169 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindLong);
}
#line 4240 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 187:
#line 2173 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindFloat);
}
#line 4248 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 188:
#line 2177 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindFloat);
}
#line 4256 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 189:
#line 2181 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindDouble);
}
#line 4264 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 190:
#line 2185 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindDouble);
}
#line 4272 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 191:
#line 2189 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindString);
}
#line 4280 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 192:
#line 2193 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindString);
}
#line 4288 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 193:
#line 2197 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindObject);
}
#line 4296 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 194:
#line 2201 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindObjectProxy);
}
#line 4304 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 195:
#line 2205 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    // TODO: equivalent to ICE_OBJECT ? above, need to merge KindObject / KindObjectProxy
    (yyval) = unit->builtin(Builtin::KindObjectProxy);
}
#line 4313 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 196:
#line 2210 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindValue);
}
#line 4321 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 197:
#line 2214 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->optionalBuiltin(Builtin::KindValue);
}
#line 4329 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 198:
#line 2218 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
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
#line 4352 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 199:
#line 2237 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    // TODO: keep '*' only as an alias for T? where T = interface
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-1]));
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
            *p = new Proxy(cl);
        }
        (yyval) = types.front();
    }
    else
    {
        (yyval) = 0;
    }
}
#line 4389 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 200:
#line 2270 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-1]));
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
            cont->checkIntroduced(scoped->v);
            *p = new Optional(*p);
        }
        (yyval) = types.front();
    }
    else
    {
        (yyval) = 0;
    }
}
#line 4416 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 201:
#line 2298 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str1 = StringTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr str2 = StringTokPtr::dynamicCast((yyvsp[0]));
    str1->v += str2->v;
}
#line 4426 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 202:
#line 2304 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4433 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 203:
#line 2312 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr stringList = StringListTokPtr::dynamicCast((yyvsp[-2]));
    stringList->v.push_back(str->v);
    (yyval) = stringList;
}
#line 4444 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 204:
#line 2319 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    (yyval) = stringList;
}
#line 4455 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 205:
#line 2331 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast((yyvsp[0]));
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    (yyval) = def;
}
#line 4468 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 206:
#line 2340 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast((yyvsp[0]));
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    (yyval) = def;
}
#line 4481 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 207:
#line 2349 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
    ConstDefTokPtr def;
    ContainedList cl = unit->currentContainer()->lookupContained(scoped->v, false);
    if(cl.empty())
    {
        // Could be an enumerator
        def = new ConstDefTok(SyntaxTreeBasePtr(0), scoped->v, scoped->v);
    }
    else
    {
        EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
        ConstPtr constant = ConstPtr::dynamicCast(cl.front());
        if(enumerator)
        {
            unit->currentContainer()->checkIntroduced(scoped->v, enumerator);
            def = new ConstDefTok(enumerator, scoped->v, scoped->v);
        }
        else if(constant)
        {
            unit->currentContainer()->checkIntroduced(scoped->v, constant);
            def = new ConstDefTok(constant, constant->value(), constant->value());
        }
        else
        {
            def = new ConstDefTok;
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
#line 4525 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 208:
#line 2389 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[0]));
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    (yyval) = def;
}
#line 4536 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 209:
#line 2396 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[0]));
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    (yyval) = def;
}
#line 4547 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 210:
#line 2403 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[0]));
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    (yyval) = def;
}
#line 4558 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 211:
#line 2415 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-4]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[-3]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));
    (yyval) = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4571 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 212:
#line 2424 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));
    unit->error("missing constant name");
    (yyval) = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4584 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 213:
#line 2438 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4591 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 214:
#line 2441 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4598 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 215:
#line 2444 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4605 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 216:
#line 2447 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4612 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 217:
#line 2450 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4619 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 218:
#line 2453 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4626 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 219:
#line 2456 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4633 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 220:
#line 2459 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4640 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 221:
#line 2462 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4647 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 222:
#line 2465 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4654 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 223:
#line 2468 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4661 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 224:
#line 2471 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4668 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 225:
#line 2474 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4675 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 226:
#line 2477 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4682 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 227:
#line 2480 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4689 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 228:
#line 2483 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4696 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 229:
#line 2486 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4703 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 230:
#line 2489 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4710 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 231:
#line 2492 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4717 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 232:
#line 2495 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4724 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 233:
#line 2498 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4731 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 234:
#line 2501 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4738 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 235:
#line 2504 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4745 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 236:
#line 2507 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4752 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 237:
#line 2510 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4759 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 238:
#line 2513 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4766 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 239:
#line 2516 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4773 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 240:
#line 2519 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4780 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;

  case 241:
#line 2522 "src/Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4787 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
    break;


#line 4791 "src/Slice/Grammar.cpp" /* yacc.c:1646  */
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
  *++yylsp = yyloc;

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

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc);
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

  yyerror_range[1] = yylsp[1-yylen];
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp);
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
#line 2526 "src/Slice/Grammar.y" /* yacc.c:1906  */

