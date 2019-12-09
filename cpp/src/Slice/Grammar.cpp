/* A Bison parser, made by GNU Bison 3.4.2.  */

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
#define YYBISON_VERSION "3.4.2"

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


#line 132 "src/Slice/Grammar.cpp"

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
    ICE_OPTIONAL = 284,
    ICE_VALUE = 285,
    ICE_IDENTIFIER = 286,
    ICE_SCOPED_IDENTIFIER = 287,
    ICE_STRING_LITERAL = 288,
    ICE_INTEGER_LITERAL = 289,
    ICE_FLOATING_POINT_LITERAL = 290,
    ICE_IDENT_OP = 291,
    ICE_KEYWORD_OP = 292,
    ICE_OPTIONAL_OP = 293,
    ICE_METADATA_OPEN = 294,
    ICE_METADATA_CLOSE = 295,
    ICE_GLOBAL_METADATA_OPEN = 296,
    ICE_GLOBAL_METADATA_CLOSE = 297,
    BAD_CHAR = 298
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
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
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
#  define YYSIZE_T unsigned
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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
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
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   788

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  85
/* YYNRULES -- Number of rules.  */
#define YYNRULES  231
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  332

#define YYUNDEFTOK  2
#define YYMAXUTOK   298

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    47,    53,     2,    49,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
      50,    48,    51,    52,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    45,     2,    46,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   116,   116,   124,   128,   135,   144,   149,   158,   157,
     167,   166,   177,   185,   184,   190,   189,   194,   199,   198,
     204,   203,   208,   213,   212,   218,   217,   222,   227,   226,
     232,   231,   236,   241,   240,   246,   245,   250,   255,   254,
     259,   264,   263,   269,   268,   273,   277,   287,   286,   320,
     324,   335,   346,   345,   371,   380,   388,   397,   400,   405,
     412,   425,   445,   538,   546,   559,   567,   582,   588,   592,
     603,   614,   613,   654,   663,   666,   671,   678,   684,   688,
     699,   724,   826,   838,   851,   850,   889,   924,   932,   937,
     945,   954,   957,   962,   969,   991,  1018,  1040,  1066,  1075,
    1086,  1095,  1104,  1114,  1128,  1134,  1142,  1154,  1178,  1203,
    1227,  1258,  1257,  1280,  1279,  1302,  1303,  1309,  1313,  1324,
    1338,  1337,  1371,  1406,  1441,  1446,  1456,  1461,  1469,  1478,
    1481,  1486,  1493,  1499,  1506,  1518,  1530,  1541,  1549,  1563,
    1573,  1589,  1593,  1605,  1604,  1636,  1635,  1653,  1659,  1667,
    1679,  1699,  1707,  1716,  1720,  1759,  1766,  1777,  1779,  1795,
    1811,  1823,  1835,  1846,  1862,  1867,  1875,  1878,  1886,  1890,
    1894,  1898,  1902,  1906,  1910,  1914,  1918,  1922,  1926,  1930,
    1934,  1938,  1942,  1946,  1950,  1954,  1958,  1963,  1967,  1971,
    1990,  2023,  2051,  2057,  2065,  2072,  2084,  2097,  2110,  2157,
    2168,  2179,  2195,  2204,  2218,  2221,  2224,  2227,  2230,  2233,
    2236,  2239,  2242,  2245,  2248,  2251,  2254,  2257,  2260,  2263,
    2266,  2269,  2272,  2275,  2278,  2281,  2284,  2287,  2290,  2293,
    2296,  2299
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
  "ICE_CONST", "ICE_FALSE", "ICE_TRUE", "ICE_IDEMPOTENT", "ICE_OPTIONAL",
  "ICE_VALUE", "ICE_IDENTIFIER", "ICE_SCOPED_IDENTIFIER",
  "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENT_OP", "ICE_KEYWORD_OP",
  "ICE_OPTIONAL_OP", "ICE_METADATA_OPEN", "ICE_METADATA_CLOSE",
  "ICE_GLOBAL_METADATA_OPEN", "ICE_GLOBAL_METADATA_CLOSE", "BAD_CHAR",
  "';'", "'{'", "'}'", "')'", "'='", "','", "'<'", "'>'", "'?'", "'*'",
  "$accept", "start", "opt_semicolon", "global_meta_data", "meta_data",
  "definitions", "$@1", "$@2", "definition", "$@3", "$@4", "$@5", "$@6",
  "$@7", "$@8", "$@9", "$@10", "$@11", "$@12", "$@13", "$@14", "$@15",
  "module_def", "@16", "exception_id", "exception_decl", "exception_def",
  "@17", "exception_extends", "exception_exports", "type_id", "optional",
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
     295,   296,   297,   298,    59,   123,   125,    41,    61,    44,
      60,    62,    63,    42
};
# endif

#define YYPACT_NINF -287

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-287)))

#define YYTABLE_NINF -158

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      11,     9,     9,    68,  -287,   132,  -287,     9,  -287,    -1,
     -20,  -287,    12,   -26,    41,   443,   507,   536,   565,    30,
      37,   594,    79,  -287,  -287,    18,    76,  -287,    81,    83,
    -287,    20,   117,    86,  -287,    22,    88,  -287,    99,   101,
    -287,  -287,   102,  -287,  -287,     9,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,    15,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,    79,    79,  -287,
     104,  -287,   274,    12,   103,    66,  -287,   106,   103,   107,
     109,   103,    66,   138,   111,   103,    75,  -287,   124,   103,
     126,   133,   140,   103,   142,  -287,   144,  -287,  -287,   141,
     145,   274,   274,   623,   139,   147,   149,   165,   166,   173,
     174,   175,    56,   180,    61,   -10,  -287,  -287,  -287,  -287,
     151,  -287,  -287,   332,  -287,  -287,  -287,    75,  -287,  -287,
    -287,  -287,  -287,  -287,   148,   188,  -287,  -287,  -287,  -287,
     623,  -287,  -287,    25,  -287,  -287,   184,   187,   189,   193,
     197,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,   194,   217,   364,   209,   750,
     195,  -287,   210,    75,   192,   211,   213,   652,    79,    50,
    -287,   623,   217,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
     212,   750,   214,   332,  -287,    -6,   215,   274,   218,  -287,
     681,  -287,   300,  -287,   220,   143,   219,  -287,  -287,  -287,
    -287,   274,  -287,  -287,  -287,  -287,  -287,   364,  -287,   274,
     222,   223,  -287,   681,  -287,  -287,   221,  -287,   224,   217,
     225,   235,   332,  -287,  -287,   228,   143,   230,   192,  -287,
     727,   274,    80,    72,  -287,   234,  -287,  -287,   229,  -287,
    -287,   217,   364,  -287,  -287,  -287,  -287,   217,  -287,   300,
     274,  -287,  -287,   238,   408,  -287,  -287,    87,  -287,  -287,
    -287,   227,  -287,    79,    28,   192,   710,  -287,  -287,  -287,
    -287,   235,   300,  -287,  -287,  -287,   750,  -287,   268,  -287,
    -287,  -287,  -287,   269,  -287,   681,   269,    79,   477,  -287,
    -287,  -287,   750,  -287,   236,  -287,  -287,  -287,   681,   477,
    -287,  -287
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       7,     0,     0,     0,     8,     0,     2,   193,   195,     0,
       0,     1,     7,     0,     0,     0,     0,     0,     0,     0,
       0,   145,     7,    10,    13,    51,    27,    28,    70,    32,
      33,    83,    87,    17,    18,   119,    22,    23,    37,    40,
     143,    41,    45,   192,     6,     0,     5,     9,    46,    47,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,    78,     0,
      79,   117,   118,    49,    50,    68,    69,     7,     7,   141,
       0,   142,     0,     7,     4,     0,    52,     0,     4,     0,
       0,     4,     0,    89,     0,     4,     0,   120,     0,     4,
       0,     0,     0,     4,     0,   194,     0,   166,   167,     0,
       0,     0,     0,   152,   168,   170,   172,   174,   176,   178,
     180,   182,   184,   187,   189,     0,    11,     3,    14,    54,
       0,    26,    29,     0,    31,    34,    86,     0,    84,    16,
      19,   124,   125,   126,   123,     0,    21,    24,    36,    39,
     152,    42,    44,     7,    80,    81,     0,     0,   149,     0,
     148,   151,   169,   171,   173,   175,   177,   179,   181,   183,
     185,   186,   188,   191,   190,     0,     0,     0,     0,     0,
       0,    88,     0,     0,     0,     0,     0,     0,     7,     0,
     146,   152,     0,   200,   201,   199,   196,   197,   198,   203,
       0,     0,     0,     0,    64,     0,    98,     0,    75,    77,
     103,    72,     0,   122,     0,     0,     0,   144,    48,   137,
     138,     0,   153,   150,   154,   147,   202,     0,    66,     0,
      94,    58,    67,    97,    53,    74,     0,    63,     0,     0,
     100,     0,     0,    60,   102,     0,     0,     0,     0,   106,
       0,     0,     0,     0,   132,   130,   105,   121,     0,    57,
      65,     0,     0,    96,    61,    62,    99,     0,    73,     0,
       0,   115,   116,    92,    97,    85,   129,     0,   104,   107,
     109,     0,   155,     7,     0,     0,     0,    95,    56,   101,
      91,   104,     0,   108,   110,   113,     0,   111,   156,   128,
     139,   140,    90,   165,   158,   162,   165,     7,     0,   114,
     160,   112,     0,   164,   134,   135,   136,   159,   163,     0,
     161,   133
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -287,  -287,    89,  -287,   -18,    -7,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -210,
    -180,  -182,  -286,  -287,  -287,  -287,  -287,  -287,  -197,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -262,    31,  -287,
      26,  -287,    32,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
    -133,  -287,  -239,  -287,   -40,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -147,  -287,  -287,     0,  -287,   -14,   -71,   -77,
      16,   305,  -192,  -287,   -15
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,   138,     4,     5,     6,    12,    93,    23,    94,
     104,   105,   108,   109,    97,    98,   100,   101,   110,   111,
     113,   114,    24,   116,    25,    26,    27,   140,    96,   212,
     238,   239,   240,   241,    28,    29,    30,    99,   190,   218,
      31,    32,    33,    34,   192,   103,   148,   257,   242,   219,
     262,   263,   264,   316,   313,   283,    35,    36,    37,   155,
     153,   107,   226,   265,   323,   324,    38,    39,    40,    41,
     112,    90,   169,   170,   233,   293,   294,   319,   134,   251,
       8,     9,   209,    42,   171
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      80,    82,    84,    86,    92,    47,    91,   217,   120,   216,
     236,   -12,   -12,   195,   191,   135,   245,   300,    48,   286,
     314,   185,    46,    43,   139,   117,   118,   269,   246,    45,
      95,   146,   -82,   -82,   106,   154,   327,   250,   186,    44,
     312,   247,     7,   261,   166,   167,   117,   118,    45,   119,
       1,     1,     2,     2,   235,   278,   309,   276,   -12,   270,
     223,   115,   298,   -55,     1,   -82,     2,  -127,    11,   121,
     122,   -12,    49,   291,   280,   307,   154,   308,   261,   297,
      87,   117,   118,   292,   232,   299,   136,    88,  -156,  -156,
    -156,  -156,  -156,  -156,  -156,  -156,  -156,   117,   118,   151,
     270,  -156,  -156,  -156,  -156,   152,   117,   118,   180,   181,
    -156,  -156,   220,   183,   184,   208,   289,   290,     1,  -157,
     -25,  -157,   154,   303,   304,   189,   -71,   -30,   234,   102,
     -15,   208,   -20,    13,   243,    14,    15,    16,    17,    18,
      19,    20,    21,   -35,   248,   -38,   -43,   137,   266,   123,
     141,   147,   143,   144,   268,   149,   196,    22,   259,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   156,   211,
     158,   260,   214,   133,   117,   118,   225,   159,   208,   284,
     231,   215,   230,   266,   288,   160,   162,   142,   164,   163,
     145,   172,   165,   224,   150,   189,   187,   193,   157,   173,
     208,   174,   161,   301,   256,   254,   208,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,   175,   176,   211,
      -7,    -7,    -7,    -7,    -7,   177,   178,   179,   273,   315,
      -7,     1,   182,   194,   189,   197,   198,   199,  -131,   200,
     225,   221,   202,   203,   204,   328,   201,   325,   117,   118,
     205,   206,   207,   213,   211,   222,   237,   227,   325,   228,
     244,   256,   252,   249,   258,   267,   253,   272,   274,   273,
     271,   275,   279,   277,   305,   306,   285,   225,   295,   292,
     296,   311,   302,   318,   256,   329,   287,   281,   282,   331,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   322,
     320,   255,   321,   326,   133,   117,   118,    10,   317,     0,
       0,     0,     0,   330,   326,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,     0,     0,     0,    -7,    -7,
      -7,    -7,    -7,   188,     0,     0,     0,     0,    -7,     1,
       0,     0,     0,     0,     0,     0,   -93,     0,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,     0,     0,     0,
       0,    -7,    -7,    -7,    -7,   210,     0,     0,     0,     0,
      -7,     1,     0,     0,     0,     0,     0,     0,   -76,     0,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,     0,
       0,     0,     0,    -7,    -7,    -7,    -7,     0,     0,     0,
       0,     0,    -7,     1,     0,     0,     0,     0,     0,     0,
     -59,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,   253,
       0,     0,     0,     0,  -105,  -105,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,     0,     0,     0,     0,    79,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,   117,   118,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    81,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    83,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    85,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    89,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,   168,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,   229,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,   253,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,   310,   259,   124,   125,   126,   127,   128,   129,   130,
     131,   132,     0,     0,     0,     0,   214,   133,   117,   118,
       0,     0,     0,     0,     0,   215,   124,   125,   126,   127,
     128,   129,   130,   131,   132,     0,     0,     0,     0,   214,
     133,   117,   118,     0,     0,     0,     0,     0,   215
};

static const yytype_int16 yycheck[] =
{
      15,    16,    17,    18,    22,    12,    21,   189,    79,   189,
     202,     0,     0,   160,   147,    92,   213,   279,    44,   258,
     306,    31,    42,     7,    95,    31,    32,   237,    34,    49,
      12,   102,    12,    13,    12,   106,   322,   217,    48,    40,
     302,    47,    33,   225,   121,   122,    31,    32,    49,    34,
      39,    39,    41,    41,   201,   252,   295,   249,    46,   239,
     193,    45,   272,    45,    39,    45,    41,    45,     0,    87,
      88,    46,    31,     1,   256,    47,   147,    49,   260,   271,
      50,    31,    32,    11,    34,   277,    93,    50,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    31,    32,    24,
     280,    29,    30,    31,    32,    30,    31,    32,    52,    53,
      38,    39,   189,    52,    53,   186,    36,    37,    39,    47,
      44,    49,   193,    36,    37,   143,    45,    44,   199,    12,
      44,   202,    44,     1,   211,     3,     4,     5,     6,     7,
       8,     9,    10,    44,   215,    44,    44,    44,   225,    45,
      44,    13,    45,    44,   231,    44,   163,    25,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    44,   187,
      44,    28,    29,    30,    31,    32,   194,    44,   249,   256,
     198,    38,   197,   260,   261,    45,    44,    98,    47,    45,
     101,    52,    47,     1,   105,   213,    45,    49,   109,    52,
     271,    52,   113,   280,   222,   220,   277,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    52,    52,   237,
      28,    29,    30,    31,    32,    52,    52,    52,   243,   306,
      38,    39,    52,    45,   252,    51,    49,    48,    46,    46,
     258,    46,    48,    26,    27,   322,    49,   318,    31,    32,
      33,    34,    35,    44,   272,    45,    44,    46,   329,    46,
      46,   279,    44,    48,    44,    46,    31,    44,    47,   284,
      48,    47,    44,    48,    47,   293,    46,   295,    44,    11,
      51,   296,    44,    14,   302,    49,   260,   256,   256,   329,
      16,    17,    18,    19,    20,    21,    22,    23,    24,   317,
     315,     1,   316,   318,    30,    31,    32,     2,   308,    -1,
      -1,    -1,    -1,   328,   329,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    28,    29,
      30,    31,    32,     1,    -1,    -1,    -1,    -1,    38,    39,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    32,     1,    -1,    -1,    -1,    -1,
      38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    46,    -1,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    29,    30,    31,    32,    -1,    -1,    -1,
      -1,    -1,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,
      46,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    36,    37,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,    36,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    29,    30,    31,    32,
      -1,    -1,    -1,    -1,    -1,    38,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    29,
      30,    31,    32,    -1,    -1,    -1,    -1,    -1,    38
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    39,    41,    55,    57,    58,    59,    33,   134,   135,
     135,     0,    60,     1,     3,     4,     5,     6,     7,     8,
       9,    10,    25,    62,    76,    78,    79,    80,    88,    89,
      90,    94,    95,    96,    97,   110,   111,   112,   120,   121,
     122,   123,   137,   134,    40,    49,    42,    59,    44,    31,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    36,
     138,    31,   138,    31,   138,    31,   138,    50,    50,    31,
     125,   138,    58,    61,    63,    12,    82,    68,    69,    91,
      70,    71,    12,    99,    64,    65,    12,   115,    66,    67,
      72,    73,   124,    74,    75,   134,    77,    31,    32,    34,
     132,    58,    58,    45,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    30,   132,   133,    59,    44,    56,   132,
      81,    44,    56,    45,    44,    56,   132,    13,   100,    44,
      56,    24,    30,   114,   132,   113,    44,    56,    44,    44,
      45,    56,    44,    45,    47,    47,   133,   133,    31,   126,
     127,   138,    52,    52,    52,    52,    52,    52,    52,    52,
      52,    53,    52,    52,    53,    31,    48,    45,     1,    58,
      92,   114,    98,    49,    45,   126,    59,    51,    49,    48,
      46,    49,    48,    26,    27,    33,    34,    35,   132,   136,
       1,    58,    83,    44,    29,    38,    84,    85,    93,   103,
     133,    46,    45,   114,     1,    58,   116,    46,    46,    31,
     138,    58,    34,   128,   132,   126,   136,    44,    84,    85,
      86,    87,   102,   133,    46,    92,    34,    47,   132,    48,
      84,   133,    44,    31,   138,     1,    58,   101,    44,    15,
      28,    85,   104,   105,   106,   117,   133,    46,   133,    83,
      84,    48,    44,   138,    47,    47,   136,    48,    92,    44,
      85,   102,   106,   109,   133,    46,   116,   104,   133,    36,
      37,     1,    11,   129,   130,    44,    51,   136,    83,   136,
     101,   133,    44,    36,    37,    47,    58,    47,    49,   116,
      31,   138,   101,   108,    86,   133,   107,   129,    14,   131,
     138,   131,    58,   118,   119,   132,   138,    86,   133,    49,
     138,   118
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    54,    55,    56,    56,    57,    58,    58,    60,    59,
      61,    59,    59,    63,    62,    64,    62,    62,    65,    62,
      66,    62,    62,    67,    62,    68,    62,    62,    69,    62,
      70,    62,    62,    71,    62,    72,    62,    62,    73,    62,
      62,    74,    62,    75,    62,    62,    62,    77,    76,    78,
      78,    79,    81,    80,    82,    82,    83,    83,    83,    83,
      84,    85,    85,    85,    85,    86,    86,    87,    88,    88,
      89,    91,    90,    92,    92,    92,    92,    93,    94,    94,
      95,    95,    95,    96,    98,    97,    99,    99,   100,   100,
     101,   101,   101,   101,   102,   102,   102,   102,   103,   103,
     103,   103,   103,   103,   104,   104,   104,   105,   105,   105,
     105,   107,   106,   108,   106,   109,   109,   110,   110,   111,
     113,   112,   114,   114,   114,   114,   115,   115,   116,   116,
     116,   116,   117,   118,   118,   119,   119,   120,   120,   121,
     121,   122,   122,   124,   123,   125,   123,   126,   126,   127,
     127,   127,   127,   128,   128,   129,   129,   130,   130,   130,
     130,   130,   130,   130,   131,   131,   132,   132,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   134,   134,   135,   135,   136,   136,   136,   136,
     136,   136,   137,   137,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     0,     0,     3,
       0,     4,     0,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       1,     0,     3,     0,     3,     1,     2,     0,     6,     2,
       2,     1,     0,     6,     2,     0,     4,     3,     2,     0,
       2,     3,     3,     2,     1,     2,     1,     1,     2,     2,
       1,     0,     5,     4,     3,     2,     0,     1,     2,     2,
       4,     4,     1,     1,     0,     7,     2,     0,     2,     0,
       4,     3,     2,     0,     1,     3,     2,     1,     1,     3,
       2,     4,     2,     1,     2,     1,     1,     2,     3,     2,
       3,     0,     5,     0,     5,     1,     1,     2,     2,     1,
       0,     6,     3,     1,     1,     1,     2,     0,     4,     3,
       2,     0,     1,     3,     1,     1,     1,     6,     6,     9,
       9,     2,     2,     0,     5,     0,     5,     3,     1,     1,
       3,     1,     0,     1,     1,     1,     0,     0,     3,     5,
       4,     6,     3,     5,     2,     0,     1,     1,     1,     2,
       1,     2,     1,     2,     1,     2,     1,     2,     1,     2,
       1,     2,     1,     2,     1,     2,     2,     1,     2,     1,
       2,     2,     2,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     6,     5,     1,     1,     1,     1,     1,     1,
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
  unsigned long yylno = yyrline[yyrule];
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

  if (! yyres)
    return yystrlen (yystr);

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
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
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
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
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
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
# else /* defined YYSTACK_RELOCATE */
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
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

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
#line 117 "src/Slice/Grammar.y"
    {
}
#line 1677 "src/Slice/Grammar.cpp"
    break;

  case 3:
#line 125 "src/Slice/Grammar.y"
    {
}
#line 1684 "src/Slice/Grammar.cpp"
    break;

  case 4:
#line 128 "src/Slice/Grammar.y"
    {
}
#line 1691 "src/Slice/Grammar.cpp"
    break;

  case 5:
#line 136 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[-1];
}
#line 1699 "src/Slice/Grammar.cpp"
    break;

  case 6:
#line 145 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[-1];
}
#line 1707 "src/Slice/Grammar.cpp"
    break;

  case 7:
#line 149 "src/Slice/Grammar.y"
    {
    yyval = new StringListTok;
}
#line 1715 "src/Slice/Grammar.cpp"
    break;

  case 8:
#line 158 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
#line 1727 "src/Slice/Grammar.cpp"
    break;

  case 10:
#line 167 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1740 "src/Slice/Grammar.cpp"
    break;

  case 12:
#line 177 "src/Slice/Grammar.y"
    {
}
#line 1747 "src/Slice/Grammar.cpp"
    break;

  case 13:
#line 185 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1755 "src/Slice/Grammar.cpp"
    break;

  case 15:
#line 190 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1763 "src/Slice/Grammar.cpp"
    break;

  case 17:
#line 195 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after class forward declaration");
}
#line 1771 "src/Slice/Grammar.cpp"
    break;

  case 18:
#line 199 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1779 "src/Slice/Grammar.cpp"
    break;

  case 20:
#line 204 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1787 "src/Slice/Grammar.cpp"
    break;

  case 22:
#line 209 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after interface forward declaration");
}
#line 1795 "src/Slice/Grammar.cpp"
    break;

  case 23:
#line 213 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1803 "src/Slice/Grammar.cpp"
    break;

  case 25:
#line 218 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0);
}
#line 1811 "src/Slice/Grammar.cpp"
    break;

  case 27:
#line 223 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after exception forward declaration");
}
#line 1819 "src/Slice/Grammar.cpp"
    break;

  case 28:
#line 227 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 1827 "src/Slice/Grammar.cpp"
    break;

  case 30:
#line 232 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0);
}
#line 1835 "src/Slice/Grammar.cpp"
    break;

  case 32:
#line 237 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after struct forward declaration");
}
#line 1843 "src/Slice/Grammar.cpp"
    break;

  case 33:
#line 241 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 1851 "src/Slice/Grammar.cpp"
    break;

  case 35:
#line 246 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 1859 "src/Slice/Grammar.cpp"
    break;

  case 37:
#line 251 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after sequence definition");
}
#line 1867 "src/Slice/Grammar.cpp"
    break;

  case 38:
#line 255 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 1875 "src/Slice/Grammar.cpp"
    break;

  case 40:
#line 260 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after dictionary definition");
}
#line 1883 "src/Slice/Grammar.cpp"
    break;

  case 41:
#line 264 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 1891 "src/Slice/Grammar.cpp"
    break;

  case 43:
#line 269 "src/Slice/Grammar.y"
    {
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 1899 "src/Slice/Grammar.cpp"
    break;

  case 45:
#line 274 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after const definition");
}
#line 1907 "src/Slice/Grammar.cpp"
    break;

  case 46:
#line 278 "src/Slice/Grammar.y"
    {
    yyerrok;
}
#line 1915 "src/Slice/Grammar.cpp"
    break;

  case 47:
#line 287 "src/Slice/Grammar.y"
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
#line 1936 "src/Slice/Grammar.cpp"
    break;

  case 48:
#line 304 "src/Slice/Grammar.y"
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
#line 1952 "src/Slice/Grammar.cpp"
    break;

  case 49:
#line 321 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 1960 "src/Slice/Grammar.cpp"
    break;

  case 50:
#line 325 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 1970 "src/Slice/Grammar.cpp"
    break;

  case 51:
#line 336 "src/Slice/Grammar.y"
    {
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 1979 "src/Slice/Grammar.cpp"
    break;

  case 52:
#line 346 "src/Slice/Grammar.y"
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
#line 1996 "src/Slice/Grammar.cpp"
    break;

  case 53:
#line 359 "src/Slice/Grammar.y"
    {
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2008 "src/Slice/Grammar.cpp"
    break;

  case 54:
#line 372 "src/Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2020 "src/Slice/Grammar.cpp"
    break;

  case 55:
#line 380 "src/Slice/Grammar.y"
    {
    yyval = 0;
}
#line 2028 "src/Slice/Grammar.cpp"
    break;

  case 56:
#line 389 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2041 "src/Slice/Grammar.cpp"
    break;

  case 57:
#line 398 "src/Slice/Grammar.y"
    {
}
#line 2048 "src/Slice/Grammar.cpp"
    break;

  case 58:
#line 401 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
}
#line 2056 "src/Slice/Grammar.cpp"
    break;

  case 59:
#line 405 "src/Slice/Grammar.y"
    {
}
#line 2063 "src/Slice/Grammar.cpp"
    break;

  case 60:
#line 413 "src/Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    yyval = typestring;
}
#line 2075 "src/Slice/Grammar.cpp"
    break;

  case 61:
#line 426 "src/Slice/Grammar.y"
    {
    IntegerTokPtr i = IntegerTokPtr::dynamicCast(yyvsp[-1]);

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
    yyval = m;
}
#line 2099 "src/Slice/Grammar.cpp"
    break;

  case 62:
#line 446 "src/Slice/Grammar.y"
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
    yyval = m;
}
#line 2196 "src/Slice/Grammar.cpp"
    break;

  case 63:
#line 539 "src/Slice/Grammar.y"
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    yyval = m;
}
#line 2208 "src/Slice/Grammar.cpp"
    break;

  case 64:
#line 547 "src/Slice/Grammar.y"
    {
    unit->error("missing tag for optional");
    OptionalDefTokPtr m = new OptionalDefTok; // Dummy
    m->v.optional = false;
    m->v.tag = -1;
    yyval = m;
}
#line 2220 "src/Slice/Grammar.cpp"
    break;

  case 65:
#line 560 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    yyval = m;
}
#line 2232 "src/Slice/Grammar.cpp"
    break;

  case 66:
#line 568 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.type = ts->v.first;
    m->v.name = ts->v.second;
    m->v.optional = false;
    m->v.tag = -1;
    yyval = m;
}
#line 2246 "src/Slice/Grammar.cpp"
    break;

  case 68:
#line 589 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 2254 "src/Slice/Grammar.cpp"
    break;

  case 69:
#line 593 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2264 "src/Slice/Grammar.cpp"
    break;

  case 70:
#line 604 "src/Slice/Grammar.y"
    {
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2273 "src/Slice/Grammar.cpp"
    break;

  case 71:
#line 614 "src/Slice/Grammar.y"
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
#line 2295 "src/Slice/Grammar.cpp"
    break;

  case 72:
#line 632 "src/Slice/Grammar.y"
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
#line 2317 "src/Slice/Grammar.cpp"
    break;

  case 73:
#line 655 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2330 "src/Slice/Grammar.cpp"
    break;

  case 74:
#line 664 "src/Slice/Grammar.y"
    {
}
#line 2337 "src/Slice/Grammar.cpp"
    break;

  case 75:
#line 667 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
}
#line 2345 "src/Slice/Grammar.cpp"
    break;

  case 76:
#line 671 "src/Slice/Grammar.y"
    {
}
#line 2352 "src/Slice/Grammar.cpp"
    break;

  case 78:
#line 685 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 2360 "src/Slice/Grammar.cpp"
    break;

  case 79:
#line 689 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2370 "src/Slice/Grammar.cpp"
    break;

  case 80:
#line 700 "src/Slice/Grammar.y"
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
#line 2399 "src/Slice/Grammar.cpp"
    break;

  case 81:
#line 725 "src/Slice/Grammar.y"
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
#line 2505 "src/Slice/Grammar.cpp"
    break;

  case 82:
#line 827 "src/Slice/Grammar.y"
    {
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2516 "src/Slice/Grammar.cpp"
    break;

  case 83:
#line 839 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false);
    yyval = cl;
}
#line 2527 "src/Slice/Grammar.cpp"
    break;

  case 84:
#line 851 "src/Slice/Grammar.y"
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
#line 2553 "src/Slice/Grammar.cpp"
    break;

  case 85:
#line 873 "src/Slice/Grammar.y"
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
#line 2569 "src/Slice/Grammar.cpp"
    break;

  case 86:
#line 890 "src/Slice/Grammar.y"
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
#line 2607 "src/Slice/Grammar.cpp"
    break;

  case 87:
#line 924 "src/Slice/Grammar.y"
    {
    yyval = 0;
}
#line 2615 "src/Slice/Grammar.cpp"
    break;

  case 88:
#line 933 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 2623 "src/Slice/Grammar.cpp"
    break;

  case 89:
#line 937 "src/Slice/Grammar.y"
    {
    yyval = new ClassListTok;
}
#line 2631 "src/Slice/Grammar.cpp"
    break;

  case 90:
#line 946 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2644 "src/Slice/Grammar.cpp"
    break;

  case 91:
#line 955 "src/Slice/Grammar.y"
    {
}
#line 2651 "src/Slice/Grammar.cpp"
    break;

  case 92:
#line 958 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
}
#line 2659 "src/Slice/Grammar.cpp"
    break;

  case 93:
#line 962 "src/Slice/Grammar.y"
    {
}
#line 2666 "src/Slice/Grammar.cpp"
    break;

  case 94:
#line 970 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr def = OptionalDefTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = dm;
}
#line 2692 "src/Slice/Grammar.cpp"
    break;

  case 95:
#line 992 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr def = OptionalDefTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);

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
    yyval = dm;
}
#line 2723 "src/Slice/Grammar.cpp"
    break;

  case 96:
#line 1019 "src/Slice/Grammar.y"
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
#line 2749 "src/Slice/Grammar.cpp"
    break;

  case 97:
#line 1041 "src/Slice/Grammar.y"
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
#line 2774 "src/Slice/Grammar.cpp"
    break;

  case 98:
#line 1067 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 2787 "src/Slice/Grammar.cpp"
    break;

  case 99:
#line 1076 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, value->v.value,
                                            value->v.valueAsString, value->v.valueAsLiteral);
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 2802 "src/Slice/Grammar.cpp"
    break;

  case 100:
#line 1087 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("optional data members not supported in struct");
}
#line 2815 "src/Slice/Grammar.cpp"
    break;

  case 101:
#line 1096 "src/Slice/Grammar.y"
    {
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("optional data members not supported in struct");
}
#line 2828 "src/Slice/Grammar.cpp"
    break;

  case 102:
#line 1105 "src/Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 2842 "src/Slice/Grammar.cpp"
    break;

  case 103:
#line 1115 "src/Slice/Grammar.y"
    {
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("missing data member name");
}
#line 2855 "src/Slice/Grammar.cpp"
    break;

  case 104:
#line 1129 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr m = OptionalDefTokPtr::dynamicCast(yyvsp[-1]);
    m->v.type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 2865 "src/Slice/Grammar.cpp"
    break;

  case 105:
#line 1135 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok();
    m->v.type = TypePtr::dynamicCast(yyvsp[0]);
    m->v.optional = false;
    m->v.tag = -1;
    yyval = m;
}
#line 2877 "src/Slice/Grammar.cpp"
    break;

  case 106:
#line 1143 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr m = new OptionalDefTok;
    m->v.optional = false;
    m->v.tag = -1;
    yyval = m;
}
#line 2888 "src/Slice/Grammar.cpp"
    break;

  case 107:
#line 1155 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag);
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
#line 2916 "src/Slice/Grammar.cpp"
    break;

  case 108:
#line 1179 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag,
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
#line 2945 "src/Slice/Grammar.cpp"
    break;

  case 109:
#line 1204 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag);
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
#line 2973 "src/Slice/Grammar.cpp"
    break;

  case 110:
#line 1228 "src/Slice/Grammar.y"
    {
    OptionalDefTokPtr returnType = OptionalDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        OperationPtr op = cl->createOperation(name, returnType->v.type, returnType->v.optional, returnType->v.tag,
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
#line 3002 "src/Slice/Grammar.cpp"
    break;

  case 111:
#line 1258 "src/Slice/Grammar.y"
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
#line 3018 "src/Slice/Grammar.cpp"
    break;

  case 112:
#line 1270 "src/Slice/Grammar.y"
    {
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3032 "src/Slice/Grammar.cpp"
    break;

  case 113:
#line 1280 "src/Slice/Grammar.y"
    {
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3044 "src/Slice/Grammar.cpp"
    break;

  case 114:
#line 1288 "src/Slice/Grammar.y"
    {
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3058 "src/Slice/Grammar.cpp"
    break;

  case 117:
#line 1310 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 3066 "src/Slice/Grammar.cpp"
    break;

  case 118:
#line 1314 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3076 "src/Slice/Grammar.cpp"
    break;

  case 119:
#line 1325 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3088 "src/Slice/Grammar.cpp"
    break;

  case 120:
#line 1338 "src/Slice/Grammar.y"
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
#line 3109 "src/Slice/Grammar.cpp"
    break;

  case 121:
#line 1355 "src/Slice/Grammar.y"
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
#line 3125 "src/Slice/Grammar.cpp"
    break;

  case 122:
#line 1372 "src/Slice/Grammar.y"
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
#line 3164 "src/Slice/Grammar.cpp"
    break;

  case 123:
#line 1407 "src/Slice/Grammar.y"
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
#line 3203 "src/Slice/Grammar.cpp"
    break;

  case 124:
#line 1442 "src/Slice/Grammar.y"
    {
    unit->error("illegal inheritance from type Object");
    yyval = new ClassListTok; // Dummy
}
#line 3212 "src/Slice/Grammar.cpp"
    break;

  case 125:
#line 1447 "src/Slice/Grammar.y"
    {
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3221 "src/Slice/Grammar.cpp"
    break;

  case 126:
#line 1457 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 3229 "src/Slice/Grammar.cpp"
    break;

  case 127:
#line 1461 "src/Slice/Grammar.y"
    {
    yyval = new ClassListTok;
}
#line 3237 "src/Slice/Grammar.cpp"
    break;

  case 128:
#line 1470 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
    contained->setMetaData(metaData->v);
    }
}
#line 3250 "src/Slice/Grammar.cpp"
    break;

  case 129:
#line 1479 "src/Slice/Grammar.y"
    {
}
#line 3257 "src/Slice/Grammar.cpp"
    break;

  case 130:
#line 1482 "src/Slice/Grammar.y"
    {
    unit->error("`;' missing after definition");
}
#line 3265 "src/Slice/Grammar.cpp"
    break;

  case 131:
#line 1486 "src/Slice/Grammar.y"
    {
}
#line 3272 "src/Slice/Grammar.cpp"
    break;

  case 133:
#line 1500 "src/Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3283 "src/Slice/Grammar.cpp"
    break;

  case 134:
#line 1507 "src/Slice/Grammar.y"
    {
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3294 "src/Slice/Grammar.cpp"
    break;

  case 135:
#line 1519 "src/Slice/Grammar.y"
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
#line 3310 "src/Slice/Grammar.cpp"
    break;

  case 136:
#line 1531 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3320 "src/Slice/Grammar.cpp"
    break;

  case 137:
#line 1542 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3332 "src/Slice/Grammar.cpp"
    break;

  case 138:
#line 1550 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3345 "src/Slice/Grammar.cpp"
    break;

  case 139:
#line 1564 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3359 "src/Slice/Grammar.cpp"
    break;

  case 140:
#line 1574 "src/Slice/Grammar.y"
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
#line 3374 "src/Slice/Grammar.cpp"
    break;

  case 141:
#line 1590 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 3382 "src/Slice/Grammar.cpp"
    break;

  case 142:
#line 1594 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3392 "src/Slice/Grammar.cpp"
    break;

  case 143:
#line 1605 "src/Slice/Grammar.y"
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
#line 3412 "src/Slice/Grammar.cpp"
    break;

  case 144:
#line 1621 "src/Slice/Grammar.y"
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
#line 3430 "src/Slice/Grammar.cpp"
    break;

  case 145:
#line 1636 "src/Slice/Grammar.y"
    {
    unit->error("missing enumeration name");
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3442 "src/Slice/Grammar.cpp"
    break;

  case 146:
#line 1644 "src/Slice/Grammar.y"
    {
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3451 "src/Slice/Grammar.cpp"
    break;

  case 147:
#line 1654 "src/Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 3461 "src/Slice/Grammar.cpp"
    break;

  case 148:
#line 1660 "src/Slice/Grammar.y"
    {
}
#line 3468 "src/Slice/Grammar.cpp"
    break;

  case 149:
#line 1668 "src/Slice/Grammar.y"
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
#line 3484 "src/Slice/Grammar.cpp"
    break;

  case 150:
#line 1680 "src/Slice/Grammar.y"
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
#line 3508 "src/Slice/Grammar.cpp"
    break;

  case 151:
#line 1700 "src/Slice/Grammar.y"
    {
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 3519 "src/Slice/Grammar.cpp"
    break;

  case 152:
#line 1707 "src/Slice/Grammar.y"
    {
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 3528 "src/Slice/Grammar.cpp"
    break;

  case 153:
#line 1717 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 3536 "src/Slice/Grammar.cpp"
    break;

  case 154:
#line 1721 "src/Slice/Grammar.y"
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
#line 3574 "src/Slice/Grammar.cpp"
    break;

  case 155:
#line 1760 "src/Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = true;
    yyval = out;
}
#line 3584 "src/Slice/Grammar.cpp"
    break;

  case 156:
#line 1766 "src/Slice/Grammar.y"
    {
    BoolTokPtr out = new BoolTok;
    out->v = false;
    yyval = out;
}
#line 3594 "src/Slice/Grammar.cpp"
    break;

  case 157:
#line 1777 "src/Slice/Grammar.y"
    {
}
#line 3601 "src/Slice/Grammar.cpp"
    break;

  case 158:
#line 1780 "src/Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    OptionalDefTokPtr tsp = OptionalDefTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->v.name, tsp->v.type, isOutParam->v, tsp->v.optional, tsp->v.tag);
        unit->currentContainer()->checkIntroduced(tsp->v.name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 3621 "src/Slice/Grammar.cpp"
    break;

  case 159:
#line 1796 "src/Slice/Grammar.y"
    {
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    OptionalDefTokPtr tsp = OptionalDefTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->v.name, tsp->v.type, isOutParam->v, tsp->v.optional, tsp->v.tag);
        unit->currentContainer()->checkIntroduced(tsp->v.name, pd);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
        if(!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 3641 "src/Slice/Grammar.cpp"
    break;

  case 160:
#line 1812 "src/Slice/Grammar.y"
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
#line 3657 "src/Slice/Grammar.cpp"
    break;

  case 161:
#line 1824 "src/Slice/Grammar.y"
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
#line 3673 "src/Slice/Grammar.cpp"
    break;

  case 162:
#line 1836 "src/Slice/Grammar.y"
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
#line 3688 "src/Slice/Grammar.cpp"
    break;

  case 163:
#line 1847 "src/Slice/Grammar.y"
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
#line 3703 "src/Slice/Grammar.cpp"
    break;

  case 164:
#line 1863 "src/Slice/Grammar.y"
    {
    yyval = yyvsp[0];
}
#line 3711 "src/Slice/Grammar.cpp"
    break;

  case 165:
#line 1867 "src/Slice/Grammar.y"
    {
    yyval = new ExceptionListTok;
}
#line 3719 "src/Slice/Grammar.cpp"
    break;

  case 166:
#line 1876 "src/Slice/Grammar.y"
    {
}
#line 3726 "src/Slice/Grammar.cpp"
    break;

  case 167:
#line 1879 "src/Slice/Grammar.y"
    {
}
#line 3733 "src/Slice/Grammar.cpp"
    break;

  case 168:
#line 1887 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindByte);
}
#line 3741 "src/Slice/Grammar.cpp"
    break;

  case 169:
#line 1891 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindByte);
}
#line 3749 "src/Slice/Grammar.cpp"
    break;

  case 170:
#line 1895 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindBool);
}
#line 3757 "src/Slice/Grammar.cpp"
    break;

  case 171:
#line 1899 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindBool);
}
#line 3765 "src/Slice/Grammar.cpp"
    break;

  case 172:
#line 1903 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindShort);
}
#line 3773 "src/Slice/Grammar.cpp"
    break;

  case 173:
#line 1907 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindShort);
}
#line 3781 "src/Slice/Grammar.cpp"
    break;

  case 174:
#line 1911 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindInt);
}
#line 3789 "src/Slice/Grammar.cpp"
    break;

  case 175:
#line 1915 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindInt);
}
#line 3797 "src/Slice/Grammar.cpp"
    break;

  case 176:
#line 1919 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindLong);
}
#line 3805 "src/Slice/Grammar.cpp"
    break;

  case 177:
#line 1923 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindLong);
}
#line 3813 "src/Slice/Grammar.cpp"
    break;

  case 178:
#line 1927 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindFloat);
}
#line 3821 "src/Slice/Grammar.cpp"
    break;

  case 179:
#line 1931 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindFloat);
}
#line 3829 "src/Slice/Grammar.cpp"
    break;

  case 180:
#line 1935 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindDouble);
}
#line 3837 "src/Slice/Grammar.cpp"
    break;

  case 181:
#line 1939 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindDouble);
}
#line 3845 "src/Slice/Grammar.cpp"
    break;

  case 182:
#line 1943 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindString);
}
#line 3853 "src/Slice/Grammar.cpp"
    break;

  case 183:
#line 1947 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindString);
}
#line 3861 "src/Slice/Grammar.cpp"
    break;

  case 184:
#line 1951 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindObject);
}
#line 3869 "src/Slice/Grammar.cpp"
    break;

  case 185:
#line 1955 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindObjectProxy);
}
#line 3877 "src/Slice/Grammar.cpp"
    break;

  case 186:
#line 1959 "src/Slice/Grammar.y"
    {
    // TODO: equivalent to ICE_OBJECT ? above, need to merge KindObject / KindObjectProxy
    yyval = unit->builtin(Builtin::KindObjectProxy);
}
#line 3886 "src/Slice/Grammar.cpp"
    break;

  case 187:
#line 1964 "src/Slice/Grammar.y"
    {
    yyval = unit->builtin(Builtin::KindValue);
}
#line 3894 "src/Slice/Grammar.cpp"
    break;

  case 188:
#line 1968 "src/Slice/Grammar.y"
    {
    yyval = unit->optionalBuiltin(Builtin::KindValue);
}
#line 3902 "src/Slice/Grammar.cpp"
    break;

  case 189:
#line 1972 "src/Slice/Grammar.y"
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
#line 3925 "src/Slice/Grammar.cpp"
    break;

  case 190:
#line 1991 "src/Slice/Grammar.y"
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
#line 3962 "src/Slice/Grammar.cpp"
    break;

  case 191:
#line 2024 "src/Slice/Grammar.y"
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
#line 3989 "src/Slice/Grammar.cpp"
    break;

  case 192:
#line 2052 "src/Slice/Grammar.y"
    {
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 3999 "src/Slice/Grammar.cpp"
    break;

  case 193:
#line 2058 "src/Slice/Grammar.y"
    {
}
#line 4006 "src/Slice/Grammar.cpp"
    break;

  case 194:
#line 2066 "src/Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4017 "src/Slice/Grammar.cpp"
    break;

  case 195:
#line 2073 "src/Slice/Grammar.y"
    {
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4028 "src/Slice/Grammar.cpp"
    break;

  case 196:
#line 2085 "src/Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = sstr.str();
    def->v.valueAsLiteral = intVal->literal;
    yyval = def;
}
#line 4045 "src/Slice/Grammar.cpp"
    break;

  case 197:
#line 2098 "src/Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = sstr.str();
    def->v.valueAsLiteral = floatVal->literal;
    yyval = def;
}
#line 4062 "src/Slice/Grammar.cpp"
    break;

  case 198:
#line 2111 "src/Slice/Grammar.y"
    {
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = def;
}
#line 4113 "src/Slice/Grammar.cpp"
    break;

  case 199:
#line 2158 "src/Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = literal->v;
    def->v.valueAsLiteral = literal->literal;
    yyval = def;
}
#line 4128 "src/Slice/Grammar.cpp"
    break;

  case 200:
#line 2169 "src/Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "false";
    def->v.valueAsLiteral = "false";
    yyval = def;
}
#line 4143 "src/Slice/Grammar.cpp"
    break;

  case 201:
#line 2180 "src/Slice/Grammar.y"
    {
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok;
    def->v.type = type;
    def->v.value = type;
    def->v.valueAsString = "true";
    def->v.valueAsLiteral = "true";
    yyval = def;
}
#line 4158 "src/Slice/Grammar.cpp"
    break;

  case 202:
#line 2196 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral);
}
#line 4171 "src/Slice/Grammar.cpp"
    break;

  case 203:
#line 2205 "src/Slice/Grammar.y"
    {
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v.value,
                                               value->v.valueAsString, value->v.valueAsLiteral, Dummy); // Dummy
}
#line 4184 "src/Slice/Grammar.cpp"
    break;

  case 204:
#line 2219 "src/Slice/Grammar.y"
    {
}
#line 4191 "src/Slice/Grammar.cpp"
    break;

  case 205:
#line 2222 "src/Slice/Grammar.y"
    {
}
#line 4198 "src/Slice/Grammar.cpp"
    break;

  case 206:
#line 2225 "src/Slice/Grammar.y"
    {
}
#line 4205 "src/Slice/Grammar.cpp"
    break;

  case 207:
#line 2228 "src/Slice/Grammar.y"
    {
}
#line 4212 "src/Slice/Grammar.cpp"
    break;

  case 208:
#line 2231 "src/Slice/Grammar.y"
    {
}
#line 4219 "src/Slice/Grammar.cpp"
    break;

  case 209:
#line 2234 "src/Slice/Grammar.y"
    {
}
#line 4226 "src/Slice/Grammar.cpp"
    break;

  case 210:
#line 2237 "src/Slice/Grammar.y"
    {
}
#line 4233 "src/Slice/Grammar.cpp"
    break;

  case 211:
#line 2240 "src/Slice/Grammar.y"
    {
}
#line 4240 "src/Slice/Grammar.cpp"
    break;

  case 212:
#line 2243 "src/Slice/Grammar.y"
    {
}
#line 4247 "src/Slice/Grammar.cpp"
    break;

  case 213:
#line 2246 "src/Slice/Grammar.y"
    {
}
#line 4254 "src/Slice/Grammar.cpp"
    break;

  case 214:
#line 2249 "src/Slice/Grammar.y"
    {
}
#line 4261 "src/Slice/Grammar.cpp"
    break;

  case 215:
#line 2252 "src/Slice/Grammar.y"
    {
}
#line 4268 "src/Slice/Grammar.cpp"
    break;

  case 216:
#line 2255 "src/Slice/Grammar.y"
    {
}
#line 4275 "src/Slice/Grammar.cpp"
    break;

  case 217:
#line 2258 "src/Slice/Grammar.y"
    {
}
#line 4282 "src/Slice/Grammar.cpp"
    break;

  case 218:
#line 2261 "src/Slice/Grammar.y"
    {
}
#line 4289 "src/Slice/Grammar.cpp"
    break;

  case 219:
#line 2264 "src/Slice/Grammar.y"
    {
}
#line 4296 "src/Slice/Grammar.cpp"
    break;

  case 220:
#line 2267 "src/Slice/Grammar.y"
    {
}
#line 4303 "src/Slice/Grammar.cpp"
    break;

  case 221:
#line 2270 "src/Slice/Grammar.y"
    {
}
#line 4310 "src/Slice/Grammar.cpp"
    break;

  case 222:
#line 2273 "src/Slice/Grammar.y"
    {
}
#line 4317 "src/Slice/Grammar.cpp"
    break;

  case 223:
#line 2276 "src/Slice/Grammar.y"
    {
}
#line 4324 "src/Slice/Grammar.cpp"
    break;

  case 224:
#line 2279 "src/Slice/Grammar.y"
    {
}
#line 4331 "src/Slice/Grammar.cpp"
    break;

  case 225:
#line 2282 "src/Slice/Grammar.y"
    {
}
#line 4338 "src/Slice/Grammar.cpp"
    break;

  case 226:
#line 2285 "src/Slice/Grammar.y"
    {
}
#line 4345 "src/Slice/Grammar.cpp"
    break;

  case 227:
#line 2288 "src/Slice/Grammar.y"
    {
}
#line 4352 "src/Slice/Grammar.cpp"
    break;

  case 228:
#line 2291 "src/Slice/Grammar.y"
    {
}
#line 4359 "src/Slice/Grammar.cpp"
    break;

  case 229:
#line 2294 "src/Slice/Grammar.y"
    {
}
#line 4366 "src/Slice/Grammar.cpp"
    break;

  case 230:
#line 2297 "src/Slice/Grammar.y"
    {
}
#line 4373 "src/Slice/Grammar.cpp"
    break;

  case 231:
#line 2300 "src/Slice/Grammar.y"
    {
}
#line 4380 "src/Slice/Grammar.cpp"
    break;


#line 4384 "src/Slice/Grammar.cpp"

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
#line 2304 "src/Slice/Grammar.y"

