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


/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yydebug         slice_debug
#define yynerrs         slice_nerrs


/* Copy the first part of user declarations.  */
#line 1 "../Slice/Grammar.y" /* yacc.c:339  */


// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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


#line 124 "Grammar.tab.c" /* yacc.c:339  */

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
#ifndef YY_SLICE_GRAMMAR_TAB_H_INCLUDED
# define YY_SLICE_GRAMMAR_TAB_H_INCLUDED
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

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int slice_parse (void);

#endif /* !YY_SLICE_GRAMMAR_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 219 "Grammar.tab.c" /* yacc.c:358  */

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
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   886

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  72
/* YYNRULES -- Number of rules.  */
#define YYNRULES  202
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  301

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   299

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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
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
  "local_qualifier", "const_initializer", "const_def", "keyword", YY_NULLPTR
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
     295,   296,   297,   298,   299,    59,   123,   125,    41,    61,
      44,    60,    62,    42
};
# endif

#define YYPACT_NINF -227

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-227)))

#define YYTABLE_NINF -136

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
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

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
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

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
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

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
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
#line 115 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1664 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 123 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[-1]);
}
#line 1672 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 132 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[-1]);
}
#line 1680 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 136 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new StringListTok;
}
#line 1688 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 145 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[0]));
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
#line 1700 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 154 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-1]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[0]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
#line 1713 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 164 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    yyerrok;
}
#line 1721 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 169 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 1729 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 173 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1736 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 181 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ModulePtr::dynamicCast((yyvsp[0])));
}
#line 1744 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 185 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[0])));
}
#line 1752 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 189 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDefPtr::dynamicCast((yyvsp[0])));
}
#line 1760 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 193 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[0])));
}
#line 1768 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 197 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDefPtr::dynamicCast((yyvsp[0])));
}
#line 1776 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 201 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0);
}
#line 1784 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 205 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ExceptionPtr::dynamicCast((yyvsp[0])));
}
#line 1792 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 209 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0);
}
#line 1800 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 213 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || StructPtr::dynamicCast((yyvsp[0])));
}
#line 1808 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 217 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || SequencePtr::dynamicCast((yyvsp[0])));
}
#line 1816 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 221 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || DictionaryPtr::dynamicCast((yyvsp[0])));
}
#line 1824 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 225 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || EnumPtr::dynamicCast((yyvsp[0])));
}
#line 1832 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 229 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ConstPtr::dynamicCast((yyvsp[0])));
}
#line 1840 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 238 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->setSeenDefinition();
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
#line 1861 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 255 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 1877 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 272 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 1885 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 276 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 1895 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 287 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("exceptions cannot be forward declared");
    (yyval) = 0;
}
#line 1904 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 297 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-2]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[-1]));
    ExceptionPtr base = ExceptionPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base, local->v);
    if(ex)
    {
	cont->checkIntroduced(ident->v, ex);
	unit->pushContainer(ex);
    }
    (yyval) = ex;
}
#line 1922 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 311 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-3]))
    {
	unit->popContainer();
    }
    (yyval) = (yyvsp[-3]);
}
#line 1934 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 324 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    (yyval) = contained;
}
#line 1946 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 332 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = 0;
}
#line 1954 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 341 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
#line 1967 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 350 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1974 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 353 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 1982 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 357 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1989 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 365 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    (yyval) = typestring;
}
#line 2001 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 378 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    IntegerTokPtr i = IntegerTokPtr::dynamicCast((yyvsp[-1]));

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
#line 2025 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 398 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-1]));

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
}
#line 2085 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 454 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2097 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 462 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2109 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 475 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[-1]));
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    (yyval) = m;
}
#line 2121 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 483 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2135 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 504 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2143 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 508 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2153 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 519 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("structs cannot be forward declared");
    (yyval) = 0; // Dummy
}
#line 2162 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 529 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
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
#line 2185 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 548 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2207 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 571 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
#line 2220 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 580 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2227 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 583 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 2235 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 587 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2242 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 601 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2250 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 605 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2260 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 616 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2289 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 641 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-1]));

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
    classId->v = StringTokPtr::dynamicCast((yyvsp[-2]))->v;
    classId->t = id;
    (yyval) = classId;

}
#line 2358 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 706 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    classId->t = -1;
    (yyval) = classId;
}
#line 2369 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 718 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false, local->v);
    (yyval) = cl;
}
#line 2381 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 731 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-3]));
    ClassIdTokPtr ident = ClassIdTokPtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast((yyvsp[-1]));
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast((yyvsp[0]));
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
#line 2408 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 754 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2424 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 771 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2462 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 805 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = 0;
}
#line 2470 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 814 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2478 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 818 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ClassListTok;
}
#line 2486 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 827 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
#line 2499 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 836 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2506 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 839 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 2514 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 843 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2521 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 851 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr def = OptionalDefTokPtr::dynamicCast((yyvsp[0]));
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
#line 2547 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 873 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr def = OptionalDefTokPtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));

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
#line 2578 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 900 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2604 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 922 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2629 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 948 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
}
#line 2642 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 957 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, value->v.value,
                                            value->v.valueAsString, value->v.valueAsLiteral);
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
}
#line 2657 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 968 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
}
#line 2670 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 977 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[-2]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
}
#line 2683 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 986 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 2697 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 996 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("missing data member name");
}
#line 2710 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 1010 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[-1]));
    m->v.type = TypePtr::dynamicCast((yyvsp[0]));
    (yyval) = m;
}
#line 2720 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 1016 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr m = new OptionalDefTok();
    m->v.type = TypePtr::dynamicCast((yyvsp[0]));
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2732 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 1024 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2743 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 1036 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
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
#line 2771 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 1060 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
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
#line 2800 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 1085 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
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
#line 2828 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 1109 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
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
#line 2857 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 1139 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2873 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 1151 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[-1]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 2887 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1161 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-2]))
    {
	unit->popContainer();
    }
    yyerrok;
}
#line 2899 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1169 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[-1]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 2913 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1191 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2921 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1195 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2931 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 1206 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    cont->checkIntroduced(ident->v, cl);
    (yyval) = cl;
}
#line 2944 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1220 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-2]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[-1]));
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast((yyvsp[0]));
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
#line 2966 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1238 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2982 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1255 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3021 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1290 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3060 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1325 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("illegal inheritance from type Object");
    (yyval) = new ClassListTok; // Dummy
}
#line 3069 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1335 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3077 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1339 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ClassListTok;
}
#line 3085 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1348 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
	contained->setMetaData(metaData->v);
    }
}
#line 3098 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1357 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3105 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1360 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 3113 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1364 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3120 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1378 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[-2]));
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
}
#line 3131 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1385 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[0]));
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
}
#line 3142 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1397 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if(!exception)
    {
	exception = cont->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    (yyval) = exception;
}
#line 3158 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1409 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
}
#line 3168 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1420 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-6]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v);
}
#line 3181 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1429 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-6]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3195 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1444 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-9]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast((yyvsp[-6]));
    TypePtr keyType = TypePtr::dynamicCast((yyvsp[-5]));
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr valueType = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v);
}
#line 3210 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1455 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-9]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast((yyvsp[-6]));
    TypePtr keyType = TypePtr::dynamicCast((yyvsp[-5]));
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr valueType = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3226 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1472 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3234 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1476 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 3244 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1487 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    cont->checkIntroduced(ident->v, en);
    (yyval) = en;
}
#line 3257 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1496 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    EnumPtr en = EnumPtr::dynamicCast((yyvsp[-3]));
    if(en)
    {
	EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast((yyvsp[-1]));
	if(enumerators->v.empty())
	{
	    unit->error("enum `" + en->name() + "' must have at least one enumerator");
	}
	en->setEnumerators(enumerators->v); // Dummy
    }
    (yyval) = (yyvsp[-3]);
}
#line 3275 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1511 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing enumeration name");
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-4]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), local->v, Dummy); // Dummy
    EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast((yyvsp[-1]));
    en->setEnumerators(enumerators->v); // Dummy
    (yyval) = en;
}
#line 3289 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1526 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast((yyvsp[-2]));
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast((yyvsp[0]))->v);
    (yyval) = ens;
}
#line 3299 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1532 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3306 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1540 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3322 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1552 "../Slice/Grammar.y" /* yacc.c:1646  */
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
            if(en)
            {
                ens->v.push_front(en);
            }
        }
    }
    (yyval) = ens;
}
#line 3349 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1575 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    (yyval) = ens;
}
#line 3360 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1582 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    (yyval) = ens; // Dummy
}
#line 3369 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1592 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3377 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1596 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3415 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1635 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr out = new BoolTok;
    out->v = true;
    (yyval) = out;
}
#line 3425 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1641 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr out = new BoolTok;
    out->v = false;
    (yyval) = out;
}
#line 3435 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1652 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3442 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1655 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[-2]));
    OptionalDefTokPtr tsp = OptionalDefTokPtr::dynamicCast((yyvsp[0]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	ParamDeclPtr pd = op->createParamDecl(tsp->v.name, tsp->v.type, isOutParam->v, tsp->v.optional, tsp->v.tag);
	unit->currentContainer()->checkIntroduced(tsp->v.name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-1]));
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 3462 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1671 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast((yyvsp[-2]));
    OptionalDefTokPtr tsp = OptionalDefTokPtr::dynamicCast((yyvsp[0]));
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
	ParamDeclPtr pd = op->createParamDecl(tsp->v.name, tsp->v.type, isOutParam->v, tsp->v.optional, tsp->v.tag);
	unit->currentContainer()->checkIntroduced(tsp->v.name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-1]));
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 3482 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1687 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3498 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1699 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3514 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1711 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3529 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1722 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3544 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1738 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3552 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1742 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ExceptionListTok;
}
#line 3560 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1751 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3567 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1754 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ident->v = "::" + ident->v;
    (yyval) = ident;
}
#line 3577 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1760 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-2]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    scoped->v += "::";
    scoped->v += ident->v;
    (yyval) = scoped;
}
#line 3589 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1773 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindByte);
}
#line 3597 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1777 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindBool);
}
#line 3605 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1781 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindShort);
}
#line 3613 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1785 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindInt);
}
#line 3621 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1789 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindLong);
}
#line 3629 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1793 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindFloat);
}
#line 3637 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1797 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindDouble);
}
#line 3645 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1801 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindString);
}
#line 3653 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1805 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindObject);
}
#line 3661 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1809 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindObjectProxy);
}
#line 3669 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1813 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindLocalObject);
}
#line 3677 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1817 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3700 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1836 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3740 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1877 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str1 = StringTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr str2 = StringTokPtr::dynamicCast((yyvsp[0]));
    str1->v += str2->v;
}
#line 3750 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1883 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3757 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1891 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr stringList = StringListTokPtr::dynamicCast((yyvsp[-2]));
    stringList->v.push_back(str->v);
    (yyval) = stringList;
}
#line 3768 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1898 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    (yyval) = stringList;
}
#line 3779 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1910 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    (yyval) = local;
}
#line 3789 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1916 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    (yyval) = local;
}
#line 3799 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1927 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast((yyvsp[0]));
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = sstr.str();
    def->v.valueAsLiteral = intVal->literal;
    (yyval) = def;
}
#line 3816 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1940 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast((yyvsp[0]));
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = sstr.str();
    def->v.valueAsLiteral = floatVal->literal;
    (yyval) = def;
}
#line 3833 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1953 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
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
}
#line 3883 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1999 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[0]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = literal->v;
    def->v.valueAsLiteral = literal->literal;
    (yyval) = def;
}
#line 3898 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 2010 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[0]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "false";
    def->v.valueAsLiteral = "false";
    (yyval) = def;
}
#line 3913 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 2021 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast((yyvsp[0]));
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "true";
    def->v.valueAsLiteral = "true";
    (yyval) = def;
}
#line 3928 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 2037 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-4]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[-3]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));
    (yyval) = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral);
}
#line 3941 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 2046 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));
    unit->error("missing constant name");
    (yyval) = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral, Dummy); // Dummy
}
#line 3954 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 2060 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3961 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 2063 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3968 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 2066 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3975 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 2069 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3982 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 2072 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3989 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 2075 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3996 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 2078 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4003 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 2081 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4010 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 2084 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4017 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 2087 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4024 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 2090 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4031 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 2093 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4038 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 2096 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4045 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 2099 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4052 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 2102 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4059 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 2105 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4066 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 2108 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4073 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 2111 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4080 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 2114 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4087 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 2117 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4094 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 2120 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4101 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 2123 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4108 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 2126 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4115 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 2129 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4122 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 2132 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4129 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 2135 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4136 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 2138 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4143 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 2141 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4150 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 2144 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4157 "Grammar.tab.c" /* yacc.c:1646  */
    break;


#line 4161 "Grammar.tab.c" /* yacc.c:1646  */
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
#line 2148 "../Slice/Grammar.y" /* yacc.c:1906  */

