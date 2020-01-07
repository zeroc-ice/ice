/* A Bison parser, made by GNU Bison 3.5.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5"

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
#define yydebug         slice_debug
#define yynerrs         slice_nerrs

/* First part of user prologue.  */
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

//
// Avoid clang conversion warnings
//
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


#line 131 "src/Slice/Grammar.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED
# define YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int slice_debug;
#endif

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
    ICE_GLOBAL_METADATA_CLOSE = 295,
    ICE_IDENT_OPEN = 296,
    ICE_KEYWORD_OPEN = 297,
    ICE_TAG_OPEN = 298,
    ICE_OPTIONAL_OPEN = 299,
    BAD_CHAR = 300
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int slice_parse (void);

#endif /* !YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

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
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   942

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  56
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  86
/* YYNRULES -- Number of rules.  */
#define YYNRULES  240
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  350

#define YYUNDEFTOK  2
#define YYMAXUTOK   300


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    49,    55,     2,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    46,
      52,    50,    53,    54,     2,     2,     2,     2,     2,     2,
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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   122,   122,   130,   133,   141,   150,   154,   164,   163,
     173,   172,   182,   191,   190,   196,   195,   200,   205,   204,
     210,   209,   214,   219,   218,   224,   223,   228,   233,   232,
     238,   237,   242,   247,   246,   252,   251,   256,   261,   260,
     265,   270,   269,   275,   274,   279,   283,   293,   292,   326,
     330,   341,   352,   351,   377,   385,   394,   403,   406,   410,
     418,   431,   449,   540,   546,   557,   576,   668,   675,   687,
     702,   712,   725,   731,   735,   746,   757,   756,   797,   806,
     809,   813,   821,   827,   831,   842,   867,   969,   981,   994,
     993,  1032,  1066,  1075,  1079,  1088,  1097,  1100,  1104,  1112,
    1134,  1161,  1183,  1209,  1218,  1229,  1238,  1247,  1256,  1265,
    1275,  1289,  1302,  1310,  1316,  1326,  1350,  1375,  1399,  1430,
    1429,  1452,  1451,  1474,  1475,  1481,  1485,  1496,  1510,  1509,
    1543,  1578,  1613,  1618,  1628,  1632,  1641,  1650,  1653,  1657,
    1665,  1671,  1678,  1690,  1702,  1713,  1721,  1735,  1745,  1761,
    1765,  1777,  1776,  1808,  1807,  1825,  1831,  1839,  1851,  1871,
    1878,  1888,  1892,  1931,  1937,  1948,  1951,  1967,  1983,  1995,
    2007,  2018,  2034,  2038,  2047,  2050,  2058,  2062,  2066,  2070,
    2074,  2078,  2082,  2086,  2090,  2094,  2098,  2102,  2106,  2110,
    2114,  2118,  2122,  2126,  2130,  2135,  2139,  2143,  2162,  2195,
    2223,  2229,  2237,  2244,  2256,  2265,  2274,  2314,  2321,  2328,
    2340,  2349,  2363,  2366,  2369,  2372,  2375,  2378,  2381,  2384,
    2387,  2390,  2393,  2396,  2399,  2402,  2405,  2408,  2411,  2414,
    2417,  2420,  2423,  2426,  2429,  2432,  2435,  2438,  2441,  2444,
    2447
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
  "ICE_GLOBAL_METADATA_CLOSE", "ICE_IDENT_OPEN", "ICE_KEYWORD_OPEN",
  "ICE_TAG_OPEN", "ICE_OPTIONAL_OPEN", "BAD_CHAR", "';'", "'{'", "'}'",
  "')'", "'='", "','", "'<'", "'>'", "'?'", "'*'", "$accept", "start",
  "opt_semicolon", "global_meta_data", "meta_data", "definitions", "$@1",
  "$@2", "definition", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9",
  "$@10", "$@11", "$@12", "$@13", "$@14", "$@15", "module_def", "@16",
  "exception_id", "exception_decl", "exception_def", "@17",
  "exception_extends", "exception_exports", "type_id", "tag", "optional",
  "tagged_type_id", "exception_export", "struct_id", "struct_decl",
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
  "const_initializer", "const_def", "keyword", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,    59,   123,   125,    41,
      61,    44,    60,    62,    63,    42
};
# endif

#define YYPACT_NINF (-302)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-166)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      30,    -3,    -3,    34,  -302,    73,  -302,    -3,  -302,    -7,
      -4,  -302,    12,    19,     3,   544,   617,   650,   683,    -9,
      23,   716,    48,  -302,  -302,    21,    46,  -302,    54,    78,
    -302,     7,   115,    83,  -302,    25,    93,  -302,   103,   107,
    -302,  -302,   111,  -302,  -302,    -3,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
      58,  -302,  -302,  -302,  -302,  -302,  -302,  -302,    48,    48,
    -302,   122,  -302,   144,    12,   127,    97,  -302,   128,   127,
     129,   132,   127,    97,   172,   140,   127,   120,  -302,   143,
     127,   151,   152,   153,   127,   155,  -302,   156,   141,  -302,
    -302,   157,   144,   144,   749,   148,   165,   166,   167,   168,
     170,   171,   173,    66,   174,    91,    -8,  -302,  -302,  -302,
    -302,   179,  -302,  -302,   425,  -302,  -302,  -302,   120,  -302,
    -302,  -302,  -302,  -302,  -302,   180,   182,  -302,  -302,  -302,
    -302,   749,  -302,  -302,   -13,  -302,  -302,   177,   181,   184,
     187,   185,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,   188,   178,   458,   191,
     898,   192,  -302,   194,   120,   355,   196,   197,   782,    48,
      64,  -302,   749,   178,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,   193,   898,   198,   425,  -302,  -302,   101,   105,   200,
     144,   144,   202,  -302,   815,  -302,   392,  -302,   205,   869,
     204,  -302,  -302,  -302,  -302,   144,  -302,  -302,  -302,  -302,
    -302,   458,  -302,   144,   144,   203,   208,  -302,   815,  -302,
    -302,   206,  -302,   207,   209,  -302,   210,   178,   212,   222,
     214,   425,  -302,  -302,   220,   869,   219,   355,  -302,    87,
     144,   144,   106,   318,  -302,   223,  -302,  -302,   189,  -302,
    -302,  -302,   178,   458,  -302,  -302,  -302,  -302,  -302,  -302,
     178,   178,  -302,   392,   144,   144,  -302,  -302,   224,   504,
    -302,  -302,   130,  -302,  -302,  -302,  -302,   211,  -302,    48,
       4,   355,   848,  -302,  -302,  -302,  -302,  -302,   222,   222,
     392,  -302,  -302,  -302,   898,  -302,   257,  -302,  -302,  -302,
    -302,   259,  -302,   815,   259,    48,   583,  -302,  -302,  -302,
     898,  -302,   225,  -302,  -302,  -302,   815,   583,  -302,  -302
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       7,     0,     0,     0,     8,     0,     2,   201,   203,     0,
       0,     1,     7,     0,     0,     0,     0,     0,     0,     0,
       0,   153,     7,    10,    13,    51,    27,    28,    75,    32,
      33,    88,    92,    17,    18,   127,    22,    23,    37,    40,
     151,    41,    45,   200,     6,     0,     5,     9,    46,    47,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,    83,
       0,    84,   125,   126,    49,    50,    73,    74,     7,     7,
     149,     0,   150,     0,     7,     4,     0,    52,     0,     4,
       0,     0,     4,     0,    94,     0,     4,     0,   128,     0,
       4,     0,     0,     0,     4,     0,   202,     0,     0,   174,
     175,     0,     0,     0,   160,   176,   178,   180,   182,   184,
     186,   188,   190,   192,   195,   197,     0,    11,     3,    14,
      54,     0,    26,    29,     0,    31,    34,    91,     0,    89,
      16,    19,   132,   133,   134,   131,     0,    21,    24,    36,
      39,   160,    42,    44,     7,    85,    86,     0,     0,   157,
       0,   156,   159,   177,   179,   181,   183,   185,   187,   189,
     191,   193,   194,   196,   199,   198,     0,     0,     0,     0,
       0,     0,    93,     0,     0,     0,     0,     0,     0,     7,
       0,   154,   160,     0,   208,   209,   207,   204,   205,   206,
     211,     0,     0,     0,     0,    64,    68,     0,     0,   103,
       0,     0,    80,    82,   110,    77,     0,   130,     0,     0,
       0,   152,    48,   145,   146,     0,   161,   158,   162,   155,
     210,     0,    71,     0,     0,    99,    58,    72,   102,    53,
      79,     0,    63,     0,     0,    67,     0,     0,   105,     0,
     107,     0,    60,   109,     0,     0,     0,     0,   114,     0,
       0,     0,     0,     0,   140,   138,   113,   129,     0,    57,
      69,    70,     0,     0,   101,    61,    62,    65,    66,   104,
       0,     0,    78,     0,     0,     0,   123,   124,    97,   102,
      90,   137,     0,   111,   112,   115,   117,     0,   163,     7,
       0,     0,     0,   100,    56,   106,   108,    96,   111,   112,
       0,   116,   118,   121,     0,   119,   164,   136,   147,   148,
      95,   173,   166,   170,   173,     7,     0,   122,   168,   120,
       0,   172,   142,   143,   144,   167,   171,     0,   169,   141
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -302,  -302,    85,  -302,   -18,    -5,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -226,
    -180,  -181,  -179,  -301,  -302,  -302,  -302,  -302,  -302,  -200,
    -302,  -302,  -302,  -302,  -302,  -302,  -302,  -302,  -268,     6,
    -302,     5,  -302,    13,  -302,  -302,  -302,  -302,  -302,  -302,
    -302,  -135,  -302,  -245,  -302,   -70,  -302,  -302,  -302,  -302,
    -302,  -302,  -302,  -144,  -302,  -302,   -47,  -302,   -54,   -75,
     -77,    11,   279,  -195,  -302,   -15
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,   139,     4,     5,     6,    12,    94,    23,    95,
     105,   106,   109,   110,    98,    99,   101,   102,   111,   112,
     114,   115,    24,   117,    25,    26,    27,   141,    97,   213,
     242,   243,   244,   245,   246,    28,    29,    30,   100,   191,
     222,    31,    32,    33,    34,   193,   104,   149,   266,   247,
     223,   272,   273,   274,   334,   331,   298,    35,    36,    37,
     156,   154,   108,   230,   275,   341,   342,    38,    39,    40,
      41,   113,    91,   170,   171,   237,   309,   310,   337,   135,
     259,     8,     9,   210,    42,   172
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      81,    83,    85,    87,    93,   121,    92,    47,   240,   220,
     219,   221,   -12,   192,   250,   279,   136,   196,    43,   -87,
     -87,   140,   301,   332,     1,   317,     2,   186,   147,     7,
     -12,    44,   155,    96,    11,   -12,    46,   107,    49,   345,
     258,   260,   187,    88,    45,   167,   168,    45,   270,     1,
     271,     2,   330,   325,   -87,   326,   116,   314,   239,   227,
     -12,   292,   289,   280,   281,    48,   327,     1,   -55,     2,
     122,   123,  -135,   155,    13,    89,    14,    15,    16,    17,
      18,    19,    20,    21,   294,     1,   295,   313,   270,   137,
     271,   118,   -25,   119,   120,   315,   316,   236,    22,   119,
     120,   -76,   268,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   209,   224,   280,   281,   215,   216,   134,   155,
     181,   182,   119,   120,   -30,   238,   190,   103,   209,   -15,
     217,   218,   119,   120,   251,   248,   119,   120,   254,   -20,
     119,   120,   253,   256,   152,   184,   185,   305,   306,   -35,
     252,   153,   276,   -38,   255,   119,   120,   -43,   278,   197,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   124,
     212,   321,   322,   138,   142,   134,   144,   229,   145,   119,
     120,   235,   209,   234,   143,   148,   150,   146,   299,   157,
     165,   151,   276,   303,   304,   158,   190,   159,   160,   162,
     161,   163,   173,   164,   204,   205,   166,   209,   265,   263,
     206,   207,   208,   119,   120,   209,   209,   318,   319,   174,
     175,   176,   177,   212,   178,   179,   188,   180,   183,   195,
     198,   194,   199,   284,   200,   201,   202,   214,   203,   241,
     225,   226,   312,   190,   231,   232,   249,   333,   261,   229,
     257,   267,   277,   282,   283,   285,   286,   262,   287,   288,
     323,   343,   290,   346,   291,   212,   293,   300,   308,   311,
     320,   296,   343,   336,   302,   265,   347,   349,   297,   335,
     339,    10,     0,     0,   284,     0,     0,     0,     0,     0,
       0,   324,     0,   229,     0,     0,     0,   329,     0,     0,
       0,     0,   265,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   340,   338,   307,
       0,   344,     0,     0,     0,     0,     0,     0,     0,   308,
       0,   348,   344,     0,  -164,  -164,  -164,  -164,  -164,  -164,
    -164,  -164,  -164,     0,     0,     0,     0,  -164,  -164,  -164,
       0,     0,     0,  -164,  -164,  -164,   228,     0,     0,     0,
       0,  -164,  -164,     0,     0,     0,     0,  -165,     0,  -165,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,
       0,     0,     0,    -7,    -7,    -7,    -7,     0,     0,     0,
      -7,    -7,     1,   264,     0,     0,     0,     0,    -7,    -7,
       0,     0,     0,  -139,     0,     0,     0,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,     0,     0,     0,
      -7,    -7,    -7,    -7,     0,     0,   189,    -7,    -7,     1,
       0,     0,     0,     0,     0,    -7,    -7,     0,     0,     0,
     -98,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,
       0,     0,     0,     0,    -7,    -7,    -7,     0,     0,   211,
      -7,    -7,     1,     0,     0,     0,     0,     0,    -7,    -7,
       0,     0,     0,   -81,    -7,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,    -7,     0,     0,     0,     0,    -7,    -7,    -7,
       0,     0,     0,    -7,    -7,     1,     0,     0,     0,     0,
       0,    -7,    -7,     0,     0,     0,   -59,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     0,     0,     0,   262,
       0,     0,     0,     0,     0,  -113,  -113,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     0,     0,     0,    79,
       0,     0,     0,     0,     0,    80,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,     0,     0,     0,   119,   120,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,     0,
       0,     0,    82,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,     0,     0,     0,    84,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,     0,     0,     0,    86,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
       0,    90,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,     0,     0,     0,   169,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,     0,   233,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,     0,     0,     0,
     262,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,     0,   328,   268,   125,   126,   127,   128,   129,
     130,   131,   132,   133,     0,     0,     0,   269,   215,   216,
     134,     0,     0,     0,   119,   120,     0,     0,     0,     0,
       0,     0,   217,   218,   125,   126,   127,   128,   129,   130,
     131,   132,   133,     0,     0,     0,     0,   215,   216,   134,
       0,     0,     0,   119,   120,     0,     0,     0,     0,     0,
       0,   217,   218
};

static const yytype_int16 yycheck[] =
{
      15,    16,    17,    18,    22,    80,    21,    12,   203,   190,
     190,   190,     0,   148,   214,   241,    93,   161,     7,    12,
      13,    96,   267,   324,    37,   293,    39,    35,   103,    32,
       0,    38,   107,    12,     0,    48,    40,    12,    35,   340,
     220,   221,    50,    52,    51,   122,   123,    51,   229,    37,
     229,    39,   320,    49,    47,    51,    45,   283,   202,   194,
      48,   261,   257,   243,   244,    46,   311,    37,    47,    39,
      88,    89,    47,   148,     1,    52,     3,     4,     5,     6,
       7,     8,     9,    10,   265,    37,   265,   282,   269,    94,
     269,    33,    46,    35,    36,   290,   291,    33,    25,    35,
      36,    47,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,   187,   190,   294,   295,    29,    30,    31,   194,
      54,    55,    35,    36,    46,   200,   144,    12,   203,    46,
      43,    44,    35,    36,    33,   212,    35,    36,    33,    46,
      35,    36,   217,   218,    24,    54,    55,    41,    42,    46,
      49,    31,   229,    46,    49,    35,    36,    46,   235,   164,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    47,
     188,    41,    42,    46,    46,    31,    47,   195,    46,    35,
      36,   199,   257,   198,    99,    13,    46,   102,   265,    46,
      49,   106,   269,   270,   271,   110,   214,    46,    46,   114,
      47,    46,    54,    47,    26,    27,    49,   282,   226,   224,
      32,    33,    34,    35,    36,   290,   291,   294,   295,    54,
      54,    54,    54,   241,    54,    54,    47,    54,    54,    47,
      53,    51,    51,   248,    50,    48,    51,    46,    50,    46,
      48,    47,    53,   261,    48,    48,    48,   324,    46,   267,
      50,    46,    48,    50,    46,    49,    49,    35,    49,    49,
      49,   336,    50,   340,    50,   283,    46,    48,    11,    46,
      46,   265,   347,    14,   269,   293,    51,   347,   265,   326,
     334,     2,    -1,    -1,   299,    -1,    -1,    -1,    -1,    -1,
      -1,   309,    -1,   311,    -1,    -1,    -1,   312,    -1,    -1,
      -1,    -1,   320,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   335,   333,     1,
      -1,   336,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    11,
      -1,   346,   347,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    29,    30,    31,
      -1,    -1,    -1,    35,    36,    37,     1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    -1,    51,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    28,    29,    30,    31,    -1,    -1,    -1,
      35,    36,    37,     1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    -1,    -1,    -1,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      28,    29,    30,    31,    -1,    -1,     1,    35,    36,    37,
      -1,    -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,
      48,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    -1,    29,    30,    31,    -1,    -1,     1,
      35,    36,    37,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    29,    30,    31,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    48,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    41,    42,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    35,    36,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    35,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    35,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    35,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    35,     3,     4,     5,     6,     7,     8,     9,    10,
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
      -1,    -1,    -1,    35,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    28,    29,    30,
      31,    -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    29,    30,    31,
      -1,    -1,    -1,    35,    36,    -1,    -1,    -1,    -1,    -1,
      -1,    43,    44
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    37,    39,    57,    59,    60,    61,    32,   137,   138,
     138,     0,    62,     1,     3,     4,     5,     6,     7,     8,
       9,    10,    25,    64,    78,    80,    81,    82,    91,    92,
      93,    97,    98,    99,   100,   113,   114,   115,   123,   124,
     125,   126,   140,   137,    38,    51,    40,    61,    46,    35,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    35,
      41,   141,    35,   141,    35,   141,    35,   141,    52,    52,
      35,   128,   141,    60,    63,    65,    12,    84,    70,    71,
      94,    72,    73,    12,   102,    66,    67,    12,   118,    68,
      69,    74,    75,   127,    76,    77,   137,    79,    33,    35,
      36,   135,    60,    60,    47,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    31,   135,   136,    61,    46,    58,
     135,    83,    46,    58,    47,    46,    58,   135,    13,   103,
      46,    58,    24,    31,   117,   135,   116,    46,    58,    46,
      46,    47,    58,    46,    47,    49,    49,   136,   136,    35,
     129,   130,   141,    54,    54,    54,    54,    54,    54,    54,
      54,    54,    55,    54,    54,    55,    35,    50,    47,     1,
      60,    95,   117,   101,    51,    47,   129,    61,    53,    51,
      50,    48,    51,    50,    26,    27,    32,    33,    34,   135,
     139,     1,    60,    85,    46,    29,    30,    43,    44,    86,
      87,    88,    96,   106,   136,    48,    47,   117,     1,    60,
     119,    48,    48,    35,   141,    60,    33,   131,   135,   129,
     139,    46,    86,    87,    88,    89,    90,   105,   136,    48,
      95,    33,    49,   135,    33,    49,   135,    50,    86,   136,
      86,    46,    35,   141,     1,    60,   104,    46,    15,    28,
      87,    88,   107,   108,   109,   120,   136,    48,   136,    85,
      86,    86,    50,    46,   141,    49,    49,    49,    49,   139,
      50,    50,    95,    46,    87,    88,   105,   109,   112,   136,
      48,   119,   107,   136,   136,    41,    42,     1,    11,   132,
     133,    46,    53,   139,    85,   139,   139,   104,   136,   136,
      46,    41,    42,    49,    60,    49,    51,   119,    35,   141,
     104,   111,    89,   136,   110,   132,    14,   134,   141,   134,
      60,   121,   122,   135,   141,    89,   136,    51,   141,   121
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    56,    57,    58,    58,    59,    60,    60,    62,    61,
      63,    61,    61,    65,    64,    66,    64,    64,    67,    64,
      68,    64,    64,    69,    64,    70,    64,    64,    71,    64,
      72,    64,    64,    73,    64,    74,    64,    64,    75,    64,
      64,    76,    64,    77,    64,    64,    64,    79,    78,    80,
      80,    81,    83,    82,    84,    84,    85,    85,    85,    85,
      86,    87,    87,    87,    87,    88,    88,    88,    88,    89,
      89,    89,    90,    91,    91,    92,    94,    93,    95,    95,
      95,    95,    96,    97,    97,    98,    98,    98,    99,   101,
     100,   102,   102,   103,   103,   104,   104,   104,   104,   105,
     105,   105,   105,   106,   106,   106,   106,   106,   106,   106,
     106,   107,   107,   107,   107,   108,   108,   108,   108,   110,
     109,   111,   109,   112,   112,   113,   113,   114,   116,   115,
     117,   117,   117,   117,   118,   118,   119,   119,   119,   119,
     120,   121,   121,   122,   122,   123,   123,   124,   124,   125,
     125,   127,   126,   128,   126,   129,   129,   130,   130,   130,
     130,   131,   131,   132,   132,   133,   133,   133,   133,   133,
     133,   133,   134,   134,   135,   135,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   136,   136,   136,
     137,   137,   138,   138,   139,   139,   139,   139,   139,   139,
     140,   140,   141,   141,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     141
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     0,     0,     3,
       0,     4,     0,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       1,     0,     3,     0,     3,     1,     2,     0,     6,     2,
       2,     1,     0,     6,     2,     0,     4,     3,     2,     0,
       2,     3,     3,     2,     1,     3,     3,     2,     1,     2,
       2,     1,     1,     2,     2,     1,     0,     5,     4,     3,
       2,     0,     1,     2,     2,     4,     4,     1,     1,     0,
       7,     2,     0,     2,     0,     4,     3,     2,     0,     1,
       3,     2,     1,     1,     3,     2,     4,     2,     4,     2,
       1,     2,     2,     1,     1,     2,     3,     2,     3,     0,
       5,     0,     5,     1,     1,     2,     2,     1,     0,     6,
       3,     1,     1,     1,     2,     0,     4,     3,     2,     0,
       1,     3,     1,     1,     1,     6,     6,     9,     9,     2,
       2,     0,     5,     0,     5,     3,     1,     1,     3,     1,
       0,     1,     1,     1,     0,     0,     3,     5,     4,     6,
       3,     5,     2,     0,     1,     1,     1,     2,     1,     2,
       1,     2,     1,     2,     1,     2,     1,     2,     1,     2,
       1,     2,     1,     2,     2,     1,     2,     1,     2,     2,
       2,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       6,     5,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
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
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
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
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
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
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

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
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
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
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
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
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
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
          ++yyp;
          ++yyformat;
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

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

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
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
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
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
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
#line 123 "src/Slice/Grammar.y"
{
}
#line 1805 "src/Slice/Grammar.cpp"
    break;

  case 3:
#line 131 "src/Slice/Grammar.y"
{
}
#line 1812 "src/Slice/Grammar.cpp"
    break;

  case 4:
#line 134 "src/Slice/Grammar.y"
{
}
#line 1819 "src/Slice/Grammar.cpp"
    break;

  case 5:
#line 142 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1827 "src/Slice/Grammar.cpp"
    break;

  case 6:
#line 151 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1835 "src/Slice/Grammar.cpp"
    break;

  case 7:
#line 155 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 1843 "src/Slice/Grammar.cpp"
    break;

  case 8:
#line 164 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
#line 1855 "src/Slice/Grammar.cpp"
    break;

  case 10:
#line 173 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1868 "src/Slice/Grammar.cpp"
    break;

  case 12:
#line 183 "src/Slice/Grammar.y"
{
}
#line 1875 "src/Slice/Grammar.cpp"
    break;

  case 13:
#line 191 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1883 "src/Slice/Grammar.cpp"
    break;

  case 15:
#line 196 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1891 "src/Slice/Grammar.cpp"
    break;

  case 17:
#line 201 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 1899 "src/Slice/Grammar.cpp"
    break;

  case 18:
#line 205 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1907 "src/Slice/Grammar.cpp"
    break;

  case 20:
#line 210 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1915 "src/Slice/Grammar.cpp"
    break;

  case 22:
#line 215 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 1923 "src/Slice/Grammar.cpp"
    break;

  case 23:
#line 219 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1931 "src/Slice/Grammar.cpp"
    break;

  case 25:
#line 224 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1939 "src/Slice/Grammar.cpp"
    break;

  case 27:
#line 229 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 1947 "src/Slice/Grammar.cpp"
    break;

  case 28:
#line 233 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 1955 "src/Slice/Grammar.cpp"
    break;

  case 30:
#line 238 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1963 "src/Slice/Grammar.cpp"
    break;

  case 32:
#line 243 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 1971 "src/Slice/Grammar.cpp"
    break;

  case 33:
#line 247 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 1979 "src/Slice/Grammar.cpp"
    break;

  case 35:
#line 252 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 1987 "src/Slice/Grammar.cpp"
    break;

  case 37:
#line 257 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 1995 "src/Slice/Grammar.cpp"
    break;

  case 38:
#line 261 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2003 "src/Slice/Grammar.cpp"
    break;

  case 40:
#line 266 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2011 "src/Slice/Grammar.cpp"
    break;

  case 41:
#line 270 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2019 "src/Slice/Grammar.cpp"
    break;

  case 43:
#line 275 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2027 "src/Slice/Grammar.cpp"
    break;

  case 45:
#line 280 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2035 "src/Slice/Grammar.cpp"
    break;

  case 46:
#line 284 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2043 "src/Slice/Grammar.cpp"
    break;

  case 47:
#line 293 "src/Slice/Grammar.y"
{
    unit->setSeenDefinition();
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ModulePtr module = cont->createModule(ident->v);
    if(module)
    {
        cont->checkIntroduced(ident->v, module);
        unit->pushContainer(module);
        yyval = module;
    }
    else
    {
        yyval = 0;
    }
}
#line 2064 "src/Slice/Grammar.cpp"
    break;

  case 48:
#line 310 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
        yyval = yyvsp[-3];
    }
    else
    {
        yyval = 0;
    }
}
#line 2080 "src/Slice/Grammar.cpp"
    break;

  case 49:
#line 327 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2088 "src/Slice/Grammar.cpp"
    break;

  case 50:
#line 331 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2098 "src/Slice/Grammar.cpp"
    break;

  case 51:
#line 342 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2107 "src/Slice/Grammar.cpp"
    break;

  case 52:
#line 352 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-1]);
    ExceptionPtr base = ExceptionPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base);
    if(ex)
    {
        cont->checkIntroduced(ident->v, ex);
        unit->pushContainer(ex);
    }
    yyval = ex;
}
#line 2124 "src/Slice/Grammar.cpp"
    break;

  case 53:
#line 365 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2136 "src/Slice/Grammar.cpp"
    break;

  case 54:
#line 378 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2148 "src/Slice/Grammar.cpp"
    break;

  case 55:
#line 386 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2156 "src/Slice/Grammar.cpp"
    break;

  case 56:
#line 395 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2169 "src/Slice/Grammar.cpp"
    break;

  case 57:
#line 404 "src/Slice/Grammar.y"
{
}
#line 2176 "src/Slice/Grammar.cpp"
    break;

  case 58:
#line 407 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2184 "src/Slice/Grammar.cpp"
    break;

  case 59:
#line 411 "src/Slice/Grammar.y"
{
}
#line 2191 "src/Slice/Grammar.cpp"
    break;

  case 60:
#line 419 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    yyval = typestring;
}
#line 2203 "src/Slice/Grammar.cpp"
    break;

  case 61:
#line 432 "src/Slice/Grammar.y"
{
    IntegerTokPtr i = IntegerTokPtr::dynamicCast(yyvsp[-1]);

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
    yyval = m;
}
#line 2225 "src/Slice/Grammar.cpp"
    break;

  case 62:
#line 450 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-1]);

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
    yyval = m;
}
#line 2320 "src/Slice/Grammar.cpp"
    break;

  case 63:
#line 541 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2330 "src/Slice/Grammar.cpp"
    break;

  case 64:
#line 547 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2340 "src/Slice/Grammar.cpp"
    break;

  case 65:
#line 558 "src/Slice/Grammar.y"
{
    IntegerTokPtr i = IntegerTokPtr::dynamicCast(yyvsp[-1]);
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
    yyval = m;
}
#line 2363 "src/Slice/Grammar.cpp"
    break;

  case 66:
#line 577 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-1]);
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
    yyval = m;
}
#line 2459 "src/Slice/Grammar.cpp"
    break;

  case 67:
#line 669 "src/Slice/Grammar.y"
{
    unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2470 "src/Slice/Grammar.cpp"
    break;

  case 68:
#line 676 "src/Slice/Grammar.y"
{
    unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2481 "src/Slice/Grammar.cpp"
    break;

  case 69:
#line 688 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

//  OptionalPtr opt = OptionalPtr::dynamicCast(ts->v.first);
//  if(!opt)
//  {
//      unit->error("Only optional types can be tagged.");
//  }

    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2500 "src/Slice/Grammar.cpp"
    break;

  case 70:
#line 703 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

    // Infer the type to be optional for backwards compatability.
    m->type = new Optional(ts->v.first);
    m->name = ts->v.second;
    yyval = m;
}
#line 2514 "src/Slice/Grammar.cpp"
    break;

  case 71:
#line 713 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2526 "src/Slice/Grammar.cpp"
    break;

  case 73:
#line 732 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2534 "src/Slice/Grammar.cpp"
    break;

  case 74:
#line 736 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2544 "src/Slice/Grammar.cpp"
    break;

  case 75:
#line 747 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2553 "src/Slice/Grammar.cpp"
    break;

  case 76:
#line 757 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = st;
}
#line 2575 "src/Slice/Grammar.cpp"
    break;

  case 77:
#line 775 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];

    //
    // Empty structures are not allowed
    //
    StructPtr st = StructPtr::dynamicCast(yyval);
    assert(st);
    if(st->dataMembers().empty())
    {
        unit->error("struct `" + st->name() + "' must have at least one member"); // $$ is a dummy
    }
}
#line 2597 "src/Slice/Grammar.cpp"
    break;

  case 78:
#line 798 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2610 "src/Slice/Grammar.cpp"
    break;

  case 79:
#line 807 "src/Slice/Grammar.y"
{
}
#line 2617 "src/Slice/Grammar.cpp"
    break;

  case 80:
#line 810 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2625 "src/Slice/Grammar.cpp"
    break;

  case 81:
#line 814 "src/Slice/Grammar.y"
{
}
#line 2632 "src/Slice/Grammar.cpp"
    break;

  case 83:
#line 828 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2640 "src/Slice/Grammar.cpp"
    break;

  case 84:
#line 832 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2650 "src/Slice/Grammar.cpp"
    break;

  case 85:
#line 843 "src/Slice/Grammar.y"
{
    IceUtil::Int64 id = IntegerTokPtr::dynamicCast(yyvsp[-1])->v;
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
    classId->v = StringTokPtr::dynamicCast(yyvsp[-2])->v;
    classId->t = static_cast<int>(id);
    yyval = classId;
}
#line 2679 "src/Slice/Grammar.cpp"
    break;

  case 86:
#line 868 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-1]);

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
    classId->v = StringTokPtr::dynamicCast(yyvsp[-2])->v;
    classId->t = id;
    yyval = classId;

}
#line 2785 "src/Slice/Grammar.cpp"
    break;

  case 87:
#line 970 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2796 "src/Slice/Grammar.cpp"
    break;

  case 88:
#line 982 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false);
    yyval = cl;
}
#line 2807 "src/Slice/Grammar.cpp"
    break;

  case 89:
#line 994 "src/Slice/Grammar.y"
{
    ClassIdTokPtr ident = ClassIdTokPtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast(yyvsp[-1]);
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast(yyvsp[0]);
    if(base)
    {
        bases->v.push_front(base);
    }
    ClassDefPtr cl = cont->createClassDef(ident->v, ident->t, false, bases->v);
    if(cl)
    {
        cont->checkIntroduced(ident->v, cl);
        unit->pushContainer(cl);
        yyval = cl;
    }
    else
    {
        yyval = 0;
    }
}
#line 2833 "src/Slice/Grammar.cpp"
    break;

  case 90:
#line 1016 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
        yyval = yyvsp[-3];
    }
    else
    {
        yyval = 0;
    }
}
#line 2849 "src/Slice/Grammar.cpp"
    break;

  case 91:
#line 1033 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    yyval = 0;
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
                yyval = def;
            }
        }
    }
}
#line 2887 "src/Slice/Grammar.cpp"
    break;

  case 92:
#line 1067 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2895 "src/Slice/Grammar.cpp"
    break;

  case 93:
#line 1076 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2903 "src/Slice/Grammar.cpp"
    break;

  case 94:
#line 1080 "src/Slice/Grammar.y"
{
    yyval = new ClassListTok;
}
#line 2911 "src/Slice/Grammar.cpp"
    break;

  case 95:
#line 1089 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2924 "src/Slice/Grammar.cpp"
    break;

  case 96:
#line 1098 "src/Slice/Grammar.y"
{
}
#line 2931 "src/Slice/Grammar.cpp"
    break;

  case 97:
#line 1101 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2939 "src/Slice/Grammar.cpp"
    break;

  case 98:
#line 1105 "src/Slice/Grammar.y"
{
}
#line 2946 "src/Slice/Grammar.cpp"
    break;

  case 99:
#line 1113 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = dm;
}
#line 2972 "src/Slice/Grammar.cpp"
    break;

  case 100:
#line 1135 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);

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
    yyval = dm;
}
#line 3003 "src/Slice/Grammar.cpp"
    break;

  case 101:
#line 1162 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        yyval = cl->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        yyval = ex->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3029 "src/Slice/Grammar.cpp"
    break;

  case 102:
#line 1184 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        yyval = cl->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        yyval = ex->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    assert(yyval);
    unit->error("missing data member name");
}
#line 3054 "src/Slice/Grammar.cpp"
    break;

  case 103:
#line 1210 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 3067 "src/Slice/Grammar.cpp"
    break;

  case 104:
#line 1219 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, value->v,
                                            value->valueAsString, value->valueAsLiteral);
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 3082 "src/Slice/Grammar.cpp"
    break;

  case 105:
#line 1230 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3095 "src/Slice/Grammar.cpp"
    break;

  case 106:
#line 1239 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3108 "src/Slice/Grammar.cpp"
    break;

  case 107:
#line 1248 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3121 "src/Slice/Grammar.cpp"
    break;

  case 108:
#line 1257 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3134 "src/Slice/Grammar.cpp"
    break;

  case 109:
#line 1266 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3148 "src/Slice/Grammar.cpp"
    break;

  case 110:
#line 1276 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("missing data member name");
}
#line 3161 "src/Slice/Grammar.cpp"
    break;

  case 111:
#line 1290 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);

//  OptionalPtr opt = OptionalPtr::dynamicCast($2);
//  if(!opt)
//  {
//      unit->error("Only optional types can be tagged.");
//  }

    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3178 "src/Slice/Grammar.cpp"
    break;

  case 112:
#line 1303 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);

    // Infer the type to be optional for backwards compatability.
    m->type = new Optional(TypePtr::dynamicCast(yyvsp[0]));
    yyval = m;
}
#line 3190 "src/Slice/Grammar.cpp"
    break;

  case 113:
#line 1311 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3200 "src/Slice/Grammar.cpp"
    break;

  case 114:
#line 1317 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    yyval = m;
}
#line 3209 "src/Slice/Grammar.cpp"
    break;

  case 115:
#line 1327 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->type, returnType->isTagged, returnType->tag);
        if(op)
        {
            cl->checkIntroduced(name, op);
            unit->pushContainer(op);
            yyval = op;
        }
        else
        {
            yyval = 0;
        }
    }
    else
    {
        yyval = 0;
    }
}
#line 3237 "src/Slice/Grammar.cpp"
    break;

  case 116:
#line 1351 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->type, returnType->isTagged, returnType->tag,
                                                Operation::Idempotent);
        if(op)
        {
            cl->checkIntroduced(name, op);
            unit->pushContainer(op);
            yyval = op;
        }
        else
        {
            yyval = 0;
        }
    }
    else
    {
        yyval = 0;
    }
}
#line 3266 "src/Slice/Grammar.cpp"
    break;

  case 117:
#line 1376 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->type, returnType->isTagged, returnType->tag);
        if(op)
        {
            unit->pushContainer(op);
            unit->error("keyword `" + name + "' cannot be used as operation name");
            yyval = op; // Dummy
        }
        else
        {
            yyval = 0;
        }
    }
    else
    {
        yyval = 0;
    }
}
#line 3294 "src/Slice/Grammar.cpp"
    break;

  case 118:
#line 1400 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->type, returnType->isTagged, returnType->tag,
                                                Operation::Idempotent);
        if(op)
        {
            unit->pushContainer(op);
            unit->error("keyword `" + name + "' cannot be used as operation name");
            yyval = op; // Dummy
        }
        else
        {
            return 0;
        }
    }
    else
    {
        yyval = 0;
    }
}
#line 3323 "src/Slice/Grammar.cpp"
    break;

  case 119:
#line 1430 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
        yyval = yyvsp[-2];
    }
    else
    {
        yyval = 0;
    }
}
#line 3339 "src/Slice/Grammar.cpp"
    break;

  case 120:
#line 1442 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3353 "src/Slice/Grammar.cpp"
    break;

  case 121:
#line 1452 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3365 "src/Slice/Grammar.cpp"
    break;

  case 122:
#line 1460 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3379 "src/Slice/Grammar.cpp"
    break;

  case 125:
#line 1482 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3387 "src/Slice/Grammar.cpp"
    break;

  case 126:
#line 1486 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3397 "src/Slice/Grammar.cpp"
    break;

  case 127:
#line 1497 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3409 "src/Slice/Grammar.cpp"
    break;

  case 128:
#line 1510 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-1]);
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast(yyvsp[0]);
    ClassDefPtr cl = cont->createClassDef(ident->v, -1, true, bases->v);
    if(cl)
    {
        cont->checkIntroduced(ident->v, cl);
        unit->pushContainer(cl);
        yyval = cl;
    }
    else
    {
        yyval = 0;
    }
}
#line 3430 "src/Slice/Grammar.cpp"
    break;

  case 129:
#line 1527 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
        yyval = yyvsp[-3];
    }
    else
    {
        yyval = 0;
    }
}
#line 3446 "src/Slice/Grammar.cpp"
    break;

  case 130:
#line 1544 "src/Slice/Grammar.y"
{
    ClassListTokPtr intfs = ClassListTokPtr::dynamicCast(yyvsp[0]);
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-2]);
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
    yyval = intfs;
}
#line 3485 "src/Slice/Grammar.cpp"
    break;

  case 131:
#line 1579 "src/Slice/Grammar.y"
{
    ClassListTokPtr intfs = new ClassListTok;
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = intfs;
}
#line 3524 "src/Slice/Grammar.cpp"
    break;

  case 132:
#line 1614 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new ClassListTok; // Dummy
}
#line 3533 "src/Slice/Grammar.cpp"
    break;

  case 133:
#line 1619 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3542 "src/Slice/Grammar.cpp"
    break;

  case 134:
#line 1629 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3550 "src/Slice/Grammar.cpp"
    break;

  case 135:
#line 1633 "src/Slice/Grammar.y"
{
    yyval = new ClassListTok;
}
#line 3558 "src/Slice/Grammar.cpp"
    break;

  case 136:
#line 1642 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
    contained->setMetaData(metaData->v);
    }
}
#line 3571 "src/Slice/Grammar.cpp"
    break;

  case 137:
#line 1651 "src/Slice/Grammar.y"
{
}
#line 3578 "src/Slice/Grammar.cpp"
    break;

  case 138:
#line 1654 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3586 "src/Slice/Grammar.cpp"
    break;

  case 139:
#line 1658 "src/Slice/Grammar.y"
{
}
#line 3593 "src/Slice/Grammar.cpp"
    break;

  case 141:
#line 1672 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3604 "src/Slice/Grammar.cpp"
    break;

  case 142:
#line 1679 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3615 "src/Slice/Grammar.cpp"
    break;

  case 143:
#line 1691 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if(!exception)
    {
        exception = cont->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    yyval = exception;
}
#line 3631 "src/Slice/Grammar.cpp"
    break;

  case 144:
#line 1703 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3641 "src/Slice/Grammar.cpp"
    break;

  case 145:
#line 1714 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3653 "src/Slice/Grammar.cpp"
    break;

  case 146:
#line 1722 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3666 "src/Slice/Grammar.cpp"
    break;

  case 147:
#line 1736 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3680 "src/Slice/Grammar.cpp"
    break;

  case 148:
#line 1746 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3695 "src/Slice/Grammar.cpp"
    break;

  case 149:
#line 1762 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3703 "src/Slice/Grammar.cpp"
    break;

  case 150:
#line 1766 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3713 "src/Slice/Grammar.cpp"
    break;

  case 151:
#line 1777 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = en;
}
#line 3733 "src/Slice/Grammar.cpp"
    break;

  case 152:
#line 1793 "src/Slice/Grammar.y"
{
    EnumPtr en = EnumPtr::dynamicCast(yyvsp[-3]);
    if(en)
    {
        EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast(yyvsp[-1]);
        if(enumerators->v.empty())
        {
            unit->error("enum `" + en->name() + "' must have at least one enumerator");
        }
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 3751 "src/Slice/Grammar.cpp"
    break;

  case 153:
#line 1808 "src/Slice/Grammar.y"
{
    unit->error("missing enumeration name");
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3763 "src/Slice/Grammar.cpp"
    break;

  case 154:
#line 1816 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3772 "src/Slice/Grammar.cpp"
    break;

  case 155:
#line 1826 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 3782 "src/Slice/Grammar.cpp"
    break;

  case 156:
#line 1832 "src/Slice/Grammar.y"
{
}
#line 3789 "src/Slice/Grammar.cpp"
    break;

  case 157:
#line 1840 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    ContainerPtr cont = unit->currentContainer();
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if(en)
    {
        ens->v.push_front(en);
    }
    yyval = ens;
}
#line 3805 "src/Slice/Grammar.cpp"
    break;

  case 158:
#line 1852 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    ContainerPtr cont = unit->currentContainer();
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = ens;
}
#line 3829 "src/Slice/Grammar.cpp"
    break;

  case 159:
#line 1872 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 3840 "src/Slice/Grammar.cpp"
    break;

  case 160:
#line 1879 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 3849 "src/Slice/Grammar.cpp"
    break;

  case 161:
#line 1889 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3857 "src/Slice/Grammar.cpp"
    break;

  case 162:
#line 1893 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
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

    yyval = tok;
}
#line 3895 "src/Slice/Grammar.cpp"
    break;

  case 163:
#line 1932 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = true;
    yyval = out;
}
#line 3905 "src/Slice/Grammar.cpp"
    break;

  case 164:
#line 1938 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = false;
    yyval = out;
}
#line 3915 "src/Slice/Grammar.cpp"
    break;

  case 165:
#line 1949 "src/Slice/Grammar.y"
{
}
#line 3922 "src/Slice/Grammar.cpp"
    break;

  case 166:
#line 1952 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    TaggedDefTokPtr tsp = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isTagged, tsp->tag);
        unit->currentContainer()->checkIntroduced(tsp->name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 3942 "src/Slice/Grammar.cpp"
    break;

  case 167:
#line 1968 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    TaggedDefTokPtr tsp = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isTagged, tsp->tag);
        unit->currentContainer()->checkIntroduced(tsp->name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 3962 "src/Slice/Grammar.cpp"
    break;

  case 168:
#line 1984 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
        unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
#line 3978 "src/Slice/Grammar.cpp"
    break;

  case 169:
#line 1996 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
        unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
#line 3994 "src/Slice/Grammar.cpp"
    break;

  case 170:
#line 2008 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        unit->error("missing parameter name");
    }
}
#line 4009 "src/Slice/Grammar.cpp"
    break;

  case 171:
#line 2019 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        unit->error("missing parameter name");
    }
}
#line 4024 "src/Slice/Grammar.cpp"
    break;

  case 172:
#line 2035 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4032 "src/Slice/Grammar.cpp"
    break;

  case 173:
#line 2039 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 4040 "src/Slice/Grammar.cpp"
    break;

  case 174:
#line 2048 "src/Slice/Grammar.y"
{
}
#line 4047 "src/Slice/Grammar.cpp"
    break;

  case 175:
#line 2051 "src/Slice/Grammar.y"
{
}
#line 4054 "src/Slice/Grammar.cpp"
    break;

  case 176:
#line 2059 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindByte);
}
#line 4062 "src/Slice/Grammar.cpp"
    break;

  case 177:
#line 2063 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindByte);
}
#line 4070 "src/Slice/Grammar.cpp"
    break;

  case 178:
#line 2067 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindBool);
}
#line 4078 "src/Slice/Grammar.cpp"
    break;

  case 179:
#line 2071 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindBool);
}
#line 4086 "src/Slice/Grammar.cpp"
    break;

  case 180:
#line 2075 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindShort);
}
#line 4094 "src/Slice/Grammar.cpp"
    break;

  case 181:
#line 2079 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindShort);
}
#line 4102 "src/Slice/Grammar.cpp"
    break;

  case 182:
#line 2083 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindInt);
}
#line 4110 "src/Slice/Grammar.cpp"
    break;

  case 183:
#line 2087 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindInt);
}
#line 4118 "src/Slice/Grammar.cpp"
    break;

  case 184:
#line 2091 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindLong);
}
#line 4126 "src/Slice/Grammar.cpp"
    break;

  case 185:
#line 2095 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindLong);
}
#line 4134 "src/Slice/Grammar.cpp"
    break;

  case 186:
#line 2099 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindFloat);
}
#line 4142 "src/Slice/Grammar.cpp"
    break;

  case 187:
#line 2103 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindFloat);
}
#line 4150 "src/Slice/Grammar.cpp"
    break;

  case 188:
#line 2107 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindDouble);
}
#line 4158 "src/Slice/Grammar.cpp"
    break;

  case 189:
#line 2111 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindDouble);
}
#line 4166 "src/Slice/Grammar.cpp"
    break;

  case 190:
#line 2115 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindString);
}
#line 4174 "src/Slice/Grammar.cpp"
    break;

  case 191:
#line 2119 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindString);
}
#line 4182 "src/Slice/Grammar.cpp"
    break;

  case 192:
#line 2123 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindObject);
}
#line 4190 "src/Slice/Grammar.cpp"
    break;

  case 193:
#line 2127 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObjectProxy);
}
#line 4198 "src/Slice/Grammar.cpp"
    break;

  case 194:
#line 2131 "src/Slice/Grammar.y"
{
    // TODO: equivalent to ICE_OBJECT ? above, need to merge KindObject / KindObjectProxy
    yyval = unit->builtin(Builtin::KindObjectProxy);
}
#line 4207 "src/Slice/Grammar.cpp"
    break;

  case 195:
#line 2136 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindValue);
}
#line 4215 "src/Slice/Grammar.cpp"
    break;

  case 196:
#line 2140 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindValue);
}
#line 4223 "src/Slice/Grammar.cpp"
    break;

  case 197:
#line 2144 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    if(cont)
    {
        TypeList types = cont->lookupType(scoped->v);
        if(types.empty())
        {
            YYERROR; // Can't continue, jump to next yyerrok
        }
        cont->checkIntroduced(scoped->v);
        yyval = types.front();
    }
    else
    {
        yyval = 0;
    }
}
#line 4246 "src/Slice/Grammar.cpp"
    break;

  case 198:
#line 2163 "src/Slice/Grammar.y"
{
    // TODO: keep '*' only as an alias for T? where T = interface
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-1]);
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
        yyval = types.front();
    }
    else
    {
        yyval = 0;
    }
}
#line 4283 "src/Slice/Grammar.cpp"
    break;

  case 199:
#line 2196 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-1]);
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
        yyval = types.front();
    }
    else
    {
        yyval = 0;
    }
}
#line 4310 "src/Slice/Grammar.cpp"
    break;

  case 200:
#line 2224 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4320 "src/Slice/Grammar.cpp"
    break;

  case 201:
#line 2230 "src/Slice/Grammar.y"
{
}
#line 4327 "src/Slice/Grammar.cpp"
    break;

  case 202:
#line 2238 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4338 "src/Slice/Grammar.cpp"
    break;

  case 203:
#line 2245 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4349 "src/Slice/Grammar.cpp"
    break;

  case 204:
#line 2257 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4362 "src/Slice/Grammar.cpp"
    break;

  case 205:
#line 2266 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4375 "src/Slice/Grammar.cpp"
    break;

  case 206:
#line 2275 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = def;
}
#line 4419 "src/Slice/Grammar.cpp"
    break;

  case 207:
#line 2315 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4430 "src/Slice/Grammar.cpp"
    break;

  case 208:
#line 2322 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4441 "src/Slice/Grammar.cpp"
    break;

  case 209:
#line 2329 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4452 "src/Slice/Grammar.cpp"
    break;

  case 210:
#line 2341 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4465 "src/Slice/Grammar.cpp"
    break;

  case 211:
#line 2350 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4478 "src/Slice/Grammar.cpp"
    break;

  case 212:
#line 2364 "src/Slice/Grammar.y"
{
}
#line 4485 "src/Slice/Grammar.cpp"
    break;

  case 213:
#line 2367 "src/Slice/Grammar.y"
{
}
#line 4492 "src/Slice/Grammar.cpp"
    break;

  case 214:
#line 2370 "src/Slice/Grammar.y"
{
}
#line 4499 "src/Slice/Grammar.cpp"
    break;

  case 215:
#line 2373 "src/Slice/Grammar.y"
{
}
#line 4506 "src/Slice/Grammar.cpp"
    break;

  case 216:
#line 2376 "src/Slice/Grammar.y"
{
}
#line 4513 "src/Slice/Grammar.cpp"
    break;

  case 217:
#line 2379 "src/Slice/Grammar.y"
{
}
#line 4520 "src/Slice/Grammar.cpp"
    break;

  case 218:
#line 2382 "src/Slice/Grammar.y"
{
}
#line 4527 "src/Slice/Grammar.cpp"
    break;

  case 219:
#line 2385 "src/Slice/Grammar.y"
{
}
#line 4534 "src/Slice/Grammar.cpp"
    break;

  case 220:
#line 2388 "src/Slice/Grammar.y"
{
}
#line 4541 "src/Slice/Grammar.cpp"
    break;

  case 221:
#line 2391 "src/Slice/Grammar.y"
{
}
#line 4548 "src/Slice/Grammar.cpp"
    break;

  case 222:
#line 2394 "src/Slice/Grammar.y"
{
}
#line 4555 "src/Slice/Grammar.cpp"
    break;

  case 223:
#line 2397 "src/Slice/Grammar.y"
{
}
#line 4562 "src/Slice/Grammar.cpp"
    break;

  case 224:
#line 2400 "src/Slice/Grammar.y"
{
}
#line 4569 "src/Slice/Grammar.cpp"
    break;

  case 225:
#line 2403 "src/Slice/Grammar.y"
{
}
#line 4576 "src/Slice/Grammar.cpp"
    break;

  case 226:
#line 2406 "src/Slice/Grammar.y"
{
}
#line 4583 "src/Slice/Grammar.cpp"
    break;

  case 227:
#line 2409 "src/Slice/Grammar.y"
{
}
#line 4590 "src/Slice/Grammar.cpp"
    break;

  case 228:
#line 2412 "src/Slice/Grammar.y"
{
}
#line 4597 "src/Slice/Grammar.cpp"
    break;

  case 229:
#line 2415 "src/Slice/Grammar.y"
{
}
#line 4604 "src/Slice/Grammar.cpp"
    break;

  case 230:
#line 2418 "src/Slice/Grammar.y"
{
}
#line 4611 "src/Slice/Grammar.cpp"
    break;

  case 231:
#line 2421 "src/Slice/Grammar.y"
{
}
#line 4618 "src/Slice/Grammar.cpp"
    break;

  case 232:
#line 2424 "src/Slice/Grammar.y"
{
}
#line 4625 "src/Slice/Grammar.cpp"
    break;

  case 233:
#line 2427 "src/Slice/Grammar.y"
{
}
#line 4632 "src/Slice/Grammar.cpp"
    break;

  case 234:
#line 2430 "src/Slice/Grammar.y"
{
}
#line 4639 "src/Slice/Grammar.cpp"
    break;

  case 235:
#line 2433 "src/Slice/Grammar.y"
{
}
#line 4646 "src/Slice/Grammar.cpp"
    break;

  case 236:
#line 2436 "src/Slice/Grammar.y"
{
}
#line 4653 "src/Slice/Grammar.cpp"
    break;

  case 237:
#line 2439 "src/Slice/Grammar.y"
{
}
#line 4660 "src/Slice/Grammar.cpp"
    break;

  case 238:
#line 2442 "src/Slice/Grammar.y"
{
}
#line 4667 "src/Slice/Grammar.cpp"
    break;

  case 239:
#line 2445 "src/Slice/Grammar.y"
{
}
#line 4674 "src/Slice/Grammar.cpp"
    break;

  case 240:
#line 2448 "src/Slice/Grammar.y"
{
}
#line 4681 "src/Slice/Grammar.cpp"
    break;


#line 4685 "src/Slice/Grammar.cpp"

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
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

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
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

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


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
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
#line 2452 "src/Slice/Grammar.y"

