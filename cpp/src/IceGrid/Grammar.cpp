/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */
#line 1 "src/IceGrid/Grammar.y"


// Copyright (c) ZeroC, Inc.

// NOLINTBEGIN


#line 76 "src/IceGrid/Grammar.cpp"



/* First part of user prologue.  */
#line 35 "src/IceGrid/Grammar.y"


#include "Ice/Ice.h"
#include "Parser.h"

#ifdef _MSC_VER
// warning C4127: conditional expression is constant
#    pragma warning(disable:4127)
// warning C4102: 'yyexhausted': unreferenced label
#    pragma warning(disable:4102)
// warning C4702: unreachable code
#    pragma warning(disable:4702)
#endif

// Avoid old style cast warnings in generated grammar
#ifdef __GNUC__
#    pragma GCC diagnostic ignored "-Wold-style-cast"
#    pragma GCC diagnostic ignored "-Wunused-label"

// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98753
#    pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif

// Avoid clang warnings in generate grammar
#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wconversion"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#    pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

void
yyerror(const char* s)
{
    parser->invalidCommand(s);
}


#line 123 "src/IceGrid/Grammar.cpp"

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

#include "Grammar.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ICEGRID_HELP = 3,               /* ICEGRID_HELP  */
  YYSYMBOL_ICEGRID_EXIT = 4,               /* ICEGRID_EXIT  */
  YYSYMBOL_ICEGRID_APPLICATION = 5,        /* ICEGRID_APPLICATION  */
  YYSYMBOL_ICEGRID_NODE = 6,               /* ICEGRID_NODE  */
  YYSYMBOL_ICEGRID_REGISTRY = 7,           /* ICEGRID_REGISTRY  */
  YYSYMBOL_ICEGRID_SERVER = 8,             /* ICEGRID_SERVER  */
  YYSYMBOL_ICEGRID_ADAPTER = 9,            /* ICEGRID_ADAPTER  */
  YYSYMBOL_ICEGRID_PING = 10,              /* ICEGRID_PING  */
  YYSYMBOL_ICEGRID_LOAD = 11,              /* ICEGRID_LOAD  */
  YYSYMBOL_ICEGRID_SOCKETS = 12,           /* ICEGRID_SOCKETS  */
  YYSYMBOL_ICEGRID_ADD = 13,               /* ICEGRID_ADD  */
  YYSYMBOL_ICEGRID_REMOVE = 14,            /* ICEGRID_REMOVE  */
  YYSYMBOL_ICEGRID_LIST = 15,              /* ICEGRID_LIST  */
  YYSYMBOL_ICEGRID_SHUTDOWN = 16,          /* ICEGRID_SHUTDOWN  */
  YYSYMBOL_ICEGRID_STRING = 17,            /* ICEGRID_STRING  */
  YYSYMBOL_ICEGRID_START = 18,             /* ICEGRID_START  */
  YYSYMBOL_ICEGRID_STOP = 19,              /* ICEGRID_STOP  */
  YYSYMBOL_ICEGRID_STATUS = 20,            /* ICEGRID_STATUS  */
  YYSYMBOL_ICEGRID_SIGNAL = 21,            /* ICEGRID_SIGNAL  */
  YYSYMBOL_ICEGRID_STDOUT = 22,            /* ICEGRID_STDOUT  */
  YYSYMBOL_ICEGRID_STDERR = 23,            /* ICEGRID_STDERR  */
  YYSYMBOL_ICEGRID_DESCRIBE = 24,          /* ICEGRID_DESCRIBE  */
  YYSYMBOL_ICEGRID_PROPERTIES = 25,        /* ICEGRID_PROPERTIES  */
  YYSYMBOL_ICEGRID_PROPERTY = 26,          /* ICEGRID_PROPERTY  */
  YYSYMBOL_ICEGRID_STATE = 27,             /* ICEGRID_STATE  */
  YYSYMBOL_ICEGRID_PID = 28,               /* ICEGRID_PID  */
  YYSYMBOL_ICEGRID_ENDPOINTS = 29,         /* ICEGRID_ENDPOINTS  */
  YYSYMBOL_ICEGRID_ACTIVATION = 30,        /* ICEGRID_ACTIVATION  */
  YYSYMBOL_ICEGRID_OBJECT = 31,            /* ICEGRID_OBJECT  */
  YYSYMBOL_ICEGRID_FIND = 32,              /* ICEGRID_FIND  */
  YYSYMBOL_ICEGRID_SHOW = 33,              /* ICEGRID_SHOW  */
  YYSYMBOL_ICEGRID_COPYING = 34,           /* ICEGRID_COPYING  */
  YYSYMBOL_ICEGRID_WARRANTY = 35,          /* ICEGRID_WARRANTY  */
  YYSYMBOL_ICEGRID_DIFF = 36,              /* ICEGRID_DIFF  */
  YYSYMBOL_ICEGRID_UPDATE = 37,            /* ICEGRID_UPDATE  */
  YYSYMBOL_ICEGRID_INSTANTIATE = 38,       /* ICEGRID_INSTANTIATE  */
  YYSYMBOL_ICEGRID_TEMPLATE = 39,          /* ICEGRID_TEMPLATE  */
  YYSYMBOL_ICEGRID_SERVICE = 40,           /* ICEGRID_SERVICE  */
  YYSYMBOL_ICEGRID_ENABLE = 41,            /* ICEGRID_ENABLE  */
  YYSYMBOL_ICEGRID_DISABLE = 42,           /* ICEGRID_DISABLE  */
  YYSYMBOL_43_ = 43,                       /* ';'  */
  YYSYMBOL_YYACCEPT = 44,                  /* $accept  */
  YYSYMBOL_start = 45,                     /* start  */
  YYSYMBOL_commands = 46,                  /* commands  */
  YYSYMBOL_checkInterrupted = 47,          /* checkInterrupted  */
  YYSYMBOL_command = 48,                   /* command  */
  YYSYMBOL_strings = 49,                   /* strings  */
  YYSYMBOL_keyword = 50                    /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 27 "src/IceGrid/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' in the generated scanner.
int yylex(YYSTYPE* yylvalp);


#line 216 "src/IceGrid/Grammar.cpp"

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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

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
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2569

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  44
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  7
/* YYNRULES -- Number of rules.  */
#define YYNRULES  170
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  362

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   297


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
static const yytype_int16 yyrline[] =
{
       0,   131,   131,   134,   142,   145,   153,   162,   166,   170,
     174,   178,   182,   186,   190,   194,   198,   202,   206,   210,
     214,   218,   222,   226,   230,   234,   238,   242,   246,   250,
     254,   258,   262,   266,   270,   274,   278,   282,   286,   290,
     294,   298,   302,   306,   310,   314,   318,   322,   326,   330,
     334,   338,   342,   346,   350,   354,   358,   362,   366,   370,
     374,   378,   382,   386,   390,   394,   398,   402,   406,   410,
     414,   418,   422,   426,   430,   434,   438,   442,   446,   450,
     454,   458,   462,   466,   470,   474,   478,   482,   486,   490,
     494,   498,   502,   506,   510,   514,   518,   522,   526,   530,
     534,   538,   542,   546,   550,   554,   558,   562,   566,   570,
     574,   578,   582,   586,   590,   594,   598,   609,   613,   624,
     635,   639,   643,   647,   653,   658,   663,   667,   675,   680,
     685,   691,   697,   706,   709,   712,   715,   718,   721,   724,
     727,   730,   733,   736,   739,   742,   745,   748,   751,   754,
     757,   760,   763,   766,   769,   772,   775,   778,   781,   784,
     787,   790,   793,   796,   799,   802,   805,   808,   811,   814,
     817
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ICEGRID_HELP",
  "ICEGRID_EXIT", "ICEGRID_APPLICATION", "ICEGRID_NODE",
  "ICEGRID_REGISTRY", "ICEGRID_SERVER", "ICEGRID_ADAPTER", "ICEGRID_PING",
  "ICEGRID_LOAD", "ICEGRID_SOCKETS", "ICEGRID_ADD", "ICEGRID_REMOVE",
  "ICEGRID_LIST", "ICEGRID_SHUTDOWN", "ICEGRID_STRING", "ICEGRID_START",
  "ICEGRID_STOP", "ICEGRID_STATUS", "ICEGRID_SIGNAL", "ICEGRID_STDOUT",
  "ICEGRID_STDERR", "ICEGRID_DESCRIBE", "ICEGRID_PROPERTIES",
  "ICEGRID_PROPERTY", "ICEGRID_STATE", "ICEGRID_PID", "ICEGRID_ENDPOINTS",
  "ICEGRID_ACTIVATION", "ICEGRID_OBJECT", "ICEGRID_FIND", "ICEGRID_SHOW",
  "ICEGRID_COPYING", "ICEGRID_WARRANTY", "ICEGRID_DIFF", "ICEGRID_UPDATE",
  "ICEGRID_INSTANTIATE", "ICEGRID_TEMPLATE", "ICEGRID_SERVICE",
  "ICEGRID_ENABLE", "ICEGRID_DISABLE", "';'", "$accept", "start",
  "commands", "checkInterrupted", "command", "strings", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-67)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-4)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      31,    53,    55,    60,   -67,    60,    19,   174,    62,   117,
    2447,   132,    96,   145,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   105,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,  2451,   -67,     9,   -67,   -67,
     -67,   -67,   -67,   -67,   125,   -67,   -67,   -67,   -67,   154,
     -67,   -67,    64,   -67,   -67,   -67,   -67,   -67,   -67,  2414,
     -67,   -67,   -67,   -67,   214,   -67,   255,   295,   335,   375,
     415,   455,   495,   535,   575,   615,   655,   695,   735,   775,
     815,   855,   895,   935,   975,  1015,  1055,  1095,  1135,  1175,
    1215,  1255,  1295,  1335,  1375,  1415,  1455,   101,  1495,  1535,
    1575,  1615,  1655,    65,  1695,  1735,  1775,  1815,  1855,    66,
      69,  1895,  1935,  1975,  2015,  2055,  2095,  2135,    34,    73,
      85,   133,   -67,  2175,    90,  2215,  2414,   -67,  2254,    93,
     103,   109,   115,   118,   119,   120,   124,   126,   129,  2417,
    2418,  2424,  2425,  2426,  2427,  2429,  2430,  2431,  2433,  2434,
    2435,  2436,  2438,  2439,  2441,  2442,  2443,  2444,  2445,  2446,
    2448,  2449,  2450,  2452,  2453,  2454,  2455,  2456,  2457,  2458,
    2459,  2460,  2461,  2462,  2463,  2464,  2465,  2466,  2467,  2468,
    2469,  2470,  2471,  2472,  2473,  2474,  2475,  2476,  2477,  2478,
    2479,  2480,  2295,  2335,  2481,  2482,  2483,  2484,  2485,  2486,
    2487,  2488,  2489,  2490,   -67,  2491,  2492,  2493,  2494,  2495,
    2496,  2497,  2498,  2499,  2500,   -67,   -67,  2501,  2502,  2503,
    2504,  2505,  2506,  2507,  2508,  2509,  2510,  2511,  2512,  2513,
    2514,  2515,  2375,   -67,   -67,  2516,  2414,   -67,   -67,  2414,
     -67,  2517,  2414,   -67,  2518,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,  2519,  2520,
    2521,  2522,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67,   -67,   -67,   -67,   -67,   -67,  2523,  2524,   -67,
     -67,   -67,   -67,  2525,   -67,   -67,   -67,   -67,   -67,   -67,
     -67,   -67
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,     6,     0,     1,     0,     0,     0,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,     0,   146,   147,   148,   149,   170,   169,   150,   153,
     154,   151,   152,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   127,     5,     0,
       4,   126,     0,   133,   134,   135,   136,   137,   138,   132,
     157,   159,   166,     7,     0,     8,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,     0,   132,   132,
     132,   132,   132,     0,   132,   132,   132,   132,   132,     0,
       0,   132,   132,   132,   132,   132,   132,   132,     0,     0,
       0,     0,   121,   132,     0,   132,   132,   115,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   132,   132,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   125,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   113,   114,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   132,   124,   122,     0,   132,   128,   120,   132,
     129,     0,   132,   116,     0,    10,     9,    12,    11,    20,
      19,    18,    17,    14,    13,    16,    15,    32,    31,    34,
      33,    36,    35,    40,    39,    38,    37,    30,    29,    42,
      41,    46,    45,    50,    49,    48,    47,    44,    43,    52,
      51,    54,    53,    80,    79,    58,    57,    60,    59,    62,
      61,    64,    63,    66,    65,    56,    55,    72,    71,    74,
      73,    68,    67,    70,    69,    82,    81,    25,     0,     0,
       0,     0,    76,    75,    78,    77,   100,    99,   102,   101,
      98,    97,   104,   103,   106,   105,   110,   109,   112,   111,
     108,   107,    96,    95,    84,    83,    86,    85,    88,    87,
      90,    89,    92,    91,    94,    93,    28,     0,     0,   123,
     130,   131,   117,     0,   118,    22,    21,    24,    23,    27,
      26,   119
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -67,   -67,   -67,   166,  2564,   -66,    49
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,     3,    48,   124,   125
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     130,   132,   134,   136,   138,   140,   142,   144,   146,   148,
     150,   152,   154,   156,   158,   160,   162,   164,   166,   168,
     170,   172,   174,   176,   178,   180,   182,   184,   186,   188,
     190,    -3,   195,   197,   199,   201,   203,   231,   206,   208,
     210,   212,   214,   109,   110,   218,   220,   222,   224,   226,
     228,   230,    49,     4,    49,    -2,    64,   237,   232,   240,
     241,     6,    51,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,   191,    65,   103,   122,   204,   215,
      84,    85,   216,   128,    86,    87,   233,    88,    89,    90,
      91,    92,    93,    94,    95,   192,   309,   311,   234,    96,
      66,    67,    68,   238,   235,    97,   245,    98,    99,   193,
     111,    69,    79,   112,   113,   114,   246,    80,    81,   115,
     116,   117,   247,    70,    71,   119,    82,   120,   248,   100,
     101,   249,   250,   251,   118,    83,   348,   252,     5,   253,
     350,   121,   254,   351,   102,    52,   353,   244,    53,    54,
      55,    56,    57,    58,    14,    15,    16,    17,    18,    19,
      20,    59,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    60,    36,    61,    38,    39,
      40,    41,    42,    43,    62,    45,    46,    63,    53,    54,
      55,    56,    57,    58,    14,    15,    16,    17,    18,    19,
      20,   126,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    60,    36,    61,    38,    39,
      40,    41,    42,    43,    62,    45,    46,   127,   129,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   131,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   133,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   135,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   137,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   139,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   141,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   143,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   145,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   147,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   149,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   151,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   153,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   155,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   157,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   159,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   161,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   163,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   165,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   167,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   169,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   171,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   173,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   175,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   177,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   179,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   181,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   183,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   185,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   187,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   189,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   194,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   196,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   198,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   200,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   202,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   205,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   207,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   209,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   211,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   213,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   217,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   219,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   221,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   223,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   225,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   227,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   229,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   236,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   239,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,    53,    54,
      55,    56,    57,    58,    14,    15,    16,    17,    18,    19,
      20,   242,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    60,    36,    61,    38,    39,
      40,    41,    42,    43,    62,    45,    46,   243,   308,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   310,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,   347,    53,
      54,    55,    56,    57,    58,    14,    15,    16,    17,    18,
      19,    20,   123,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    60,    36,    61,    38,
      39,    40,    41,    42,    43,    62,    45,    46,    53,    54,
      55,    56,    57,    58,    14,    15,    16,    17,    18,    19,
      20,   123,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    60,    36,    61,    38,    39,
      40,    41,    42,    43,    62,    45,    46,    72,    73,    74,
     255,   256,    75,    76,   104,   105,   106,   257,   258,   259,
     260,    77,   261,   262,   263,   107,   264,   265,   266,   267,
      78,   268,   269,   108,   270,   271,   272,   273,   274,   275,
       0,   276,   277,   278,     0,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   312,   313,   314,   315,   316,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   349,
     352,   354,   355,   356,   357,   358,   359,   360,   361,    50
};

static const yytype_int16 yycheck[] =
{
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,     0,    98,    99,   100,   101,   102,     3,   104,   105,
     106,   107,   108,    34,    35,   111,   112,   113,   114,   115,
     116,   117,     3,     0,     5,     0,     7,   123,    24,   125,
     126,     1,    43,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,     3,    43,     1,    43,    43,    43,
      14,    15,    43,    64,    18,    19,    43,    21,    22,    23,
      24,    25,    26,    27,    28,    24,   192,   193,    43,    33,
      13,    14,    15,    43,     1,    39,    43,    41,    42,    38,
      15,    24,    10,    18,    19,    20,    43,    15,    16,    24,
      25,    26,    43,    36,    37,     1,    24,     3,    43,    14,
      15,    43,    43,    43,    39,    33,   232,    43,     2,    43,
     236,    17,    43,   239,    29,     1,   242,   128,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    10,    11,    12,
      43,    43,    15,    16,    13,    14,    15,    43,    43,    43,
      43,    24,    43,    43,    43,    24,    43,    43,    43,    43,
      33,    43,    43,    32,    43,    43,    43,    43,    43,    43,
      -1,    43,    43,    43,    -1,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,     5
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    45,    46,    47,     0,    47,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    48,    50,
      48,    43,     1,     4,     5,     6,     7,     8,     9,    17,
      31,    33,    40,    43,    50,    43,    13,    14,    15,    24,
      36,    37,    10,    11,    12,    15,    16,    24,    33,    10,
      15,    16,    24,    33,    14,    15,    18,    19,    21,    22,
      23,    24,    25,    26,    27,    28,    33,    39,    41,    42,
      14,    15,    29,     1,    13,    14,    15,    24,    32,    34,
      35,    15,    18,    19,    20,    24,    25,    26,    39,     1,
       3,    17,    43,    17,    49,    50,    17,    43,    50,     3,
      49,     3,    49,     3,    49,     3,    49,     3,    49,     3,
      49,     3,    49,     3,    49,     3,    49,     3,    49,     3,
      49,     3,    49,     3,    49,     3,    49,     3,    49,     3,
      49,     3,    49,     3,    49,     3,    49,     3,    49,     3,
      49,     3,    49,     3,    49,     3,    49,     3,    49,     3,
      49,     3,    49,     3,    49,     3,    49,     3,    49,     3,
      49,     3,    24,    38,     3,    49,     3,    49,     3,    49,
       3,    49,     3,    49,    43,     3,    49,     3,    49,     3,
      49,     3,    49,     3,    49,    43,    43,     3,    49,     3,
      49,     3,    49,     3,    49,     3,    49,     3,    49,     3,
      49,     3,    24,    43,    43,     1,     3,    49,    43,     3,
      49,    49,    17,    43,    50,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,     3,    49,
       3,    49,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,    43,    43,    43,
      43,    43,    43,    43,    43,    43,    43,     3,    49,    43,
      49,    49,    43,    49,    43,    43,    43,    43,    43,    43,
      43,    43
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
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
      48,    48,    48,    48,    48,    48,    48,    48,    49,    49,
      49,    49,    49,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50,    50,    50,    50,    50,    50,    50,    50,    50,    50,
      50
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     3,     2,     0,     2,     2,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     5,     5,     5,     5,     4,     5,     5,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     3,     3,     3,     4,     5,     5,     6,
       4,     3,     3,     4,     3,     3,     2,     1,     2,     2,
       3,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
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
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

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
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  case 2: /* start: commands  */
#line 132 "src/IceGrid/Grammar.y"
{
}
#line 1849 "src/IceGrid/Grammar.cpp"
    break;

  case 3: /* start: %empty  */
#line 135 "src/IceGrid/Grammar.y"
{
}
#line 1856 "src/IceGrid/Grammar.cpp"
    break;

  case 4: /* commands: commands checkInterrupted command  */
#line 143 "src/IceGrid/Grammar.y"
{
}
#line 1863 "src/IceGrid/Grammar.cpp"
    break;

  case 5: /* commands: checkInterrupted command  */
#line 146 "src/IceGrid/Grammar.y"
{
}
#line 1870 "src/IceGrid/Grammar.cpp"
    break;

  case 6: /* checkInterrupted: %empty  */
#line 154 "src/IceGrid/Grammar.y"
{
    parser->checkInterrupted();
}
#line 1878 "src/IceGrid/Grammar.cpp"
    break;

  case 7: /* command: ICEGRID_HELP ';'  */
#line 163 "src/IceGrid/Grammar.y"
{
    parser->usage();
}
#line 1886 "src/IceGrid/Grammar.cpp"
    break;

  case 8: /* command: ICEGRID_EXIT ';'  */
#line 167 "src/IceGrid/Grammar.y"
{
    return 0;
}
#line 1894 "src/IceGrid/Grammar.cpp"
    break;

  case 9: /* command: ICEGRID_APPLICATION ICEGRID_ADD strings ';'  */
#line 171 "src/IceGrid/Grammar.y"
{
    parser->addApplication(yyvsp[-1]);
}
#line 1902 "src/IceGrid/Grammar.cpp"
    break;

  case 10: /* command: ICEGRID_APPLICATION ICEGRID_ADD ICEGRID_HELP ';'  */
#line 175 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "add");
}
#line 1910 "src/IceGrid/Grammar.cpp"
    break;

  case 11: /* command: ICEGRID_APPLICATION ICEGRID_REMOVE strings ';'  */
#line 179 "src/IceGrid/Grammar.y"
{
    parser->removeApplication(yyvsp[-1]);
}
#line 1918 "src/IceGrid/Grammar.cpp"
    break;

  case 12: /* command: ICEGRID_APPLICATION ICEGRID_REMOVE ICEGRID_HELP ';'  */
#line 183 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "remove");
}
#line 1926 "src/IceGrid/Grammar.cpp"
    break;

  case 13: /* command: ICEGRID_APPLICATION ICEGRID_DIFF strings ';'  */
#line 187 "src/IceGrid/Grammar.y"
{
    parser->diffApplication(yyvsp[-1]);
}
#line 1934 "src/IceGrid/Grammar.cpp"
    break;

  case 14: /* command: ICEGRID_APPLICATION ICEGRID_DIFF ICEGRID_HELP ';'  */
#line 191 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "diff");
}
#line 1942 "src/IceGrid/Grammar.cpp"
    break;

  case 15: /* command: ICEGRID_APPLICATION ICEGRID_UPDATE strings ';'  */
#line 195 "src/IceGrid/Grammar.y"
{
    parser->updateApplication(yyvsp[-1]);
}
#line 1950 "src/IceGrid/Grammar.cpp"
    break;

  case 16: /* command: ICEGRID_APPLICATION ICEGRID_UPDATE ICEGRID_HELP ';'  */
#line 199 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "update");
}
#line 1958 "src/IceGrid/Grammar.cpp"
    break;

  case 17: /* command: ICEGRID_APPLICATION ICEGRID_DESCRIBE strings ';'  */
#line 203 "src/IceGrid/Grammar.y"
{
    parser->describeApplication(yyvsp[-1]);
}
#line 1966 "src/IceGrid/Grammar.cpp"
    break;

  case 18: /* command: ICEGRID_APPLICATION ICEGRID_DESCRIBE ICEGRID_HELP ';'  */
#line 207 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "describe");
}
#line 1974 "src/IceGrid/Grammar.cpp"
    break;

  case 19: /* command: ICEGRID_APPLICATION ICEGRID_LIST strings ';'  */
#line 211 "src/IceGrid/Grammar.y"
{
    parser->listAllApplications(yyvsp[-1]);
}
#line 1982 "src/IceGrid/Grammar.cpp"
    break;

  case 20: /* command: ICEGRID_APPLICATION ICEGRID_LIST ICEGRID_HELP ';'  */
#line 215 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "list");
}
#line 1990 "src/IceGrid/Grammar.cpp"
    break;

  case 21: /* command: ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_DESCRIBE strings ';'  */
#line 219 "src/IceGrid/Grammar.y"
{
    parser->describeServerTemplate(yyvsp[-1]);
}
#line 1998 "src/IceGrid/Grammar.cpp"
    break;

  case 22: /* command: ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_DESCRIBE ICEGRID_HELP ';'  */
#line 223 "src/IceGrid/Grammar.y"
{
    parser->usage("server template", "describe");
}
#line 2006 "src/IceGrid/Grammar.cpp"
    break;

  case 23: /* command: ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_INSTANTIATE strings ';'  */
#line 227 "src/IceGrid/Grammar.y"
{
    parser->instantiateServerTemplate(yyvsp[-1]);
}
#line 2014 "src/IceGrid/Grammar.cpp"
    break;

  case 24: /* command: ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_INSTANTIATE ICEGRID_HELP ';'  */
#line 231 "src/IceGrid/Grammar.y"
{
    parser->usage("server template", "instantiate");
}
#line 2022 "src/IceGrid/Grammar.cpp"
    break;

  case 25: /* command: ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_HELP ';'  */
#line 235 "src/IceGrid/Grammar.y"
{
    parser->usage("server template");
}
#line 2030 "src/IceGrid/Grammar.cpp"
    break;

  case 26: /* command: ICEGRID_SERVICE ICEGRID_TEMPLATE ICEGRID_DESCRIBE strings ';'  */
#line 239 "src/IceGrid/Grammar.y"
{
    parser->describeServiceTemplate(yyvsp[-1]);
}
#line 2038 "src/IceGrid/Grammar.cpp"
    break;

  case 27: /* command: ICEGRID_SERVICE ICEGRID_TEMPLATE ICEGRID_DESCRIBE ICEGRID_HELP ';'  */
#line 243 "src/IceGrid/Grammar.y"
{
    parser->usage("service template", "describe");
}
#line 2046 "src/IceGrid/Grammar.cpp"
    break;

  case 28: /* command: ICEGRID_SERVICE ICEGRID_TEMPLATE ICEGRID_HELP ';'  */
#line 247 "src/IceGrid/Grammar.y"
{
    parser->usage("service template");
}
#line 2054 "src/IceGrid/Grammar.cpp"
    break;

  case 29: /* command: ICEGRID_NODE ICEGRID_DESCRIBE strings ';'  */
#line 251 "src/IceGrid/Grammar.y"
{
    parser->describeNode(yyvsp[-1]);
}
#line 2062 "src/IceGrid/Grammar.cpp"
    break;

  case 30: /* command: ICEGRID_NODE ICEGRID_DESCRIBE ICEGRID_HELP ';'  */
#line 255 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "describe");
}
#line 2070 "src/IceGrid/Grammar.cpp"
    break;

  case 31: /* command: ICEGRID_NODE ICEGRID_PING strings ';'  */
#line 259 "src/IceGrid/Grammar.y"
{
    parser->pingNode(yyvsp[-1]);
}
#line 2078 "src/IceGrid/Grammar.cpp"
    break;

  case 32: /* command: ICEGRID_NODE ICEGRID_PING ICEGRID_HELP ';'  */
#line 263 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "ping");
}
#line 2086 "src/IceGrid/Grammar.cpp"
    break;

  case 33: /* command: ICEGRID_NODE ICEGRID_LOAD strings ';'  */
#line 267 "src/IceGrid/Grammar.y"
{
    parser->printLoadNode(yyvsp[-1]);
}
#line 2094 "src/IceGrid/Grammar.cpp"
    break;

  case 34: /* command: ICEGRID_NODE ICEGRID_LOAD ICEGRID_HELP ';'  */
#line 271 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "load");
}
#line 2102 "src/IceGrid/Grammar.cpp"
    break;

  case 35: /* command: ICEGRID_NODE ICEGRID_SOCKETS strings ';'  */
#line 275 "src/IceGrid/Grammar.y"
{
    parser->printNodeProcessorSockets(yyvsp[-1]);
}
#line 2110 "src/IceGrid/Grammar.cpp"
    break;

  case 36: /* command: ICEGRID_NODE ICEGRID_SOCKETS ICEGRID_HELP ';'  */
#line 279 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "sockets");
}
#line 2118 "src/IceGrid/Grammar.cpp"
    break;

  case 37: /* command: ICEGRID_NODE ICEGRID_SHUTDOWN strings ';'  */
#line 283 "src/IceGrid/Grammar.y"
{
    parser->shutdownNode(yyvsp[-1]);
}
#line 2126 "src/IceGrid/Grammar.cpp"
    break;

  case 38: /* command: ICEGRID_NODE ICEGRID_SHUTDOWN ICEGRID_HELP ';'  */
#line 287 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "shutdown");
}
#line 2134 "src/IceGrid/Grammar.cpp"
    break;

  case 39: /* command: ICEGRID_NODE ICEGRID_LIST strings ';'  */
#line 291 "src/IceGrid/Grammar.y"
{
    parser->listAllNodes(yyvsp[-1]);
}
#line 2142 "src/IceGrid/Grammar.cpp"
    break;

  case 40: /* command: ICEGRID_NODE ICEGRID_LIST ICEGRID_HELP ';'  */
#line 295 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "list");
}
#line 2150 "src/IceGrid/Grammar.cpp"
    break;

  case 41: /* command: ICEGRID_NODE ICEGRID_SHOW strings ';'  */
#line 299 "src/IceGrid/Grammar.y"
{
    parser->show("node", yyvsp[-1]);
}
#line 2158 "src/IceGrid/Grammar.cpp"
    break;

  case 42: /* command: ICEGRID_NODE ICEGRID_SHOW ICEGRID_HELP ';'  */
#line 303 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "show");
}
#line 2166 "src/IceGrid/Grammar.cpp"
    break;

  case 43: /* command: ICEGRID_REGISTRY ICEGRID_DESCRIBE strings ';'  */
#line 307 "src/IceGrid/Grammar.y"
{
    parser->describeRegistry(yyvsp[-1]);
}
#line 2174 "src/IceGrid/Grammar.cpp"
    break;

  case 44: /* command: ICEGRID_REGISTRY ICEGRID_DESCRIBE ICEGRID_HELP ';'  */
#line 311 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "describe");
}
#line 2182 "src/IceGrid/Grammar.cpp"
    break;

  case 45: /* command: ICEGRID_REGISTRY ICEGRID_PING strings ';'  */
#line 315 "src/IceGrid/Grammar.y"
{
    parser->pingRegistry(yyvsp[-1]);
}
#line 2190 "src/IceGrid/Grammar.cpp"
    break;

  case 46: /* command: ICEGRID_REGISTRY ICEGRID_PING ICEGRID_HELP ';'  */
#line 319 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "ping");
}
#line 2198 "src/IceGrid/Grammar.cpp"
    break;

  case 47: /* command: ICEGRID_REGISTRY ICEGRID_SHUTDOWN strings ';'  */
#line 323 "src/IceGrid/Grammar.y"
{
    parser->shutdownRegistry(yyvsp[-1]);
}
#line 2206 "src/IceGrid/Grammar.cpp"
    break;

  case 48: /* command: ICEGRID_REGISTRY ICEGRID_SHUTDOWN ICEGRID_HELP ';'  */
#line 327 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "shutdown");
}
#line 2214 "src/IceGrid/Grammar.cpp"
    break;

  case 49: /* command: ICEGRID_REGISTRY ICEGRID_LIST strings ';'  */
#line 331 "src/IceGrid/Grammar.y"
{
    parser->listAllRegistries(yyvsp[-1]);
}
#line 2222 "src/IceGrid/Grammar.cpp"
    break;

  case 50: /* command: ICEGRID_REGISTRY ICEGRID_LIST ICEGRID_HELP ';'  */
#line 335 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "list");
}
#line 2230 "src/IceGrid/Grammar.cpp"
    break;

  case 51: /* command: ICEGRID_REGISTRY ICEGRID_SHOW strings ';'  */
#line 339 "src/IceGrid/Grammar.y"
{
    parser->show("registry", yyvsp[-1]);
}
#line 2238 "src/IceGrid/Grammar.cpp"
    break;

  case 52: /* command: ICEGRID_REGISTRY ICEGRID_SHOW ICEGRID_HELP ';'  */
#line 343 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "show");
}
#line 2246 "src/IceGrid/Grammar.cpp"
    break;

  case 53: /* command: ICEGRID_SERVER ICEGRID_REMOVE strings ';'  */
#line 347 "src/IceGrid/Grammar.y"
{
    parser->removeServer(yyvsp[-1]);
}
#line 2254 "src/IceGrid/Grammar.cpp"
    break;

  case 54: /* command: ICEGRID_SERVER ICEGRID_REMOVE ICEGRID_HELP ';'  */
#line 351 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "remove");
}
#line 2262 "src/IceGrid/Grammar.cpp"
    break;

  case 55: /* command: ICEGRID_SERVER ICEGRID_DESCRIBE strings ';'  */
#line 355 "src/IceGrid/Grammar.y"
{
    parser->describeServer(yyvsp[-1]);
}
#line 2270 "src/IceGrid/Grammar.cpp"
    break;

  case 56: /* command: ICEGRID_SERVER ICEGRID_DESCRIBE ICEGRID_HELP ';'  */
#line 359 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "describe");
}
#line 2278 "src/IceGrid/Grammar.cpp"
    break;

  case 57: /* command: ICEGRID_SERVER ICEGRID_START strings ';'  */
#line 363 "src/IceGrid/Grammar.y"
{
    parser->startServer(yyvsp[-1]);
}
#line 2286 "src/IceGrid/Grammar.cpp"
    break;

  case 58: /* command: ICEGRID_SERVER ICEGRID_START ICEGRID_HELP ';'  */
#line 367 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "start");
}
#line 2294 "src/IceGrid/Grammar.cpp"
    break;

  case 59: /* command: ICEGRID_SERVER ICEGRID_STOP strings ';'  */
#line 371 "src/IceGrid/Grammar.y"
{
    parser->stopServer(yyvsp[-1]);
}
#line 2302 "src/IceGrid/Grammar.cpp"
    break;

  case 60: /* command: ICEGRID_SERVER ICEGRID_STOP ICEGRID_HELP ';'  */
#line 375 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "stop");
}
#line 2310 "src/IceGrid/Grammar.cpp"
    break;

  case 61: /* command: ICEGRID_SERVER ICEGRID_SIGNAL strings ';'  */
#line 379 "src/IceGrid/Grammar.y"
{
    parser->signalServer(yyvsp[-1]);
}
#line 2318 "src/IceGrid/Grammar.cpp"
    break;

  case 62: /* command: ICEGRID_SERVER ICEGRID_SIGNAL ICEGRID_HELP ';'  */
#line 383 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "signal");
}
#line 2326 "src/IceGrid/Grammar.cpp"
    break;

  case 63: /* command: ICEGRID_SERVER ICEGRID_STDOUT strings ';'  */
#line 387 "src/IceGrid/Grammar.y"
{
    parser->writeMessage(yyvsp[-1], 1);
}
#line 2334 "src/IceGrid/Grammar.cpp"
    break;

  case 64: /* command: ICEGRID_SERVER ICEGRID_STDOUT ICEGRID_HELP ';'  */
#line 391 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "stdout");
}
#line 2342 "src/IceGrid/Grammar.cpp"
    break;

  case 65: /* command: ICEGRID_SERVER ICEGRID_STDERR strings ';'  */
#line 395 "src/IceGrid/Grammar.y"
{
    parser->writeMessage(yyvsp[-1], 2);
}
#line 2350 "src/IceGrid/Grammar.cpp"
    break;

  case 66: /* command: ICEGRID_SERVER ICEGRID_STDERR ICEGRID_HELP ';'  */
#line 399 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "stderr");
}
#line 2358 "src/IceGrid/Grammar.cpp"
    break;

  case 67: /* command: ICEGRID_SERVER ICEGRID_STATE strings ';'  */
#line 403 "src/IceGrid/Grammar.y"
{
    parser->stateServer(yyvsp[-1]);
}
#line 2366 "src/IceGrid/Grammar.cpp"
    break;

  case 68: /* command: ICEGRID_SERVER ICEGRID_STATE ICEGRID_HELP ';'  */
#line 407 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "start");
}
#line 2374 "src/IceGrid/Grammar.cpp"
    break;

  case 69: /* command: ICEGRID_SERVER ICEGRID_PID strings ';'  */
#line 411 "src/IceGrid/Grammar.y"
{
    parser->pidServer(yyvsp[-1]);
}
#line 2382 "src/IceGrid/Grammar.cpp"
    break;

  case 70: /* command: ICEGRID_SERVER ICEGRID_PID ICEGRID_HELP ';'  */
#line 415 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "pid");
}
#line 2390 "src/IceGrid/Grammar.cpp"
    break;

  case 71: /* command: ICEGRID_SERVER ICEGRID_PROPERTIES strings ';'  */
#line 419 "src/IceGrid/Grammar.y"
{
    parser->propertiesServer(yyvsp[-1], false);
}
#line 2398 "src/IceGrid/Grammar.cpp"
    break;

  case 72: /* command: ICEGRID_SERVER ICEGRID_PROPERTIES ICEGRID_HELP ';'  */
#line 423 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "properties");
}
#line 2406 "src/IceGrid/Grammar.cpp"
    break;

  case 73: /* command: ICEGRID_SERVER ICEGRID_PROPERTY strings ';'  */
#line 427 "src/IceGrid/Grammar.y"
{
    parser->propertiesServer(yyvsp[-1], true);
}
#line 2414 "src/IceGrid/Grammar.cpp"
    break;

  case 74: /* command: ICEGRID_SERVER ICEGRID_PROPERTY ICEGRID_HELP ';'  */
#line 431 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "property");
}
#line 2422 "src/IceGrid/Grammar.cpp"
    break;

  case 75: /* command: ICEGRID_SERVER ICEGRID_ENABLE strings ';'  */
#line 435 "src/IceGrid/Grammar.y"
{
    parser->enableServer(yyvsp[-1], true);
}
#line 2430 "src/IceGrid/Grammar.cpp"
    break;

  case 76: /* command: ICEGRID_SERVER ICEGRID_ENABLE ICEGRID_HELP ';'  */
#line 439 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "enable");
}
#line 2438 "src/IceGrid/Grammar.cpp"
    break;

  case 77: /* command: ICEGRID_SERVER ICEGRID_DISABLE strings ';'  */
#line 443 "src/IceGrid/Grammar.y"
{
    parser->enableServer(yyvsp[-1], false);
}
#line 2446 "src/IceGrid/Grammar.cpp"
    break;

  case 78: /* command: ICEGRID_SERVER ICEGRID_DISABLE ICEGRID_HELP ';'  */
#line 447 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "disable");
}
#line 2454 "src/IceGrid/Grammar.cpp"
    break;

  case 79: /* command: ICEGRID_SERVER ICEGRID_LIST strings ';'  */
#line 451 "src/IceGrid/Grammar.y"
{
    parser->listAllServers(yyvsp[-1]);
}
#line 2462 "src/IceGrid/Grammar.cpp"
    break;

  case 80: /* command: ICEGRID_SERVER ICEGRID_LIST ICEGRID_HELP ';'  */
#line 455 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "list");
}
#line 2470 "src/IceGrid/Grammar.cpp"
    break;

  case 81: /* command: ICEGRID_SERVER ICEGRID_SHOW strings ';'  */
#line 459 "src/IceGrid/Grammar.y"
{
    parser->show("server", yyvsp[-1]);
}
#line 2478 "src/IceGrid/Grammar.cpp"
    break;

  case 82: /* command: ICEGRID_SERVER ICEGRID_SHOW ICEGRID_HELP ';'  */
#line 463 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "show");
}
#line 2486 "src/IceGrid/Grammar.cpp"
    break;

  case 83: /* command: ICEGRID_SERVICE ICEGRID_START strings ';'  */
#line 467 "src/IceGrid/Grammar.y"
{
    parser->startService(yyvsp[-1]);
}
#line 2494 "src/IceGrid/Grammar.cpp"
    break;

  case 84: /* command: ICEGRID_SERVICE ICEGRID_START ICEGRID_HELP ';'  */
#line 471 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "start");
}
#line 2502 "src/IceGrid/Grammar.cpp"
    break;

  case 85: /* command: ICEGRID_SERVICE ICEGRID_STOP strings ';'  */
#line 475 "src/IceGrid/Grammar.y"
{
    parser->stopService(yyvsp[-1]);
}
#line 2510 "src/IceGrid/Grammar.cpp"
    break;

  case 86: /* command: ICEGRID_SERVICE ICEGRID_STOP ICEGRID_HELP ';'  */
#line 479 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "stop");
}
#line 2518 "src/IceGrid/Grammar.cpp"
    break;

  case 87: /* command: ICEGRID_SERVICE ICEGRID_STATUS strings ';'  */
#line 483 "src/IceGrid/Grammar.y"
{
    parser->serviceStatus(yyvsp[-1]);
}
#line 2526 "src/IceGrid/Grammar.cpp"
    break;

  case 88: /* command: ICEGRID_SERVICE ICEGRID_STATUS ICEGRID_HELP ';'  */
#line 487 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "status");
}
#line 2534 "src/IceGrid/Grammar.cpp"
    break;

  case 89: /* command: ICEGRID_SERVICE ICEGRID_DESCRIBE strings ';'  */
#line 491 "src/IceGrid/Grammar.y"
{
    parser->describeService(yyvsp[-1]);
}
#line 2542 "src/IceGrid/Grammar.cpp"
    break;

  case 90: /* command: ICEGRID_SERVICE ICEGRID_DESCRIBE ICEGRID_HELP ';'  */
#line 495 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "describe");
}
#line 2550 "src/IceGrid/Grammar.cpp"
    break;

  case 91: /* command: ICEGRID_SERVICE ICEGRID_PROPERTIES strings ';'  */
#line 499 "src/IceGrid/Grammar.y"
{
    parser->propertiesService(yyvsp[-1], false);
}
#line 2558 "src/IceGrid/Grammar.cpp"
    break;

  case 92: /* command: ICEGRID_SERVICE ICEGRID_PROPERTIES ICEGRID_HELP ';'  */
#line 503 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "properties");
}
#line 2566 "src/IceGrid/Grammar.cpp"
    break;

  case 93: /* command: ICEGRID_SERVICE ICEGRID_PROPERTY strings ';'  */
#line 507 "src/IceGrid/Grammar.y"
{
    parser->propertiesService(yyvsp[-1], true);
}
#line 2574 "src/IceGrid/Grammar.cpp"
    break;

  case 94: /* command: ICEGRID_SERVICE ICEGRID_PROPERTY ICEGRID_HELP ';'  */
#line 511 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "property");
}
#line 2582 "src/IceGrid/Grammar.cpp"
    break;

  case 95: /* command: ICEGRID_SERVICE ICEGRID_LIST strings ';'  */
#line 515 "src/IceGrid/Grammar.y"
{
    parser->listServices(yyvsp[-1]);
}
#line 2590 "src/IceGrid/Grammar.cpp"
    break;

  case 96: /* command: ICEGRID_SERVICE ICEGRID_LIST ICEGRID_HELP ';'  */
#line 519 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "list");
}
#line 2598 "src/IceGrid/Grammar.cpp"
    break;

  case 97: /* command: ICEGRID_ADAPTER ICEGRID_ENDPOINTS strings ';'  */
#line 523 "src/IceGrid/Grammar.y"
{
    parser->endpointsAdapter(yyvsp[-1]);
}
#line 2606 "src/IceGrid/Grammar.cpp"
    break;

  case 98: /* command: ICEGRID_ADAPTER ICEGRID_ENDPOINTS ICEGRID_HELP ';'  */
#line 527 "src/IceGrid/Grammar.y"
{
    parser->usage("adapter", "endpoints");
}
#line 2614 "src/IceGrid/Grammar.cpp"
    break;

  case 99: /* command: ICEGRID_ADAPTER ICEGRID_REMOVE strings ';'  */
#line 531 "src/IceGrid/Grammar.y"
{
    parser->removeAdapter(yyvsp[-1]);
}
#line 2622 "src/IceGrid/Grammar.cpp"
    break;

  case 100: /* command: ICEGRID_ADAPTER ICEGRID_REMOVE ICEGRID_HELP ';'  */
#line 535 "src/IceGrid/Grammar.y"
{
    parser->usage("adapter", "remove");
}
#line 2630 "src/IceGrid/Grammar.cpp"
    break;

  case 101: /* command: ICEGRID_ADAPTER ICEGRID_LIST strings ';'  */
#line 539 "src/IceGrid/Grammar.y"
{
    parser->listAllAdapters(yyvsp[-1]);
}
#line 2638 "src/IceGrid/Grammar.cpp"
    break;

  case 102: /* command: ICEGRID_ADAPTER ICEGRID_LIST ICEGRID_HELP ';'  */
#line 543 "src/IceGrid/Grammar.y"
{
    parser->usage("adapter", "list");
}
#line 2646 "src/IceGrid/Grammar.cpp"
    break;

  case 103: /* command: ICEGRID_OBJECT ICEGRID_ADD strings ';'  */
#line 547 "src/IceGrid/Grammar.y"
{
    parser->addObject(yyvsp[-1]);
}
#line 2654 "src/IceGrid/Grammar.cpp"
    break;

  case 104: /* command: ICEGRID_OBJECT ICEGRID_ADD ICEGRID_HELP ';'  */
#line 551 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "add");
}
#line 2662 "src/IceGrid/Grammar.cpp"
    break;

  case 105: /* command: ICEGRID_OBJECT ICEGRID_REMOVE strings ';'  */
#line 555 "src/IceGrid/Grammar.y"
{
    parser->removeObject(yyvsp[-1]);
}
#line 2670 "src/IceGrid/Grammar.cpp"
    break;

  case 106: /* command: ICEGRID_OBJECT ICEGRID_REMOVE ICEGRID_HELP ';'  */
#line 559 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "remove");
}
#line 2678 "src/IceGrid/Grammar.cpp"
    break;

  case 107: /* command: ICEGRID_OBJECT ICEGRID_FIND strings ';'  */
#line 563 "src/IceGrid/Grammar.y"
{
    parser->findObject(yyvsp[-1]);
}
#line 2686 "src/IceGrid/Grammar.cpp"
    break;

  case 108: /* command: ICEGRID_OBJECT ICEGRID_FIND ICEGRID_HELP ';'  */
#line 567 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "find");
}
#line 2694 "src/IceGrid/Grammar.cpp"
    break;

  case 109: /* command: ICEGRID_OBJECT ICEGRID_LIST strings ';'  */
#line 571 "src/IceGrid/Grammar.y"
{
    parser->listObject(yyvsp[-1]);
}
#line 2702 "src/IceGrid/Grammar.cpp"
    break;

  case 110: /* command: ICEGRID_OBJECT ICEGRID_LIST ICEGRID_HELP ';'  */
#line 575 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "list");
}
#line 2710 "src/IceGrid/Grammar.cpp"
    break;

  case 111: /* command: ICEGRID_OBJECT ICEGRID_DESCRIBE strings ';'  */
#line 579 "src/IceGrid/Grammar.y"
{
    parser->describeObject(yyvsp[-1]);
}
#line 2718 "src/IceGrid/Grammar.cpp"
    break;

  case 112: /* command: ICEGRID_OBJECT ICEGRID_DESCRIBE ICEGRID_HELP ';'  */
#line 583 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "describe");
}
#line 2726 "src/IceGrid/Grammar.cpp"
    break;

  case 113: /* command: ICEGRID_SHOW ICEGRID_COPYING ';'  */
#line 587 "src/IceGrid/Grammar.y"
{
    parser->showCopying();
}
#line 2734 "src/IceGrid/Grammar.cpp"
    break;

  case 114: /* command: ICEGRID_SHOW ICEGRID_WARRANTY ';'  */
#line 591 "src/IceGrid/Grammar.y"
{
    parser->showWarranty();
}
#line 2742 "src/IceGrid/Grammar.cpp"
    break;

  case 115: /* command: ICEGRID_HELP keyword ';'  */
#line 595 "src/IceGrid/Grammar.y"
{
    parser->usage(yyvsp[-1].front());
}
#line 2750 "src/IceGrid/Grammar.cpp"
    break;

  case 116: /* command: ICEGRID_HELP keyword keyword ';'  */
#line 599 "src/IceGrid/Grammar.y"
{
    if ((yyvsp[-2].front() == "server" || yyvsp[-2].front() == "service") && yyvsp[-1].front() == "template")
    {
        parser->usage(yyvsp[-2].front() + " " + yyvsp[-1].front());
    }
    else
    {
        parser->usage(yyvsp[-2].front(), yyvsp[-1].front());
    }
}
#line 2765 "src/IceGrid/Grammar.cpp"
    break;

  case 117: /* command: ICEGRID_HELP keyword ICEGRID_STRING strings ';'  */
#line 610 "src/IceGrid/Grammar.y"
{
    parser->usage(yyvsp[-3].front(), yyvsp[-2].front());
}
#line 2773 "src/IceGrid/Grammar.cpp"
    break;

  case 118: /* command: ICEGRID_HELP keyword keyword keyword ';'  */
#line 614 "src/IceGrid/Grammar.y"
{
    if ((yyvsp[-3].front() == "server" || yyvsp[-3].front() == "service") && yyvsp[-2].front() == "template")
    {
        parser->usage(yyvsp[-3].front() + " " + yyvsp[-2].front(), yyvsp[-1].front());
    }
    else
    {
        parser->usage(yyvsp[-3].front(), yyvsp[-2].front());
    }
}
#line 2788 "src/IceGrid/Grammar.cpp"
    break;

  case 119: /* command: ICEGRID_HELP keyword keyword ICEGRID_STRING strings ';'  */
#line 625 "src/IceGrid/Grammar.y"
{
    if ((yyvsp[-4].front() == "server" || yyvsp[-4].front() == "service") && yyvsp[-3].front() == "template")
    {
        parser->usage(yyvsp[-4].front() + " " + yyvsp[-3].front(), yyvsp[-2].front());
    }
    else
    {
        parser->usage(yyvsp[-4].front(), yyvsp[-3].front());
    }
}
#line 2803 "src/IceGrid/Grammar.cpp"
    break;

  case 120: /* command: ICEGRID_HELP ICEGRID_STRING strings ';'  */
#line 636 "src/IceGrid/Grammar.y"
{
    parser->usage(yyvsp[-2].front());
}
#line 2811 "src/IceGrid/Grammar.cpp"
    break;

  case 121: /* command: ICEGRID_HELP error ';'  */
#line 640 "src/IceGrid/Grammar.y"
{
    parser->usage();
}
#line 2819 "src/IceGrid/Grammar.cpp"
    break;

  case 122: /* command: keyword ICEGRID_HELP ';'  */
#line 644 "src/IceGrid/Grammar.y"
{
    parser->usage(yyvsp[-2].front());
}
#line 2827 "src/IceGrid/Grammar.cpp"
    break;

  case 123: /* command: keyword ICEGRID_STRING error ';'  */
#line 648 "src/IceGrid/Grammar.y"
{
    yyvsp[-3].push_back(yyvsp[-2].front());
    parser->invalidCommand(yyvsp[-3]);
    yyerrok;
}
#line 2837 "src/IceGrid/Grammar.cpp"
    break;

  case 124: /* command: keyword error ';'  */
#line 654 "src/IceGrid/Grammar.y"
{
    parser->invalidCommand(yyvsp[-2]);
    yyerrok;
}
#line 2846 "src/IceGrid/Grammar.cpp"
    break;

  case 125: /* command: ICEGRID_STRING error ';'  */
#line 659 "src/IceGrid/Grammar.y"
{
    parser->invalidCommand(yyvsp[-2]);
    yyerrok;
}
#line 2855 "src/IceGrid/Grammar.cpp"
    break;

  case 126: /* command: error ';'  */
#line 664 "src/IceGrid/Grammar.y"
{
    yyerrok;
}
#line 2863 "src/IceGrid/Grammar.cpp"
    break;

  case 127: /* command: ';'  */
#line 668 "src/IceGrid/Grammar.y"
{
}
#line 2870 "src/IceGrid/Grammar.cpp"
    break;

  case 128: /* strings: ICEGRID_STRING strings  */
#line 676 "src/IceGrid/Grammar.y"
{
    yyval = yyvsp[0];
    yyval.push_front(yyvsp[-1].front());
}
#line 2879 "src/IceGrid/Grammar.cpp"
    break;

  case 129: /* strings: keyword strings  */
#line 681 "src/IceGrid/Grammar.y"
{
    yyval = yyvsp[0];
    yyval.push_front(yyvsp[-1].front());
}
#line 2888 "src/IceGrid/Grammar.cpp"
    break;

  case 130: /* strings: ICEGRID_STRING ICEGRID_HELP strings  */
#line 686 "src/IceGrid/Grammar.y"
{
    yyval = yyvsp[-1];
    yyval.push_front("help");
    yyval.push_front(yyvsp[-2].front());
}
#line 2898 "src/IceGrid/Grammar.cpp"
    break;

  case 131: /* strings: keyword ICEGRID_HELP strings  */
#line 692 "src/IceGrid/Grammar.y"
{
    yyval = yyvsp[-1];
    yyval.push_front("help");
    yyval.push_front(yyvsp[-2].front());
}
#line 2908 "src/IceGrid/Grammar.cpp"
    break;

  case 132: /* strings: %empty  */
#line 698 "src/IceGrid/Grammar.y"
{
    yyval = YYSTYPE();
}
#line 2916 "src/IceGrid/Grammar.cpp"
    break;

  case 133: /* keyword: ICEGRID_EXIT  */
#line 707 "src/IceGrid/Grammar.y"
{
}
#line 2923 "src/IceGrid/Grammar.cpp"
    break;

  case 134: /* keyword: ICEGRID_APPLICATION  */
#line 710 "src/IceGrid/Grammar.y"
{
}
#line 2930 "src/IceGrid/Grammar.cpp"
    break;

  case 135: /* keyword: ICEGRID_NODE  */
#line 713 "src/IceGrid/Grammar.y"
{
}
#line 2937 "src/IceGrid/Grammar.cpp"
    break;

  case 136: /* keyword: ICEGRID_REGISTRY  */
#line 716 "src/IceGrid/Grammar.y"
{
}
#line 2944 "src/IceGrid/Grammar.cpp"
    break;

  case 137: /* keyword: ICEGRID_SERVER  */
#line 719 "src/IceGrid/Grammar.y"
{
}
#line 2951 "src/IceGrid/Grammar.cpp"
    break;

  case 138: /* keyword: ICEGRID_ADAPTER  */
#line 722 "src/IceGrid/Grammar.y"
{
}
#line 2958 "src/IceGrid/Grammar.cpp"
    break;

  case 139: /* keyword: ICEGRID_PING  */
#line 725 "src/IceGrid/Grammar.y"
{
}
#line 2965 "src/IceGrid/Grammar.cpp"
    break;

  case 140: /* keyword: ICEGRID_LOAD  */
#line 728 "src/IceGrid/Grammar.y"
{
}
#line 2972 "src/IceGrid/Grammar.cpp"
    break;

  case 141: /* keyword: ICEGRID_SOCKETS  */
#line 731 "src/IceGrid/Grammar.y"
{
}
#line 2979 "src/IceGrid/Grammar.cpp"
    break;

  case 142: /* keyword: ICEGRID_ADD  */
#line 734 "src/IceGrid/Grammar.y"
{
}
#line 2986 "src/IceGrid/Grammar.cpp"
    break;

  case 143: /* keyword: ICEGRID_REMOVE  */
#line 737 "src/IceGrid/Grammar.y"
{
}
#line 2993 "src/IceGrid/Grammar.cpp"
    break;

  case 144: /* keyword: ICEGRID_LIST  */
#line 740 "src/IceGrid/Grammar.y"
{
}
#line 3000 "src/IceGrid/Grammar.cpp"
    break;

  case 145: /* keyword: ICEGRID_SHUTDOWN  */
#line 743 "src/IceGrid/Grammar.y"
{
}
#line 3007 "src/IceGrid/Grammar.cpp"
    break;

  case 146: /* keyword: ICEGRID_START  */
#line 746 "src/IceGrid/Grammar.y"
{
}
#line 3014 "src/IceGrid/Grammar.cpp"
    break;

  case 147: /* keyword: ICEGRID_STOP  */
#line 749 "src/IceGrid/Grammar.y"
{
}
#line 3021 "src/IceGrid/Grammar.cpp"
    break;

  case 148: /* keyword: ICEGRID_STATUS  */
#line 752 "src/IceGrid/Grammar.y"
{
}
#line 3028 "src/IceGrid/Grammar.cpp"
    break;

  case 149: /* keyword: ICEGRID_SIGNAL  */
#line 755 "src/IceGrid/Grammar.y"
{
}
#line 3035 "src/IceGrid/Grammar.cpp"
    break;

  case 150: /* keyword: ICEGRID_DESCRIBE  */
#line 758 "src/IceGrid/Grammar.y"
{
}
#line 3042 "src/IceGrid/Grammar.cpp"
    break;

  case 151: /* keyword: ICEGRID_STATE  */
#line 761 "src/IceGrid/Grammar.y"
{
}
#line 3049 "src/IceGrid/Grammar.cpp"
    break;

  case 152: /* keyword: ICEGRID_PID  */
#line 764 "src/IceGrid/Grammar.y"
{
}
#line 3056 "src/IceGrid/Grammar.cpp"
    break;

  case 153: /* keyword: ICEGRID_PROPERTIES  */
#line 767 "src/IceGrid/Grammar.y"
{
}
#line 3063 "src/IceGrid/Grammar.cpp"
    break;

  case 154: /* keyword: ICEGRID_PROPERTY  */
#line 770 "src/IceGrid/Grammar.y"
{
}
#line 3070 "src/IceGrid/Grammar.cpp"
    break;

  case 155: /* keyword: ICEGRID_ENDPOINTS  */
#line 773 "src/IceGrid/Grammar.y"
{
}
#line 3077 "src/IceGrid/Grammar.cpp"
    break;

  case 156: /* keyword: ICEGRID_ACTIVATION  */
#line 776 "src/IceGrid/Grammar.y"
{
}
#line 3084 "src/IceGrid/Grammar.cpp"
    break;

  case 157: /* keyword: ICEGRID_OBJECT  */
#line 779 "src/IceGrid/Grammar.y"
{
}
#line 3091 "src/IceGrid/Grammar.cpp"
    break;

  case 158: /* keyword: ICEGRID_FIND  */
#line 782 "src/IceGrid/Grammar.y"
{
}
#line 3098 "src/IceGrid/Grammar.cpp"
    break;

  case 159: /* keyword: ICEGRID_SHOW  */
#line 785 "src/IceGrid/Grammar.y"
{
}
#line 3105 "src/IceGrid/Grammar.cpp"
    break;

  case 160: /* keyword: ICEGRID_COPYING  */
#line 788 "src/IceGrid/Grammar.y"
{
}
#line 3112 "src/IceGrid/Grammar.cpp"
    break;

  case 161: /* keyword: ICEGRID_WARRANTY  */
#line 791 "src/IceGrid/Grammar.y"
{
}
#line 3119 "src/IceGrid/Grammar.cpp"
    break;

  case 162: /* keyword: ICEGRID_DIFF  */
#line 794 "src/IceGrid/Grammar.y"
{
}
#line 3126 "src/IceGrid/Grammar.cpp"
    break;

  case 163: /* keyword: ICEGRID_UPDATE  */
#line 797 "src/IceGrid/Grammar.y"
{
}
#line 3133 "src/IceGrid/Grammar.cpp"
    break;

  case 164: /* keyword: ICEGRID_INSTANTIATE  */
#line 800 "src/IceGrid/Grammar.y"
{
}
#line 3140 "src/IceGrid/Grammar.cpp"
    break;

  case 165: /* keyword: ICEGRID_TEMPLATE  */
#line 803 "src/IceGrid/Grammar.y"
{
}
#line 3147 "src/IceGrid/Grammar.cpp"
    break;

  case 166: /* keyword: ICEGRID_SERVICE  */
#line 806 "src/IceGrid/Grammar.y"
{
}
#line 3154 "src/IceGrid/Grammar.cpp"
    break;

  case 167: /* keyword: ICEGRID_ENABLE  */
#line 809 "src/IceGrid/Grammar.y"
{
}
#line 3161 "src/IceGrid/Grammar.cpp"
    break;

  case 168: /* keyword: ICEGRID_DISABLE  */
#line 812 "src/IceGrid/Grammar.y"
{
}
#line 3168 "src/IceGrid/Grammar.cpp"
    break;

  case 169: /* keyword: ICEGRID_STDERR  */
#line 815 "src/IceGrid/Grammar.y"
{
}
#line 3175 "src/IceGrid/Grammar.cpp"
    break;

  case 170: /* keyword: ICEGRID_STDOUT  */
#line 818 "src/IceGrid/Grammar.y"
{
}
#line 3182 "src/IceGrid/Grammar.cpp"
    break;


#line 3186 "src/IceGrid/Grammar.cpp"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 822 "src/IceGrid/Grammar.y"


// NOLINTEND
