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

/* "%code top" blocks.  */
#line 1 "src/Slice/Grammar.y"


//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Included first to get 'TokenLocation' which we need to define YYLTYPE before flex does.
#include <Slice/GrammarUtil.h>

#line 30 "src/Slice/Grammar.y"


// Defines the rule bison uses to reduce token locations. Yes, I know. But bison asks that the
// macro should be one-line, and treatable as a single statement when followed by a semi-colon.
#define YYLLOC_DEFAULT(Cur, Rhs, N)                           \
do                                                            \
    if(N)                                                     \
    {                                                         \
        (Cur).firstLine = (YYRHSLOC((Rhs), 1)).firstLine;     \
        (Cur).lastLine = (YYRHSLOC((Rhs), N)).lastLine;       \
        (Cur).firstColumn = (YYRHSLOC((Rhs), 1)).firstColumn; \
        (Cur).lastColumn = (YYRHSLOC((Rhs), N)).lastColumn;   \
    }                                                         \
    else                                                      \
    {                                                         \
        (Cur).firstLine = (YYRHSLOC((Rhs), 0)).lastLine;      \
        (Cur).lastLine = (YYRHSLOC((Rhs), 0)).lastLine;       \
        (Cur).firstColumn = (YYRHSLOC((Rhs), 0)).lastColumn;  \
        (Cur).lastColumn = (YYRHSLOC((Rhs), 0)).lastColumn;   \
    }                                                         \
while(0)


#line 101 "src/Slice/Grammar.cpp"

/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yydebug         slice_debug
#define yynerrs         slice_nerrs

/* First part of user prologue.  */
#line 62 "src/Slice/Grammar.y"


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


#line 158 "src/Slice/Grammar.cpp"

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
/* "%code requires" blocks.  */
#line 12 "src/Slice/Grammar.y"


// Define a custom location type for storing the location (and filename) of matched tokens.
#define YYLTYPE Slice::TokenLocation

// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
#define YYMAXDEPTH  10000
#define YYINITDEPTH YYMAXDEPTH

// Newer bison versions allow to disable stack resizing by defining yyoverflow.
#define yyoverflow(a, b, c, d, e, f, g, h) yyerror(a)


#line 219 "src/Slice/Grammar.cpp"

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


/* Unqualified %code blocks.  */
#line 54 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 309 "src/Slice/Grammar.cpp"

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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
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
#define YYLAST   975

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  87
/* YYNRULES -- Number of rules.  */
#define YYNRULES  242
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  352

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
       2,    49,    56,     2,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    50,    46,
      53,    51,    54,    55,     2,     2,     2,     2,     2,     2,
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
       0,   170,   170,   178,   181,   189,   198,   202,   212,   211,
     221,   220,   230,   239,   238,   244,   243,   248,   253,   252,
     258,   257,   262,   267,   266,   272,   271,   276,   281,   280,
     286,   285,   290,   295,   294,   300,   299,   304,   309,   308,
     313,   318,   317,   323,   322,   327,   331,   341,   340,   374,
     378,   389,   400,   399,   425,   433,   442,   451,   454,   458,
     466,   479,   497,   588,   594,   605,   624,   716,   723,   735,
     750,   760,   773,   779,   783,   794,   805,   804,   845,   854,
     857,   861,   869,   875,   879,   890,   915,  1017,  1029,  1042,
    1041,  1080,  1114,  1123,  1126,  1134,  1138,  1147,  1156,  1159,
    1163,  1171,  1193,  1220,  1242,  1268,  1277,  1288,  1297,  1306,
    1315,  1324,  1334,  1348,  1361,  1369,  1375,  1385,  1409,  1434,
    1458,  1489,  1488,  1511,  1510,  1533,  1534,  1540,  1544,  1555,
    1569,  1568,  1602,  1637,  1672,  1677,  1687,  1691,  1700,  1709,
    1712,  1716,  1724,  1730,  1737,  1749,  1761,  1772,  1780,  1794,
    1804,  1820,  1824,  1836,  1835,  1867,  1866,  1884,  1890,  1898,
    1910,  1930,  1937,  1947,  1951,  1990,  1996,  2007,  2010,  2026,
    2042,  2054,  2066,  2077,  2093,  2097,  2106,  2109,  2117,  2121,
    2125,  2129,  2133,  2137,  2141,  2145,  2149,  2153,  2157,  2161,
    2165,  2169,  2173,  2177,  2181,  2185,  2189,  2194,  2198,  2202,
    2221,  2254,  2282,  2288,  2296,  2303,  2315,  2324,  2333,  2373,
    2380,  2387,  2399,  2408,  2422,  2425,  2428,  2431,  2434,  2437,
    2440,  2443,  2446,  2449,  2452,  2455,  2458,  2461,  2464,  2467,
    2470,  2473,  2476,  2479,  2482,  2485,  2488,  2491,  2494,  2497,
    2500,  2503,  2506
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
  "')'", "':'", "'='", "','", "'<'", "'>'", "'?'", "'*'", "$accept",
  "start", "opt_semicolon", "global_meta_data", "meta_data", "definitions",
  "$@1", "$@2", "definition", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8",
  "$@9", "$@10", "$@11", "$@12", "$@13", "$@14", "$@15", "module_def",
  "@16", "exception_id", "exception_decl", "exception_def", "@17",
  "exception_extends", "exception_exports", "type_id", "tag", "optional",
  "tagged_type_id", "exception_export", "struct_id", "struct_decl",
  "struct_def", "@18", "struct_exports", "struct_export", "class_name",
  "class_id", "class_decl", "class_def", "@19", "class_extends", "extends",
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
      58,    61,    44,    60,    62,    63,    42
};
# endif

#define YYPACT_NINF (-277)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-168)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      29,    -4,    -4,    45,  -277,    97,  -277,    -4,  -277,    47,
     -20,  -277,    17,    23,    41,   547,   620,   653,   686,    40,
      57,   719,    80,  -277,  -277,    30,    38,  -277,    84,    96,
    -277,    28,     3,   100,  -277,    32,   102,  -277,   107,   125,
    -277,  -277,   127,  -277,  -277,    -4,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
     -10,  -277,  -277,  -277,  -277,  -277,  -277,  -277,    80,    80,
    -277,    91,  -277,   146,    17,   128,  -277,  -277,  -277,    37,
     129,   128,   131,   130,   128,   167,    37,   140,   128,   105,
    -277,   141,   128,   142,   143,   144,   128,   147,  -277,   145,
     148,  -277,  -277,   150,   146,   146,   752,   149,   151,   152,
     153,   157,   158,   159,   160,    68,   161,    70,   -14,  -277,
    -277,  -277,   154,  -277,  -277,  -277,   428,  -277,  -277,   105,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,   169,   155,  -277,
    -277,  -277,  -277,   752,  -277,  -277,    72,  -277,  -277,   168,
     171,   173,   178,   175,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,   177,   123,
     461,   183,   931,   182,  -277,   156,   105,   358,   184,   185,
     785,    80,    83,  -277,   752,   123,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,   188,   931,   189,   428,  -277,  -277,     0,
      59,   180,   146,   146,   190,  -277,   818,  -277,   395,  -277,
     192,   872,   191,  -277,  -277,  -277,  -277,   146,  -277,  -277,
    -277,  -277,  -277,   461,  -277,   146,   146,   193,   194,  -277,
     818,  -277,  -277,   197,  -277,   198,   199,  -277,   201,   123,
     202,   165,   203,   428,  -277,  -277,   195,   872,   204,   358,
    -277,   902,   146,   146,    92,   320,  -277,   196,  -277,  -277,
     205,  -277,  -277,  -277,   123,   461,  -277,  -277,  -277,  -277,
    -277,  -277,   123,   123,  -277,   395,   146,   146,  -277,  -277,
     209,   507,  -277,  -277,   110,  -277,  -277,  -277,  -277,   207,
    -277,    80,   -22,   358,   851,  -277,  -277,  -277,  -277,  -277,
     165,   165,   395,  -277,  -277,  -277,   931,  -277,   232,  -277,
    -277,  -277,  -277,   243,  -277,   818,   243,    80,   586,  -277,
    -277,  -277,   931,  -277,   206,  -277,  -277,  -277,   818,   586,
    -277,  -277
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       7,     0,     0,     0,     8,     0,     2,   203,   205,     0,
       0,     1,     7,     0,     0,     0,     0,     0,     0,     0,
       0,   155,     7,    10,    13,    51,    27,    28,    75,    32,
      33,    88,    92,    17,    18,   129,    22,    23,    37,    40,
     153,    41,    45,   202,     6,     0,     5,     9,    46,    47,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,    83,
       0,    84,   127,   128,    49,    50,    73,    74,     7,     7,
     151,     0,   152,     0,     7,     4,    93,    94,    52,     0,
       0,     4,     0,     0,     4,    96,     0,     0,     4,     0,
     130,     0,     4,     0,     0,     0,     4,     0,   204,     0,
       0,   176,   177,     0,     0,     0,   162,   178,   180,   182,
     184,   186,   188,   190,   192,   194,   197,   199,     0,    11,
       3,    14,     0,    54,    26,    29,     0,    31,    34,     0,
      89,    91,    16,    19,   134,   135,   136,   133,     0,    21,
      24,    36,    39,   162,    42,    44,     7,    85,    86,     0,
       0,   159,     0,   158,   161,   179,   181,   183,   185,   187,
     189,   191,   193,   195,   196,   198,   201,   200,     0,     0,
       0,     0,     0,     0,    95,     0,     0,     0,     0,     0,
       0,     7,     0,   156,   162,     0,   210,   211,   209,   206,
     207,   208,   213,     0,     0,     0,     0,    64,    68,     0,
       0,   105,     0,     0,    80,    82,   112,    77,     0,   132,
       0,     0,     0,   154,    48,   147,   148,     0,   163,   160,
     164,   157,   212,     0,    71,     0,     0,   101,    58,    72,
     104,    53,    79,     0,    63,     0,     0,    67,     0,     0,
     107,     0,   109,     0,    60,   111,     0,     0,     0,     0,
     116,     0,     0,     0,     0,     0,   142,   140,   115,   131,
       0,    57,    69,    70,     0,     0,   103,    61,    62,    65,
      66,   106,     0,     0,    78,     0,     0,     0,   125,   126,
      99,   104,    90,   139,     0,   113,   114,   117,   119,     0,
     165,     7,     0,     0,     0,   102,    56,   108,   110,    98,
     113,   114,     0,   118,   120,   123,     0,   121,   166,   138,
     149,   150,    97,   175,   168,   172,   175,     7,     0,   124,
     170,   122,     0,   174,   144,   145,   146,   169,   173,     0,
     171,   143
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -277,  -277,    31,  -277,   -18,    -5,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -221,
    -184,  -181,  -180,  -259,  -277,  -277,  -277,  -277,  -277,  -206,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,    20,  -277,  -276,
     -62,  -277,   -11,  -277,    -6,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -136,  -277,  -255,  -277,   -87,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -145,  -277,  -277,   -64,  -277,   -70,
     -75,   -77,    36,   266,  -196,  -277,   -15
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,   141,     4,     5,     6,    12,    94,    23,    95,
     107,   108,   111,   112,   100,   101,   103,   104,   113,   114,
     116,   117,    24,   119,    25,    26,    27,   142,    98,   215,
     244,   245,   246,   247,   248,    28,    29,    30,   102,   193,
     224,    31,    32,    33,    34,   195,   105,    99,   150,   268,
     249,   225,   274,   275,   276,   336,   333,   300,    35,    36,
      37,   158,   156,   110,   232,   277,   343,   344,    38,    39,
      40,    41,   115,    91,   172,   173,   239,   311,   312,   339,
     137,   261,     8,     9,   212,    42,   174
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      81,    83,    85,    87,    93,   123,    92,    47,   221,   242,
     252,   222,   223,   194,   303,    96,   138,   -12,   198,   319,
      46,   188,   281,   120,   143,   121,   122,   327,     7,   -12,
     328,   151,    45,   253,   157,   121,   122,   189,   260,   262,
     -87,   -87,    96,    43,    96,    11,   332,   169,   170,   254,
     272,   273,   106,    97,     1,   109,     2,   294,   329,   241,
     229,   282,   283,   291,   316,   -12,     1,   334,     2,    48,
     124,   125,   121,   122,   157,   -87,    49,   -55,   -87,  -137,
      97,   118,    97,   347,   -25,    44,   296,   297,   315,   139,
     272,   273,   256,    88,   121,   122,   317,   318,    13,    45,
      14,    15,    16,    17,    18,    19,    20,    21,   257,     1,
      89,     2,   282,   283,   211,   226,   238,     1,   121,   122,
     -12,   157,    22,   183,   184,   186,   187,   240,   192,   154,
     211,   -76,   145,   307,   308,   148,   155,   250,   126,   153,
     121,   122,   -30,   160,   255,   258,   -15,   164,   -20,   206,
     207,   323,   324,   -35,   278,   208,   209,   210,   121,   122,
     280,   199,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   -38,   214,   -43,   140,   144,   147,   136,   146,   231,
     149,   121,   122,   237,   211,   236,   152,   159,   161,   162,
     301,   163,   166,   165,   278,   305,   306,   167,   192,   168,
     264,   190,   197,   228,   175,   298,   176,   177,   178,   211,
     267,   265,   179,   180,   181,   182,   185,   211,   211,   320,
     321,   196,   200,   201,   202,   214,   203,   204,   205,   216,
     227,   259,   233,   234,   243,   286,   263,   251,   269,   279,
     285,   295,   313,   310,   284,   192,   287,   288,   289,   335,
     290,   231,   302,   292,   293,   322,   325,   338,   349,   314,
     304,   299,   351,   345,   337,   348,   341,   214,    10,     0,
       0,     0,     0,     0,   345,     0,     0,   267,     0,     0,
       0,     0,     0,     0,     0,     0,   286,     0,     0,     0,
       0,     0,     0,   326,     0,   231,     0,     0,     0,   331,
       0,     0,     0,     0,   267,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   342,
     340,   309,     0,   346,     0,     0,     0,     0,     0,     0,
       0,   310,     0,   350,   346,     0,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,  -166,  -166,     0,     0,     0,     0,  -166,
    -166,  -166,     0,     0,     0,  -166,  -166,  -166,     0,   230,
       0,     0,     0,  -166,  -166,     0,     0,     0,     0,  -167,
       0,     0,  -167,    -7,    -7,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,    -7,     0,     0,     0,    -7,    -7,    -7,    -7,
       0,     0,     0,    -7,    -7,     1,   266,     0,     0,     0,
       0,    -7,    -7,     0,     0,     0,  -141,     0,     0,     0,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,
       0,     0,     0,    -7,    -7,    -7,    -7,     0,     0,   191,
      -7,    -7,     1,     0,     0,     0,     0,     0,    -7,    -7,
       0,     0,     0,  -100,    -7,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,    -7,     0,     0,     0,     0,    -7,    -7,    -7,
       0,     0,   213,    -7,    -7,     1,     0,     0,     0,     0,
       0,    -7,    -7,     0,     0,     0,   -81,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,     0,     0,     0,     0,
      -7,    -7,    -7,     0,     0,     0,    -7,    -7,     1,     0,
       0,     0,     0,     0,    -7,    -7,     0,     0,     0,   -59,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,     0,
       0,     0,   264,     0,     0,     0,     0,     0,  -115,  -115,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,     0,
       0,     0,    79,     0,     0,     0,     0,     0,    80,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
       0,   121,   122,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,     0,     0,     0,    82,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,     0,     0,     0,    84,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
       0,    86,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,     0,     0,     0,    90,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,     0,   171,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,     0,     0,     0,
     235,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,     0,   264,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,     0,     0,     0,   330,   270,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     0,     0,     0,
     271,   217,   218,   136,     0,     0,     0,   121,   122,     0,
       0,     0,     0,     0,     0,   219,   220,   270,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     0,     0,     0,
       0,   217,   218,   136,     0,     0,     0,   121,   122,     0,
       0,     0,     0,     0,     0,   219,   220,   127,   128,   129,
     130,   131,   132,   133,   134,   135,     0,     0,     0,     0,
     217,   218,   136,     0,     0,     0,   121,   122,     0,     0,
       0,     0,     0,     0,   219,   220
};

static const yytype_int16 yycheck[] =
{
      15,    16,    17,    18,    22,    80,    21,    12,   192,   205,
     216,   192,   192,   149,   269,    12,    93,     0,   163,   295,
      40,    35,   243,    33,    99,    35,    36,    49,    32,     0,
      52,   106,    52,    33,   109,    35,    36,    51,   222,   223,
      12,    13,    12,     7,    12,     0,   322,   124,   125,    49,
     231,   231,    32,    50,    37,    35,    39,   263,   313,   204,
     196,   245,   246,   259,   285,    48,    37,   326,    39,    46,
      88,    89,    35,    36,   149,    47,    35,    47,    50,    47,
      50,    45,    50,   342,    46,    38,   267,   267,   284,    94,
     271,   271,    33,    53,    35,    36,   292,   293,     1,    52,
       3,     4,     5,     6,     7,     8,     9,    10,    49,    37,
      53,    39,   296,   297,   189,   192,    33,    37,    35,    36,
      48,   196,    25,    55,    56,    55,    56,   202,   146,    24,
     205,    47,   101,    41,    42,   104,    31,   214,    47,   108,
      35,    36,    46,   112,   219,   220,    46,   116,    46,    26,
      27,    41,    42,    46,   231,    32,    33,    34,    35,    36,
     237,   166,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    46,   190,    46,    46,    46,    46,    31,    47,   197,
      13,    35,    36,   201,   259,   200,    46,    46,    46,    46,
     267,    47,    47,    46,   271,   272,   273,    49,   216,    49,
      35,    47,    47,    47,    55,   267,    55,    55,    55,   284,
     228,   226,    55,    55,    55,    55,    55,   292,   293,   296,
     297,    52,    54,    52,    51,   243,    48,    52,    51,    46,
      48,    51,    48,    48,    46,   250,    46,    48,    46,    48,
      46,    46,    46,    11,    51,   263,    49,    49,    49,   326,
      49,   269,    48,    51,    51,    46,    49,    14,    52,    54,
     271,   267,   349,   338,   328,   342,   336,   285,     2,    -1,
      -1,    -1,    -1,    -1,   349,    -1,    -1,   295,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   301,    -1,    -1,    -1,
      -1,    -1,    -1,   311,    -1,   313,    -1,    -1,    -1,   314,
      -1,    -1,    -1,    -1,   322,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   337,
     335,     1,    -1,   338,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    11,    -1,   348,   349,    -1,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    29,
      30,    31,    -1,    -1,    -1,    35,    36,    37,    -1,     1,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    52,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    28,    29,    30,    31,
      -1,    -1,    -1,    35,    36,    37,     1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    48,    -1,    -1,    -1,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    28,    29,    30,    31,    -1,    -1,     1,
      35,    36,    37,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    29,    30,    31,
      -1,    -1,     1,    35,    36,    37,    -1,    -1,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    48,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,
      29,    30,    31,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    48,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    41,    42,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    35,    -1,    -1,    -1,    -1,    -1,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    35,    36,     3,     4,     5,     6,     7,     8,     9,
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
      -1,    -1,    -1,    35,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    35,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      28,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    43,    44,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,
      29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    37,    39,    58,    60,    61,    62,    32,   139,   140,
     140,     0,    63,     1,     3,     4,     5,     6,     7,     8,
       9,    10,    25,    65,    79,    81,    82,    83,    92,    93,
      94,    98,    99,   100,   101,   115,   116,   117,   125,   126,
     127,   128,   142,   139,    38,    52,    40,    62,    46,    35,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    35,
      41,   143,    35,   143,    35,   143,    35,   143,    53,    53,
      35,   130,   143,    61,    64,    66,    12,    50,    85,   104,
      71,    72,    95,    73,    74,   103,   104,    67,    68,   104,
     120,    69,    70,    75,    76,   129,    77,    78,   139,    80,
      33,    35,    36,   137,    61,    61,    47,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    31,   137,   138,    62,
      46,    59,    84,   137,    46,    59,    47,    46,    59,    13,
     105,   137,    46,    59,    24,    31,   119,   137,   118,    46,
      59,    46,    46,    47,    59,    46,    47,    49,    49,   138,
     138,    35,   131,   132,   143,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    56,    55,    55,    56,    35,    51,
      47,     1,    61,    96,   119,   102,    52,    47,   131,    62,
      54,    52,    51,    48,    52,    51,    26,    27,    32,    33,
      34,   137,   141,     1,    61,    86,    46,    29,    30,    43,
      44,    87,    88,    89,    97,   108,   138,    48,    47,   119,
       1,    61,   121,    48,    48,    35,   143,    61,    33,   133,
     137,   131,   141,    46,    87,    88,    89,    90,    91,   107,
     138,    48,    96,    33,    49,   137,    33,    49,   137,    51,
      87,   138,    87,    46,    35,   143,     1,    61,   106,    46,
      15,    28,    88,    89,   109,   110,   111,   122,   138,    48,
     138,    86,    87,    87,    51,    46,   143,    49,    49,    49,
      49,   141,    51,    51,    96,    46,    88,    89,   107,   111,
     114,   138,    48,   121,   109,   138,   138,    41,    42,     1,
      11,   134,   135,    46,    54,   141,    86,   141,   141,   106,
     138,   138,    46,    41,    42,    49,    61,    49,    52,   121,
      35,   143,   106,   113,    90,   138,   112,   134,    14,   136,
     143,   136,    61,   123,   124,   137,   143,    90,   138,    52,
     143,   123
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    57,    58,    59,    59,    60,    61,    61,    63,    62,
      64,    62,    62,    66,    65,    67,    65,    65,    68,    65,
      69,    65,    65,    70,    65,    71,    65,    65,    72,    65,
      73,    65,    65,    74,    65,    75,    65,    65,    76,    65,
      65,    77,    65,    78,    65,    65,    65,    80,    79,    81,
      81,    82,    84,    83,    85,    85,    86,    86,    86,    86,
      87,    88,    88,    88,    88,    89,    89,    89,    89,    90,
      90,    90,    91,    92,    92,    93,    95,    94,    96,    96,
      96,    96,    97,    98,    98,    99,    99,    99,   100,   102,
     101,   103,   103,   104,   104,   105,   105,   106,   106,   106,
     106,   107,   107,   107,   107,   108,   108,   108,   108,   108,
     108,   108,   108,   109,   109,   109,   109,   110,   110,   110,
     110,   112,   111,   113,   111,   114,   114,   115,   115,   116,
     118,   117,   119,   119,   119,   119,   120,   120,   121,   121,
     121,   121,   122,   123,   123,   124,   124,   125,   125,   126,
     126,   127,   127,   129,   128,   130,   128,   131,   131,   132,
     132,   132,   132,   133,   133,   134,   134,   135,   135,   135,
     135,   135,   135,   135,   136,   136,   137,   137,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   138,   139,   139,   140,   140,   141,   141,   141,   141,
     141,   141,   142,   142,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143
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
       7,     2,     0,     1,     1,     2,     0,     4,     3,     2,
       0,     1,     3,     2,     1,     1,     3,     2,     4,     2,
       4,     2,     1,     2,     2,     1,     1,     2,     3,     2,
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
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
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


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (yylocationp);
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
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yytype, yyvaluep, yylocationp);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
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

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

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

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYPTRDIFF_T yystacksize;

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
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 171 "src/Slice/Grammar.y"
{
}
#line 1985 "src/Slice/Grammar.cpp"
    break;

  case 3:
#line 179 "src/Slice/Grammar.y"
{
}
#line 1992 "src/Slice/Grammar.cpp"
    break;

  case 4:
#line 182 "src/Slice/Grammar.y"
{
}
#line 1999 "src/Slice/Grammar.cpp"
    break;

  case 5:
#line 190 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 2007 "src/Slice/Grammar.cpp"
    break;

  case 6:
#line 199 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 2015 "src/Slice/Grammar.cpp"
    break;

  case 7:
#line 203 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 2023 "src/Slice/Grammar.cpp"
    break;

  case 8:
#line 212 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
#line 2035 "src/Slice/Grammar.cpp"
    break;

  case 10:
#line 221 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2048 "src/Slice/Grammar.cpp"
    break;

  case 12:
#line 231 "src/Slice/Grammar.y"
{
}
#line 2055 "src/Slice/Grammar.cpp"
    break;

  case 13:
#line 239 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 2063 "src/Slice/Grammar.cpp"
    break;

  case 15:
#line 244 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 2071 "src/Slice/Grammar.cpp"
    break;

  case 17:
#line 249 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 2079 "src/Slice/Grammar.cpp"
    break;

  case 18:
#line 253 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 2087 "src/Slice/Grammar.cpp"
    break;

  case 20:
#line 258 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 2095 "src/Slice/Grammar.cpp"
    break;

  case 22:
#line 263 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 2103 "src/Slice/Grammar.cpp"
    break;

  case 23:
#line 267 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 2111 "src/Slice/Grammar.cpp"
    break;

  case 25:
#line 272 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2119 "src/Slice/Grammar.cpp"
    break;

  case 27:
#line 277 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 2127 "src/Slice/Grammar.cpp"
    break;

  case 28:
#line 281 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 2135 "src/Slice/Grammar.cpp"
    break;

  case 30:
#line 286 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2143 "src/Slice/Grammar.cpp"
    break;

  case 32:
#line 291 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 2151 "src/Slice/Grammar.cpp"
    break;

  case 33:
#line 295 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 2159 "src/Slice/Grammar.cpp"
    break;

  case 35:
#line 300 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2167 "src/Slice/Grammar.cpp"
    break;

  case 37:
#line 305 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2175 "src/Slice/Grammar.cpp"
    break;

  case 38:
#line 309 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2183 "src/Slice/Grammar.cpp"
    break;

  case 40:
#line 314 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2191 "src/Slice/Grammar.cpp"
    break;

  case 41:
#line 318 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2199 "src/Slice/Grammar.cpp"
    break;

  case 43:
#line 323 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2207 "src/Slice/Grammar.cpp"
    break;

  case 45:
#line 328 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2215 "src/Slice/Grammar.cpp"
    break;

  case 46:
#line 332 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2223 "src/Slice/Grammar.cpp"
    break;

  case 47:
#line 341 "src/Slice/Grammar.y"
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
#line 2244 "src/Slice/Grammar.cpp"
    break;

  case 48:
#line 358 "src/Slice/Grammar.y"
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
#line 2260 "src/Slice/Grammar.cpp"
    break;

  case 49:
#line 375 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2268 "src/Slice/Grammar.cpp"
    break;

  case 50:
#line 379 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2278 "src/Slice/Grammar.cpp"
    break;

  case 51:
#line 390 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2287 "src/Slice/Grammar.cpp"
    break;

  case 52:
#line 400 "src/Slice/Grammar.y"
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
#line 2304 "src/Slice/Grammar.cpp"
    break;

  case 53:
#line 413 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2316 "src/Slice/Grammar.cpp"
    break;

  case 54:
#line 426 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2328 "src/Slice/Grammar.cpp"
    break;

  case 55:
#line 434 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2336 "src/Slice/Grammar.cpp"
    break;

  case 56:
#line 443 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2349 "src/Slice/Grammar.cpp"
    break;

  case 57:
#line 452 "src/Slice/Grammar.y"
{
}
#line 2356 "src/Slice/Grammar.cpp"
    break;

  case 58:
#line 455 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2364 "src/Slice/Grammar.cpp"
    break;

  case 59:
#line 459 "src/Slice/Grammar.y"
{
}
#line 2371 "src/Slice/Grammar.cpp"
    break;

  case 60:
#line 467 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    yyval = typestring;
}
#line 2383 "src/Slice/Grammar.cpp"
    break;

  case 61:
#line 480 "src/Slice/Grammar.y"
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
#line 2405 "src/Slice/Grammar.cpp"
    break;

  case 62:
#line 498 "src/Slice/Grammar.y"
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
#line 2500 "src/Slice/Grammar.cpp"
    break;

  case 63:
#line 589 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2510 "src/Slice/Grammar.cpp"
    break;

  case 64:
#line 595 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2520 "src/Slice/Grammar.cpp"
    break;

  case 65:
#line 606 "src/Slice/Grammar.y"
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
#line 2543 "src/Slice/Grammar.cpp"
    break;

  case 66:
#line 625 "src/Slice/Grammar.y"
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
#line 2639 "src/Slice/Grammar.cpp"
    break;

  case 67:
#line 717 "src/Slice/Grammar.y"
{
    unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2650 "src/Slice/Grammar.cpp"
    break;

  case 68:
#line 724 "src/Slice/Grammar.y"
{
    unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2661 "src/Slice/Grammar.cpp"
    break;

  case 69:
#line 736 "src/Slice/Grammar.y"
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
#line 2680 "src/Slice/Grammar.cpp"
    break;

  case 70:
#line 751 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

    // Infer the type to be optional for backwards compatability.
    m->type = new Optional(ts->v.first);
    m->name = ts->v.second;
    yyval = m;
}
#line 2694 "src/Slice/Grammar.cpp"
    break;

  case 71:
#line 761 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2706 "src/Slice/Grammar.cpp"
    break;

  case 73:
#line 780 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2714 "src/Slice/Grammar.cpp"
    break;

  case 74:
#line 784 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2724 "src/Slice/Grammar.cpp"
    break;

  case 75:
#line 795 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2733 "src/Slice/Grammar.cpp"
    break;

  case 76:
#line 805 "src/Slice/Grammar.y"
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
#line 2755 "src/Slice/Grammar.cpp"
    break;

  case 77:
#line 823 "src/Slice/Grammar.y"
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
#line 2777 "src/Slice/Grammar.cpp"
    break;

  case 78:
#line 846 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2790 "src/Slice/Grammar.cpp"
    break;

  case 79:
#line 855 "src/Slice/Grammar.y"
{
}
#line 2797 "src/Slice/Grammar.cpp"
    break;

  case 80:
#line 858 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2805 "src/Slice/Grammar.cpp"
    break;

  case 81:
#line 862 "src/Slice/Grammar.y"
{
}
#line 2812 "src/Slice/Grammar.cpp"
    break;

  case 83:
#line 876 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2820 "src/Slice/Grammar.cpp"
    break;

  case 84:
#line 880 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2830 "src/Slice/Grammar.cpp"
    break;

  case 85:
#line 891 "src/Slice/Grammar.y"
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
#line 2859 "src/Slice/Grammar.cpp"
    break;

  case 86:
#line 916 "src/Slice/Grammar.y"
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
#line 2965 "src/Slice/Grammar.cpp"
    break;

  case 87:
#line 1018 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2976 "src/Slice/Grammar.cpp"
    break;

  case 88:
#line 1030 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false);
    yyval = cl;
}
#line 2987 "src/Slice/Grammar.cpp"
    break;

  case 89:
#line 1042 "src/Slice/Grammar.y"
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
#line 3013 "src/Slice/Grammar.cpp"
    break;

  case 90:
#line 1064 "src/Slice/Grammar.y"
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
#line 3029 "src/Slice/Grammar.cpp"
    break;

  case 91:
#line 1081 "src/Slice/Grammar.y"
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
#line 3067 "src/Slice/Grammar.cpp"
    break;

  case 92:
#line 1115 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 3075 "src/Slice/Grammar.cpp"
    break;

  case 93:
#line 1124 "src/Slice/Grammar.y"
{
}
#line 3082 "src/Slice/Grammar.cpp"
    break;

  case 94:
#line 1127 "src/Slice/Grammar.y"
{
}
#line 3089 "src/Slice/Grammar.cpp"
    break;

  case 95:
#line 1135 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3097 "src/Slice/Grammar.cpp"
    break;

  case 96:
#line 1139 "src/Slice/Grammar.y"
{
    yyval = new ClassListTok;
}
#line 3105 "src/Slice/Grammar.cpp"
    break;

  case 97:
#line 1148 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3118 "src/Slice/Grammar.cpp"
    break;

  case 98:
#line 1157 "src/Slice/Grammar.y"
{
}
#line 3125 "src/Slice/Grammar.cpp"
    break;

  case 99:
#line 1160 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3133 "src/Slice/Grammar.cpp"
    break;

  case 100:
#line 1164 "src/Slice/Grammar.y"
{
}
#line 3140 "src/Slice/Grammar.cpp"
    break;

  case 101:
#line 1172 "src/Slice/Grammar.y"
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
#line 3166 "src/Slice/Grammar.cpp"
    break;

  case 102:
#line 1194 "src/Slice/Grammar.y"
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
#line 3197 "src/Slice/Grammar.cpp"
    break;

  case 103:
#line 1221 "src/Slice/Grammar.y"
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
#line 3223 "src/Slice/Grammar.cpp"
    break;

  case 104:
#line 1243 "src/Slice/Grammar.y"
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
#line 3248 "src/Slice/Grammar.cpp"
    break;

  case 105:
#line 1269 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 3261 "src/Slice/Grammar.cpp"
    break;

  case 106:
#line 1278 "src/Slice/Grammar.y"
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
#line 3276 "src/Slice/Grammar.cpp"
    break;

  case 107:
#line 1289 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3289 "src/Slice/Grammar.cpp"
    break;

  case 108:
#line 1298 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3302 "src/Slice/Grammar.cpp"
    break;

  case 109:
#line 1307 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3315 "src/Slice/Grammar.cpp"
    break;

  case 110:
#line 1316 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3328 "src/Slice/Grammar.cpp"
    break;

  case 111:
#line 1325 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3342 "src/Slice/Grammar.cpp"
    break;

  case 112:
#line 1335 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("missing data member name");
}
#line 3355 "src/Slice/Grammar.cpp"
    break;

  case 113:
#line 1349 "src/Slice/Grammar.y"
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
#line 3372 "src/Slice/Grammar.cpp"
    break;

  case 114:
#line 1362 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);

    // Infer the type to be optional for backwards compatability.
    m->type = new Optional(TypePtr::dynamicCast(yyvsp[0]));
    yyval = m;
}
#line 3384 "src/Slice/Grammar.cpp"
    break;

  case 115:
#line 1370 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3394 "src/Slice/Grammar.cpp"
    break;

  case 116:
#line 1376 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    yyval = m;
}
#line 3403 "src/Slice/Grammar.cpp"
    break;

  case 117:
#line 1386 "src/Slice/Grammar.y"
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
#line 3431 "src/Slice/Grammar.cpp"
    break;

  case 118:
#line 1410 "src/Slice/Grammar.y"
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
#line 3460 "src/Slice/Grammar.cpp"
    break;

  case 119:
#line 1435 "src/Slice/Grammar.y"
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
#line 3488 "src/Slice/Grammar.cpp"
    break;

  case 120:
#line 1459 "src/Slice/Grammar.y"
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
#line 3517 "src/Slice/Grammar.cpp"
    break;

  case 121:
#line 1489 "src/Slice/Grammar.y"
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
#line 3533 "src/Slice/Grammar.cpp"
    break;

  case 122:
#line 1501 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3547 "src/Slice/Grammar.cpp"
    break;

  case 123:
#line 1511 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3559 "src/Slice/Grammar.cpp"
    break;

  case 124:
#line 1519 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3573 "src/Slice/Grammar.cpp"
    break;

  case 127:
#line 1541 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3581 "src/Slice/Grammar.cpp"
    break;

  case 128:
#line 1545 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3591 "src/Slice/Grammar.cpp"
    break;

  case 129:
#line 1556 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3603 "src/Slice/Grammar.cpp"
    break;

  case 130:
#line 1569 "src/Slice/Grammar.y"
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
#line 3624 "src/Slice/Grammar.cpp"
    break;

  case 131:
#line 1586 "src/Slice/Grammar.y"
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
#line 3640 "src/Slice/Grammar.cpp"
    break;

  case 132:
#line 1603 "src/Slice/Grammar.y"
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
#line 3679 "src/Slice/Grammar.cpp"
    break;

  case 133:
#line 1638 "src/Slice/Grammar.y"
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
#line 3718 "src/Slice/Grammar.cpp"
    break;

  case 134:
#line 1673 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new ClassListTok; // Dummy
}
#line 3727 "src/Slice/Grammar.cpp"
    break;

  case 135:
#line 1678 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3736 "src/Slice/Grammar.cpp"
    break;

  case 136:
#line 1688 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3744 "src/Slice/Grammar.cpp"
    break;

  case 137:
#line 1692 "src/Slice/Grammar.y"
{
    yyval = new ClassListTok;
}
#line 3752 "src/Slice/Grammar.cpp"
    break;

  case 138:
#line 1701 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3765 "src/Slice/Grammar.cpp"
    break;

  case 139:
#line 1710 "src/Slice/Grammar.y"
{
}
#line 3772 "src/Slice/Grammar.cpp"
    break;

  case 140:
#line 1713 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3780 "src/Slice/Grammar.cpp"
    break;

  case 141:
#line 1717 "src/Slice/Grammar.y"
{
}
#line 3787 "src/Slice/Grammar.cpp"
    break;

  case 143:
#line 1731 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3798 "src/Slice/Grammar.cpp"
    break;

  case 144:
#line 1738 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3809 "src/Slice/Grammar.cpp"
    break;

  case 145:
#line 1750 "src/Slice/Grammar.y"
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
#line 3825 "src/Slice/Grammar.cpp"
    break;

  case 146:
#line 1762 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3835 "src/Slice/Grammar.cpp"
    break;

  case 147:
#line 1773 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3847 "src/Slice/Grammar.cpp"
    break;

  case 148:
#line 1781 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3860 "src/Slice/Grammar.cpp"
    break;

  case 149:
#line 1795 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3874 "src/Slice/Grammar.cpp"
    break;

  case 150:
#line 1805 "src/Slice/Grammar.y"
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
#line 3889 "src/Slice/Grammar.cpp"
    break;

  case 151:
#line 1821 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3897 "src/Slice/Grammar.cpp"
    break;

  case 152:
#line 1825 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3907 "src/Slice/Grammar.cpp"
    break;

  case 153:
#line 1836 "src/Slice/Grammar.y"
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
#line 3927 "src/Slice/Grammar.cpp"
    break;

  case 154:
#line 1852 "src/Slice/Grammar.y"
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
#line 3945 "src/Slice/Grammar.cpp"
    break;

  case 155:
#line 1867 "src/Slice/Grammar.y"
{
    unit->error("missing enumeration name");
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3957 "src/Slice/Grammar.cpp"
    break;

  case 156:
#line 1875 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3966 "src/Slice/Grammar.cpp"
    break;

  case 157:
#line 1885 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 3976 "src/Slice/Grammar.cpp"
    break;

  case 158:
#line 1891 "src/Slice/Grammar.y"
{
}
#line 3983 "src/Slice/Grammar.cpp"
    break;

  case 159:
#line 1899 "src/Slice/Grammar.y"
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
#line 3999 "src/Slice/Grammar.cpp"
    break;

  case 160:
#line 1911 "src/Slice/Grammar.y"
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
#line 4023 "src/Slice/Grammar.cpp"
    break;

  case 161:
#line 1931 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 4034 "src/Slice/Grammar.cpp"
    break;

  case 162:
#line 1938 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 4043 "src/Slice/Grammar.cpp"
    break;

  case 163:
#line 1948 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4051 "src/Slice/Grammar.cpp"
    break;

  case 164:
#line 1952 "src/Slice/Grammar.y"
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
#line 4089 "src/Slice/Grammar.cpp"
    break;

  case 165:
#line 1991 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = true;
    yyval = out;
}
#line 4099 "src/Slice/Grammar.cpp"
    break;

  case 166:
#line 1997 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = false;
    yyval = out;
}
#line 4109 "src/Slice/Grammar.cpp"
    break;

  case 167:
#line 2008 "src/Slice/Grammar.y"
{
}
#line 4116 "src/Slice/Grammar.cpp"
    break;

  case 168:
#line 2011 "src/Slice/Grammar.y"
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
#line 4136 "src/Slice/Grammar.cpp"
    break;

  case 169:
#line 2027 "src/Slice/Grammar.y"
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
#line 4156 "src/Slice/Grammar.cpp"
    break;

  case 170:
#line 2043 "src/Slice/Grammar.y"
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
#line 4172 "src/Slice/Grammar.cpp"
    break;

  case 171:
#line 2055 "src/Slice/Grammar.y"
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
#line 4188 "src/Slice/Grammar.cpp"
    break;

  case 172:
#line 2067 "src/Slice/Grammar.y"
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
#line 4203 "src/Slice/Grammar.cpp"
    break;

  case 173:
#line 2078 "src/Slice/Grammar.y"
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
#line 4218 "src/Slice/Grammar.cpp"
    break;

  case 174:
#line 2094 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4226 "src/Slice/Grammar.cpp"
    break;

  case 175:
#line 2098 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 4234 "src/Slice/Grammar.cpp"
    break;

  case 176:
#line 2107 "src/Slice/Grammar.y"
{
}
#line 4241 "src/Slice/Grammar.cpp"
    break;

  case 177:
#line 2110 "src/Slice/Grammar.y"
{
}
#line 4248 "src/Slice/Grammar.cpp"
    break;

  case 178:
#line 2118 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindByte);
}
#line 4256 "src/Slice/Grammar.cpp"
    break;

  case 179:
#line 2122 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindByte);
}
#line 4264 "src/Slice/Grammar.cpp"
    break;

  case 180:
#line 2126 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindBool);
}
#line 4272 "src/Slice/Grammar.cpp"
    break;

  case 181:
#line 2130 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindBool);
}
#line 4280 "src/Slice/Grammar.cpp"
    break;

  case 182:
#line 2134 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindShort);
}
#line 4288 "src/Slice/Grammar.cpp"
    break;

  case 183:
#line 2138 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindShort);
}
#line 4296 "src/Slice/Grammar.cpp"
    break;

  case 184:
#line 2142 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindInt);
}
#line 4304 "src/Slice/Grammar.cpp"
    break;

  case 185:
#line 2146 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindInt);
}
#line 4312 "src/Slice/Grammar.cpp"
    break;

  case 186:
#line 2150 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindLong);
}
#line 4320 "src/Slice/Grammar.cpp"
    break;

  case 187:
#line 2154 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindLong);
}
#line 4328 "src/Slice/Grammar.cpp"
    break;

  case 188:
#line 2158 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindFloat);
}
#line 4336 "src/Slice/Grammar.cpp"
    break;

  case 189:
#line 2162 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindFloat);
}
#line 4344 "src/Slice/Grammar.cpp"
    break;

  case 190:
#line 2166 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindDouble);
}
#line 4352 "src/Slice/Grammar.cpp"
    break;

  case 191:
#line 2170 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindDouble);
}
#line 4360 "src/Slice/Grammar.cpp"
    break;

  case 192:
#line 2174 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindString);
}
#line 4368 "src/Slice/Grammar.cpp"
    break;

  case 193:
#line 2178 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindString);
}
#line 4376 "src/Slice/Grammar.cpp"
    break;

  case 194:
#line 2182 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindObject);
}
#line 4384 "src/Slice/Grammar.cpp"
    break;

  case 195:
#line 2186 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObjectProxy);
}
#line 4392 "src/Slice/Grammar.cpp"
    break;

  case 196:
#line 2190 "src/Slice/Grammar.y"
{
    // TODO: equivalent to ICE_OBJECT ? above, need to merge KindObject / KindObjectProxy
    yyval = unit->builtin(Builtin::KindObjectProxy);
}
#line 4401 "src/Slice/Grammar.cpp"
    break;

  case 197:
#line 2195 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindValue);
}
#line 4409 "src/Slice/Grammar.cpp"
    break;

  case 198:
#line 2199 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindValue);
}
#line 4417 "src/Slice/Grammar.cpp"
    break;

  case 199:
#line 2203 "src/Slice/Grammar.y"
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
#line 4440 "src/Slice/Grammar.cpp"
    break;

  case 200:
#line 2222 "src/Slice/Grammar.y"
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
#line 4477 "src/Slice/Grammar.cpp"
    break;

  case 201:
#line 2255 "src/Slice/Grammar.y"
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
#line 4504 "src/Slice/Grammar.cpp"
    break;

  case 202:
#line 2283 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4514 "src/Slice/Grammar.cpp"
    break;

  case 203:
#line 2289 "src/Slice/Grammar.y"
{
}
#line 4521 "src/Slice/Grammar.cpp"
    break;

  case 204:
#line 2297 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4532 "src/Slice/Grammar.cpp"
    break;

  case 205:
#line 2304 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4543 "src/Slice/Grammar.cpp"
    break;

  case 206:
#line 2316 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4556 "src/Slice/Grammar.cpp"
    break;

  case 207:
#line 2325 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4569 "src/Slice/Grammar.cpp"
    break;

  case 208:
#line 2334 "src/Slice/Grammar.y"
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
#line 4613 "src/Slice/Grammar.cpp"
    break;

  case 209:
#line 2374 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4624 "src/Slice/Grammar.cpp"
    break;

  case 210:
#line 2381 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4635 "src/Slice/Grammar.cpp"
    break;

  case 211:
#line 2388 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4646 "src/Slice/Grammar.cpp"
    break;

  case 212:
#line 2400 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4659 "src/Slice/Grammar.cpp"
    break;

  case 213:
#line 2409 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4672 "src/Slice/Grammar.cpp"
    break;

  case 214:
#line 2423 "src/Slice/Grammar.y"
{
}
#line 4679 "src/Slice/Grammar.cpp"
    break;

  case 215:
#line 2426 "src/Slice/Grammar.y"
{
}
#line 4686 "src/Slice/Grammar.cpp"
    break;

  case 216:
#line 2429 "src/Slice/Grammar.y"
{
}
#line 4693 "src/Slice/Grammar.cpp"
    break;

  case 217:
#line 2432 "src/Slice/Grammar.y"
{
}
#line 4700 "src/Slice/Grammar.cpp"
    break;

  case 218:
#line 2435 "src/Slice/Grammar.y"
{
}
#line 4707 "src/Slice/Grammar.cpp"
    break;

  case 219:
#line 2438 "src/Slice/Grammar.y"
{
}
#line 4714 "src/Slice/Grammar.cpp"
    break;

  case 220:
#line 2441 "src/Slice/Grammar.y"
{
}
#line 4721 "src/Slice/Grammar.cpp"
    break;

  case 221:
#line 2444 "src/Slice/Grammar.y"
{
}
#line 4728 "src/Slice/Grammar.cpp"
    break;

  case 222:
#line 2447 "src/Slice/Grammar.y"
{
}
#line 4735 "src/Slice/Grammar.cpp"
    break;

  case 223:
#line 2450 "src/Slice/Grammar.y"
{
}
#line 4742 "src/Slice/Grammar.cpp"
    break;

  case 224:
#line 2453 "src/Slice/Grammar.y"
{
}
#line 4749 "src/Slice/Grammar.cpp"
    break;

  case 225:
#line 2456 "src/Slice/Grammar.y"
{
}
#line 4756 "src/Slice/Grammar.cpp"
    break;

  case 226:
#line 2459 "src/Slice/Grammar.y"
{
}
#line 4763 "src/Slice/Grammar.cpp"
    break;

  case 227:
#line 2462 "src/Slice/Grammar.y"
{
}
#line 4770 "src/Slice/Grammar.cpp"
    break;

  case 228:
#line 2465 "src/Slice/Grammar.y"
{
}
#line 4777 "src/Slice/Grammar.cpp"
    break;

  case 229:
#line 2468 "src/Slice/Grammar.y"
{
}
#line 4784 "src/Slice/Grammar.cpp"
    break;

  case 230:
#line 2471 "src/Slice/Grammar.y"
{
}
#line 4791 "src/Slice/Grammar.cpp"
    break;

  case 231:
#line 2474 "src/Slice/Grammar.y"
{
}
#line 4798 "src/Slice/Grammar.cpp"
    break;

  case 232:
#line 2477 "src/Slice/Grammar.y"
{
}
#line 4805 "src/Slice/Grammar.cpp"
    break;

  case 233:
#line 2480 "src/Slice/Grammar.y"
{
}
#line 4812 "src/Slice/Grammar.cpp"
    break;

  case 234:
#line 2483 "src/Slice/Grammar.y"
{
}
#line 4819 "src/Slice/Grammar.cpp"
    break;

  case 235:
#line 2486 "src/Slice/Grammar.y"
{
}
#line 4826 "src/Slice/Grammar.cpp"
    break;

  case 236:
#line 2489 "src/Slice/Grammar.y"
{
}
#line 4833 "src/Slice/Grammar.cpp"
    break;

  case 237:
#line 2492 "src/Slice/Grammar.y"
{
}
#line 4840 "src/Slice/Grammar.cpp"
    break;

  case 238:
#line 2495 "src/Slice/Grammar.y"
{
}
#line 4847 "src/Slice/Grammar.cpp"
    break;

  case 239:
#line 2498 "src/Slice/Grammar.y"
{
}
#line 4854 "src/Slice/Grammar.cpp"
    break;

  case 240:
#line 2501 "src/Slice/Grammar.y"
{
}
#line 4861 "src/Slice/Grammar.cpp"
    break;

  case 241:
#line 2504 "src/Slice/Grammar.y"
{
}
#line 4868 "src/Slice/Grammar.cpp"
    break;

  case 242:
#line 2507 "src/Slice/Grammar.y"
{
}
#line 4875 "src/Slice/Grammar.cpp"
    break;


#line 4879 "src/Slice/Grammar.cpp"

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
#line 2511 "src/Slice/Grammar.y"

