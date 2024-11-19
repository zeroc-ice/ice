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
#line 1 "src/Slice/Grammar.y"


// Copyright (c) ZeroC, Inc.

#line 25 "src/Slice/Grammar.y"


// Defines the rule bison uses to reduce token locations. Bison asks that the macro should
// be one-line, and treatable as a single statement when followed by a semi-colon.
// `N` is the number of tokens that are being combined, and (Cur) is their combined location.
#define YYLLOC_DEFAULT(Cur, Rhs, N)                               \
do                                                                \
    if (N == 1)                                                   \
    {                                                             \
        (Cur) = (YYRHSLOC((Rhs), 1));                             \
    }                                                             \
    else                                                          \
    {                                                             \
        if (N)                                                    \
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


#line 104 "src/Slice/Grammar.cpp"

/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yydebug         slice_debug
#define yynerrs         slice_nerrs

/* First part of user prologue.  */
#line 64 "src/Slice/Grammar.y"


#include "Ice/UUID.h"
#include "Parser.h"

#include <cstring>
#include <limits>

#ifdef _MSC_VER
// warning C4102: 'yyoverflowlab' : unreferenced label
#    pragma warning(disable:4102)
// warning C4127: conditional expression is constant
#    pragma warning(disable:4127)
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

// Avoid clang warnings in generated grammar
#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wconversion"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#    pragma clang diagnostic ignored "-Wunused-but-set-variable"
#    pragma clang diagnostic ignored "-Wunused-label"
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
        currentUnit->error("syntax error");
    }
    else
    {
        currentUnit->error(s);
    }
}


#line 169 "src/Slice/Grammar.cpp"

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
  YYSYMBOL_ICE_MODULE = 3,                 /* ICE_MODULE  */
  YYSYMBOL_ICE_CLASS = 4,                  /* ICE_CLASS  */
  YYSYMBOL_ICE_INTERFACE = 5,              /* ICE_INTERFACE  */
  YYSYMBOL_ICE_EXCEPTION = 6,              /* ICE_EXCEPTION  */
  YYSYMBOL_ICE_STRUCT = 7,                 /* ICE_STRUCT  */
  YYSYMBOL_ICE_SEQUENCE = 8,               /* ICE_SEQUENCE  */
  YYSYMBOL_ICE_DICTIONARY = 9,             /* ICE_DICTIONARY  */
  YYSYMBOL_ICE_ENUM = 10,                  /* ICE_ENUM  */
  YYSYMBOL_ICE_OUT = 11,                   /* ICE_OUT  */
  YYSYMBOL_ICE_EXTENDS = 12,               /* ICE_EXTENDS  */
  YYSYMBOL_ICE_THROWS = 13,                /* ICE_THROWS  */
  YYSYMBOL_ICE_VOID = 14,                  /* ICE_VOID  */
  YYSYMBOL_ICE_BOOL = 15,                  /* ICE_BOOL  */
  YYSYMBOL_ICE_BYTE = 16,                  /* ICE_BYTE  */
  YYSYMBOL_ICE_SHORT = 17,                 /* ICE_SHORT  */
  YYSYMBOL_ICE_INT = 18,                   /* ICE_INT  */
  YYSYMBOL_ICE_LONG = 19,                  /* ICE_LONG  */
  YYSYMBOL_ICE_FLOAT = 20,                 /* ICE_FLOAT  */
  YYSYMBOL_ICE_DOUBLE = 21,                /* ICE_DOUBLE  */
  YYSYMBOL_ICE_STRING = 22,                /* ICE_STRING  */
  YYSYMBOL_ICE_OBJECT = 23,                /* ICE_OBJECT  */
  YYSYMBOL_ICE_CONST = 24,                 /* ICE_CONST  */
  YYSYMBOL_ICE_FALSE = 25,                 /* ICE_FALSE  */
  YYSYMBOL_ICE_TRUE = 26,                  /* ICE_TRUE  */
  YYSYMBOL_ICE_IDEMPOTENT = 27,            /* ICE_IDEMPOTENT  */
  YYSYMBOL_ICE_OPTIONAL = 28,              /* ICE_OPTIONAL  */
  YYSYMBOL_ICE_VALUE = 29,                 /* ICE_VALUE  */
  YYSYMBOL_ICE_STRING_LITERAL = 30,        /* ICE_STRING_LITERAL  */
  YYSYMBOL_ICE_INTEGER_LITERAL = 31,       /* ICE_INTEGER_LITERAL  */
  YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 32, /* ICE_FLOATING_POINT_LITERAL  */
  YYSYMBOL_ICE_IDENTIFIER = 33,            /* ICE_IDENTIFIER  */
  YYSYMBOL_ICE_SCOPED_IDENTIFIER = 34,     /* ICE_SCOPED_IDENTIFIER  */
  YYSYMBOL_ICE_METADATA_OPEN = 35,         /* ICE_METADATA_OPEN  */
  YYSYMBOL_ICE_METADATA_CLOSE = 36,        /* ICE_METADATA_CLOSE  */
  YYSYMBOL_ICE_FILE_METADATA_OPEN = 37,    /* ICE_FILE_METADATA_OPEN  */
  YYSYMBOL_ICE_FILE_METADATA_CLOSE = 38,   /* ICE_FILE_METADATA_CLOSE  */
  YYSYMBOL_ICE_IDENT_OPEN = 39,            /* ICE_IDENT_OPEN  */
  YYSYMBOL_ICE_KEYWORD_OPEN = 40,          /* ICE_KEYWORD_OPEN  */
  YYSYMBOL_ICE_OPTIONAL_OPEN = 41,         /* ICE_OPTIONAL_OPEN  */
  YYSYMBOL_BAD_CHAR = 42,                  /* BAD_CHAR  */
  YYSYMBOL_43_ = 43,                       /* ';'  */
  YYSYMBOL_44_ = 44,                       /* '{'  */
  YYSYMBOL_45_ = 45,                       /* '}'  */
  YYSYMBOL_46_ = 46,                       /* ')'  */
  YYSYMBOL_47_ = 47,                       /* ':'  */
  YYSYMBOL_48_ = 48,                       /* '='  */
  YYSYMBOL_49_ = 49,                       /* ','  */
  YYSYMBOL_50_ = 50,                       /* '<'  */
  YYSYMBOL_51_ = 51,                       /* '>'  */
  YYSYMBOL_52_ = 52,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 53,                  /* $accept  */
  YYSYMBOL_start = 54,                     /* start  */
  YYSYMBOL_opt_semicolon = 55,             /* opt_semicolon  */
  YYSYMBOL_file_metadata = 56,             /* file_metadata  */
  YYSYMBOL_metadata = 57,                  /* metadata  */
  YYSYMBOL_definitions = 58,               /* definitions  */
  YYSYMBOL_definition = 59,                /* definition  */
  YYSYMBOL_60_1 = 60,                      /* $@1  */
  YYSYMBOL_61_2 = 61,                      /* $@2  */
  YYSYMBOL_62_3 = 62,                      /* $@3  */
  YYSYMBOL_63_4 = 63,                      /* $@4  */
  YYSYMBOL_64_5 = 64,                      /* $@5  */
  YYSYMBOL_65_6 = 65,                      /* $@6  */
  YYSYMBOL_66_7 = 66,                      /* $@7  */
  YYSYMBOL_67_8 = 67,                      /* $@8  */
  YYSYMBOL_68_9 = 68,                      /* $@9  */
  YYSYMBOL_69_10 = 69,                     /* $@10  */
  YYSYMBOL_70_11 = 70,                     /* $@11  */
  YYSYMBOL_71_12 = 71,                     /* $@12  */
  YYSYMBOL_72_13 = 72,                     /* $@13  */
  YYSYMBOL_module_def = 73,                /* module_def  */
  YYSYMBOL_74_14 = 74,                     /* @14  */
  YYSYMBOL_75_15 = 75,                     /* @15  */
  YYSYMBOL_exception_id = 76,              /* exception_id  */
  YYSYMBOL_exception_decl = 77,            /* exception_decl  */
  YYSYMBOL_exception_def = 78,             /* exception_def  */
  YYSYMBOL_79_16 = 79,                     /* @16  */
  YYSYMBOL_exception_extends = 80,         /* exception_extends  */
  YYSYMBOL_type_id = 81,                   /* type_id  */
  YYSYMBOL_optional = 82,                  /* optional  */
  YYSYMBOL_optional_type_id = 83,          /* optional_type_id  */
  YYSYMBOL_struct_id = 84,                 /* struct_id  */
  YYSYMBOL_struct_decl = 85,               /* struct_decl  */
  YYSYMBOL_struct_def = 86,                /* struct_def  */
  YYSYMBOL_87_17 = 87,                     /* @17  */
  YYSYMBOL_class_name = 88,                /* class_name  */
  YYSYMBOL_class_id = 89,                  /* class_id  */
  YYSYMBOL_class_decl = 90,                /* class_decl  */
  YYSYMBOL_class_def = 91,                 /* class_def  */
  YYSYMBOL_92_18 = 92,                     /* @18  */
  YYSYMBOL_class_extends = 93,             /* class_extends  */
  YYSYMBOL_extends = 94,                   /* extends  */
  YYSYMBOL_data_members = 95,              /* data_members  */
  YYSYMBOL_data_member = 96,               /* data_member  */
  YYSYMBOL_return_type = 97,               /* return_type  */
  YYSYMBOL_operation_preamble = 98,        /* operation_preamble  */
  YYSYMBOL_operation = 99,                 /* operation  */
  YYSYMBOL_100_19 = 100,                   /* @19  */
  YYSYMBOL_101_20 = 101,                   /* @20  */
  YYSYMBOL_interface_id = 102,             /* interface_id  */
  YYSYMBOL_interface_decl = 103,           /* interface_decl  */
  YYSYMBOL_interface_def = 104,            /* interface_def  */
  YYSYMBOL_105_21 = 105,                   /* @21  */
  YYSYMBOL_interface_list = 106,           /* interface_list  */
  YYSYMBOL_interface_extends = 107,        /* interface_extends  */
  YYSYMBOL_operations = 108,               /* operations  */
  YYSYMBOL_exception_list = 109,           /* exception_list  */
  YYSYMBOL_exception = 110,                /* exception  */
  YYSYMBOL_sequence_def = 111,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 112,           /* dictionary_def  */
  YYSYMBOL_enum_id = 113,                  /* enum_id  */
  YYSYMBOL_enum_def = 114,                 /* enum_def  */
  YYSYMBOL_115_22 = 115,                   /* @22  */
  YYSYMBOL_116_23 = 116,                   /* @23  */
  YYSYMBOL_enumerator_list = 117,          /* enumerator_list  */
  YYSYMBOL_enumerator = 118,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 119,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 120,            /* out_qualifier  */
  YYSYMBOL_parameters = 121,               /* parameters  */
  YYSYMBOL_throws = 122,                   /* throws  */
  YYSYMBOL_scoped_name = 123,              /* scoped_name  */
  YYSYMBOL_builtin = 124,                  /* builtin  */
  YYSYMBOL_type = 125,                     /* type  */
  YYSYMBOL_string_literal = 126,           /* string_literal  */
  YYSYMBOL_metadata_list = 127,            /* metadata_list  */
  YYSYMBOL_const_initializer = 128,        /* const_initializer  */
  YYSYMBOL_const_def = 129,                /* const_def  */
  YYSYMBOL_keyword = 130                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 56 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 342 "src/Slice/Grammar.cpp"

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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   695

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  78
/* YYNRULES -- Number of rules.  */
#define YYNRULES  201
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  291

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
       2,    46,    52,     2,    49,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    47,    43,
      50,    48,    51,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    44,     2,    45,     2,     2,     2,     2,
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
       0,   182,   182,   190,   193,   201,   210,   214,   223,   231,
     240,   249,   248,   254,   253,   258,   263,   262,   268,   267,
     272,   277,   276,   282,   281,   286,   291,   290,   296,   295,
     300,   305,   304,   310,   309,   314,   319,   318,   323,   328,
     327,   333,   332,   337,   341,   351,   350,   381,   380,   464,
     468,   479,   490,   489,   515,   523,   532,   543,   561,   637,
     643,   654,   671,   684,   688,   699,   710,   709,   748,   752,
     763,   788,   876,   888,   901,   900,   934,   968,   977,   980,
     988,   997,  1000,  1004,  1012,  1042,  1073,  1095,  1121,  1136,
    1142,  1152,  1176,  1206,  1230,  1265,  1264,  1287,  1286,  1309,
    1313,  1324,  1338,  1337,  1371,  1406,  1441,  1446,  1456,  1460,
    1469,  1478,  1481,  1485,  1493,  1500,  1512,  1524,  1535,  1543,
    1557,  1567,  1583,  1587,  1599,  1598,  1630,  1629,  1647,  1659,
    1671,  1680,  1686,  1704,  1716,  1720,  1761,  1767,  1778,  1781,
    1797,  1813,  1825,  1837,  1848,  1864,  1868,  1877,  1880,  1888,
    1889,  1890,  1891,  1892,  1893,  1894,  1895,  1896,  1897,  1902,
    1906,  1911,  1942,  1978,  1984,  1992,  2002,  2017,  2026,  2035,
    2070,  2077,  2084,  2096,  2105,  2119,  2120,  2121,  2122,  2123,
    2124,  2125,  2126,  2127,  2128,  2129,  2130,  2131,  2132,  2133,
    2134,  2135,  2136,  2137,  2138,  2139,  2140,  2141,  2142,  2143,
    2144,  2145
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
  "\"end of file\"", "error", "\"invalid token\"", "ICE_MODULE",
  "ICE_CLASS", "ICE_INTERFACE", "ICE_EXCEPTION", "ICE_STRUCT",
  "ICE_SEQUENCE", "ICE_DICTIONARY", "ICE_ENUM", "ICE_OUT", "ICE_EXTENDS",
  "ICE_THROWS", "ICE_VOID", "ICE_BOOL", "ICE_BYTE", "ICE_SHORT", "ICE_INT",
  "ICE_LONG", "ICE_FLOAT", "ICE_DOUBLE", "ICE_STRING", "ICE_OBJECT",
  "ICE_CONST", "ICE_FALSE", "ICE_TRUE", "ICE_IDEMPOTENT", "ICE_OPTIONAL",
  "ICE_VALUE", "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENTIFIER", "ICE_SCOPED_IDENTIFIER",
  "ICE_METADATA_OPEN", "ICE_METADATA_CLOSE", "ICE_FILE_METADATA_OPEN",
  "ICE_FILE_METADATA_CLOSE", "ICE_IDENT_OPEN", "ICE_KEYWORD_OPEN",
  "ICE_OPTIONAL_OPEN", "BAD_CHAR", "';'", "'{'", "'}'", "')'", "':'",
  "'='", "','", "'<'", "'>'", "'*'", "$accept", "start", "opt_semicolon",
  "file_metadata", "metadata", "definitions", "definition", "$@1", "$@2",
  "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "$@12",
  "$@13", "module_def", "@14", "@15", "exception_id", "exception_decl",
  "exception_def", "@16", "exception_extends", "type_id", "optional",
  "optional_type_id", "struct_id", "struct_decl", "struct_def", "@17",
  "class_name", "class_id", "class_decl", "class_def", "@18",
  "class_extends", "extends", "data_members", "data_member", "return_type",
  "operation_preamble", "operation", "@19", "@20", "interface_id",
  "interface_decl", "interface_def", "@21", "interface_list",
  "interface_extends", "operations", "exception_list", "exception",
  "sequence_def", "dictionary_def", "enum_id", "enum_def", "@22", "@23",
  "enumerator_list", "enumerator", "enumerator_initializer",
  "out_qualifier", "parameters", "throws", "scoped_name", "builtin",
  "type", "string_literal", "metadata_list", "const_initializer",
  "const_def", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-237)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-139)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -237,   123,    39,  -237,    38,    38,  -237,   128,    38,  -237,
       0,   -21,    81,    27,   273,   342,   373,   404,    97,   100,
     435,   127,  -237,  -237,    28,   121,  -237,   119,   130,  -237,
      67,     3,   131,  -237,    68,   132,  -237,   133,   134,  -237,
    -237,   136,  -237,  -237,    38,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,   111,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,   127,   127,  -237,   126,  -237,
     661,   139,  -237,  -237,  -237,    33,   141,   139,   142,   144,
     139,  -237,    33,   146,   139,    93,  -237,   147,   139,   148,
     149,   151,   139,   153,  -237,   154,   155,   109,  -237,  -237,
     125,   661,   661,   -19,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,   145,  -237,   150,  -237,    -6,  -237,  -237,   156,
    -237,  -237,  -237,   192,  -237,  -237,   157,  -237,  -237,  -237,
    -237,  -237,  -237,   167,   160,  -237,  -237,  -237,  -237,   -19,
    -237,  -237,  -237,  -237,  -237,  -237,   168,   173,   466,   140,
    -237,  -237,   175,   135,   192,   162,   634,   179,   192,    93,
      72,   183,    83,   104,   497,   127,   181,   182,  -237,  -237,
     135,  -237,  -237,  -237,  -237,  -237,  -237,  -237,   185,   192,
    -237,   -11,  -237,   661,   186,   193,   528,  -237,   190,  -237,
     195,   579,   194,  -237,  -237,  -237,  -237,  -237,   661,   120,
     -19,  -237,  -237,  -237,   196,  -237,   197,  -237,   207,   135,
     192,  -237,  -237,  -237,    72,  -237,   607,   661,    57,    36,
     198,  -237,  -237,   200,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,    64,  -237,  -237,  -237,   199,  -237,   127,
      32,    72,   559,  -237,  -237,  -237,   634,  -237,   233,  -237,
    -237,  -237,   234,  -237,   528,   234,   127,   310,  -237,  -237,
    -237,   634,  -237,   203,  -237,  -237,  -237,   528,   310,  -237,
    -237
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      10,     0,     7,     1,     0,     0,     8,     0,   164,   166,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     126,     7,     9,    11,    51,    25,    26,    65,    30,    31,
      73,    77,    15,    16,   101,    20,    21,    35,    38,   124,
      39,    43,   163,     6,     0,     5,    44,    45,    47,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,    68,     0,    69,    99,
     100,    49,    50,    63,    64,     7,     7,   122,     0,   123,
       0,     4,    78,    79,    52,     0,     0,     4,     0,     0,
       4,    74,     0,     0,     4,     0,   102,     0,     4,     0,
       0,     0,     4,     0,   165,     0,     0,     0,   147,   148,
       0,     0,     0,     7,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   161,   160,     0,     3,    12,     0,
      54,    24,    27,     0,    29,    32,     0,    76,    14,    17,
     106,   107,   108,   105,     0,    19,    22,    34,    37,     7,
      40,    42,    10,    10,    70,    71,     0,     0,     0,     0,
     159,   162,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     7,     7,     0,     7,   131,   129,   133,   127,
       0,   171,   172,   170,   167,   168,   169,   174,     0,     0,
      60,     0,    62,     0,    84,    82,    87,    67,     0,   104,
       0,     0,     0,   125,    46,    48,   118,   119,     0,     0,
       7,   173,    53,    81,     0,    59,     0,    61,     0,     0,
       0,    56,    86,    75,     0,    90,     0,     0,     0,     0,
     112,    89,   103,     0,   134,   132,   135,   128,    57,    58,
      85,    80,   111,     0,    88,    91,    93,     0,   136,     7,
       0,     0,     0,    92,    94,    97,     0,    95,   137,   110,
     120,   121,   146,   139,   143,   146,     7,     0,    98,   141,
      96,     0,   145,   115,   116,   117,   140,   144,     0,   142,
     114
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -237,  -237,   -79,  -237,    -2,   -53,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,    43,  -198,
    -236,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,  -237,
    -237,   112,  -167,  -237,    12,  -237,  -237,  -237,  -237,  -237,
    -237,  -237,  -237,    70,  -237,  -220,   -35,  -237,  -237,  -237,
    -237,  -237,  -237,  -237,  -158,  -237,  -237,   -14,  -237,   -20,
     -71,  -237,   -78,     2,   251,  -181,  -237,   -12
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   138,     6,   176,     2,    22,    91,   103,   104,
     107,   108,    96,    97,    99,   100,   109,   110,   112,   113,
      23,   115,   116,    24,    25,    26,   139,    94,   202,   203,
     204,    27,    28,    29,    98,    30,    31,    32,    33,   146,
     101,    95,   177,   205,   238,   239,   240,   275,   272,    34,
      35,    36,   154,   152,   106,   212,   282,   283,    37,    38,
      39,    40,   111,    88,   169,   187,   245,   259,   260,   278,
     134,   135,   241,     9,    10,   197,    41,   285
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       7,   181,    78,    80,    82,    84,   120,   198,    89,   221,
      42,   208,   136,   237,   252,    92,     4,    45,   142,    90,
     224,   145,   118,   119,   140,   149,  -130,   172,    44,   156,
     273,   147,   223,   160,   153,   225,    43,   257,   237,    -2,
      92,   269,   173,   166,   167,   286,   114,   258,   250,    44,
      93,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,  -137,
      47,    48,   247,   251,  -137,  -137,   118,   119,     8,  -137,
    -137,  -137,   -55,   210,     4,    93,     5,  -137,   267,   -72,
      92,   268,  -138,   121,   122,  -138,    -7,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,   255,   256,   206,    -7,
      -7,    -7,   196,   263,   264,    -7,    -7,     4,   153,   182,
     183,   -72,  -109,    -7,   -72,    93,   150,  -113,     4,   196,
       5,   168,   151,     3,    46,   228,   118,   119,   214,    12,
     226,    13,    14,    15,    16,    17,    18,    19,    20,     4,
     243,     5,   117,   102,   118,   119,   105,    85,   246,   215,
      86,   244,    21,   118,   119,   164,   188,   168,   196,   254,
     191,   192,     4,   -66,   -23,   193,   194,   195,   118,   119,
     123,   165,   217,   -28,   -13,   -18,   -33,   -36,   211,   -41,
       7,     7,   137,   218,   141,   189,   143,   144,   274,   148,
     155,   157,   158,   175,   232,   159,   161,   170,   162,   163,
     174,   178,   171,   287,   180,   199,   284,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,   179,   284,   168,   184,
      -7,    -7,   185,   190,   207,    -7,    -7,     4,   213,   219,
     222,   220,   211,    -7,   229,   233,   230,   -83,   234,   242,
     231,   261,   248,   249,   258,   265,   227,   277,   253,   209,
     271,   262,   288,   290,   276,   280,    11,   266,     0,   211,
       0,     0,   279,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   281,   289,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,     0,     0,     0,    76,     0,     0,     0,
       0,     0,    77,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
       0,     0,     0,   118,   119,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,     0,     0,     0,    79,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,     0,     0,     0,    81,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,     0,     0,     0,    83,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,     0,     0,     0,    87,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,     0,     0,     0,   186,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,     0,     0,     0,
     216,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,     0,     0,
       0,   231,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,     0,
       0,     0,   270,   235,   124,   125,   126,   127,   128,   129,
     130,   131,   132,     0,     0,     0,   236,   200,   133,     0,
       0,     0,   118,   119,     0,     0,     0,     0,     0,     0,
     201,   235,   124,   125,   126,   127,   128,   129,   130,   131,
     132,     0,     0,     0,     0,   200,   133,     0,     0,     0,
     118,   119,     0,     0,     0,     0,     0,     0,   201,   124,
     125,   126,   127,   128,   129,   130,   131,   132,     0,     0,
       0,     0,   200,   133,     0,     0,     0,   118,   119,     0,
       0,     0,     0,     0,     0,   201,   124,   125,   126,   127,
     128,   129,   130,   131,   132,     0,     0,     0,     0,     0,
     133,     0,     0,     0,   118,   119
};

static const yytype_int16 yycheck[] =
{
       2,   159,    14,    15,    16,    17,    77,   174,    20,   190,
       8,   178,    90,   211,   234,    12,    35,    38,    97,    21,
      31,   100,    33,    34,    95,   104,    45,    33,    49,   108,
     266,   102,   199,   112,   105,    46,    36,     1,   236,     0,
      12,   261,    48,   121,   122,   281,    44,    11,   229,    49,
      47,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      33,    34,   220,   230,    28,    29,    33,    34,    30,    33,
      34,    35,    44,     1,    35,    47,    37,    41,    46,    12,
      12,    49,    46,    85,    86,    49,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    39,    40,   176,    27,
      28,    29,   173,    39,    40,    33,    34,    35,   179,   162,
     163,    44,    44,    41,    47,    47,    23,    45,    35,   190,
      37,   123,    29,     0,    43,   203,    33,    34,    45,     1,
     201,     3,     4,     5,     6,     7,     8,     9,    10,    35,
     218,    37,    31,    31,    33,    34,    34,    50,   219,    45,
      50,    31,    24,    33,    34,    46,   168,   159,   229,   237,
      25,    26,    35,    44,    43,    30,    31,    32,    33,    34,
      44,    46,   184,    43,    43,    43,    43,    43,   180,    43,
     182,   183,    43,   185,    43,    45,    44,    43,   266,    43,
      43,    43,    43,     1,   206,    44,    43,    52,    44,    44,
      44,    44,    52,   281,    44,    43,   277,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    49,   288,   220,    51,
      28,    29,    49,    48,    45,    33,    34,    35,    45,    48,
      45,    49,   234,    41,    48,    45,    43,    45,    43,    45,
      33,    43,    46,    46,    11,    46,   203,    13,   236,   179,
     262,    51,    49,   288,   268,   275,     5,   259,    -1,   261,
      -1,    -1,   274,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   276,   287,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    39,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    -1,    33,    34,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    -1,    33,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    -1,    33,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    -1,    -1,    -1,    33,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    -1,    33,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    -1,    33,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    -1,
      33,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      -1,    33,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    -1,    33,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    28,    29,    -1,
      -1,    -1,    33,    34,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,
      33,    34,    -1,    -1,    -1,    -1,    -1,    -1,    41,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    -1,    -1,
      -1,    -1,    28,    29,    -1,    -1,    -1,    33,    34,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    -1,    -1,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    33,    34
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    54,    58,     0,    35,    37,    56,    57,    30,   126,
     127,   127,     1,     3,     4,     5,     6,     7,     8,     9,
      10,    24,    59,    73,    76,    77,    78,    84,    85,    86,
      88,    89,    90,    91,   102,   103,   104,   111,   112,   113,
     114,   129,   126,    36,    49,    38,    43,    33,    34,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    33,    39,   130,    33,
     130,    33,   130,    33,   130,    50,    50,    33,   116,   130,
      57,    60,    12,    47,    80,    94,    65,    66,    87,    67,
      68,    93,    94,    61,    62,    94,   107,    63,    64,    69,
      70,   115,    71,    72,   126,    74,    75,    31,    33,    34,
     123,    57,    57,    44,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    29,   123,   124,   125,    43,    55,    79,
     123,    43,    55,    44,    43,    55,    92,   123,    43,    55,
      23,    29,   106,   123,   105,    43,    55,    43,    43,    44,
      55,    43,    44,    44,    46,    46,   125,   125,    57,   117,
      52,    52,    33,    48,    44,     1,    57,    95,    44,    49,
      44,   117,    58,    58,    51,    49,    33,   118,   130,    45,
      48,    25,    26,    30,    31,    32,   123,   128,    95,    43,
      28,    41,    81,    82,    83,    96,   125,    45,    95,   106,
       1,    57,   108,    45,    45,    45,    33,   130,    57,    48,
      49,   128,    45,    95,    31,    46,   123,    81,   125,    48,
      43,    33,   130,    45,    43,    14,    27,    82,    97,    98,
      99,   125,    45,   125,    31,   119,   123,   117,    46,    46,
     128,    95,   108,    97,   125,    39,    40,     1,    11,   120,
     121,    43,    51,    39,    40,    46,    57,    46,    49,   108,
      33,   130,   101,    83,   125,   100,   120,    13,   122,   130,
     122,    57,   109,   110,   123,   130,    83,   125,    49,   130,
     109
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    53,    54,    55,    55,    56,    57,    57,    58,    58,
      58,    60,    59,    61,    59,    59,    62,    59,    63,    59,
      59,    64,    59,    65,    59,    59,    66,    59,    67,    59,
      59,    68,    59,    69,    59,    59,    70,    59,    59,    71,
      59,    72,    59,    59,    59,    74,    73,    75,    73,    76,
      76,    77,    79,    78,    80,    80,    81,    82,    82,    82,
      82,    83,    83,    84,    84,    85,    87,    86,    88,    88,
      89,    89,    89,    90,    92,    91,    93,    93,    94,    94,
      95,    95,    95,    95,    96,    96,    96,    96,    97,    97,
      97,    98,    98,    98,    98,   100,    99,   101,    99,   102,
     102,   103,   105,   104,   106,   106,   106,   106,   107,   107,
     108,   108,   108,   108,   109,   109,   110,   110,   111,   111,
     112,   112,   113,   113,   115,   114,   116,   114,   117,   117,
     117,   118,   118,   118,   119,   119,   120,   120,   121,   121,
     121,   121,   121,   121,   121,   122,   122,   123,   123,   124,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   125,
     125,   125,   125,   126,   126,   127,   127,   128,   128,   128,
     128,   128,   128,   129,   129,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   130
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     0,     2,     3,
       0,     0,     3,     0,     3,     1,     0,     3,     0,     3,
       1,     0,     3,     0,     3,     1,     0,     3,     0,     3,
       1,     0,     3,     0,     3,     1,     0,     3,     1,     0,
       3,     0,     3,     1,     2,     0,     6,     0,     6,     2,
       2,     1,     0,     6,     2,     0,     2,     3,     3,     2,
       1,     2,     1,     2,     2,     1,     0,     5,     2,     2,
       4,     4,     1,     1,     0,     6,     2,     0,     1,     1,
       4,     3,     2,     0,     1,     3,     2,     1,     2,     1,
       1,     2,     3,     2,     3,     0,     5,     0,     5,     2,
       2,     1,     0,     6,     3,     1,     1,     1,     2,     0,
       4,     3,     2,     0,     3,     1,     1,     1,     6,     6,
       9,     9,     2,     2,     0,     5,     0,     5,     4,     2,
       0,     1,     3,     1,     1,     1,     1,     0,     0,     3,
       5,     4,     6,     3,     5,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     1,     2,     2,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     6,     5,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1
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


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

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

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
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
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
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

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

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

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc);
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
      yyerror_range[1] = yylloc;
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
  case 2: /* start: definitions  */
#line 183 "src/Slice/Grammar.y"
{
}
#line 1735 "src/Slice/Grammar.cpp"
    break;

  case 3: /* opt_semicolon: ';'  */
#line 191 "src/Slice/Grammar.y"
{
}
#line 1742 "src/Slice/Grammar.cpp"
    break;

  case 4: /* opt_semicolon: %empty  */
#line 194 "src/Slice/Grammar.y"
{
}
#line 1749 "src/Slice/Grammar.cpp"
    break;

  case 5: /* file_metadata: ICE_FILE_METADATA_OPEN metadata_list ICE_FILE_METADATA_CLOSE  */
#line 202 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1757 "src/Slice/Grammar.cpp"
    break;

  case 6: /* metadata: ICE_METADATA_OPEN metadata_list ICE_METADATA_CLOSE  */
#line 211 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1765 "src/Slice/Grammar.cpp"
    break;

  case 7: /* metadata: %empty  */
#line 215 "src/Slice/Grammar.y"
{
    yyval = make_shared<MetadataListTok>();
}
#line 1773 "src/Slice/Grammar.cpp"
    break;

  case 8: /* definitions: definitions file_metadata  */
#line 224 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[0]);
    if (!metadata->v.empty())
    {
        currentUnit->addFileMetadata(std::move(metadata->v));
    }
}
#line 1785 "src/Slice/Grammar.cpp"
    break;

  case 9: /* definitions: definitions metadata definition  */
#line 232 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[0]);
    if (contained && !metadata->v.empty())
    {
        contained->setMetadata(std::move(metadata->v));
    }
}
#line 1798 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: %empty  */
#line 241 "src/Slice/Grammar.y"
{
}
#line 1805 "src/Slice/Grammar.cpp"
    break;

  case 11: /* $@1: %empty  */
#line 249 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Module>(yyvsp[0]));
}
#line 1813 "src/Slice/Grammar.cpp"
    break;

  case 13: /* $@2: %empty  */
#line 254 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<ClassDecl>(yyvsp[0]));
}
#line 1821 "src/Slice/Grammar.cpp"
    break;

  case 15: /* definition: class_decl  */
#line 259 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after class forward declaration");
}
#line 1829 "src/Slice/Grammar.cpp"
    break;

  case 16: /* $@3: %empty  */
#line 263 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<ClassDef>(yyvsp[0]));
}
#line 1837 "src/Slice/Grammar.cpp"
    break;

  case 18: /* $@4: %empty  */
#line 268 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<InterfaceDecl>(yyvsp[0]));
}
#line 1845 "src/Slice/Grammar.cpp"
    break;

  case 20: /* definition: interface_decl  */
#line 273 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after interface forward declaration");
}
#line 1853 "src/Slice/Grammar.cpp"
    break;

  case 21: /* $@5: %empty  */
#line 277 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<InterfaceDef>(yyvsp[0]));
}
#line 1861 "src/Slice/Grammar.cpp"
    break;

  case 23: /* $@6: %empty  */
#line 282 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr);
}
#line 1869 "src/Slice/Grammar.cpp"
    break;

  case 25: /* definition: exception_decl  */
#line 287 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after exception forward declaration");
}
#line 1877 "src/Slice/Grammar.cpp"
    break;

  case 26: /* $@7: %empty  */
#line 291 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Exception>(yyvsp[0]));
}
#line 1885 "src/Slice/Grammar.cpp"
    break;

  case 28: /* $@8: %empty  */
#line 296 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr);
}
#line 1893 "src/Slice/Grammar.cpp"
    break;

  case 30: /* definition: struct_decl  */
#line 301 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after struct forward declaration");
}
#line 1901 "src/Slice/Grammar.cpp"
    break;

  case 31: /* $@9: %empty  */
#line 305 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Struct>(yyvsp[0]));
}
#line 1909 "src/Slice/Grammar.cpp"
    break;

  case 33: /* $@10: %empty  */
#line 310 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Sequence>(yyvsp[0]));
}
#line 1917 "src/Slice/Grammar.cpp"
    break;

  case 35: /* definition: sequence_def  */
#line 315 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after sequence definition");
}
#line 1925 "src/Slice/Grammar.cpp"
    break;

  case 36: /* $@11: %empty  */
#line 319 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Dictionary>(yyvsp[0]));
}
#line 1933 "src/Slice/Grammar.cpp"
    break;

  case 38: /* definition: dictionary_def  */
#line 324 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after dictionary definition");
}
#line 1941 "src/Slice/Grammar.cpp"
    break;

  case 39: /* $@12: %empty  */
#line 328 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Enum>(yyvsp[0]));
}
#line 1949 "src/Slice/Grammar.cpp"
    break;

  case 41: /* $@13: %empty  */
#line 333 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Const>(yyvsp[0]));
}
#line 1957 "src/Slice/Grammar.cpp"
    break;

  case 43: /* definition: const_def  */
#line 338 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after const definition");
}
#line 1965 "src/Slice/Grammar.cpp"
    break;

  case 44: /* definition: error ';'  */
#line 342 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 1973 "src/Slice/Grammar.cpp"
    break;

  case 45: /* @14: %empty  */
#line 351 "src/Slice/Grammar.y"
{
    currentUnit->setSeenDefinition();

    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ModulePtr module = cont->createModule(ident->v);
    if (module)
    {
        cont->checkIntroduced(ident->v, module);
        currentUnit->pushContainer(module);
        yyval = module;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 1995 "src/Slice/Grammar.cpp"
    break;

  case 46: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
#line 369 "src/Slice/Grammar.y"
{
    if (yyvsp[-3])
    {
        currentUnit->popContainer();
        yyval = yyvsp[-3];
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2011 "src/Slice/Grammar.cpp"
    break;

  case 47: /* @15: %empty  */
#line 381 "src/Slice/Grammar.y"
{
    currentUnit->setSeenDefinition();

    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);

    // Reject scoped identifiers starting with "::". This is generally indicates global scope, but is invalid here.
    size_t startPos = 0;
    if (ident->v.find("::") == 0)
    {
        currentUnit->error("illegal identifier: module identifiers cannot start with '::' prefix");
        startPos += 2; // Skip the leading "::".
    }

    // Split the scoped-identifier token into separate module names.
    size_t endPos;
    vector<string> modules;
    while ((endPos = ident->v.find("::", startPos)) != string::npos)
    {
        modules.push_back(ident->v.substr(startPos, (endPos - startPos)));
        startPos = endPos + 2; // Skip the "::" separator.
    }
    modules.push_back(ident->v.substr(startPos));

    // Create the nested modules.
    ContainerPtr cont = currentUnit->currentContainer();
    for (size_t i = 0; i < modules.size(); i++)
    {
        const auto currentModuleName = modules[i];
        ModulePtr module = cont->createModule(currentModuleName);
        if (module)
        {
            cont->checkIntroduced(currentModuleName, module);
            currentUnit->pushContainer(module);
            yyval = cont = module;
        }
        else
        {
            // If an error occurs while creating one of the modules, we have to stop. But, to eagerly report as many
            // errors as possible, we still 'create' any remaining modules, which will run _some_ validation on them.
            for (size_t j = (i + 1); j < modules.size(); j++)
            {
                cont->createModule(modules[j]); // Dummy
            }

            // Then we roll back the chain, i.e. pop the successfully-created-modules off the container stack.
            for (; i > 0; i--)
            {
                currentUnit->popContainer();
            }
            yyval = nullptr;
            break;
        }
    }
}
#line 2070 "src/Slice/Grammar.cpp"
    break;

  case 48: /* module_def: ICE_MODULE ICE_SCOPED_IDENTIFIER @15 '{' definitions '}'  */
#line 436 "src/Slice/Grammar.y"
{
    if (yyvsp[-3])
    {
        // We need to pop '(N+1)' modules off the container stack, to navigate out of the nested module.
        // Where `N` is the number of scope separators ("::").
        size_t startPos = 0;
        auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-4]);
        while ((startPos = ident->v.find("::", startPos)) != string::npos)
        {
            currentUnit->popContainer();
            startPos += 2; // Skip the "::" separator.
        }

        // Set the 'return value' to the outer-most module, before we pop it off the stack.
        // Whichever module we return, is the one that metadata will be applied to.
        yyval = currentUnit->currentContainer();
        currentUnit->popContainer();
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2098 "src/Slice/Grammar.cpp"
    break;

  case 49: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 465 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2106 "src/Slice/Grammar.cpp"
    break;

  case 50: /* exception_id: ICE_EXCEPTION keyword  */
#line 469 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2116 "src/Slice/Grammar.cpp"
    break;

  case 51: /* exception_decl: exception_id  */
#line 480 "src/Slice/Grammar.y"
{
    currentUnit->error("exceptions cannot be forward declared");
    yyval = nullptr;
}
#line 2125 "src/Slice/Grammar.cpp"
    break;

  case 52: /* @16: %empty  */
#line 490 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    auto base = dynamic_pointer_cast<Exception>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base);
    if (ex)
    {
        cont->checkIntroduced(ident->v, ex);
        currentUnit->pushContainer(ex);
    }
    yyval = ex;
}
#line 2142 "src/Slice/Grammar.cpp"
    break;

  case 53: /* exception_def: exception_id exception_extends @16 '{' data_members '}'  */
#line 503 "src/Slice/Grammar.y"
{
    if (yyvsp[-3])
    {
        currentUnit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2154 "src/Slice/Grammar.cpp"
    break;

  case 54: /* exception_extends: extends scoped_name  */
#line 516 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v, true);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2166 "src/Slice/Grammar.cpp"
    break;

  case 55: /* exception_extends: %empty  */
#line 524 "src/Slice/Grammar.y"
{
    yyval = nullptr;
}
#line 2174 "src/Slice/Grammar.cpp"
    break;

  case 56: /* type_id: type ICE_IDENTIFIER  */
#line 533 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    yyval = make_shared<TypeStringTok>(type, ident->v);
}
#line 2184 "src/Slice/Grammar.cpp"
    break;

  case 57: /* optional: ICE_OPTIONAL_OPEN ICE_INTEGER_LITERAL ')'  */
#line 544 "src/Slice/Grammar.y"
{
    auto i = dynamic_pointer_cast<IntegerTok>(yyvsp[-1]);

    int tag;
    if (i->v < 0 || i->v > std::numeric_limits<int32_t>::max())
    {
        currentUnit->error("tag is out of range");
        tag = -1;
    }
    else
    {
        tag = static_cast<int>(i->v);
    }

    auto m = make_shared<OptionalDefTok>(tag);
    yyval = m;
}
#line 2206 "src/Slice/Grammar.cpp"
    break;

  case 58: /* optional: ICE_OPTIONAL_OPEN scoped_name ')'  */
#line 562 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    ContainerPtr cont = currentUnit->currentContainer();
    assert(cont);
    ContainedList cl = cont->lookupContained(scoped->v, false);
    if (cl.empty())
    {
        EnumeratorList enumerators = cont->enumerators(scoped->v);
        if (enumerators.size() == 1)
        {
            // Found
            cl.push_back(enumerators.front());
            scoped->v = enumerators.front()->scoped();
        }
        else if (enumerators.size() > 1)
        {
            ostringstream os;
            os << "enumerator `" << scoped->v << "' could designate";
            bool first = true;
            for (const auto& p : enumerators)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    os << " or";
                }

                os << " `" << p->scoped() << "'";
            }
            currentUnit->error(os.str());
        }
        else
        {
            currentUnit->error(string("`") + scoped->v + "' is not defined");
        }
    }

    if (cl.empty())
    {
        YYERROR; // Can't continue, jump to next yyerrok
    }
    cont->checkIntroduced(scoped->v);

    int tag = -1;
    auto enumerator = dynamic_pointer_cast<Enumerator>(cl.front());
    auto constant = dynamic_pointer_cast<Const>(cl.front());
    if (constant)
    {
        auto b = dynamic_pointer_cast<Builtin>(constant->type());
        if (b && b->isIntegralType())
        {
            int64_t l = std::stoll(constant->value(), nullptr, 0);
            if (l < 0 || l > std::numeric_limits<int32_t>::max())
            {
                currentUnit->error("tag is out of range");
            }
            tag = static_cast<int>(l);
        }
    }
    else if (enumerator)
    {
        tag = enumerator->value();
    }

    if (tag < 0)
    {
        currentUnit->error("invalid tag `" + scoped->v + "'");
    }

    auto m = make_shared<OptionalDefTok>(tag);
    yyval = m;
}
#line 2286 "src/Slice/Grammar.cpp"
    break;

  case 59: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 638 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<OptionalDefTok>(-1); // Dummy
    yyval = m;
}
#line 2296 "src/Slice/Grammar.cpp"
    break;

  case 60: /* optional: ICE_OPTIONAL  */
#line 644 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<OptionalDefTok>(-1); // Dummy
    yyval = m;
}
#line 2306 "src/Slice/Grammar.cpp"
    break;

  case 61: /* optional_type_id: optional type_id  */
#line 655 "src/Slice/Grammar.y"
{
    auto m = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
    auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);
    m->type = ts->type;
    m->name = ts->name;

    // It's safe to perform this check in the parser, since we already have enough information to know whether a type
    // can be optional. This is because the only types that can be forward declared (classes/interfaces) have constant
    // values for `usesClasses` (true/false respectively).
    if (m->type->usesClasses())
    {
        currentUnit->error("types that use classes cannot be marked with 'optional'");
    }

    yyval = m;
}
#line 2327 "src/Slice/Grammar.cpp"
    break;

  case 62: /* optional_type_id: type_id  */
#line 672 "src/Slice/Grammar.y"
{
    auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);
    auto m = make_shared<OptionalDefTok>(-1);
    m->type = ts->type;
    m->name = ts->name;
    yyval = m;
}
#line 2339 "src/Slice/Grammar.cpp"
    break;

  case 63: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 685 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2347 "src/Slice/Grammar.cpp"
    break;

  case 64: /* struct_id: ICE_STRUCT keyword  */
#line 689 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2357 "src/Slice/Grammar.cpp"
    break;

  case 65: /* struct_decl: struct_id  */
#line 700 "src/Slice/Grammar.y"
{
    currentUnit->error("structs cannot be forward declared");
    yyval = nullptr; // Dummy
}
#line 2366 "src/Slice/Grammar.cpp"
    break;

  case 66: /* @17: %empty  */
#line 710 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    StructPtr st = cont->createStruct(ident->v);
    if (st)
    {
        cont->checkIntroduced(ident->v, st);
        currentUnit->pushContainer(st);
    }
    else
    {
        st = cont->createStruct(Ice::generateUUID()); // Dummy
        assert(st);
        currentUnit->pushContainer(st);
    }
    yyval = st;
}
#line 2388 "src/Slice/Grammar.cpp"
    break;

  case 67: /* struct_def: struct_id @17 '{' data_members '}'  */
#line 728 "src/Slice/Grammar.y"
{
    if (yyvsp[-3])
    {
        currentUnit->popContainer();
    }
    yyval = yyvsp[-3];

    // Empty structures are not allowed
    auto st = dynamic_pointer_cast<Struct>(yyval);
    assert(st);
    if (st->dataMembers().empty())
    {
        currentUnit->error("struct `" + st->name() + "' must have at least one member"); // $$ is a dummy
    }
}
#line 2408 "src/Slice/Grammar.cpp"
    break;

  case 68: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 749 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2416 "src/Slice/Grammar.cpp"
    break;

  case 69: /* class_name: ICE_CLASS keyword  */
#line 753 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2426 "src/Slice/Grammar.cpp"
    break;

  case 70: /* class_id: ICE_CLASS ICE_IDENT_OPEN ICE_INTEGER_LITERAL ')'  */
#line 764 "src/Slice/Grammar.y"
{
    int64_t id = dynamic_pointer_cast<IntegerTok>(yyvsp[-1])->v;
    if (id < 0)
    {
        currentUnit->error("invalid compact id for class: id must be a positive integer");
    }
    else if (id > std::numeric_limits<int32_t>::max())
    {
        currentUnit->error("invalid compact id for class: value is out of range");
    }
    else
    {
        string typeId = currentUnit->getTypeId(static_cast<int>(id));
        if (!typeId.empty())
        {
            currentUnit->error("invalid compact id for class: already assigned to class `" + typeId + "'");
        }
    }

    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[-2])->v;
    classId->t = static_cast<int>(id);
    yyval = classId;
}
#line 2455 "src/Slice/Grammar.cpp"
    break;

  case 71: /* class_id: ICE_CLASS ICE_IDENT_OPEN scoped_name ')'  */
#line 789 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[-1]);

    ContainerPtr cont = currentUnit->currentContainer();
    assert(cont);
    ContainedList cl = cont->lookupContained(scoped->v, false);
    if (cl.empty())
    {
        EnumeratorList enumerators = cont->enumerators(scoped->v);
        if (enumerators.size() == 1)
        {
            // Found
            cl.push_back(enumerators.front());
            scoped->v = enumerators.front()->scoped();
        }
        else if (enumerators.size() > 1)
        {
            ostringstream os;
            os << "enumerator `" << scoped->v << "' could designate";
            bool first = true;
            for (const auto& p : enumerators)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    os << " or";
                }

                os << " `" << p->scoped() << "'";
            }
            currentUnit->error(os.str());
        }
        else
        {
            currentUnit->error(string("`") + scoped->v + "' is not defined");
        }
    }

    if (cl.empty())
    {
        YYERROR; // Can't continue, jump to next yyerrok
    }
    cont->checkIntroduced(scoped->v);

    int id = -1;
    auto enumerator = dynamic_pointer_cast<Enumerator>(cl.front());
    auto constant = dynamic_pointer_cast<Const>(cl.front());
    if (constant)
    {
        auto b = dynamic_pointer_cast<Builtin>(constant->type());
        if (b && b->isIntegralType())
        {
            int64_t l = std::stoll(constant->value(), nullptr, 0);
            if (l < 0 || l > std::numeric_limits<int32_t>::max())
            {
                currentUnit->error("compact id for class is out of range");
            }
            id = static_cast<int>(l);
        }
    }
    else if (enumerator)
    {
        id = enumerator->value();
    }

    if (id < 0)
    {
        currentUnit->error("invalid compact id for class: id must be a positive integer");
    }
    else
    {
        string typeId = currentUnit->getTypeId(id);
        if (!typeId.empty())
        {
            currentUnit->error("invalid compact id for class: already assigned to class `" + typeId + "'");
        }
    }

    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[-2])->v;
    classId->t = id;
    yyval = classId;

}
#line 2547 "src/Slice/Grammar.cpp"
    break;

  case 72: /* class_id: class_name  */
#line 877 "src/Slice/Grammar.y"
{
    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2558 "src/Slice/Grammar.cpp"
    break;

  case 73: /* class_decl: class_name  */
#line 889 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2569 "src/Slice/Grammar.cpp"
    break;

  case 74: /* @18: %empty  */
#line 901 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<ClassIdTok>(yyvsp[-1]);
    ContainerPtr cont = currentUnit->currentContainer();
    auto base = dynamic_pointer_cast<ClassDef>(yyvsp[0]);
    ClassDefPtr cl = cont->createClassDef(ident->v, ident->t, base);
    if (cl)
    {
        cont->checkIntroduced(ident->v, cl);
        currentUnit->pushContainer(cl);
        yyval = cl;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2590 "src/Slice/Grammar.cpp"
    break;

  case 75: /* class_def: class_id class_extends @18 '{' data_members '}'  */
#line 918 "src/Slice/Grammar.y"
{
    if (yyvsp[-3])
    {
        currentUnit->popContainer();
        yyval = yyvsp[-3];
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2606 "src/Slice/Grammar.cpp"
    break;

  case 76: /* class_extends: extends scoped_name  */
#line 935 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    yyval = nullptr;
    if (!types.empty())
    {
        auto cl = dynamic_pointer_cast<ClassDecl>(types.front());
        if (!cl)
        {
            string msg = "`";
            msg += scoped->v;
            msg += "' is not a class";
            currentUnit->error(msg);
        }
        else
        {
            ClassDefPtr def = cl->definition();
            if (!def)
            {
                string msg = "`";
                msg += scoped->v;
                msg += "' has been declared but not defined";
                currentUnit->error(msg);
            }
            else
            {
                cont->checkIntroduced(scoped->v);
                yyval = def;
            }
        }
    }
}
#line 2644 "src/Slice/Grammar.cpp"
    break;

  case 77: /* class_extends: %empty  */
#line 969 "src/Slice/Grammar.y"
{
    yyval = nullptr;
}
#line 2652 "src/Slice/Grammar.cpp"
    break;

  case 78: /* extends: ICE_EXTENDS  */
#line 978 "src/Slice/Grammar.y"
{
}
#line 2659 "src/Slice/Grammar.cpp"
    break;

  case 79: /* extends: ':'  */
#line 981 "src/Slice/Grammar.y"
{
}
#line 2666 "src/Slice/Grammar.cpp"
    break;

  case 80: /* data_members: metadata data_member ';' data_members  */
#line 989 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
    if (contained && !metadata->v.empty())
    {
        contained->setMetadata(std::move(metadata->v));
    }
}
#line 2679 "src/Slice/Grammar.cpp"
    break;

  case 81: /* data_members: error ';' data_members  */
#line 998 "src/Slice/Grammar.y"
{
}
#line 2686 "src/Slice/Grammar.cpp"
    break;

  case 82: /* data_members: metadata data_member  */
#line 1001 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after definition");
}
#line 2694 "src/Slice/Grammar.cpp"
    break;

  case 83: /* data_members: %empty  */
#line 1005 "src/Slice/Grammar.y"
{
}
#line 2701 "src/Slice/Grammar.cpp"
    break;

  case 84: /* data_member: optional_type_id  */
#line 1013 "src/Slice/Grammar.y"
{
    auto def = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    DataMemberPtr dm;
    if (cl)
    {
        dm = cl->createDataMember(def->name, def->type, def->isOptional, def->tag, 0, "");
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        if (def->isOptional)
        {
            currentUnit->error("optional data members are not supported in structs");
            dm = st->createDataMember(def->name, def->type, false, 0, 0, ""); // Dummy
        }
        else
        {
            dm = st->createDataMember(def->name, def->type, false, -1, 0, "");
        }
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        dm = ex->createDataMember(def->name, def->type, def->isOptional, def->tag, 0, "");
    }
    currentUnit->currentContainer()->checkIntroduced(def->name, dm);
    yyval = dm;
}
#line 2735 "src/Slice/Grammar.cpp"
    break;

  case 85: /* data_member: optional_type_id '=' const_initializer  */
#line 1043 "src/Slice/Grammar.y"
{
    auto def = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    DataMemberPtr dm;
    if (cl)
    {
        dm = cl->createDataMember(def->name, def->type, def->isOptional, def->tag, value->v, value->valueAsString);
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        if (def->isOptional)
        {
            currentUnit->error("optional data members are not supported in structs");
            dm = st->createDataMember(def->name, def->type, false, 0, 0, ""); // Dummy
        }
        else
        {
            dm = st->createDataMember(def->name, def->type, false, -1, value->v, value->valueAsString);
        }
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        dm = ex->createDataMember(def->name, def->type, def->isOptional, def->tag, value->v, value->valueAsString);
    }
    currentUnit->currentContainer()->checkIntroduced(def->name, dm);
    yyval = dm;
}
#line 2770 "src/Slice/Grammar.cpp"
    break;

  case 86: /* data_member: type keyword  */
#line 1074 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    if (cl)
    {
        yyval = cl->createDataMember(name, type, false, 0, 0, ""); // Dummy
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        yyval = st->createDataMember(name, type, false, 0, 0, ""); // Dummy
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        yyval = ex->createDataMember(name, type, false, 0, 0, ""); // Dummy
    }
    assert(yyval);
    currentUnit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 2796 "src/Slice/Grammar.cpp"
    break;

  case 87: /* data_member: type  */
#line 1096 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    if (cl)
    {
        yyval = cl->createDataMember(Ice::generateUUID(), type, false, 0, 0, ""); // Dummy
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        yyval = st->createDataMember(Ice::generateUUID(), type, false, 0, 0, ""); // Dummy
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        yyval = ex->createDataMember(Ice::generateUUID(), type, false, 0, 0, ""); // Dummy
    }
    assert(yyval);
    currentUnit->error("missing data member name");
}
#line 2821 "src/Slice/Grammar.cpp"
    break;

  case 88: /* return_type: optional type  */
#line 1122 "src/Slice/Grammar.y"
{
    auto m = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
    m->type = dynamic_pointer_cast<Type>(yyvsp[0]);

    // It's safe to perform this check in the parser, since we already have enough information to know whether a type
    // can be optional. This is because the only types that can be forward declared (classes/interfaces) have constant
    // values for `usesClasses` (true/false respectively).
    if (m->type->usesClasses())
    {
        currentUnit->error("types that use classes cannot be marked with 'optional'");
    }

    yyval = m;
}
#line 2840 "src/Slice/Grammar.cpp"
    break;

  case 89: /* return_type: type  */
#line 1137 "src/Slice/Grammar.y"
{
    auto m = make_shared<OptionalDefTok>(-1);
    m->type = dynamic_pointer_cast<Type>(yyvsp[0]);
    yyval = m;
}
#line 2850 "src/Slice/Grammar.cpp"
    break;

  case 90: /* return_type: ICE_VOID  */
#line 1143 "src/Slice/Grammar.y"
{
    auto m = make_shared<OptionalDefTok>(-1);
    yyval = m;
}
#line 2859 "src/Slice/Grammar.cpp"
    break;

  case 91: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1153 "src/Slice/Grammar.y"
{
    auto returnType = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
    if (interface)
    {
        OperationPtr op = interface->createOperation(name, returnType->type, returnType->isOptional, returnType->tag);
        if (op)
        {
            interface->checkIntroduced(name, op);
            currentUnit->pushContainer(op);
            yyval = op;
        }
        else
        {
            yyval = nullptr;
        }
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2887 "src/Slice/Grammar.cpp"
    break;

  case 92: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1177 "src/Slice/Grammar.y"
{
    auto returnType = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
    if (interface)
    {
        OperationPtr op = interface->createOperation(
            name,
            returnType->type,
            returnType->isOptional,
            returnType->tag,
            Operation::Idempotent);

        if (op)
        {
            interface->checkIntroduced(name, op);
            currentUnit->pushContainer(op);
            yyval = op;
        }
        else
        {
            yyval = nullptr;
        }
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2921 "src/Slice/Grammar.cpp"
    break;

  case 93: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1207 "src/Slice/Grammar.y"
{
    auto returnType = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
    if (interface)
    {
        OperationPtr op = interface->createOperation(name, returnType->type, returnType->isOptional, returnType->tag);
        if (op)
        {
            currentUnit->pushContainer(op);
            currentUnit->error("keyword `" + name + "' cannot be used as operation name");
            yyval = op; // Dummy
        }
        else
        {
            yyval = nullptr;
        }
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2949 "src/Slice/Grammar.cpp"
    break;

  case 94: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1231 "src/Slice/Grammar.y"
{
    auto returnType = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
    if (interface)
    {
        OperationPtr op = interface->createOperation(
            name,
            returnType->type,
            returnType->isOptional,
            returnType->tag,
            Operation::Idempotent);
        if (op)
        {
            currentUnit->pushContainer(op);
            currentUnit->error("keyword `" + name + "' cannot be used as operation name");
            yyval = op; // Dummy
        }
        else
        {
            return 0;
        }
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2982 "src/Slice/Grammar.cpp"
    break;

  case 95: /* @19: %empty  */
#line 1265 "src/Slice/Grammar.y"
{
    if (yyvsp[-2])
    {
        currentUnit->popContainer();
        yyval = yyvsp[-2];
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2998 "src/Slice/Grammar.cpp"
    break;

  case 96: /* operation: operation_preamble parameters ')' @19 throws  */
#line 1277 "src/Slice/Grammar.y"
{
    auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
    auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    assert(el);
    if (op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3012 "src/Slice/Grammar.cpp"
    break;

  case 97: /* @20: %empty  */
#line 1287 "src/Slice/Grammar.y"
{
    if (yyvsp[-2])
    {
        currentUnit->popContainer();
    }
    yyerrok;
}
#line 3024 "src/Slice/Grammar.cpp"
    break;

  case 98: /* operation: operation_preamble error ')' @20 throws  */
#line 1295 "src/Slice/Grammar.y"
{
    auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
    auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    assert(el);
    if (op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3038 "src/Slice/Grammar.cpp"
    break;

  case 99: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1310 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3046 "src/Slice/Grammar.cpp"
    break;

  case 100: /* interface_id: ICE_INTERFACE keyword  */
#line 1314 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3056 "src/Slice/Grammar.cpp"
    break;

  case 101: /* interface_decl: interface_id  */
#line 1325 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto cont = currentUnit->currentContainer();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3068 "src/Slice/Grammar.cpp"
    break;

  case 102: /* @21: %empty  */
#line 1338 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    ContainerPtr cont = currentUnit->currentContainer();
    auto bases = dynamic_pointer_cast<InterfaceListTok>(yyvsp[0]);
    InterfaceDefPtr interface = cont->createInterfaceDef(ident->v, bases->v);
    if (interface)
    {
        cont->checkIntroduced(ident->v, interface);
        currentUnit->pushContainer(interface);
        yyval = interface;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 3089 "src/Slice/Grammar.cpp"
    break;

  case 103: /* interface_def: interface_id interface_extends @21 '{' operations '}'  */
#line 1355 "src/Slice/Grammar.y"
{
    if (yyvsp[-3])
    {
        currentUnit->popContainer();
        yyval = yyvsp[-3];
    }
    else
    {
        yyval = nullptr;
    }
}
#line 3105 "src/Slice/Grammar.cpp"
    break;

  case 104: /* interface_list: scoped_name ',' interface_list  */
#line 1372 "src/Slice/Grammar.y"
{
    auto intfs = dynamic_pointer_cast<InterfaceListTok>(yyvsp[0]);
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if (!types.empty())
    {
        auto interface = dynamic_pointer_cast<InterfaceDecl>(types.front());
        if (!interface)
        {
            string msg = "`";
            msg += scoped->v;
            msg += "' is not an interface";
            currentUnit->error(msg);
        }
        else
        {
            InterfaceDefPtr def = interface->definition();
            if (!def)
            {
                string msg = "`";
                msg += scoped->v;
                msg += "' has been declared but not defined";
                currentUnit->error(msg);
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
#line 3144 "src/Slice/Grammar.cpp"
    break;

  case 105: /* interface_list: scoped_name  */
#line 1407 "src/Slice/Grammar.y"
{
    auto intfs = make_shared<InterfaceListTok>();
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if (!types.empty())
    {
        auto interface = dynamic_pointer_cast<InterfaceDecl>(types.front());
        if (!interface)
        {
            string msg = "`";
            msg += scoped->v;
            msg += "' is not an interface";
            currentUnit->error(msg); // $$ is a dummy
        }
        else
        {
            InterfaceDefPtr def = interface->definition();
            if (!def)
            {
                string msg = "`";
                msg += scoped->v;
                msg += "' has been declared but not defined";
                currentUnit->error(msg); // $$ is a dummy
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
#line 3183 "src/Slice/Grammar.cpp"
    break;

  case 106: /* interface_list: ICE_OBJECT  */
#line 1442 "src/Slice/Grammar.y"
{
    currentUnit->error("illegal inheritance from type Object");
    yyval = make_shared<InterfaceListTok>(); // Dummy
}
#line 3192 "src/Slice/Grammar.cpp"
    break;

  case 107: /* interface_list: ICE_VALUE  */
#line 1447 "src/Slice/Grammar.y"
{
    currentUnit->error("illegal inheritance from type Value");
    yyval = make_shared<InterfaceListTok>(); // Dummy
}
#line 3201 "src/Slice/Grammar.cpp"
    break;

  case 108: /* interface_extends: extends interface_list  */
#line 1457 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3209 "src/Slice/Grammar.cpp"
    break;

  case 109: /* interface_extends: %empty  */
#line 1461 "src/Slice/Grammar.y"
{
    yyval = make_shared<InterfaceListTok>();
}
#line 3217 "src/Slice/Grammar.cpp"
    break;

  case 110: /* operations: metadata operation ';' operations  */
#line 1470 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
    if (contained && !metadata->v.empty())
    {
        contained->setMetadata(std::move(metadata->v));
    }
}
#line 3230 "src/Slice/Grammar.cpp"
    break;

  case 111: /* operations: error ';' operations  */
#line 1479 "src/Slice/Grammar.y"
{
}
#line 3237 "src/Slice/Grammar.cpp"
    break;

  case 112: /* operations: metadata operation  */
#line 1482 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after definition");
}
#line 3245 "src/Slice/Grammar.cpp"
    break;

  case 113: /* operations: %empty  */
#line 1486 "src/Slice/Grammar.y"
{
}
#line 3252 "src/Slice/Grammar.cpp"
    break;

  case 114: /* exception_list: exception ',' exception_list  */
#line 1494 "src/Slice/Grammar.y"
{
    auto exception = dynamic_pointer_cast<Exception>(yyvsp[-2]);
    auto exceptionList = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3263 "src/Slice/Grammar.cpp"
    break;

  case 115: /* exception_list: exception  */
#line 1501 "src/Slice/Grammar.y"
{
    auto exception = dynamic_pointer_cast<Exception>(yyvsp[0]);
    auto exceptionList = make_shared<ExceptionListTok>();
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3274 "src/Slice/Grammar.cpp"
    break;

  case 116: /* exception: scoped_name  */
#line 1513 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v, true);
    if (!exception)
    {
        exception = cont->createException(Ice::generateUUID(), 0, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    yyval = exception;
}
#line 3290 "src/Slice/Grammar.cpp"
    break;

  case 117: /* exception: keyword  */
#line 1525 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = currentUnit->currentContainer()->createException(Ice::generateUUID(), 0, Dummy); // Dummy
}
#line 3300 "src/Slice/Grammar.cpp"
    break;

  case 118: /* sequence_def: ICE_SEQUENCE '<' metadata type '>' ICE_IDENTIFIER  */
#line 1536 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createSequence(ident->v, type, std::move(metadata->v));
}
#line 3312 "src/Slice/Grammar.cpp"
    break;

  case 119: /* sequence_def: ICE_SEQUENCE '<' metadata type '>' keyword  */
#line 1544 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createSequence(ident->v, type, std::move(metadata->v)); // Dummy
    currentUnit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3325 "src/Slice/Grammar.cpp"
    break;

  case 120: /* dictionary_def: ICE_DICTIONARY '<' metadata type ',' metadata type '>' ICE_IDENTIFIER  */
#line 1558 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto keyMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-6]);
    auto keyType = dynamic_pointer_cast<Type>(yyvsp[-5]);
    auto valueMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto valueType = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, std::move(keyMetadata->v), valueType, std::move(valueMetadata->v));
}
#line 3339 "src/Slice/Grammar.cpp"
    break;

  case 121: /* dictionary_def: ICE_DICTIONARY '<' metadata type ',' metadata type '>' keyword  */
#line 1568 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto keyMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-6]);
    auto keyType = dynamic_pointer_cast<Type>(yyvsp[-5]);
    auto valueMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto valueType = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, std::move(keyMetadata->v), valueType, std::move(valueMetadata->v)); // Dummy
    currentUnit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3354 "src/Slice/Grammar.cpp"
    break;

  case 122: /* enum_id: ICE_ENUM ICE_IDENTIFIER  */
#line 1584 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3362 "src/Slice/Grammar.cpp"
    break;

  case 123: /* enum_id: ICE_ENUM keyword  */
#line 1588 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3372 "src/Slice/Grammar.cpp"
    break;

  case 124: /* @22: %empty  */
#line 1599 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v);
    if (en)
    {
        cont->checkIntroduced(ident->v, en);
    }
    else
    {
        en = cont->createEnum(Ice::generateUUID(), Dummy);
    }
    currentUnit->pushContainer(en);
    yyval = en;
}
#line 3392 "src/Slice/Grammar.cpp"
    break;

  case 125: /* enum_def: enum_id @22 '{' enumerator_list '}'  */
#line 1615 "src/Slice/Grammar.y"
{
    auto en = dynamic_pointer_cast<Enum>(yyvsp[-3]);
    if (en)
    {
        auto enumerators = dynamic_pointer_cast<EnumeratorListTok>(yyvsp[-1]);
        if (enumerators->v.empty())
        {
            currentUnit->error("enum `" + en->name() + "' must have at least one enumerator");
        }
        currentUnit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 3410 "src/Slice/Grammar.cpp"
    break;

  case 126: /* @23: %empty  */
#line 1630 "src/Slice/Grammar.y"
{
    currentUnit->error("missing enumeration name");
    ContainerPtr cont = currentUnit->currentContainer();
    EnumPtr en = cont->createEnum(Ice::generateUUID(), Dummy);
    currentUnit->pushContainer(en);
    yyval = en;
}
#line 3422 "src/Slice/Grammar.cpp"
    break;

  case 127: /* enum_def: ICE_ENUM @23 '{' enumerator_list '}'  */
#line 1638 "src/Slice/Grammar.y"
{
    currentUnit->popContainer();
    yyval = yyvsp[-4];
}
#line 3431 "src/Slice/Grammar.cpp"
    break;

  case 128: /* enumerator_list: metadata enumerator ',' enumerator_list  */
#line 1648 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto enumerator = dynamic_pointer_cast<Enumerator>(yyvsp[-2]);
    if (enumerator && !metadata->v.empty())
    {
        enumerator->setMetadata(std::move(metadata->v));
    }
    auto enumeratorList = dynamic_pointer_cast<EnumeratorListTok>(yyvsp[0]);
    enumeratorList->v.push_front(enumerator);
    yyval = enumeratorList;
}
#line 3447 "src/Slice/Grammar.cpp"
    break;

  case 129: /* enumerator_list: metadata enumerator  */
#line 1660 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto enumerator = dynamic_pointer_cast<Enumerator>(yyvsp[0]);
    if (enumerator && !metadata->v.empty())
    {
        enumerator->setMetadata(std::move(metadata->v));
    }
    auto enumeratorList = make_shared<EnumeratorListTok>();
    enumeratorList->v.push_front(enumerator);
    yyval = enumeratorList;
}
#line 3463 "src/Slice/Grammar.cpp"
    break;

  case 130: /* enumerator_list: %empty  */
#line 1672 "src/Slice/Grammar.y"
{
    yyval = make_shared<EnumeratorListTok>(); // Empty list
}
#line 3471 "src/Slice/Grammar.cpp"
    break;

  case 131: /* enumerator: ICE_IDENTIFIER  */
#line 1681 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    yyval = cont->createEnumerator(ident->v, nullopt);
}
#line 3481 "src/Slice/Grammar.cpp"
    break;

  case 132: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1687 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
    if (intVal)
    {
        if (intVal->v < 0 || intVal->v > std::numeric_limits<int32_t>::max())
        {
            currentUnit->error("value for enumerator `" + ident->v + "' is out of range");
        }
        yyval = cont->createEnumerator(ident->v, static_cast<int>(intVal->v));
    }
    else
    {
        yyval = cont->createEnumerator(ident->v, nullopt); // Dummy
    }
}
#line 3503 "src/Slice/Grammar.cpp"
    break;

  case 133: /* enumerator: keyword  */
#line 1705 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    currentUnit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    yyval = cont->createEnumerator(ident->v, nullopt); // Dummy
}
#line 3514 "src/Slice/Grammar.cpp"
    break;

  case 134: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1717 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3522 "src/Slice/Grammar.cpp"
    break;

  case 135: /* enumerator_initializer: scoped_name  */
#line 1721 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainedList cl = currentUnit->currentContainer()->lookupContained(scoped->v, true);
    IntegerTokPtr tok;
    if (!cl.empty())
    {
        auto constant = dynamic_pointer_cast<Const>(cl.front());
        if (constant)
        {
            currentUnit->currentContainer()->checkIntroduced(scoped->v, constant);
            auto b = dynamic_pointer_cast<Builtin>(constant->type());
            if (b && b->isIntegralType())
            {
                try
                {
                    int64_t v = std::stoll(constant->value(), nullptr, 0);
                    tok = make_shared<IntegerTok>();
                    tok->v = v;
                    tok->literal = constant->value();
                }
                catch (const std::exception&)
                {
                }
            }
        }
    }

    if (!tok)
    {
        string msg = "illegal initializer: `" + scoped->v + "' is not an integer constant";
        currentUnit->error(msg); // $$ is dummy
    }

    yyval = tok;
}
#line 3562 "src/Slice/Grammar.cpp"
    break;

  case 136: /* out_qualifier: ICE_OUT  */
#line 1762 "src/Slice/Grammar.y"
{
    auto out = make_shared<BoolTok>();
    out->v = true;
    yyval = out;
}
#line 3572 "src/Slice/Grammar.cpp"
    break;

  case 137: /* out_qualifier: %empty  */
#line 1768 "src/Slice/Grammar.y"
{
    auto out = make_shared<BoolTok>();
    out->v = false;
    yyval = out;
}
#line 3582 "src/Slice/Grammar.cpp"
    break;

  case 138: /* parameters: %empty  */
#line 1779 "src/Slice/Grammar.y"
{
}
#line 3589 "src/Slice/Grammar.cpp"
    break;

  case 139: /* parameters: out_qualifier metadata optional_type_id  */
#line 1782 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto tsp = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isOptional, tsp->tag);
        currentUnit->currentContainer()->checkIntroduced(tsp->name, pd);
        auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
        if (!metadata->v.empty())
        {
            pd->setMetadata(std::move(metadata->v));
        }
    }
}
#line 3609 "src/Slice/Grammar.cpp"
    break;

  case 140: /* parameters: parameters ',' out_qualifier metadata optional_type_id  */
#line 1798 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto tsp = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isOptional, tsp->tag);
        currentUnit->currentContainer()->checkIntroduced(tsp->name, pd);
        auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
        if (!metadata->v.empty())
        {
            pd->setMetadata(std::move(metadata->v));
        }
    }
}
#line 3629 "src/Slice/Grammar.cpp"
    break;

  case 141: /* parameters: out_qualifier metadata type keyword  */
#line 1814 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
#line 3645 "src/Slice/Grammar.cpp"
    break;

  case 142: /* parameters: parameters ',' out_qualifier metadata type keyword  */
#line 1826 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParamDecl(ident->v, type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
#line 3661 "src/Slice/Grammar.cpp"
    break;

  case 143: /* parameters: out_qualifier metadata type  */
#line 1838 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParamDecl(Ice::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("missing parameter name");
    }
}
#line 3676 "src/Slice/Grammar.cpp"
    break;

  case 144: /* parameters: parameters ',' out_qualifier metadata type  */
#line 1849 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParamDecl(Ice::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("missing parameter name");
    }
}
#line 3691 "src/Slice/Grammar.cpp"
    break;

  case 145: /* throws: ICE_THROWS exception_list  */
#line 1865 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3699 "src/Slice/Grammar.cpp"
    break;

  case 146: /* throws: %empty  */
#line 1869 "src/Slice/Grammar.y"
{
    yyval = make_shared<ExceptionListTok>();
}
#line 3707 "src/Slice/Grammar.cpp"
    break;

  case 147: /* scoped_name: ICE_IDENTIFIER  */
#line 1878 "src/Slice/Grammar.y"
{
}
#line 3714 "src/Slice/Grammar.cpp"
    break;

  case 148: /* scoped_name: ICE_SCOPED_IDENTIFIER  */
#line 1881 "src/Slice/Grammar.y"
{
}
#line 3721 "src/Slice/Grammar.cpp"
    break;

  case 149: /* builtin: ICE_BOOL  */
#line 1888 "src/Slice/Grammar.y"
           {}
#line 3727 "src/Slice/Grammar.cpp"
    break;

  case 150: /* builtin: ICE_BYTE  */
#line 1889 "src/Slice/Grammar.y"
           {}
#line 3733 "src/Slice/Grammar.cpp"
    break;

  case 151: /* builtin: ICE_SHORT  */
#line 1890 "src/Slice/Grammar.y"
            {}
#line 3739 "src/Slice/Grammar.cpp"
    break;

  case 152: /* builtin: ICE_INT  */
#line 1891 "src/Slice/Grammar.y"
          {}
#line 3745 "src/Slice/Grammar.cpp"
    break;

  case 153: /* builtin: ICE_LONG  */
#line 1892 "src/Slice/Grammar.y"
           {}
#line 3751 "src/Slice/Grammar.cpp"
    break;

  case 154: /* builtin: ICE_FLOAT  */
#line 1893 "src/Slice/Grammar.y"
            {}
#line 3757 "src/Slice/Grammar.cpp"
    break;

  case 155: /* builtin: ICE_DOUBLE  */
#line 1894 "src/Slice/Grammar.y"
             {}
#line 3763 "src/Slice/Grammar.cpp"
    break;

  case 156: /* builtin: ICE_STRING  */
#line 1895 "src/Slice/Grammar.y"
             {}
#line 3769 "src/Slice/Grammar.cpp"
    break;

  case 157: /* builtin: ICE_OBJECT  */
#line 1896 "src/Slice/Grammar.y"
             {}
#line 3775 "src/Slice/Grammar.cpp"
    break;

  case 158: /* builtin: ICE_VALUE  */
#line 1897 "src/Slice/Grammar.y"
            {}
#line 3781 "src/Slice/Grammar.cpp"
    break;

  case 159: /* type: ICE_OBJECT '*'  */
#line 1903 "src/Slice/Grammar.y"
{
    yyval = currentUnit->createBuiltin(Builtin::KindObjectProxy);
}
#line 3789 "src/Slice/Grammar.cpp"
    break;

  case 160: /* type: builtin  */
#line 1907 "src/Slice/Grammar.y"
{
    auto typeName = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    yyval = currentUnit->createBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 3798 "src/Slice/Grammar.cpp"
    break;

  case 161: /* type: scoped_name  */
#line 1912 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    if (cont)
    {
        TypeList types = cont->lookupType(scoped->v);
        if (types.empty())
        {
            YYERROR; // Can't continue, jump to next yyerrok
        }
        TypePtr firstType = types.front();

        auto interface = dynamic_pointer_cast<InterfaceDecl>(firstType);
        if (interface)
        {
            string msg = "add a '*' after the interface name to specify its proxy type: '";
            msg += scoped->v;
            msg += "*'";
            currentUnit->error(msg);
            YYERROR; // Can't continue, jump to next yyerrok
        }
        cont->checkIntroduced(scoped->v);

        yyval = firstType;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 3833 "src/Slice/Grammar.cpp"
    break;

  case 162: /* type: scoped_name '*'  */
#line 1943 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    ContainerPtr cont = currentUnit->currentContainer();
    if (cont)
    {
        TypeList types = cont->lookupType(scoped->v);
        if (types.empty())
        {
            YYERROR; // Can't continue, jump to next yyerrok
        }
        TypePtr firstType = types.front();

        auto interface = dynamic_pointer_cast<InterfaceDecl>(firstType);
        if (!interface)
        {
            string msg = "`";
            msg += scoped->v;
            msg += "' must be an interface";
            currentUnit->error(msg);
            YYERROR; // Can't continue, jump to next yyerrok
        }
        cont->checkIntroduced(scoped->v);

        yyval = firstType;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 3868 "src/Slice/Grammar.cpp"
    break;

  case 163: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 1979 "src/Slice/Grammar.y"
{
    auto str1 = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    auto str2 = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    str1->v += str2->v;
}
#line 3878 "src/Slice/Grammar.cpp"
    break;

  case 164: /* string_literal: ICE_STRING_LITERAL  */
#line 1985 "src/Slice/Grammar.y"
{
}
#line 3885 "src/Slice/Grammar.cpp"
    break;

  case 165: /* metadata_list: metadata_list ',' string_literal  */
#line 1993 "src/Slice/Grammar.y"
{
    auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadataList = dynamic_pointer_cast<MetadataListTok>(yyvsp[-2]);

    auto metadata = make_shared<Metadata>(str->v, currentUnit->currentFile(), currentUnit->currentLine());
    metadataList->v.push_back(metadata);

    yyval = metadataList;
}
#line 3899 "src/Slice/Grammar.cpp"
    break;

  case 166: /* metadata_list: string_literal  */
#line 2003 "src/Slice/Grammar.y"
{
    auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadataList = make_shared<MetadataListTok>();

    auto metadata = make_shared<Metadata>(str->v, currentUnit->currentFile(), currentUnit->currentLine());
    metadataList->v.push_back(metadata);

    yyval = metadataList;
}
#line 3913 "src/Slice/Grammar.cpp"
    break;

  case 167: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2018 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindLong);
    auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    auto def = make_shared<ConstDefTok>(type, sstr.str());
    yyval = def;
}
#line 3926 "src/Slice/Grammar.cpp"
    break;

  case 168: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2027 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindDouble);
    auto floatVal = dynamic_pointer_cast<FloatingTok>(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    auto def = make_shared<ConstDefTok>(type, sstr.str());
    yyval = def;
}
#line 3939 "src/Slice/Grammar.cpp"
    break;

  case 169: /* const_initializer: scoped_name  */
#line 2036 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ConstDefTokPtr def;
    ContainedList cl = currentUnit->currentContainer()->lookupContained(scoped->v, false);
    if (cl.empty())
    {
        // Could be an enumerator
        def = make_shared<ConstDefTok>(nullptr, scoped->v);
    }
    else
    {
        auto enumerator = dynamic_pointer_cast<Enumerator>(cl.front());
        auto constant = dynamic_pointer_cast<Const>(cl.front());
        if (enumerator)
        {
            currentUnit->currentContainer()->checkIntroduced(scoped->v, enumerator);
            def = make_shared<ConstDefTok>(enumerator, scoped->v);
        }
        else if (constant)
        {
            currentUnit->currentContainer()->checkIntroduced(scoped->v, constant);
            def = make_shared<ConstDefTok>(constant, constant->value());
        }
        else
        {
            def = make_shared<ConstDefTok>();
            string msg = "illegal initializer: `" + scoped->v + "' is ";
            string kindOf = cl.front()->kindOf();
            msg += prependA(kindOf);
            currentUnit->error(msg); // $$ is dummy
        }
    }
    yyval = def;
}
#line 3978 "src/Slice/Grammar.cpp"
    break;

  case 170: /* const_initializer: ICE_STRING_LITERAL  */
#line 2071 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindString);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, literal->v);
    yyval = def;
}
#line 3989 "src/Slice/Grammar.cpp"
    break;

  case 171: /* const_initializer: ICE_FALSE  */
#line 2078 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindBool);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, "false");
    yyval = def;
}
#line 4000 "src/Slice/Grammar.cpp"
    break;

  case 172: /* const_initializer: ICE_TRUE  */
#line 2085 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindBool);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, "true");
    yyval = def;
}
#line 4011 "src/Slice/Grammar.cpp"
    break;

  case 173: /* const_def: ICE_CONST metadata type ICE_IDENTIFIER '=' const_initializer  */
#line 2097 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-4]);
    auto const_type = dynamic_pointer_cast<Type>(yyvsp[-3]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    yyval = currentUnit->currentContainer()->createConst(ident->v, const_type, std::move(metadata->v), value->v,
                                                      value->valueAsString);
}
#line 4024 "src/Slice/Grammar.cpp"
    break;

  case 174: /* const_def: ICE_CONST metadata type '=' const_initializer  */
#line 2106 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto const_type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    currentUnit->error("missing constant name");
    yyval = currentUnit->currentContainer()->createConst(Ice::generateUUID(), const_type, std::move(metadata->v),
                                                      value->v, value->valueAsString, Dummy); // Dummy
}
#line 4037 "src/Slice/Grammar.cpp"
    break;

  case 175: /* keyword: ICE_MODULE  */
#line 2119 "src/Slice/Grammar.y"
             {}
#line 4043 "src/Slice/Grammar.cpp"
    break;

  case 176: /* keyword: ICE_CLASS  */
#line 2120 "src/Slice/Grammar.y"
            {}
#line 4049 "src/Slice/Grammar.cpp"
    break;

  case 177: /* keyword: ICE_INTERFACE  */
#line 2121 "src/Slice/Grammar.y"
                {}
#line 4055 "src/Slice/Grammar.cpp"
    break;

  case 178: /* keyword: ICE_EXCEPTION  */
#line 2122 "src/Slice/Grammar.y"
                {}
#line 4061 "src/Slice/Grammar.cpp"
    break;

  case 179: /* keyword: ICE_STRUCT  */
#line 2123 "src/Slice/Grammar.y"
             {}
#line 4067 "src/Slice/Grammar.cpp"
    break;

  case 180: /* keyword: ICE_SEQUENCE  */
#line 2124 "src/Slice/Grammar.y"
               {}
#line 4073 "src/Slice/Grammar.cpp"
    break;

  case 181: /* keyword: ICE_DICTIONARY  */
#line 2125 "src/Slice/Grammar.y"
                 {}
#line 4079 "src/Slice/Grammar.cpp"
    break;

  case 182: /* keyword: ICE_ENUM  */
#line 2126 "src/Slice/Grammar.y"
           {}
#line 4085 "src/Slice/Grammar.cpp"
    break;

  case 183: /* keyword: ICE_OUT  */
#line 2127 "src/Slice/Grammar.y"
          {}
#line 4091 "src/Slice/Grammar.cpp"
    break;

  case 184: /* keyword: ICE_EXTENDS  */
#line 2128 "src/Slice/Grammar.y"
              {}
#line 4097 "src/Slice/Grammar.cpp"
    break;

  case 185: /* keyword: ICE_THROWS  */
#line 2129 "src/Slice/Grammar.y"
             {}
#line 4103 "src/Slice/Grammar.cpp"
    break;

  case 186: /* keyword: ICE_VOID  */
#line 2130 "src/Slice/Grammar.y"
           {}
#line 4109 "src/Slice/Grammar.cpp"
    break;

  case 187: /* keyword: ICE_BOOL  */
#line 2131 "src/Slice/Grammar.y"
           {}
#line 4115 "src/Slice/Grammar.cpp"
    break;

  case 188: /* keyword: ICE_BYTE  */
#line 2132 "src/Slice/Grammar.y"
           {}
#line 4121 "src/Slice/Grammar.cpp"
    break;

  case 189: /* keyword: ICE_SHORT  */
#line 2133 "src/Slice/Grammar.y"
            {}
#line 4127 "src/Slice/Grammar.cpp"
    break;

  case 190: /* keyword: ICE_INT  */
#line 2134 "src/Slice/Grammar.y"
          {}
#line 4133 "src/Slice/Grammar.cpp"
    break;

  case 191: /* keyword: ICE_LONG  */
#line 2135 "src/Slice/Grammar.y"
           {}
#line 4139 "src/Slice/Grammar.cpp"
    break;

  case 192: /* keyword: ICE_FLOAT  */
#line 2136 "src/Slice/Grammar.y"
            {}
#line 4145 "src/Slice/Grammar.cpp"
    break;

  case 193: /* keyword: ICE_DOUBLE  */
#line 2137 "src/Slice/Grammar.y"
             {}
#line 4151 "src/Slice/Grammar.cpp"
    break;

  case 194: /* keyword: ICE_STRING  */
#line 2138 "src/Slice/Grammar.y"
             {}
#line 4157 "src/Slice/Grammar.cpp"
    break;

  case 195: /* keyword: ICE_OBJECT  */
#line 2139 "src/Slice/Grammar.y"
             {}
#line 4163 "src/Slice/Grammar.cpp"
    break;

  case 196: /* keyword: ICE_CONST  */
#line 2140 "src/Slice/Grammar.y"
            {}
#line 4169 "src/Slice/Grammar.cpp"
    break;

  case 197: /* keyword: ICE_FALSE  */
#line 2141 "src/Slice/Grammar.y"
            {}
#line 4175 "src/Slice/Grammar.cpp"
    break;

  case 198: /* keyword: ICE_TRUE  */
#line 2142 "src/Slice/Grammar.y"
           {}
#line 4181 "src/Slice/Grammar.cpp"
    break;

  case 199: /* keyword: ICE_IDEMPOTENT  */
#line 2143 "src/Slice/Grammar.y"
                 {}
#line 4187 "src/Slice/Grammar.cpp"
    break;

  case 200: /* keyword: ICE_OPTIONAL  */
#line 2144 "src/Slice/Grammar.y"
               {}
#line 4193 "src/Slice/Grammar.cpp"
    break;

  case 201: /* keyword: ICE_VALUE  */
#line 2145 "src/Slice/Grammar.y"
            {}
#line 4199 "src/Slice/Grammar.cpp"
    break;


#line 4203 "src/Slice/Grammar.cpp"

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

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
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 2148 "src/Slice/Grammar.y"

