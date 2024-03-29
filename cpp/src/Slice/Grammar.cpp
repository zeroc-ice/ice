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


//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Included first to get 'TokenContext' which we need to define YYLTYPE before flex does.
#include "GrammarUtil.h"

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


#line 109 "src/Slice/Grammar.cpp"

/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yydebug         slice_debug
#define yynerrs         slice_nerrs

/* First part of user prologue.  */
#line 66 "src/Slice/Grammar.y"


#include "IceUtil/UUID.h"

#include <cstring>
#include <limits>

#ifdef _MSC_VER
// warning C4127: conditional expression is constant
#    pragma warning(disable:4127)
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


#line 168 "src/Slice/Grammar.cpp"

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
  YYSYMBOL_ICE_TAG = 28,                   /* ICE_TAG  */
  YYSYMBOL_ICE_OPTIONAL = 29,              /* ICE_OPTIONAL  */
  YYSYMBOL_ICE_VALUE = 30,                 /* ICE_VALUE  */
  YYSYMBOL_ICE_STRING_LITERAL = 31,        /* ICE_STRING_LITERAL  */
  YYSYMBOL_ICE_INTEGER_LITERAL = 32,       /* ICE_INTEGER_LITERAL  */
  YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 33, /* ICE_FLOATING_POINT_LITERAL  */
  YYSYMBOL_ICE_IDENTIFIER = 34,            /* ICE_IDENTIFIER  */
  YYSYMBOL_ICE_SCOPED_IDENTIFIER = 35,     /* ICE_SCOPED_IDENTIFIER  */
  YYSYMBOL_ICE_METADATA_OPEN = 36,         /* ICE_METADATA_OPEN  */
  YYSYMBOL_ICE_METADATA_CLOSE = 37,        /* ICE_METADATA_CLOSE  */
  YYSYMBOL_ICE_GLOBAL_METADATA_OPEN = 38,  /* ICE_GLOBAL_METADATA_OPEN  */
  YYSYMBOL_ICE_GLOBAL_METADATA_IGNORE = 39, /* ICE_GLOBAL_METADATA_IGNORE  */
  YYSYMBOL_ICE_GLOBAL_METADATA_CLOSE = 40, /* ICE_GLOBAL_METADATA_CLOSE  */
  YYSYMBOL_ICE_IDENT_OPEN = 41,            /* ICE_IDENT_OPEN  */
  YYSYMBOL_ICE_KEYWORD_OPEN = 42,          /* ICE_KEYWORD_OPEN  */
  YYSYMBOL_ICE_TAG_OPEN = 43,              /* ICE_TAG_OPEN  */
  YYSYMBOL_ICE_OPTIONAL_OPEN = 44,         /* ICE_OPTIONAL_OPEN  */
  YYSYMBOL_BAD_CHAR = 45,                  /* BAD_CHAR  */
  YYSYMBOL_46_ = 46,                       /* ';'  */
  YYSYMBOL_47_ = 47,                       /* '{'  */
  YYSYMBOL_48_ = 48,                       /* '}'  */
  YYSYMBOL_49_ = 49,                       /* ')'  */
  YYSYMBOL_50_ = 50,                       /* ':'  */
  YYSYMBOL_51_ = 51,                       /* '='  */
  YYSYMBOL_52_ = 52,                       /* ','  */
  YYSYMBOL_53_ = 53,                       /* '<'  */
  YYSYMBOL_54_ = 54,                       /* '>'  */
  YYSYMBOL_55_ = 55,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 56,                  /* $accept  */
  YYSYMBOL_start = 57,                     /* start  */
  YYSYMBOL_opt_semicolon = 58,             /* opt_semicolon  */
  YYSYMBOL_global_meta_data = 59,          /* global_meta_data  */
  YYSYMBOL_meta_data = 60,                 /* meta_data  */
  YYSYMBOL_definitions = 61,               /* definitions  */
  YYSYMBOL_definition = 62,                /* definition  */
  YYSYMBOL_63_1 = 63,                      /* $@1  */
  YYSYMBOL_64_2 = 64,                      /* $@2  */
  YYSYMBOL_65_3 = 65,                      /* $@3  */
  YYSYMBOL_66_4 = 66,                      /* $@4  */
  YYSYMBOL_67_5 = 67,                      /* $@5  */
  YYSYMBOL_68_6 = 68,                      /* $@6  */
  YYSYMBOL_69_7 = 69,                      /* $@7  */
  YYSYMBOL_70_8 = 70,                      /* $@8  */
  YYSYMBOL_71_9 = 71,                      /* $@9  */
  YYSYMBOL_72_10 = 72,                     /* $@10  */
  YYSYMBOL_73_11 = 73,                     /* $@11  */
  YYSYMBOL_74_12 = 74,                     /* $@12  */
  YYSYMBOL_75_13 = 75,                     /* $@13  */
  YYSYMBOL_module_def = 76,                /* module_def  */
  YYSYMBOL_77_14 = 77,                     /* @14  */
  YYSYMBOL_exception_id = 78,              /* exception_id  */
  YYSYMBOL_exception_decl = 79,            /* exception_decl  */
  YYSYMBOL_exception_def = 80,             /* exception_def  */
  YYSYMBOL_81_15 = 81,                     /* @15  */
  YYSYMBOL_exception_extends = 82,         /* exception_extends  */
  YYSYMBOL_type_id = 83,                   /* type_id  */
  YYSYMBOL_tag = 84,                       /* tag  */
  YYSYMBOL_optional = 85,                  /* optional  */
  YYSYMBOL_tagged_type_id = 86,            /* tagged_type_id  */
  YYSYMBOL_struct_id = 87,                 /* struct_id  */
  YYSYMBOL_struct_decl = 88,               /* struct_decl  */
  YYSYMBOL_struct_def = 89,                /* struct_def  */
  YYSYMBOL_90_16 = 90,                     /* @16  */
  YYSYMBOL_class_name = 91,                /* class_name  */
  YYSYMBOL_class_id = 92,                  /* class_id  */
  YYSYMBOL_class_decl = 93,                /* class_decl  */
  YYSYMBOL_class_def = 94,                 /* class_def  */
  YYSYMBOL_95_17 = 95,                     /* @17  */
  YYSYMBOL_class_extends = 96,             /* class_extends  */
  YYSYMBOL_extends = 97,                   /* extends  */
  YYSYMBOL_data_members = 98,              /* data_members  */
  YYSYMBOL_data_member = 99,               /* data_member  */
  YYSYMBOL_return_type = 100,              /* return_type  */
  YYSYMBOL_operation_preamble = 101,       /* operation_preamble  */
  YYSYMBOL_operation = 102,                /* operation  */
  YYSYMBOL_103_18 = 103,                   /* @18  */
  YYSYMBOL_104_19 = 104,                   /* @19  */
  YYSYMBOL_interface_id = 105,             /* interface_id  */
  YYSYMBOL_interface_decl = 106,           /* interface_decl  */
  YYSYMBOL_interface_def = 107,            /* interface_def  */
  YYSYMBOL_108_20 = 108,                   /* @20  */
  YYSYMBOL_interface_list = 109,           /* interface_list  */
  YYSYMBOL_interface_extends = 110,        /* interface_extends  */
  YYSYMBOL_operations = 111,               /* operations  */
  YYSYMBOL_exception_list = 112,           /* exception_list  */
  YYSYMBOL_exception = 113,                /* exception  */
  YYSYMBOL_sequence_def = 114,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 115,           /* dictionary_def  */
  YYSYMBOL_enum_id = 116,                  /* enum_id  */
  YYSYMBOL_enum_def = 117,                 /* enum_def  */
  YYSYMBOL_118_21 = 118,                   /* @21  */
  YYSYMBOL_119_22 = 119,                   /* @22  */
  YYSYMBOL_enumerator_list = 120,          /* enumerator_list  */
  YYSYMBOL_enumerator = 121,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 122,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 123,            /* out_qualifier  */
  YYSYMBOL_parameters = 124,               /* parameters  */
  YYSYMBOL_throws = 125,                   /* throws  */
  YYSYMBOL_scoped_name = 126,              /* scoped_name  */
  YYSYMBOL_builtin = 127,                  /* builtin  */
  YYSYMBOL_type = 128,                     /* type  */
  YYSYMBOL_string_literal = 129,           /* string_literal  */
  YYSYMBOL_string_list = 130,              /* string_list  */
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


#line 344 "src/Slice/Grammar.cpp"

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
#define YYLAST   674

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  56
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  78
/* YYNRULES -- Number of rules.  */
#define YYNRULES  207
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  300

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   300


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
       2,    49,    55,     2,    52,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    50,    46,
      53,    51,    54,     2,     2,     2,     2,     2,     2,     2,
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
       0,   179,   179,   187,   190,   198,   202,   212,   216,   225,
     233,   242,   251,   250,   256,   255,   260,   265,   264,   270,
     269,   274,   279,   278,   284,   283,   288,   293,   292,   298,
     297,   302,   307,   306,   312,   311,   316,   321,   320,   325,
     330,   329,   335,   334,   339,   343,   353,   352,   385,   389,
     400,   411,   410,   436,   444,   453,   466,   484,   563,   569,
     580,   598,   676,   682,   693,   702,   711,   724,   728,   739,
     750,   749,   788,   792,   803,   828,   918,   930,   943,   942,
     976,  1010,  1019,  1022,  1030,  1039,  1042,  1046,  1054,  1084,
    1118,  1140,  1166,  1172,  1178,  1184,  1194,  1218,  1248,  1272,
    1307,  1306,  1329,  1328,  1351,  1355,  1366,  1380,  1379,  1413,
    1448,  1483,  1488,  1498,  1502,  1511,  1520,  1523,  1527,  1535,
    1542,  1554,  1566,  1577,  1585,  1599,  1609,  1625,  1629,  1641,
    1640,  1672,  1671,  1689,  1695,  1703,  1715,  1735,  1742,  1752,
    1756,  1797,  1803,  1814,  1817,  1833,  1849,  1861,  1873,  1884,
    1900,  1904,  1913,  1916,  1924,  1925,  1926,  1927,  1928,  1929,
    1930,  1931,  1932,  1933,  1938,  1942,  1947,  1978,  2014,  2020,
    2028,  2035,  2047,  2056,  2065,  2100,  2107,  2114,  2126,  2135,
    2149,  2150,  2151,  2152,  2153,  2154,  2155,  2156,  2157,  2158,
    2159,  2160,  2161,  2162,  2163,  2164,  2165,  2166,  2167,  2168,
    2169,  2170,  2171,  2172,  2173,  2174,  2175,  2176
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
  "ICE_CONST", "ICE_FALSE", "ICE_TRUE", "ICE_IDEMPOTENT", "ICE_TAG",
  "ICE_OPTIONAL", "ICE_VALUE", "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENTIFIER", "ICE_SCOPED_IDENTIFIER",
  "ICE_METADATA_OPEN", "ICE_METADATA_CLOSE", "ICE_GLOBAL_METADATA_OPEN",
  "ICE_GLOBAL_METADATA_IGNORE", "ICE_GLOBAL_METADATA_CLOSE",
  "ICE_IDENT_OPEN", "ICE_KEYWORD_OPEN", "ICE_TAG_OPEN",
  "ICE_OPTIONAL_OPEN", "BAD_CHAR", "';'", "'{'", "'}'", "')'", "':'",
  "'='", "','", "'<'", "'>'", "'*'", "$accept", "start", "opt_semicolon",
  "global_meta_data", "meta_data", "definitions", "definition", "$@1",
  "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
  "$@12", "$@13", "module_def", "@14", "exception_id", "exception_decl",
  "exception_def", "@15", "exception_extends", "type_id", "tag",
  "optional", "tagged_type_id", "struct_id", "struct_decl", "struct_def",
  "@16", "class_name", "class_id", "class_decl", "class_def", "@17",
  "class_extends", "extends", "data_members", "data_member", "return_type",
  "operation_preamble", "operation", "@18", "@19", "interface_id",
  "interface_decl", "interface_def", "@20", "interface_list",
  "interface_extends", "operations", "exception_list", "exception",
  "sequence_def", "dictionary_def", "enum_id", "enum_def", "@21", "@22",
  "enumerator_list", "enumerator", "enumerator_initializer",
  "out_qualifier", "parameters", "throws", "scoped_name", "builtin",
  "type", "string_literal", "string_list", "const_initializer",
  "const_def", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-256)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-144)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -256,    16,    28,  -256,    -6,    -6,    -6,  -256,   643,    -6,
    -256,    22,   -29,   -28,    37,     2,   283,   355,   387,   419,
      36,    49,   451,    87,  -256,  -256,    18,    92,  -256,    94,
     100,  -256,    41,   -10,   102,  -256,    45,   122,  -256,   125,
     126,  -256,  -256,   131,  -256,  -256,    -6,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,
     105,  -256,  -256,  -256,  -256,  -256,  -256,  -256,    87,    87,
    -256,   133,  -256,   639,   136,  -256,  -256,  -256,    42,   142,
     136,   150,   153,   136,  -256,    42,   156,   136,    50,  -256,
     160,   136,   162,   173,   174,   136,   179,  -256,   180,   171,
    -256,  -256,   177,   639,   639,   483,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,   176,  -256,   178,  -256,   -17,  -256,
    -256,   185,  -256,  -256,  -256,   194,  -256,  -256,   187,  -256,
    -256,  -256,  -256,  -256,  -256,   183,   189,  -256,  -256,  -256,
    -256,   483,  -256,  -256,  -256,  -256,  -256,   186,   191,   188,
     196,   193,  -256,  -256,  -256,   195,    73,   194,   201,   599,
     200,   194,    50,    99,   212,   155,   515,    87,   141,  -256,
     483,    73,  -256,  -256,  -256,  -256,  -256,  -256,  -256,   213,
     194,  -256,  -256,    47,   152,  -256,   639,   639,   216,   226,
     547,  -256,   225,  -256,   229,   235,   228,  -256,  -256,  -256,
    -256,   639,  -256,  -256,  -256,  -256,  -256,  -256,  -256,   231,
    -256,   232,   233,  -256,   265,  -256,   243,  -256,    73,   194,
    -256,  -256,  -256,    99,  -256,   135,   639,   639,    69,    26,
     269,  -256,  -256,   262,  -256,  -256,  -256,  -256,  -256,  -256,
    -256,    90,  -256,  -256,  -256,  -256,   270,  -256,    87,   -34,
      99,   579,  -256,  -256,  -256,   599,  -256,   307,  -256,  -256,
    -256,   308,  -256,   547,   308,    87,   322,  -256,  -256,  -256,
     599,  -256,   268,  -256,  -256,  -256,   547,   322,  -256,  -256
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      11,     0,     8,     1,     0,     0,     0,     9,     0,   169,
     171,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   131,     8,    10,    12,    50,    26,    27,    69,
      31,    32,    77,    81,    16,    17,   106,    21,    22,    36,
      39,   129,    40,    44,   168,     7,     0,     5,     6,    45,
      46,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,    72,
       0,    73,   104,   105,    48,    49,    67,    68,     8,     8,
     127,     0,   128,     0,     4,    82,    83,    51,     0,     0,
       4,     0,     0,     4,    78,     0,     0,     4,     0,   107,
       0,     4,     0,     0,     0,     4,     0,   170,     0,     0,
     152,   153,     0,     0,     0,   138,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   166,   165,     0,     3,
      13,     0,    53,    25,    28,     0,    30,    33,     0,    80,
      15,    18,   111,   112,   113,   110,     0,    20,    23,    35,
      38,   138,    41,    43,    11,    74,    75,     0,     0,   135,
       0,   134,   137,   164,   167,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     8,     0,     8,     0,   132,
     138,     0,   176,   177,   175,   172,   173,   174,   179,     0,
       0,    59,    63,     0,     0,    66,     0,     0,    88,    86,
      91,    71,     0,   109,     0,     0,     0,   130,    47,   123,
     124,     0,   139,   136,   140,   133,   178,    52,    85,     0,
      58,     0,     0,    62,     0,    64,     0,    65,     0,     0,
      55,    90,    79,     0,    95,     0,     0,     0,     0,     0,
     117,    94,   108,     0,    56,    57,    60,    61,    89,    84,
     116,     0,    92,    93,    96,    98,     0,   141,     8,     0,
       0,     0,    97,    99,   102,     0,   100,   142,   115,   125,
     126,   151,   144,   148,   151,     8,     0,   103,   146,   101,
       0,   150,   120,   121,   122,   145,   149,     0,   147,   119
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -256,  -256,    89,  -256,    -2,   158,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,  -256,  -256,  -256,   -62,  -193,  -182,
    -255,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,  -256,
    -256,    57,  -168,  -256,    78,  -256,  -256,  -256,  -256,  -256,
    -256,  -256,  -256,   172,  -256,  -212,    56,  -256,  -256,  -256,
    -256,  -256,  -256,  -256,   -89,  -256,  -256,   109,  -256,    71,
     -79,  -256,   -85,     5,   161,  -188,  -256,   -12
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   140,     7,   179,     2,    24,    94,   106,   107,
     110,   111,    99,   100,   102,   103,   112,   113,   115,   116,
      25,   118,    26,    27,    28,   141,    97,   205,   206,   207,
     208,    29,    30,    31,   101,    32,    33,    34,    35,   148,
     104,    98,   180,   209,   248,   249,   250,   284,   281,    36,
      37,    38,   156,   154,   109,   216,   291,   292,    39,    40,
      41,    42,   114,    91,   170,   171,   223,   268,   269,   287,
     136,   137,   236,    10,    11,   198,    43,   172
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       8,   122,    95,   226,    81,    83,    85,    87,   138,   199,
      92,    47,    48,   212,    44,   276,     3,   175,   277,   142,
     282,    93,   246,    46,    46,     9,   149,   266,    -2,   155,
      95,   260,   228,   247,   176,   295,    50,   267,   167,   168,
      96,  -142,  -142,  -142,  -142,  -142,  -142,  -142,  -142,  -142,
     258,   117,   246,   -76,  -142,  -142,  -142,    95,   278,    45,
    -142,  -142,  -142,   247,     4,   -54,     5,     6,    96,  -142,
    -142,   259,   184,   152,    46,  -143,   120,   121,  -143,   229,
     153,   120,   121,    49,   120,   121,   123,   124,   -76,    88,
     105,   -76,  -114,   108,   210,    96,   230,   197,   192,   193,
     214,   225,    89,   155,   194,   195,   196,   120,   121,   224,
     264,   265,   197,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,     4,   231,   234,    -8,    -8,    -8,    -8,
     251,   272,   273,    -8,    -8,     4,   253,   119,   -24,   120,
     121,   -70,    -8,    -8,   235,   237,   -29,  -118,   -14,   244,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   197,
     251,   262,   263,   201,   202,   135,    12,    13,   -19,   120,
     121,   -34,   -37,   222,   220,   120,   121,   -42,   203,   204,
     125,   215,   139,     8,   232,   221,   120,   121,   143,   144,
     283,     4,   147,     5,     6,   178,   151,   145,   241,   146,
     158,   233,   150,   218,   162,   296,   157,   293,   159,    -8,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,   293,   160,
     165,   161,    -8,    -8,    -8,   163,   166,   164,    -8,    -8,
       4,   173,   177,   174,   181,   182,   183,    -8,    -8,   188,
     186,   215,   -87,   187,   189,   190,   191,   200,   211,   244,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   280,
     217,   227,   245,   201,   202,   135,   275,   238,   215,   120,
     121,   288,   239,   242,   294,   243,   252,   240,   203,   204,
     254,   255,   256,   290,   298,   294,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,   257,   270,   271,    79,   267,   274,
     297,   286,   185,   261,    80,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,   299,   213,   289,   120,   121,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,   285,     0,     0,    82,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
       0,    84,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,     0,    86,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,     0,     0,     0,    90,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,     0,   169,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     0,     0,     0,   219,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
       0,   240,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,     0,   279,   126,   127,   128,   129,   130,   131,
     132,   133,   134,     0,     0,     0,     0,   201,   202,   135,
       0,     0,     0,   120,   121,     0,     0,     0,     0,     0,
       0,     0,   203,   204,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,   126,   127,   128,   129,   130,   131,
     132,   133,   134,     0,     0,     0,     0,    23,     0,   135,
       0,     0,     0,   120,   121
};

static const yytype_int16 yycheck[] =
{
       2,    80,    12,   191,    16,    17,    18,    19,    93,   177,
      22,    40,    40,   181,     9,    49,     0,    34,    52,    98,
     275,    23,   215,    52,    52,    31,   105,     1,     0,   108,
      12,   243,   200,   215,    51,   290,    34,    11,   123,   124,
      50,    15,    16,    17,    18,    19,    20,    21,    22,    23,
     238,    46,   245,    12,    28,    29,    30,    12,   270,    37,
      34,    35,    36,   245,    36,    47,    38,    39,    50,    43,
      44,   239,   161,    23,    52,    49,    34,    35,    52,    32,
      30,    34,    35,    46,    34,    35,    88,    89,    47,    53,
      33,    50,    47,    36,   179,    50,    49,   176,    25,    26,
       1,   190,    53,   182,    31,    32,    33,    34,    35,   188,
      41,    42,   191,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    36,   203,   204,    27,    28,    29,    30,
     215,    41,    42,    34,    35,    36,   221,    32,    46,    34,
      35,    47,    43,    44,   206,   207,    46,    48,    46,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,   238,
     245,   246,   247,    28,    29,    30,     5,     6,    46,    34,
      35,    46,    46,    32,   186,    34,    35,    46,    43,    44,
      47,   183,    46,   185,    32,   187,    34,    35,    46,   100,
     275,    36,   103,    38,    39,     1,   107,    47,   210,    46,
     111,    49,    46,    48,   115,   290,    46,   286,    46,    15,
      16,    17,    18,    19,    20,    21,    22,    23,   297,    46,
      49,    47,    28,    29,    30,    46,    49,    47,    34,    35,
      36,    55,    47,    55,    47,    52,    47,    43,    44,    51,
      54,   243,    48,    52,    48,    52,    51,    46,    48,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,   271,
      48,    48,    27,    28,    29,    30,   268,    51,   270,    34,
      35,   283,    46,    48,   286,    46,    48,    34,    43,    44,
      49,    49,    49,   285,   296,   297,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    49,    46,    54,    34,    11,    49,
      52,    13,   164,   245,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,   297,   182,   284,    34,    35,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,   277,    -1,    -1,    34,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    -1,
      -1,    34,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    -1,    -1,    34,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    -1,    -1,    34,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    -1,    -1,    34,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    -1,    -1,    -1,    34,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    -1,
      -1,    34,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      -1,    -1,    -1,    34,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    -1,    28,    29,    30,
      -1,    -1,    -1,    34,    35,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,     1,    -1,     3,     4,     5,     6,
       7,     8,     9,    10,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    -1,    24,    -1,    30,
      -1,    -1,    -1,    34,    35
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    57,    61,     0,    36,    38,    39,    59,    60,    31,
     129,   130,   130,   130,     1,     3,     4,     5,     6,     7,
       8,     9,    10,    24,    62,    76,    78,    79,    80,    87,
      88,    89,    91,    92,    93,    94,   105,   106,   107,   114,
     115,   116,   117,   132,   129,    37,    52,    40,    40,    46,
      34,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    34,
      41,   133,    34,   133,    34,   133,    34,   133,    53,    53,
      34,   119,   133,    60,    63,    12,    50,    82,    97,    68,
      69,    90,    70,    71,    96,    97,    64,    65,    97,   110,
      66,    67,    72,    73,   118,    74,    75,   129,    77,    32,
      34,    35,   126,    60,    60,    47,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    30,   126,   127,   128,    46,
      58,    81,   126,    46,    58,    47,    46,    58,    95,   126,
      46,    58,    23,    30,   109,   126,   108,    46,    58,    46,
      46,    47,    58,    46,    47,    49,    49,   128,   128,    34,
     120,   121,   133,    55,    55,    34,    51,    47,     1,    60,
      98,    47,    52,    47,   120,    61,    54,    52,    51,    48,
      52,    51,    25,    26,    31,    32,    33,   126,   131,    98,
      46,    28,    29,    43,    44,    83,    84,    85,    86,    99,
     128,    48,    98,   109,     1,    60,   111,    48,    48,    34,
     133,    60,    32,   122,   126,   120,   131,    48,    98,    32,
      49,   126,    32,    49,   126,    83,   128,    83,    51,    46,
      34,   133,    48,    46,    14,    27,    84,    85,   100,   101,
     102,   128,    48,   128,    49,    49,    49,    49,   131,    98,
     111,   100,   128,   128,    41,    42,     1,    11,   123,   124,
      46,    54,    41,    42,    49,    60,    49,    52,   111,    34,
     133,   104,    86,   128,   103,   123,    13,   125,   133,   125,
      60,   112,   113,   126,   133,    86,   128,    52,   133,   112
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    56,    57,    58,    58,    59,    59,    60,    60,    61,
      61,    61,    63,    62,    64,    62,    62,    65,    62,    66,
      62,    62,    67,    62,    68,    62,    62,    69,    62,    70,
      62,    62,    71,    62,    72,    62,    62,    73,    62,    62,
      74,    62,    75,    62,    62,    62,    77,    76,    78,    78,
      79,    81,    80,    82,    82,    83,    84,    84,    84,    84,
      85,    85,    85,    85,    86,    86,    86,    87,    87,    88,
      90,    89,    91,    91,    92,    92,    92,    93,    95,    94,
      96,    96,    97,    97,    98,    98,    98,    98,    99,    99,
      99,    99,   100,   100,   100,   100,   101,   101,   101,   101,
     103,   102,   104,   102,   105,   105,   106,   108,   107,   109,
     109,   109,   109,   110,   110,   111,   111,   111,   111,   112,
     112,   113,   113,   114,   114,   115,   115,   116,   116,   118,
     117,   119,   117,   120,   120,   121,   121,   121,   121,   122,
     122,   123,   123,   124,   124,   124,   124,   124,   124,   124,
     125,   125,   126,   126,   127,   127,   127,   127,   127,   127,
     127,   127,   127,   127,   128,   128,   128,   128,   129,   129,
     130,   130,   131,   131,   131,   131,   131,   131,   132,   132,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     3,     0,     2,
       3,     0,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     1,
       0,     3,     0,     3,     1,     2,     0,     6,     2,     2,
       1,     0,     6,     2,     0,     2,     3,     3,     2,     1,
       3,     3,     2,     1,     2,     2,     1,     2,     2,     1,
       0,     5,     2,     2,     4,     4,     1,     1,     0,     6,
       2,     0,     1,     1,     4,     3,     2,     0,     1,     3,
       2,     1,     2,     2,     1,     1,     2,     3,     2,     3,
       0,     5,     0,     5,     2,     2,     1,     0,     6,     3,
       1,     1,     1,     2,     0,     4,     3,     2,     0,     3,
       1,     1,     1,     6,     6,     9,     9,     2,     2,     0,
       5,     0,     5,     3,     1,     1,     3,     1,     0,     1,
       1,     1,     0,     0,     3,     5,     4,     6,     3,     5,
       2,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     2,     2,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     6,     5,
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
#line 180 "src/Slice/Grammar.y"
{
}
#line 1735 "src/Slice/Grammar.cpp"
    break;

  case 3: /* opt_semicolon: ';'  */
#line 188 "src/Slice/Grammar.y"
{
}
#line 1742 "src/Slice/Grammar.cpp"
    break;

  case 4: /* opt_semicolon: %empty  */
#line 191 "src/Slice/Grammar.y"
{
}
#line 1749 "src/Slice/Grammar.cpp"
    break;

  case 5: /* global_meta_data: ICE_GLOBAL_METADATA_OPEN string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 199 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1757 "src/Slice/Grammar.cpp"
    break;

  case 6: /* global_meta_data: ICE_GLOBAL_METADATA_IGNORE string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 203 "src/Slice/Grammar.y"
{
    currentUnit->error("global metadata must appear before any definitions");
    yyval = yyvsp[-1]; // Dummy
}
#line 1766 "src/Slice/Grammar.cpp"
    break;

  case 7: /* meta_data: ICE_METADATA_OPEN string_list ICE_METADATA_CLOSE  */
#line 213 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1774 "src/Slice/Grammar.cpp"
    break;

  case 8: /* meta_data: %empty  */
#line 217 "src/Slice/Grammar.y"
{
    yyval = make_shared<StringListTok>();
}
#line 1782 "src/Slice/Grammar.cpp"
    break;

  case 9: /* definitions: definitions global_meta_data  */
#line 226 "src/Slice/Grammar.y"
{
    auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[0]);
    if (!metaData->v.empty())
    {
        currentUnit->addGlobalMetaData(metaData->v);
    }
}
#line 1794 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: definitions meta_data definition  */
#line 234 "src/Slice/Grammar.y"
{
    auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-1]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[0]);
    if (contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1807 "src/Slice/Grammar.cpp"
    break;

  case 11: /* definitions: %empty  */
#line 243 "src/Slice/Grammar.y"
{
}
#line 1814 "src/Slice/Grammar.cpp"
    break;

  case 12: /* $@1: %empty  */
#line 251 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Module>(yyvsp[0]));
}
#line 1822 "src/Slice/Grammar.cpp"
    break;

  case 14: /* $@2: %empty  */
#line 256 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<ClassDecl>(yyvsp[0]));
}
#line 1830 "src/Slice/Grammar.cpp"
    break;

  case 16: /* definition: class_decl  */
#line 261 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after class forward declaration");
}
#line 1838 "src/Slice/Grammar.cpp"
    break;

  case 17: /* $@3: %empty  */
#line 265 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<ClassDef>(yyvsp[0]));
}
#line 1846 "src/Slice/Grammar.cpp"
    break;

  case 19: /* $@4: %empty  */
#line 270 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<InterfaceDecl>(yyvsp[0]));
}
#line 1854 "src/Slice/Grammar.cpp"
    break;

  case 21: /* definition: interface_decl  */
#line 275 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after interface forward declaration");
}
#line 1862 "src/Slice/Grammar.cpp"
    break;

  case 22: /* $@5: %empty  */
#line 279 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<InterfaceDef>(yyvsp[0]));
}
#line 1870 "src/Slice/Grammar.cpp"
    break;

  case 24: /* $@6: %empty  */
#line 284 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr);
}
#line 1878 "src/Slice/Grammar.cpp"
    break;

  case 26: /* definition: exception_decl  */
#line 289 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after exception forward declaration");
}
#line 1886 "src/Slice/Grammar.cpp"
    break;

  case 27: /* $@7: %empty  */
#line 293 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Exception>(yyvsp[0]));
}
#line 1894 "src/Slice/Grammar.cpp"
    break;

  case 29: /* $@8: %empty  */
#line 298 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr);
}
#line 1902 "src/Slice/Grammar.cpp"
    break;

  case 31: /* definition: struct_decl  */
#line 303 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after struct forward declaration");
}
#line 1910 "src/Slice/Grammar.cpp"
    break;

  case 32: /* $@9: %empty  */
#line 307 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Struct>(yyvsp[0]));
}
#line 1918 "src/Slice/Grammar.cpp"
    break;

  case 34: /* $@10: %empty  */
#line 312 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Sequence>(yyvsp[0]));
}
#line 1926 "src/Slice/Grammar.cpp"
    break;

  case 36: /* definition: sequence_def  */
#line 317 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after sequence definition");
}
#line 1934 "src/Slice/Grammar.cpp"
    break;

  case 37: /* $@11: %empty  */
#line 321 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Dictionary>(yyvsp[0]));
}
#line 1942 "src/Slice/Grammar.cpp"
    break;

  case 39: /* definition: dictionary_def  */
#line 326 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after dictionary definition");
}
#line 1950 "src/Slice/Grammar.cpp"
    break;

  case 40: /* $@12: %empty  */
#line 330 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Enum>(yyvsp[0]));
}
#line 1958 "src/Slice/Grammar.cpp"
    break;

  case 42: /* $@13: %empty  */
#line 335 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Const>(yyvsp[0]));
}
#line 1966 "src/Slice/Grammar.cpp"
    break;

  case 44: /* definition: const_def  */
#line 340 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after const definition");
}
#line 1974 "src/Slice/Grammar.cpp"
    break;

  case 45: /* definition: error ';'  */
#line 344 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 1982 "src/Slice/Grammar.cpp"
    break;

  case 46: /* @14: %empty  */
#line 353 "src/Slice/Grammar.y"
{
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
#line 2002 "src/Slice/Grammar.cpp"
    break;

  case 47: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
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
#line 2018 "src/Slice/Grammar.cpp"
    break;

  case 48: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 386 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2026 "src/Slice/Grammar.cpp"
    break;

  case 49: /* exception_id: ICE_EXCEPTION keyword  */
#line 390 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2036 "src/Slice/Grammar.cpp"
    break;

  case 50: /* exception_decl: exception_id  */
#line 401 "src/Slice/Grammar.y"
{
    currentUnit->error("exceptions cannot be forward declared");
    yyval = nullptr;
}
#line 2045 "src/Slice/Grammar.cpp"
    break;

  case 51: /* @15: %empty  */
#line 411 "src/Slice/Grammar.y"
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
#line 2062 "src/Slice/Grammar.cpp"
    break;

  case 52: /* exception_def: exception_id exception_extends @15 '{' data_members '}'  */
#line 424 "src/Slice/Grammar.y"
{
    if (yyvsp[-3])
    {
        currentUnit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2074 "src/Slice/Grammar.cpp"
    break;

  case 53: /* exception_extends: extends scoped_name  */
#line 437 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2086 "src/Slice/Grammar.cpp"
    break;

  case 54: /* exception_extends: %empty  */
#line 445 "src/Slice/Grammar.y"
{
    yyval = nullptr;
}
#line 2094 "src/Slice/Grammar.cpp"
    break;

  case 55: /* type_id: type ICE_IDENTIFIER  */
#line 454 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto typestring = make_shared<TypeStringTok>();
    typestring->v = make_pair(type, ident->v);
    yyval = typestring;
}
#line 2106 "src/Slice/Grammar.cpp"
    break;

  case 56: /* tag: ICE_TAG_OPEN ICE_INTEGER_LITERAL ')'  */
#line 467 "src/Slice/Grammar.y"
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

    auto m = make_shared<TaggedDefTok>(tag);
    yyval = m;
}
#line 2128 "src/Slice/Grammar.cpp"
    break;

  case 57: /* tag: ICE_TAG_OPEN scoped_name ')'  */
#line 485 "src/Slice/Grammar.y"
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
            currentUnit->warning(Deprecated, string("referencing enumerator `") + scoped->v
                          + "' without its enumeration's scope is deprecated");
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

    auto m = make_shared<TaggedDefTok>(tag);
    yyval = m;
}
#line 2211 "src/Slice/Grammar.cpp"
    break;

  case 58: /* tag: ICE_TAG_OPEN ')'  */
#line 564 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<TaggedDefTok>(-1); // Dummy
    yyval = m;
}
#line 2221 "src/Slice/Grammar.cpp"
    break;

  case 59: /* tag: ICE_TAG  */
#line 570 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<TaggedDefTok>(-1); // Dummy
    yyval = m;
}
#line 2231 "src/Slice/Grammar.cpp"
    break;

  case 60: /* optional: ICE_OPTIONAL_OPEN ICE_INTEGER_LITERAL ')'  */
#line 581 "src/Slice/Grammar.y"
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

    auto m = make_shared<TaggedDefTok>(tag);
    yyval = m;
}
#line 2253 "src/Slice/Grammar.cpp"
    break;

  case 61: /* optional: ICE_OPTIONAL_OPEN scoped_name ')'  */
#line 599 "src/Slice/Grammar.y"
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
            currentUnit->warning(Deprecated, string("referencing enumerator `") + scoped->v
                          + "' without its enumeration's scope is deprecated");
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

    auto m = make_shared<TaggedDefTok>(tag);
    yyval = m;
}
#line 2335 "src/Slice/Grammar.cpp"
    break;

  case 62: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 677 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<TaggedDefTok>(-1); // Dummy
    yyval = m;
}
#line 2345 "src/Slice/Grammar.cpp"
    break;

  case 63: /* optional: ICE_OPTIONAL  */
#line 683 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<TaggedDefTok>(-1); // Dummy
    yyval = m;
}
#line 2355 "src/Slice/Grammar.cpp"
    break;

  case 64: /* tagged_type_id: tag type_id  */
#line 694 "src/Slice/Grammar.y"
{
    auto m = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-1]);
    auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);

    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2368 "src/Slice/Grammar.cpp"
    break;

  case 65: /* tagged_type_id: optional type_id  */
#line 703 "src/Slice/Grammar.y"
{
    auto m = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-1]);
    auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);

    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2381 "src/Slice/Grammar.cpp"
    break;

  case 66: /* tagged_type_id: type_id  */
#line 712 "src/Slice/Grammar.y"
{
    auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);
    auto m = make_shared<TaggedDefTok>(-1);
    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2393 "src/Slice/Grammar.cpp"
    break;

  case 67: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 725 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2401 "src/Slice/Grammar.cpp"
    break;

  case 68: /* struct_id: ICE_STRUCT keyword  */
#line 729 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2411 "src/Slice/Grammar.cpp"
    break;

  case 69: /* struct_decl: struct_id  */
#line 740 "src/Slice/Grammar.y"
{
    currentUnit->error("structs cannot be forward declared");
    yyval = nullptr; // Dummy
}
#line 2420 "src/Slice/Grammar.cpp"
    break;

  case 70: /* @16: %empty  */
#line 750 "src/Slice/Grammar.y"
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
        st = cont->createStruct(IceUtil::generateUUID()); // Dummy
        assert(st);
        currentUnit->pushContainer(st);
    }
    yyval = st;
}
#line 2442 "src/Slice/Grammar.cpp"
    break;

  case 71: /* struct_def: struct_id @16 '{' data_members '}'  */
#line 768 "src/Slice/Grammar.y"
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
#line 2462 "src/Slice/Grammar.cpp"
    break;

  case 72: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 789 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2470 "src/Slice/Grammar.cpp"
    break;

  case 73: /* class_name: ICE_CLASS keyword  */
#line 793 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2480 "src/Slice/Grammar.cpp"
    break;

  case 74: /* class_id: ICE_CLASS ICE_IDENT_OPEN ICE_INTEGER_LITERAL ')'  */
#line 804 "src/Slice/Grammar.y"
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
#line 2509 "src/Slice/Grammar.cpp"
    break;

  case 75: /* class_id: ICE_CLASS ICE_IDENT_OPEN scoped_name ')'  */
#line 829 "src/Slice/Grammar.y"
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
            currentUnit->warning(Deprecated, string("referencing enumerator `") + scoped->v
                          + "' without its enumeration's scope is deprecated");
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
#line 2603 "src/Slice/Grammar.cpp"
    break;

  case 76: /* class_id: class_name  */
#line 919 "src/Slice/Grammar.y"
{
    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2614 "src/Slice/Grammar.cpp"
    break;

  case 77: /* class_decl: class_name  */
#line 931 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2625 "src/Slice/Grammar.cpp"
    break;

  case 78: /* @17: %empty  */
#line 943 "src/Slice/Grammar.y"
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
#line 2646 "src/Slice/Grammar.cpp"
    break;

  case 79: /* class_def: class_id class_extends @17 '{' data_members '}'  */
#line 960 "src/Slice/Grammar.y"
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
#line 2662 "src/Slice/Grammar.cpp"
    break;

  case 80: /* class_extends: extends scoped_name  */
#line 977 "src/Slice/Grammar.y"
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
#line 2700 "src/Slice/Grammar.cpp"
    break;

  case 81: /* class_extends: %empty  */
#line 1011 "src/Slice/Grammar.y"
{
    yyval = nullptr;
}
#line 2708 "src/Slice/Grammar.cpp"
    break;

  case 82: /* extends: ICE_EXTENDS  */
#line 1020 "src/Slice/Grammar.y"
{
}
#line 2715 "src/Slice/Grammar.cpp"
    break;

  case 83: /* extends: ':'  */
#line 1023 "src/Slice/Grammar.y"
{
}
#line 2722 "src/Slice/Grammar.cpp"
    break;

  case 84: /* data_members: meta_data data_member ';' data_members  */
#line 1031 "src/Slice/Grammar.y"
{
    auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
    if (contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2735 "src/Slice/Grammar.cpp"
    break;

  case 85: /* data_members: error ';' data_members  */
#line 1040 "src/Slice/Grammar.y"
{
}
#line 2742 "src/Slice/Grammar.cpp"
    break;

  case 86: /* data_members: meta_data data_member  */
#line 1043 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after definition");
}
#line 2750 "src/Slice/Grammar.cpp"
    break;

  case 87: /* data_members: %empty  */
#line 1047 "src/Slice/Grammar.y"
{
}
#line 2757 "src/Slice/Grammar.cpp"
    break;

  case 88: /* data_member: tagged_type_id  */
#line 1055 "src/Slice/Grammar.y"
{
    auto def = dynamic_pointer_cast<TaggedDefTok>(yyvsp[0]);
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    DataMemberPtr dm;
    if (cl)
    {
        dm = cl->createDataMember(def->name, def->type, def->isTagged, def->tag, 0, "", "");
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        if (def->isTagged)
        {
            currentUnit->error("tagged data members are not supported in structs");
            dm = st->createDataMember(def->name, def->type, false, 0, 0, "", ""); // Dummy
        }
        else
        {
            dm = st->createDataMember(def->name, def->type, false, -1, 0, "", "");
        }
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        dm = ex->createDataMember(def->name, def->type, def->isTagged, def->tag, 0, "", "");
    }
    currentUnit->currentContainer()->checkIntroduced(def->name, dm);
    yyval = dm;
}
#line 2791 "src/Slice/Grammar.cpp"
    break;

  case 89: /* data_member: tagged_type_id '=' const_initializer  */
#line 1085 "src/Slice/Grammar.y"
{
    auto def = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    DataMemberPtr dm;
    if (cl)
    {
        dm = cl->createDataMember(def->name, def->type, def->isTagged, def->tag, value->v,
                                  value->valueAsString, value->valueAsLiteral);
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        if (def->isTagged)
        {
            currentUnit->error("tagged data members are not supported in structs");
            dm = st->createDataMember(def->name, def->type, false, 0, 0, "", ""); // Dummy
        }
        else
        {
            dm = st->createDataMember(def->name, def->type, false, -1, value->v,
                                      value->valueAsString, value->valueAsLiteral);
        }
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        dm = ex->createDataMember(def->name, def->type, def->isTagged, def->tag, value->v,
                                  value->valueAsString, value->valueAsLiteral);
    }
    currentUnit->currentContainer()->checkIntroduced(def->name, dm);
    yyval = dm;
}
#line 2829 "src/Slice/Grammar.cpp"
    break;

  case 90: /* data_member: type keyword  */
#line 1119 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    if (cl)
    {
        yyval = cl->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        yyval = ex->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    }
    assert(yyval);
    currentUnit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 2855 "src/Slice/Grammar.cpp"
    break;

  case 91: /* data_member: type  */
#line 1141 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
    if (cl)
    {
        yyval = cl->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
    if (st)
    {
        yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
    if (ex)
    {
        yyval = ex->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    }
    assert(yyval);
    currentUnit->error("missing data member name");
}
#line 2880 "src/Slice/Grammar.cpp"
    break;

  case 92: /* return_type: tag type  */
#line 1167 "src/Slice/Grammar.y"
{
    auto m = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-1]);
    m->type = dynamic_pointer_cast<Type>(yyvsp[0]);
    yyval = m;
}
#line 2890 "src/Slice/Grammar.cpp"
    break;

  case 93: /* return_type: optional type  */
#line 1173 "src/Slice/Grammar.y"
{
    auto m = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-1]);
    m->type = dynamic_pointer_cast<Type>(yyvsp[0]);
    yyval = m;
}
#line 2900 "src/Slice/Grammar.cpp"
    break;

  case 94: /* return_type: type  */
#line 1179 "src/Slice/Grammar.y"
{
    auto m = make_shared<TaggedDefTok>(-1);
    m->type = dynamic_pointer_cast<Type>(yyvsp[0]);
    yyval = m;
}
#line 2910 "src/Slice/Grammar.cpp"
    break;

  case 95: /* return_type: ICE_VOID  */
#line 1185 "src/Slice/Grammar.y"
{
    auto m = make_shared<TaggedDefTok>(-1);
    yyval = m;
}
#line 2919 "src/Slice/Grammar.cpp"
    break;

  case 96: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1195 "src/Slice/Grammar.y"
{
    auto returnType = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
    if (interface)
    {
        OperationPtr op = interface->createOperation(name, returnType->type, returnType->isTagged, returnType->tag);
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
#line 2947 "src/Slice/Grammar.cpp"
    break;

  case 97: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1219 "src/Slice/Grammar.y"
{
    auto returnType = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
    if (interface)
    {
        OperationPtr op = interface->createOperation(
            name,
            returnType->type,
            returnType->isTagged,
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
#line 2981 "src/Slice/Grammar.cpp"
    break;

  case 98: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1249 "src/Slice/Grammar.y"
{
    auto returnType = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
    if (interface)
    {
        OperationPtr op = interface->createOperation(name, returnType->type, returnType->isTagged, returnType->tag);
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
#line 3009 "src/Slice/Grammar.cpp"
    break;

  case 99: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1273 "src/Slice/Grammar.y"
{
    auto returnType = dynamic_pointer_cast<TaggedDefTok>(yyvsp[-1]);
    string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
    if (interface)
    {
        OperationPtr op = interface->createOperation(
            name,
            returnType->type,
            returnType->isTagged,
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
#line 3042 "src/Slice/Grammar.cpp"
    break;

  case 100: /* @18: %empty  */
#line 1307 "src/Slice/Grammar.y"
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
#line 3058 "src/Slice/Grammar.cpp"
    break;

  case 101: /* operation: operation_preamble parameters ')' @18 throws  */
#line 1319 "src/Slice/Grammar.y"
{
    auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
    auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    assert(el);
    if (op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3072 "src/Slice/Grammar.cpp"
    break;

  case 102: /* @19: %empty  */
#line 1329 "src/Slice/Grammar.y"
{
    if (yyvsp[-2])
    {
        currentUnit->popContainer();
    }
    yyerrok;
}
#line 3084 "src/Slice/Grammar.cpp"
    break;

  case 103: /* operation: operation_preamble error ')' @19 throws  */
#line 1337 "src/Slice/Grammar.y"
{
    auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
    auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    assert(el);
    if (op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3098 "src/Slice/Grammar.cpp"
    break;

  case 104: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1352 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3106 "src/Slice/Grammar.cpp"
    break;

  case 105: /* interface_id: ICE_INTERFACE keyword  */
#line 1356 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3116 "src/Slice/Grammar.cpp"
    break;

  case 106: /* interface_decl: interface_id  */
#line 1367 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto cont = currentUnit->currentContainer();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3128 "src/Slice/Grammar.cpp"
    break;

  case 107: /* @20: %empty  */
#line 1380 "src/Slice/Grammar.y"
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
#line 3149 "src/Slice/Grammar.cpp"
    break;

  case 108: /* interface_def: interface_id interface_extends @20 '{' operations '}'  */
#line 1397 "src/Slice/Grammar.y"
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
#line 3165 "src/Slice/Grammar.cpp"
    break;

  case 109: /* interface_list: scoped_name ',' interface_list  */
#line 1414 "src/Slice/Grammar.y"
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
#line 3204 "src/Slice/Grammar.cpp"
    break;

  case 110: /* interface_list: scoped_name  */
#line 1449 "src/Slice/Grammar.y"
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
#line 3243 "src/Slice/Grammar.cpp"
    break;

  case 111: /* interface_list: ICE_OBJECT  */
#line 1484 "src/Slice/Grammar.y"
{
    currentUnit->error("illegal inheritance from type Object");
    yyval = make_shared<InterfaceListTok>(); // Dummy
}
#line 3252 "src/Slice/Grammar.cpp"
    break;

  case 112: /* interface_list: ICE_VALUE  */
#line 1489 "src/Slice/Grammar.y"
{
    currentUnit->error("illegal inheritance from type Value");
    yyval = make_shared<ClassListTok>(); // Dummy
}
#line 3261 "src/Slice/Grammar.cpp"
    break;

  case 113: /* interface_extends: extends interface_list  */
#line 1499 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3269 "src/Slice/Grammar.cpp"
    break;

  case 114: /* interface_extends: %empty  */
#line 1503 "src/Slice/Grammar.y"
{
    yyval = make_shared<InterfaceListTok>();
}
#line 3277 "src/Slice/Grammar.cpp"
    break;

  case 115: /* operations: meta_data operation ';' operations  */
#line 1512 "src/Slice/Grammar.y"
{
    auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
    if (contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3290 "src/Slice/Grammar.cpp"
    break;

  case 116: /* operations: error ';' operations  */
#line 1521 "src/Slice/Grammar.y"
{
}
#line 3297 "src/Slice/Grammar.cpp"
    break;

  case 117: /* operations: meta_data operation  */
#line 1524 "src/Slice/Grammar.y"
{
    currentUnit->error("`;' missing after definition");
}
#line 3305 "src/Slice/Grammar.cpp"
    break;

  case 118: /* operations: %empty  */
#line 1528 "src/Slice/Grammar.y"
{
}
#line 3312 "src/Slice/Grammar.cpp"
    break;

  case 119: /* exception_list: exception ',' exception_list  */
#line 1536 "src/Slice/Grammar.y"
{
    auto exception = dynamic_pointer_cast<Exception>(yyvsp[-2]);
    auto exceptionList = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3323 "src/Slice/Grammar.cpp"
    break;

  case 120: /* exception_list: exception  */
#line 1543 "src/Slice/Grammar.y"
{
    auto exception = dynamic_pointer_cast<Exception>(yyvsp[0]);
    auto exceptionList = make_shared<ExceptionListTok>();
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3334 "src/Slice/Grammar.cpp"
    break;

  case 121: /* exception: scoped_name  */
#line 1555 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if (!exception)
    {
        exception = cont->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    yyval = exception;
}
#line 3350 "src/Slice/Grammar.cpp"
    break;

  case 122: /* exception: keyword  */
#line 1567 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = currentUnit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3360 "src/Slice/Grammar.cpp"
    break;

  case 123: /* sequence_def: ICE_SEQUENCE '<' meta_data type '>' ICE_IDENTIFIER  */
#line 1578 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3372 "src/Slice/Grammar.cpp"
    break;

  case 124: /* sequence_def: ICE_SEQUENCE '<' meta_data type '>' keyword  */
#line 1586 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    currentUnit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3385 "src/Slice/Grammar.cpp"
    break;

  case 125: /* dictionary_def: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' ICE_IDENTIFIER  */
#line 1600 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto keyMetaData = dynamic_pointer_cast<StringListTok>(yyvsp[-6]);
    auto keyType = dynamic_pointer_cast<Type>(yyvsp[-5]);
    auto valueMetaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
    auto valueType = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3399 "src/Slice/Grammar.cpp"
    break;

  case 126: /* dictionary_def: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' keyword  */
#line 1610 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto keyMetaData = dynamic_pointer_cast<StringListTok>(yyvsp[-6]);
    auto keyType = dynamic_pointer_cast<Type>(yyvsp[-5]);
    auto valueMetaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
    auto valueType = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v); // Dummy
    currentUnit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3414 "src/Slice/Grammar.cpp"
    break;

  case 127: /* enum_id: ICE_ENUM ICE_IDENTIFIER  */
#line 1626 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3422 "src/Slice/Grammar.cpp"
    break;

  case 128: /* enum_id: ICE_ENUM keyword  */
#line 1630 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3432 "src/Slice/Grammar.cpp"
    break;

  case 129: /* @21: %empty  */
#line 1641 "src/Slice/Grammar.y"
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
        en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    }
    currentUnit->pushContainer(en);
    yyval = en;
}
#line 3452 "src/Slice/Grammar.cpp"
    break;

  case 130: /* enum_def: enum_id @21 '{' enumerator_list '}'  */
#line 1657 "src/Slice/Grammar.y"
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
#line 3470 "src/Slice/Grammar.cpp"
    break;

  case 131: /* @22: %empty  */
#line 1672 "src/Slice/Grammar.y"
{
    currentUnit->error("missing enumeration name");
    ContainerPtr cont = currentUnit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    currentUnit->pushContainer(en);
    yyval = en;
}
#line 3482 "src/Slice/Grammar.cpp"
    break;

  case 132: /* enum_def: ICE_ENUM @22 '{' enumerator_list '}'  */
#line 1680 "src/Slice/Grammar.y"
{
    currentUnit->popContainer();
    yyval = yyvsp[-4];
}
#line 3491 "src/Slice/Grammar.cpp"
    break;

  case 133: /* enumerator_list: enumerator ',' enumerator_list  */
#line 1690 "src/Slice/Grammar.y"
{
    auto ens = dynamic_pointer_cast<EnumeratorListTok>(yyvsp[-2]);
    ens->v.splice(ens->v.end(), dynamic_pointer_cast<EnumeratorListTok>(yyvsp[0])->v);
    yyval = ens;
}
#line 3501 "src/Slice/Grammar.cpp"
    break;

  case 134: /* enumerator_list: enumerator  */
#line 1696 "src/Slice/Grammar.y"
{
}
#line 3508 "src/Slice/Grammar.cpp"
    break;

  case 135: /* enumerator: ICE_IDENTIFIER  */
#line 1704 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto ens = make_shared<EnumeratorListTok>();
    ContainerPtr cont = currentUnit->currentContainer();
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if (en)
    {
        ens->v.push_front(en);
    }
    yyval = ens;
}
#line 3524 "src/Slice/Grammar.cpp"
    break;

  case 136: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1716 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    auto ens = make_shared<EnumeratorListTok>();
    ContainerPtr cont = currentUnit->currentContainer();
    auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
    if (intVal)
    {
        if (intVal->v < 0 || intVal->v > std::numeric_limits<int32_t>::max())
        {
            currentUnit->error("value for enumerator `" + ident->v + "' is out of range");
        }
        else
        {
            EnumeratorPtr en = cont->createEnumerator(ident->v, static_cast<int>(intVal->v));
            ens->v.push_front(en);
        }
    }
    yyval = ens;
}
#line 3548 "src/Slice/Grammar.cpp"
    break;

  case 137: /* enumerator: keyword  */
#line 1736 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    auto ens = make_shared<EnumeratorListTok>(); // Dummy
    yyval = ens;
}
#line 3559 "src/Slice/Grammar.cpp"
    break;

  case 138: /* enumerator: %empty  */
#line 1743 "src/Slice/Grammar.y"
{
    auto ens = make_shared<EnumeratorListTok>();
    yyval = ens; // Dummy
}
#line 3568 "src/Slice/Grammar.cpp"
    break;

  case 139: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1753 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3576 "src/Slice/Grammar.cpp"
    break;

  case 140: /* enumerator_initializer: scoped_name  */
#line 1757 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainedList cl = currentUnit->currentContainer()->lookupContained(scoped->v);
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
#line 3616 "src/Slice/Grammar.cpp"
    break;

  case 141: /* out_qualifier: ICE_OUT  */
#line 1798 "src/Slice/Grammar.y"
{
    auto out = make_shared<BoolTok>();
    out->v = true;
    yyval = out;
}
#line 3626 "src/Slice/Grammar.cpp"
    break;

  case 142: /* out_qualifier: %empty  */
#line 1804 "src/Slice/Grammar.y"
{
    auto out = make_shared<BoolTok>();
    out->v = false;
    yyval = out;
}
#line 3636 "src/Slice/Grammar.cpp"
    break;

  case 143: /* parameters: %empty  */
#line 1815 "src/Slice/Grammar.y"
{
}
#line 3643 "src/Slice/Grammar.cpp"
    break;

  case 144: /* parameters: out_qualifier meta_data tagged_type_id  */
#line 1818 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto tsp = dynamic_pointer_cast<TaggedDefTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isTagged, tsp->tag);
        currentUnit->currentContainer()->checkIntroduced(tsp->name, pd);
        auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-1]);
        if (!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 3663 "src/Slice/Grammar.cpp"
    break;

  case 145: /* parameters: parameters ',' out_qualifier meta_data tagged_type_id  */
#line 1834 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto tsp = dynamic_pointer_cast<TaggedDefTok>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isTagged, tsp->tag);
        currentUnit->currentContainer()->checkIntroduced(tsp->name, pd);
        auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-1]);
        if (!metaData->v.empty())
        {
            pd->setMetaData(metaData->v);
        }
    }
}
#line 3683 "src/Slice/Grammar.cpp"
    break;

  case 146: /* parameters: out_qualifier meta_data type keyword  */
#line 1850 "src/Slice/Grammar.y"
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
#line 3699 "src/Slice/Grammar.cpp"
    break;

  case 147: /* parameters: parameters ',' out_qualifier meta_data type keyword  */
#line 1862 "src/Slice/Grammar.y"
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
#line 3715 "src/Slice/Grammar.cpp"
    break;

  case 148: /* parameters: out_qualifier meta_data type  */
#line 1874 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("missing parameter name");
    }
}
#line 3730 "src/Slice/Grammar.cpp"
    break;

  case 149: /* parameters: parameters ',' out_qualifier meta_data type  */
#line 1885 "src/Slice/Grammar.y"
{
    auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        op->createParamDecl(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        currentUnit->error("missing parameter name");
    }
}
#line 3745 "src/Slice/Grammar.cpp"
    break;

  case 150: /* throws: ICE_THROWS exception_list  */
#line 1901 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3753 "src/Slice/Grammar.cpp"
    break;

  case 151: /* throws: %empty  */
#line 1905 "src/Slice/Grammar.y"
{
    yyval = make_shared<ExceptionListTok>();
}
#line 3761 "src/Slice/Grammar.cpp"
    break;

  case 152: /* scoped_name: ICE_IDENTIFIER  */
#line 1914 "src/Slice/Grammar.y"
{
}
#line 3768 "src/Slice/Grammar.cpp"
    break;

  case 153: /* scoped_name: ICE_SCOPED_IDENTIFIER  */
#line 1917 "src/Slice/Grammar.y"
{
}
#line 3775 "src/Slice/Grammar.cpp"
    break;

  case 154: /* builtin: ICE_BOOL  */
#line 1924 "src/Slice/Grammar.y"
           {}
#line 3781 "src/Slice/Grammar.cpp"
    break;

  case 155: /* builtin: ICE_BYTE  */
#line 1925 "src/Slice/Grammar.y"
           {}
#line 3787 "src/Slice/Grammar.cpp"
    break;

  case 156: /* builtin: ICE_SHORT  */
#line 1926 "src/Slice/Grammar.y"
            {}
#line 3793 "src/Slice/Grammar.cpp"
    break;

  case 157: /* builtin: ICE_INT  */
#line 1927 "src/Slice/Grammar.y"
          {}
#line 3799 "src/Slice/Grammar.cpp"
    break;

  case 158: /* builtin: ICE_LONG  */
#line 1928 "src/Slice/Grammar.y"
           {}
#line 3805 "src/Slice/Grammar.cpp"
    break;

  case 159: /* builtin: ICE_FLOAT  */
#line 1929 "src/Slice/Grammar.y"
            {}
#line 3811 "src/Slice/Grammar.cpp"
    break;

  case 160: /* builtin: ICE_DOUBLE  */
#line 1930 "src/Slice/Grammar.y"
             {}
#line 3817 "src/Slice/Grammar.cpp"
    break;

  case 161: /* builtin: ICE_STRING  */
#line 1931 "src/Slice/Grammar.y"
             {}
#line 3823 "src/Slice/Grammar.cpp"
    break;

  case 162: /* builtin: ICE_OBJECT  */
#line 1932 "src/Slice/Grammar.y"
             {}
#line 3829 "src/Slice/Grammar.cpp"
    break;

  case 163: /* builtin: ICE_VALUE  */
#line 1933 "src/Slice/Grammar.y"
            {}
#line 3835 "src/Slice/Grammar.cpp"
    break;

  case 164: /* type: ICE_OBJECT '*'  */
#line 1939 "src/Slice/Grammar.y"
{
    yyval = currentUnit->builtin(Builtin::KindObjectProxy);
}
#line 3843 "src/Slice/Grammar.cpp"
    break;

  case 165: /* type: builtin  */
#line 1943 "src/Slice/Grammar.y"
{
    auto typeName = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    yyval = currentUnit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 3852 "src/Slice/Grammar.cpp"
    break;

  case 166: /* type: scoped_name  */
#line 1948 "src/Slice/Grammar.y"
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
#line 3887 "src/Slice/Grammar.cpp"
    break;

  case 167: /* type: scoped_name '*'  */
#line 1979 "src/Slice/Grammar.y"
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
#line 3922 "src/Slice/Grammar.cpp"
    break;

  case 168: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 2015 "src/Slice/Grammar.y"
{
    auto str1 = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    auto str2 = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    str1->v += str2->v;
}
#line 3932 "src/Slice/Grammar.cpp"
    break;

  case 169: /* string_literal: ICE_STRING_LITERAL  */
#line 2021 "src/Slice/Grammar.y"
{
}
#line 3939 "src/Slice/Grammar.cpp"
    break;

  case 170: /* string_list: string_list ',' string_literal  */
#line 2029 "src/Slice/Grammar.y"
{
    auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto stringList = dynamic_pointer_cast<StringListTok>(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 3950 "src/Slice/Grammar.cpp"
    break;

  case 171: /* string_list: string_literal  */
#line 2036 "src/Slice/Grammar.y"
{
    auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto stringList = make_shared<StringListTok>();
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 3961 "src/Slice/Grammar.cpp"
    break;

  case 172: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2048 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->builtin(Builtin::KindLong);
    auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    auto def = make_shared<ConstDefTok>(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 3974 "src/Slice/Grammar.cpp"
    break;

  case 173: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2057 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->builtin(Builtin::KindDouble);
    auto floatVal = dynamic_pointer_cast<FloatingTok>(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    auto def = make_shared<ConstDefTok>(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 3987 "src/Slice/Grammar.cpp"
    break;

  case 174: /* const_initializer: scoped_name  */
#line 2066 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ConstDefTokPtr def;
    ContainedList cl = currentUnit->currentContainer()->lookupContained(scoped->v, false);
    if (cl.empty())
    {
        // Could be an enumerator
        def = make_shared<ConstDefTok>(nullptr, scoped->v, scoped->v);
    }
    else
    {
        auto enumerator = dynamic_pointer_cast<Enumerator>(cl.front());
        auto constant = dynamic_pointer_cast<Const>(cl.front());
        if (enumerator)
        {
            currentUnit->currentContainer()->checkIntroduced(scoped->v, enumerator);
            def = make_shared<ConstDefTok>(enumerator, scoped->v, scoped->v);
        }
        else if (constant)
        {
            currentUnit->currentContainer()->checkIntroduced(scoped->v, constant);
            def = make_shared<ConstDefTok>(constant, constant->value(), constant->value());
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
#line 4026 "src/Slice/Grammar.cpp"
    break;

  case 175: /* const_initializer: ICE_STRING_LITERAL  */
#line 2101 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->builtin(Builtin::KindString);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, literal->v, literal->literal);
    yyval = def;
}
#line 4037 "src/Slice/Grammar.cpp"
    break;

  case 176: /* const_initializer: ICE_FALSE  */
#line 2108 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->builtin(Builtin::KindBool);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, "false", "false");
    yyval = def;
}
#line 4048 "src/Slice/Grammar.cpp"
    break;

  case 177: /* const_initializer: ICE_TRUE  */
#line 2115 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->builtin(Builtin::KindBool);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, "true", "true");
    yyval = def;
}
#line 4059 "src/Slice/Grammar.cpp"
    break;

  case 178: /* const_def: ICE_CONST meta_data type ICE_IDENTIFIER '=' const_initializer  */
#line 2127 "src/Slice/Grammar.y"
{
    auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-4]);
    auto const_type = dynamic_pointer_cast<Type>(yyvsp[-3]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    yyval = currentUnit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4072 "src/Slice/Grammar.cpp"
    break;

  case 179: /* const_def: ICE_CONST meta_data type '=' const_initializer  */
#line 2136 "src/Slice/Grammar.y"
{
    auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
    auto const_type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    currentUnit->error("missing constant name");
    yyval = currentUnit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4085 "src/Slice/Grammar.cpp"
    break;

  case 180: /* keyword: ICE_MODULE  */
#line 2149 "src/Slice/Grammar.y"
             {}
#line 4091 "src/Slice/Grammar.cpp"
    break;

  case 181: /* keyword: ICE_CLASS  */
#line 2150 "src/Slice/Grammar.y"
            {}
#line 4097 "src/Slice/Grammar.cpp"
    break;

  case 182: /* keyword: ICE_INTERFACE  */
#line 2151 "src/Slice/Grammar.y"
                {}
#line 4103 "src/Slice/Grammar.cpp"
    break;

  case 183: /* keyword: ICE_EXCEPTION  */
#line 2152 "src/Slice/Grammar.y"
                {}
#line 4109 "src/Slice/Grammar.cpp"
    break;

  case 184: /* keyword: ICE_STRUCT  */
#line 2153 "src/Slice/Grammar.y"
             {}
#line 4115 "src/Slice/Grammar.cpp"
    break;

  case 185: /* keyword: ICE_SEQUENCE  */
#line 2154 "src/Slice/Grammar.y"
               {}
#line 4121 "src/Slice/Grammar.cpp"
    break;

  case 186: /* keyword: ICE_DICTIONARY  */
#line 2155 "src/Slice/Grammar.y"
                 {}
#line 4127 "src/Slice/Grammar.cpp"
    break;

  case 187: /* keyword: ICE_ENUM  */
#line 2156 "src/Slice/Grammar.y"
           {}
#line 4133 "src/Slice/Grammar.cpp"
    break;

  case 188: /* keyword: ICE_OUT  */
#line 2157 "src/Slice/Grammar.y"
          {}
#line 4139 "src/Slice/Grammar.cpp"
    break;

  case 189: /* keyword: ICE_EXTENDS  */
#line 2158 "src/Slice/Grammar.y"
              {}
#line 4145 "src/Slice/Grammar.cpp"
    break;

  case 190: /* keyword: ICE_THROWS  */
#line 2159 "src/Slice/Grammar.y"
             {}
#line 4151 "src/Slice/Grammar.cpp"
    break;

  case 191: /* keyword: ICE_VOID  */
#line 2160 "src/Slice/Grammar.y"
           {}
#line 4157 "src/Slice/Grammar.cpp"
    break;

  case 192: /* keyword: ICE_BOOL  */
#line 2161 "src/Slice/Grammar.y"
           {}
#line 4163 "src/Slice/Grammar.cpp"
    break;

  case 193: /* keyword: ICE_BYTE  */
#line 2162 "src/Slice/Grammar.y"
           {}
#line 4169 "src/Slice/Grammar.cpp"
    break;

  case 194: /* keyword: ICE_SHORT  */
#line 2163 "src/Slice/Grammar.y"
            {}
#line 4175 "src/Slice/Grammar.cpp"
    break;

  case 195: /* keyword: ICE_INT  */
#line 2164 "src/Slice/Grammar.y"
          {}
#line 4181 "src/Slice/Grammar.cpp"
    break;

  case 196: /* keyword: ICE_LONG  */
#line 2165 "src/Slice/Grammar.y"
           {}
#line 4187 "src/Slice/Grammar.cpp"
    break;

  case 197: /* keyword: ICE_FLOAT  */
#line 2166 "src/Slice/Grammar.y"
            {}
#line 4193 "src/Slice/Grammar.cpp"
    break;

  case 198: /* keyword: ICE_DOUBLE  */
#line 2167 "src/Slice/Grammar.y"
             {}
#line 4199 "src/Slice/Grammar.cpp"
    break;

  case 199: /* keyword: ICE_STRING  */
#line 2168 "src/Slice/Grammar.y"
             {}
#line 4205 "src/Slice/Grammar.cpp"
    break;

  case 200: /* keyword: ICE_OBJECT  */
#line 2169 "src/Slice/Grammar.y"
             {}
#line 4211 "src/Slice/Grammar.cpp"
    break;

  case 201: /* keyword: ICE_CONST  */
#line 2170 "src/Slice/Grammar.y"
            {}
#line 4217 "src/Slice/Grammar.cpp"
    break;

  case 202: /* keyword: ICE_FALSE  */
#line 2171 "src/Slice/Grammar.y"
            {}
#line 4223 "src/Slice/Grammar.cpp"
    break;

  case 203: /* keyword: ICE_TRUE  */
#line 2172 "src/Slice/Grammar.y"
           {}
#line 4229 "src/Slice/Grammar.cpp"
    break;

  case 204: /* keyword: ICE_IDEMPOTENT  */
#line 2173 "src/Slice/Grammar.y"
                 {}
#line 4235 "src/Slice/Grammar.cpp"
    break;

  case 205: /* keyword: ICE_TAG  */
#line 2174 "src/Slice/Grammar.y"
          {}
#line 4241 "src/Slice/Grammar.cpp"
    break;

  case 206: /* keyword: ICE_OPTIONAL  */
#line 2175 "src/Slice/Grammar.y"
               {}
#line 4247 "src/Slice/Grammar.cpp"
    break;

  case 207: /* keyword: ICE_VALUE  */
#line 2176 "src/Slice/Grammar.y"
            {}
#line 4253 "src/Slice/Grammar.cpp"
    break;


#line 4257 "src/Slice/Grammar.cpp"

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

#line 2179 "src/Slice/Grammar.y"

