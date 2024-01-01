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
  YYSYMBOL_exception_exports = 83,         /* exception_exports  */
  YYSYMBOL_type_id = 84,                   /* type_id  */
  YYSYMBOL_tag = 85,                       /* tag  */
  YYSYMBOL_optional = 86,                  /* optional  */
  YYSYMBOL_tagged_type_id = 87,            /* tagged_type_id  */
  YYSYMBOL_exception_export = 88,          /* exception_export  */
  YYSYMBOL_struct_id = 89,                 /* struct_id  */
  YYSYMBOL_struct_decl = 90,               /* struct_decl  */
  YYSYMBOL_struct_def = 91,                /* struct_def  */
  YYSYMBOL_92_16 = 92,                     /* @16  */
  YYSYMBOL_struct_exports = 93,            /* struct_exports  */
  YYSYMBOL_struct_export = 94,             /* struct_export  */
  YYSYMBOL_class_name = 95,                /* class_name  */
  YYSYMBOL_class_id = 96,                  /* class_id  */
  YYSYMBOL_class_decl = 97,                /* class_decl  */
  YYSYMBOL_class_def = 98,                 /* class_def  */
  YYSYMBOL_99_17 = 99,                     /* @17  */
  YYSYMBOL_class_extends = 100,            /* class_extends  */
  YYSYMBOL_extends = 101,                  /* extends  */
  YYSYMBOL_class_exports = 102,            /* class_exports  */
  YYSYMBOL_data_member = 103,              /* data_member  */
  YYSYMBOL_struct_data_member = 104,       /* struct_data_member  */
  YYSYMBOL_return_type = 105,              /* return_type  */
  YYSYMBOL_operation_preamble = 106,       /* operation_preamble  */
  YYSYMBOL_operation = 107,                /* operation  */
  YYSYMBOL_108_18 = 108,                   /* @18  */
  YYSYMBOL_109_19 = 109,                   /* @19  */
  YYSYMBOL_class_export = 110,             /* class_export  */
  YYSYMBOL_interface_id = 111,             /* interface_id  */
  YYSYMBOL_interface_decl = 112,           /* interface_decl  */
  YYSYMBOL_interface_def = 113,            /* interface_def  */
  YYSYMBOL_114_20 = 114,                   /* @20  */
  YYSYMBOL_interface_list = 115,           /* interface_list  */
  YYSYMBOL_interface_extends = 116,        /* interface_extends  */
  YYSYMBOL_interface_exports = 117,        /* interface_exports  */
  YYSYMBOL_interface_export = 118,         /* interface_export  */
  YYSYMBOL_exception_list = 119,           /* exception_list  */
  YYSYMBOL_exception = 120,                /* exception  */
  YYSYMBOL_sequence_def = 121,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 122,           /* dictionary_def  */
  YYSYMBOL_enum_id = 123,                  /* enum_id  */
  YYSYMBOL_enum_def = 124,                 /* enum_def  */
  YYSYMBOL_125_21 = 125,                   /* @21  */
  YYSYMBOL_126_22 = 126,                   /* @22  */
  YYSYMBOL_enumerator_list = 127,          /* enumerator_list  */
  YYSYMBOL_enumerator = 128,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 129,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 130,            /* out_qualifier  */
  YYSYMBOL_parameters = 131,               /* parameters  */
  YYSYMBOL_throws = 132,                   /* throws  */
  YYSYMBOL_scoped_name = 133,              /* scoped_name  */
  YYSYMBOL_builtin = 134,                  /* builtin  */
  YYSYMBOL_type = 135,                     /* type  */
  YYSYMBOL_string_literal = 136,           /* string_literal  */
  YYSYMBOL_string_list = 137,              /* string_list  */
  YYSYMBOL_const_initializer = 138,        /* const_initializer  */
  YYSYMBOL_const_def = 139,                /* const_def  */
  YYSYMBOL_keyword = 140                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 58 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 356 "src/Slice/Grammar.cpp"

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
#define YYLAST   848

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  56
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  85
/* YYNRULES -- Number of rules.  */
#define YYNRULES  227
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  331

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
       0,   184,   184,   192,   195,   203,   207,   217,   221,   230,
     238,   247,   256,   255,   261,   260,   265,   270,   269,   275,
     274,   279,   284,   283,   289,   288,   293,   298,   297,   303,
     302,   307,   312,   311,   317,   316,   321,   326,   325,   330,
     335,   334,   340,   339,   344,   348,   358,   357,   390,   394,
     405,   416,   415,   441,   449,   458,   467,   470,   474,   482,
     496,   514,   593,   599,   610,   628,   706,   712,   723,   732,
     741,   754,   760,   764,   775,   786,   785,   826,   835,   838,
     842,   850,   856,   860,   871,   896,   986,   998,  1011,  1010,
    1044,  1078,  1087,  1090,  1098,  1107,  1110,  1114,  1122,  1144,
    1171,  1193,  1219,  1228,  1239,  1248,  1257,  1266,  1275,  1285,
    1299,  1305,  1311,  1317,  1327,  1351,  1376,  1400,  1431,  1430,
    1453,  1452,  1475,  1481,  1485,  1496,  1510,  1509,  1543,  1578,
    1613,  1618,  1628,  1632,  1641,  1650,  1653,  1657,  1665,  1671,
    1678,  1690,  1702,  1713,  1721,  1735,  1745,  1761,  1765,  1777,
    1776,  1808,  1807,  1825,  1831,  1839,  1851,  1871,  1878,  1888,
    1892,  1930,  1936,  1947,  1950,  1966,  1982,  1994,  2006,  2017,
    2033,  2037,  2046,  2049,  2057,  2058,  2059,  2060,  2061,  2062,
    2063,  2064,  2065,  2066,  2071,  2075,  2080,  2111,  2147,  2153,
    2161,  2168,  2180,  2189,  2198,  2238,  2245,  2252,  2264,  2273,
    2287,  2288,  2289,  2290,  2291,  2292,  2293,  2294,  2295,  2296,
    2297,  2298,  2299,  2300,  2301,  2302,  2303,  2304,  2305,  2306,
    2307,  2308,  2309,  2310,  2311,  2312,  2313,  2314
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
  "exception_def", "@15", "exception_extends", "exception_exports",
  "type_id", "tag", "optional", "tagged_type_id", "exception_export",
  "struct_id", "struct_decl", "struct_def", "@16", "struct_exports",
  "struct_export", "class_name", "class_id", "class_decl", "class_def",
  "@17", "class_extends", "extends", "class_exports", "data_member",
  "struct_data_member", "return_type", "operation_preamble", "operation",
  "@18", "@19", "class_export", "interface_id", "interface_decl",
  "interface_def", "@20", "interface_list", "interface_extends",
  "interface_exports", "interface_export", "exception_list", "exception",
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

#define YYPACT_NINF (-287)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-164)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -287,    22,    18,  -287,    12,    12,    12,  -287,    74,    12,
    -287,   -11,     6,    33,     1,    -5,   457,   529,   561,   593,
       7,    23,   625,    36,  -287,  -287,    19,    44,  -287,    48,
      54,  -287,    20,     2,    89,  -287,    24,    94,  -287,   109,
     111,  -287,  -287,   118,  -287,  -287,    12,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
      77,  -287,  -287,  -287,  -287,  -287,  -287,  -287,    36,    36,
    -287,    68,  -287,   102,   121,  -287,  -287,  -287,    69,   122,
     121,    86,   123,   121,  -287,    69,   124,   121,   104,  -287,
     128,   121,   130,   132,   103,   121,   134,  -287,   107,   133,
    -287,  -287,   135,   102,   102,   657,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,   136,  -287,   137,  -287,   -17,  -287,
    -287,   126,  -287,  -287,  -287,   245,  -287,  -287,   142,  -287,
    -287,  -287,  -287,  -287,  -287,   125,   146,  -287,  -287,  -287,
    -287,   657,  -287,  -287,  -287,  -287,  -287,   140,   138,   144,
     148,   147,  -287,  -287,  -287,   149,   127,   375,   155,   804,
     154,   411,   104,   339,   156,   108,   689,    36,   131,  -287,
     657,   127,  -287,  -287,  -287,  -287,  -287,  -287,  -287,   157,
     804,   160,   245,  -287,  -287,    -7,    59,   158,   102,   102,
     159,  -287,   721,  -287,   174,   804,   173,  -287,   176,   774,
     179,  -287,  -287,  -287,  -287,   102,  -287,  -287,  -287,  -287,
    -287,   375,  -287,   102,   102,   177,   187,  -287,   721,  -287,
    -287,   185,  -287,   186,   188,  -287,   189,   127,   190,   202,
     192,   245,  -287,  -287,   411,  -287,   198,  -287,   339,  -287,
     196,   102,   102,    87,   300,  -287,   199,  -287,  -287,   194,
    -287,  -287,  -287,   127,   375,  -287,  -287,  -287,  -287,  -287,
    -287,   127,   127,  -287,  -287,   411,  -287,   101,  -287,  -287,
    -287,  -287,   200,  -287,    36,   -12,   339,   753,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,   804,  -287,   239,  -287,
    -287,  -287,   238,  -287,   721,   238,    36,   496,  -287,  -287,
    -287,   804,  -287,   203,  -287,  -287,  -287,   721,   496,  -287,
    -287
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      11,     0,     8,     1,     0,     0,     0,     9,     0,   189,
     191,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   151,     8,    10,    12,    50,    26,    27,    74,
      31,    32,    87,    91,    16,    17,   125,    21,    22,    36,
      39,   149,    40,    44,   188,     7,     0,     5,     6,    45,
      46,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,   210,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   220,   221,   222,   223,   224,   225,   226,   227,    82,
       0,    83,   123,   124,    48,    49,    72,    73,     8,     8,
     147,     0,   148,     0,     4,    92,    93,    51,     0,     0,
       4,     0,     0,     4,    88,     0,     0,     4,     0,   126,
       0,     4,     0,     0,     0,     4,     0,   190,     0,     0,
     172,   173,     0,     0,     0,   158,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   186,   185,     0,     3,
      13,     0,    53,    25,    28,     0,    30,    33,     0,    90,
      15,    18,   130,   131,   132,   129,     0,    20,    23,    35,
      38,   158,    41,    43,    11,    84,    85,     0,     0,   155,
       0,   154,   157,   184,   187,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     8,     0,     8,     0,   152,
     158,     0,   196,   197,   195,   192,   193,   194,   199,     0,
       0,     0,     0,    63,    67,     0,     0,   102,     0,     0,
      79,    81,   109,    76,     0,     0,     0,   128,     0,     0,
       0,   150,    47,   143,   144,     0,   159,   156,   160,   153,
     198,     0,    70,     0,     0,    98,    57,    71,   101,    52,
      78,     0,    62,     0,     0,    66,     0,     0,   104,     0,
     106,     0,    59,   108,     0,   122,    96,    89,     0,   113,
       0,     0,     0,     0,     0,   138,   136,   112,   127,     0,
      56,    68,    69,     0,     0,   100,    60,    61,    64,    65,
     103,     0,     0,    77,    95,     0,   135,     0,   110,   111,
     114,   116,     0,   161,     8,     0,     0,     0,    99,    55,
     105,   107,    94,   115,   117,   120,     0,   118,   162,   134,
     145,   146,   171,   164,   168,   171,     8,     0,   121,   166,
     119,     0,   170,   140,   141,   142,   165,   169,     0,   167,
     139
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -287,  -287,    -1,  -287,    -2,    90,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -219,  -170,  -171,
    -168,  -286,  -287,  -287,  -287,  -287,  -287,  -192,  -287,  -287,
    -287,  -287,  -287,  -287,  -287,    32,  -241,    42,  -287,     9,
    -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,  -287,    76,
    -287,  -243,  -287,   -69,  -287,  -287,  -287,  -287,  -287,  -287,
    -287,  -145,  -287,  -287,   -38,  -287,   -44,   -75,  -287,   -74,
      15,   143,  -185,  -287,   -15
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,   140,     7,   179,     2,    24,    94,   106,   107,
     110,   111,    99,   100,   102,   103,   112,   113,   115,   116,
      25,   118,    26,    27,    28,   141,    97,   201,   232,   233,
     234,   235,   236,    29,    30,    31,   101,   180,   210,    32,
      33,    34,    35,   148,   104,    98,   216,   237,   211,   263,
     264,   265,   315,   312,   256,    36,    37,    38,   156,   154,
     109,   220,   266,   322,   323,    39,    40,    41,    42,   114,
      91,   170,   171,   227,   294,   295,   318,   136,   137,   249,
      10,    11,   198,    43,   172
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       8,    81,    83,    85,    87,   122,   230,    92,   208,   207,
     240,   209,   270,   284,    95,   286,   184,   175,    -2,   138,
     313,    93,     3,   142,    44,   241,    45,   120,   121,    50,
     149,    95,   -86,   155,   176,   326,    95,   307,   248,   250,
     308,    46,   242,     9,   302,   229,    47,    49,   261,   167,
     168,   262,    96,   309,     4,   299,     5,     6,    46,   283,
      88,   117,   280,   271,   272,   105,   -54,   -86,   108,    96,
     -86,  -133,     4,    48,    96,    14,    89,    15,    16,    17,
      18,    19,    20,    21,    22,    46,   123,   124,   298,   261,
     -24,   244,   262,   120,   121,   -75,   300,   301,    23,   144,
     -29,   197,   147,   120,   121,   212,   151,   155,   245,   119,
     158,   120,   121,   228,   162,   125,   197,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   238,   152,   290,   291,
     243,   246,   135,   145,   153,   -14,   120,   121,   120,   121,
     -19,   238,   303,   304,     4,   267,     5,     6,    12,    13,
     161,   269,   192,   193,   164,   -34,   222,   -37,   194,   195,
     196,   120,   121,   226,   -42,   120,   121,   139,   143,   146,
     150,   224,   197,   177,   157,   200,   159,   182,   160,   215,
     163,   219,   165,     8,   166,   225,   267,   288,   289,   181,
     187,   173,   174,   183,   186,   188,   189,   253,   197,   190,
     191,   202,   213,   231,   221,   251,   197,   197,   239,   247,
     259,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     254,   257,   258,   275,   203,   204,   135,   268,   273,   200,
     120,   121,   314,   274,   276,   277,   252,   278,   279,   205,
     206,   281,   324,   282,   285,   296,   178,   327,   297,   305,
     293,   317,   215,   324,   185,   328,   219,   255,   217,   330,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,   287,
     316,   320,   200,    -8,    -8,    -8,     0,     0,     0,    -8,
      -8,     4,   311,   215,     0,     0,     0,     0,    -8,    -8,
       0,     0,   306,   -80,   219,     0,     0,     0,     0,   319,
       0,   292,   325,     0,     0,     0,     0,     0,     0,     0,
       0,   293,   329,   325,   321,  -162,  -162,  -162,  -162,  -162,
    -162,  -162,  -162,  -162,     0,     0,     0,     0,  -162,  -162,
    -162,     0,     0,     0,  -162,  -162,  -162,     0,     0,     0,
     218,     0,     0,  -162,  -162,     0,     0,     0,     0,  -163,
       0,     0,  -163,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,     0,     0,     0,    -8,    -8,    -8,    -8,
       0,     0,     0,    -8,    -8,     4,   199,     0,     0,     0,
       0,     0,    -8,    -8,     0,     0,     0,  -137,     0,     0,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,     0,
       0,     0,     0,    -8,    -8,    -8,     0,     0,     0,    -8,
      -8,     4,   214,     0,     0,     0,     0,     0,    -8,    -8,
       0,     0,     0,   -58,     0,     0,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,     0,     0,     0,     0,    -8,
      -8,    -8,     0,     0,     0,    -8,    -8,     4,     0,     0,
       0,     0,     0,     0,    -8,    -8,     0,     0,     0,   -97,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
       0,    79,     0,     0,     0,     0,     0,     0,    80,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,     0,     0,     0,
     120,   121,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,     0,    82,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,     0,     0,     0,    84,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,     0,    86,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,     0,     0,     0,    90,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,     0,
       0,   169,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
       0,     0,     0,   223,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,     0,     0,     0,   252,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,     0,     0,     0,   310,   259,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,     0,
       0,   260,   203,   204,   135,     0,     0,     0,   120,   121,
       0,     0,     0,     0,     0,     0,     0,   205,   206,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,     0,
       0,     0,   203,   204,   135,     0,     0,     0,   120,   121,
       0,     0,     0,     0,     0,     0,     0,   205,   206
};

static const yytype_int16 yycheck[] =
{
       2,    16,    17,    18,    19,    80,   191,    22,   179,   179,
     202,   179,   231,   254,    12,   258,   161,    34,     0,    93,
     306,    23,     0,    98,     9,    32,    37,    34,    35,    34,
     105,    12,    12,   108,    51,   321,    12,    49,   208,   209,
      52,    52,    49,    31,   285,   190,    40,    46,   219,   123,
     124,   219,    50,   296,    36,   274,    38,    39,    52,   251,
      53,    46,   247,   233,   234,    33,    47,    47,    36,    50,
      50,    47,    36,    40,    50,     1,    53,     3,     4,     5,
       6,     7,     8,     9,    10,    52,    88,    89,   273,   260,
      46,    32,   260,    34,    35,    47,   281,   282,    24,   100,
      46,   176,   103,    34,    35,   179,   107,   182,    49,    32,
     111,    34,    35,   188,   115,    47,   191,    15,    16,    17,
      18,    19,    20,    21,    22,    23,   200,    23,    41,    42,
     205,   206,    30,    47,    30,    46,    34,    35,    34,    35,
      46,   215,    41,    42,    36,   219,    38,    39,     5,     6,
      47,   225,    25,    26,    47,    46,    48,    46,    31,    32,
      33,    34,    35,    32,    46,    34,    35,    46,    46,    46,
      46,   186,   247,    47,    46,   177,    46,    52,    46,   181,
      46,   183,    49,   185,    49,   187,   260,   261,   262,    47,
      52,    55,    55,    47,    54,    51,    48,   212,   273,    52,
      51,    46,    48,    46,    48,    46,   281,   282,    48,    51,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      46,    48,    46,   238,    28,    29,    30,    48,    51,   231,
      34,    35,   306,    46,    49,    49,    34,    49,    49,    43,
      44,    51,   317,    51,    46,    46,     1,   321,    54,    49,
      11,    13,   254,   328,   164,    52,   258,   215,   182,   328,
      15,    16,    17,    18,    19,    20,    21,    22,    23,   260,
     308,   315,   274,    28,    29,    30,    -1,    -1,    -1,    34,
      35,    36,   297,   285,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,   294,    48,   296,    -1,    -1,    -1,    -1,   314,
      -1,     1,   317,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    11,   327,   328,   316,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    -1,    -1,    -1,    -1,    28,    29,
      30,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,    -1,
       1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    52,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    27,    28,    29,    30,
      -1,    -1,    -1,    34,    35,    36,     1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    48,    -1,    -1,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    -1,
      -1,    -1,    -1,    28,    29,    30,    -1,    -1,    -1,    34,
      35,    36,     1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    -1,    -1,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    -1,    -1,    -1,    -1,    28,
      29,    30,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    48,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    -1,    -1,
      -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    -1,    -1,    -1,
      34,    35,     3,     4,     5,     6,     7,     8,     9,    10,
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
      -1,    -1,    -1,    34,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    -1,    -1,    -1,    34,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    -1,    -1,    34,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    -1,    -1,
      -1,    27,    28,    29,    30,    -1,    -1,    -1,    34,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    -1,    -1,
      -1,    -1,    28,    29,    30,    -1,    -1,    -1,    34,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    57,    61,     0,    36,    38,    39,    59,    60,    31,
     136,   137,   137,   137,     1,     3,     4,     5,     6,     7,
       8,     9,    10,    24,    62,    76,    78,    79,    80,    89,
      90,    91,    95,    96,    97,    98,   111,   112,   113,   121,
     122,   123,   124,   139,   136,    37,    52,    40,    40,    46,
      34,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    34,
      41,   140,    34,   140,    34,   140,    34,   140,    53,    53,
      34,   126,   140,    60,    63,    12,    50,    82,   101,    68,
      69,    92,    70,    71,   100,   101,    64,    65,   101,   116,
      66,    67,    72,    73,   125,    74,    75,   136,    77,    32,
      34,    35,   133,    60,    60,    47,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    30,   133,   134,   135,    46,
      58,    81,   133,    46,    58,    47,    46,    58,    99,   133,
      46,    58,    23,    30,   115,   133,   114,    46,    58,    46,
      46,    47,    58,    46,    47,    49,    49,   135,   135,    34,
     127,   128,   140,    55,    55,    34,    51,    47,     1,    60,
      93,    47,    52,    47,   127,    61,    54,    52,    51,    48,
      52,    51,    25,    26,    31,    32,    33,   133,   138,     1,
      60,    83,    46,    28,    29,    43,    44,    84,    85,    86,
      94,   104,   135,    48,     1,    60,   102,   115,     1,    60,
     117,    48,    48,    34,   140,    60,    32,   129,   133,   127,
     138,    46,    84,    85,    86,    87,    88,   103,   135,    48,
      93,    32,    49,   133,    32,    49,   133,    51,    84,   135,
      84,    46,    34,   140,    46,   103,   110,    48,    46,    14,
      27,    85,    86,   105,   106,   107,   118,   135,    48,   135,
      83,    84,    84,    51,    46,   140,    49,    49,    49,    49,
     138,    51,    51,    93,   102,    46,   117,   105,   135,   135,
      41,    42,     1,    11,   130,   131,    46,    54,   138,    83,
     138,   138,   102,    41,    42,    49,    60,    49,    52,   117,
      34,   140,   109,    87,   135,   108,   130,    13,   132,   140,
     132,    60,   119,   120,   133,   140,    87,   135,    52,   140,
     119
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    56,    57,    58,    58,    59,    59,    60,    60,    61,
      61,    61,    63,    62,    64,    62,    62,    65,    62,    66,
      62,    62,    67,    62,    68,    62,    62,    69,    62,    70,
      62,    62,    71,    62,    72,    62,    62,    73,    62,    62,
      74,    62,    75,    62,    62,    62,    77,    76,    78,    78,
      79,    81,    80,    82,    82,    83,    83,    83,    83,    84,
      85,    85,    85,    85,    86,    86,    86,    86,    87,    87,
      87,    88,    89,    89,    90,    92,    91,    93,    93,    93,
      93,    94,    95,    95,    96,    96,    96,    97,    99,    98,
     100,   100,   101,   101,   102,   102,   102,   102,   103,   103,
     103,   103,   104,   104,   104,   104,   104,   104,   104,   104,
     105,   105,   105,   105,   106,   106,   106,   106,   108,   107,
     109,   107,   110,   111,   111,   112,   114,   113,   115,   115,
     115,   115,   116,   116,   117,   117,   117,   117,   118,   119,
     119,   120,   120,   121,   121,   122,   122,   123,   123,   125,
     124,   126,   124,   127,   127,   128,   128,   128,   128,   129,
     129,   130,   130,   131,   131,   131,   131,   131,   131,   131,
     132,   132,   133,   133,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   135,   135,   135,   135,   136,   136,
     137,   137,   138,   138,   138,   138,   138,   138,   139,   139,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140
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
       0,     1,     2,     2,     4,     4,     1,     1,     0,     6,
       2,     0,     1,     1,     4,     3,     2,     0,     1,     3,
       2,     1,     1,     3,     2,     4,     2,     4,     2,     1,
       2,     2,     1,     1,     2,     3,     2,     3,     0,     5,
       0,     5,     1,     2,     2,     1,     0,     6,     3,     1,
       1,     1,     2,     0,     4,     3,     2,     0,     1,     3,
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
#line 185 "src/Slice/Grammar.y"
{
}
#line 1803 "src/Slice/Grammar.cpp"
    break;

  case 3: /* opt_semicolon: ';'  */
#line 193 "src/Slice/Grammar.y"
{
}
#line 1810 "src/Slice/Grammar.cpp"
    break;

  case 4: /* opt_semicolon: %empty  */
#line 196 "src/Slice/Grammar.y"
{
}
#line 1817 "src/Slice/Grammar.cpp"
    break;

  case 5: /* global_meta_data: ICE_GLOBAL_METADATA_OPEN string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 204 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1825 "src/Slice/Grammar.cpp"
    break;

  case 6: /* global_meta_data: ICE_GLOBAL_METADATA_IGNORE string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 208 "src/Slice/Grammar.y"
{
    unit->error("global metadata must appear before any definitions");
    yyval = yyvsp[-1]; // Dummy
}
#line 1834 "src/Slice/Grammar.cpp"
    break;

  case 7: /* meta_data: ICE_METADATA_OPEN string_list ICE_METADATA_CLOSE  */
#line 218 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1842 "src/Slice/Grammar.cpp"
    break;

  case 8: /* meta_data: %empty  */
#line 222 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 1850 "src/Slice/Grammar.cpp"
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
#line 1862 "src/Slice/Grammar.cpp"
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
#line 1875 "src/Slice/Grammar.cpp"
    break;

  case 11: /* definitions: %empty  */
#line 248 "src/Slice/Grammar.y"
{
}
#line 1882 "src/Slice/Grammar.cpp"
    break;

  case 12: /* $@1: %empty  */
#line 256 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1890 "src/Slice/Grammar.cpp"
    break;

  case 14: /* $@2: %empty  */
#line 261 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1898 "src/Slice/Grammar.cpp"
    break;

  case 16: /* definition: class_decl  */
#line 266 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 1906 "src/Slice/Grammar.cpp"
    break;

  case 17: /* $@3: %empty  */
#line 270 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1914 "src/Slice/Grammar.cpp"
    break;

  case 19: /* $@4: %empty  */
#line 275 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1922 "src/Slice/Grammar.cpp"
    break;

  case 21: /* definition: interface_decl  */
#line 280 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 1930 "src/Slice/Grammar.cpp"
    break;

  case 22: /* $@5: %empty  */
#line 284 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDefPtr::dynamicCast(yyvsp[0]));
}
#line 1938 "src/Slice/Grammar.cpp"
    break;

  case 24: /* $@6: %empty  */
#line 289 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1946 "src/Slice/Grammar.cpp"
    break;

  case 26: /* definition: exception_decl  */
#line 294 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 1954 "src/Slice/Grammar.cpp"
    break;

  case 27: /* $@7: %empty  */
#line 298 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 1962 "src/Slice/Grammar.cpp"
    break;

  case 29: /* $@8: %empty  */
#line 303 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1970 "src/Slice/Grammar.cpp"
    break;

  case 31: /* definition: struct_decl  */
#line 308 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 1978 "src/Slice/Grammar.cpp"
    break;

  case 32: /* $@9: %empty  */
#line 312 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 1986 "src/Slice/Grammar.cpp"
    break;

  case 34: /* $@10: %empty  */
#line 317 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 1994 "src/Slice/Grammar.cpp"
    break;

  case 36: /* definition: sequence_def  */
#line 322 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2002 "src/Slice/Grammar.cpp"
    break;

  case 37: /* $@11: %empty  */
#line 326 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2010 "src/Slice/Grammar.cpp"
    break;

  case 39: /* definition: dictionary_def  */
#line 331 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2018 "src/Slice/Grammar.cpp"
    break;

  case 40: /* $@12: %empty  */
#line 335 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2026 "src/Slice/Grammar.cpp"
    break;

  case 42: /* $@13: %empty  */
#line 340 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2034 "src/Slice/Grammar.cpp"
    break;

  case 44: /* definition: const_def  */
#line 345 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2042 "src/Slice/Grammar.cpp"
    break;

  case 45: /* definition: error ';'  */
#line 349 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2050 "src/Slice/Grammar.cpp"
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
#line 2070 "src/Slice/Grammar.cpp"
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
#line 2086 "src/Slice/Grammar.cpp"
    break;

  case 48: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 391 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2094 "src/Slice/Grammar.cpp"
    break;

  case 49: /* exception_id: ICE_EXCEPTION keyword  */
#line 395 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2104 "src/Slice/Grammar.cpp"
    break;

  case 50: /* exception_decl: exception_id  */
#line 406 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2113 "src/Slice/Grammar.cpp"
    break;

  case 51: /* @15: %empty  */
#line 416 "src/Slice/Grammar.y"
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
#line 2130 "src/Slice/Grammar.cpp"
    break;

  case 52: /* exception_def: exception_id exception_extends @15 '{' exception_exports '}'  */
#line 429 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2142 "src/Slice/Grammar.cpp"
    break;

  case 53: /* exception_extends: extends scoped_name  */
#line 442 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2154 "src/Slice/Grammar.cpp"
    break;

  case 54: /* exception_extends: %empty  */
#line 450 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2162 "src/Slice/Grammar.cpp"
    break;

  case 55: /* exception_exports: meta_data exception_export ';' exception_exports  */
#line 459 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2175 "src/Slice/Grammar.cpp"
    break;

  case 56: /* exception_exports: error ';' exception_exports  */
#line 468 "src/Slice/Grammar.y"
{
}
#line 2182 "src/Slice/Grammar.cpp"
    break;

  case 57: /* exception_exports: meta_data exception_export  */
#line 471 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2190 "src/Slice/Grammar.cpp"
    break;

  case 58: /* exception_exports: %empty  */
#line 475 "src/Slice/Grammar.y"
{
}
#line 2197 "src/Slice/Grammar.cpp"
    break;

  case 59: /* type_id: type ICE_IDENTIFIER  */
#line 483 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    checkIdentifier(ident->v);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    yyval = typestring;
}
#line 2210 "src/Slice/Grammar.cpp"
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
#line 2232 "src/Slice/Grammar.cpp"
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
#line 2315 "src/Slice/Grammar.cpp"
    break;

  case 62: /* tag: ICE_TAG_OPEN ')'  */
#line 594 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2325 "src/Slice/Grammar.cpp"
    break;

  case 63: /* tag: ICE_TAG  */
#line 600 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2335 "src/Slice/Grammar.cpp"
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
#line 2357 "src/Slice/Grammar.cpp"
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
#line 2439 "src/Slice/Grammar.cpp"
    break;

  case 66: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 707 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2449 "src/Slice/Grammar.cpp"
    break;

  case 67: /* optional: ICE_OPTIONAL  */
#line 713 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2459 "src/Slice/Grammar.cpp"
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
#line 2472 "src/Slice/Grammar.cpp"
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
#line 2485 "src/Slice/Grammar.cpp"
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
#line 2497 "src/Slice/Grammar.cpp"
    break;

  case 72: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 761 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2505 "src/Slice/Grammar.cpp"
    break;

  case 73: /* struct_id: ICE_STRUCT keyword  */
#line 765 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2515 "src/Slice/Grammar.cpp"
    break;

  case 74: /* struct_decl: struct_id  */
#line 776 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2524 "src/Slice/Grammar.cpp"
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
#line 2546 "src/Slice/Grammar.cpp"
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
#line 2568 "src/Slice/Grammar.cpp"
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
#line 2581 "src/Slice/Grammar.cpp"
    break;

  case 78: /* struct_exports: error ';' struct_exports  */
#line 836 "src/Slice/Grammar.y"
{
}
#line 2588 "src/Slice/Grammar.cpp"
    break;

  case 79: /* struct_exports: meta_data struct_export  */
#line 839 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2596 "src/Slice/Grammar.cpp"
    break;

  case 80: /* struct_exports: %empty  */
#line 843 "src/Slice/Grammar.y"
{
}
#line 2603 "src/Slice/Grammar.cpp"
    break;

  case 82: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 857 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2611 "src/Slice/Grammar.cpp"
    break;

  case 83: /* class_name: ICE_CLASS keyword  */
#line 861 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2621 "src/Slice/Grammar.cpp"
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
#line 2650 "src/Slice/Grammar.cpp"
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
#line 2744 "src/Slice/Grammar.cpp"
    break;

  case 86: /* class_id: class_name  */
#line 987 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2755 "src/Slice/Grammar.cpp"
    break;

  case 87: /* class_decl: class_name  */
#line 999 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2766 "src/Slice/Grammar.cpp"
    break;

  case 88: /* @17: %empty  */
#line 1011 "src/Slice/Grammar.y"
{
    ClassIdTokPtr ident = ClassIdTokPtr::dynamicCast(yyvsp[-1]);
    ContainerPtr cont = unit->currentContainer();
    ClassDefPtr base = ClassDefPtr::dynamicCast(yyvsp[0]);
    ClassDefPtr cl = cont->createClassDef(ident->v, ident->t, base);
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
#line 2787 "src/Slice/Grammar.cpp"
    break;

  case 89: /* class_def: class_id class_extends @17 '{' class_exports '}'  */
#line 1028 "src/Slice/Grammar.y"
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
#line 2803 "src/Slice/Grammar.cpp"
    break;

  case 90: /* class_extends: extends scoped_name  */
#line 1045 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    yyval = 0;
    if(!types.empty())
    {
        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
        if(!cl)
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
#line 2841 "src/Slice/Grammar.cpp"
    break;

  case 91: /* class_extends: %empty  */
#line 1079 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2849 "src/Slice/Grammar.cpp"
    break;

  case 92: /* extends: ICE_EXTENDS  */
#line 1088 "src/Slice/Grammar.y"
{
}
#line 2856 "src/Slice/Grammar.cpp"
    break;

  case 93: /* extends: ':'  */
#line 1091 "src/Slice/Grammar.y"
{
}
#line 2863 "src/Slice/Grammar.cpp"
    break;

  case 94: /* class_exports: meta_data class_export ';' class_exports  */
#line 1099 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2876 "src/Slice/Grammar.cpp"
    break;

  case 95: /* class_exports: error ';' class_exports  */
#line 1108 "src/Slice/Grammar.y"
{
}
#line 2883 "src/Slice/Grammar.cpp"
    break;

  case 96: /* class_exports: meta_data class_export  */
#line 1111 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2891 "src/Slice/Grammar.cpp"
    break;

  case 97: /* class_exports: %empty  */
#line 1115 "src/Slice/Grammar.y"
{
}
#line 2898 "src/Slice/Grammar.cpp"
    break;

  case 98: /* data_member: tagged_type_id  */
#line 1123 "src/Slice/Grammar.y"
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
#line 2924 "src/Slice/Grammar.cpp"
    break;

  case 99: /* data_member: tagged_type_id '=' const_initializer  */
#line 1145 "src/Slice/Grammar.y"
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
#line 2955 "src/Slice/Grammar.cpp"
    break;

  case 100: /* data_member: type keyword  */
#line 1172 "src/Slice/Grammar.y"
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
#line 2981 "src/Slice/Grammar.cpp"
    break;

  case 101: /* data_member: type  */
#line 1194 "src/Slice/Grammar.y"
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
#line 3006 "src/Slice/Grammar.cpp"
    break;

  case 102: /* struct_data_member: type_id  */
#line 1220 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 3019 "src/Slice/Grammar.cpp"
    break;

  case 103: /* struct_data_member: type_id '=' const_initializer  */
#line 1229 "src/Slice/Grammar.y"
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
#line 3034 "src/Slice/Grammar.cpp"
    break;

  case 104: /* struct_data_member: tag type_id  */
#line 1240 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3047 "src/Slice/Grammar.cpp"
    break;

  case 105: /* struct_data_member: tag type_id '=' const_initializer  */
#line 1249 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3060 "src/Slice/Grammar.cpp"
    break;

  case 106: /* struct_data_member: optional type_id  */
#line 1258 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3073 "src/Slice/Grammar.cpp"
    break;

  case 107: /* struct_data_member: optional type_id '=' const_initializer  */
#line 1267 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3086 "src/Slice/Grammar.cpp"
    break;

  case 108: /* struct_data_member: type keyword  */
#line 1276 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3100 "src/Slice/Grammar.cpp"
    break;

  case 109: /* struct_data_member: type  */
#line 1286 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("missing data member name");
}
#line 3113 "src/Slice/Grammar.cpp"
    break;

  case 110: /* return_type: tag type  */
#line 1300 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3123 "src/Slice/Grammar.cpp"
    break;

  case 111: /* return_type: optional type  */
#line 1306 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3133 "src/Slice/Grammar.cpp"
    break;

  case 112: /* return_type: type  */
#line 1312 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3143 "src/Slice/Grammar.cpp"
    break;

  case 113: /* return_type: ICE_VOID  */
#line 1318 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    yyval = m;
}
#line 3152 "src/Slice/Grammar.cpp"
    break;

  case 114: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1328 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer());
    if(interface)
    {
        OperationPtr op = interface->createOperation(name, returnType->type, returnType->isTagged, returnType->tag);
        if(op)
        {
            interface->checkIntroduced(name, op);
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
#line 3180 "src/Slice/Grammar.cpp"
    break;

  case 115: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1352 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer());
    if(interface)
    {
        OperationPtr op = interface->createOperation(name, returnType->type, returnType->isTagged, returnType->tag,
                                                Operation::Idempotent);
        if(op)
        {
            interface->checkIntroduced(name, op);
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
#line 3209 "src/Slice/Grammar.cpp"
    break;

  case 116: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1377 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer());
    if(interface)
    {
        OperationPtr op = interface->createOperation(name, returnType->type, returnType->isTagged, returnType->tag);
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
#line 3237 "src/Slice/Grammar.cpp"
    break;

  case 117: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1401 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer());
    if(interface)
    {
        OperationPtr op = interface->createOperation(name, returnType->type, returnType->isTagged, returnType->tag,
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
#line 3266 "src/Slice/Grammar.cpp"
    break;

  case 118: /* @18: %empty  */
#line 1431 "src/Slice/Grammar.y"
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
#line 3282 "src/Slice/Grammar.cpp"
    break;

  case 119: /* operation: operation_preamble parameters ')' @18 throws  */
#line 1443 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3296 "src/Slice/Grammar.cpp"
    break;

  case 120: /* @19: %empty  */
#line 1453 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3308 "src/Slice/Grammar.cpp"
    break;

  case 121: /* operation: operation_preamble error ')' @19 throws  */
#line 1461 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3322 "src/Slice/Grammar.cpp"
    break;

  case 123: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1482 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3330 "src/Slice/Grammar.cpp"
    break;

  case 124: /* interface_id: ICE_INTERFACE keyword  */
#line 1486 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3340 "src/Slice/Grammar.cpp"
    break;

  case 125: /* interface_decl: interface_id  */
#line 1497 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3352 "src/Slice/Grammar.cpp"
    break;

  case 126: /* @20: %empty  */
#line 1510 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-1]);
    ContainerPtr cont = unit->currentContainer();
    InterfaceListTokPtr bases = InterfaceListTokPtr::dynamicCast(yyvsp[0]);
    InterfaceDefPtr interface = cont->createInterfaceDef(ident->v, bases->v);
    if(interface)
    {
        cont->checkIntroduced(ident->v, interface);
        unit->pushContainer(interface);
        yyval = interface;
    }
    else
    {
        yyval = 0;
    }
}
#line 3373 "src/Slice/Grammar.cpp"
    break;

  case 127: /* interface_def: interface_id interface_extends @20 '{' interface_exports '}'  */
#line 1527 "src/Slice/Grammar.y"
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
#line 3389 "src/Slice/Grammar.cpp"
    break;

  case 128: /* interface_list: scoped_name ',' interface_list  */
#line 1544 "src/Slice/Grammar.y"
{
    InterfaceListTokPtr intfs = InterfaceListTokPtr::dynamicCast(yyvsp[0]);
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if(!types.empty())
    {
        InterfaceDeclPtr interface = InterfaceDeclPtr::dynamicCast(types.front());
        if(!interface)
        {
            string msg = "`";
            msg += scoped->v;
            msg += "' is not an interface";
            unit->error(msg);
        }
        else
        {
            InterfaceDefPtr def = interface->definition();
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
#line 3428 "src/Slice/Grammar.cpp"
    break;

  case 129: /* interface_list: scoped_name  */
#line 1579 "src/Slice/Grammar.y"
{
    InterfaceListTokPtr intfs = new InterfaceListTok;
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if(!types.empty())
    {
        InterfaceDeclPtr interface = InterfaceDeclPtr::dynamicCast(types.front());
        if(!interface)
        {
            string msg = "`";
            msg += scoped->v;
            msg += "' is not an interface";
            unit->error(msg); // $$ is a dummy
        }
        else
        {
            InterfaceDefPtr def = interface->definition();
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
#line 3467 "src/Slice/Grammar.cpp"
    break;

  case 130: /* interface_list: ICE_OBJECT  */
#line 1614 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new InterfaceListTok; // Dummy
}
#line 3476 "src/Slice/Grammar.cpp"
    break;

  case 131: /* interface_list: ICE_VALUE  */
#line 1619 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3485 "src/Slice/Grammar.cpp"
    break;

  case 132: /* interface_extends: extends interface_list  */
#line 1629 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3493 "src/Slice/Grammar.cpp"
    break;

  case 133: /* interface_extends: %empty  */
#line 1633 "src/Slice/Grammar.y"
{
    yyval = new InterfaceListTok;
}
#line 3501 "src/Slice/Grammar.cpp"
    break;

  case 134: /* interface_exports: meta_data interface_export ';' interface_exports  */
#line 1642 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3514 "src/Slice/Grammar.cpp"
    break;

  case 135: /* interface_exports: error ';' interface_exports  */
#line 1651 "src/Slice/Grammar.y"
{
}
#line 3521 "src/Slice/Grammar.cpp"
    break;

  case 136: /* interface_exports: meta_data interface_export  */
#line 1654 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3529 "src/Slice/Grammar.cpp"
    break;

  case 137: /* interface_exports: %empty  */
#line 1658 "src/Slice/Grammar.y"
{
}
#line 3536 "src/Slice/Grammar.cpp"
    break;

  case 139: /* exception_list: exception ',' exception_list  */
#line 1672 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3547 "src/Slice/Grammar.cpp"
    break;

  case 140: /* exception_list: exception  */
#line 1679 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3558 "src/Slice/Grammar.cpp"
    break;

  case 141: /* exception: scoped_name  */
#line 1691 "src/Slice/Grammar.y"
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
#line 3574 "src/Slice/Grammar.cpp"
    break;

  case 142: /* exception: keyword  */
#line 1703 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3584 "src/Slice/Grammar.cpp"
    break;

  case 143: /* sequence_def: ICE_SEQUENCE '<' meta_data type '>' ICE_IDENTIFIER  */
#line 1714 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3596 "src/Slice/Grammar.cpp"
    break;

  case 144: /* sequence_def: ICE_SEQUENCE '<' meta_data type '>' keyword  */
#line 1722 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3609 "src/Slice/Grammar.cpp"
    break;

  case 145: /* dictionary_def: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' ICE_IDENTIFIER  */
#line 1736 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3623 "src/Slice/Grammar.cpp"
    break;

  case 146: /* dictionary_def: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' keyword  */
#line 1746 "src/Slice/Grammar.y"
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
#line 3638 "src/Slice/Grammar.cpp"
    break;

  case 147: /* enum_id: ICE_ENUM ICE_IDENTIFIER  */
#line 1762 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3646 "src/Slice/Grammar.cpp"
    break;

  case 148: /* enum_id: ICE_ENUM keyword  */
#line 1766 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3656 "src/Slice/Grammar.cpp"
    break;

  case 149: /* @21: %empty  */
#line 1777 "src/Slice/Grammar.y"
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
#line 3676 "src/Slice/Grammar.cpp"
    break;

  case 150: /* enum_def: enum_id @21 '{' enumerator_list '}'  */
#line 1793 "src/Slice/Grammar.y"
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
#line 3694 "src/Slice/Grammar.cpp"
    break;

  case 151: /* @22: %empty  */
#line 1808 "src/Slice/Grammar.y"
{
    unit->error("missing enumeration name");
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3706 "src/Slice/Grammar.cpp"
    break;

  case 152: /* enum_def: ICE_ENUM @22 '{' enumerator_list '}'  */
#line 1816 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3715 "src/Slice/Grammar.cpp"
    break;

  case 153: /* enumerator_list: enumerator ',' enumerator_list  */
#line 1826 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 3725 "src/Slice/Grammar.cpp"
    break;

  case 154: /* enumerator_list: enumerator  */
#line 1832 "src/Slice/Grammar.y"
{
}
#line 3732 "src/Slice/Grammar.cpp"
    break;

  case 155: /* enumerator: ICE_IDENTIFIER  */
#line 1840 "src/Slice/Grammar.y"
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
#line 3748 "src/Slice/Grammar.cpp"
    break;

  case 156: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1852 "src/Slice/Grammar.y"
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
#line 3772 "src/Slice/Grammar.cpp"
    break;

  case 157: /* enumerator: keyword  */
#line 1872 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 3783 "src/Slice/Grammar.cpp"
    break;

  case 158: /* enumerator: %empty  */
#line 1879 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 3792 "src/Slice/Grammar.cpp"
    break;

  case 159: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1889 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3800 "src/Slice/Grammar.cpp"
    break;

  case 160: /* enumerator_initializer: scoped_name  */
#line 1893 "src/Slice/Grammar.y"
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
#line 3837 "src/Slice/Grammar.cpp"
    break;

  case 161: /* out_qualifier: ICE_OUT  */
#line 1931 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = true;
    yyval = out;
}
#line 3847 "src/Slice/Grammar.cpp"
    break;

  case 162: /* out_qualifier: %empty  */
#line 1937 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = false;
    yyval = out;
}
#line 3857 "src/Slice/Grammar.cpp"
    break;

  case 163: /* parameters: %empty  */
#line 1948 "src/Slice/Grammar.y"
{
}
#line 3864 "src/Slice/Grammar.cpp"
    break;

  case 164: /* parameters: out_qualifier meta_data tagged_type_id  */
#line 1951 "src/Slice/Grammar.y"
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
#line 3884 "src/Slice/Grammar.cpp"
    break;

  case 165: /* parameters: parameters ',' out_qualifier meta_data tagged_type_id  */
#line 1967 "src/Slice/Grammar.y"
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
#line 3904 "src/Slice/Grammar.cpp"
    break;

  case 166: /* parameters: out_qualifier meta_data type keyword  */
#line 1983 "src/Slice/Grammar.y"
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
#line 3920 "src/Slice/Grammar.cpp"
    break;

  case 167: /* parameters: parameters ',' out_qualifier meta_data type keyword  */
#line 1995 "src/Slice/Grammar.y"
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
#line 3936 "src/Slice/Grammar.cpp"
    break;

  case 168: /* parameters: out_qualifier meta_data type  */
#line 2007 "src/Slice/Grammar.y"
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
#line 3951 "src/Slice/Grammar.cpp"
    break;

  case 169: /* parameters: parameters ',' out_qualifier meta_data type  */
#line 2018 "src/Slice/Grammar.y"
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
#line 3966 "src/Slice/Grammar.cpp"
    break;

  case 170: /* throws: ICE_THROWS exception_list  */
#line 2034 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3974 "src/Slice/Grammar.cpp"
    break;

  case 171: /* throws: %empty  */
#line 2038 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 3982 "src/Slice/Grammar.cpp"
    break;

  case 172: /* scoped_name: ICE_IDENTIFIER  */
#line 2047 "src/Slice/Grammar.y"
{
}
#line 3989 "src/Slice/Grammar.cpp"
    break;

  case 173: /* scoped_name: ICE_SCOPED_IDENTIFIER  */
#line 2050 "src/Slice/Grammar.y"
{
}
#line 3996 "src/Slice/Grammar.cpp"
    break;

  case 174: /* builtin: ICE_BOOL  */
#line 2057 "src/Slice/Grammar.y"
           {}
#line 4002 "src/Slice/Grammar.cpp"
    break;

  case 175: /* builtin: ICE_BYTE  */
#line 2058 "src/Slice/Grammar.y"
           {}
#line 4008 "src/Slice/Grammar.cpp"
    break;

  case 176: /* builtin: ICE_SHORT  */
#line 2059 "src/Slice/Grammar.y"
            {}
#line 4014 "src/Slice/Grammar.cpp"
    break;

  case 177: /* builtin: ICE_INT  */
#line 2060 "src/Slice/Grammar.y"
          {}
#line 4020 "src/Slice/Grammar.cpp"
    break;

  case 178: /* builtin: ICE_LONG  */
#line 2061 "src/Slice/Grammar.y"
           {}
#line 4026 "src/Slice/Grammar.cpp"
    break;

  case 179: /* builtin: ICE_FLOAT  */
#line 2062 "src/Slice/Grammar.y"
            {}
#line 4032 "src/Slice/Grammar.cpp"
    break;

  case 180: /* builtin: ICE_DOUBLE  */
#line 2063 "src/Slice/Grammar.y"
             {}
#line 4038 "src/Slice/Grammar.cpp"
    break;

  case 181: /* builtin: ICE_STRING  */
#line 2064 "src/Slice/Grammar.y"
             {}
#line 4044 "src/Slice/Grammar.cpp"
    break;

  case 182: /* builtin: ICE_OBJECT  */
#line 2065 "src/Slice/Grammar.y"
             {}
#line 4050 "src/Slice/Grammar.cpp"
    break;

  case 183: /* builtin: ICE_VALUE  */
#line 2066 "src/Slice/Grammar.y"
            {}
#line 4056 "src/Slice/Grammar.cpp"
    break;

  case 184: /* type: ICE_OBJECT '*'  */
#line 2072 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindObjectProxy);
}
#line 4064 "src/Slice/Grammar.cpp"
    break;

  case 185: /* type: builtin  */
#line 2076 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 4073 "src/Slice/Grammar.cpp"
    break;

  case 186: /* type: scoped_name  */
#line 2081 "src/Slice/Grammar.y"
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
        for(TypeList::iterator p = types.begin(); p != types.end(); ++p)
        {
            InterfaceDeclPtr interface = InterfaceDeclPtr::dynamicCast(*p);
            if(interface)
            {
                string msg = "add '*' to specify the proxy type for interface '";
                msg += scoped->v;
                msg += "'";
                unit->error(msg);
                YYERROR; // Can't continue, jump to next yyerrok
            }
            cont->checkIntroduced(scoped->v);
        }
        yyval = types.front();
    }
    else
    {
        yyval = 0;
    }
}
#line 4108 "src/Slice/Grammar.cpp"
    break;

  case 187: /* type: scoped_name '*'  */
#line 2112 "src/Slice/Grammar.y"
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
            InterfaceDeclPtr interface = InterfaceDeclPtr::dynamicCast(*p);
            if(!interface)
            {
                string msg = "`";
                msg += scoped->v;
                msg += "' must be an interface";
                unit->error(msg);
                YYERROR; // Can't continue, jump to next yyerrok
            }
            cont->checkIntroduced(scoped->v);
        }
        yyval = types.front();
    }
    else
    {
        yyval = 0;
    }
}
#line 4143 "src/Slice/Grammar.cpp"
    break;

  case 188: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 2148 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4153 "src/Slice/Grammar.cpp"
    break;

  case 189: /* string_literal: ICE_STRING_LITERAL  */
#line 2154 "src/Slice/Grammar.y"
{
}
#line 4160 "src/Slice/Grammar.cpp"
    break;

  case 190: /* string_list: string_list ',' string_literal  */
#line 2162 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4171 "src/Slice/Grammar.cpp"
    break;

  case 191: /* string_list: string_literal  */
#line 2169 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4182 "src/Slice/Grammar.cpp"
    break;

  case 192: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2181 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4195 "src/Slice/Grammar.cpp"
    break;

  case 193: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2190 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4208 "src/Slice/Grammar.cpp"
    break;

  case 194: /* const_initializer: scoped_name  */
#line 2199 "src/Slice/Grammar.y"
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
#line 4252 "src/Slice/Grammar.cpp"
    break;

  case 195: /* const_initializer: ICE_STRING_LITERAL  */
#line 2239 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4263 "src/Slice/Grammar.cpp"
    break;

  case 196: /* const_initializer: ICE_FALSE  */
#line 2246 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4274 "src/Slice/Grammar.cpp"
    break;

  case 197: /* const_initializer: ICE_TRUE  */
#line 2253 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4285 "src/Slice/Grammar.cpp"
    break;

  case 198: /* const_def: ICE_CONST meta_data type ICE_IDENTIFIER '=' const_initializer  */
#line 2265 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4298 "src/Slice/Grammar.cpp"
    break;

  case 199: /* const_def: ICE_CONST meta_data type '=' const_initializer  */
#line 2274 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4311 "src/Slice/Grammar.cpp"
    break;

  case 200: /* keyword: ICE_MODULE  */
#line 2287 "src/Slice/Grammar.y"
             {}
#line 4317 "src/Slice/Grammar.cpp"
    break;

  case 201: /* keyword: ICE_CLASS  */
#line 2288 "src/Slice/Grammar.y"
            {}
#line 4323 "src/Slice/Grammar.cpp"
    break;

  case 202: /* keyword: ICE_INTERFACE  */
#line 2289 "src/Slice/Grammar.y"
                {}
#line 4329 "src/Slice/Grammar.cpp"
    break;

  case 203: /* keyword: ICE_EXCEPTION  */
#line 2290 "src/Slice/Grammar.y"
                {}
#line 4335 "src/Slice/Grammar.cpp"
    break;

  case 204: /* keyword: ICE_STRUCT  */
#line 2291 "src/Slice/Grammar.y"
             {}
#line 4341 "src/Slice/Grammar.cpp"
    break;

  case 205: /* keyword: ICE_SEQUENCE  */
#line 2292 "src/Slice/Grammar.y"
               {}
#line 4347 "src/Slice/Grammar.cpp"
    break;

  case 206: /* keyword: ICE_DICTIONARY  */
#line 2293 "src/Slice/Grammar.y"
                 {}
#line 4353 "src/Slice/Grammar.cpp"
    break;

  case 207: /* keyword: ICE_ENUM  */
#line 2294 "src/Slice/Grammar.y"
           {}
#line 4359 "src/Slice/Grammar.cpp"
    break;

  case 208: /* keyword: ICE_OUT  */
#line 2295 "src/Slice/Grammar.y"
          {}
#line 4365 "src/Slice/Grammar.cpp"
    break;

  case 209: /* keyword: ICE_EXTENDS  */
#line 2296 "src/Slice/Grammar.y"
              {}
#line 4371 "src/Slice/Grammar.cpp"
    break;

  case 210: /* keyword: ICE_THROWS  */
#line 2297 "src/Slice/Grammar.y"
             {}
#line 4377 "src/Slice/Grammar.cpp"
    break;

  case 211: /* keyword: ICE_VOID  */
#line 2298 "src/Slice/Grammar.y"
           {}
#line 4383 "src/Slice/Grammar.cpp"
    break;

  case 212: /* keyword: ICE_BOOL  */
#line 2299 "src/Slice/Grammar.y"
           {}
#line 4389 "src/Slice/Grammar.cpp"
    break;

  case 213: /* keyword: ICE_BYTE  */
#line 2300 "src/Slice/Grammar.y"
           {}
#line 4395 "src/Slice/Grammar.cpp"
    break;

  case 214: /* keyword: ICE_SHORT  */
#line 2301 "src/Slice/Grammar.y"
            {}
#line 4401 "src/Slice/Grammar.cpp"
    break;

  case 215: /* keyword: ICE_INT  */
#line 2302 "src/Slice/Grammar.y"
          {}
#line 4407 "src/Slice/Grammar.cpp"
    break;

  case 216: /* keyword: ICE_LONG  */
#line 2303 "src/Slice/Grammar.y"
           {}
#line 4413 "src/Slice/Grammar.cpp"
    break;

  case 217: /* keyword: ICE_FLOAT  */
#line 2304 "src/Slice/Grammar.y"
            {}
#line 4419 "src/Slice/Grammar.cpp"
    break;

  case 218: /* keyword: ICE_DOUBLE  */
#line 2305 "src/Slice/Grammar.y"
             {}
#line 4425 "src/Slice/Grammar.cpp"
    break;

  case 219: /* keyword: ICE_STRING  */
#line 2306 "src/Slice/Grammar.y"
             {}
#line 4431 "src/Slice/Grammar.cpp"
    break;

  case 220: /* keyword: ICE_OBJECT  */
#line 2307 "src/Slice/Grammar.y"
             {}
#line 4437 "src/Slice/Grammar.cpp"
    break;

  case 221: /* keyword: ICE_CONST  */
#line 2308 "src/Slice/Grammar.y"
            {}
#line 4443 "src/Slice/Grammar.cpp"
    break;

  case 222: /* keyword: ICE_FALSE  */
#line 2309 "src/Slice/Grammar.y"
            {}
#line 4449 "src/Slice/Grammar.cpp"
    break;

  case 223: /* keyword: ICE_TRUE  */
#line 2310 "src/Slice/Grammar.y"
           {}
#line 4455 "src/Slice/Grammar.cpp"
    break;

  case 224: /* keyword: ICE_IDEMPOTENT  */
#line 2311 "src/Slice/Grammar.y"
                 {}
#line 4461 "src/Slice/Grammar.cpp"
    break;

  case 225: /* keyword: ICE_TAG  */
#line 2312 "src/Slice/Grammar.y"
          {}
#line 4467 "src/Slice/Grammar.cpp"
    break;

  case 226: /* keyword: ICE_OPTIONAL  */
#line 2313 "src/Slice/Grammar.y"
               {}
#line 4473 "src/Slice/Grammar.cpp"
    break;

  case 227: /* keyword: ICE_VALUE  */
#line 2314 "src/Slice/Grammar.y"
            {}
#line 4479 "src/Slice/Grammar.cpp"
    break;


#line 4483 "src/Slice/Grammar.cpp"

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

#line 2317 "src/Slice/Grammar.y"

