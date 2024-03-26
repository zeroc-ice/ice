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




/* First part of user prologue.  */
#line 1 "src/IceGrid/Grammar.y"


//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/Ice.h"
#include "Parser.h"

#ifdef _MSC_VER
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning(disable:4102)
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning(disable:4065)
// warning C4244: '=': conversion from 'int' to 'yytype_int16', possible loss of data
#   pragma warning(disable:4244)
// warning C4702: unreachable code
#   pragma warning(disable:4702)
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
#   pragma clang diagnostic ignored "-Wunused-but-set-variable"
#   pragma clang diagnostic ignored "-Wunused-label"
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

void
yyerror(const char* s)
{
    parser->invalidCommand(s);
}


#line 120 "src/IceGrid/Grammar.cpp"

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
  YYSYMBOL_ICE_GRID_HELP = 3,              /* ICE_GRID_HELP  */
  YYSYMBOL_ICE_GRID_EXIT = 4,              /* ICE_GRID_EXIT  */
  YYSYMBOL_ICE_GRID_APPLICATION = 5,       /* ICE_GRID_APPLICATION  */
  YYSYMBOL_ICE_GRID_NODE = 6,              /* ICE_GRID_NODE  */
  YYSYMBOL_ICE_GRID_REGISTRY = 7,          /* ICE_GRID_REGISTRY  */
  YYSYMBOL_ICE_GRID_SERVER = 8,            /* ICE_GRID_SERVER  */
  YYSYMBOL_ICE_GRID_ADAPTER = 9,           /* ICE_GRID_ADAPTER  */
  YYSYMBOL_ICE_GRID_PING = 10,             /* ICE_GRID_PING  */
  YYSYMBOL_ICE_GRID_LOAD = 11,             /* ICE_GRID_LOAD  */
  YYSYMBOL_ICE_GRID_SOCKETS = 12,          /* ICE_GRID_SOCKETS  */
  YYSYMBOL_ICE_GRID_ADD = 13,              /* ICE_GRID_ADD  */
  YYSYMBOL_ICE_GRID_REMOVE = 14,           /* ICE_GRID_REMOVE  */
  YYSYMBOL_ICE_GRID_LIST = 15,             /* ICE_GRID_LIST  */
  YYSYMBOL_ICE_GRID_SHUTDOWN = 16,         /* ICE_GRID_SHUTDOWN  */
  YYSYMBOL_ICE_GRID_STRING = 17,           /* ICE_GRID_STRING  */
  YYSYMBOL_ICE_GRID_START = 18,            /* ICE_GRID_START  */
  YYSYMBOL_ICE_GRID_STOP = 19,             /* ICE_GRID_STOP  */
  YYSYMBOL_ICE_GRID_SIGNAL = 20,           /* ICE_GRID_SIGNAL  */
  YYSYMBOL_ICE_GRID_STDOUT = 21,           /* ICE_GRID_STDOUT  */
  YYSYMBOL_ICE_GRID_STDERR = 22,           /* ICE_GRID_STDERR  */
  YYSYMBOL_ICE_GRID_DESCRIBE = 23,         /* ICE_GRID_DESCRIBE  */
  YYSYMBOL_ICE_GRID_PROPERTIES = 24,       /* ICE_GRID_PROPERTIES  */
  YYSYMBOL_ICE_GRID_PROPERTY = 25,         /* ICE_GRID_PROPERTY  */
  YYSYMBOL_ICE_GRID_STATE = 26,            /* ICE_GRID_STATE  */
  YYSYMBOL_ICE_GRID_PID = 27,              /* ICE_GRID_PID  */
  YYSYMBOL_ICE_GRID_ENDPOINTS = 28,        /* ICE_GRID_ENDPOINTS  */
  YYSYMBOL_ICE_GRID_ACTIVATION = 29,       /* ICE_GRID_ACTIVATION  */
  YYSYMBOL_ICE_GRID_OBJECT = 30,           /* ICE_GRID_OBJECT  */
  YYSYMBOL_ICE_GRID_FIND = 31,             /* ICE_GRID_FIND  */
  YYSYMBOL_ICE_GRID_SHOW = 32,             /* ICE_GRID_SHOW  */
  YYSYMBOL_ICE_GRID_COPYING = 33,          /* ICE_GRID_COPYING  */
  YYSYMBOL_ICE_GRID_WARRANTY = 34,         /* ICE_GRID_WARRANTY  */
  YYSYMBOL_ICE_GRID_DIFF = 35,             /* ICE_GRID_DIFF  */
  YYSYMBOL_ICE_GRID_UPDATE = 36,           /* ICE_GRID_UPDATE  */
  YYSYMBOL_ICE_GRID_INSTANTIATE = 37,      /* ICE_GRID_INSTANTIATE  */
  YYSYMBOL_ICE_GRID_TEMPLATE = 38,         /* ICE_GRID_TEMPLATE  */
  YYSYMBOL_ICE_GRID_SERVICE = 39,          /* ICE_GRID_SERVICE  */
  YYSYMBOL_ICE_GRID_ENABLE = 40,           /* ICE_GRID_ENABLE  */
  YYSYMBOL_ICE_GRID_DISABLE = 41,          /* ICE_GRID_DISABLE  */
  YYSYMBOL_42_ = 42,                       /* ';'  */
  YYSYMBOL_YYACCEPT = 43,                  /* $accept  */
  YYSYMBOL_start = 44,                     /* start  */
  YYSYMBOL_commands = 45,                  /* commands  */
  YYSYMBOL_checkInterrupted = 46,          /* checkInterrupted  */
  YYSYMBOL_command = 47,                   /* command  */
  YYSYMBOL_strings = 48,                   /* strings  */
  YYSYMBOL_keyword = 49                    /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




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
#define YYLAST   2468

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  7
/* YYNRULES -- Number of rules.  */
#define YYNRULES  167
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  356

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   296


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
       2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
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
      35,    36,    37,    38,    39,    40,    41
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   102,   102,   106,   113,   116,   123,   131,   135,   139,
     143,   147,   151,   155,   159,   163,   167,   171,   175,   179,
     183,   187,   191,   195,   199,   203,   207,   211,   215,   219,
     223,   227,   231,   235,   239,   243,   247,   251,   255,   259,
     263,   267,   271,   275,   279,   283,   287,   291,   295,   299,
     303,   307,   311,   315,   319,   323,   327,   331,   335,   339,
     343,   347,   351,   355,   359,   363,   367,   371,   375,   379,
     383,   387,   391,   395,   399,   403,   407,   411,   415,   419,
     423,   427,   431,   435,   439,   443,   447,   451,   455,   459,
     463,   467,   471,   475,   479,   483,   487,   491,   495,   499,
     503,   507,   511,   515,   519,   523,   527,   531,   535,   539,
     543,   547,   551,   555,   559,   570,   574,   585,   596,   600,
     604,   608,   614,   619,   624,   628,   636,   641,   646,   652,
     659,   667,   670,   673,   676,   679,   682,   685,   688,   691,
     694,   697,   700,   703,   706,   709,   712,   715,   718,   721,
     724,   727,   730,   733,   736,   739,   742,   745,   748,   751,
     754,   757,   760,   763,   766,   769,   772,   775
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
  "\"end of file\"", "error", "\"invalid token\"", "ICE_GRID_HELP",
  "ICE_GRID_EXIT", "ICE_GRID_APPLICATION", "ICE_GRID_NODE",
  "ICE_GRID_REGISTRY", "ICE_GRID_SERVER", "ICE_GRID_ADAPTER",
  "ICE_GRID_PING", "ICE_GRID_LOAD", "ICE_GRID_SOCKETS", "ICE_GRID_ADD",
  "ICE_GRID_REMOVE", "ICE_GRID_LIST", "ICE_GRID_SHUTDOWN",
  "ICE_GRID_STRING", "ICE_GRID_START", "ICE_GRID_STOP", "ICE_GRID_SIGNAL",
  "ICE_GRID_STDOUT", "ICE_GRID_STDERR", "ICE_GRID_DESCRIBE",
  "ICE_GRID_PROPERTIES", "ICE_GRID_PROPERTY", "ICE_GRID_STATE",
  "ICE_GRID_PID", "ICE_GRID_ENDPOINTS", "ICE_GRID_ACTIVATION",
  "ICE_GRID_OBJECT", "ICE_GRID_FIND", "ICE_GRID_SHOW", "ICE_GRID_COPYING",
  "ICE_GRID_WARRANTY", "ICE_GRID_DIFF", "ICE_GRID_UPDATE",
  "ICE_GRID_INSTANTIATE", "ICE_GRID_TEMPLATE", "ICE_GRID_SERVICE",
  "ICE_GRID_ENABLE", "ICE_GRID_DISABLE", "';'", "$accept", "start",
  "commands", "checkInterrupted", "command", "strings", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-66)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-4)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      31,    52,    54,    59,   -66,    59,    19,   171,    64,  2345,
    2351,   100,   119,   103,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   108,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   135,   -66,    10,   -66,   -66,   -66,
     -66,   -66,   -66,    89,   -66,   -66,   -66,   -66,   102,   -66,
     -66,    78,   -66,   -66,   -66,   -66,   -66,   -66,  2316,   -66,
     -66,   -66,   -66,   210,   -66,   250,   289,   328,   367,   406,
     445,   484,   523,   562,   601,   640,   679,   718,   757,   796,
     835,   874,   913,   952,   991,  1030,  1069,  1108,  1147,  1186,
    1225,  1264,  1303,  1342,  1381,  1420,    99,  1459,  1498,  1537,
    1576,  1615,    79,  1654,  1693,  1732,  1771,  1810,    82,    86,
    1849,  1888,  1927,  1966,  2005,  2044,    34,    87,    88,   134,
     -66,  2083,   105,  2122,  2316,   -66,  2160,   110,   111,   112,
     113,   114,   120,   122,   123,   126,   127,   129,  2322,  2323,
    2327,  2328,  2329,  2330,  2331,  2333,  2334,  2335,  2336,  2337,
    2340,  2342,  2343,  2344,  2346,  2347,  2348,  2349,  2350,  2352,
    2353,  2354,  2355,  2356,  2357,  2358,  2359,  2360,  2361,  2362,
    2363,  2364,  2365,  2366,  2367,  2368,  2369,  2370,  2371,  2372,
    2373,  2374,  2375,  2376,  2377,  2378,  2379,  2380,  2381,  2382,
    2200,  2239,  2383,  2384,  2385,  2386,  2387,  2388,  2389,  2390,
    2391,  2392,   -66,  2393,  2394,  2395,  2396,  2397,  2398,  2399,
    2400,  2401,  2402,   -66,   -66,  2403,  2404,  2405,  2406,  2407,
    2408,  2409,  2410,  2411,  2412,  2413,  2414,  2415,  2278,   -66,
     -66,  2416,  2316,   -66,   -66,  2316,   -66,  2417,  2316,   -66,
    2418,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,  2419,  2420,  2421,  2422,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,  2423,  2424,   -66,   -66,   -66,   -66,  2425,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       6,     0,     6,     0,     1,     0,     0,     0,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,     0,   144,   145,   146,   167,   166,   147,   150,   151,
     148,   149,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   125,     5,     0,     4,
     124,     0,   131,   132,   133,   134,   135,   136,   130,   154,
     156,   163,     7,     0,     8,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,     0,   130,   130,   130,
     130,   130,     0,   130,   130,   130,   130,   130,     0,     0,
     130,   130,   130,   130,   130,   130,     0,     0,     0,     0,
     119,   130,     0,   130,   130,   113,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     130,   130,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   123,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   111,   112,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   130,   122,
     120,     0,   130,   126,   118,   130,   127,     0,   130,   114,
       0,    10,     9,    12,    11,    20,    19,    18,    17,    14,
      13,    16,    15,    32,    31,    34,    33,    36,    35,    40,
      39,    38,    37,    30,    29,    42,    41,    46,    45,    50,
      49,    48,    47,    44,    43,    52,    51,    54,    53,    80,
      79,    58,    57,    60,    59,    62,    61,    64,    63,    66,
      65,    56,    55,    72,    71,    74,    73,    68,    67,    70,
      69,    82,    81,    25,     0,     0,     0,     0,    76,    75,
      78,    77,    98,    97,   100,    99,    96,    95,   102,   101,
     104,   103,   108,   107,   110,   109,   106,   105,    94,    93,
      84,    83,    86,    85,    88,    87,    90,    89,    92,    91,
      28,     0,     0,   121,   128,   129,   115,     0,   116,    22,
      21,    24,    23,    27,    26,   117
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -66,   -66,   -66,   159,  2463,   -65,    48
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     2,     3,    47,   122,   123
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     128,   130,   132,   134,   136,   138,   140,   142,   144,   146,
     148,   150,   152,   154,   156,   158,   160,   162,   164,   166,
     168,   170,   172,   174,   176,   178,   180,   182,   184,   186,
     188,    -3,   193,   195,   197,   199,   201,   227,   204,   206,
     208,   210,   212,   108,   109,   216,   218,   220,   222,   224,
     226,    48,     4,    48,    -2,    63,   233,   228,   236,   237,
       6,    50,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,   189,   117,   110,   118,    64,   111,   112,   102,
      78,   126,   113,   114,   115,    79,    80,    99,   100,   119,
     120,   202,   190,    81,   213,   305,   307,   116,   214,   229,
     230,   101,    82,    83,    84,   231,   191,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,   234,   103,   104,
     105,    95,   241,   242,   243,   244,   245,    96,   106,    97,
      98,     5,   246,   342,   247,   248,   107,   344,   249,   250,
     345,   251,    51,   347,   240,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,    58,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,    62,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   124,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   125,   127,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   129,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   131,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     133,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   135,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   137,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   139,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   141,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   143,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   145,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,   121,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   147,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   149,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   151,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     153,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   155,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   157,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   159,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   161,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   163,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   165,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,   121,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   167,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   169,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   171,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     173,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   175,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   177,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   179,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   181,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   183,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   185,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,   121,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   187,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   192,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   194,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     196,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   198,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   200,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   203,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   205,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   207,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   209,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,    20,   121,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   211,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   215,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   217,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     219,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,    20,   121,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   221,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   223,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,    20,   121,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   225,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
      20,   121,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   232,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,    20,
     121,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   235,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,    20,   121,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   238,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   239,   304,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,    20,   121,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   306,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,    20,   121,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   341,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,    20,   121,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    20,   121,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,    65,    66,
      67,    71,    72,    73,   252,   253,    74,    75,    68,   254,
     255,   256,   257,   258,    76,   259,   260,   261,   262,   263,
      69,    70,   264,    77,   265,   266,   267,     0,   268,   269,
     270,   271,   272,     0,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   308,   309,   310,   311,   312,
     313,   314,   315,   316,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   343,   346,
     348,   349,   350,   351,   352,   353,   354,   355,    49
};

static const yytype_int16 yycheck[] =
{
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     0,    97,    98,    99,   100,   101,     3,   103,   104,
     105,   106,   107,    33,    34,   110,   111,   112,   113,   114,
     115,     3,     0,     5,     0,     7,   121,    23,   123,   124,
       1,    42,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,     3,     1,    15,     3,    42,    18,    19,     1,
      10,    63,    23,    24,    25,    15,    16,    14,    15,    17,
      42,    42,    23,    23,    42,   190,   191,    38,    42,    42,
      42,    28,    32,    14,    15,     1,    37,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    42,    13,    14,
      15,    32,    42,    42,    42,    42,    42,    38,    23,    40,
      41,     2,    42,   228,    42,    42,    31,   232,    42,    42,
     235,    42,     1,   238,   126,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    13,    14,
      15,    10,    11,    12,    42,    42,    15,    16,    23,    42,
      42,    42,    42,    42,    23,    42,    42,    42,    42,    42,
      35,    36,    42,    32,    42,    42,    42,    -1,    42,    42,
      42,    42,    42,    -1,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,     5
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    44,    45,    46,     0,    46,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    47,    49,    47,
      42,     1,     4,     5,     6,     7,     8,     9,    17,    30,
      32,    39,    42,    49,    42,    13,    14,    15,    23,    35,
      36,    10,    11,    12,    15,    16,    23,    32,    10,    15,
      16,    23,    32,    14,    15,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    32,    38,    40,    41,    14,
      15,    28,     1,    13,    14,    15,    23,    31,    33,    34,
      15,    18,    19,    23,    24,    25,    38,     1,     3,    17,
      42,    17,    48,    49,    17,    42,    49,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    48,     3,
      23,    37,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    48,    42,     3,    48,     3,    48,     3,    48,     3,
      48,     3,    48,    42,    42,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,     3,    48,     3,    23,    42,
      42,     1,     3,    48,    42,     3,    48,    48,    17,    42,
      49,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,     3,    48,     3,    48,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,     3,    48,    42,    48,    48,    42,    48,    42,    42,
      42,    42,    42,    42,    42,    42
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    43,    44,    44,    45,    45,    46,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    48,    48,    48,    48,
      48,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49
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
       4,     3,     3,     3,     4,     5,     5,     6,     4,     3,
       3,     4,     3,     3,     2,     1,     2,     2,     3,     3,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1
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
#line 103 "src/IceGrid/Grammar.y"
{
}
#line 1810 "src/IceGrid/Grammar.cpp"
    break;

  case 3: /* start: %empty  */
#line 106 "src/IceGrid/Grammar.y"
{
}
#line 1817 "src/IceGrid/Grammar.cpp"
    break;

  case 4: /* commands: commands checkInterrupted command  */
#line 114 "src/IceGrid/Grammar.y"
{
}
#line 1824 "src/IceGrid/Grammar.cpp"
    break;

  case 5: /* commands: checkInterrupted command  */
#line 117 "src/IceGrid/Grammar.y"
{
}
#line 1831 "src/IceGrid/Grammar.cpp"
    break;

  case 6: /* checkInterrupted: %empty  */
#line 123 "src/IceGrid/Grammar.y"
{
    parser->checkInterrupted();
}
#line 1839 "src/IceGrid/Grammar.cpp"
    break;

  case 7: /* command: ICE_GRID_HELP ';'  */
#line 132 "src/IceGrid/Grammar.y"
{
    parser->usage();
}
#line 1847 "src/IceGrid/Grammar.cpp"
    break;

  case 8: /* command: ICE_GRID_EXIT ';'  */
#line 136 "src/IceGrid/Grammar.y"
{
    return 0;
}
#line 1855 "src/IceGrid/Grammar.cpp"
    break;

  case 9: /* command: ICE_GRID_APPLICATION ICE_GRID_ADD strings ';'  */
#line 140 "src/IceGrid/Grammar.y"
{
    parser->addApplication(yyvsp[-1]);
}
#line 1863 "src/IceGrid/Grammar.cpp"
    break;

  case 10: /* command: ICE_GRID_APPLICATION ICE_GRID_ADD ICE_GRID_HELP ';'  */
#line 144 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "add");
}
#line 1871 "src/IceGrid/Grammar.cpp"
    break;

  case 11: /* command: ICE_GRID_APPLICATION ICE_GRID_REMOVE strings ';'  */
#line 148 "src/IceGrid/Grammar.y"
{
    parser->removeApplication(yyvsp[-1]);
}
#line 1879 "src/IceGrid/Grammar.cpp"
    break;

  case 12: /* command: ICE_GRID_APPLICATION ICE_GRID_REMOVE ICE_GRID_HELP ';'  */
#line 152 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "remove");
}
#line 1887 "src/IceGrid/Grammar.cpp"
    break;

  case 13: /* command: ICE_GRID_APPLICATION ICE_GRID_DIFF strings ';'  */
#line 156 "src/IceGrid/Grammar.y"
{
    parser->diffApplication(yyvsp[-1]);
}
#line 1895 "src/IceGrid/Grammar.cpp"
    break;

  case 14: /* command: ICE_GRID_APPLICATION ICE_GRID_DIFF ICE_GRID_HELP ';'  */
#line 160 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "diff");
}
#line 1903 "src/IceGrid/Grammar.cpp"
    break;

  case 15: /* command: ICE_GRID_APPLICATION ICE_GRID_UPDATE strings ';'  */
#line 164 "src/IceGrid/Grammar.y"
{
    parser->updateApplication(yyvsp[-1]);
}
#line 1911 "src/IceGrid/Grammar.cpp"
    break;

  case 16: /* command: ICE_GRID_APPLICATION ICE_GRID_UPDATE ICE_GRID_HELP ';'  */
#line 168 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "update");
}
#line 1919 "src/IceGrid/Grammar.cpp"
    break;

  case 17: /* command: ICE_GRID_APPLICATION ICE_GRID_DESCRIBE strings ';'  */
#line 172 "src/IceGrid/Grammar.y"
{
    parser->describeApplication(yyvsp[-1]);
}
#line 1927 "src/IceGrid/Grammar.cpp"
    break;

  case 18: /* command: ICE_GRID_APPLICATION ICE_GRID_DESCRIBE ICE_GRID_HELP ';'  */
#line 176 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "describe");
}
#line 1935 "src/IceGrid/Grammar.cpp"
    break;

  case 19: /* command: ICE_GRID_APPLICATION ICE_GRID_LIST strings ';'  */
#line 180 "src/IceGrid/Grammar.y"
{
    parser->listAllApplications(yyvsp[-1]);
}
#line 1943 "src/IceGrid/Grammar.cpp"
    break;

  case 20: /* command: ICE_GRID_APPLICATION ICE_GRID_LIST ICE_GRID_HELP ';'  */
#line 184 "src/IceGrid/Grammar.y"
{
    parser->usage("application", "list");
}
#line 1951 "src/IceGrid/Grammar.cpp"
    break;

  case 21: /* command: ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE strings ';'  */
#line 188 "src/IceGrid/Grammar.y"
{
    parser->describeServerTemplate(yyvsp[-1]);
}
#line 1959 "src/IceGrid/Grammar.cpp"
    break;

  case 22: /* command: ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'  */
#line 192 "src/IceGrid/Grammar.y"
{
    parser->usage("server template", "describe");
}
#line 1967 "src/IceGrid/Grammar.cpp"
    break;

  case 23: /* command: ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_INSTANTIATE strings ';'  */
#line 196 "src/IceGrid/Grammar.y"
{
    parser->instantiateServerTemplate(yyvsp[-1]);
}
#line 1975 "src/IceGrid/Grammar.cpp"
    break;

  case 24: /* command: ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_INSTANTIATE ICE_GRID_HELP ';'  */
#line 200 "src/IceGrid/Grammar.y"
{
    parser->usage("server template", "instantiate");
}
#line 1983 "src/IceGrid/Grammar.cpp"
    break;

  case 25: /* command: ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_HELP ';'  */
#line 204 "src/IceGrid/Grammar.y"
{
    parser->usage("server template");
}
#line 1991 "src/IceGrid/Grammar.cpp"
    break;

  case 26: /* command: ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE strings ';'  */
#line 208 "src/IceGrid/Grammar.y"
{
    parser->describeServiceTemplate(yyvsp[-1]);
}
#line 1999 "src/IceGrid/Grammar.cpp"
    break;

  case 27: /* command: ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'  */
#line 212 "src/IceGrid/Grammar.y"
{
    parser->usage("service template", "describe");
}
#line 2007 "src/IceGrid/Grammar.cpp"
    break;

  case 28: /* command: ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_HELP ';'  */
#line 216 "src/IceGrid/Grammar.y"
{
    parser->usage("service template");
}
#line 2015 "src/IceGrid/Grammar.cpp"
    break;

  case 29: /* command: ICE_GRID_NODE ICE_GRID_DESCRIBE strings ';'  */
#line 220 "src/IceGrid/Grammar.y"
{
    parser->describeNode(yyvsp[-1]);
}
#line 2023 "src/IceGrid/Grammar.cpp"
    break;

  case 30: /* command: ICE_GRID_NODE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'  */
#line 224 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "describe");
}
#line 2031 "src/IceGrid/Grammar.cpp"
    break;

  case 31: /* command: ICE_GRID_NODE ICE_GRID_PING strings ';'  */
#line 228 "src/IceGrid/Grammar.y"
{
    parser->pingNode(yyvsp[-1]);
}
#line 2039 "src/IceGrid/Grammar.cpp"
    break;

  case 32: /* command: ICE_GRID_NODE ICE_GRID_PING ICE_GRID_HELP ';'  */
#line 232 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "ping");
}
#line 2047 "src/IceGrid/Grammar.cpp"
    break;

  case 33: /* command: ICE_GRID_NODE ICE_GRID_LOAD strings ';'  */
#line 236 "src/IceGrid/Grammar.y"
{
    parser->printLoadNode(yyvsp[-1]);
}
#line 2055 "src/IceGrid/Grammar.cpp"
    break;

  case 34: /* command: ICE_GRID_NODE ICE_GRID_LOAD ICE_GRID_HELP ';'  */
#line 240 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "load");
}
#line 2063 "src/IceGrid/Grammar.cpp"
    break;

  case 35: /* command: ICE_GRID_NODE ICE_GRID_SOCKETS strings ';'  */
#line 244 "src/IceGrid/Grammar.y"
{
    parser->printNodeProcessorSockets(yyvsp[-1]);
}
#line 2071 "src/IceGrid/Grammar.cpp"
    break;

  case 36: /* command: ICE_GRID_NODE ICE_GRID_SOCKETS ICE_GRID_HELP ';'  */
#line 248 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "sockets");
}
#line 2079 "src/IceGrid/Grammar.cpp"
    break;

  case 37: /* command: ICE_GRID_NODE ICE_GRID_SHUTDOWN strings ';'  */
#line 252 "src/IceGrid/Grammar.y"
{
    parser->shutdownNode(yyvsp[-1]);
}
#line 2087 "src/IceGrid/Grammar.cpp"
    break;

  case 38: /* command: ICE_GRID_NODE ICE_GRID_SHUTDOWN ICE_GRID_HELP ';'  */
#line 256 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "shutdown");
}
#line 2095 "src/IceGrid/Grammar.cpp"
    break;

  case 39: /* command: ICE_GRID_NODE ICE_GRID_LIST strings ';'  */
#line 260 "src/IceGrid/Grammar.y"
{
    parser->listAllNodes(yyvsp[-1]);
}
#line 2103 "src/IceGrid/Grammar.cpp"
    break;

  case 40: /* command: ICE_GRID_NODE ICE_GRID_LIST ICE_GRID_HELP ';'  */
#line 264 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "list");
}
#line 2111 "src/IceGrid/Grammar.cpp"
    break;

  case 41: /* command: ICE_GRID_NODE ICE_GRID_SHOW strings ';'  */
#line 268 "src/IceGrid/Grammar.y"
{
    parser->show("node", yyvsp[-1]);
}
#line 2119 "src/IceGrid/Grammar.cpp"
    break;

  case 42: /* command: ICE_GRID_NODE ICE_GRID_SHOW ICE_GRID_HELP ';'  */
#line 272 "src/IceGrid/Grammar.y"
{
    parser->usage("node", "show");
}
#line 2127 "src/IceGrid/Grammar.cpp"
    break;

  case 43: /* command: ICE_GRID_REGISTRY ICE_GRID_DESCRIBE strings ';'  */
#line 276 "src/IceGrid/Grammar.y"
{
    parser->describeRegistry(yyvsp[-1]);
}
#line 2135 "src/IceGrid/Grammar.cpp"
    break;

  case 44: /* command: ICE_GRID_REGISTRY ICE_GRID_DESCRIBE ICE_GRID_HELP ';'  */
#line 280 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "describe");
}
#line 2143 "src/IceGrid/Grammar.cpp"
    break;

  case 45: /* command: ICE_GRID_REGISTRY ICE_GRID_PING strings ';'  */
#line 284 "src/IceGrid/Grammar.y"
{
    parser->pingRegistry(yyvsp[-1]);
}
#line 2151 "src/IceGrid/Grammar.cpp"
    break;

  case 46: /* command: ICE_GRID_REGISTRY ICE_GRID_PING ICE_GRID_HELP ';'  */
#line 288 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "ping");
}
#line 2159 "src/IceGrid/Grammar.cpp"
    break;

  case 47: /* command: ICE_GRID_REGISTRY ICE_GRID_SHUTDOWN strings ';'  */
#line 292 "src/IceGrid/Grammar.y"
{
    parser->shutdownRegistry(yyvsp[-1]);
}
#line 2167 "src/IceGrid/Grammar.cpp"
    break;

  case 48: /* command: ICE_GRID_REGISTRY ICE_GRID_SHUTDOWN ICE_GRID_HELP ';'  */
#line 296 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "shutdown");
}
#line 2175 "src/IceGrid/Grammar.cpp"
    break;

  case 49: /* command: ICE_GRID_REGISTRY ICE_GRID_LIST strings ';'  */
#line 300 "src/IceGrid/Grammar.y"
{
    parser->listAllRegistries(yyvsp[-1]);
}
#line 2183 "src/IceGrid/Grammar.cpp"
    break;

  case 50: /* command: ICE_GRID_REGISTRY ICE_GRID_LIST ICE_GRID_HELP ';'  */
#line 304 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "list");
}
#line 2191 "src/IceGrid/Grammar.cpp"
    break;

  case 51: /* command: ICE_GRID_REGISTRY ICE_GRID_SHOW strings ';'  */
#line 308 "src/IceGrid/Grammar.y"
{
    parser->show("registry", yyvsp[-1]);
}
#line 2199 "src/IceGrid/Grammar.cpp"
    break;

  case 52: /* command: ICE_GRID_REGISTRY ICE_GRID_SHOW ICE_GRID_HELP ';'  */
#line 312 "src/IceGrid/Grammar.y"
{
    parser->usage("registry", "show");
}
#line 2207 "src/IceGrid/Grammar.cpp"
    break;

  case 53: /* command: ICE_GRID_SERVER ICE_GRID_REMOVE strings ';'  */
#line 316 "src/IceGrid/Grammar.y"
{
    parser->removeServer(yyvsp[-1]);
}
#line 2215 "src/IceGrid/Grammar.cpp"
    break;

  case 54: /* command: ICE_GRID_SERVER ICE_GRID_REMOVE ICE_GRID_HELP ';'  */
#line 320 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "remove");
}
#line 2223 "src/IceGrid/Grammar.cpp"
    break;

  case 55: /* command: ICE_GRID_SERVER ICE_GRID_DESCRIBE strings ';'  */
#line 324 "src/IceGrid/Grammar.y"
{
    parser->describeServer(yyvsp[-1]);
}
#line 2231 "src/IceGrid/Grammar.cpp"
    break;

  case 56: /* command: ICE_GRID_SERVER ICE_GRID_DESCRIBE ICE_GRID_HELP ';'  */
#line 328 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "describe");
}
#line 2239 "src/IceGrid/Grammar.cpp"
    break;

  case 57: /* command: ICE_GRID_SERVER ICE_GRID_START strings ';'  */
#line 332 "src/IceGrid/Grammar.y"
{
    parser->startServer(yyvsp[-1]);
}
#line 2247 "src/IceGrid/Grammar.cpp"
    break;

  case 58: /* command: ICE_GRID_SERVER ICE_GRID_START ICE_GRID_HELP ';'  */
#line 336 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "start");
}
#line 2255 "src/IceGrid/Grammar.cpp"
    break;

  case 59: /* command: ICE_GRID_SERVER ICE_GRID_STOP strings ';'  */
#line 340 "src/IceGrid/Grammar.y"
{
    parser->stopServer(yyvsp[-1]);
}
#line 2263 "src/IceGrid/Grammar.cpp"
    break;

  case 60: /* command: ICE_GRID_SERVER ICE_GRID_STOP ICE_GRID_HELP ';'  */
#line 344 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "stop");
}
#line 2271 "src/IceGrid/Grammar.cpp"
    break;

  case 61: /* command: ICE_GRID_SERVER ICE_GRID_SIGNAL strings ';'  */
#line 348 "src/IceGrid/Grammar.y"
{
    parser->signalServer(yyvsp[-1]);
}
#line 2279 "src/IceGrid/Grammar.cpp"
    break;

  case 62: /* command: ICE_GRID_SERVER ICE_GRID_SIGNAL ICE_GRID_HELP ';'  */
#line 352 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "signal");
}
#line 2287 "src/IceGrid/Grammar.cpp"
    break;

  case 63: /* command: ICE_GRID_SERVER ICE_GRID_STDOUT strings ';'  */
#line 356 "src/IceGrid/Grammar.y"
{
    parser->writeMessage(yyvsp[-1], 1);
}
#line 2295 "src/IceGrid/Grammar.cpp"
    break;

  case 64: /* command: ICE_GRID_SERVER ICE_GRID_STDOUT ICE_GRID_HELP ';'  */
#line 360 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "stdout");
}
#line 2303 "src/IceGrid/Grammar.cpp"
    break;

  case 65: /* command: ICE_GRID_SERVER ICE_GRID_STDERR strings ';'  */
#line 364 "src/IceGrid/Grammar.y"
{
    parser->writeMessage(yyvsp[-1], 2);
}
#line 2311 "src/IceGrid/Grammar.cpp"
    break;

  case 66: /* command: ICE_GRID_SERVER ICE_GRID_STDERR ICE_GRID_HELP ';'  */
#line 368 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "stderr");
}
#line 2319 "src/IceGrid/Grammar.cpp"
    break;

  case 67: /* command: ICE_GRID_SERVER ICE_GRID_STATE strings ';'  */
#line 372 "src/IceGrid/Grammar.y"
{
    parser->stateServer(yyvsp[-1]);
}
#line 2327 "src/IceGrid/Grammar.cpp"
    break;

  case 68: /* command: ICE_GRID_SERVER ICE_GRID_STATE ICE_GRID_HELP ';'  */
#line 376 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "start");
}
#line 2335 "src/IceGrid/Grammar.cpp"
    break;

  case 69: /* command: ICE_GRID_SERVER ICE_GRID_PID strings ';'  */
#line 380 "src/IceGrid/Grammar.y"
{
    parser->pidServer(yyvsp[-1]);
}
#line 2343 "src/IceGrid/Grammar.cpp"
    break;

  case 70: /* command: ICE_GRID_SERVER ICE_GRID_PID ICE_GRID_HELP ';'  */
#line 384 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "pid");
}
#line 2351 "src/IceGrid/Grammar.cpp"
    break;

  case 71: /* command: ICE_GRID_SERVER ICE_GRID_PROPERTIES strings ';'  */
#line 388 "src/IceGrid/Grammar.y"
{
    parser->propertiesServer(yyvsp[-1], false);
}
#line 2359 "src/IceGrid/Grammar.cpp"
    break;

  case 72: /* command: ICE_GRID_SERVER ICE_GRID_PROPERTIES ICE_GRID_HELP ';'  */
#line 392 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "properties");
}
#line 2367 "src/IceGrid/Grammar.cpp"
    break;

  case 73: /* command: ICE_GRID_SERVER ICE_GRID_PROPERTY strings ';'  */
#line 396 "src/IceGrid/Grammar.y"
{
    parser->propertiesServer(yyvsp[-1], true);
}
#line 2375 "src/IceGrid/Grammar.cpp"
    break;

  case 74: /* command: ICE_GRID_SERVER ICE_GRID_PROPERTY ICE_GRID_HELP ';'  */
#line 400 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "property");
}
#line 2383 "src/IceGrid/Grammar.cpp"
    break;

  case 75: /* command: ICE_GRID_SERVER ICE_GRID_ENABLE strings ';'  */
#line 404 "src/IceGrid/Grammar.y"
{
    parser->enableServer(yyvsp[-1], true);
}
#line 2391 "src/IceGrid/Grammar.cpp"
    break;

  case 76: /* command: ICE_GRID_SERVER ICE_GRID_ENABLE ICE_GRID_HELP ';'  */
#line 408 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "enable");
}
#line 2399 "src/IceGrid/Grammar.cpp"
    break;

  case 77: /* command: ICE_GRID_SERVER ICE_GRID_DISABLE strings ';'  */
#line 412 "src/IceGrid/Grammar.y"
{
    parser->enableServer(yyvsp[-1], false);
}
#line 2407 "src/IceGrid/Grammar.cpp"
    break;

  case 78: /* command: ICE_GRID_SERVER ICE_GRID_DISABLE ICE_GRID_HELP ';'  */
#line 416 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "disable");
}
#line 2415 "src/IceGrid/Grammar.cpp"
    break;

  case 79: /* command: ICE_GRID_SERVER ICE_GRID_LIST strings ';'  */
#line 420 "src/IceGrid/Grammar.y"
{
    parser->listAllServers(yyvsp[-1]);
}
#line 2423 "src/IceGrid/Grammar.cpp"
    break;

  case 80: /* command: ICE_GRID_SERVER ICE_GRID_LIST ICE_GRID_HELP ';'  */
#line 424 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "list");
}
#line 2431 "src/IceGrid/Grammar.cpp"
    break;

  case 81: /* command: ICE_GRID_SERVER ICE_GRID_SHOW strings ';'  */
#line 428 "src/IceGrid/Grammar.y"
{
    parser->show("server", yyvsp[-1]);
}
#line 2439 "src/IceGrid/Grammar.cpp"
    break;

  case 82: /* command: ICE_GRID_SERVER ICE_GRID_SHOW ICE_GRID_HELP ';'  */
#line 432 "src/IceGrid/Grammar.y"
{
    parser->usage("server", "show");
}
#line 2447 "src/IceGrid/Grammar.cpp"
    break;

  case 83: /* command: ICE_GRID_SERVICE ICE_GRID_START strings ';'  */
#line 436 "src/IceGrid/Grammar.y"
{
    parser->startService(yyvsp[-1]);
}
#line 2455 "src/IceGrid/Grammar.cpp"
    break;

  case 84: /* command: ICE_GRID_SERVICE ICE_GRID_START ICE_GRID_HELP ';'  */
#line 440 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "start");
}
#line 2463 "src/IceGrid/Grammar.cpp"
    break;

  case 85: /* command: ICE_GRID_SERVICE ICE_GRID_STOP strings ';'  */
#line 444 "src/IceGrid/Grammar.y"
{
    parser->stopService(yyvsp[-1]);
}
#line 2471 "src/IceGrid/Grammar.cpp"
    break;

  case 86: /* command: ICE_GRID_SERVICE ICE_GRID_STOP ICE_GRID_HELP ';'  */
#line 448 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "stop");
}
#line 2479 "src/IceGrid/Grammar.cpp"
    break;

  case 87: /* command: ICE_GRID_SERVICE ICE_GRID_DESCRIBE strings ';'  */
#line 452 "src/IceGrid/Grammar.y"
{
    parser->describeService(yyvsp[-1]);
}
#line 2487 "src/IceGrid/Grammar.cpp"
    break;

  case 88: /* command: ICE_GRID_SERVICE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'  */
#line 456 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "describe");
}
#line 2495 "src/IceGrid/Grammar.cpp"
    break;

  case 89: /* command: ICE_GRID_SERVICE ICE_GRID_PROPERTIES strings ';'  */
#line 460 "src/IceGrid/Grammar.y"
{
    parser->propertiesService(yyvsp[-1], false);
}
#line 2503 "src/IceGrid/Grammar.cpp"
    break;

  case 90: /* command: ICE_GRID_SERVICE ICE_GRID_PROPERTIES ICE_GRID_HELP ';'  */
#line 464 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "properties");
}
#line 2511 "src/IceGrid/Grammar.cpp"
    break;

  case 91: /* command: ICE_GRID_SERVICE ICE_GRID_PROPERTY strings ';'  */
#line 468 "src/IceGrid/Grammar.y"
{
    parser->propertiesService(yyvsp[-1], true);
}
#line 2519 "src/IceGrid/Grammar.cpp"
    break;

  case 92: /* command: ICE_GRID_SERVICE ICE_GRID_PROPERTY ICE_GRID_HELP ';'  */
#line 472 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "property");
}
#line 2527 "src/IceGrid/Grammar.cpp"
    break;

  case 93: /* command: ICE_GRID_SERVICE ICE_GRID_LIST strings ';'  */
#line 476 "src/IceGrid/Grammar.y"
{
    parser->listServices(yyvsp[-1]);
}
#line 2535 "src/IceGrid/Grammar.cpp"
    break;

  case 94: /* command: ICE_GRID_SERVICE ICE_GRID_LIST ICE_GRID_HELP ';'  */
#line 480 "src/IceGrid/Grammar.y"
{
    parser->usage("service", "list");
}
#line 2543 "src/IceGrid/Grammar.cpp"
    break;

  case 95: /* command: ICE_GRID_ADAPTER ICE_GRID_ENDPOINTS strings ';'  */
#line 484 "src/IceGrid/Grammar.y"
{
    parser->endpointsAdapter(yyvsp[-1]);
}
#line 2551 "src/IceGrid/Grammar.cpp"
    break;

  case 96: /* command: ICE_GRID_ADAPTER ICE_GRID_ENDPOINTS ICE_GRID_HELP ';'  */
#line 488 "src/IceGrid/Grammar.y"
{
    parser->usage("adapter", "endpoints");
}
#line 2559 "src/IceGrid/Grammar.cpp"
    break;

  case 97: /* command: ICE_GRID_ADAPTER ICE_GRID_REMOVE strings ';'  */
#line 492 "src/IceGrid/Grammar.y"
{
    parser->removeAdapter(yyvsp[-1]);
}
#line 2567 "src/IceGrid/Grammar.cpp"
    break;

  case 98: /* command: ICE_GRID_ADAPTER ICE_GRID_REMOVE ICE_GRID_HELP ';'  */
#line 496 "src/IceGrid/Grammar.y"
{
    parser->usage("adapter", "remove");
}
#line 2575 "src/IceGrid/Grammar.cpp"
    break;

  case 99: /* command: ICE_GRID_ADAPTER ICE_GRID_LIST strings ';'  */
#line 500 "src/IceGrid/Grammar.y"
{
    parser->listAllAdapters(yyvsp[-1]);
}
#line 2583 "src/IceGrid/Grammar.cpp"
    break;

  case 100: /* command: ICE_GRID_ADAPTER ICE_GRID_LIST ICE_GRID_HELP ';'  */
#line 504 "src/IceGrid/Grammar.y"
{
    parser->usage("adapter", "list");
}
#line 2591 "src/IceGrid/Grammar.cpp"
    break;

  case 101: /* command: ICE_GRID_OBJECT ICE_GRID_ADD strings ';'  */
#line 508 "src/IceGrid/Grammar.y"
{
    parser->addObject(yyvsp[-1]);
}
#line 2599 "src/IceGrid/Grammar.cpp"
    break;

  case 102: /* command: ICE_GRID_OBJECT ICE_GRID_ADD ICE_GRID_HELP ';'  */
#line 512 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "add");
}
#line 2607 "src/IceGrid/Grammar.cpp"
    break;

  case 103: /* command: ICE_GRID_OBJECT ICE_GRID_REMOVE strings ';'  */
#line 516 "src/IceGrid/Grammar.y"
{
    parser->removeObject(yyvsp[-1]);
}
#line 2615 "src/IceGrid/Grammar.cpp"
    break;

  case 104: /* command: ICE_GRID_OBJECT ICE_GRID_REMOVE ICE_GRID_HELP ';'  */
#line 520 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "remove");
}
#line 2623 "src/IceGrid/Grammar.cpp"
    break;

  case 105: /* command: ICE_GRID_OBJECT ICE_GRID_FIND strings ';'  */
#line 524 "src/IceGrid/Grammar.y"
{
    parser->findObject(yyvsp[-1]);
}
#line 2631 "src/IceGrid/Grammar.cpp"
    break;

  case 106: /* command: ICE_GRID_OBJECT ICE_GRID_FIND ICE_GRID_HELP ';'  */
#line 528 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "find");
}
#line 2639 "src/IceGrid/Grammar.cpp"
    break;

  case 107: /* command: ICE_GRID_OBJECT ICE_GRID_LIST strings ';'  */
#line 532 "src/IceGrid/Grammar.y"
{
    parser->listObject(yyvsp[-1]);
}
#line 2647 "src/IceGrid/Grammar.cpp"
    break;

  case 108: /* command: ICE_GRID_OBJECT ICE_GRID_LIST ICE_GRID_HELP ';'  */
#line 536 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "list");
}
#line 2655 "src/IceGrid/Grammar.cpp"
    break;

  case 109: /* command: ICE_GRID_OBJECT ICE_GRID_DESCRIBE strings ';'  */
#line 540 "src/IceGrid/Grammar.y"
{
    parser->describeObject(yyvsp[-1]);
}
#line 2663 "src/IceGrid/Grammar.cpp"
    break;

  case 110: /* command: ICE_GRID_OBJECT ICE_GRID_DESCRIBE ICE_GRID_HELP ';'  */
#line 544 "src/IceGrid/Grammar.y"
{
    parser->usage("object", "describe");
}
#line 2671 "src/IceGrid/Grammar.cpp"
    break;

  case 111: /* command: ICE_GRID_SHOW ICE_GRID_COPYING ';'  */
#line 548 "src/IceGrid/Grammar.y"
{
    parser->showCopying();
}
#line 2679 "src/IceGrid/Grammar.cpp"
    break;

  case 112: /* command: ICE_GRID_SHOW ICE_GRID_WARRANTY ';'  */
#line 552 "src/IceGrid/Grammar.y"
{
    parser->showWarranty();
}
#line 2687 "src/IceGrid/Grammar.cpp"
    break;

  case 113: /* command: ICE_GRID_HELP keyword ';'  */
#line 556 "src/IceGrid/Grammar.y"
{
    parser->usage(yyvsp[-1].front());
}
#line 2695 "src/IceGrid/Grammar.cpp"
    break;

  case 114: /* command: ICE_GRID_HELP keyword keyword ';'  */
#line 560 "src/IceGrid/Grammar.y"
{
    if((yyvsp[-2].front() == "server" || yyvsp[-2].front() == "service") && yyvsp[-1].front() == "template")
    {
        parser->usage(yyvsp[-2].front() + " " + yyvsp[-1].front());
    }
    else
    {
        parser->usage(yyvsp[-2].front(), yyvsp[-1].front());
    }
}
#line 2710 "src/IceGrid/Grammar.cpp"
    break;

  case 115: /* command: ICE_GRID_HELP keyword ICE_GRID_STRING strings ';'  */
#line 571 "src/IceGrid/Grammar.y"
{
    parser->usage(yyvsp[-3].front(), yyvsp[-2].front());
}
#line 2718 "src/IceGrid/Grammar.cpp"
    break;

  case 116: /* command: ICE_GRID_HELP keyword keyword keyword ';'  */
#line 575 "src/IceGrid/Grammar.y"
{
    if((yyvsp[-3].front() == "server" || yyvsp[-3].front() == "service") && yyvsp[-2].front() == "template")
    {
        parser->usage(yyvsp[-3].front() + " " + yyvsp[-2].front(), yyvsp[-1].front());
    }
    else
    {
        parser->usage(yyvsp[-3].front(), yyvsp[-2].front());
    }
}
#line 2733 "src/IceGrid/Grammar.cpp"
    break;

  case 117: /* command: ICE_GRID_HELP keyword keyword ICE_GRID_STRING strings ';'  */
#line 586 "src/IceGrid/Grammar.y"
{
    if((yyvsp[-4].front() == "server" || yyvsp[-4].front() == "service") && yyvsp[-3].front() == "template")
    {
        parser->usage(yyvsp[-4].front() + " " + yyvsp[-3].front(), yyvsp[-2].front());
    }
    else
    {
        parser->usage(yyvsp[-4].front(), yyvsp[-3].front());
    }
}
#line 2748 "src/IceGrid/Grammar.cpp"
    break;

  case 118: /* command: ICE_GRID_HELP ICE_GRID_STRING strings ';'  */
#line 597 "src/IceGrid/Grammar.y"
{
    parser->usage(yyvsp[-2].front());
}
#line 2756 "src/IceGrid/Grammar.cpp"
    break;

  case 119: /* command: ICE_GRID_HELP error ';'  */
#line 601 "src/IceGrid/Grammar.y"
{
    parser->usage();
}
#line 2764 "src/IceGrid/Grammar.cpp"
    break;

  case 120: /* command: keyword ICE_GRID_HELP ';'  */
#line 605 "src/IceGrid/Grammar.y"
{
    parser->usage(yyvsp[-2].front());
}
#line 2772 "src/IceGrid/Grammar.cpp"
    break;

  case 121: /* command: keyword ICE_GRID_STRING error ';'  */
#line 609 "src/IceGrid/Grammar.y"
{
    yyvsp[-3].push_back(yyvsp[-2].front());
    parser->invalidCommand(yyvsp[-3]);
    yyerrok;
}
#line 2782 "src/IceGrid/Grammar.cpp"
    break;

  case 122: /* command: keyword error ';'  */
#line 615 "src/IceGrid/Grammar.y"
{
    parser->invalidCommand(yyvsp[-2]);
    yyerrok;
}
#line 2791 "src/IceGrid/Grammar.cpp"
    break;

  case 123: /* command: ICE_GRID_STRING error ';'  */
#line 620 "src/IceGrid/Grammar.y"
{
    parser->invalidCommand(yyvsp[-2]);
    yyerrok;
}
#line 2800 "src/IceGrid/Grammar.cpp"
    break;

  case 124: /* command: error ';'  */
#line 625 "src/IceGrid/Grammar.y"
{
    yyerrok;
}
#line 2808 "src/IceGrid/Grammar.cpp"
    break;

  case 125: /* command: ';'  */
#line 629 "src/IceGrid/Grammar.y"
{
}
#line 2815 "src/IceGrid/Grammar.cpp"
    break;

  case 126: /* strings: ICE_GRID_STRING strings  */
#line 637 "src/IceGrid/Grammar.y"
{
    yyval = yyvsp[0];
    yyval.push_front(yyvsp[-1].front());
}
#line 2824 "src/IceGrid/Grammar.cpp"
    break;

  case 127: /* strings: keyword strings  */
#line 642 "src/IceGrid/Grammar.y"
{
    yyval = yyvsp[0];
    yyval.push_front(yyvsp[-1].front());
}
#line 2833 "src/IceGrid/Grammar.cpp"
    break;

  case 128: /* strings: ICE_GRID_STRING ICE_GRID_HELP strings  */
#line 647 "src/IceGrid/Grammar.y"
{
    yyval = yyvsp[-1];
    yyval.push_front("help");
    yyval.push_front(yyvsp[-2].front());
}
#line 2843 "src/IceGrid/Grammar.cpp"
    break;

  case 129: /* strings: keyword ICE_GRID_HELP strings  */
#line 653 "src/IceGrid/Grammar.y"
{
    yyval = yyvsp[-1];
    yyval.push_front("help");
    yyval.push_front(yyvsp[-2].front());
}
#line 2853 "src/IceGrid/Grammar.cpp"
    break;

  case 130: /* strings: %empty  */
#line 659 "src/IceGrid/Grammar.y"
{
    yyval = YYSTYPE();
}
#line 2861 "src/IceGrid/Grammar.cpp"
    break;

  case 131: /* keyword: ICE_GRID_EXIT  */
#line 668 "src/IceGrid/Grammar.y"
{
}
#line 2868 "src/IceGrid/Grammar.cpp"
    break;

  case 132: /* keyword: ICE_GRID_APPLICATION  */
#line 671 "src/IceGrid/Grammar.y"
{
}
#line 2875 "src/IceGrid/Grammar.cpp"
    break;

  case 133: /* keyword: ICE_GRID_NODE  */
#line 674 "src/IceGrid/Grammar.y"
{
}
#line 2882 "src/IceGrid/Grammar.cpp"
    break;

  case 134: /* keyword: ICE_GRID_REGISTRY  */
#line 677 "src/IceGrid/Grammar.y"
{
}
#line 2889 "src/IceGrid/Grammar.cpp"
    break;

  case 135: /* keyword: ICE_GRID_SERVER  */
#line 680 "src/IceGrid/Grammar.y"
{
}
#line 2896 "src/IceGrid/Grammar.cpp"
    break;

  case 136: /* keyword: ICE_GRID_ADAPTER  */
#line 683 "src/IceGrid/Grammar.y"
{
}
#line 2903 "src/IceGrid/Grammar.cpp"
    break;

  case 137: /* keyword: ICE_GRID_PING  */
#line 686 "src/IceGrid/Grammar.y"
{
}
#line 2910 "src/IceGrid/Grammar.cpp"
    break;

  case 138: /* keyword: ICE_GRID_LOAD  */
#line 689 "src/IceGrid/Grammar.y"
{
}
#line 2917 "src/IceGrid/Grammar.cpp"
    break;

  case 139: /* keyword: ICE_GRID_SOCKETS  */
#line 692 "src/IceGrid/Grammar.y"
{
}
#line 2924 "src/IceGrid/Grammar.cpp"
    break;

  case 140: /* keyword: ICE_GRID_ADD  */
#line 695 "src/IceGrid/Grammar.y"
{
}
#line 2931 "src/IceGrid/Grammar.cpp"
    break;

  case 141: /* keyword: ICE_GRID_REMOVE  */
#line 698 "src/IceGrid/Grammar.y"
{
}
#line 2938 "src/IceGrid/Grammar.cpp"
    break;

  case 142: /* keyword: ICE_GRID_LIST  */
#line 701 "src/IceGrid/Grammar.y"
{
}
#line 2945 "src/IceGrid/Grammar.cpp"
    break;

  case 143: /* keyword: ICE_GRID_SHUTDOWN  */
#line 704 "src/IceGrid/Grammar.y"
{
}
#line 2952 "src/IceGrid/Grammar.cpp"
    break;

  case 144: /* keyword: ICE_GRID_START  */
#line 707 "src/IceGrid/Grammar.y"
{
}
#line 2959 "src/IceGrid/Grammar.cpp"
    break;

  case 145: /* keyword: ICE_GRID_STOP  */
#line 710 "src/IceGrid/Grammar.y"
{
}
#line 2966 "src/IceGrid/Grammar.cpp"
    break;

  case 146: /* keyword: ICE_GRID_SIGNAL  */
#line 713 "src/IceGrid/Grammar.y"
{
}
#line 2973 "src/IceGrid/Grammar.cpp"
    break;

  case 147: /* keyword: ICE_GRID_DESCRIBE  */
#line 716 "src/IceGrid/Grammar.y"
{
}
#line 2980 "src/IceGrid/Grammar.cpp"
    break;

  case 148: /* keyword: ICE_GRID_STATE  */
#line 719 "src/IceGrid/Grammar.y"
{
}
#line 2987 "src/IceGrid/Grammar.cpp"
    break;

  case 149: /* keyword: ICE_GRID_PID  */
#line 722 "src/IceGrid/Grammar.y"
{
}
#line 2994 "src/IceGrid/Grammar.cpp"
    break;

  case 150: /* keyword: ICE_GRID_PROPERTIES  */
#line 725 "src/IceGrid/Grammar.y"
{
}
#line 3001 "src/IceGrid/Grammar.cpp"
    break;

  case 151: /* keyword: ICE_GRID_PROPERTY  */
#line 728 "src/IceGrid/Grammar.y"
{
}
#line 3008 "src/IceGrid/Grammar.cpp"
    break;

  case 152: /* keyword: ICE_GRID_ENDPOINTS  */
#line 731 "src/IceGrid/Grammar.y"
{
}
#line 3015 "src/IceGrid/Grammar.cpp"
    break;

  case 153: /* keyword: ICE_GRID_ACTIVATION  */
#line 734 "src/IceGrid/Grammar.y"
{
}
#line 3022 "src/IceGrid/Grammar.cpp"
    break;

  case 154: /* keyword: ICE_GRID_OBJECT  */
#line 737 "src/IceGrid/Grammar.y"
{
}
#line 3029 "src/IceGrid/Grammar.cpp"
    break;

  case 155: /* keyword: ICE_GRID_FIND  */
#line 740 "src/IceGrid/Grammar.y"
{
}
#line 3036 "src/IceGrid/Grammar.cpp"
    break;

  case 156: /* keyword: ICE_GRID_SHOW  */
#line 743 "src/IceGrid/Grammar.y"
{
}
#line 3043 "src/IceGrid/Grammar.cpp"
    break;

  case 157: /* keyword: ICE_GRID_COPYING  */
#line 746 "src/IceGrid/Grammar.y"
{
}
#line 3050 "src/IceGrid/Grammar.cpp"
    break;

  case 158: /* keyword: ICE_GRID_WARRANTY  */
#line 749 "src/IceGrid/Grammar.y"
{
}
#line 3057 "src/IceGrid/Grammar.cpp"
    break;

  case 159: /* keyword: ICE_GRID_DIFF  */
#line 752 "src/IceGrid/Grammar.y"
{
}
#line 3064 "src/IceGrid/Grammar.cpp"
    break;

  case 160: /* keyword: ICE_GRID_UPDATE  */
#line 755 "src/IceGrid/Grammar.y"
{
}
#line 3071 "src/IceGrid/Grammar.cpp"
    break;

  case 161: /* keyword: ICE_GRID_INSTANTIATE  */
#line 758 "src/IceGrid/Grammar.y"
{
}
#line 3078 "src/IceGrid/Grammar.cpp"
    break;

  case 162: /* keyword: ICE_GRID_TEMPLATE  */
#line 761 "src/IceGrid/Grammar.y"
{
}
#line 3085 "src/IceGrid/Grammar.cpp"
    break;

  case 163: /* keyword: ICE_GRID_SERVICE  */
#line 764 "src/IceGrid/Grammar.y"
{
}
#line 3092 "src/IceGrid/Grammar.cpp"
    break;

  case 164: /* keyword: ICE_GRID_ENABLE  */
#line 767 "src/IceGrid/Grammar.y"
{
}
#line 3099 "src/IceGrid/Grammar.cpp"
    break;

  case 165: /* keyword: ICE_GRID_DISABLE  */
#line 770 "src/IceGrid/Grammar.y"
{
}
#line 3106 "src/IceGrid/Grammar.cpp"
    break;

  case 166: /* keyword: ICE_GRID_STDERR  */
#line 773 "src/IceGrid/Grammar.y"
{
}
#line 3113 "src/IceGrid/Grammar.cpp"
    break;

  case 167: /* keyword: ICE_GRID_STDOUT  */
#line 776 "src/IceGrid/Grammar.y"
{
}
#line 3120 "src/IceGrid/Grammar.cpp"
    break;


#line 3124 "src/IceGrid/Grammar.cpp"

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

