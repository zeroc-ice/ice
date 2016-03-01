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
    ICE_VALUE = 287,
    ICE_SCOPE_DELIMITER = 288,
    ICE_IDENTIFIER = 289,
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

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int slice_parse (void);

#endif /* !YY_SLICE_GRAMMAR_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 220 "Grammar.tab.c" /* yacc.c:358  */

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
#define YYLAST   920

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  55
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  72
/* YYNRULES -- Number of rules.  */
#define YYNRULES  205
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  304

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   300

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
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   115,   115,   123,   132,   137,   146,   145,   155,   154,
     165,   164,   169,   174,   181,   185,   189,   193,   197,   201,
     205,   209,   213,   217,   221,   225,   229,   239,   238,   272,
     276,   287,   298,   297,   324,   333,   341,   350,   353,   358,
     365,   378,   398,   454,   462,   475,   483,   498,   504,   508,
     519,   530,   529,   571,   580,   583,   588,   595,   601,   605,
     616,   641,   706,   718,   732,   731,   771,   806,   814,   819,
     827,   836,   839,   844,   851,   873,   900,   922,   948,   957,
     968,   977,   986,   996,  1010,  1016,  1024,  1036,  1060,  1085,
    1109,  1140,  1139,  1162,  1161,  1184,  1185,  1191,  1195,  1206,
    1221,  1220,  1255,  1290,  1325,  1330,  1340,  1345,  1353,  1362,
    1365,  1370,  1377,  1383,  1390,  1402,  1414,  1425,  1434,  1449,
    1460,  1477,  1481,  1493,  1492,  1516,  1531,  1537,  1545,  1557,
    1580,  1588,  1597,  1601,  1640,  1647,  1658,  1660,  1676,  1692,
    1704,  1716,  1727,  1743,  1748,  1756,  1759,  1765,  1778,  1782,
    1786,  1790,  1794,  1798,  1802,  1806,  1810,  1814,  1818,  1822,
    1826,  1845,  1886,  1892,  1900,  1907,  1919,  1926,  1936,  1949,
    1962,  2008,  2019,  2030,  2046,  2055,  2069,  2072,  2075,  2078,
    2081,  2084,  2087,  2090,  2093,  2096,  2099,  2102,  2105,  2108,
    2111,  2114,  2117,  2120,  2123,  2126,  2129,  2132,  2135,  2138,
    2141,  2144,  2147,  2150,  2153,  2156
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
  "ICE_IDEMPOTENT", "ICE_OPTIONAL", "ICE_VALUE", "ICE_SCOPE_DELIMITER",
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
     295,   296,   297,   298,   299,   300,    59,   123,   125,    41,
      61,    44,    60,    62,    42
};
# endif

#define YYPACT_NINF -221

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-221)))

#define YYTABLE_NINF -137

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     523,   -16,     2,     2,    34,  -221,    19,  -221,  -221,     2,
    -221,    24,   -15,  -221,   115,    43,  -221,    40,    39,  -221,
    -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,
    -221,   124,  -221,   115,  -221,  -221,     2,  -221,  -221,  -221,
     221,    65,   601,   669,   701,   733,    60,    61,   488,     3,
      79,     9,   123,     4,  -221,  -221,  -221,    89,  -221,  -221,
    -221,  -221,  -221,  -221,  -221,  -221,    83,  -221,  -221,   104,
    -221,     6,   -10,   115,  -221,  -221,  -221,  -221,  -221,  -221,
    -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,
    -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,  -221,
    -221,  -221,  -221,  -221,  -221,    -1,  -221,  -221,  -221,  -221,
    -221,  -221,  -221,    40,    40,  -221,   765,  -221,    46,  -221,
      93,    46,   130,    59,  -221,   100,   442,  -221,  -221,   114,
    -221,    99,    66,  -221,   101,    10,   221,   221,   102,   103,
     106,  -221,   120,   108,   362,   120,    59,  -221,  -221,  -221,
    -221,    -7,   112,   765,   116,  -221,    66,  -221,  -221,  -221,
    -221,  -221,   120,  -221,  -221,  -221,   117,   118,    54,  -221,
     765,   396,   119,   880,   125,  -221,   121,    59,   294,   127,
    -221,  -221,   797,    40,  -221,  -221,   120,  -221,   126,   880,
     131,   362,  -221,    35,   128,   221,   134,  -221,   829,  -221,
     328,  -221,   136,   440,   137,  -221,  -221,  -221,   221,   396,
    -221,   221,   138,   141,  -221,   829,  -221,  -221,   122,  -221,
      29,    66,   139,   133,   362,  -221,  -221,   144,   440,   143,
     294,  -221,   521,   221,    67,   257,  -221,   147,  -221,  -221,
     142,  -221,  -221,    66,   396,  -221,  -221,  -221,  -221,    66,
    -221,   328,   221,  -221,  -221,   148,   564,  -221,  -221,    69,
    -221,  -221,  -221,   135,  -221,    40,    23,   294,   861,  -221,
    -221,  -221,  -221,   133,   328,  -221,  -221,  -221,   880,  -221,
     185,  -221,  -221,  -221,  -221,   184,  -221,   829,   184,    40,
     637,  -221,  -221,  -221,   880,  -221,   149,   120,  -221,  -221,
     829,   637,  -221,  -221
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     6,   167,     2,    10,   163,
     165,     0,     0,     1,     0,     0,   166,     5,    12,    14,
      19,    20,    21,    22,    15,    16,    17,    18,    23,    24,
      25,     0,    26,     0,   162,     4,     0,     3,     7,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    31,
      50,    62,    67,    99,   123,    11,   164,     0,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   158,   159,     0,
     145,   160,     0,     0,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,    58,     0,    59,    97,    98,    29,
      30,    48,    49,     5,     5,   121,   131,   122,     0,    32,
       0,     0,    69,     0,   100,     0,     0,   157,   146,     0,
     161,     0,     0,     9,     0,     0,     0,     0,   128,     0,
     127,   130,    34,     0,     0,    66,     0,    64,   104,   105,
     106,   103,     0,   131,     0,   147,     0,   172,   173,   171,
     168,   169,   170,   175,    60,    61,     0,     0,     0,   125,
     131,     0,     0,     0,     0,    68,     0,     0,     0,     0,
      28,   174,     0,     5,   132,   129,   133,   126,     0,     0,
       0,     0,    44,     0,    78,     0,    55,    57,    83,    52,
       0,   102,     0,     0,     0,   124,   117,   118,     0,     0,
      46,     0,    74,    38,    47,    77,    33,    54,     0,    43,
       0,     0,    80,     0,     0,    40,    82,     0,     0,     0,
       0,    86,     0,     0,     0,     0,   112,   110,    85,   101,
       0,    37,    45,     0,     0,    76,    41,    42,    79,     0,
      53,     0,     0,    95,    96,    72,    77,    65,   109,     0,
      84,    87,    89,     0,   134,     5,     0,     0,     0,    75,
      36,    81,    71,    84,     0,    88,    90,    93,     0,    91,
     135,   108,   119,   120,    70,   144,   137,   141,   144,     5,
       0,    94,   139,    92,     0,   143,   114,   115,   116,   138,
     142,     0,   140,   113
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -221,  -221,  -221,   -17,   -12,  -221,  -221,  -221,  -221,  -221,
    -221,  -221,  -221,  -221,  -221,  -221,  -191,  -170,  -165,  -208,
    -221,  -221,  -221,  -221,  -221,  -168,  -221,  -221,  -221,  -221,
    -221,  -221,  -221,  -221,  -220,   -29,  -221,   -30,  -221,   -25,
    -221,  -221,  -221,  -221,  -221,  -221,  -221,  -119,  -221,  -218,
    -221,   -97,  -221,  -221,  -221,  -221,  -221,  -221,  -105,  -221,
    -221,   -72,  -221,   -79,  -104,   -27,    11,   207,  -221,  -145,
    -221,   -38
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,     6,     7,    14,    41,    33,    18,    19,
      57,    49,    20,    21,   143,   119,   190,   210,   211,   212,
     213,    50,    22,    23,   120,   174,   196,    51,    52,    24,
      25,   176,   122,   147,   229,   214,   197,   234,   235,   236,
     288,   285,   255,    53,    26,    27,   152,   150,   124,   204,
     237,   295,   296,    28,    29,    54,    30,   125,   139,   140,
     185,   265,   266,   291,    71,   223,    10,    11,    31,   163,
      32,   141
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      40,   135,    38,   194,   106,   108,   110,   112,   195,   -63,
     117,   181,   258,    72,   142,   118,   123,   145,   241,   151,
      34,    55,    15,   217,   131,   222,   129,   175,   162,    37,
       8,   272,    69,    70,    13,   134,    36,     9,   233,   129,
     132,   242,   151,   129,   177,    16,    17,    56,   179,   281,
     -35,  -107,   162,   270,   284,   -63,   250,   -63,   201,   165,
     130,   133,   129,   252,   186,   187,    35,   233,    69,    70,
     286,   218,   279,   151,   280,    36,   248,    39,   247,    69,
      70,     2,   242,   148,   219,    -8,   299,    69,    70,   220,
     184,   149,    69,    70,   157,   158,   136,   137,   269,    69,
      70,   159,   160,   161,   271,   261,   262,   275,   276,   166,
     167,    73,   113,   114,   154,   -13,     1,   162,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,   -51,   173,    42,    43,
      44,    45,    46,    47,    48,   121,   126,   127,   128,   162,
     144,    -5,    -5,   146,   207,   162,   198,   153,   155,   156,
     164,   169,   168,   129,   189,   171,     2,   170,     3,   178,
     226,   203,   215,   -13,   180,   191,   208,   225,   200,   183,
     182,   246,   209,   199,   173,   205,   238,   245,   221,   216,
     224,   240,   230,   228,   277,   239,   297,   244,   243,   249,
     251,   257,   189,   267,   274,   268,   264,   297,   290,   253,
     301,   256,   259,   254,   303,   238,   260,   173,   289,   293,
      12,     0,     0,   203,     0,     0,     0,     0,   245,     0,
       0,     0,     0,     0,     0,   273,     0,   189,     0,     0,
     283,     0,     0,     0,   228,     0,     0,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,   278,   292,
     203,   287,   298,    68,    69,    70,     0,   228,   263,     0,
       0,     0,   302,   298,     0,     0,     0,   300,   264,     0,
       0,     0,   294,  -135,  -135,  -135,  -135,  -135,  -135,  -135,
    -135,  -135,  -135,     0,     0,     0,     0,     0,  -135,  -135,
    -135,  -135,     0,     0,     0,   202,     0,  -135,  -135,     0,
       0,     0,     0,     0,     0,     0,  -136,     0,  -136,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
       0,     0,     0,     0,    -5,    -5,    -5,    -5,    -5,   227,
       0,     0,     0,     0,    -5,     2,     0,     0,     0,     0,
       0,     0,  -111,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,     0,     0,     0,     0,    -5,    -5,
      -5,    -5,    -5,   172,     0,     0,     0,     0,    -5,     2,
       0,     0,     0,     0,     0,     0,   -73,     0,    -5,    -5,
      -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,     0,     0,
       0,     0,     0,    -5,    -5,    -5,    -5,   188,     0,     0,
       0,     0,    -5,     2,     0,     0,     0,     0,     0,     0,
     -56,     0,    -5,    -5,    -5,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,     0,     0,     0,     0,     0,    -5,    -5,    -5,
      -5,     0,     0,     0,     0,     0,    -5,     2,     0,     0,
       0,     0,     0,     1,   -39,    -5,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,     0,     0,   231,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,     0,     0,    -5,    -5,
     232,   192,    68,    69,    70,     0,     0,     0,     0,     0,
     193,     0,     0,     2,     0,     3,     0,     0,     0,     0,
     -13,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,     0,   115,   -13,     1,     0,    -5,    -5,    -5,    -5,
      -5,    -5,    -5,    -5,     0,   116,   231,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,     0,    -5,
      -5,     0,   192,    68,    69,    70,     0,     0,     0,     0,
       0,   193,     0,     0,     2,     0,     3,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,     0,   225,     0,
       0,     0,   -85,   -85,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,     0,   104,     0,     0,     0,   105,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
      69,    70,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,     0,   107,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,     0,   109,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,     0,   111,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,     0,   138,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
       0,   206,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,     0,   225,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,     0,   282,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,     0,     0,     0,     0,
       0,   192,    68,    69,    70,     0,     0,     0,     0,     0,
     193
};

static const yytype_int16 yycheck[] =
{
      17,   105,    14,   173,    42,    43,    44,    45,   173,     0,
      48,   156,   230,    40,   118,    12,    12,   121,   209,   123,
       9,    33,     3,   191,    34,   195,    33,   146,   132,    44,
      46,   251,    33,    34,     0,    36,    51,    35,   203,    33,
      50,   211,   146,    33,    51,    26,    27,    36,   153,   267,
      47,    47,   156,   244,   274,    46,   224,    48,   177,    49,
      54,    73,    33,   228,   168,   170,    42,   232,    33,    34,
     278,    36,    49,   177,    51,    51,   221,    34,    49,    33,
      34,    41,   252,    24,    49,    46,   294,    33,    34,   193,
      36,    32,    33,    34,    28,    29,   113,   114,   243,    33,
      34,    35,    36,    37,   249,    38,    39,    38,    39,   136,
     137,    46,    52,    52,   126,     0,     1,   221,     3,     4,
       5,     6,     7,     8,     9,    10,    47,   144,     4,     5,
       6,     7,     8,     9,    10,    12,    47,    54,    34,   243,
      47,    26,    27,    13,   182,   249,   173,    47,    34,    50,
      49,    48,    50,    33,   171,    47,    41,    51,    43,    47,
     198,   178,   189,    48,    48,    46,   183,    34,    47,    51,
      53,    49,    46,    48,   191,    48,   203,   215,    50,    48,
      46,   208,    46,   200,    49,    48,   290,    46,    50,    50,
      46,    48,   209,    46,    46,    53,    11,   301,    14,   228,
      51,   228,   232,   228,   301,   232,   233,   224,   280,   288,
       3,    -1,    -1,   230,    -1,    -1,    -1,    -1,   256,    -1,
      -1,    -1,    -1,    -1,    -1,   252,    -1,   244,    -1,    -1,
     268,    -1,    -1,    -1,   251,    -1,    -1,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    -1,   265,   287,
     267,   278,   290,    32,    33,    34,    -1,   274,     1,    -1,
      -1,    -1,   300,   301,    -1,    -1,    -1,   294,    11,    -1,
      -1,    -1,   289,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,
      33,    34,    -1,    -1,    -1,     1,    -1,    40,    41,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    49,    -1,    51,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      -1,    -1,    -1,    -1,    30,    31,    32,    33,    34,     1,
      -1,    -1,    -1,    -1,    40,    41,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    30,    31,
      32,    33,    34,     1,    -1,    -1,    -1,    -1,    40,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    48,    -1,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    -1,
      -1,    -1,    -1,    31,    32,    33,    34,     1,    -1,    -1,
      -1,    -1,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    -1,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,
      34,    -1,    -1,    -1,    -1,    -1,    40,    41,    -1,    -1,
      -1,    -1,    -1,     1,    48,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    -1,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    26,    27,
      30,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    41,    -1,    43,    -1,    -1,    -1,    -1,
      48,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    -1,    34,     0,     1,    -1,     3,     4,     5,     6,
       7,     8,     9,    10,    -1,    47,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    -1,    -1,    26,
      27,    -1,    31,    32,    33,    34,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    41,    -1,    43,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    -1,    34,    -1,
      -1,    -1,    38,    39,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    -1,    -1,    -1,    38,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    -1,    34,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    -1,    34,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      -1,    34,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    34,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,
      -1,    31,    32,    33,    34,    -1,    -1,    -1,    -1,    -1,
      40
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    41,    43,    56,    57,    58,    59,    46,    35,
     121,   122,   122,     0,    60,     3,    26,    27,    63,    64,
      67,    68,    77,    78,    84,    85,    99,   100,   108,   109,
     111,   123,   125,    62,   121,    42,    51,    44,    59,    34,
      58,    61,     4,     5,     6,     7,     8,     9,    10,    66,
      76,    82,    83,    98,   110,    59,   121,    65,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    32,    33,
      34,   119,   120,    46,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    34,    38,   126,    34,   126,    34,
     126,    34,   126,    52,    52,    34,    47,   126,    12,    70,
      79,    12,    87,    12,   103,   112,    47,    54,    34,    33,
      54,    34,    50,    59,    36,   119,    58,    58,    34,   113,
     114,   126,   119,    69,    47,   119,    13,    88,    24,    32,
     102,   119,   101,    47,    59,    34,    50,    28,    29,    35,
      36,    37,   119,   124,    49,    49,   120,   120,    50,    48,
      51,    47,     1,    58,    80,   102,    86,    51,    47,   113,
      48,   124,    53,    51,    36,   115,   119,   113,     1,    58,
      71,    46,    31,    40,    72,    73,    81,    91,   120,    48,
      47,   102,     1,    58,   104,    48,    34,   126,    58,    46,
      72,    73,    74,    75,    90,   120,    48,    80,    36,    49,
     119,    50,    72,   120,    46,    34,   126,     1,    58,    89,
      46,    15,    30,    73,    92,    93,    94,   105,   120,    48,
     120,    71,    72,    50,    46,   126,    49,    49,   124,    50,
      80,    46,    73,    90,    94,    97,   120,    48,   104,    92,
     120,    38,    39,     1,    11,   116,   117,    46,    53,   124,
      71,   124,    89,   120,    46,    38,    39,    49,    58,    49,
      51,   104,    34,   126,    89,    96,    74,   120,    95,   116,
      14,   118,   126,   118,    58,   106,   107,   119,   126,    74,
     120,    51,   126,   106
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    55,    56,    57,    58,    58,    60,    59,    61,    59,
      62,    59,    59,    59,    63,    63,    63,    63,    63,    63,
      63,    63,    63,    63,    63,    63,    63,    65,    64,    66,
      66,    67,    69,    68,    70,    70,    71,    71,    71,    71,
      72,    73,    73,    73,    73,    74,    74,    75,    76,    76,
      77,    79,    78,    80,    80,    80,    80,    81,    82,    82,
      83,    83,    83,    84,    86,    85,    87,    87,    88,    88,
      89,    89,    89,    89,    90,    90,    90,    90,    91,    91,
      91,    91,    91,    91,    92,    92,    92,    93,    93,    93,
      93,    95,    94,    96,    94,    97,    97,    98,    98,    99,
     101,   100,   102,   102,   102,   102,   103,   103,   104,   104,
     104,   104,   105,   106,   106,   107,   107,   108,   108,   109,
     109,   110,   110,   112,   111,   111,   113,   113,   114,   114,
     114,   114,   115,   115,   116,   116,   117,   117,   117,   117,
     117,   117,   117,   118,   118,   119,   119,   119,   120,   120,
     120,   120,   120,   120,   120,   120,   120,   120,   120,   120,
     120,   120,   121,   121,   122,   122,   123,   123,   124,   124,
     124,   124,   124,   124,   125,   125,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     126,   126,   126,   126,   126,   126
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
       0,     7,     3,     1,     1,     1,     2,     0,     4,     3,
       2,     0,     1,     3,     1,     1,     1,     7,     7,    10,
      10,     2,     2,     0,     6,     5,     3,     1,     1,     3,
       1,     0,     1,     1,     1,     0,     0,     3,     5,     4,
       6,     3,     5,     2,     0,     1,     2,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       1,     2,     2,     1,     3,     1,     1,     0,     1,     1,
       1,     1,     1,     1,     6,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
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
#line 116 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1674 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 124 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[-1]);
}
#line 1682 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 133 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[-1]);
}
#line 1690 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 137 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new StringListTok;
}
#line 1698 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 146 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[0]));
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
#line 1710 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 155 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-1]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[0]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1723 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 165 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    yyerrok;
}
#line 1731 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 170 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 1739 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 174 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1746 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 182 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ModulePtr::dynamicCast((yyvsp[0])));
}
#line 1754 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 186 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[0])));
}
#line 1762 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 190 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDefPtr::dynamicCast((yyvsp[0])));
}
#line 1770 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 194 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDeclPtr::dynamicCast((yyvsp[0])));
}
#line 1778 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 198 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ClassDefPtr::dynamicCast((yyvsp[0])));
}
#line 1786 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 202 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0);
}
#line 1794 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 206 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ExceptionPtr::dynamicCast((yyvsp[0])));
}
#line 1802 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 210 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0);
}
#line 1810 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 214 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || StructPtr::dynamicCast((yyvsp[0])));
}
#line 1818 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 218 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || SequencePtr::dynamicCast((yyvsp[0])));
}
#line 1826 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 222 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || DictionaryPtr::dynamicCast((yyvsp[0])));
}
#line 1834 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 226 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || EnumPtr::dynamicCast((yyvsp[0])));
}
#line 1842 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 230 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    assert((yyvsp[0]) == 0 || ConstPtr::dynamicCast((yyvsp[0])));
}
#line 1850 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 239 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 1871 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 256 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 1887 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 273 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 1895 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 277 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 1905 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 288 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("exceptions cannot be forward declared");
    (yyval) = 0;
}
#line 1914 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 298 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 1932 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 312 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-3]))
    {
        unit->popContainer();
    }
    (yyval) = (yyvsp[-3]);
}
#line 1944 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 325 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    (yyval) = contained;
}
#line 1956 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 333 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = 0;
}
#line 1964 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 342 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1977 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 351 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1984 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 354 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 1992 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 358 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 1999 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 366 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    (yyval) = typestring;
}
#line 2011 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 379 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2035 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 399 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2095 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 455 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2107 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 463 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2119 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 476 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[-1]));
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    (yyval) = m;
}
#line 2131 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 484 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2145 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 505 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2153 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 509 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2163 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 520 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("structs cannot be forward declared");
    (yyval) = 0; // Dummy
}
#line 2172 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 530 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2195 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 549 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2217 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 572 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2230 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 581 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2237 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 584 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 2245 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 588 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2252 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 602 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2260 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 606 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2270 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 617 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2299 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 642 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2368 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 707 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    classId->t = -1;
    (yyval) = classId;
}
#line 2379 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 719 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false, local->v);
    (yyval) = cl;
}
#line 2391 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 732 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2418 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 755 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2434 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 772 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2472 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 806 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = 0;
}
#line 2480 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 815 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2488 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 819 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ClassListTok;
}
#line 2496 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 828 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2509 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 837 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2516 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 840 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 2524 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 844 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 2531 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 852 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2557 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 874 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2588 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 901 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2614 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 923 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2639 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 949 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    (yyval) = dm;
}
#line 2652 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 958 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2667 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 969 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
}
#line 2680 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 978 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast((yyvsp[-2]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("optional data members not supported in struct");
}
#line 2693 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 987 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[-1]));
    string name = StringTokPtr::dynamicCast((yyvsp[0]))->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 2707 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 997 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    TypePtr type = TypePtr::dynamicCast((yyvsp[0]));
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    (yyval) = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert((yyval));
    unit->error("missing data member name");
}
#line 2720 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 1011 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast((yyvsp[-1]));
    m->v.type = TypePtr::dynamicCast((yyvsp[0]));
    (yyval) = m;
}
#line 2730 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 1017 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr m = new OptionalDefTok();
    m->v.type = TypePtr::dynamicCast((yyvsp[0]));
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2742 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 1025 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = false;
    m->v.tag = -1;
    (yyval) = m;
}
#line 2753 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 1037 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2781 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 1061 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2810 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 1086 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2838 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 1110 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2867 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 1140 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2883 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 1152 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[-1]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 2897 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1162 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    if((yyvsp[-2]))
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 2909 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1170 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    OperationPtr op = OperationPtr::dynamicCast((yyvsp[-1]));
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 2923 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1192 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 2931 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1196 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 2941 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 1207 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    cont->checkIntroduced(ident->v, cl);
    (yyval) = cl;
}
#line 2954 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1221 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2976 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1239 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 2992 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1256 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3031 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1291 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3070 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1326 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("illegal inheritance from type Object");
    (yyval) = new ClassListTok; // Dummy
}
#line 3079 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1331 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("illegal inheritance from type Value");
    (yyval) = new ClassListTok; // Dummy
}
#line 3088 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1341 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3096 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1345 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ClassListTok;
}
#line 3104 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1354 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    ContainedPtr contained = ContainedPtr::dynamicCast((yyvsp[-2]));
    if(contained && !metaData->v.empty())
    {
    contained->setMetaData(metaData->v);
    }
}
#line 3117 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1363 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3124 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1366 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("`;' missing after definition");
}
#line 3132 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1370 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3139 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1384 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[-2]));
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast((yyvsp[0]));
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
}
#line 3150 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1391 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast((yyvsp[0]));
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    (yyval) = exceptionList;
}
#line 3161 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1403 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3177 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1415 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    (yyval) = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
}
#line 3187 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1426 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-6]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v);
}
#line 3200 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1435 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-6]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr type = TypePtr::dynamicCast((yyvsp[-2]));
    ContainerPtr cont = unit->currentContainer();
    (yyval) = cont->createSequence(ident->v, type, metaData->v, local->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3214 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1450 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3229 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1461 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3245 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1478 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3253 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1482 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    (yyval) = (yyvsp[0]); // Dummy
}
#line 3263 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1493 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    cont->checkIntroduced(ident->v, en);
    (yyval) = en;
}
#line 3276 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1502 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3294 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1517 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    unit->error("missing enumeration name");
    BoolTokPtr local = BoolTokPtr::dynamicCast((yyvsp[-4]));
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), local->v, Dummy); // Dummy
    EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast((yyvsp[-1]));
    en->setEnumerators(enumerators->v); // Dummy
    (yyval) = en;
}
#line 3308 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1532 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast((yyvsp[-2]));
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast((yyvsp[0]))->v);
    (yyval) = ens;
}
#line 3318 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1538 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3325 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1546 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3341 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1558 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3368 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1581 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    (yyval) = ens;
}
#line 3379 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1588 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    (yyval) = ens; // Dummy
}
#line 3388 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1598 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3396 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1602 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3434 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1641 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr out = new BoolTok;
    out->v = true;
    (yyval) = out;
}
#line 3444 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1647 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr out = new BoolTok;
    out->v = false;
    (yyval) = out;
}
#line 3454 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1658 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3461 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1661 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3481 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1677 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3501 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1693 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3517 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1705 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3533 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1717 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3548 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1728 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3563 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1744 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = (yyvsp[0]);
}
#line 3571 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1748 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = new ExceptionListTok;
}
#line 3579 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1757 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3586 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1760 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    ident->v = "::" + ident->v;
    (yyval) = ident;
}
#line 3596 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1766 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast((yyvsp[-2]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[0]));
    scoped->v += "::";
    scoped->v += ident->v;
    (yyval) = scoped;
}
#line 3608 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1779 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindByte);
}
#line 3616 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1783 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindBool);
}
#line 3624 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1787 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindShort);
}
#line 3632 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1791 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindInt);
}
#line 3640 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1795 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindLong);
}
#line 3648 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1799 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindFloat);
}
#line 3656 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1803 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindDouble);
}
#line 3664 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1807 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindString);
}
#line 3672 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1811 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindObject);
}
#line 3680 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1815 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindObjectProxy);
}
#line 3688 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1819 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindLocalObject);
}
#line 3696 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1823 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    (yyval) = unit->builtin(Builtin::KindValue);
}
#line 3704 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1827 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3727 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1846 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3767 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1887 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str1 = StringTokPtr::dynamicCast((yyvsp[-1]));
    StringTokPtr str2 = StringTokPtr::dynamicCast((yyvsp[0]));
    str1->v += str2->v;
}
#line 3777 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1893 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3784 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1901 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr stringList = StringListTokPtr::dynamicCast((yyvsp[-2]));
    stringList->v.push_back(str->v);
    (yyval) = stringList;
}
#line 3795 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1908 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringTokPtr str = StringTokPtr::dynamicCast((yyvsp[0]));
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    (yyval) = stringList;
}
#line 3806 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 1920 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    (yyval) = local;
}
#line 3816 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1926 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    (yyval) = local;
}
#line 3826 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1937 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3843 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1950 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3860 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1963 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3910 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 2009 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3925 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 2020 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3940 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 2031 "../Slice/Grammar.y" /* yacc.c:1646  */
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
#line 3955 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 2047 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-4]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[-3]));
    StringTokPtr ident = StringTokPtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));
    (yyval) = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral);
}
#line 3968 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 2056 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast((yyvsp[-3]));
    TypePtr const_type = TypePtr::dynamicCast((yyvsp[-2]));
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast((yyvsp[0]));
    unit->error("missing constant name");
    (yyval) = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral, Dummy); // Dummy
}
#line 3981 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 2070 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3988 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 2073 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 3995 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 2076 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4002 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 2079 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4009 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 2082 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4016 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 2085 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4023 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 2088 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4030 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 2091 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4037 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 2094 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4044 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 2097 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4051 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 2100 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4058 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 2103 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4065 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 2106 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4072 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 2109 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4079 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 2112 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4086 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 2115 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4093 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 2118 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4100 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 2121 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4107 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 2124 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4114 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 2127 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4121 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 2130 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4128 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 2133 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4135 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 2136 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4142 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 2139 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4149 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 2142 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4156 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 2145 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4163 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 2148 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4170 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 2151 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4177 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 2154 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4184 "Grammar.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 2157 "../Slice/Grammar.y" /* yacc.c:1646  */
    {
}
#line 4191 "Grammar.tab.c" /* yacc.c:1646  */
    break;


#line 4195 "Grammar.tab.c" /* yacc.c:1646  */
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
#line 2161 "../Slice/Grammar.y" /* yacc.c:1906  */

