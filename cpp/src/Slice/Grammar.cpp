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
#include <Slice/GrammarUtil.h>

#line 27 "src/Slice/Grammar.y"


// Defines the rule bison uses to reduce token locations. Bison asks that the macro should
// be one-line, and treatable as a single statement when followed by a semi-colon.
// `N` is the number of tokens that are being combined, and (Cur) is their combined location.
#define YYLLOC_DEFAULT(Cur, Rhs, N)                               \
do                                                                \
    if(N == 1)                                                    \
    {                                                             \
        (Cur) = (YYRHSLOC((Rhs), 1));                             \
    }                                                             \
    else                                                          \
    {                                                             \
        if(N)                                                     \
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
// warning C4127: conditional expression is constant
#   pragma warning(disable:4127)
#endif

// Avoid old style cast warnings in generated grammar
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wunused-label"

// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98753
#  pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif

// Avoid clang warnings in generate grammar
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wconversion"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wunused-but-set-variable"
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


#line 173 "src/Slice/Grammar.cpp"

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
  YYSYMBOL_ICE_IMPLEMENTS = 13,            /* ICE_IMPLEMENTS  */
  YYSYMBOL_ICE_THROWS = 14,                /* ICE_THROWS  */
  YYSYMBOL_ICE_VOID = 15,                  /* ICE_VOID  */
  YYSYMBOL_ICE_BOOL = 16,                  /* ICE_BOOL  */
  YYSYMBOL_ICE_BYTE = 17,                  /* ICE_BYTE  */
  YYSYMBOL_ICE_SHORT = 18,                 /* ICE_SHORT  */
  YYSYMBOL_ICE_INT = 19,                   /* ICE_INT  */
  YYSYMBOL_ICE_LONG = 20,                  /* ICE_LONG  */
  YYSYMBOL_ICE_FLOAT = 21,                 /* ICE_FLOAT  */
  YYSYMBOL_ICE_DOUBLE = 22,                /* ICE_DOUBLE  */
  YYSYMBOL_ICE_STRING = 23,                /* ICE_STRING  */
  YYSYMBOL_ICE_OBJECT = 24,                /* ICE_OBJECT  */
  YYSYMBOL_ICE_CONST = 25,                 /* ICE_CONST  */
  YYSYMBOL_ICE_FALSE = 26,                 /* ICE_FALSE  */
  YYSYMBOL_ICE_TRUE = 27,                  /* ICE_TRUE  */
  YYSYMBOL_ICE_IDEMPOTENT = 28,            /* ICE_IDEMPOTENT  */
  YYSYMBOL_ICE_TAG = 29,                   /* ICE_TAG  */
  YYSYMBOL_ICE_OPTIONAL = 30,              /* ICE_OPTIONAL  */
  YYSYMBOL_ICE_VALUE = 31,                 /* ICE_VALUE  */
  YYSYMBOL_ICE_STRING_LITERAL = 32,        /* ICE_STRING_LITERAL  */
  YYSYMBOL_ICE_INTEGER_LITERAL = 33,       /* ICE_INTEGER_LITERAL  */
  YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 34, /* ICE_FLOATING_POINT_LITERAL  */
  YYSYMBOL_ICE_IDENTIFIER = 35,            /* ICE_IDENTIFIER  */
  YYSYMBOL_ICE_SCOPED_IDENTIFIER = 36,     /* ICE_SCOPED_IDENTIFIER  */
  YYSYMBOL_ICE_METADATA_OPEN = 37,         /* ICE_METADATA_OPEN  */
  YYSYMBOL_ICE_METADATA_CLOSE = 38,        /* ICE_METADATA_CLOSE  */
  YYSYMBOL_ICE_GLOBAL_METADATA_OPEN = 39,  /* ICE_GLOBAL_METADATA_OPEN  */
  YYSYMBOL_ICE_GLOBAL_METADATA_IGNORE = 40, /* ICE_GLOBAL_METADATA_IGNORE  */
  YYSYMBOL_ICE_GLOBAL_METADATA_CLOSE = 41, /* ICE_GLOBAL_METADATA_CLOSE  */
  YYSYMBOL_ICE_IDENT_OPEN = 42,            /* ICE_IDENT_OPEN  */
  YYSYMBOL_ICE_KEYWORD_OPEN = 43,          /* ICE_KEYWORD_OPEN  */
  YYSYMBOL_ICE_TAG_OPEN = 44,              /* ICE_TAG_OPEN  */
  YYSYMBOL_ICE_OPTIONAL_OPEN = 45,         /* ICE_OPTIONAL_OPEN  */
  YYSYMBOL_BAD_CHAR = 46,                  /* BAD_CHAR  */
  YYSYMBOL_47_ = 47,                       /* ';'  */
  YYSYMBOL_48_ = 48,                       /* '{'  */
  YYSYMBOL_49_ = 49,                       /* '}'  */
  YYSYMBOL_50_ = 50,                       /* ')'  */
  YYSYMBOL_51_ = 51,                       /* ':'  */
  YYSYMBOL_52_ = 52,                       /* '='  */
  YYSYMBOL_53_ = 53,                       /* ','  */
  YYSYMBOL_54_ = 54,                       /* '<'  */
  YYSYMBOL_55_ = 55,                       /* '>'  */
  YYSYMBOL_56_ = 56,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 57,                  /* $accept  */
  YYSYMBOL_start = 58,                     /* start  */
  YYSYMBOL_opt_semicolon = 59,             /* opt_semicolon  */
  YYSYMBOL_global_meta_data = 60,          /* global_meta_data  */
  YYSYMBOL_meta_data = 61,                 /* meta_data  */
  YYSYMBOL_definitions = 62,               /* definitions  */
  YYSYMBOL_definition = 63,                /* definition  */
  YYSYMBOL_64_1 = 64,                      /* $@1  */
  YYSYMBOL_65_2 = 65,                      /* $@2  */
  YYSYMBOL_66_3 = 66,                      /* $@3  */
  YYSYMBOL_67_4 = 67,                      /* $@4  */
  YYSYMBOL_68_5 = 68,                      /* $@5  */
  YYSYMBOL_69_6 = 69,                      /* $@6  */
  YYSYMBOL_70_7 = 70,                      /* $@7  */
  YYSYMBOL_71_8 = 71,                      /* $@8  */
  YYSYMBOL_72_9 = 72,                      /* $@9  */
  YYSYMBOL_73_10 = 73,                     /* $@10  */
  YYSYMBOL_74_11 = 74,                     /* $@11  */
  YYSYMBOL_75_12 = 75,                     /* $@12  */
  YYSYMBOL_76_13 = 76,                     /* $@13  */
  YYSYMBOL_module_def = 77,                /* module_def  */
  YYSYMBOL_78_14 = 78,                     /* @14  */
  YYSYMBOL_exception_id = 79,              /* exception_id  */
  YYSYMBOL_exception_decl = 80,            /* exception_decl  */
  YYSYMBOL_exception_def = 81,             /* exception_def  */
  YYSYMBOL_82_15 = 82,                     /* @15  */
  YYSYMBOL_exception_extends = 83,         /* exception_extends  */
  YYSYMBOL_exception_exports = 84,         /* exception_exports  */
  YYSYMBOL_type_id = 85,                   /* type_id  */
  YYSYMBOL_tag = 86,                       /* tag  */
  YYSYMBOL_optional = 87,                  /* optional  */
  YYSYMBOL_tagged_type_id = 88,            /* tagged_type_id  */
  YYSYMBOL_exception_export = 89,          /* exception_export  */
  YYSYMBOL_struct_id = 90,                 /* struct_id  */
  YYSYMBOL_struct_decl = 91,               /* struct_decl  */
  YYSYMBOL_struct_def = 92,                /* struct_def  */
  YYSYMBOL_93_16 = 93,                     /* @16  */
  YYSYMBOL_struct_exports = 94,            /* struct_exports  */
  YYSYMBOL_struct_export = 95,             /* struct_export  */
  YYSYMBOL_class_name = 96,                /* class_name  */
  YYSYMBOL_class_id = 97,                  /* class_id  */
  YYSYMBOL_class_decl = 98,                /* class_decl  */
  YYSYMBOL_class_def = 99,                 /* class_def  */
  YYSYMBOL_100_17 = 100,                   /* @17  */
  YYSYMBOL_class_extends = 101,            /* class_extends  */
  YYSYMBOL_extends = 102,                  /* extends  */
  YYSYMBOL_implements = 103,               /* implements  */
  YYSYMBOL_class_exports = 104,            /* class_exports  */
  YYSYMBOL_data_member = 105,              /* data_member  */
  YYSYMBOL_struct_data_member = 106,       /* struct_data_member  */
  YYSYMBOL_return_type = 107,              /* return_type  */
  YYSYMBOL_operation_preamble = 108,       /* operation_preamble  */
  YYSYMBOL_operation = 109,                /* operation  */
  YYSYMBOL_110_18 = 110,                   /* @18  */
  YYSYMBOL_111_19 = 111,                   /* @19  */
  YYSYMBOL_class_export = 112,             /* class_export  */
  YYSYMBOL_interface_id = 113,             /* interface_id  */
  YYSYMBOL_interface_decl = 114,           /* interface_decl  */
  YYSYMBOL_interface_def = 115,            /* interface_def  */
  YYSYMBOL_116_20 = 116,                   /* @20  */
  YYSYMBOL_interface_list = 117,           /* interface_list  */
  YYSYMBOL_interface_extends = 118,        /* interface_extends  */
  YYSYMBOL_interface_exports = 119,        /* interface_exports  */
  YYSYMBOL_interface_export = 120,         /* interface_export  */
  YYSYMBOL_exception_list = 121,           /* exception_list  */
  YYSYMBOL_exception = 122,                /* exception  */
  YYSYMBOL_sequence_def = 123,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 124,           /* dictionary_def  */
  YYSYMBOL_enum_id = 125,                  /* enum_id  */
  YYSYMBOL_enum_def = 126,                 /* enum_def  */
  YYSYMBOL_127_21 = 127,                   /* @21  */
  YYSYMBOL_128_22 = 128,                   /* @22  */
  YYSYMBOL_enumerator_list = 129,          /* enumerator_list  */
  YYSYMBOL_enumerator = 130,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 131,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 132,            /* out_qualifier  */
  YYSYMBOL_parameters = 133,               /* parameters  */
  YYSYMBOL_throws = 134,                   /* throws  */
  YYSYMBOL_scoped_name = 135,              /* scoped_name  */
  YYSYMBOL_builtin = 136,                  /* builtin  */
  YYSYMBOL_type = 137,                     /* type  */
  YYSYMBOL_string_literal = 138,           /* string_literal  */
  YYSYMBOL_string_list = 139,              /* string_list  */
  YYSYMBOL_const_initializer = 140,        /* const_initializer  */
  YYSYMBOL_const_def = 141,                /* const_def  */
  YYSYMBOL_keyword = 142                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 58 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 358 "src/Slice/Grammar.cpp"

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
#define YYLAST   928

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  57
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  86
/* YYNRULES -- Number of rules.  */
#define YYNRULES  231
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  341

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   301


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
       2,    50,    56,     2,    53,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    51,    47,
      54,    52,    55,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,     2,    49,     2,     2,     2,     2,
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
      45,    46
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   185,   185,   193,   196,   204,   208,   218,   222,   231,
     239,   248,   257,   256,   262,   261,   266,   271,   270,   276,
     275,   280,   285,   284,   290,   289,   294,   299,   298,   304,
     303,   308,   313,   312,   318,   317,   322,   327,   326,   331,
     336,   335,   341,   340,   345,   349,   359,   358,   391,   395,
     406,   417,   416,   442,   450,   459,   468,   471,   475,   483,
     496,   514,   593,   599,   610,   628,   706,   712,   723,   732,
     741,   754,   760,   764,   775,   786,   785,   826,   835,   838,
     842,   850,   856,   860,   871,   896,   986,   998,  1011,  1010,
    1049,  1083,  1092,  1095,  1103,  1107,  1116,  1125,  1128,  1132,
    1140,  1162,  1189,  1211,  1237,  1246,  1257,  1266,  1275,  1284,
    1293,  1303,  1317,  1323,  1329,  1335,  1345,  1369,  1394,  1418,
    1449,  1448,  1471,  1470,  1493,  1494,  1500,  1504,  1515,  1529,
    1528,  1562,  1597,  1632,  1637,  1647,  1651,  1660,  1669,  1672,
    1676,  1684,  1690,  1697,  1709,  1721,  1732,  1740,  1754,  1764,
    1780,  1784,  1796,  1795,  1827,  1826,  1844,  1850,  1858,  1870,
    1890,  1897,  1907,  1911,  1949,  1955,  1966,  1969,  1985,  2001,
    2013,  2025,  2036,  2052,  2056,  2065,  2068,  2076,  2077,  2078,
    2079,  2080,  2081,  2082,  2083,  2084,  2085,  2090,  2094,  2099,
    2118,  2155,  2161,  2169,  2176,  2188,  2197,  2206,  2246,  2253,
    2260,  2272,  2281,  2295,  2296,  2297,  2298,  2299,  2300,  2301,
    2302,  2303,  2304,  2305,  2306,  2307,  2308,  2309,  2310,  2311,
    2312,  2313,  2314,  2315,  2316,  2317,  2318,  2319,  2320,  2321,
    2322,  2323
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
  "ICE_IMPLEMENTS", "ICE_THROWS", "ICE_VOID", "ICE_BOOL", "ICE_BYTE",
  "ICE_SHORT", "ICE_INT", "ICE_LONG", "ICE_FLOAT", "ICE_DOUBLE",
  "ICE_STRING", "ICE_OBJECT", "ICE_CONST", "ICE_FALSE", "ICE_TRUE",
  "ICE_IDEMPOTENT", "ICE_TAG", "ICE_OPTIONAL", "ICE_VALUE",
  "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENTIFIER", "ICE_SCOPED_IDENTIFIER",
  "ICE_METADATA_OPEN", "ICE_METADATA_CLOSE", "ICE_GLOBAL_METADATA_OPEN",
  "ICE_GLOBAL_METADATA_IGNORE", "ICE_GLOBAL_METADATA_CLOSE",
  "ICE_IDENT_OPEN", "ICE_KEYWORD_OPEN", "ICE_TAG_OPEN",
  "ICE_OPTIONAL_OPEN", "BAD_CHAR", "';'", "'{'", "'}'", "')'", "':'",
  "'='", "','", "'<'", "'>'", "'*'", "$accept", "start", "opt_semicolon",
  "global_meta_data", "meta_data", "definitions", "definition", "$@1",
  "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
  "$@12", "$@13", "module_def", "@14", "exception_id", "exception_decl",
  "exception_def", "@15", "exception_extends", "exception_exports",
  "type_id", "tag", "optional", "tagged_type_id", "exception_export",
  "struct_id", "struct_decl", "struct_def", "@16", "struct_exports",
  "struct_export", "class_name", "class_id", "class_decl", "class_def",
  "@17", "class_extends", "extends", "implements", "class_exports",
  "data_member", "struct_data_member", "return_type", "operation_preamble",
  "operation", "@18", "@19", "class_export", "interface_id",
  "interface_decl", "interface_def", "@20", "interface_list",
  "interface_extends", "interface_exports", "interface_export",
  "exception_list", "exception", "sequence_def", "dictionary_def",
  "enum_id", "enum_def", "@21", "@22", "enumerator_list", "enumerator",
  "enumerator_initializer", "out_qualifier", "parameters", "throws",
  "scoped_name", "builtin", "type", "string_literal", "string_list",
  "const_initializer", "const_def", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-292)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-167)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -292,    20,    23,  -292,     7,     7,     7,  -292,   150,     7,
    -292,   -10,    49,    56,    -6,    11,   475,   549,   582,   615,
      -4,     0,   648,    28,  -292,  -292,    10,     1,  -292,    18,
      32,  -292,    24,    13,    39,  -292,    26,    47,  -292,    52,
      61,  -292,  -292,    88,  -292,  -292,     7,  -292,  -292,  -292,
    -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,
    -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,
    -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,
    -292,    48,  -292,  -292,  -292,  -292,  -292,  -292,  -292,    28,
      28,  -292,    55,  -292,   892,    90,  -292,  -292,  -292,    -5,
      95,    90,   113,   116,    90,   154,    -5,   126,    90,   145,
    -292,   127,    90,   135,   136,   137,    90,   142,  -292,   139,
     141,  -292,  -292,   144,   892,   892,   681,  -292,  -292,  -292,
    -292,  -292,  -292,  -292,  -292,   134,  -292,   143,  -292,    -8,
    -292,  -292,   149,  -292,  -292,  -292,   350,  -292,  -292,   145,
    -292,  -292,  -292,  -292,  -292,  -292,  -292,   147,   157,  -292,
    -292,  -292,  -292,   681,  -292,  -292,  -292,  -292,  -292,   152,
     155,   146,   160,   158,  -292,  -292,  -292,   161,   251,   387,
     165,   862,   168,  -292,   171,   145,   103,   172,    31,   714,
      28,   108,  -292,   681,   251,  -292,  -292,  -292,  -292,  -292,
    -292,  -292,   173,   862,   174,   350,  -292,  -292,    60,   129,
     170,   892,   892,   177,  -292,   747,  -292,   313,  -292,   180,
     801,   191,  -292,  -292,  -292,  -292,   892,  -292,  -292,  -292,
    -292,  -292,   387,  -292,   892,   892,   190,   182,  -292,   747,
    -292,  -292,   193,  -292,   198,   199,  -292,   203,   251,   202,
     220,   205,   350,  -292,  -292,   214,   801,   213,   103,  -292,
     832,   892,   892,    74,   215,  -292,   216,  -292,  -292,   209,
    -292,  -292,  -292,   251,   387,  -292,  -292,  -292,  -292,  -292,
    -292,   251,   251,  -292,   313,   892,   892,  -292,  -292,   219,
     434,  -292,  -292,   107,  -292,  -292,  -292,  -292,   217,  -292,
      28,    62,   103,   780,  -292,  -292,  -292,  -292,  -292,   220,
     220,   313,  -292,  -292,  -292,   862,  -292,   181,  -292,  -292,
    -292,  -292,   255,  -292,   747,   255,    28,   515,  -292,  -292,
    -292,   862,  -292,   218,  -292,  -292,  -292,   747,   515,  -292,
    -292
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      11,     0,     8,     1,     0,     0,     0,     9,     0,   192,
     194,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   154,     8,    10,    12,    50,    26,    27,    74,
      31,    32,    87,    91,    16,    17,   128,    21,    22,    36,
      39,   152,    40,    44,   191,     7,     0,     5,     6,    45,
      46,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
      82,     0,    83,   126,   127,    48,    49,    72,    73,     8,
       8,   150,     0,   151,     0,     4,    92,    93,    51,     0,
       0,     4,     0,     0,     4,    95,     0,     0,     4,     0,
     129,     0,     4,     0,     0,     0,     4,     0,   193,     0,
       0,   175,   176,     0,     0,     0,   161,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   189,   188,     0,
       3,    13,     0,    53,    25,    28,     0,    30,    33,     0,
      88,    90,    15,    18,   133,   134,   135,   132,     0,    20,
      23,    35,    38,   161,    41,    43,    11,    84,    85,     0,
       0,   158,     0,   157,   160,   187,   190,     0,     0,     0,
       0,     0,     0,    94,     0,     0,     0,     0,     8,     0,
       8,     0,   155,   161,     0,   199,   200,   198,   195,   196,
     197,   202,     0,     0,     0,     0,    63,    67,     0,     0,
     104,     0,     0,    79,    81,   111,    76,     0,   131,     0,
       0,     0,   153,    47,   146,   147,     0,   162,   159,   163,
     156,   201,     0,    70,     0,     0,   100,    57,    71,   103,
      52,    78,     0,    62,     0,     0,    66,     0,     0,   106,
       0,   108,     0,    59,   110,     0,     0,     0,     0,   115,
       0,     0,     0,     0,     0,   141,   139,   114,   130,     0,
      56,    68,    69,     0,     0,   102,    60,    61,    64,    65,
     105,     0,     0,    77,     0,     0,     0,   124,   125,    98,
     103,    89,   138,     0,   112,   113,   116,   118,     0,   164,
       8,     0,     0,     0,   101,    55,   107,   109,    97,   112,
     113,     0,   117,   119,   122,     0,   120,   165,   137,   148,
     149,    96,   174,   167,   171,   174,     8,     0,   123,   169,
     121,     0,   173,   143,   144,   145,   168,   172,     0,   170,
     142
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -292,  -292,   102,  -292,    -2,   104,  -292,  -292,  -292,  -292,
    -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,  -292,
    -292,  -292,  -292,  -292,  -292,  -292,  -292,  -217,  -179,  -178,
    -171,  -291,  -292,  -292,  -292,  -292,  -292,  -193,  -292,  -292,
    -292,  -292,  -292,  -292,  -292,    40,  -292,  -266,    17,  -292,
      14,  -292,    19,  -292,  -292,  -292,  -292,  -292,  -292,  -292,
    -133,  -292,  -249,  -292,   -62,  -292,  -292,  -292,  -292,  -292,
    -292,  -292,  -146,  -292,  -292,   -37,  -292,   -44,   -80,  -292,
     -90,     5,   140,  -181,  -292,   -11
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   141,     7,   181,     2,    24,    95,   107,   108,
     111,   112,   100,   101,   103,   104,   113,   114,   116,   117,
      25,   119,    26,    27,    28,   142,    98,   204,   233,   234,
     235,   236,   237,    29,    30,    31,   102,   182,   213,    32,
      33,    34,    35,   184,   105,    99,   150,   257,   238,   214,
     263,   264,   265,   325,   322,   289,    36,    37,    38,   158,
     156,   110,   221,   266,   332,   333,    39,    40,    41,    42,
     115,    92,   172,   173,   228,   300,   301,   328,   137,   138,
     250,    10,    11,   201,    43,   174
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       8,   123,   210,   211,   139,    82,    84,    86,    88,   292,
     212,    93,   241,   231,    44,   270,   183,   187,   308,   143,
       3,    94,    96,    -2,   323,    96,   151,   177,    45,   157,
     121,   122,   249,   251,   169,   170,   -86,   -86,    96,     9,
     336,    49,   261,    46,   178,   321,    50,   230,   -24,   262,
      89,   118,   218,   318,    90,   271,   272,   305,   -54,   283,
       4,    97,     5,     6,    97,     4,   -75,   280,     4,   157,
       5,     6,   -86,   106,  -136,   -86,   109,    97,   285,   -29,
     223,   120,   261,   121,   122,   286,   -14,   124,   125,   262,
      47,   215,   304,   242,   -19,   121,   122,    48,   200,   -34,
     306,   307,    46,   126,   219,   157,   271,   272,   -37,    46,
     243,   229,   316,   239,   200,   317,   296,   297,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,   244,   247,
     267,    -8,    -8,    -8,    -8,   -42,   269,   140,    -8,    -8,
       4,   227,   144,   121,   122,    12,    13,    -8,    -8,   312,
     313,    14,  -140,    15,    16,    17,    18,    19,    20,    21,
      22,   146,   245,   147,   121,   122,   290,   149,   200,   154,
     267,   294,   295,   152,   159,    23,   155,   203,   225,   246,
     121,   122,   161,   162,   220,   163,     8,   166,   226,   165,
     175,   167,   299,   200,   168,   309,   310,   179,   191,   176,
     185,   200,   200,   145,   254,   186,   148,   189,   190,   192,
     153,   193,   205,   194,   160,   256,   298,   216,   164,   217,
     232,   222,   248,   240,   252,   324,   299,   258,   275,   274,
     203,  -165,  -165,  -165,  -165,  -165,  -165,  -165,  -165,  -165,
     268,   337,   273,   276,  -165,  -165,  -165,   334,   277,   278,
    -165,  -165,  -165,   279,   281,   253,   220,   282,   334,  -165,
    -165,   284,   291,   302,   303,  -166,   311,   314,  -166,   327,
     188,   338,   203,   287,   293,   288,   340,   195,   196,   275,
     326,   330,   256,   197,   198,   199,   121,   122,     0,     0,
       0,     0,   320,     0,     0,     0,     0,     0,   315,     0,
     220,     0,     0,     0,     0,     0,     0,     0,     0,   256,
       0,     0,     0,   329,   255,     0,   335,     0,     0,     0,
       0,     0,     0,     0,   331,     0,   339,   335,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,     0,     0,
       0,    -8,    -8,    -8,    -8,     0,     0,     0,    -8,    -8,
       4,   180,     0,     0,     0,     0,     0,    -8,    -8,     0,
       0,     0,   -99,     0,     0,     0,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,     0,     0,     0,     0,    -8,
      -8,    -8,     0,     0,     0,    -8,    -8,     4,   202,     0,
       0,     0,     0,     0,    -8,    -8,     0,     0,     0,   -80,
       0,     0,     0,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,     0,     0,     0,     0,    -8,    -8,    -8,     0,
       0,     0,    -8,    -8,     4,     0,     0,     0,     0,     0,
       0,    -8,    -8,     0,     0,     0,   -58,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,     0,     0,     0,   253,
       0,     0,     0,     0,     0,     0,  -114,  -114,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
      80,     0,     0,     0,     0,     0,     0,    81,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
     121,   122,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,     0,     0,     0,    83,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,     0,     0,     0,    85,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
      87,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
       0,     0,     0,    91,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,   171,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,     0,     0,     0,   224,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,     0,
       0,     0,   253,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,     0,     0,     0,   319,   259,   127,   128,   129,
     130,   131,   132,   133,   134,   135,     0,     0,     0,   260,
     206,   207,   136,     0,     0,     0,   121,   122,     0,     0,
       0,     0,     0,     0,     0,   208,   209,   259,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     0,     0,     0,
       0,   206,   207,   136,     0,     0,     0,   121,   122,     0,
       0,     0,     0,     0,     0,     0,   208,   209,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     0,     0,     0,
       0,   206,   207,   136,     0,     0,     0,   121,   122,     0,
       0,     0,     0,     0,     0,     0,   208,   209,   127,   128,
     129,   130,   131,   132,   133,   134,   135,     0,     0,     0,
       0,     0,     0,   136,     0,     0,     0,   121,   122
};

static const yytype_int16 yycheck[] =
{
       2,    81,   181,   181,    94,    16,    17,    18,    19,   258,
     181,    22,   205,   194,     9,   232,   149,   163,   284,    99,
       0,    23,    12,     0,   315,    12,   106,    35,    38,   109,
      35,    36,   211,   212,   124,   125,    12,    13,    12,    32,
     331,    47,   220,    53,    52,   311,    35,   193,    47,   220,
      54,    46,   185,   302,    54,   234,   235,   274,    48,   252,
      37,    51,    39,    40,    51,    37,    48,   248,    37,   149,
      39,    40,    48,    33,    48,    51,    36,    51,   256,    47,
      49,    33,   260,    35,    36,   256,    47,    89,    90,   260,
      41,   181,   273,    33,    47,    35,    36,    41,   178,    47,
     281,   282,    53,    48,     1,   185,   285,   286,    47,    53,
      50,   191,    50,   203,   194,    53,    42,    43,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,   208,   209,
     220,    28,    29,    30,    31,    47,   226,    47,    35,    36,
      37,    33,    47,    35,    36,     5,     6,    44,    45,    42,
      43,     1,    49,     3,     4,     5,     6,     7,     8,     9,
      10,    48,    33,    47,    35,    36,   256,    13,   248,    24,
     260,   261,   262,    47,    47,    25,    31,   179,   189,    50,
      35,    36,    47,    47,   186,    48,   188,    48,   190,    47,
      56,    50,    11,   273,    50,   285,   286,    48,    52,    56,
      53,   281,   282,   101,   215,    48,   104,    55,    53,    49,
     108,    53,    47,    52,   112,   217,     1,    49,   116,    48,
      47,    49,    52,    49,    47,   315,    11,    47,   239,    47,
     232,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      49,   331,    52,    50,    29,    30,    31,   327,    50,    50,
      35,    36,    37,    50,    52,    35,   258,    52,   338,    44,
      45,    47,    49,    47,    55,    50,    47,    50,    53,    14,
     166,    53,   274,   256,   260,   256,   338,    26,    27,   290,
     317,   325,   284,    32,    33,    34,    35,    36,    -1,    -1,
      -1,    -1,   303,    -1,    -1,    -1,    -1,    -1,   300,    -1,
     302,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   311,
      -1,    -1,    -1,   324,     1,    -1,   327,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   326,    -1,   337,   338,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    -1,
      -1,    28,    29,    30,    31,    -1,    -1,    -1,    35,    36,
      37,     1,    -1,    -1,    -1,    -1,    -1,    44,    45,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,    29,
      30,    31,    -1,    -1,    -1,    35,    36,    37,     1,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    -1,    -1,    -1,    29,    30,    31,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    49,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      35,    36,     3,     4,     5,     6,     7,     8,     9,    10,
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
      29,    30,    31,    -1,    -1,    -1,    35,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    35,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    35,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    35,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    28,
      29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    45,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    29,    30,    31,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    44,    45,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    36
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    58,    62,     0,    37,    39,    40,    60,    61,    32,
     138,   139,   139,   139,     1,     3,     4,     5,     6,     7,
       8,     9,    10,    25,    63,    77,    79,    80,    81,    90,
      91,    92,    96,    97,    98,    99,   113,   114,   115,   123,
     124,   125,   126,   141,   138,    38,    53,    41,    41,    47,
      35,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      35,    42,   142,    35,   142,    35,   142,    35,   142,    54,
      54,    35,   128,   142,    61,    64,    12,    51,    83,   102,
      69,    70,    93,    71,    72,   101,   102,    65,    66,   102,
     118,    67,    68,    73,    74,   127,    75,    76,   138,    78,
      33,    35,    36,   135,    61,    61,    48,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    31,   135,   136,   137,
      47,    59,    82,   135,    47,    59,    48,    47,    59,    13,
     103,   135,    47,    59,    24,    31,   117,   135,   116,    47,
      59,    47,    47,    48,    59,    47,    48,    50,    50,   137,
     137,    35,   129,   130,   142,    56,    56,    35,    52,    48,
       1,    61,    94,   117,   100,    53,    48,   129,    62,    55,
      53,    52,    49,    53,    52,    26,    27,    32,    33,    34,
     135,   140,     1,    61,    84,    47,    29,    30,    44,    45,
      85,    86,    87,    95,   106,   137,    49,    48,   117,     1,
      61,   119,    49,    49,    35,   142,    61,    33,   131,   135,
     129,   140,    47,    85,    86,    87,    88,    89,   105,   137,
      49,    94,    33,    50,   135,    33,    50,   135,    52,    85,
     137,    85,    47,    35,   142,     1,    61,   104,    47,    15,
      28,    86,    87,   107,   108,   109,   120,   137,    49,   137,
      84,    85,    85,    52,    47,   142,    50,    50,    50,    50,
     140,    52,    52,    94,    47,    86,    87,   105,   109,   112,
     137,    49,   119,   107,   137,   137,    42,    43,     1,    11,
     132,   133,    47,    55,   140,    84,   140,   140,   104,   137,
     137,    47,    42,    43,    50,    61,    50,    53,   119,    35,
     142,   104,   111,    88,   137,   110,   132,    14,   134,   142,
     134,    61,   121,   122,   135,   142,    88,   137,    53,   142,
     121
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    57,    58,    59,    59,    60,    60,    61,    61,    62,
      62,    62,    64,    63,    65,    63,    63,    66,    63,    67,
      63,    63,    68,    63,    69,    63,    63,    70,    63,    71,
      63,    63,    72,    63,    73,    63,    63,    74,    63,    63,
      75,    63,    76,    63,    63,    63,    78,    77,    79,    79,
      80,    82,    81,    83,    83,    84,    84,    84,    84,    85,
      86,    86,    86,    86,    87,    87,    87,    87,    88,    88,
      88,    89,    90,    90,    91,    93,    92,    94,    94,    94,
      94,    95,    96,    96,    97,    97,    97,    98,   100,    99,
     101,   101,   102,   102,   103,   103,   104,   104,   104,   104,
     105,   105,   105,   105,   106,   106,   106,   106,   106,   106,
     106,   106,   107,   107,   107,   107,   108,   108,   108,   108,
     110,   109,   111,   109,   112,   112,   113,   113,   114,   116,
     115,   117,   117,   117,   117,   118,   118,   119,   119,   119,
     119,   120,   121,   121,   122,   122,   123,   123,   124,   124,
     125,   125,   127,   126,   128,   126,   129,   129,   130,   130,
     130,   130,   131,   131,   132,   132,   133,   133,   133,   133,
     133,   133,   133,   134,   134,   135,   135,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   137,   137,   137,
     137,   138,   138,   139,   139,   140,   140,   140,   140,   140,
     140,   141,   141,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   142,
     142,   142
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     3,     0,     2,
       3,     0,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     1,
       0,     3,     0,     3,     1,     2,     0,     6,     2,     2,
       1,     0,     6,     2,     0,     4,     3,     2,     0,     2,
       3,     3,     2,     1,     3,     3,     2,     1,     2,     2,
       1,     1,     2,     2,     1,     0,     5,     4,     3,     2,
       0,     1,     2,     2,     4,     4,     1,     1,     0,     7,
       2,     0,     1,     1,     2,     0,     4,     3,     2,     0,
       1,     3,     2,     1,     1,     3,     2,     4,     2,     4,
       2,     1,     2,     2,     1,     1,     2,     3,     2,     3,
       0,     5,     0,     5,     1,     1,     2,     2,     1,     0,
       6,     3,     1,     1,     1,     2,     0,     4,     3,     2,
       0,     1,     3,     1,     1,     1,     6,     6,     9,     9,
       2,     2,     0,     5,     0,     5,     3,     1,     1,     3,
       1,     0,     1,     1,     1,     0,     0,     3,     5,     4,
       6,     3,     5,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       2,     2,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     6,     5,     1,     1,     1,     1,     1,     1,     1,
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
#line 186 "src/Slice/Grammar.y"
{
}
#line 1828 "src/Slice/Grammar.cpp"
    break;

  case 3: /* opt_semicolon: ';'  */
#line 194 "src/Slice/Grammar.y"
{
}
#line 1835 "src/Slice/Grammar.cpp"
    break;

  case 4: /* opt_semicolon: %empty  */
#line 197 "src/Slice/Grammar.y"
{
}
#line 1842 "src/Slice/Grammar.cpp"
    break;

  case 5: /* global_meta_data: ICE_GLOBAL_METADATA_OPEN string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 205 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1850 "src/Slice/Grammar.cpp"
    break;

  case 6: /* global_meta_data: ICE_GLOBAL_METADATA_IGNORE string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 209 "src/Slice/Grammar.y"
{
    unit->error("global metadata must appear before any definitions");
    yyval = yyvsp[-1]; // Dummy
}
#line 1859 "src/Slice/Grammar.cpp"
    break;

  case 7: /* meta_data: ICE_METADATA_OPEN string_list ICE_METADATA_CLOSE  */
#line 219 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1867 "src/Slice/Grammar.cpp"
    break;

  case 8: /* meta_data: %empty  */
#line 223 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 1875 "src/Slice/Grammar.cpp"
    break;

  case 9: /* definitions: definitions global_meta_data  */
#line 232 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
#line 1887 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: definitions meta_data definition  */
#line 240 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1900 "src/Slice/Grammar.cpp"
    break;

  case 11: /* definitions: %empty  */
#line 249 "src/Slice/Grammar.y"
{
}
#line 1907 "src/Slice/Grammar.cpp"
    break;

  case 12: /* $@1: %empty  */
#line 257 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1915 "src/Slice/Grammar.cpp"
    break;

  case 14: /* $@2: %empty  */
#line 262 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1923 "src/Slice/Grammar.cpp"
    break;

  case 16: /* definition: class_decl  */
#line 267 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 1931 "src/Slice/Grammar.cpp"
    break;

  case 17: /* $@3: %empty  */
#line 271 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1939 "src/Slice/Grammar.cpp"
    break;

  case 19: /* $@4: %empty  */
#line 276 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1947 "src/Slice/Grammar.cpp"
    break;

  case 21: /* definition: interface_decl  */
#line 281 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 1955 "src/Slice/Grammar.cpp"
    break;

  case 22: /* $@5: %empty  */
#line 285 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1963 "src/Slice/Grammar.cpp"
    break;

  case 24: /* $@6: %empty  */
#line 290 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1971 "src/Slice/Grammar.cpp"
    break;

  case 26: /* definition: exception_decl  */
#line 295 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 1979 "src/Slice/Grammar.cpp"
    break;

  case 27: /* $@7: %empty  */
#line 299 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 1987 "src/Slice/Grammar.cpp"
    break;

  case 29: /* $@8: %empty  */
#line 304 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1995 "src/Slice/Grammar.cpp"
    break;

  case 31: /* definition: struct_decl  */
#line 309 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 2003 "src/Slice/Grammar.cpp"
    break;

  case 32: /* $@9: %empty  */
#line 313 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 2011 "src/Slice/Grammar.cpp"
    break;

  case 34: /* $@10: %empty  */
#line 318 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2019 "src/Slice/Grammar.cpp"
    break;

  case 36: /* definition: sequence_def  */
#line 323 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2027 "src/Slice/Grammar.cpp"
    break;

  case 37: /* $@11: %empty  */
#line 327 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2035 "src/Slice/Grammar.cpp"
    break;

  case 39: /* definition: dictionary_def  */
#line 332 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2043 "src/Slice/Grammar.cpp"
    break;

  case 40: /* $@12: %empty  */
#line 336 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2051 "src/Slice/Grammar.cpp"
    break;

  case 42: /* $@13: %empty  */
#line 341 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2059 "src/Slice/Grammar.cpp"
    break;

  case 44: /* definition: const_def  */
#line 346 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2067 "src/Slice/Grammar.cpp"
    break;

  case 45: /* definition: error ';'  */
#line 350 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2075 "src/Slice/Grammar.cpp"
    break;

  case 46: /* @14: %empty  */
#line 359 "src/Slice/Grammar.y"
{
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
#line 2095 "src/Slice/Grammar.cpp"
    break;

  case 47: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
#line 375 "src/Slice/Grammar.y"
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
#line 2111 "src/Slice/Grammar.cpp"
    break;

  case 48: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 392 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2119 "src/Slice/Grammar.cpp"
    break;

  case 49: /* exception_id: ICE_EXCEPTION keyword  */
#line 396 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2129 "src/Slice/Grammar.cpp"
    break;

  case 50: /* exception_decl: exception_id  */
#line 407 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2138 "src/Slice/Grammar.cpp"
    break;

  case 51: /* @15: %empty  */
#line 417 "src/Slice/Grammar.y"
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
#line 2155 "src/Slice/Grammar.cpp"
    break;

  case 52: /* exception_def: exception_id exception_extends @15 '{' exception_exports '}'  */
#line 430 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2167 "src/Slice/Grammar.cpp"
    break;

  case 53: /* exception_extends: extends scoped_name  */
#line 443 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2179 "src/Slice/Grammar.cpp"
    break;

  case 54: /* exception_extends: %empty  */
#line 451 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2187 "src/Slice/Grammar.cpp"
    break;

  case 55: /* exception_exports: meta_data exception_export ';' exception_exports  */
#line 460 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2200 "src/Slice/Grammar.cpp"
    break;

  case 56: /* exception_exports: error ';' exception_exports  */
#line 469 "src/Slice/Grammar.y"
{
}
#line 2207 "src/Slice/Grammar.cpp"
    break;

  case 57: /* exception_exports: meta_data exception_export  */
#line 472 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2215 "src/Slice/Grammar.cpp"
    break;

  case 58: /* exception_exports: %empty  */
#line 476 "src/Slice/Grammar.y"
{
}
#line 2222 "src/Slice/Grammar.cpp"
    break;

  case 59: /* type_id: type ICE_IDENTIFIER  */
#line 484 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    yyval = typestring;
}
#line 2234 "src/Slice/Grammar.cpp"
    break;

  case 60: /* tag: ICE_TAG_OPEN ICE_INTEGER_LITERAL ')'  */
#line 497 "src/Slice/Grammar.y"
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
#line 2256 "src/Slice/Grammar.cpp"
    break;

  case 61: /* tag: ICE_TAG_OPEN scoped_name ')'  */
#line 515 "src/Slice/Grammar.y"
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
        if(b && b->isIntegralType())
        {
            IceUtil::Int64 l = IceUtilInternal::strToInt64(constant->value().c_str(), 0, 0);
            if(l < 0 || l > Int32Max)
            {
                unit->error("tag is out of range");
            }
            tag = static_cast<int>(l);
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
#line 2339 "src/Slice/Grammar.cpp"
    break;

  case 62: /* tag: ICE_TAG_OPEN ')'  */
#line 594 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2349 "src/Slice/Grammar.cpp"
    break;

  case 63: /* tag: ICE_TAG  */
#line 600 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2359 "src/Slice/Grammar.cpp"
    break;

  case 64: /* optional: ICE_OPTIONAL_OPEN ICE_INTEGER_LITERAL ')'  */
#line 611 "src/Slice/Grammar.y"
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
#line 2381 "src/Slice/Grammar.cpp"
    break;

  case 65: /* optional: ICE_OPTIONAL_OPEN scoped_name ')'  */
#line 629 "src/Slice/Grammar.y"
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
        if(b && b->isIntegralType())
        {
            IceUtil::Int64 l = IceUtilInternal::strToInt64(constant->value().c_str(), 0, 0);
            if(l < 0 || l > Int32Max)
            {
                unit->error("tag is out of range");
            }
            tag = static_cast<int>(l);
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
#line 2463 "src/Slice/Grammar.cpp"
    break;

  case 66: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 707 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2473 "src/Slice/Grammar.cpp"
    break;

  case 67: /* optional: ICE_OPTIONAL  */
#line 713 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2483 "src/Slice/Grammar.cpp"
    break;

  case 68: /* tagged_type_id: tag type_id  */
#line 724 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2496 "src/Slice/Grammar.cpp"
    break;

  case 69: /* tagged_type_id: optional type_id  */
#line 733 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2509 "src/Slice/Grammar.cpp"
    break;

  case 70: /* tagged_type_id: type_id  */
#line 742 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2521 "src/Slice/Grammar.cpp"
    break;

  case 72: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 761 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2529 "src/Slice/Grammar.cpp"
    break;

  case 73: /* struct_id: ICE_STRUCT keyword  */
#line 765 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2539 "src/Slice/Grammar.cpp"
    break;

  case 74: /* struct_decl: struct_id  */
#line 776 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2548 "src/Slice/Grammar.cpp"
    break;

  case 75: /* @16: %empty  */
#line 786 "src/Slice/Grammar.y"
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
#line 2570 "src/Slice/Grammar.cpp"
    break;

  case 76: /* struct_def: struct_id @16 '{' struct_exports '}'  */
#line 804 "src/Slice/Grammar.y"
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
#line 2592 "src/Slice/Grammar.cpp"
    break;

  case 77: /* struct_exports: meta_data struct_export ';' struct_exports  */
#line 827 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2605 "src/Slice/Grammar.cpp"
    break;

  case 78: /* struct_exports: error ';' struct_exports  */
#line 836 "src/Slice/Grammar.y"
{
}
#line 2612 "src/Slice/Grammar.cpp"
    break;

  case 79: /* struct_exports: meta_data struct_export  */
#line 839 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2620 "src/Slice/Grammar.cpp"
    break;

  case 80: /* struct_exports: %empty  */
#line 843 "src/Slice/Grammar.y"
{
}
#line 2627 "src/Slice/Grammar.cpp"
    break;

  case 82: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 857 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2635 "src/Slice/Grammar.cpp"
    break;

  case 83: /* class_name: ICE_CLASS keyword  */
#line 861 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2645 "src/Slice/Grammar.cpp"
    break;

  case 84: /* class_id: ICE_CLASS ICE_IDENT_OPEN ICE_INTEGER_LITERAL ')'  */
#line 872 "src/Slice/Grammar.y"
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
#line 2674 "src/Slice/Grammar.cpp"
    break;

  case 85: /* class_id: ICE_CLASS ICE_IDENT_OPEN scoped_name ')'  */
#line 897 "src/Slice/Grammar.y"
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
        if(b && b->isIntegralType())
        {
            IceUtil::Int64 l = IceUtilInternal::strToInt64(constant->value().c_str(), 0, 0);
            if(l < 0 || l > Int32Max)
            {
                unit->error("compact id for class is out of range");
            }
            id = static_cast<int>(l);
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
#line 2768 "src/Slice/Grammar.cpp"
    break;

  case 86: /* class_id: class_name  */
#line 987 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2779 "src/Slice/Grammar.cpp"
    break;

  case 87: /* class_decl: class_name  */
#line 999 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false);
    yyval = cl;
}
#line 2790 "src/Slice/Grammar.cpp"
    break;

  case 88: /* @17: %empty  */
#line 1011 "src/Slice/Grammar.y"
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
#line 2816 "src/Slice/Grammar.cpp"
    break;

  case 89: /* class_def: class_id class_extends implements @17 '{' class_exports '}'  */
#line 1033 "src/Slice/Grammar.y"
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
#line 2832 "src/Slice/Grammar.cpp"
    break;

  case 90: /* class_extends: extends scoped_name  */
#line 1050 "src/Slice/Grammar.y"
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
#line 2870 "src/Slice/Grammar.cpp"
    break;

  case 91: /* class_extends: %empty  */
#line 1084 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2878 "src/Slice/Grammar.cpp"
    break;

  case 92: /* extends: ICE_EXTENDS  */
#line 1093 "src/Slice/Grammar.y"
{
}
#line 2885 "src/Slice/Grammar.cpp"
    break;

  case 93: /* extends: ':'  */
#line 1096 "src/Slice/Grammar.y"
{
}
#line 2892 "src/Slice/Grammar.cpp"
    break;

  case 94: /* implements: ICE_IMPLEMENTS interface_list  */
#line 1104 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2900 "src/Slice/Grammar.cpp"
    break;

  case 95: /* implements: %empty  */
#line 1108 "src/Slice/Grammar.y"
{
    yyval = new ClassListTok;
}
#line 2908 "src/Slice/Grammar.cpp"
    break;

  case 96: /* class_exports: meta_data class_export ';' class_exports  */
#line 1117 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2921 "src/Slice/Grammar.cpp"
    break;

  case 97: /* class_exports: error ';' class_exports  */
#line 1126 "src/Slice/Grammar.y"
{
}
#line 2928 "src/Slice/Grammar.cpp"
    break;

  case 98: /* class_exports: meta_data class_export  */
#line 1129 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2936 "src/Slice/Grammar.cpp"
    break;

  case 99: /* class_exports: %empty  */
#line 1133 "src/Slice/Grammar.y"
{
}
#line 2943 "src/Slice/Grammar.cpp"
    break;

  case 100: /* data_member: tagged_type_id  */
#line 1141 "src/Slice/Grammar.y"
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
#line 2969 "src/Slice/Grammar.cpp"
    break;

  case 101: /* data_member: tagged_type_id '=' const_initializer  */
#line 1163 "src/Slice/Grammar.y"
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
#line 3000 "src/Slice/Grammar.cpp"
    break;

  case 102: /* data_member: type keyword  */
#line 1190 "src/Slice/Grammar.y"
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
#line 3026 "src/Slice/Grammar.cpp"
    break;

  case 103: /* data_member: type  */
#line 1212 "src/Slice/Grammar.y"
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
#line 3051 "src/Slice/Grammar.cpp"
    break;

  case 104: /* struct_data_member: type_id  */
#line 1238 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 3064 "src/Slice/Grammar.cpp"
    break;

  case 105: /* struct_data_member: type_id '=' const_initializer  */
#line 1247 "src/Slice/Grammar.y"
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
#line 3079 "src/Slice/Grammar.cpp"
    break;

  case 106: /* struct_data_member: tag type_id  */
#line 1258 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3092 "src/Slice/Grammar.cpp"
    break;

  case 107: /* struct_data_member: tag type_id '=' const_initializer  */
#line 1267 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3105 "src/Slice/Grammar.cpp"
    break;

  case 108: /* struct_data_member: optional type_id  */
#line 1276 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3118 "src/Slice/Grammar.cpp"
    break;

  case 109: /* struct_data_member: optional type_id '=' const_initializer  */
#line 1285 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3131 "src/Slice/Grammar.cpp"
    break;

  case 110: /* struct_data_member: type keyword  */
#line 1294 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3145 "src/Slice/Grammar.cpp"
    break;

  case 111: /* struct_data_member: type  */
#line 1304 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("missing data member name");
}
#line 3158 "src/Slice/Grammar.cpp"
    break;

  case 112: /* return_type: tag type  */
#line 1318 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3168 "src/Slice/Grammar.cpp"
    break;

  case 113: /* return_type: optional type  */
#line 1324 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3178 "src/Slice/Grammar.cpp"
    break;

  case 114: /* return_type: type  */
#line 1330 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3188 "src/Slice/Grammar.cpp"
    break;

  case 115: /* return_type: ICE_VOID  */
#line 1336 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    yyval = m;
}
#line 3197 "src/Slice/Grammar.cpp"
    break;

  case 116: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1346 "src/Slice/Grammar.y"
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
#line 3225 "src/Slice/Grammar.cpp"
    break;

  case 117: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1370 "src/Slice/Grammar.y"
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
#line 3254 "src/Slice/Grammar.cpp"
    break;

  case 118: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1395 "src/Slice/Grammar.y"
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
#line 3282 "src/Slice/Grammar.cpp"
    break;

  case 119: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1419 "src/Slice/Grammar.y"
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
#line 3311 "src/Slice/Grammar.cpp"
    break;

  case 120: /* @18: %empty  */
#line 1449 "src/Slice/Grammar.y"
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
#line 3327 "src/Slice/Grammar.cpp"
    break;

  case 121: /* operation: operation_preamble parameters ')' @18 throws  */
#line 1461 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3341 "src/Slice/Grammar.cpp"
    break;

  case 122: /* @19: %empty  */
#line 1471 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3353 "src/Slice/Grammar.cpp"
    break;

  case 123: /* operation: operation_preamble error ')' @19 throws  */
#line 1479 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3367 "src/Slice/Grammar.cpp"
    break;

  case 126: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1501 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3375 "src/Slice/Grammar.cpp"
    break;

  case 127: /* interface_id: ICE_INTERFACE keyword  */
#line 1505 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3385 "src/Slice/Grammar.cpp"
    break;

  case 128: /* interface_decl: interface_id  */
#line 1516 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3397 "src/Slice/Grammar.cpp"
    break;

  case 129: /* @20: %empty  */
#line 1529 "src/Slice/Grammar.y"
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
#line 3418 "src/Slice/Grammar.cpp"
    break;

  case 130: /* interface_def: interface_id interface_extends @20 '{' interface_exports '}'  */
#line 1546 "src/Slice/Grammar.y"
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
#line 3434 "src/Slice/Grammar.cpp"
    break;

  case 131: /* interface_list: scoped_name ',' interface_list  */
#line 1563 "src/Slice/Grammar.y"
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
#line 3473 "src/Slice/Grammar.cpp"
    break;

  case 132: /* interface_list: scoped_name  */
#line 1598 "src/Slice/Grammar.y"
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
#line 3512 "src/Slice/Grammar.cpp"
    break;

  case 133: /* interface_list: ICE_OBJECT  */
#line 1633 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new ClassListTok; // Dummy
}
#line 3521 "src/Slice/Grammar.cpp"
    break;

  case 134: /* interface_list: ICE_VALUE  */
#line 1638 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3530 "src/Slice/Grammar.cpp"
    break;

  case 135: /* interface_extends: extends interface_list  */
#line 1648 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3538 "src/Slice/Grammar.cpp"
    break;

  case 136: /* interface_extends: %empty  */
#line 1652 "src/Slice/Grammar.y"
{
    yyval = new ClassListTok;
}
#line 3546 "src/Slice/Grammar.cpp"
    break;

  case 137: /* interface_exports: meta_data interface_export ';' interface_exports  */
#line 1661 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3559 "src/Slice/Grammar.cpp"
    break;

  case 138: /* interface_exports: error ';' interface_exports  */
#line 1670 "src/Slice/Grammar.y"
{
}
#line 3566 "src/Slice/Grammar.cpp"
    break;

  case 139: /* interface_exports: meta_data interface_export  */
#line 1673 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3574 "src/Slice/Grammar.cpp"
    break;

  case 140: /* interface_exports: %empty  */
#line 1677 "src/Slice/Grammar.y"
{
}
#line 3581 "src/Slice/Grammar.cpp"
    break;

  case 142: /* exception_list: exception ',' exception_list  */
#line 1691 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3592 "src/Slice/Grammar.cpp"
    break;

  case 143: /* exception_list: exception  */
#line 1698 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3603 "src/Slice/Grammar.cpp"
    break;

  case 144: /* exception: scoped_name  */
#line 1710 "src/Slice/Grammar.y"
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
#line 3619 "src/Slice/Grammar.cpp"
    break;

  case 145: /* exception: keyword  */
#line 1722 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3629 "src/Slice/Grammar.cpp"
    break;

  case 146: /* sequence_def: ICE_SEQUENCE '<' meta_data type '>' ICE_IDENTIFIER  */
#line 1733 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3641 "src/Slice/Grammar.cpp"
    break;

  case 147: /* sequence_def: ICE_SEQUENCE '<' meta_data type '>' keyword  */
#line 1741 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3654 "src/Slice/Grammar.cpp"
    break;

  case 148: /* dictionary_def: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' ICE_IDENTIFIER  */
#line 1755 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3668 "src/Slice/Grammar.cpp"
    break;

  case 149: /* dictionary_def: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' keyword  */
#line 1765 "src/Slice/Grammar.y"
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
#line 3683 "src/Slice/Grammar.cpp"
    break;

  case 150: /* enum_id: ICE_ENUM ICE_IDENTIFIER  */
#line 1781 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3691 "src/Slice/Grammar.cpp"
    break;

  case 151: /* enum_id: ICE_ENUM keyword  */
#line 1785 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3701 "src/Slice/Grammar.cpp"
    break;

  case 152: /* @21: %empty  */
#line 1796 "src/Slice/Grammar.y"
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
#line 3721 "src/Slice/Grammar.cpp"
    break;

  case 153: /* enum_def: enum_id @21 '{' enumerator_list '}'  */
#line 1812 "src/Slice/Grammar.y"
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
#line 3739 "src/Slice/Grammar.cpp"
    break;

  case 154: /* @22: %empty  */
#line 1827 "src/Slice/Grammar.y"
{
    unit->error("missing enumeration name");
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3751 "src/Slice/Grammar.cpp"
    break;

  case 155: /* enum_def: ICE_ENUM @22 '{' enumerator_list '}'  */
#line 1835 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3760 "src/Slice/Grammar.cpp"
    break;

  case 156: /* enumerator_list: enumerator ',' enumerator_list  */
#line 1845 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 3770 "src/Slice/Grammar.cpp"
    break;

  case 157: /* enumerator_list: enumerator  */
#line 1851 "src/Slice/Grammar.y"
{
}
#line 3777 "src/Slice/Grammar.cpp"
    break;

  case 158: /* enumerator: ICE_IDENTIFIER  */
#line 1859 "src/Slice/Grammar.y"
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
#line 3793 "src/Slice/Grammar.cpp"
    break;

  case 159: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1871 "src/Slice/Grammar.y"
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
#line 3817 "src/Slice/Grammar.cpp"
    break;

  case 160: /* enumerator: keyword  */
#line 1891 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 3828 "src/Slice/Grammar.cpp"
    break;

  case 161: /* enumerator: %empty  */
#line 1898 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 3837 "src/Slice/Grammar.cpp"
    break;

  case 162: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1908 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3845 "src/Slice/Grammar.cpp"
    break;

  case 163: /* enumerator_initializer: scoped_name  */
#line 1912 "src/Slice/Grammar.y"
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
            if(b && b->isIntegralType())
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
#line 3882 "src/Slice/Grammar.cpp"
    break;

  case 164: /* out_qualifier: ICE_OUT  */
#line 1950 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = true;
    yyval = out;
}
#line 3892 "src/Slice/Grammar.cpp"
    break;

  case 165: /* out_qualifier: %empty  */
#line 1956 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = false;
    yyval = out;
}
#line 3902 "src/Slice/Grammar.cpp"
    break;

  case 166: /* parameters: %empty  */
#line 1967 "src/Slice/Grammar.y"
{
}
#line 3909 "src/Slice/Grammar.cpp"
    break;

  case 167: /* parameters: out_qualifier meta_data tagged_type_id  */
#line 1970 "src/Slice/Grammar.y"
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
#line 3929 "src/Slice/Grammar.cpp"
    break;

  case 168: /* parameters: parameters ',' out_qualifier meta_data tagged_type_id  */
#line 1986 "src/Slice/Grammar.y"
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
#line 3949 "src/Slice/Grammar.cpp"
    break;

  case 169: /* parameters: out_qualifier meta_data type keyword  */
#line 2002 "src/Slice/Grammar.y"
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
#line 3965 "src/Slice/Grammar.cpp"
    break;

  case 170: /* parameters: parameters ',' out_qualifier meta_data type keyword  */
#line 2014 "src/Slice/Grammar.y"
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
#line 3981 "src/Slice/Grammar.cpp"
    break;

  case 171: /* parameters: out_qualifier meta_data type  */
#line 2026 "src/Slice/Grammar.y"
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
#line 3996 "src/Slice/Grammar.cpp"
    break;

  case 172: /* parameters: parameters ',' out_qualifier meta_data type  */
#line 2037 "src/Slice/Grammar.y"
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
#line 4011 "src/Slice/Grammar.cpp"
    break;

  case 173: /* throws: ICE_THROWS exception_list  */
#line 2053 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4019 "src/Slice/Grammar.cpp"
    break;

  case 174: /* throws: %empty  */
#line 2057 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 4027 "src/Slice/Grammar.cpp"
    break;

  case 175: /* scoped_name: ICE_IDENTIFIER  */
#line 2066 "src/Slice/Grammar.y"
{
}
#line 4034 "src/Slice/Grammar.cpp"
    break;

  case 176: /* scoped_name: ICE_SCOPED_IDENTIFIER  */
#line 2069 "src/Slice/Grammar.y"
{
}
#line 4041 "src/Slice/Grammar.cpp"
    break;

  case 177: /* builtin: ICE_BOOL  */
#line 2076 "src/Slice/Grammar.y"
           {}
#line 4047 "src/Slice/Grammar.cpp"
    break;

  case 178: /* builtin: ICE_BYTE  */
#line 2077 "src/Slice/Grammar.y"
           {}
#line 4053 "src/Slice/Grammar.cpp"
    break;

  case 179: /* builtin: ICE_SHORT  */
#line 2078 "src/Slice/Grammar.y"
            {}
#line 4059 "src/Slice/Grammar.cpp"
    break;

  case 180: /* builtin: ICE_INT  */
#line 2079 "src/Slice/Grammar.y"
          {}
#line 4065 "src/Slice/Grammar.cpp"
    break;

  case 181: /* builtin: ICE_LONG  */
#line 2080 "src/Slice/Grammar.y"
           {}
#line 4071 "src/Slice/Grammar.cpp"
    break;

  case 182: /* builtin: ICE_FLOAT  */
#line 2081 "src/Slice/Grammar.y"
            {}
#line 4077 "src/Slice/Grammar.cpp"
    break;

  case 183: /* builtin: ICE_DOUBLE  */
#line 2082 "src/Slice/Grammar.y"
             {}
#line 4083 "src/Slice/Grammar.cpp"
    break;

  case 184: /* builtin: ICE_STRING  */
#line 2083 "src/Slice/Grammar.y"
             {}
#line 4089 "src/Slice/Grammar.cpp"
    break;

  case 185: /* builtin: ICE_OBJECT  */
#line 2084 "src/Slice/Grammar.y"
             {}
#line 4095 "src/Slice/Grammar.cpp"
    break;

  case 186: /* builtin: ICE_VALUE  */
#line 2085 "src/Slice/Grammar.y"
            {}
#line 4101 "src/Slice/Grammar.cpp"
    break;

  case 187: /* type: ICE_OBJECT '*'  */
#line 2091 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindObjectProxy);
}
#line 4109 "src/Slice/Grammar.cpp"
    break;

  case 188: /* type: builtin  */
#line 2095 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 4118 "src/Slice/Grammar.cpp"
    break;

  case 189: /* type: scoped_name  */
#line 2100 "src/Slice/Grammar.y"
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
#line 4141 "src/Slice/Grammar.cpp"
    break;

  case 190: /* type: scoped_name '*'  */
#line 2119 "src/Slice/Grammar.y"
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
#line 4177 "src/Slice/Grammar.cpp"
    break;

  case 191: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 2156 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4187 "src/Slice/Grammar.cpp"
    break;

  case 192: /* string_literal: ICE_STRING_LITERAL  */
#line 2162 "src/Slice/Grammar.y"
{
}
#line 4194 "src/Slice/Grammar.cpp"
    break;

  case 193: /* string_list: string_list ',' string_literal  */
#line 2170 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4205 "src/Slice/Grammar.cpp"
    break;

  case 194: /* string_list: string_literal  */
#line 2177 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4216 "src/Slice/Grammar.cpp"
    break;

  case 195: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2189 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4229 "src/Slice/Grammar.cpp"
    break;

  case 196: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2198 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4242 "src/Slice/Grammar.cpp"
    break;

  case 197: /* const_initializer: scoped_name  */
#line 2207 "src/Slice/Grammar.y"
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
#line 4286 "src/Slice/Grammar.cpp"
    break;

  case 198: /* const_initializer: ICE_STRING_LITERAL  */
#line 2247 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4297 "src/Slice/Grammar.cpp"
    break;

  case 199: /* const_initializer: ICE_FALSE  */
#line 2254 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4308 "src/Slice/Grammar.cpp"
    break;

  case 200: /* const_initializer: ICE_TRUE  */
#line 2261 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4319 "src/Slice/Grammar.cpp"
    break;

  case 201: /* const_def: ICE_CONST meta_data type ICE_IDENTIFIER '=' const_initializer  */
#line 2273 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4332 "src/Slice/Grammar.cpp"
    break;

  case 202: /* const_def: ICE_CONST meta_data type '=' const_initializer  */
#line 2282 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4345 "src/Slice/Grammar.cpp"
    break;

  case 203: /* keyword: ICE_MODULE  */
#line 2295 "src/Slice/Grammar.y"
             {}
#line 4351 "src/Slice/Grammar.cpp"
    break;

  case 204: /* keyword: ICE_CLASS  */
#line 2296 "src/Slice/Grammar.y"
            {}
#line 4357 "src/Slice/Grammar.cpp"
    break;

  case 205: /* keyword: ICE_INTERFACE  */
#line 2297 "src/Slice/Grammar.y"
                {}
#line 4363 "src/Slice/Grammar.cpp"
    break;

  case 206: /* keyword: ICE_EXCEPTION  */
#line 2298 "src/Slice/Grammar.y"
                {}
#line 4369 "src/Slice/Grammar.cpp"
    break;

  case 207: /* keyword: ICE_STRUCT  */
#line 2299 "src/Slice/Grammar.y"
             {}
#line 4375 "src/Slice/Grammar.cpp"
    break;

  case 208: /* keyword: ICE_SEQUENCE  */
#line 2300 "src/Slice/Grammar.y"
               {}
#line 4381 "src/Slice/Grammar.cpp"
    break;

  case 209: /* keyword: ICE_DICTIONARY  */
#line 2301 "src/Slice/Grammar.y"
                 {}
#line 4387 "src/Slice/Grammar.cpp"
    break;

  case 210: /* keyword: ICE_ENUM  */
#line 2302 "src/Slice/Grammar.y"
           {}
#line 4393 "src/Slice/Grammar.cpp"
    break;

  case 211: /* keyword: ICE_OUT  */
#line 2303 "src/Slice/Grammar.y"
          {}
#line 4399 "src/Slice/Grammar.cpp"
    break;

  case 212: /* keyword: ICE_EXTENDS  */
#line 2304 "src/Slice/Grammar.y"
              {}
#line 4405 "src/Slice/Grammar.cpp"
    break;

  case 213: /* keyword: ICE_IMPLEMENTS  */
#line 2305 "src/Slice/Grammar.y"
                 {}
#line 4411 "src/Slice/Grammar.cpp"
    break;

  case 214: /* keyword: ICE_THROWS  */
#line 2306 "src/Slice/Grammar.y"
             {}
#line 4417 "src/Slice/Grammar.cpp"
    break;

  case 215: /* keyword: ICE_VOID  */
#line 2307 "src/Slice/Grammar.y"
           {}
#line 4423 "src/Slice/Grammar.cpp"
    break;

  case 216: /* keyword: ICE_BOOL  */
#line 2308 "src/Slice/Grammar.y"
           {}
#line 4429 "src/Slice/Grammar.cpp"
    break;

  case 217: /* keyword: ICE_BYTE  */
#line 2309 "src/Slice/Grammar.y"
           {}
#line 4435 "src/Slice/Grammar.cpp"
    break;

  case 218: /* keyword: ICE_SHORT  */
#line 2310 "src/Slice/Grammar.y"
            {}
#line 4441 "src/Slice/Grammar.cpp"
    break;

  case 219: /* keyword: ICE_INT  */
#line 2311 "src/Slice/Grammar.y"
          {}
#line 4447 "src/Slice/Grammar.cpp"
    break;

  case 220: /* keyword: ICE_LONG  */
#line 2312 "src/Slice/Grammar.y"
           {}
#line 4453 "src/Slice/Grammar.cpp"
    break;

  case 221: /* keyword: ICE_FLOAT  */
#line 2313 "src/Slice/Grammar.y"
            {}
#line 4459 "src/Slice/Grammar.cpp"
    break;

  case 222: /* keyword: ICE_DOUBLE  */
#line 2314 "src/Slice/Grammar.y"
             {}
#line 4465 "src/Slice/Grammar.cpp"
    break;

  case 223: /* keyword: ICE_STRING  */
#line 2315 "src/Slice/Grammar.y"
             {}
#line 4471 "src/Slice/Grammar.cpp"
    break;

  case 224: /* keyword: ICE_OBJECT  */
#line 2316 "src/Slice/Grammar.y"
             {}
#line 4477 "src/Slice/Grammar.cpp"
    break;

  case 225: /* keyword: ICE_CONST  */
#line 2317 "src/Slice/Grammar.y"
            {}
#line 4483 "src/Slice/Grammar.cpp"
    break;

  case 226: /* keyword: ICE_FALSE  */
#line 2318 "src/Slice/Grammar.y"
            {}
#line 4489 "src/Slice/Grammar.cpp"
    break;

  case 227: /* keyword: ICE_TRUE  */
#line 2319 "src/Slice/Grammar.y"
           {}
#line 4495 "src/Slice/Grammar.cpp"
    break;

  case 228: /* keyword: ICE_IDEMPOTENT  */
#line 2320 "src/Slice/Grammar.y"
                 {}
#line 4501 "src/Slice/Grammar.cpp"
    break;

  case 229: /* keyword: ICE_TAG  */
#line 2321 "src/Slice/Grammar.y"
          {}
#line 4507 "src/Slice/Grammar.cpp"
    break;

  case 230: /* keyword: ICE_OPTIONAL  */
#line 2322 "src/Slice/Grammar.y"
               {}
#line 4513 "src/Slice/Grammar.cpp"
    break;

  case 231: /* keyword: ICE_VALUE  */
#line 2323 "src/Slice/Grammar.y"
            {}
#line 4519 "src/Slice/Grammar.cpp"
    break;


#line 4523 "src/Slice/Grammar.cpp"

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

#line 2326 "src/Slice/Grammar.y"

