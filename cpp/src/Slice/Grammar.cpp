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


#line 170 "src/Slice/Grammar.cpp"

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
  YYSYMBOL_ICE_BYTE = 16,                  /* ICE_BYTE  */
  YYSYMBOL_ICE_BOOL = 17,                  /* ICE_BOOL  */
  YYSYMBOL_ICE_SHORT = 18,                 /* ICE_SHORT  */
  YYSYMBOL_ICE_INT = 19,                   /* ICE_INT  */
  YYSYMBOL_ICE_LONG = 20,                  /* ICE_LONG  */
  YYSYMBOL_ICE_FLOAT = 21,                 /* ICE_FLOAT  */
  YYSYMBOL_ICE_DOUBLE = 22,                /* ICE_DOUBLE  */
  YYSYMBOL_ICE_STRING = 23,                /* ICE_STRING  */
  YYSYMBOL_ICE_OBJECT = 24,                /* ICE_OBJECT  */
  YYSYMBOL_ICE_LOCAL_OBJECT = 25,          /* ICE_LOCAL_OBJECT  */
  YYSYMBOL_ICE_LOCAL = 26,                 /* ICE_LOCAL  */
  YYSYMBOL_ICE_CONST = 27,                 /* ICE_CONST  */
  YYSYMBOL_ICE_FALSE = 28,                 /* ICE_FALSE  */
  YYSYMBOL_ICE_TRUE = 29,                  /* ICE_TRUE  */
  YYSYMBOL_ICE_IDEMPOTENT = 30,            /* ICE_IDEMPOTENT  */
  YYSYMBOL_ICE_TAG = 31,                   /* ICE_TAG  */
  YYSYMBOL_ICE_OPTIONAL = 32,              /* ICE_OPTIONAL  */
  YYSYMBOL_ICE_VALUE = 33,                 /* ICE_VALUE  */
  YYSYMBOL_ICE_STRING_LITERAL = 34,        /* ICE_STRING_LITERAL  */
  YYSYMBOL_ICE_INTEGER_LITERAL = 35,       /* ICE_INTEGER_LITERAL  */
  YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 36, /* ICE_FLOATING_POINT_LITERAL  */
  YYSYMBOL_ICE_IDENTIFIER = 37,            /* ICE_IDENTIFIER  */
  YYSYMBOL_ICE_SCOPED_IDENTIFIER = 38,     /* ICE_SCOPED_IDENTIFIER  */
  YYSYMBOL_ICE_METADATA_OPEN = 39,         /* ICE_METADATA_OPEN  */
  YYSYMBOL_ICE_METADATA_CLOSE = 40,        /* ICE_METADATA_CLOSE  */
  YYSYMBOL_ICE_GLOBAL_METADATA_OPEN = 41,  /* ICE_GLOBAL_METADATA_OPEN  */
  YYSYMBOL_ICE_GLOBAL_METADATA_IGNORE = 42, /* ICE_GLOBAL_METADATA_IGNORE  */
  YYSYMBOL_ICE_GLOBAL_METADATA_CLOSE = 43, /* ICE_GLOBAL_METADATA_CLOSE  */
  YYSYMBOL_ICE_IDENT_OPEN = 44,            /* ICE_IDENT_OPEN  */
  YYSYMBOL_ICE_KEYWORD_OPEN = 45,          /* ICE_KEYWORD_OPEN  */
  YYSYMBOL_ICE_TAG_OPEN = 46,              /* ICE_TAG_OPEN  */
  YYSYMBOL_ICE_OPTIONAL_OPEN = 47,         /* ICE_OPTIONAL_OPEN  */
  YYSYMBOL_BAD_CHAR = 48,                  /* BAD_CHAR  */
  YYSYMBOL_49_ = 49,                       /* ';'  */
  YYSYMBOL_50_ = 50,                       /* '{'  */
  YYSYMBOL_51_ = 51,                       /* '}'  */
  YYSYMBOL_52_ = 52,                       /* ')'  */
  YYSYMBOL_53_ = 53,                       /* ':'  */
  YYSYMBOL_54_ = 54,                       /* '='  */
  YYSYMBOL_55_ = 55,                       /* ','  */
  YYSYMBOL_56_ = 56,                       /* '<'  */
  YYSYMBOL_57_ = 57,                       /* '>'  */
  YYSYMBOL_58_ = 58,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 59,                  /* $accept  */
  YYSYMBOL_start = 60,                     /* start  */
  YYSYMBOL_opt_semicolon = 61,             /* opt_semicolon  */
  YYSYMBOL_global_meta_data = 62,          /* global_meta_data  */
  YYSYMBOL_meta_data = 63,                 /* meta_data  */
  YYSYMBOL_definitions = 64,               /* definitions  */
  YYSYMBOL_definition = 65,                /* definition  */
  YYSYMBOL_66_1 = 66,                      /* $@1  */
  YYSYMBOL_67_2 = 67,                      /* $@2  */
  YYSYMBOL_68_3 = 68,                      /* $@3  */
  YYSYMBOL_69_4 = 69,                      /* $@4  */
  YYSYMBOL_70_5 = 70,                      /* $@5  */
  YYSYMBOL_71_6 = 71,                      /* $@6  */
  YYSYMBOL_72_7 = 72,                      /* $@7  */
  YYSYMBOL_73_8 = 73,                      /* $@8  */
  YYSYMBOL_74_9 = 74,                      /* $@9  */
  YYSYMBOL_75_10 = 75,                     /* $@10  */
  YYSYMBOL_76_11 = 76,                     /* $@11  */
  YYSYMBOL_77_12 = 77,                     /* $@12  */
  YYSYMBOL_78_13 = 78,                     /* $@13  */
  YYSYMBOL_module_def = 79,                /* module_def  */
  YYSYMBOL_80_14 = 80,                     /* @14  */
  YYSYMBOL_exception_id = 81,              /* exception_id  */
  YYSYMBOL_exception_decl = 82,            /* exception_decl  */
  YYSYMBOL_exception_def = 83,             /* exception_def  */
  YYSYMBOL_84_15 = 84,                     /* @15  */
  YYSYMBOL_exception_extends = 85,         /* exception_extends  */
  YYSYMBOL_exception_exports = 86,         /* exception_exports  */
  YYSYMBOL_type_id = 87,                   /* type_id  */
  YYSYMBOL_tag = 88,                       /* tag  */
  YYSYMBOL_optional = 89,                  /* optional  */
  YYSYMBOL_tagged_type_id = 90,            /* tagged_type_id  */
  YYSYMBOL_exception_export = 91,          /* exception_export  */
  YYSYMBOL_struct_id = 92,                 /* struct_id  */
  YYSYMBOL_struct_decl = 93,               /* struct_decl  */
  YYSYMBOL_struct_def = 94,                /* struct_def  */
  YYSYMBOL_95_16 = 95,                     /* @16  */
  YYSYMBOL_struct_exports = 96,            /* struct_exports  */
  YYSYMBOL_struct_export = 97,             /* struct_export  */
  YYSYMBOL_class_name = 98,                /* class_name  */
  YYSYMBOL_class_id = 99,                  /* class_id  */
  YYSYMBOL_class_decl = 100,               /* class_decl  */
  YYSYMBOL_class_def = 101,                /* class_def  */
  YYSYMBOL_102_17 = 102,                   /* @17  */
  YYSYMBOL_class_extends = 103,            /* class_extends  */
  YYSYMBOL_extends = 104,                  /* extends  */
  YYSYMBOL_implements = 105,               /* implements  */
  YYSYMBOL_class_exports = 106,            /* class_exports  */
  YYSYMBOL_data_member = 107,              /* data_member  */
  YYSYMBOL_struct_data_member = 108,       /* struct_data_member  */
  YYSYMBOL_return_type = 109,              /* return_type  */
  YYSYMBOL_operation_preamble = 110,       /* operation_preamble  */
  YYSYMBOL_operation = 111,                /* operation  */
  YYSYMBOL_112_18 = 112,                   /* @18  */
  YYSYMBOL_113_19 = 113,                   /* @19  */
  YYSYMBOL_class_export = 114,             /* class_export  */
  YYSYMBOL_interface_id = 115,             /* interface_id  */
  YYSYMBOL_interface_decl = 116,           /* interface_decl  */
  YYSYMBOL_interface_def = 117,            /* interface_def  */
  YYSYMBOL_118_20 = 118,                   /* @20  */
  YYSYMBOL_interface_list = 119,           /* interface_list  */
  YYSYMBOL_interface_extends = 120,        /* interface_extends  */
  YYSYMBOL_interface_exports = 121,        /* interface_exports  */
  YYSYMBOL_interface_export = 122,         /* interface_export  */
  YYSYMBOL_exception_list = 123,           /* exception_list  */
  YYSYMBOL_exception = 124,                /* exception  */
  YYSYMBOL_sequence_def = 125,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 126,           /* dictionary_def  */
  YYSYMBOL_enum_id = 127,                  /* enum_id  */
  YYSYMBOL_enum_def = 128,                 /* enum_def  */
  YYSYMBOL_129_21 = 129,                   /* @21  */
  YYSYMBOL_130_22 = 130,                   /* @22  */
  YYSYMBOL_enumerator_list = 131,          /* enumerator_list  */
  YYSYMBOL_enumerator = 132,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 133,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 134,            /* out_qualifier  */
  YYSYMBOL_parameters = 135,               /* parameters  */
  YYSYMBOL_throws = 136,                   /* throws  */
  YYSYMBOL_scoped_name = 137,              /* scoped_name  */
  YYSYMBOL_type = 138,                     /* type  */
  YYSYMBOL_string_literal = 139,           /* string_literal  */
  YYSYMBOL_string_list = 140,              /* string_list  */
  YYSYMBOL_local_qualifier = 141,          /* local_qualifier  */
  YYSYMBOL_const_initializer = 142,        /* const_initializer  */
  YYSYMBOL_const_def = 143,                /* const_def  */
  YYSYMBOL_keyword = 144                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 58 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 357 "src/Slice/Grammar.cpp"

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
#define YYLAST   967

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  59
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  86
/* YYNRULES -- Number of rules.  */
#define YYNRULES  235
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  345

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   303


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
       2,    52,    58,     2,    55,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    53,    49,
      56,    54,    57,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,     2,    51,     2,     2,     2,     2,
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
      45,    46,    47,    48
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   184,   184,   192,   195,   203,   207,   217,   221,   230,
     238,   247,   256,   255,   261,   260,   265,   270,   269,   275,
     274,   279,   284,   283,   289,   288,   293,   298,   297,   303,
     302,   307,   312,   311,   317,   316,   321,   326,   325,   330,
     335,   334,   340,   339,   344,   348,   358,   357,   390,   394,
     405,   416,   415,   442,   450,   459,   468,   471,   475,   483,
     496,   514,   605,   611,   622,   640,   730,   736,   747,   756,
     765,   778,   784,   788,   799,   810,   809,   851,   860,   863,
     867,   875,   881,   885,   896,   921,  1023,  1035,  1049,  1048,
    1088,  1122,  1131,  1134,  1142,  1146,  1155,  1164,  1167,  1171,
    1179,  1201,  1228,  1250,  1276,  1285,  1296,  1305,  1314,  1323,
    1332,  1342,  1356,  1362,  1368,  1374,  1384,  1408,  1433,  1457,
    1488,  1487,  1510,  1509,  1532,  1533,  1539,  1543,  1554,  1569,
    1568,  1603,  1638,  1673,  1678,  1688,  1692,  1701,  1710,  1713,
    1717,  1725,  1731,  1738,  1750,  1762,  1773,  1782,  1797,  1808,
    1825,  1829,  1841,  1840,  1873,  1872,  1891,  1897,  1905,  1917,
    1937,  1944,  1954,  1958,  1997,  2003,  2014,  2017,  2033,  2049,
    2061,  2073,  2084,  2100,  2104,  2113,  2116,  2124,  2128,  2132,
    2136,  2140,  2144,  2148,  2152,  2156,  2160,  2164,  2168,  2172,
    2191,  2232,  2238,  2246,  2253,  2265,  2271,  2282,  2291,  2300,
    2340,  2347,  2354,  2366,  2375,  2389,  2392,  2395,  2398,  2401,
    2404,  2407,  2410,  2413,  2416,  2419,  2422,  2425,  2428,  2431,
    2434,  2437,  2440,  2443,  2446,  2449,  2452,  2455,  2458,  2461,
    2464,  2467,  2470,  2473,  2476,  2479
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
  "ICE_IMPLEMENTS", "ICE_THROWS", "ICE_VOID", "ICE_BYTE", "ICE_BOOL",
  "ICE_SHORT", "ICE_INT", "ICE_LONG", "ICE_FLOAT", "ICE_DOUBLE",
  "ICE_STRING", "ICE_OBJECT", "ICE_LOCAL_OBJECT", "ICE_LOCAL", "ICE_CONST",
  "ICE_FALSE", "ICE_TRUE", "ICE_IDEMPOTENT", "ICE_TAG", "ICE_OPTIONAL",
  "ICE_VALUE", "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
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
  "scoped_name", "type", "string_literal", "string_list",
  "local_qualifier", "const_initializer", "const_def", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-297)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-197)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -297,    37,    19,  -297,    11,    11,    11,  -297,    88,    11,
    -297,    -4,    28,    45,    18,    35,  -297,    42,  -297,  -297,
      54,  -297,    56,  -297,    61,  -297,    69,  -297,    72,    77,
    -297,   231,    94,  -297,  -297,    11,  -297,  -297,  -297,  -297,
     247,   101,   102,   101,   104,   101,   106,   101,   110,   101,
     125,   130,   101,   509,   587,   622,   657,    96,   124,   692,
       0,   131,    12,     6,    16,  -297,   136,  -297,   137,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,   128,  -297,  -297,
    -297,  -297,   140,   -10,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,    82,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,    42,    42,  -297,
     139,  -297,  -297,  -297,  -297,     5,   149,   182,     5,    92,
    -297,   150,  -297,  -297,  -297,  -297,   147,   103,   153,   155,
     247,   247,   727,   159,  -297,   378,    92,  -297,  -297,  -297,
    -297,  -297,   156,   162,   727,    60,   103,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,   158,   163,   165,   166,
     168,  -297,   417,   167,   920,   169,  -297,   174,    92,   145,
     178,  -297,  -297,   762,    42,   107,  -297,   727,   176,   920,
     179,   378,  -297,  -297,    -5,    38,   177,   247,   247,   183,
    -297,   797,  -297,   339,  -297,   184,   855,   191,  -297,  -297,
    -297,   247,  -297,  -297,  -297,  -297,   417,  -297,   247,   247,
     189,   195,  -297,   797,  -297,  -297,   194,  -297,   196,   197,
    -297,   198,   103,   193,   216,   200,   378,  -297,  -297,   206,
     855,   205,   145,  -297,   888,   247,   247,    64,   298,  -297,
     208,  -297,  -297,   202,  -297,  -297,  -297,   103,   417,  -297,
    -297,  -297,  -297,  -297,  -297,   103,   103,  -297,   339,   247,
     247,  -297,  -297,   212,   466,  -297,  -297,    83,  -297,  -297,
    -297,  -297,   210,  -297,    42,    30,   145,   832,  -297,  -297,
    -297,  -297,  -297,   216,   216,   339,  -297,  -297,  -297,   920,
    -297,   262,  -297,  -297,  -297,  -297,   260,  -297,   797,   260,
      42,   551,  -297,  -297,  -297,   920,  -297,   220,  -297,  -297,
    -297,   797,   551,  -297,  -297
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      11,     0,     8,     1,     0,     0,     0,     9,     0,   192,
     194,     0,     0,     0,     0,     0,   195,     8,    10,    12,
      26,    27,    31,    32,    16,    17,    21,    22,    36,    39,
      40,     0,    44,   191,     7,     0,     5,     6,    45,    46,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     0,     4,     0,     0,     0,     0,     0,     0,   154,
      50,    74,    87,    91,   128,   152,     0,   193,     0,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   187,   188,
     175,   176,   189,     0,     3,    13,    25,    28,    30,    33,
      15,    18,    20,    23,    35,    38,    41,   205,   206,   207,
     208,   209,   210,   211,   212,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,    82,     0,
      83,   126,   127,    48,    49,    72,    73,     8,     8,   150,
       0,   151,    92,    93,    51,     0,     0,    95,     0,     0,
     129,     0,    43,    11,   186,   190,     0,     0,     0,     0,
       0,     0,   161,     0,    53,     0,     0,    88,    90,   133,
     134,   135,   132,     0,   161,     8,     0,   201,   202,   200,
     197,   198,   199,   204,    84,    85,     0,     0,   158,     0,
     157,   160,     0,     0,     0,     0,    94,     0,     0,     0,
       0,    47,   203,     0,     8,     0,   155,   161,     0,     0,
       0,     0,    63,    67,     0,     0,   104,     0,     0,    79,
      81,   111,    76,     0,   131,     0,     0,     0,   153,   146,
     147,     0,   162,   159,   163,   156,     0,    70,     0,     0,
     100,    57,    71,   103,    52,    78,     0,    62,     0,     0,
      66,     0,     0,   106,     0,   108,     0,    59,   110,     0,
       0,     0,     0,   115,     0,     0,     0,     0,     0,   141,
     139,   114,   130,     0,    56,    68,    69,     0,     0,   102,
      60,    61,    64,    65,   105,     0,     0,    77,     0,     0,
       0,   124,   125,    98,   103,    89,   138,     0,   112,   113,
     116,   118,     0,   164,     8,     0,     0,     0,   101,    55,
     107,   109,    97,   112,   113,     0,   117,   119,   122,     0,
     120,   165,   137,   148,   149,    96,   174,   167,   171,   174,
       8,     0,   123,   169,   121,     0,   173,   143,   144,   145,
     168,   172,     0,   170,   142
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -297,  -297,   161,  -297,    -2,   134,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,  -297,  -297,  -214,  -183,  -186,
    -180,  -296,  -297,  -297,  -297,  -297,  -297,  -202,  -297,  -297,
    -297,  -297,  -297,  -297,  -297,    70,  -297,  -247,    23,  -297,
      13,  -297,    29,  -297,  -297,  -297,  -297,  -297,  -297,  -297,
    -135,  -297,  -249,  -297,   -64,  -297,  -297,  -297,  -297,  -297,
    -297,  -297,  -158,  -297,  -297,   -33,  -297,   -39,  -128,   -38,
      17,   142,  -297,  -173,  -297,   -49
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,    85,     7,   194,     2,    18,    41,    46,    47,
      48,    49,    42,    43,    44,    45,    50,    51,    52,    66,
      19,    68,    60,    20,    21,   163,   144,   210,   237,   238,
     239,   240,   241,    61,    22,    23,   146,   195,   219,    62,
      63,    24,    25,   197,   147,   145,   167,   261,   242,   220,
     267,   268,   269,   329,   326,   293,    64,    26,    27,   173,
     171,   150,   227,   270,   336,   337,    28,    29,    65,    30,
     151,   140,   189,   190,   233,   304,   305,   332,    82,   254,
      10,    11,    31,   183,    32,   191
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       8,   159,    83,   202,   130,   132,   134,   136,   217,   245,
     141,   216,   142,   296,   218,    40,   200,   164,   142,    -2,
     168,   172,   274,   327,   -86,   -86,    33,   156,   142,   182,
     246,   196,    80,    81,   253,   255,    34,     3,   172,   340,
     265,   312,    80,    81,   157,     9,   266,   247,   182,   235,
     -54,    35,    67,   143,   287,   275,   276,   322,     4,   143,
       5,     6,   -86,   224,   309,   -86,  -136,    38,   325,   143,
     172,    36,    39,   249,   289,    80,    81,   234,   265,   284,
     290,     4,   320,    35,   266,   321,   248,   251,    37,    14,
     250,    15,  -196,  -196,  -196,  -196,  -196,  -196,  -196,     4,
      35,     5,     6,   -24,   308,   -29,   275,   276,   300,   301,
     -14,   201,   310,   311,    16,    17,   169,   158,   -19,    80,
      81,   -34,   186,   187,   182,   170,   -37,   316,   317,    80,
      81,   177,   178,   148,   149,   160,   161,   179,   180,   181,
      80,    81,   232,   -42,    80,    81,   225,    12,    13,   182,
      84,    86,   137,    88,   230,    90,   221,   182,   182,    92,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,   243,   258,     8,    94,    -8,    -8,    -8,    -8,    95,
     138,   -75,    -8,    -8,     4,   152,   154,   153,   271,   162,
     209,    -8,    -8,   273,   279,   166,  -140,   226,   155,   165,
     174,   176,   231,   338,    87,   184,    89,   185,    91,   192,
      93,   198,   199,    96,   338,   203,   211,   206,   204,   205,
     222,   260,   294,   207,   223,   236,   271,   298,   299,   228,
     244,   252,   256,   262,   209,    53,    54,    55,    56,    57,
      58,    59,   272,   277,   278,   279,   280,   285,   281,   282,
     283,   313,   314,   257,   286,   288,   295,   306,   324,   307,
     226,   315,   318,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,   303,   331,   342,   209,   297,   344,   333,
      79,   328,   339,   291,    80,    81,   260,   175,   330,   292,
     334,     0,   343,   339,     0,     0,     0,   341,     0,   302,
       0,     0,   319,     0,   226,     0,     0,     0,     0,   303,
       0,     0,     0,   260,  -165,  -165,  -165,  -165,  -165,  -165,
    -165,  -165,  -165,  -165,     0,     0,     0,     0,   335,  -165,
    -165,  -165,     0,     0,     0,  -165,  -165,  -165,     0,     0,
     259,     0,     0,     0,  -165,  -165,     0,     0,     0,     0,
    -166,     0,     0,  -166,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,     0,     0,     0,     0,    -8,
      -8,    -8,    -8,     0,     0,     0,    -8,    -8,     4,   193,
       0,     0,     0,     0,     0,    -8,    -8,     0,     0,     0,
     -99,     0,     0,     0,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,     0,     0,     0,     0,     0,    -8,
      -8,    -8,     0,     0,     0,    -8,    -8,     4,   208,     0,
       0,     0,     0,     0,    -8,    -8,     0,     0,     0,   -80,
       0,     0,     0,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,     0,     0,     0,     0,     0,    -8,    -8,
      -8,     0,     0,     0,    -8,    -8,     4,     0,     0,     0,
       0,     0,     0,    -8,    -8,     0,     0,     0,   -58,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
       0,     0,     0,   257,     0,     0,     0,     0,     0,     0,
    -114,  -114,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,     0,     0,     0,   128,     0,     0,     0,
       0,     0,     0,   129,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,     0,     0,     0,    80,    81,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     0,     0,     0,   131,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,     0,     0,     0,   133,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     0,     0,     0,   135,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,     0,     0,     0,   139,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     0,     0,     0,   188,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,     0,     0,     0,   229,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,   123,   124,   125,   126,
     127,     0,     0,     0,   257,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,     0,     0,     0,   323,
     263,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,     0,     0,     0,     0,   264,   212,   213,    79,     0,
       0,     0,    80,    81,     0,     0,     0,     0,     0,     0,
       0,   214,   215,   263,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,     0,     0,     0,   212,
     213,    79,     0,     0,     0,    80,    81,     0,     0,     0,
       0,     0,     0,     0,   214,   215,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     0,     0,     0,     0,
       0,   212,   213,    79,     0,     0,     0,    80,    81,     0,
       0,     0,     0,     0,     0,     0,   214,   215
};

static const yytype_int16 yycheck[] =
{
       2,   129,    40,   176,    53,    54,    55,    56,   194,   211,
      59,   194,    12,   262,   194,    17,   174,   145,    12,     0,
     148,   149,   236,   319,    12,    13,     9,    37,    12,   157,
      35,   166,    37,    38,   217,   218,    40,     0,   166,   335,
     226,   288,    37,    38,    54,    34,   226,    52,   176,   207,
      50,    55,    35,    53,   256,   238,   239,   306,    39,    53,
      41,    42,    50,   198,   278,    53,    50,    49,   315,    53,
     198,    43,    37,    35,   260,    37,    38,   205,   264,   252,
     260,    39,    52,    55,   264,    55,   214,   215,    43,     1,
      52,     3,     4,     5,     6,     7,     8,     9,    10,    39,
      55,    41,    42,    49,   277,    49,   289,   290,    44,    45,
      49,    51,   285,   286,    26,    27,    24,    35,    49,    37,
      38,    49,   160,   161,   252,    33,    49,    44,    45,    37,
      38,    28,    29,    63,    64,   137,   138,    34,    35,    36,
      37,    38,    35,    49,    37,    38,     1,     5,     6,   277,
      49,    49,    56,    49,   203,    49,   194,   285,   286,    49,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   209,   221,   175,    49,    30,    31,    32,    33,    49,
      56,    50,    37,    38,    39,    49,    58,    50,   226,    50,
     192,    46,    47,   231,   243,    13,    51,   199,    58,    50,
      50,    54,   204,   331,    43,    52,    45,    52,    47,    50,
      49,    55,    50,    52,   342,    57,    49,    51,    55,    54,
      51,   223,   260,    55,    50,    49,   264,   265,   266,    51,
      51,    54,    49,    49,   236,     4,     5,     6,     7,     8,
       9,    10,    51,    54,    49,   294,    52,    54,    52,    52,
      52,   289,   290,    37,    54,    49,    51,    49,   307,    57,
     262,    49,    52,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    11,    14,    55,   278,   264,   342,   328,
      33,   319,   331,   260,    37,    38,   288,   153,   321,   260,
     329,    -1,   341,   342,    -1,    -1,    -1,   335,    -1,     1,
      -1,    -1,   304,    -1,   306,    -1,    -1,    -1,    -1,    11,
      -1,    -1,    -1,   315,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,   330,    31,
      32,    33,    -1,    -1,    -1,    37,    38,    39,    -1,    -1,
       1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    -1,    -1,    55,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,    30,
      31,    32,    33,    -1,    -1,    -1,    37,    38,    39,     1,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      51,    -1,    -1,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      32,    33,    -1,    -1,    -1,    37,    38,    39,     1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,    32,
      33,    -1,    -1,    -1,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    51,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,
      44,    45,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    -1,    -1,    -1,    37,    -1,    -1,    -1,
      -1,    -1,    -1,    44,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    -1,    -1,    37,    38,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    -1,    -1,    37,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    -1,    37,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    -1,    -1,    37,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    -1,    37,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    -1,    -1,    37,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    -1,    37,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    -1,    -1,    37,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    -1,    37,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    -1,    -1,    -1,    -1,    30,    31,    32,    33,    -1,
      -1,    -1,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    46,    47,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,    31,
      32,    33,    -1,    -1,    -1,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    46,    47,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    -1,    -1,    -1,    -1,
      -1,    31,    32,    33,    -1,    -1,    -1,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    46,    47
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    60,    64,     0,    39,    41,    42,    62,    63,    34,
     139,   140,   140,   140,     1,     3,    26,    27,    65,    79,
      82,    83,    93,    94,   100,   101,   116,   117,   125,   126,
     128,   141,   143,   139,    40,    55,    43,    43,    49,    37,
      63,    66,    71,    72,    73,    74,    67,    68,    69,    70,
      75,    76,    77,     4,     5,     6,     7,     8,     9,    10,
      81,    92,    98,    99,   115,   127,    78,   139,    80,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    33,
      37,    38,   137,   138,    49,    61,    49,    61,    49,    61,
      49,    61,    49,    61,    49,    49,    61,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    37,    44,
     144,    37,   144,    37,   144,    37,   144,    56,    56,    37,
     130,   144,    12,    53,    85,   104,    95,   103,   104,   104,
     120,   129,    49,    50,    58,    58,    37,    54,    35,   137,
      63,    63,    50,    84,   137,    50,    13,   105,   137,    24,
      33,   119,   137,   118,    50,    64,    54,    28,    29,    34,
      35,    36,   137,   142,    52,    52,   138,   138,    37,   131,
     132,   144,    50,     1,    63,    96,   119,   102,    55,    50,
     131,    51,   142,    57,    55,    54,    51,    55,     1,    63,
      86,    49,    31,    32,    46,    47,    87,    88,    89,    97,
     108,   138,    51,    50,   119,     1,    63,   121,    51,    37,
     144,    63,    35,   133,   137,   131,    49,    87,    88,    89,
      90,    91,   107,   138,    51,    96,    35,    52,   137,    35,
      52,   137,    54,    87,   138,    87,    49,    37,   144,     1,
      63,   106,    49,    15,    30,    88,    89,   109,   110,   111,
     122,   138,    51,   138,    86,    87,    87,    54,    49,   144,
      52,    52,    52,    52,   142,    54,    54,    96,    49,    88,
      89,   107,   111,   114,   138,    51,   121,   109,   138,   138,
      44,    45,     1,    11,   134,   135,    49,    57,   142,    86,
     142,   142,   106,   138,   138,    49,    44,    45,    52,    63,
      52,    55,   121,    37,   144,   106,   113,    90,   138,   112,
     134,    14,   136,   144,   136,    63,   123,   124,   137,   144,
      90,   138,    55,   144,   123
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    59,    60,    61,    61,    62,    62,    63,    63,    64,
      64,    64,    66,    65,    67,    65,    65,    68,    65,    69,
      65,    65,    70,    65,    71,    65,    65,    72,    65,    73,
      65,    65,    74,    65,    75,    65,    65,    76,    65,    65,
      77,    65,    78,    65,    65,    65,    80,    79,    81,    81,
      82,    84,    83,    85,    85,    86,    86,    86,    86,    87,
      88,    88,    88,    88,    89,    89,    89,    89,    90,    90,
      90,    91,    92,    92,    93,    95,    94,    96,    96,    96,
      96,    97,    98,    98,    99,    99,    99,   100,   102,   101,
     103,   103,   104,   104,   105,   105,   106,   106,   106,   106,
     107,   107,   107,   107,   108,   108,   108,   108,   108,   108,
     108,   108,   109,   109,   109,   109,   110,   110,   110,   110,
     112,   111,   113,   111,   114,   114,   115,   115,   116,   118,
     117,   119,   119,   119,   119,   120,   120,   121,   121,   121,
     121,   122,   123,   123,   124,   124,   125,   125,   126,   126,
     127,   127,   129,   128,   130,   128,   131,   131,   132,   132,
     132,   132,   133,   133,   134,   134,   135,   135,   135,   135,
     135,   135,   135,   136,   136,   137,   137,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   138,   138,   138,
     138,   139,   139,   140,   140,   141,   141,   142,   142,   142,
     142,   142,   142,   143,   143,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   144,   144
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     3,     0,     2,
       3,     0,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     1,
       0,     3,     0,     3,     1,     2,     0,     6,     2,     2,
       2,     0,     7,     2,     0,     4,     3,     2,     0,     2,
       3,     3,     2,     1,     3,     3,     2,     1,     2,     2,
       1,     1,     2,     2,     2,     0,     6,     4,     3,     2,
       0,     1,     2,     2,     4,     4,     1,     2,     0,     8,
       2,     0,     1,     1,     2,     0,     4,     3,     2,     0,
       1,     3,     2,     1,     1,     3,     2,     4,     2,     4,
       2,     1,     2,     2,     1,     1,     2,     3,     2,     3,
       0,     5,     0,     5,     1,     1,     2,     2,     2,     0,
       7,     3,     1,     1,     1,     2,     0,     4,     3,     2,
       0,     1,     3,     1,     1,     1,     7,     7,    10,    10,
       2,     2,     0,     6,     0,     6,     3,     1,     1,     3,
       1,     0,     1,     1,     1,     0,     0,     3,     5,     4,
       6,     3,     5,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     1,
       2,     2,     1,     3,     1,     1,     0,     1,     1,     1,
       1,     1,     1,     6,     5,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
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
#line 1835 "src/Slice/Grammar.cpp"
    break;

  case 3: /* opt_semicolon: ';'  */
#line 193 "src/Slice/Grammar.y"
{
}
#line 1842 "src/Slice/Grammar.cpp"
    break;

  case 4: /* opt_semicolon: %empty  */
#line 196 "src/Slice/Grammar.y"
{
}
#line 1849 "src/Slice/Grammar.cpp"
    break;

  case 5: /* global_meta_data: ICE_GLOBAL_METADATA_OPEN string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 204 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1857 "src/Slice/Grammar.cpp"
    break;

  case 6: /* global_meta_data: ICE_GLOBAL_METADATA_IGNORE string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 208 "src/Slice/Grammar.y"
{
    unit->error("global metadata must appear before any definitions");
    yyval = yyvsp[-1]; // Dummy
}
#line 1866 "src/Slice/Grammar.cpp"
    break;

  case 7: /* meta_data: ICE_METADATA_OPEN string_list ICE_METADATA_CLOSE  */
#line 218 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1874 "src/Slice/Grammar.cpp"
    break;

  case 8: /* meta_data: %empty  */
#line 222 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 1882 "src/Slice/Grammar.cpp"
    break;

  case 9: /* definitions: definitions global_meta_data  */
#line 231 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addGlobalMetaData(metaData->v);
    }
}
#line 1894 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: definitions meta_data definition  */
#line 239 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1907 "src/Slice/Grammar.cpp"
    break;

  case 11: /* definitions: %empty  */
#line 248 "src/Slice/Grammar.y"
{
}
#line 1914 "src/Slice/Grammar.cpp"
    break;

  case 12: /* $@1: %empty  */
#line 256 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1922 "src/Slice/Grammar.cpp"
    break;

  case 14: /* $@2: %empty  */
#line 261 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1930 "src/Slice/Grammar.cpp"
    break;

  case 16: /* definition: class_decl  */
#line 266 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 1938 "src/Slice/Grammar.cpp"
    break;

  case 17: /* $@3: %empty  */
#line 270 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1946 "src/Slice/Grammar.cpp"
    break;

  case 19: /* $@4: %empty  */
#line 275 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1954 "src/Slice/Grammar.cpp"
    break;

  case 21: /* definition: interface_decl  */
#line 280 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 1962 "src/Slice/Grammar.cpp"
    break;

  case 22: /* $@5: %empty  */
#line 284 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1970 "src/Slice/Grammar.cpp"
    break;

  case 24: /* $@6: %empty  */
#line 289 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1978 "src/Slice/Grammar.cpp"
    break;

  case 26: /* definition: exception_decl  */
#line 294 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 1986 "src/Slice/Grammar.cpp"
    break;

  case 27: /* $@7: %empty  */
#line 298 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 1994 "src/Slice/Grammar.cpp"
    break;

  case 29: /* $@8: %empty  */
#line 303 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2002 "src/Slice/Grammar.cpp"
    break;

  case 31: /* definition: struct_decl  */
#line 308 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 2010 "src/Slice/Grammar.cpp"
    break;

  case 32: /* $@9: %empty  */
#line 312 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 2018 "src/Slice/Grammar.cpp"
    break;

  case 34: /* $@10: %empty  */
#line 317 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2026 "src/Slice/Grammar.cpp"
    break;

  case 36: /* definition: sequence_def  */
#line 322 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2034 "src/Slice/Grammar.cpp"
    break;

  case 37: /* $@11: %empty  */
#line 326 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2042 "src/Slice/Grammar.cpp"
    break;

  case 39: /* definition: dictionary_def  */
#line 331 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2050 "src/Slice/Grammar.cpp"
    break;

  case 40: /* $@12: %empty  */
#line 335 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2058 "src/Slice/Grammar.cpp"
    break;

  case 42: /* $@13: %empty  */
#line 340 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2066 "src/Slice/Grammar.cpp"
    break;

  case 44: /* definition: const_def  */
#line 345 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2074 "src/Slice/Grammar.cpp"
    break;

  case 45: /* definition: error ';'  */
#line 349 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2082 "src/Slice/Grammar.cpp"
    break;

  case 46: /* @14: %empty  */
#line 358 "src/Slice/Grammar.y"
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
#line 2102 "src/Slice/Grammar.cpp"
    break;

  case 47: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
#line 374 "src/Slice/Grammar.y"
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
#line 2118 "src/Slice/Grammar.cpp"
    break;

  case 48: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 391 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2126 "src/Slice/Grammar.cpp"
    break;

  case 49: /* exception_id: ICE_EXCEPTION keyword  */
#line 395 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2136 "src/Slice/Grammar.cpp"
    break;

  case 50: /* exception_decl: local_qualifier exception_id  */
#line 406 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2145 "src/Slice/Grammar.cpp"
    break;

  case 51: /* @15: %empty  */
#line 416 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-2]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-1]);
    ExceptionPtr base = ExceptionPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base, local->v);
    if(ex)
    {
        cont->checkIntroduced(ident->v, ex);
        unit->pushContainer(ex);
    }
    yyval = ex;
}
#line 2163 "src/Slice/Grammar.cpp"
    break;

  case 52: /* exception_def: local_qualifier exception_id exception_extends @15 '{' exception_exports '}'  */
#line 430 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2175 "src/Slice/Grammar.cpp"
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
#line 2187 "src/Slice/Grammar.cpp"
    break;

  case 54: /* exception_extends: %empty  */
#line 451 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2195 "src/Slice/Grammar.cpp"
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
#line 2208 "src/Slice/Grammar.cpp"
    break;

  case 56: /* exception_exports: error ';' exception_exports  */
#line 469 "src/Slice/Grammar.y"
{
}
#line 2215 "src/Slice/Grammar.cpp"
    break;

  case 57: /* exception_exports: meta_data exception_export  */
#line 472 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2223 "src/Slice/Grammar.cpp"
    break;

  case 58: /* exception_exports: %empty  */
#line 476 "src/Slice/Grammar.y"
{
}
#line 2230 "src/Slice/Grammar.cpp"
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
#line 2242 "src/Slice/Grammar.cpp"
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
#line 2264 "src/Slice/Grammar.cpp"
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
#line 2359 "src/Slice/Grammar.cpp"
    break;

  case 62: /* tag: ICE_TAG_OPEN ')'  */
#line 606 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2369 "src/Slice/Grammar.cpp"
    break;

  case 63: /* tag: ICE_TAG  */
#line 612 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2379 "src/Slice/Grammar.cpp"
    break;

  case 64: /* optional: ICE_OPTIONAL_OPEN ICE_INTEGER_LITERAL ')'  */
#line 623 "src/Slice/Grammar.y"
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
#line 2401 "src/Slice/Grammar.cpp"
    break;

  case 65: /* optional: ICE_OPTIONAL_OPEN scoped_name ')'  */
#line 641 "src/Slice/Grammar.y"
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
#line 2495 "src/Slice/Grammar.cpp"
    break;

  case 66: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 731 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2505 "src/Slice/Grammar.cpp"
    break;

  case 67: /* optional: ICE_OPTIONAL  */
#line 737 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2515 "src/Slice/Grammar.cpp"
    break;

  case 68: /* tagged_type_id: tag type_id  */
#line 748 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2528 "src/Slice/Grammar.cpp"
    break;

  case 69: /* tagged_type_id: optional type_id  */
#line 757 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2541 "src/Slice/Grammar.cpp"
    break;

  case 70: /* tagged_type_id: type_id  */
#line 766 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2553 "src/Slice/Grammar.cpp"
    break;

  case 72: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 785 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2561 "src/Slice/Grammar.cpp"
    break;

  case 73: /* struct_id: ICE_STRUCT keyword  */
#line 789 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2571 "src/Slice/Grammar.cpp"
    break;

  case 74: /* struct_decl: local_qualifier struct_id  */
#line 800 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2580 "src/Slice/Grammar.cpp"
    break;

  case 75: /* @16: %empty  */
#line 810 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
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
    yyval = st;
}
#line 2603 "src/Slice/Grammar.cpp"
    break;

  case 76: /* struct_def: local_qualifier struct_id @16 '{' struct_exports '}'  */
#line 829 "src/Slice/Grammar.y"
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
#line 2625 "src/Slice/Grammar.cpp"
    break;

  case 77: /* struct_exports: meta_data struct_export ';' struct_exports  */
#line 852 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2638 "src/Slice/Grammar.cpp"
    break;

  case 78: /* struct_exports: error ';' struct_exports  */
#line 861 "src/Slice/Grammar.y"
{
}
#line 2645 "src/Slice/Grammar.cpp"
    break;

  case 79: /* struct_exports: meta_data struct_export  */
#line 864 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2653 "src/Slice/Grammar.cpp"
    break;

  case 80: /* struct_exports: %empty  */
#line 868 "src/Slice/Grammar.y"
{
}
#line 2660 "src/Slice/Grammar.cpp"
    break;

  case 82: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 882 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2668 "src/Slice/Grammar.cpp"
    break;

  case 83: /* class_name: ICE_CLASS keyword  */
#line 886 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2678 "src/Slice/Grammar.cpp"
    break;

  case 84: /* class_id: ICE_CLASS ICE_IDENT_OPEN ICE_INTEGER_LITERAL ')'  */
#line 897 "src/Slice/Grammar.y"
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
#line 2707 "src/Slice/Grammar.cpp"
    break;

  case 85: /* class_id: ICE_CLASS ICE_IDENT_OPEN scoped_name ')'  */
#line 922 "src/Slice/Grammar.y"
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
#line 2813 "src/Slice/Grammar.cpp"
    break;

  case 86: /* class_id: class_name  */
#line 1024 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2824 "src/Slice/Grammar.cpp"
    break;

  case 87: /* class_decl: local_qualifier class_name  */
#line 1036 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, false, local->v);
    yyval = cl;
}
#line 2836 "src/Slice/Grammar.cpp"
    break;

  case 88: /* @17: %empty  */
#line 1049 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-3]);
    ClassIdTokPtr ident = ClassIdTokPtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast(yyvsp[-1]);
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast(yyvsp[0]);
    if(base)
    {
        bases->v.push_front(base);
    }
    ClassDefPtr cl = cont->createClassDef(ident->v, ident->t, false, bases->v, local->v);
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
#line 2863 "src/Slice/Grammar.cpp"
    break;

  case 89: /* class_def: local_qualifier class_id class_extends implements @17 '{' class_exports '}'  */
#line 1072 "src/Slice/Grammar.y"
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
#line 2879 "src/Slice/Grammar.cpp"
    break;

  case 90: /* class_extends: extends scoped_name  */
#line 1089 "src/Slice/Grammar.y"
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
#line 2917 "src/Slice/Grammar.cpp"
    break;

  case 91: /* class_extends: %empty  */
#line 1123 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2925 "src/Slice/Grammar.cpp"
    break;

  case 92: /* extends: ICE_EXTENDS  */
#line 1132 "src/Slice/Grammar.y"
{
}
#line 2932 "src/Slice/Grammar.cpp"
    break;

  case 93: /* extends: ':'  */
#line 1135 "src/Slice/Grammar.y"
{
}
#line 2939 "src/Slice/Grammar.cpp"
    break;

  case 94: /* implements: ICE_IMPLEMENTS interface_list  */
#line 1143 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2947 "src/Slice/Grammar.cpp"
    break;

  case 95: /* implements: %empty  */
#line 1147 "src/Slice/Grammar.y"
{
    yyval = new ClassListTok;
}
#line 2955 "src/Slice/Grammar.cpp"
    break;

  case 96: /* class_exports: meta_data class_export ';' class_exports  */
#line 1156 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2968 "src/Slice/Grammar.cpp"
    break;

  case 97: /* class_exports: error ';' class_exports  */
#line 1165 "src/Slice/Grammar.y"
{
}
#line 2975 "src/Slice/Grammar.cpp"
    break;

  case 98: /* class_exports: meta_data class_export  */
#line 1168 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2983 "src/Slice/Grammar.cpp"
    break;

  case 99: /* class_exports: %empty  */
#line 1172 "src/Slice/Grammar.y"
{
}
#line 2990 "src/Slice/Grammar.cpp"
    break;

  case 100: /* data_member: tagged_type_id  */
#line 1180 "src/Slice/Grammar.y"
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
#line 3016 "src/Slice/Grammar.cpp"
    break;

  case 101: /* data_member: tagged_type_id '=' const_initializer  */
#line 1202 "src/Slice/Grammar.y"
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
#line 3047 "src/Slice/Grammar.cpp"
    break;

  case 102: /* data_member: type keyword  */
#line 1229 "src/Slice/Grammar.y"
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
#line 3073 "src/Slice/Grammar.cpp"
    break;

  case 103: /* data_member: type  */
#line 1251 "src/Slice/Grammar.y"
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
#line 3098 "src/Slice/Grammar.cpp"
    break;

  case 104: /* struct_data_member: type_id  */
#line 1277 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 3111 "src/Slice/Grammar.cpp"
    break;

  case 105: /* struct_data_member: type_id '=' const_initializer  */
#line 1286 "src/Slice/Grammar.y"
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
#line 3126 "src/Slice/Grammar.cpp"
    break;

  case 106: /* struct_data_member: tag type_id  */
#line 1297 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3139 "src/Slice/Grammar.cpp"
    break;

  case 107: /* struct_data_member: tag type_id '=' const_initializer  */
#line 1306 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3152 "src/Slice/Grammar.cpp"
    break;

  case 108: /* struct_data_member: optional type_id  */
#line 1315 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3165 "src/Slice/Grammar.cpp"
    break;

  case 109: /* struct_data_member: optional type_id '=' const_initializer  */
#line 1324 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3178 "src/Slice/Grammar.cpp"
    break;

  case 110: /* struct_data_member: type keyword  */
#line 1333 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3192 "src/Slice/Grammar.cpp"
    break;

  case 111: /* struct_data_member: type  */
#line 1343 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("missing data member name");
}
#line 3205 "src/Slice/Grammar.cpp"
    break;

  case 112: /* return_type: tag type  */
#line 1357 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3215 "src/Slice/Grammar.cpp"
    break;

  case 113: /* return_type: optional type  */
#line 1363 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3225 "src/Slice/Grammar.cpp"
    break;

  case 114: /* return_type: type  */
#line 1369 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3235 "src/Slice/Grammar.cpp"
    break;

  case 115: /* return_type: ICE_VOID  */
#line 1375 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    yyval = m;
}
#line 3244 "src/Slice/Grammar.cpp"
    break;

  case 116: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1385 "src/Slice/Grammar.y"
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
#line 3272 "src/Slice/Grammar.cpp"
    break;

  case 117: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1409 "src/Slice/Grammar.y"
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
#line 3301 "src/Slice/Grammar.cpp"
    break;

  case 118: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1434 "src/Slice/Grammar.y"
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
#line 3329 "src/Slice/Grammar.cpp"
    break;

  case 119: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1458 "src/Slice/Grammar.y"
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
#line 3358 "src/Slice/Grammar.cpp"
    break;

  case 120: /* @18: %empty  */
#line 1488 "src/Slice/Grammar.y"
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
#line 3374 "src/Slice/Grammar.cpp"
    break;

  case 121: /* operation: operation_preamble parameters ')' @18 throws  */
#line 1500 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3388 "src/Slice/Grammar.cpp"
    break;

  case 122: /* @19: %empty  */
#line 1510 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3400 "src/Slice/Grammar.cpp"
    break;

  case 123: /* operation: operation_preamble error ')' @19 throws  */
#line 1518 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3414 "src/Slice/Grammar.cpp"
    break;

  case 126: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1540 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3422 "src/Slice/Grammar.cpp"
    break;

  case 127: /* interface_id: ICE_INTERFACE keyword  */
#line 1544 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3432 "src/Slice/Grammar.cpp"
    break;

  case 128: /* interface_decl: local_qualifier interface_id  */
#line 1555 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v, true, local->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3445 "src/Slice/Grammar.cpp"
    break;

  case 129: /* @20: %empty  */
#line 1569 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-2]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-1]);
    ContainerPtr cont = unit->currentContainer();
    ClassListTokPtr bases = ClassListTokPtr::dynamicCast(yyvsp[0]);
    ClassDefPtr cl = cont->createClassDef(ident->v, -1, true, bases->v, local->v);
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
#line 3467 "src/Slice/Grammar.cpp"
    break;

  case 130: /* interface_def: local_qualifier interface_id interface_extends @20 '{' interface_exports '}'  */
#line 1587 "src/Slice/Grammar.y"
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
#line 3483 "src/Slice/Grammar.cpp"
    break;

  case 131: /* interface_list: scoped_name ',' interface_list  */
#line 1604 "src/Slice/Grammar.y"
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
#line 3522 "src/Slice/Grammar.cpp"
    break;

  case 132: /* interface_list: scoped_name  */
#line 1639 "src/Slice/Grammar.y"
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
#line 3561 "src/Slice/Grammar.cpp"
    break;

  case 133: /* interface_list: ICE_OBJECT  */
#line 1674 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new ClassListTok; // Dummy
}
#line 3570 "src/Slice/Grammar.cpp"
    break;

  case 134: /* interface_list: ICE_VALUE  */
#line 1679 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3579 "src/Slice/Grammar.cpp"
    break;

  case 135: /* interface_extends: extends interface_list  */
#line 1689 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3587 "src/Slice/Grammar.cpp"
    break;

  case 136: /* interface_extends: %empty  */
#line 1693 "src/Slice/Grammar.y"
{
    yyval = new ClassListTok;
}
#line 3595 "src/Slice/Grammar.cpp"
    break;

  case 137: /* interface_exports: meta_data interface_export ';' interface_exports  */
#line 1702 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3608 "src/Slice/Grammar.cpp"
    break;

  case 138: /* interface_exports: error ';' interface_exports  */
#line 1711 "src/Slice/Grammar.y"
{
}
#line 3615 "src/Slice/Grammar.cpp"
    break;

  case 139: /* interface_exports: meta_data interface_export  */
#line 1714 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3623 "src/Slice/Grammar.cpp"
    break;

  case 140: /* interface_exports: %empty  */
#line 1718 "src/Slice/Grammar.y"
{
}
#line 3630 "src/Slice/Grammar.cpp"
    break;

  case 142: /* exception_list: exception ',' exception_list  */
#line 1732 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3641 "src/Slice/Grammar.cpp"
    break;

  case 143: /* exception_list: exception  */
#line 1739 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3652 "src/Slice/Grammar.cpp"
    break;

  case 144: /* exception: scoped_name  */
#line 1751 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if(!exception)
    {
        exception = cont->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    yyval = exception;
}
#line 3668 "src/Slice/Grammar.cpp"
    break;

  case 145: /* exception: keyword  */
#line 1763 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, false, Dummy); // Dummy
}
#line 3678 "src/Slice/Grammar.cpp"
    break;

  case 146: /* sequence_def: local_qualifier ICE_SEQUENCE '<' meta_data type '>' ICE_IDENTIFIER  */
#line 1774 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-6]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v, local->v);
}
#line 3691 "src/Slice/Grammar.cpp"
    break;

  case 147: /* sequence_def: local_qualifier ICE_SEQUENCE '<' meta_data type '>' keyword  */
#line 1783 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-6]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v, local->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3705 "src/Slice/Grammar.cpp"
    break;

  case 148: /* dictionary_def: local_qualifier ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' ICE_IDENTIFIER  */
#line 1798 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-9]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v);
}
#line 3720 "src/Slice/Grammar.cpp"
    break;

  case 149: /* dictionary_def: local_qualifier ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' keyword  */
#line 1809 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-9]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v, local->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3736 "src/Slice/Grammar.cpp"
    break;

  case 150: /* enum_id: ICE_ENUM ICE_IDENTIFIER  */
#line 1826 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3744 "src/Slice/Grammar.cpp"
    break;

  case 151: /* enum_id: ICE_ENUM keyword  */
#line 1830 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3754 "src/Slice/Grammar.cpp"
    break;

  case 152: /* @21: %empty  */
#line 1841 "src/Slice/Grammar.y"
{
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, local->v);
    if(en)
    {
        cont->checkIntroduced(ident->v, en);
    }
    else
    {
        en = cont->createEnum(IceUtil::generateUUID(), local->v, Dummy);
    }
    unit->pushContainer(en);
    yyval = en;
}
#line 3775 "src/Slice/Grammar.cpp"
    break;

  case 153: /* enum_def: local_qualifier enum_id @21 '{' enumerator_list '}'  */
#line 1858 "src/Slice/Grammar.y"
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
#line 3793 "src/Slice/Grammar.cpp"
    break;

  case 154: /* @22: %empty  */
#line 1873 "src/Slice/Grammar.y"
{
    unit->error("missing enumeration name");
    BoolTokPtr local = BoolTokPtr::dynamicCast(yyvsp[-1]);
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), local->v, Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3806 "src/Slice/Grammar.cpp"
    break;

  case 155: /* enum_def: local_qualifier ICE_ENUM @22 '{' enumerator_list '}'  */
#line 1882 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3815 "src/Slice/Grammar.cpp"
    break;

  case 156: /* enumerator_list: enumerator ',' enumerator_list  */
#line 1892 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 3825 "src/Slice/Grammar.cpp"
    break;

  case 157: /* enumerator_list: enumerator  */
#line 1898 "src/Slice/Grammar.y"
{
}
#line 3832 "src/Slice/Grammar.cpp"
    break;

  case 158: /* enumerator: ICE_IDENTIFIER  */
#line 1906 "src/Slice/Grammar.y"
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
#line 3848 "src/Slice/Grammar.cpp"
    break;

  case 159: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1918 "src/Slice/Grammar.y"
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
#line 3872 "src/Slice/Grammar.cpp"
    break;

  case 160: /* enumerator: keyword  */
#line 1938 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 3883 "src/Slice/Grammar.cpp"
    break;

  case 161: /* enumerator: %empty  */
#line 1945 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 3892 "src/Slice/Grammar.cpp"
    break;

  case 162: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1955 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3900 "src/Slice/Grammar.cpp"
    break;

  case 163: /* enumerator_initializer: scoped_name  */
#line 1959 "src/Slice/Grammar.y"
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
#line 3938 "src/Slice/Grammar.cpp"
    break;

  case 164: /* out_qualifier: ICE_OUT  */
#line 1998 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = true;
    yyval = out;
}
#line 3948 "src/Slice/Grammar.cpp"
    break;

  case 165: /* out_qualifier: %empty  */
#line 2004 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = false;
    yyval = out;
}
#line 3958 "src/Slice/Grammar.cpp"
    break;

  case 166: /* parameters: %empty  */
#line 2015 "src/Slice/Grammar.y"
{
}
#line 3965 "src/Slice/Grammar.cpp"
    break;

  case 167: /* parameters: out_qualifier meta_data tagged_type_id  */
#line 2018 "src/Slice/Grammar.y"
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
#line 3985 "src/Slice/Grammar.cpp"
    break;

  case 168: /* parameters: parameters ',' out_qualifier meta_data tagged_type_id  */
#line 2034 "src/Slice/Grammar.y"
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
#line 4005 "src/Slice/Grammar.cpp"
    break;

  case 169: /* parameters: out_qualifier meta_data type keyword  */
#line 2050 "src/Slice/Grammar.y"
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
#line 4021 "src/Slice/Grammar.cpp"
    break;

  case 170: /* parameters: parameters ',' out_qualifier meta_data type keyword  */
#line 2062 "src/Slice/Grammar.y"
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
#line 4037 "src/Slice/Grammar.cpp"
    break;

  case 171: /* parameters: out_qualifier meta_data type  */
#line 2074 "src/Slice/Grammar.y"
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
#line 4052 "src/Slice/Grammar.cpp"
    break;

  case 172: /* parameters: parameters ',' out_qualifier meta_data type  */
#line 2085 "src/Slice/Grammar.y"
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
#line 4067 "src/Slice/Grammar.cpp"
    break;

  case 173: /* throws: ICE_THROWS exception_list  */
#line 2101 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4075 "src/Slice/Grammar.cpp"
    break;

  case 174: /* throws: %empty  */
#line 2105 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 4083 "src/Slice/Grammar.cpp"
    break;

  case 175: /* scoped_name: ICE_IDENTIFIER  */
#line 2114 "src/Slice/Grammar.y"
{
}
#line 4090 "src/Slice/Grammar.cpp"
    break;

  case 176: /* scoped_name: ICE_SCOPED_IDENTIFIER  */
#line 2117 "src/Slice/Grammar.y"
{
}
#line 4097 "src/Slice/Grammar.cpp"
    break;

  case 177: /* type: ICE_BYTE  */
#line 2125 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindByte);
}
#line 4105 "src/Slice/Grammar.cpp"
    break;

  case 178: /* type: ICE_BOOL  */
#line 2129 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindBool);
}
#line 4113 "src/Slice/Grammar.cpp"
    break;

  case 179: /* type: ICE_SHORT  */
#line 2133 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindShort);
}
#line 4121 "src/Slice/Grammar.cpp"
    break;

  case 180: /* type: ICE_INT  */
#line 2137 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindInt);
}
#line 4129 "src/Slice/Grammar.cpp"
    break;

  case 181: /* type: ICE_LONG  */
#line 2141 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindLong);
}
#line 4137 "src/Slice/Grammar.cpp"
    break;

  case 182: /* type: ICE_FLOAT  */
#line 2145 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindFloat);
}
#line 4145 "src/Slice/Grammar.cpp"
    break;

  case 183: /* type: ICE_DOUBLE  */
#line 2149 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindDouble);
}
#line 4153 "src/Slice/Grammar.cpp"
    break;

  case 184: /* type: ICE_STRING  */
#line 2153 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindString);
}
#line 4161 "src/Slice/Grammar.cpp"
    break;

  case 185: /* type: ICE_OBJECT  */
#line 2157 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindObject);
}
#line 4169 "src/Slice/Grammar.cpp"
    break;

  case 186: /* type: ICE_OBJECT '*'  */
#line 2161 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindObjectProxy);
}
#line 4177 "src/Slice/Grammar.cpp"
    break;

  case 187: /* type: ICE_LOCAL_OBJECT  */
#line 2165 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindLocalObject);
}
#line 4185 "src/Slice/Grammar.cpp"
    break;

  case 188: /* type: ICE_VALUE  */
#line 2169 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindValue);
}
#line 4193 "src/Slice/Grammar.cpp"
    break;

  case 189: /* type: scoped_name  */
#line 2173 "src/Slice/Grammar.y"
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
#line 4216 "src/Slice/Grammar.cpp"
    break;

  case 190: /* type: scoped_name '*'  */
#line 2192 "src/Slice/Grammar.y"
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
            if(cl->isLocal())
            {
                unit->error("cannot create proxy for " + cl->kindOf() + " `" + cl->name() + "'"); // $$ is dummy
            }
            *p = new Proxy(cl);
        }
        yyval = types.front();
    }
    else
    {
        yyval = 0;
    }
}
#line 4256 "src/Slice/Grammar.cpp"
    break;

  case 191: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 2233 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4266 "src/Slice/Grammar.cpp"
    break;

  case 192: /* string_literal: ICE_STRING_LITERAL  */
#line 2239 "src/Slice/Grammar.y"
{
}
#line 4273 "src/Slice/Grammar.cpp"
    break;

  case 193: /* string_list: string_list ',' string_literal  */
#line 2247 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4284 "src/Slice/Grammar.cpp"
    break;

  case 194: /* string_list: string_literal  */
#line 2254 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4295 "src/Slice/Grammar.cpp"
    break;

  case 195: /* local_qualifier: ICE_LOCAL  */
#line 2266 "src/Slice/Grammar.y"
{
    BoolTokPtr local = new BoolTok;
    local->v = true;
    yyval = local;
}
#line 4305 "src/Slice/Grammar.cpp"
    break;

  case 196: /* local_qualifier: %empty  */
#line 2272 "src/Slice/Grammar.y"
{
    BoolTokPtr local = new BoolTok;
    local->v = false;
    yyval = local;
}
#line 4315 "src/Slice/Grammar.cpp"
    break;

  case 197: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2283 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4328 "src/Slice/Grammar.cpp"
    break;

  case 198: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2292 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4341 "src/Slice/Grammar.cpp"
    break;

  case 199: /* const_initializer: scoped_name  */
#line 2301 "src/Slice/Grammar.y"
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
#line 4385 "src/Slice/Grammar.cpp"
    break;

  case 200: /* const_initializer: ICE_STRING_LITERAL  */
#line 2341 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4396 "src/Slice/Grammar.cpp"
    break;

  case 201: /* const_initializer: ICE_FALSE  */
#line 2348 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4407 "src/Slice/Grammar.cpp"
    break;

  case 202: /* const_initializer: ICE_TRUE  */
#line 2355 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4418 "src/Slice/Grammar.cpp"
    break;

  case 203: /* const_def: ICE_CONST meta_data type ICE_IDENTIFIER '=' const_initializer  */
#line 2367 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4431 "src/Slice/Grammar.cpp"
    break;

  case 204: /* const_def: ICE_CONST meta_data type '=' const_initializer  */
#line 2376 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4444 "src/Slice/Grammar.cpp"
    break;

  case 205: /* keyword: ICE_MODULE  */
#line 2390 "src/Slice/Grammar.y"
{
}
#line 4451 "src/Slice/Grammar.cpp"
    break;

  case 206: /* keyword: ICE_CLASS  */
#line 2393 "src/Slice/Grammar.y"
{
}
#line 4458 "src/Slice/Grammar.cpp"
    break;

  case 207: /* keyword: ICE_INTERFACE  */
#line 2396 "src/Slice/Grammar.y"
{
}
#line 4465 "src/Slice/Grammar.cpp"
    break;

  case 208: /* keyword: ICE_EXCEPTION  */
#line 2399 "src/Slice/Grammar.y"
{
}
#line 4472 "src/Slice/Grammar.cpp"
    break;

  case 209: /* keyword: ICE_STRUCT  */
#line 2402 "src/Slice/Grammar.y"
{
}
#line 4479 "src/Slice/Grammar.cpp"
    break;

  case 210: /* keyword: ICE_SEQUENCE  */
#line 2405 "src/Slice/Grammar.y"
{
}
#line 4486 "src/Slice/Grammar.cpp"
    break;

  case 211: /* keyword: ICE_DICTIONARY  */
#line 2408 "src/Slice/Grammar.y"
{
}
#line 4493 "src/Slice/Grammar.cpp"
    break;

  case 212: /* keyword: ICE_ENUM  */
#line 2411 "src/Slice/Grammar.y"
{
}
#line 4500 "src/Slice/Grammar.cpp"
    break;

  case 213: /* keyword: ICE_OUT  */
#line 2414 "src/Slice/Grammar.y"
{
}
#line 4507 "src/Slice/Grammar.cpp"
    break;

  case 214: /* keyword: ICE_EXTENDS  */
#line 2417 "src/Slice/Grammar.y"
{
}
#line 4514 "src/Slice/Grammar.cpp"
    break;

  case 215: /* keyword: ICE_IMPLEMENTS  */
#line 2420 "src/Slice/Grammar.y"
{
}
#line 4521 "src/Slice/Grammar.cpp"
    break;

  case 216: /* keyword: ICE_THROWS  */
#line 2423 "src/Slice/Grammar.y"
{
}
#line 4528 "src/Slice/Grammar.cpp"
    break;

  case 217: /* keyword: ICE_VOID  */
#line 2426 "src/Slice/Grammar.y"
{
}
#line 4535 "src/Slice/Grammar.cpp"
    break;

  case 218: /* keyword: ICE_BYTE  */
#line 2429 "src/Slice/Grammar.y"
{
}
#line 4542 "src/Slice/Grammar.cpp"
    break;

  case 219: /* keyword: ICE_BOOL  */
#line 2432 "src/Slice/Grammar.y"
{
}
#line 4549 "src/Slice/Grammar.cpp"
    break;

  case 220: /* keyword: ICE_SHORT  */
#line 2435 "src/Slice/Grammar.y"
{
}
#line 4556 "src/Slice/Grammar.cpp"
    break;

  case 221: /* keyword: ICE_INT  */
#line 2438 "src/Slice/Grammar.y"
{
}
#line 4563 "src/Slice/Grammar.cpp"
    break;

  case 222: /* keyword: ICE_LONG  */
#line 2441 "src/Slice/Grammar.y"
{
}
#line 4570 "src/Slice/Grammar.cpp"
    break;

  case 223: /* keyword: ICE_FLOAT  */
#line 2444 "src/Slice/Grammar.y"
{
}
#line 4577 "src/Slice/Grammar.cpp"
    break;

  case 224: /* keyword: ICE_DOUBLE  */
#line 2447 "src/Slice/Grammar.y"
{
}
#line 4584 "src/Slice/Grammar.cpp"
    break;

  case 225: /* keyword: ICE_STRING  */
#line 2450 "src/Slice/Grammar.y"
{
}
#line 4591 "src/Slice/Grammar.cpp"
    break;

  case 226: /* keyword: ICE_OBJECT  */
#line 2453 "src/Slice/Grammar.y"
{
}
#line 4598 "src/Slice/Grammar.cpp"
    break;

  case 227: /* keyword: ICE_LOCAL_OBJECT  */
#line 2456 "src/Slice/Grammar.y"
{
}
#line 4605 "src/Slice/Grammar.cpp"
    break;

  case 228: /* keyword: ICE_LOCAL  */
#line 2459 "src/Slice/Grammar.y"
{
}
#line 4612 "src/Slice/Grammar.cpp"
    break;

  case 229: /* keyword: ICE_CONST  */
#line 2462 "src/Slice/Grammar.y"
{
}
#line 4619 "src/Slice/Grammar.cpp"
    break;

  case 230: /* keyword: ICE_FALSE  */
#line 2465 "src/Slice/Grammar.y"
{
}
#line 4626 "src/Slice/Grammar.cpp"
    break;

  case 231: /* keyword: ICE_TRUE  */
#line 2468 "src/Slice/Grammar.y"
{
}
#line 4633 "src/Slice/Grammar.cpp"
    break;

  case 232: /* keyword: ICE_IDEMPOTENT  */
#line 2471 "src/Slice/Grammar.y"
{
}
#line 4640 "src/Slice/Grammar.cpp"
    break;

  case 233: /* keyword: ICE_TAG  */
#line 2474 "src/Slice/Grammar.y"
{
}
#line 4647 "src/Slice/Grammar.cpp"
    break;

  case 234: /* keyword: ICE_OPTIONAL  */
#line 2477 "src/Slice/Grammar.y"
{
}
#line 4654 "src/Slice/Grammar.cpp"
    break;

  case 235: /* keyword: ICE_VALUE  */
#line 2480 "src/Slice/Grammar.y"
{
}
#line 4661 "src/Slice/Grammar.cpp"
    break;


#line 4665 "src/Slice/Grammar.cpp"

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

#line 2484 "src/Slice/Grammar.y"

