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

// NOLINTBEGIN

#line 27 "src/Slice/Grammar.y"


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


#line 106 "src/Slice/Grammar.cpp"

/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yydebug         slice_debug
#define yynerrs         slice_nerrs

/* First part of user prologue.  */
#line 66 "src/Slice/Grammar.y"


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


#line 171 "src/Slice/Grammar.cpp"

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
  YYSYMBOL_metadata_directives = 58,       /* metadata_directives  */
  YYSYMBOL_definitions = 59,               /* definitions  */
  YYSYMBOL_definition = 60,                /* definition  */
  YYSYMBOL_61_1 = 61,                      /* $@1  */
  YYSYMBOL_62_2 = 62,                      /* $@2  */
  YYSYMBOL_63_3 = 63,                      /* $@3  */
  YYSYMBOL_64_4 = 64,                      /* $@4  */
  YYSYMBOL_65_5 = 65,                      /* $@5  */
  YYSYMBOL_66_6 = 66,                      /* $@6  */
  YYSYMBOL_67_7 = 67,                      /* $@7  */
  YYSYMBOL_68_8 = 68,                      /* $@8  */
  YYSYMBOL_69_9 = 69,                      /* $@9  */
  YYSYMBOL_70_10 = 70,                     /* $@10  */
  YYSYMBOL_71_11 = 71,                     /* $@11  */
  YYSYMBOL_72_12 = 72,                     /* $@12  */
  YYSYMBOL_73_13 = 73,                     /* $@13  */
  YYSYMBOL_module_def = 74,                /* module_def  */
  YYSYMBOL_75_14 = 75,                     /* @14  */
  YYSYMBOL_76_15 = 76,                     /* @15  */
  YYSYMBOL_exception_id = 77,              /* exception_id  */
  YYSYMBOL_exception_decl = 78,            /* exception_decl  */
  YYSYMBOL_exception_def = 79,             /* exception_def  */
  YYSYMBOL_80_16 = 80,                     /* @16  */
  YYSYMBOL_exception_extends = 81,         /* exception_extends  */
  YYSYMBOL_type_id = 82,                   /* type_id  */
  YYSYMBOL_optional = 83,                  /* optional  */
  YYSYMBOL_optional_type_id = 84,          /* optional_type_id  */
  YYSYMBOL_struct_id = 85,                 /* struct_id  */
  YYSYMBOL_struct_decl = 86,               /* struct_decl  */
  YYSYMBOL_struct_def = 87,                /* struct_def  */
  YYSYMBOL_88_17 = 88,                     /* @17  */
  YYSYMBOL_class_name = 89,                /* class_name  */
  YYSYMBOL_class_id = 90,                  /* class_id  */
  YYSYMBOL_class_decl = 91,                /* class_decl  */
  YYSYMBOL_class_def = 92,                 /* class_def  */
  YYSYMBOL_93_18 = 93,                     /* @18  */
  YYSYMBOL_class_extends = 94,             /* class_extends  */
  YYSYMBOL_extends = 95,                   /* extends  */
  YYSYMBOL_data_members = 96,              /* data_members  */
  YYSYMBOL_data_member = 97,               /* data_member  */
  YYSYMBOL_return_type = 98,               /* return_type  */
  YYSYMBOL_operation_preamble = 99,        /* operation_preamble  */
  YYSYMBOL_operation = 100,                /* operation  */
  YYSYMBOL_101_19 = 101,                   /* @19  */
  YYSYMBOL_102_20 = 102,                   /* @20  */
  YYSYMBOL_interface_id = 103,             /* interface_id  */
  YYSYMBOL_interface_decl = 104,           /* interface_decl  */
  YYSYMBOL_interface_def = 105,            /* interface_def  */
  YYSYMBOL_106_21 = 106,                   /* @21  */
  YYSYMBOL_interface_list = 107,           /* interface_list  */
  YYSYMBOL_interface_extends = 108,        /* interface_extends  */
  YYSYMBOL_operations = 109,               /* operations  */
  YYSYMBOL_exception_list = 110,           /* exception_list  */
  YYSYMBOL_exception = 111,                /* exception  */
  YYSYMBOL_sequence_def = 112,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 113,           /* dictionary_def  */
  YYSYMBOL_enum_id = 114,                  /* enum_id  */
  YYSYMBOL_enum_def = 115,                 /* enum_def  */
  YYSYMBOL_116_22 = 116,                   /* @22  */
  YYSYMBOL_117_23 = 117,                   /* @23  */
  YYSYMBOL_enumerators = 118,              /* enumerators  */
  YYSYMBOL_enumerator_list = 119,          /* enumerator_list  */
  YYSYMBOL_enumerator = 120,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 121,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 122,            /* out_qualifier  */
  YYSYMBOL_parameters = 123,               /* parameters  */
  YYSYMBOL_parameter_list = 124,           /* parameter_list  */
  YYSYMBOL_throws = 125,                   /* throws  */
  YYSYMBOL_scoped_name = 126,              /* scoped_name  */
  YYSYMBOL_builtin = 127,                  /* builtin  */
  YYSYMBOL_type = 128,                     /* type  */
  YYSYMBOL_string_literal = 129,           /* string_literal  */
  YYSYMBOL_metadata_list = 130,            /* metadata_list  */
  YYSYMBOL_const_initializer = 131,        /* const_initializer  */
  YYSYMBOL_const_def = 132,                /* const_def  */
  YYSYMBOL_keyword = 133                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 58 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 347 "src/Slice/Grammar.cpp"

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
#define YYLAST   676

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  81
/* YYNRULES -- Number of rules.  */
#define YYNRULES  206
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  298

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
       0,   184,   184,   192,   195,   203,   212,   215,   224,   228,
     240,   248,   257,   266,   265,   271,   270,   275,   280,   279,
     285,   284,   289,   294,   293,   299,   298,   303,   308,   307,
     313,   312,   317,   322,   321,   327,   326,   331,   336,   335,
     340,   345,   344,   350,   349,   354,   358,   368,   367,   398,
     397,   481,   485,   496,   507,   506,   532,   540,   549,   560,
     578,   653,   659,   670,   687,   700,   704,   715,   726,   725,
     764,   768,   779,   804,   891,   903,   916,   915,   949,   983,
     992,   995,  1003,  1012,  1015,  1019,  1027,  1057,  1088,  1110,
    1136,  1151,  1157,  1167,  1187,  1213,  1233,  1264,  1263,  1286,
    1285,  1308,  1312,  1323,  1337,  1336,  1370,  1405,  1440,  1445,
    1455,  1459,  1468,  1477,  1480,  1484,  1492,  1499,  1511,  1523,
    1534,  1542,  1556,  1566,  1582,  1586,  1598,  1597,  1629,  1628,
    1646,  1647,  1651,  1660,  1672,  1689,  1695,  1713,  1725,  1729,
    1770,  1776,  1787,  1788,  1794,  1810,  1826,  1838,  1850,  1861,
    1877,  1881,  1890,  1893,  1901,  1902,  1903,  1904,  1905,  1906,
    1907,  1908,  1909,  1910,  1915,  1919,  1924,  1949,  1979,  1985,
    1993,  2003,  2018,  2027,  2036,  2071,  2078,  2085,  2097,  2106,
    2120,  2121,  2122,  2123,  2124,  2125,  2126,  2127,  2128,  2129,
    2130,  2131,  2132,  2133,  2134,  2135,  2136,  2137,  2138,  2139,
    2140,  2141,  2142,  2143,  2144,  2145,  2146
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
  "file_metadata", "metadata", "metadata_directives", "definitions",
  "definition", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8",
  "$@9", "$@10", "$@11", "$@12", "$@13", "module_def", "@14", "@15",
  "exception_id", "exception_decl", "exception_def", "@16",
  "exception_extends", "type_id", "optional", "optional_type_id",
  "struct_id", "struct_decl", "struct_def", "@17", "class_name",
  "class_id", "class_decl", "class_def", "@18", "class_extends", "extends",
  "data_members", "data_member", "return_type", "operation_preamble",
  "operation", "@19", "@20", "interface_id", "interface_decl",
  "interface_def", "@21", "interface_list", "interface_extends",
  "operations", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_id", "enum_def", "@22", "@23", "enumerators",
  "enumerator_list", "enumerator", "enumerator_initializer",
  "out_qualifier", "parameters", "parameter_list", "throws", "scoped_name",
  "builtin", "type", "string_literal", "metadata_list",
  "const_initializer", "const_def", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-205)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-144)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -205,    18,     2,  -205,    -3,    -3,  -205,    71,     1,    -3,
    -205,    47,    33,    50,   111,   281,   350,   381,   412,    48,
      53,   443,    70,  -205,  -205,    10,    74,  -205,    75,    80,
    -205,    19,    11,    87,  -205,    23,    89,  -205,    96,   115,
    -205,  -205,   121,    -3,  -205,  -205,    -3,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,   137,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,    70,    70,  -205,
     125,  -205,   642,   129,  -205,  -205,  -205,   147,   132,   129,
     138,   141,   129,  -205,   147,   144,   129,   108,  -205,   149,
     129,   151,   160,   161,   129,   170,    52,  -205,   168,   171,
     -14,  -205,  -205,   172,   642,   642,   -11,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,   164,  -205,   167,  -205,   -18,
    -205,  -205,   176,  -205,  -205,  -205,   206,  -205,  -205,   186,
    -205,  -205,  -205,  -205,  -205,   182,  -205,   188,  -205,  -205,
    -205,  -205,   -11,  -205,  -205,  -205,  -205,  -205,  -205,  -205,
     163,   184,   474,   191,   193,  -205,  -205,   189,   165,   206,
     200,   133,   199,   206,   147,    93,   201,    98,   128,   505,
      70,   197,  -205,  -205,  -205,   101,   165,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,   203,   206,  -205,    58,  -205,   642,
     202,   209,   536,  -205,   204,  -205,   210,   587,   211,  -205,
    -205,  -205,  -205,  -205,   642,   145,   474,  -205,  -205,  -205,
     208,  -205,   212,  -205,   222,   165,   206,  -205,  -205,  -205,
      93,  -205,   615,   642,   162,    27,   216,  -205,  -205,   213,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,   169,
    -205,  -205,  -205,   214,  -205,    70,   215,   218,    93,   567,
    -205,  -205,  -205,   133,  -205,   251,  -205,  -205,  -205,   252,
    -205,   536,   252,    70,   318,  -205,  -205,  -205,   133,   219,
    -205,  -205,  -205,  -205,   536,   318,  -205,  -205
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      12,     0,     7,     1,     0,     0,    10,     0,     6,   169,
     171,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   128,     7,    11,    13,    53,    27,    28,    67,    32,
      33,    75,    79,    17,    18,   103,    22,    23,    37,    40,
     126,    41,    45,     0,   168,     8,     0,     5,    46,    47,
      49,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,    70,     0,
      71,   101,   102,    51,    52,    65,    66,     7,     7,   124,
       0,   125,     0,     4,    80,    81,    54,     0,     0,     4,
       0,     0,     4,    76,     0,     0,     4,     0,   104,     0,
       4,     0,     0,     0,     4,     0,     0,   170,     0,     0,
       0,   152,   153,     0,     0,     0,     7,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   166,   165,     0,
       3,    14,     0,    56,    26,    29,     0,    31,    34,     0,
      78,    16,    19,   108,   109,   110,   107,     0,    21,    24,
      36,    39,     7,    42,    44,     9,    12,    12,    72,    73,
       0,     0,     0,     0,   130,   164,   167,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     7,     0,
       7,   135,   134,   137,   129,     7,     0,   176,   177,   175,
     172,   173,   174,   179,     0,     0,    62,     0,    64,     0,
      86,    84,    89,    69,     0,   106,     0,     0,     0,   127,
      48,    50,   120,   121,     0,     0,     0,   178,    55,    83,
       0,    61,     0,    63,     0,     0,     0,    58,    88,    77,
       0,    92,     0,     0,     0,     0,   114,    91,   105,     0,
     138,   136,   139,   133,    59,    60,    87,    82,   113,     0,
      90,    93,    95,     0,   140,     7,     0,   142,     0,     0,
      94,    96,    99,     0,    97,   141,   112,   122,   123,   151,
     144,   148,   151,     7,     0,   100,   146,    98,     0,   150,
     117,   118,   119,   145,   149,     0,   147,   116
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -205,  -205,   -89,  -205,    -2,  -205,    44,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,    61,
    -152,  -204,  -205,  -205,  -205,  -205,  -205,  -205,  -205,  -205,
    -205,  -205,    67,  -172,  -205,    29,  -205,  -205,  -205,  -205,
    -205,  -205,  -205,  -205,  -205,  -205,  -181,  -205,   -22,  -205,
    -205,  -205,  -205,  -205,  -205,   112,  -205,    49,  -205,     3,
    -205,  -205,    -6,   -78,  -205,   -84,     7,     9,  -184,  -205,
     -12
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   141,     6,   181,     8,     2,    23,    93,   105,
     106,   109,   110,    98,    99,   101,   102,   111,   112,   114,
     115,    24,   118,   119,    25,    26,    27,   142,    96,   208,
     209,   210,    28,    29,    30,   100,    31,    32,    33,    34,
     149,   103,    97,   182,   211,   244,   245,   246,   282,   279,
      35,    36,    37,   157,   155,   108,   218,   289,   290,    38,
      39,    40,    41,   113,    90,   173,   174,   192,   251,   265,
     266,   267,   285,   137,   138,   247,    10,    11,   203,    42,
     193
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       7,   123,    -2,    80,    82,    84,    86,   204,   139,    91,
     145,   214,   227,   148,    12,   177,    44,   152,     3,   143,
      92,   159,    94,    94,     4,   163,   150,     9,   263,   156,
     178,   -74,   168,   229,  -132,    94,    43,     4,   264,     5,
     170,   171,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,
    -141,   256,   116,   117,   -57,  -141,  -141,    95,    95,   258,
    -141,  -141,  -141,   -74,   257,   243,   -74,  -111,  -141,   280,
      95,    47,    13,  -143,    14,    15,    16,    17,    18,    19,
      20,    21,    46,    45,   293,   124,   125,   276,   165,   230,
     243,   121,   122,    48,   216,    22,    46,   212,    87,   104,
     202,    46,   107,    88,   231,     4,   215,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,   -25,   202,   -68,
      -7,    -7,    -7,   -30,   172,   234,    -7,    -7,     4,   232,
     -15,   153,   -20,     4,    -7,     5,     4,   154,  -115,   -35,
     249,   121,   122,   220,    49,    50,  -131,   252,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   202,   -38,   260,
     172,   206,   136,     4,   -43,     5,   121,   122,   120,   126,
     121,   122,   140,   221,   207,   144,   250,   223,   121,   122,
     121,   122,   146,   217,   147,     7,     7,   151,   224,   281,
     197,   198,   158,   226,   160,   199,   200,   201,   121,   122,
     238,   261,   262,   161,   294,   162,   291,   180,   270,   271,
     187,   188,   166,   164,   189,   167,   175,   291,   169,   176,
     179,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,
     183,   184,   185,   190,    -7,    -7,   194,   196,   217,    -7,
      -7,     4,   195,   205,   213,   225,   219,    -7,   228,   239,
     235,   -85,   236,   240,   254,   237,   248,   278,   255,   268,
     272,   274,   264,   273,   269,   284,   217,   275,   295,   286,
     233,   259,   292,   297,   186,   253,   287,     0,   283,     0,
       0,   288,   296,   292,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,     0,     0,     0,    78,     0,     0,     0,     0,     0,
      79,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,     0,     0,
       0,   121,   122,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
       0,     0,     0,    81,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,     0,     0,     0,    83,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,     0,     0,     0,    85,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,     0,     0,     0,    89,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,     0,     0,     0,   191,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,     0,     0,     0,   222,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,     0,     0,     0,   237,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,     0,     0,     0,
     277,   241,   127,   128,   129,   130,   131,   132,   133,   134,
     135,     0,     0,     0,   242,   206,   136,     0,     0,     0,
     121,   122,     0,     0,     0,     0,     0,     0,   207,   241,
     127,   128,   129,   130,   131,   132,   133,   134,   135,     0,
       0,     0,     0,   206,   136,     0,     0,     0,   121,   122,
       0,     0,     0,     0,     0,     0,   207,   127,   128,   129,
     130,   131,   132,   133,   134,   135,     0,     0,     0,     0,
       0,   136,     0,     0,     0,   121,   122
};

static const yytype_int16 yycheck[] =
{
       2,    79,     0,    15,    16,    17,    18,   179,    92,    21,
      99,   183,   196,   102,     5,    33,     9,   106,     0,    97,
      22,   110,    12,    12,    35,   114,   104,    30,     1,   107,
      48,    12,    46,   205,    45,    12,    35,    35,    11,    37,
     124,   125,    15,    16,    17,    18,    19,    20,    21,    22,
      23,   235,    43,    46,    44,    28,    29,    47,    47,   240,
      33,    34,    35,    44,   236,   217,    47,    44,    41,   273,
      47,    38,     1,    46,     3,     4,     5,     6,     7,     8,
       9,    10,    49,    36,   288,    87,    88,   268,    36,    31,
     242,    33,    34,    43,     1,    24,    49,   181,    50,    32,
     178,    49,    35,    50,    46,    35,   184,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    43,   196,    44,
      27,    28,    29,    43,   126,   209,    33,    34,    35,   207,
      43,    23,    43,    35,    41,    37,    35,    29,    45,    43,
     224,    33,    34,    45,    33,    34,    45,   225,    15,    16,
      17,    18,    19,    20,    21,    22,    23,   235,    43,   243,
     162,    28,    29,    35,    43,    37,    33,    34,    31,    44,
      33,    34,    43,    45,    41,    43,    31,   189,    33,    34,
      33,    34,    44,   185,    43,   187,   188,    43,   190,   273,
      25,    26,    43,   195,    43,    30,    31,    32,    33,    34,
     212,    39,    40,    43,   288,    44,   284,     1,    39,    40,
     166,   167,    44,    43,    51,    44,    52,   295,    46,    52,
      44,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      44,    49,    44,    49,    28,    29,    45,    48,   240,    33,
      34,    35,    49,    43,    45,    48,    45,    41,    45,    45,
      48,    45,    43,    43,    46,    33,    45,   269,    46,    43,
      46,    46,    11,   265,    51,    13,   268,    49,    49,   281,
     209,   242,   284,   295,   162,   226,   282,    -1,   275,    -1,
      -1,   283,   294,   295,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      39,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
      -1,    33,    34,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    -1,    33,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    -1,    33,     3,     4,     5,     6,     7,
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
      33,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    -1,
      33,    34,    -1,    -1,    -1,    -1,    -1,    -1,    41,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    -1,
      -1,    -1,    -1,    28,    29,    -1,    -1,    -1,    33,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    -1,
      -1,    29,    -1,    -1,    -1,    33,    34
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    54,    59,     0,    35,    37,    56,    57,    58,    30,
     129,   130,   130,     1,     3,     4,     5,     6,     7,     8,
       9,    10,    24,    60,    74,    77,    78,    79,    85,    86,
      87,    89,    90,    91,    92,   103,   104,   105,   112,   113,
     114,   115,   132,    35,   129,    36,    49,    38,    43,    33,
      34,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    33,    39,
     133,    33,   133,    33,   133,    33,   133,    50,    50,    33,
     117,   133,    57,    61,    12,    47,    81,    95,    66,    67,
      88,    68,    69,    94,    95,    62,    63,    95,   108,    64,
      65,    70,    71,   116,    72,    73,   130,   129,    75,    76,
      31,    33,    34,   126,    57,    57,    44,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    29,   126,   127,   128,
      43,    55,    80,   126,    43,    55,    44,    43,    55,    93,
     126,    43,    55,    23,    29,   107,   126,   106,    43,    55,
      43,    43,    44,    55,    43,    36,    44,    44,    46,    46,
     128,   128,    57,   118,   119,    52,    52,    33,    48,    44,
       1,    57,    96,    44,    49,    44,   118,    59,    59,    51,
      49,    33,   120,   133,    45,    49,    48,    25,    26,    30,
      31,    32,   126,   131,    96,    43,    28,    41,    82,    83,
      84,    97,   128,    45,    96,   126,     1,    57,   109,    45,
      45,    45,    33,   133,    57,    48,    57,   131,    45,    96,
      31,    46,   126,    82,   128,    48,    43,    33,   133,    45,
      43,    14,    27,    83,    98,    99,   100,   128,    45,   128,
      31,   121,   126,   120,    46,    46,   131,    96,   109,    98,
     128,    39,    40,     1,    11,   122,   123,   124,    43,    51,
      39,    40,    46,    57,    46,    49,   109,    33,   133,   102,
      84,   128,   101,   122,    13,   125,   133,   125,    57,   110,
     111,   126,   133,    84,   128,    49,   133,   111
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    53,    54,    55,    55,    56,    57,    57,    58,    58,
      59,    59,    59,    61,    60,    62,    60,    60,    63,    60,
      64,    60,    60,    65,    60,    66,    60,    60,    67,    60,
      68,    60,    60,    69,    60,    70,    60,    60,    71,    60,
      60,    72,    60,    73,    60,    60,    60,    75,    74,    76,
      74,    77,    77,    78,    80,    79,    81,    81,    82,    83,
      83,    83,    83,    84,    84,    85,    85,    86,    88,    87,
      89,    89,    90,    90,    90,    91,    93,    92,    94,    94,
      95,    95,    96,    96,    96,    96,    97,    97,    97,    97,
      98,    98,    98,    99,    99,    99,    99,   101,   100,   102,
     100,   103,   103,   104,   106,   105,   107,   107,   107,   107,
     108,   108,   109,   109,   109,   109,   110,   110,   111,   111,
     112,   112,   113,   113,   114,   114,   116,   115,   117,   115,
     118,   118,   118,   119,   119,   120,   120,   120,   121,   121,
     122,   122,   123,   123,   124,   124,   124,   124,   124,   124,
     125,   125,   126,   126,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   128,   128,   128,   128,   129,   129,
     130,   130,   131,   131,   131,   131,   131,   131,   132,   132,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     1,     0,     3,     4,
       2,     3,     0,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       1,     0,     3,     0,     3,     1,     2,     0,     6,     0,
       6,     2,     2,     1,     0,     6,     2,     0,     2,     3,
       3,     2,     1,     2,     1,     2,     2,     1,     0,     5,
       2,     2,     4,     4,     1,     1,     0,     6,     2,     0,
       1,     1,     4,     3,     2,     0,     1,     3,     2,     1,
       2,     1,     1,     2,     3,     2,     3,     0,     5,     0,
       5,     2,     2,     1,     0,     6,     3,     1,     1,     1,
       2,     0,     4,     3,     2,     0,     3,     1,     1,     1,
       6,     6,     9,     9,     2,     2,     0,     5,     0,     5,
       1,     2,     0,     4,     2,     1,     3,     1,     1,     1,
       1,     0,     1,     0,     3,     5,     4,     6,     3,     5,
       2,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     2,     2,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     6,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
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
#line 185 "src/Slice/Grammar.y"
{
}
#line 1739 "src/Slice/Grammar.cpp"
    break;

  case 3: /* opt_semicolon: ';'  */
#line 193 "src/Slice/Grammar.y"
{
}
#line 1746 "src/Slice/Grammar.cpp"
    break;

  case 4: /* opt_semicolon: %empty  */
#line 196 "src/Slice/Grammar.y"
{
}
#line 1753 "src/Slice/Grammar.cpp"
    break;

  case 5: /* file_metadata: ICE_FILE_METADATA_OPEN metadata_list ICE_FILE_METADATA_CLOSE  */
#line 204 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1761 "src/Slice/Grammar.cpp"
    break;

  case 6: /* metadata: metadata_directives  */
#line 213 "src/Slice/Grammar.y"
{
}
#line 1768 "src/Slice/Grammar.cpp"
    break;

  case 7: /* metadata: %empty  */
#line 216 "src/Slice/Grammar.y"
{
    yyval = make_shared<MetadataListTok>();
}
#line 1776 "src/Slice/Grammar.cpp"
    break;

  case 8: /* metadata_directives: ICE_METADATA_OPEN metadata_list ICE_METADATA_CLOSE  */
#line 225 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1784 "src/Slice/Grammar.cpp"
    break;

  case 9: /* metadata_directives: metadata_directives ICE_METADATA_OPEN metadata_list ICE_METADATA_CLOSE  */
#line 229 "src/Slice/Grammar.y"
{
    auto metadata1 = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto metadata2 = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    metadata1->v.splice(metadata1->v.end(), std::move(metadata2->v));
    yyval = metadata1;
}
#line 1795 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: definitions file_metadata  */
#line 241 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[0]);
    if (!metadata->v.empty())
    {
        currentUnit->addFileMetadata(std::move(metadata->v));
    }
}
#line 1807 "src/Slice/Grammar.cpp"
    break;

  case 11: /* definitions: definitions metadata definition  */
#line 249 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[0]);
    if (contained && !metadata->v.empty())
    {
        contained->appendMetadata(std::move(metadata->v));
    }
}
#line 1820 "src/Slice/Grammar.cpp"
    break;

  case 12: /* definitions: %empty  */
#line 258 "src/Slice/Grammar.y"
{
}
#line 1827 "src/Slice/Grammar.cpp"
    break;

  case 13: /* $@1: %empty  */
#line 266 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Module>(yyvsp[0]));
}
#line 1835 "src/Slice/Grammar.cpp"
    break;

  case 15: /* $@2: %empty  */
#line 271 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<ClassDecl>(yyvsp[0]));
}
#line 1843 "src/Slice/Grammar.cpp"
    break;

  case 17: /* definition: class_decl  */
#line 276 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after class forward declaration");
}
#line 1851 "src/Slice/Grammar.cpp"
    break;

  case 18: /* $@3: %empty  */
#line 280 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<ClassDef>(yyvsp[0]));
}
#line 1859 "src/Slice/Grammar.cpp"
    break;

  case 20: /* $@4: %empty  */
#line 285 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<InterfaceDecl>(yyvsp[0]));
}
#line 1867 "src/Slice/Grammar.cpp"
    break;

  case 22: /* definition: interface_decl  */
#line 290 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after interface forward declaration");
}
#line 1875 "src/Slice/Grammar.cpp"
    break;

  case 23: /* $@5: %empty  */
#line 294 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<InterfaceDef>(yyvsp[0]));
}
#line 1883 "src/Slice/Grammar.cpp"
    break;

  case 25: /* $@6: %empty  */
#line 299 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr);
}
#line 1891 "src/Slice/Grammar.cpp"
    break;

  case 27: /* definition: exception_decl  */
#line 304 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after exception forward declaration");
}
#line 1899 "src/Slice/Grammar.cpp"
    break;

  case 28: /* $@7: %empty  */
#line 308 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Exception>(yyvsp[0]));
}
#line 1907 "src/Slice/Grammar.cpp"
    break;

  case 30: /* $@8: %empty  */
#line 313 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr);
}
#line 1915 "src/Slice/Grammar.cpp"
    break;

  case 32: /* definition: struct_decl  */
#line 318 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after struct forward declaration");
}
#line 1923 "src/Slice/Grammar.cpp"
    break;

  case 33: /* $@9: %empty  */
#line 322 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Struct>(yyvsp[0]));
}
#line 1931 "src/Slice/Grammar.cpp"
    break;

  case 35: /* $@10: %empty  */
#line 327 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Sequence>(yyvsp[0]));
}
#line 1939 "src/Slice/Grammar.cpp"
    break;

  case 37: /* definition: sequence_def  */
#line 332 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after sequence definition");
}
#line 1947 "src/Slice/Grammar.cpp"
    break;

  case 38: /* $@11: %empty  */
#line 336 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Dictionary>(yyvsp[0]));
}
#line 1955 "src/Slice/Grammar.cpp"
    break;

  case 40: /* definition: dictionary_def  */
#line 341 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after dictionary definition");
}
#line 1963 "src/Slice/Grammar.cpp"
    break;

  case 41: /* $@12: %empty  */
#line 345 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Enum>(yyvsp[0]));
}
#line 1971 "src/Slice/Grammar.cpp"
    break;

  case 43: /* $@13: %empty  */
#line 350 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Const>(yyvsp[0]));
}
#line 1979 "src/Slice/Grammar.cpp"
    break;

  case 45: /* definition: const_def  */
#line 355 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after const definition");
}
#line 1987 "src/Slice/Grammar.cpp"
    break;

  case 46: /* definition: error ';'  */
#line 359 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 1995 "src/Slice/Grammar.cpp"
    break;

  case 47: /* @14: %empty  */
#line 368 "src/Slice/Grammar.y"
{
    currentUnit->setSeenDefinition();

    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ModulePtr module = cont->createModule(ident->v, false);
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
#line 2017 "src/Slice/Grammar.cpp"
    break;

  case 48: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
#line 386 "src/Slice/Grammar.y"
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
#line 2033 "src/Slice/Grammar.cpp"
    break;

  case 49: /* @15: %empty  */
#line 398 "src/Slice/Grammar.y"
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
        ModulePtr module = cont->createModule(currentModuleName, true);
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
                cont->createModule(modules[j], true); // Dummy
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
#line 2092 "src/Slice/Grammar.cpp"
    break;

  case 50: /* module_def: ICE_MODULE ICE_SCOPED_IDENTIFIER @15 '{' definitions '}'  */
#line 453 "src/Slice/Grammar.y"
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
#line 2120 "src/Slice/Grammar.cpp"
    break;

  case 51: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 482 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2128 "src/Slice/Grammar.cpp"
    break;

  case 52: /* exception_id: ICE_EXCEPTION keyword  */
#line 486 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2138 "src/Slice/Grammar.cpp"
    break;

  case 53: /* exception_decl: exception_id  */
#line 497 "src/Slice/Grammar.y"
{
    currentUnit->error("exceptions cannot be forward declared");
    yyval = nullptr;
}
#line 2147 "src/Slice/Grammar.cpp"
    break;

  case 54: /* @16: %empty  */
#line 507 "src/Slice/Grammar.y"
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
#line 2164 "src/Slice/Grammar.cpp"
    break;

  case 55: /* exception_def: exception_id exception_extends @16 '{' data_members '}'  */
#line 520 "src/Slice/Grammar.y"
{
    if (yyvsp[-3])
    {
        currentUnit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2176 "src/Slice/Grammar.cpp"
    break;

  case 56: /* exception_extends: extends scoped_name  */
#line 533 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v, true);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2188 "src/Slice/Grammar.cpp"
    break;

  case 57: /* exception_extends: %empty  */
#line 541 "src/Slice/Grammar.y"
{
    yyval = nullptr;
}
#line 2196 "src/Slice/Grammar.cpp"
    break;

  case 58: /* type_id: type ICE_IDENTIFIER  */
#line 550 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    yyval = make_shared<TypeStringTok>(type, ident->v);
}
#line 2206 "src/Slice/Grammar.cpp"
    break;

  case 59: /* optional: ICE_OPTIONAL_OPEN ICE_INTEGER_LITERAL ')'  */
#line 561 "src/Slice/Grammar.y"
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
#line 2228 "src/Slice/Grammar.cpp"
    break;

  case 60: /* optional: ICE_OPTIONAL_OPEN scoped_name ')'  */
#line 579 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    ContainerPtr cont = currentUnit->currentContainer();
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
            os << "enumerator '" << scoped->v << "' could designate";
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

                os << " '" << p->scoped() << "'";
            }
            currentUnit->error(os.str());
        }
        else
        {
            currentUnit->error(string("'") + scoped->v + "' is not defined");
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
        currentUnit->error("invalid tag '" + scoped->v + "'");
    }

    auto m = make_shared<OptionalDefTok>(tag);
    yyval = m;
}
#line 2307 "src/Slice/Grammar.cpp"
    break;

  case 61: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 654 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<OptionalDefTok>(-1); // Dummy
    yyval = m;
}
#line 2317 "src/Slice/Grammar.cpp"
    break;

  case 62: /* optional: ICE_OPTIONAL  */
#line 660 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<OptionalDefTok>(-1); // Dummy
    yyval = m;
}
#line 2327 "src/Slice/Grammar.cpp"
    break;

  case 63: /* optional_type_id: optional type_id  */
#line 671 "src/Slice/Grammar.y"
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
#line 2348 "src/Slice/Grammar.cpp"
    break;

  case 64: /* optional_type_id: type_id  */
#line 688 "src/Slice/Grammar.y"
{
    auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);
    auto m = make_shared<OptionalDefTok>(-1);
    m->type = ts->type;
    m->name = ts->name;
    yyval = m;
}
#line 2360 "src/Slice/Grammar.cpp"
    break;

  case 65: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 701 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2368 "src/Slice/Grammar.cpp"
    break;

  case 66: /* struct_id: ICE_STRUCT keyword  */
#line 705 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2378 "src/Slice/Grammar.cpp"
    break;

  case 67: /* struct_decl: struct_id  */
#line 716 "src/Slice/Grammar.y"
{
    currentUnit->error("structs cannot be forward declared");
    yyval = nullptr; // Dummy
}
#line 2387 "src/Slice/Grammar.cpp"
    break;

  case 68: /* @17: %empty  */
#line 726 "src/Slice/Grammar.y"
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
#line 2409 "src/Slice/Grammar.cpp"
    break;

  case 69: /* struct_def: struct_id @17 '{' data_members '}'  */
#line 744 "src/Slice/Grammar.y"
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
        currentUnit->error("struct '" + st->name() + "' must have at least one member"); // $$ is a dummy
    }
}
#line 2429 "src/Slice/Grammar.cpp"
    break;

  case 70: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 765 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2437 "src/Slice/Grammar.cpp"
    break;

  case 71: /* class_name: ICE_CLASS keyword  */
#line 769 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2447 "src/Slice/Grammar.cpp"
    break;

  case 72: /* class_id: ICE_CLASS ICE_IDENT_OPEN ICE_INTEGER_LITERAL ')'  */
#line 780 "src/Slice/Grammar.y"
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
            currentUnit->error("invalid compact id for class: already assigned to class '" + typeId + "'");
        }
    }

    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[-2])->v;
    classId->t = static_cast<int>(id);
    yyval = classId;
}
#line 2476 "src/Slice/Grammar.cpp"
    break;

  case 73: /* class_id: ICE_CLASS ICE_IDENT_OPEN scoped_name ')'  */
#line 805 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[-1]);

    ContainerPtr cont = currentUnit->currentContainer();
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
            os << "enumerator '" << scoped->v << "' could designate";
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

                os << " '" << p->scoped() << "'";
            }
            currentUnit->error(os.str());
        }
        else
        {
            currentUnit->error(string("'") + scoped->v + "' is not defined");
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
            currentUnit->error("invalid compact id for class: already assigned to class '" + typeId + "'");
        }
    }

    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[-2])->v;
    classId->t = id;
    yyval = classId;

}
#line 2567 "src/Slice/Grammar.cpp"
    break;

  case 74: /* class_id: class_name  */
#line 892 "src/Slice/Grammar.y"
{
    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2578 "src/Slice/Grammar.cpp"
    break;

  case 75: /* class_decl: class_name  */
#line 904 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2589 "src/Slice/Grammar.cpp"
    break;

  case 76: /* @18: %empty  */
#line 916 "src/Slice/Grammar.y"
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
#line 2610 "src/Slice/Grammar.cpp"
    break;

  case 77: /* class_def: class_id class_extends @18 '{' data_members '}'  */
#line 933 "src/Slice/Grammar.y"
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
#line 2626 "src/Slice/Grammar.cpp"
    break;

  case 78: /* class_extends: extends scoped_name  */
#line 950 "src/Slice/Grammar.y"
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
            string msg = "'";
            msg += scoped->v;
            msg += "' is not a class";
            currentUnit->error(msg);
        }
        else
        {
            ClassDefPtr def = cl->definition();
            if (!def)
            {
                string msg = "'";
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
#line 2664 "src/Slice/Grammar.cpp"
    break;

  case 79: /* class_extends: %empty  */
#line 984 "src/Slice/Grammar.y"
{
    yyval = nullptr;
}
#line 2672 "src/Slice/Grammar.cpp"
    break;

  case 80: /* extends: ICE_EXTENDS  */
#line 993 "src/Slice/Grammar.y"
{
}
#line 2679 "src/Slice/Grammar.cpp"
    break;

  case 81: /* extends: ':'  */
#line 996 "src/Slice/Grammar.y"
{
}
#line 2686 "src/Slice/Grammar.cpp"
    break;

  case 82: /* data_members: metadata data_member ';' data_members  */
#line 1004 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
    if (contained && !metadata->v.empty())
    {
        contained->appendMetadata(std::move(metadata->v));
    }
}
#line 2699 "src/Slice/Grammar.cpp"
    break;

  case 83: /* data_members: error ';' data_members  */
#line 1013 "src/Slice/Grammar.y"
{
}
#line 2706 "src/Slice/Grammar.cpp"
    break;

  case 84: /* data_members: metadata data_member  */
#line 1016 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after definition");
}
#line 2714 "src/Slice/Grammar.cpp"
    break;

  case 85: /* data_members: %empty  */
#line 1020 "src/Slice/Grammar.y"
{
}
#line 2721 "src/Slice/Grammar.cpp"
    break;

  case 86: /* data_member: optional_type_id  */
#line 1028 "src/Slice/Grammar.y"
{
    auto def = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    DataMemberPtr dm;
    if (cl)
    {
        dm = cl->createDataMember(def->name, def->type, def->isOptional, def->tag, nullptr, std::nullopt);
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        if (def->isOptional)
        {
            currentUnit->error("optional data members are not supported in structs");
            dm = st->createDataMember(def->name, def->type, false, 0, nullptr, std::nullopt); // Dummy
        }
        else
        {
            dm = st->createDataMember(def->name, def->type, false, -1, nullptr, std::nullopt);
        }
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        dm = ex->createDataMember(def->name, def->type, def->isOptional, def->tag, nullptr, std::nullopt);
    }
    currentUnit->currentContainer()->checkIntroduced(def->name, dm);
    yyval = dm;
}
#line 2755 "src/Slice/Grammar.cpp"
    break;

  case 87: /* data_member: optional_type_id '=' const_initializer  */
#line 1058 "src/Slice/Grammar.y"
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
            dm = st->createDataMember(def->name, def->type, false, 0, nullptr, std::nullopt); // Dummy
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
#line 2790 "src/Slice/Grammar.cpp"
    break;

  case 88: /* data_member: type keyword  */
#line 1089 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    if (cl)
    {
        yyval = cl->createDataMember(name, type, false, 0, nullptr, std::nullopt); // Dummy
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        yyval = st->createDataMember(name, type, false, 0, nullptr, std::nullopt); // Dummy
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        yyval = ex->createDataMember(name, type, false, 0, nullptr, std::nullopt); // Dummy
    }
    assert(yyval);
    currentUnit->error("keyword '" + name + "' cannot be used as data member name");
}
#line 2816 "src/Slice/Grammar.cpp"
    break;

  case 89: /* data_member: type  */
#line 1111 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    if (cl)
    {
        yyval = cl->createDataMember(Ice::generateUUID(), type, false, 0, nullptr, std::nullopt); // Dummy
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        yyval = st->createDataMember(Ice::generateUUID(), type, false, 0, nullptr, std::nullopt); // Dummy
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        yyval = ex->createDataMember(Ice::generateUUID(), type, false, 0, nullptr, std::nullopt); // Dummy
    }
    assert(yyval);
    currentUnit->error("missing data member name");
}
#line 2841 "src/Slice/Grammar.cpp"
    break;

  case 90: /* return_type: optional type  */
#line 1137 "src/Slice/Grammar.y"
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
#line 2860 "src/Slice/Grammar.cpp"
    break;

  case 91: /* return_type: type  */
#line 1152 "src/Slice/Grammar.y"
{
    auto m = make_shared<OptionalDefTok>(-1);
    m->type = dynamic_pointer_cast<Type>(yyvsp[0]);
    yyval = m;
}
#line 2870 "src/Slice/Grammar.cpp"
    break;

  case 92: /* return_type: ICE_VOID  */
#line 1158 "src/Slice/Grammar.y"
{
    auto m = make_shared<OptionalDefTok>(-1);
    yyval = m;
}
#line 2879 "src/Slice/Grammar.cpp"
    break;

  case 93: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1168 "src/Slice/Grammar.y"
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
        }
        yyval = op;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2903 "src/Slice/Grammar.cpp"
    break;

  case 94: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1188 "src/Slice/Grammar.y"
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
        }
        yyval = op;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2933 "src/Slice/Grammar.cpp"
    break;

  case 95: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1214 "src/Slice/Grammar.y"
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
            currentUnit->error("keyword '" + name + "' cannot be used as operation name");
        }
        yyval = op; // Dummy
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2957 "src/Slice/Grammar.cpp"
    break;

  case 96: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1234 "src/Slice/Grammar.y"
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
            currentUnit->error("keyword '" + name + "' cannot be used as operation name");
        }
        yyval = op; // Dummy
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2986 "src/Slice/Grammar.cpp"
    break;

  case 97: /* @19: %empty  */
#line 1264 "src/Slice/Grammar.y"
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
#line 3002 "src/Slice/Grammar.cpp"
    break;

  case 98: /* operation: operation_preamble parameters ')' @19 throws  */
#line 1276 "src/Slice/Grammar.y"
{
    auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
    auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    assert(el);
    if (op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3016 "src/Slice/Grammar.cpp"
    break;

  case 99: /* @20: %empty  */
#line 1286 "src/Slice/Grammar.y"
{
    if (yyvsp[-2])
    {
        currentUnit->popContainer();
    }
    yyerrok;
}
#line 3028 "src/Slice/Grammar.cpp"
    break;

  case 100: /* operation: operation_preamble error ')' @20 throws  */
#line 1294 "src/Slice/Grammar.y"
{
    auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
    auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    assert(el);
    if (op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3042 "src/Slice/Grammar.cpp"
    break;

  case 101: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1309 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3050 "src/Slice/Grammar.cpp"
    break;

  case 102: /* interface_id: ICE_INTERFACE keyword  */
#line 1313 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3060 "src/Slice/Grammar.cpp"
    break;

  case 103: /* interface_decl: interface_id  */
#line 1324 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto cont = currentUnit->currentContainer();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3072 "src/Slice/Grammar.cpp"
    break;

  case 104: /* @21: %empty  */
#line 1337 "src/Slice/Grammar.y"
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
#line 3093 "src/Slice/Grammar.cpp"
    break;

  case 105: /* interface_def: interface_id interface_extends @21 '{' operations '}'  */
#line 1354 "src/Slice/Grammar.y"
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
#line 3109 "src/Slice/Grammar.cpp"
    break;

  case 106: /* interface_list: interface_list ',' scoped_name  */
#line 1371 "src/Slice/Grammar.y"
{
    auto intfs = dynamic_pointer_cast<InterfaceListTok>(yyvsp[-2]);
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if (!types.empty())
    {
        auto interface = dynamic_pointer_cast<InterfaceDecl>(types.front());
        if (!interface)
        {
            string msg = "'";
            msg += scoped->v;
            msg += "' is not an interface";
            currentUnit->error(msg);
        }
        else
        {
            InterfaceDefPtr def = interface->definition();
            if (!def)
            {
                string msg = "'";
                msg += scoped->v;
                msg += "' has been declared but not defined";
                currentUnit->error(msg);
            }
            else
            {
                cont->checkIntroduced(scoped->v);
                intfs->v.push_back(def);
            }
        }
    }
    yyval = intfs;
}
#line 3148 "src/Slice/Grammar.cpp"
    break;

  case 107: /* interface_list: scoped_name  */
#line 1406 "src/Slice/Grammar.y"
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
            string msg = "'";
            msg += scoped->v;
            msg += "' is not an interface";
            currentUnit->error(msg); // $$ is a dummy
        }
        else
        {
            InterfaceDefPtr def = interface->definition();
            if (!def)
            {
                string msg = "'";
                msg += scoped->v;
                msg += "' has been declared but not defined";
                currentUnit->error(msg); // $$ is a dummy
            }
            else
            {
                cont->checkIntroduced(scoped->v);
                intfs->v.push_back(def);
            }
        }
    }
    yyval = intfs;
}
#line 3187 "src/Slice/Grammar.cpp"
    break;

  case 108: /* interface_list: ICE_OBJECT  */
#line 1441 "src/Slice/Grammar.y"
{
    currentUnit->error("illegal inheritance from type Object");
    yyval = make_shared<InterfaceListTok>(); // Dummy
}
#line 3196 "src/Slice/Grammar.cpp"
    break;

  case 109: /* interface_list: ICE_VALUE  */
#line 1446 "src/Slice/Grammar.y"
{
    currentUnit->error("illegal inheritance from type Value");
    yyval = make_shared<InterfaceListTok>(); // Dummy
}
#line 3205 "src/Slice/Grammar.cpp"
    break;

  case 110: /* interface_extends: extends interface_list  */
#line 1456 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3213 "src/Slice/Grammar.cpp"
    break;

  case 111: /* interface_extends: %empty  */
#line 1460 "src/Slice/Grammar.y"
{
    yyval = make_shared<InterfaceListTok>();
}
#line 3221 "src/Slice/Grammar.cpp"
    break;

  case 112: /* operations: metadata operation ';' operations  */
#line 1469 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
    if (contained && !metadata->v.empty())
    {
        contained->appendMetadata(std::move(metadata->v));
    }
}
#line 3234 "src/Slice/Grammar.cpp"
    break;

  case 113: /* operations: error ';' operations  */
#line 1478 "src/Slice/Grammar.y"
{
}
#line 3241 "src/Slice/Grammar.cpp"
    break;

  case 114: /* operations: metadata operation  */
#line 1481 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after definition");
}
#line 3249 "src/Slice/Grammar.cpp"
    break;

  case 115: /* operations: %empty  */
#line 1485 "src/Slice/Grammar.y"
{
}
#line 3256 "src/Slice/Grammar.cpp"
    break;

  case 116: /* exception_list: exception_list ',' exception  */
#line 1493 "src/Slice/Grammar.y"
{
    auto exceptionList = dynamic_pointer_cast<ExceptionListTok>(yyvsp[-2]);
    auto exception = dynamic_pointer_cast<Exception>(yyvsp[0]);
    exceptionList->v.push_back(exception);
    yyval = exceptionList;
}
#line 3267 "src/Slice/Grammar.cpp"
    break;

  case 117: /* exception_list: exception  */
#line 1500 "src/Slice/Grammar.y"
{
    auto exceptionList = make_shared<ExceptionListTok>();
    auto exception = dynamic_pointer_cast<Exception>(yyvsp[0]);
    exceptionList->v.push_back(exception);
    yyval = exceptionList;
}
#line 3278 "src/Slice/Grammar.cpp"
    break;

  case 118: /* exception: scoped_name  */
#line 1512 "src/Slice/Grammar.y"
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
#line 3294 "src/Slice/Grammar.cpp"
    break;

  case 119: /* exception: keyword  */
#line 1524 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as exception name");
    yyval = currentUnit->currentContainer()->createException(Ice::generateUUID(), 0, Dummy); // Dummy
}
#line 3304 "src/Slice/Grammar.cpp"
    break;

  case 120: /* sequence_def: ICE_SEQUENCE '<' metadata type '>' ICE_IDENTIFIER  */
#line 1535 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createSequence(ident->v, type, std::move(metadata->v));
}
#line 3316 "src/Slice/Grammar.cpp"
    break;

  case 121: /* sequence_def: ICE_SEQUENCE '<' metadata type '>' keyword  */
#line 1543 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createSequence(ident->v, type, std::move(metadata->v)); // Dummy
    currentUnit->error("keyword '" + ident->v + "' cannot be used as sequence name");
}
#line 3329 "src/Slice/Grammar.cpp"
    break;

  case 122: /* dictionary_def: ICE_DICTIONARY '<' metadata type ',' metadata type '>' ICE_IDENTIFIER  */
#line 1557 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto keyMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-6]);
    auto keyType = dynamic_pointer_cast<Type>(yyvsp[-5]);
    auto valueMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto valueType = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, std::move(keyMetadata->v), valueType, std::move(valueMetadata->v));
}
#line 3343 "src/Slice/Grammar.cpp"
    break;

  case 123: /* dictionary_def: ICE_DICTIONARY '<' metadata type ',' metadata type '>' keyword  */
#line 1567 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto keyMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-6]);
    auto keyType = dynamic_pointer_cast<Type>(yyvsp[-5]);
    auto valueMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto valueType = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, std::move(keyMetadata->v), valueType, std::move(valueMetadata->v)); // Dummy
    currentUnit->error("keyword '" + ident->v + "' cannot be used as dictionary name");
}
#line 3358 "src/Slice/Grammar.cpp"
    break;

  case 124: /* enum_id: ICE_ENUM ICE_IDENTIFIER  */
#line 1583 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3366 "src/Slice/Grammar.cpp"
    break;

  case 125: /* enum_id: ICE_ENUM keyword  */
#line 1587 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3376 "src/Slice/Grammar.cpp"
    break;

  case 126: /* @22: %empty  */
#line 1598 "src/Slice/Grammar.y"
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
#line 3396 "src/Slice/Grammar.cpp"
    break;

  case 127: /* enum_def: enum_id @22 '{' enumerators '}'  */
#line 1614 "src/Slice/Grammar.y"
{
    auto en = dynamic_pointer_cast<Enum>(yyvsp[-3]);
    if (en)
    {
        auto enumerators = dynamic_pointer_cast<EnumeratorListTok>(yyvsp[-1]);
        if (enumerators->v.empty())
        {
            currentUnit->error("enum '" + en->name() + "' must have at least one enumerator");
        }
        currentUnit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 3414 "src/Slice/Grammar.cpp"
    break;

  case 128: /* @23: %empty  */
#line 1629 "src/Slice/Grammar.y"
{
    currentUnit->error("missing enumeration name");
    ContainerPtr cont = currentUnit->currentContainer();
    EnumPtr en = cont->createEnum(Ice::generateUUID(), Dummy);
    currentUnit->pushContainer(en);
    yyval = en;
}
#line 3426 "src/Slice/Grammar.cpp"
    break;

  case 129: /* enum_def: ICE_ENUM @23 '{' enumerators '}'  */
#line 1637 "src/Slice/Grammar.y"
{
    currentUnit->popContainer();
    yyval = yyvsp[-4];
}
#line 3435 "src/Slice/Grammar.cpp"
    break;

  case 131: /* enumerators: enumerator_list ','  */
#line 1648 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 3443 "src/Slice/Grammar.cpp"
    break;

  case 132: /* enumerators: %empty  */
#line 1652 "src/Slice/Grammar.y"
{
    yyval = make_shared<EnumeratorListTok>(); // Empty list
}
#line 3451 "src/Slice/Grammar.cpp"
    break;

  case 133: /* enumerator_list: enumerator_list ',' metadata enumerator  */
#line 1661 "src/Slice/Grammar.y"
{
    auto enumeratorList = dynamic_pointer_cast<EnumeratorListTok>(yyvsp[-3]);
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto enumerator = dynamic_pointer_cast<Enumerator>(yyvsp[0]);
    if (enumerator && !metadata->v.empty())
    {
        enumerator->appendMetadata(std::move(metadata->v));
    }
    enumeratorList->v.push_back(enumerator);
    yyval = enumeratorList;
}
#line 3467 "src/Slice/Grammar.cpp"
    break;

  case 134: /* enumerator_list: metadata enumerator  */
#line 1673 "src/Slice/Grammar.y"
{
    auto enumeratorList = make_shared<EnumeratorListTok>();
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto enumerator = dynamic_pointer_cast<Enumerator>(yyvsp[0]);
    if (enumerator && !metadata->v.empty())
    {
        enumerator->appendMetadata(std::move(metadata->v));
    }
    enumeratorList->v.push_back(enumerator);
    yyval = enumeratorList;
}
#line 3483 "src/Slice/Grammar.cpp"
    break;

  case 135: /* enumerator: ICE_IDENTIFIER  */
#line 1690 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    yyval = cont->createEnumerator(ident->v, nullopt);
}
#line 3493 "src/Slice/Grammar.cpp"
    break;

  case 136: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1696 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
    if (intVal)
    {
        if (intVal->v < 0 || intVal->v > std::numeric_limits<int32_t>::max())
        {
            currentUnit->error("value for enumerator '" + ident->v + "' is out of range");
        }
        yyval = cont->createEnumerator(ident->v, static_cast<int>(intVal->v));
    }
    else
    {
        yyval = cont->createEnumerator(ident->v, nullopt); // Dummy
    }
}
#line 3515 "src/Slice/Grammar.cpp"
    break;

  case 137: /* enumerator: keyword  */
#line 1714 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    currentUnit->error("keyword '" + ident->v + "' cannot be used as enumerator");
    yyval = cont->createEnumerator(ident->v, nullopt); // Dummy
}
#line 3526 "src/Slice/Grammar.cpp"
    break;

  case 138: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1726 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3534 "src/Slice/Grammar.cpp"
    break;

  case 139: /* enumerator_initializer: scoped_name  */
#line 1730 "src/Slice/Grammar.y"
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
        string msg = "illegal initializer: '" + scoped->v + "' is not an integer constant";
        currentUnit->error(msg); // $$ is dummy
    }

    yyval = tok;
}
#line 3574 "src/Slice/Grammar.cpp"
    break;

  case 140: /* out_qualifier: ICE_OUT  */
#line 1771 "src/Slice/Grammar.y"
{
    auto out = make_shared<BoolTok>();
    out->v = true;
    yyval = out;
}
#line 3584 "src/Slice/Grammar.cpp"
    break;

  case 141: /* out_qualifier: %empty  */
#line 1777 "src/Slice/Grammar.y"
{
    auto out = make_shared<BoolTok>();
    out->v = false;
    yyval = out;
}
#line 3594 "src/Slice/Grammar.cpp"
    break;

  case 144: /* parameter_list: out_qualifier metadata optional_type_id  */
#line 1795 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto tsp = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        ParameterPtr pd = op->createParameter(tsp->name, tsp->type, isOutParam->v, tsp->isOptional, tsp->tag);
        currentUnit->currentContainer()->checkIntroduced(tsp->name, pd);
        auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
        if (!metadata->v.empty())
        {
            pd->appendMetadata(std::move(metadata->v));
        }
    }
}
#line 3614 "src/Slice/Grammar.cpp"
    break;

  case 145: /* parameter_list: parameter_list ',' out_qualifier metadata optional_type_id  */
#line 1811 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto tsp = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        ParameterPtr pd = op->createParameter(tsp->name, tsp->type, isOutParam->v, tsp->isOptional, tsp->tag);
        currentUnit->currentContainer()->checkIntroduced(tsp->name, pd);
        auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
        if (!metadata->v.empty())
        {
            pd->appendMetadata(std::move(metadata->v));
        }
    }
}
#line 3634 "src/Slice/Grammar.cpp"
    break;

  case 146: /* parameter_list: out_qualifier metadata type keyword  */
#line 1827 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParameter(ident->v, type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("keyword '" + ident->v + "' cannot be used as parameter name");
    }
}
#line 3650 "src/Slice/Grammar.cpp"
    break;

  case 147: /* parameter_list: parameter_list ',' out_qualifier metadata type keyword  */
#line 1839 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParameter(ident->v, type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("keyword '" + ident->v + "' cannot be used as parameter name");
    }
}
#line 3666 "src/Slice/Grammar.cpp"
    break;

  case 148: /* parameter_list: out_qualifier metadata type  */
#line 1851 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParameter(Ice::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("missing parameter name");
    }
}
#line 3681 "src/Slice/Grammar.cpp"
    break;

  case 149: /* parameter_list: parameter_list ',' out_qualifier metadata type  */
#line 1862 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParameter(Ice::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("missing parameter name");
    }
}
#line 3696 "src/Slice/Grammar.cpp"
    break;

  case 150: /* throws: ICE_THROWS exception_list  */
#line 1878 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3704 "src/Slice/Grammar.cpp"
    break;

  case 151: /* throws: %empty  */
#line 1882 "src/Slice/Grammar.y"
{
    yyval = make_shared<ExceptionListTok>();
}
#line 3712 "src/Slice/Grammar.cpp"
    break;

  case 152: /* scoped_name: ICE_IDENTIFIER  */
#line 1891 "src/Slice/Grammar.y"
{
}
#line 3719 "src/Slice/Grammar.cpp"
    break;

  case 153: /* scoped_name: ICE_SCOPED_IDENTIFIER  */
#line 1894 "src/Slice/Grammar.y"
{
}
#line 3726 "src/Slice/Grammar.cpp"
    break;

  case 154: /* builtin: ICE_BOOL  */
#line 1901 "src/Slice/Grammar.y"
           {}
#line 3732 "src/Slice/Grammar.cpp"
    break;

  case 155: /* builtin: ICE_BYTE  */
#line 1902 "src/Slice/Grammar.y"
           {}
#line 3738 "src/Slice/Grammar.cpp"
    break;

  case 156: /* builtin: ICE_SHORT  */
#line 1903 "src/Slice/Grammar.y"
            {}
#line 3744 "src/Slice/Grammar.cpp"
    break;

  case 157: /* builtin: ICE_INT  */
#line 1904 "src/Slice/Grammar.y"
          {}
#line 3750 "src/Slice/Grammar.cpp"
    break;

  case 158: /* builtin: ICE_LONG  */
#line 1905 "src/Slice/Grammar.y"
           {}
#line 3756 "src/Slice/Grammar.cpp"
    break;

  case 159: /* builtin: ICE_FLOAT  */
#line 1906 "src/Slice/Grammar.y"
            {}
#line 3762 "src/Slice/Grammar.cpp"
    break;

  case 160: /* builtin: ICE_DOUBLE  */
#line 1907 "src/Slice/Grammar.y"
             {}
#line 3768 "src/Slice/Grammar.cpp"
    break;

  case 161: /* builtin: ICE_STRING  */
#line 1908 "src/Slice/Grammar.y"
             {}
#line 3774 "src/Slice/Grammar.cpp"
    break;

  case 162: /* builtin: ICE_OBJECT  */
#line 1909 "src/Slice/Grammar.y"
             {}
#line 3780 "src/Slice/Grammar.cpp"
    break;

  case 163: /* builtin: ICE_VALUE  */
#line 1910 "src/Slice/Grammar.y"
            {}
#line 3786 "src/Slice/Grammar.cpp"
    break;

  case 164: /* type: ICE_OBJECT '*'  */
#line 1916 "src/Slice/Grammar.y"
{
    yyval = currentUnit->createBuiltin(Builtin::KindObjectProxy);
}
#line 3794 "src/Slice/Grammar.cpp"
    break;

  case 165: /* type: builtin  */
#line 1920 "src/Slice/Grammar.y"
{
    auto typeName = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    yyval = currentUnit->createBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 3803 "src/Slice/Grammar.cpp"
    break;

  case 166: /* type: scoped_name  */
#line 1925 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();

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
#line 3832 "src/Slice/Grammar.cpp"
    break;

  case 167: /* type: scoped_name '*'  */
#line 1950 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    ContainerPtr cont = currentUnit->currentContainer();

    TypeList types = cont->lookupType(scoped->v);
    if (types.empty())
    {
        YYERROR; // Can't continue, jump to next yyerrok
    }
    TypePtr firstType = types.front();

    auto interface = dynamic_pointer_cast<InterfaceDecl>(firstType);
    if (!interface)
    {
        string msg = "'";
        msg += scoped->v;
        msg += "' must be an interface";
        currentUnit->error(msg);
        YYERROR; // Can't continue, jump to next yyerrok
    }
    cont->checkIntroduced(scoped->v);

    yyval = firstType;
}
#line 3861 "src/Slice/Grammar.cpp"
    break;

  case 168: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 1980 "src/Slice/Grammar.y"
{
    auto str1 = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    auto str2 = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    str1->v += str2->v;
}
#line 3871 "src/Slice/Grammar.cpp"
    break;

  case 169: /* string_literal: ICE_STRING_LITERAL  */
#line 1986 "src/Slice/Grammar.y"
{
}
#line 3878 "src/Slice/Grammar.cpp"
    break;

  case 170: /* metadata_list: metadata_list ',' string_literal  */
#line 1994 "src/Slice/Grammar.y"
{
    auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadataList = dynamic_pointer_cast<MetadataListTok>(yyvsp[-2]);

    auto metadata = make_shared<Metadata>(str->v, currentUnit->currentFile(), currentUnit->currentLine());
    metadataList->v.push_back(metadata);

    yyval = metadataList;
}
#line 3892 "src/Slice/Grammar.cpp"
    break;

  case 171: /* metadata_list: string_literal  */
#line 2004 "src/Slice/Grammar.y"
{
    auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadataList = make_shared<MetadataListTok>();

    auto metadata = make_shared<Metadata>(str->v, currentUnit->currentFile(), currentUnit->currentLine());
    metadataList->v.push_back(metadata);

    yyval = metadataList;
}
#line 3906 "src/Slice/Grammar.cpp"
    break;

  case 172: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2019 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindLong);
    auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    auto def = make_shared<ConstDefTok>(type, sstr.str());
    yyval = def;
}
#line 3919 "src/Slice/Grammar.cpp"
    break;

  case 173: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2028 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindDouble);
    auto floatVal = dynamic_pointer_cast<FloatingTok>(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    auto def = make_shared<ConstDefTok>(type, sstr.str());
    yyval = def;
}
#line 3932 "src/Slice/Grammar.cpp"
    break;

  case 174: /* const_initializer: scoped_name  */
#line 2037 "src/Slice/Grammar.y"
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
            string msg = "illegal initializer: '" + scoped->v + "' is ";
            string kindOf = cl.front()->kindOf();
            msg += getArticleFor(kindOf) + " " + kindOf;
            currentUnit->error(msg); // $$ is dummy
        }
    }
    yyval = def;
}
#line 3971 "src/Slice/Grammar.cpp"
    break;

  case 175: /* const_initializer: ICE_STRING_LITERAL  */
#line 2072 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindString);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, literal->v);
    yyval = def;
}
#line 3982 "src/Slice/Grammar.cpp"
    break;

  case 176: /* const_initializer: ICE_FALSE  */
#line 2079 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindBool);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, "false");
    yyval = def;
}
#line 3993 "src/Slice/Grammar.cpp"
    break;

  case 177: /* const_initializer: ICE_TRUE  */
#line 2086 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindBool);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, "true");
    yyval = def;
}
#line 4004 "src/Slice/Grammar.cpp"
    break;

  case 178: /* const_def: ICE_CONST metadata type ICE_IDENTIFIER '=' const_initializer  */
#line 2098 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-4]);
    auto const_type = dynamic_pointer_cast<Type>(yyvsp[-3]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    yyval = currentUnit->currentContainer()->createConst(ident->v, const_type, std::move(metadata->v), value->v,
                                                      value->valueAsString);
}
#line 4017 "src/Slice/Grammar.cpp"
    break;

  case 179: /* const_def: ICE_CONST metadata type '=' const_initializer  */
#line 2107 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto const_type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    currentUnit->error("missing constant name");
    yyval = currentUnit->currentContainer()->createConst(Ice::generateUUID(), const_type, std::move(metadata->v),
                                                      value->v, value->valueAsString, Dummy); // Dummy
}
#line 4030 "src/Slice/Grammar.cpp"
    break;

  case 180: /* keyword: ICE_MODULE  */
#line 2120 "src/Slice/Grammar.y"
             {}
#line 4036 "src/Slice/Grammar.cpp"
    break;

  case 181: /* keyword: ICE_CLASS  */
#line 2121 "src/Slice/Grammar.y"
            {}
#line 4042 "src/Slice/Grammar.cpp"
    break;

  case 182: /* keyword: ICE_INTERFACE  */
#line 2122 "src/Slice/Grammar.y"
                {}
#line 4048 "src/Slice/Grammar.cpp"
    break;

  case 183: /* keyword: ICE_EXCEPTION  */
#line 2123 "src/Slice/Grammar.y"
                {}
#line 4054 "src/Slice/Grammar.cpp"
    break;

  case 184: /* keyword: ICE_STRUCT  */
#line 2124 "src/Slice/Grammar.y"
             {}
#line 4060 "src/Slice/Grammar.cpp"
    break;

  case 185: /* keyword: ICE_SEQUENCE  */
#line 2125 "src/Slice/Grammar.y"
               {}
#line 4066 "src/Slice/Grammar.cpp"
    break;

  case 186: /* keyword: ICE_DICTIONARY  */
#line 2126 "src/Slice/Grammar.y"
                 {}
#line 4072 "src/Slice/Grammar.cpp"
    break;

  case 187: /* keyword: ICE_ENUM  */
#line 2127 "src/Slice/Grammar.y"
           {}
#line 4078 "src/Slice/Grammar.cpp"
    break;

  case 188: /* keyword: ICE_OUT  */
#line 2128 "src/Slice/Grammar.y"
          {}
#line 4084 "src/Slice/Grammar.cpp"
    break;

  case 189: /* keyword: ICE_EXTENDS  */
#line 2129 "src/Slice/Grammar.y"
              {}
#line 4090 "src/Slice/Grammar.cpp"
    break;

  case 190: /* keyword: ICE_THROWS  */
#line 2130 "src/Slice/Grammar.y"
             {}
#line 4096 "src/Slice/Grammar.cpp"
    break;

  case 191: /* keyword: ICE_VOID  */
#line 2131 "src/Slice/Grammar.y"
           {}
#line 4102 "src/Slice/Grammar.cpp"
    break;

  case 192: /* keyword: ICE_BOOL  */
#line 2132 "src/Slice/Grammar.y"
           {}
#line 4108 "src/Slice/Grammar.cpp"
    break;

  case 193: /* keyword: ICE_BYTE  */
#line 2133 "src/Slice/Grammar.y"
           {}
#line 4114 "src/Slice/Grammar.cpp"
    break;

  case 194: /* keyword: ICE_SHORT  */
#line 2134 "src/Slice/Grammar.y"
            {}
#line 4120 "src/Slice/Grammar.cpp"
    break;

  case 195: /* keyword: ICE_INT  */
#line 2135 "src/Slice/Grammar.y"
          {}
#line 4126 "src/Slice/Grammar.cpp"
    break;

  case 196: /* keyword: ICE_LONG  */
#line 2136 "src/Slice/Grammar.y"
           {}
#line 4132 "src/Slice/Grammar.cpp"
    break;

  case 197: /* keyword: ICE_FLOAT  */
#line 2137 "src/Slice/Grammar.y"
            {}
#line 4138 "src/Slice/Grammar.cpp"
    break;

  case 198: /* keyword: ICE_DOUBLE  */
#line 2138 "src/Slice/Grammar.y"
             {}
#line 4144 "src/Slice/Grammar.cpp"
    break;

  case 199: /* keyword: ICE_STRING  */
#line 2139 "src/Slice/Grammar.y"
             {}
#line 4150 "src/Slice/Grammar.cpp"
    break;

  case 200: /* keyword: ICE_OBJECT  */
#line 2140 "src/Slice/Grammar.y"
             {}
#line 4156 "src/Slice/Grammar.cpp"
    break;

  case 201: /* keyword: ICE_CONST  */
#line 2141 "src/Slice/Grammar.y"
            {}
#line 4162 "src/Slice/Grammar.cpp"
    break;

  case 202: /* keyword: ICE_FALSE  */
#line 2142 "src/Slice/Grammar.y"
            {}
#line 4168 "src/Slice/Grammar.cpp"
    break;

  case 203: /* keyword: ICE_TRUE  */
#line 2143 "src/Slice/Grammar.y"
           {}
#line 4174 "src/Slice/Grammar.cpp"
    break;

  case 204: /* keyword: ICE_IDEMPOTENT  */
#line 2144 "src/Slice/Grammar.y"
                 {}
#line 4180 "src/Slice/Grammar.cpp"
    break;

  case 205: /* keyword: ICE_OPTIONAL  */
#line 2145 "src/Slice/Grammar.y"
               {}
#line 4186 "src/Slice/Grammar.cpp"
    break;

  case 206: /* keyword: ICE_VALUE  */
#line 2146 "src/Slice/Grammar.y"
            {}
#line 4192 "src/Slice/Grammar.cpp"
    break;


#line 4196 "src/Slice/Grammar.cpp"

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

#line 2149 "src/Slice/Grammar.y"


// NOLINTEND
