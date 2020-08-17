/* A Bison parser, made by GNU Bison 3.7.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.7.1"

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

#line 30 "src/Slice/Grammar.y"


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
#line 69 "src/Slice/Grammar.y"


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
  YYSYMBOL_ICE_IMPLEMENTS = 13,            /* ICE_IMPLEMENTS  */
  YYSYMBOL_ICE_THROWS = 14,                /* ICE_THROWS  */
  YYSYMBOL_ICE_VOID = 15,                  /* ICE_VOID  */
  YYSYMBOL_ICE_BOOL = 16,                  /* ICE_BOOL  */
  YYSYMBOL_ICE_BYTE = 17,                  /* ICE_BYTE  */
  YYSYMBOL_ICE_SHORT = 18,                 /* ICE_SHORT  */
  YYSYMBOL_ICE_USHORT = 19,                /* ICE_USHORT  */
  YYSYMBOL_ICE_INT = 20,                   /* ICE_INT  */
  YYSYMBOL_ICE_UINT = 21,                  /* ICE_UINT  */
  YYSYMBOL_ICE_VARINT = 22,                /* ICE_VARINT  */
  YYSYMBOL_ICE_VARUINT = 23,               /* ICE_VARUINT  */
  YYSYMBOL_ICE_LONG = 24,                  /* ICE_LONG  */
  YYSYMBOL_ICE_ULONG = 25,                 /* ICE_ULONG  */
  YYSYMBOL_ICE_VARLONG = 26,               /* ICE_VARLONG  */
  YYSYMBOL_ICE_VARULONG = 27,              /* ICE_VARULONG  */
  YYSYMBOL_ICE_FLOAT = 28,                 /* ICE_FLOAT  */
  YYSYMBOL_ICE_DOUBLE = 29,                /* ICE_DOUBLE  */
  YYSYMBOL_ICE_STRING = 30,                /* ICE_STRING  */
  YYSYMBOL_ICE_OBJECT = 31,                /* ICE_OBJECT  */
  YYSYMBOL_ICE_CONST = 32,                 /* ICE_CONST  */
  YYSYMBOL_ICE_FALSE = 33,                 /* ICE_FALSE  */
  YYSYMBOL_ICE_TRUE = 34,                  /* ICE_TRUE  */
  YYSYMBOL_ICE_IDEMPOTENT = 35,            /* ICE_IDEMPOTENT  */
  YYSYMBOL_ICE_TAG = 36,                   /* ICE_TAG  */
  YYSYMBOL_ICE_OPTIONAL = 37,              /* ICE_OPTIONAL  */
  YYSYMBOL_ICE_ANYCLASS = 38,              /* ICE_ANYCLASS  */
  YYSYMBOL_ICE_VALUE = 39,                 /* ICE_VALUE  */
  YYSYMBOL_ICE_UNCHECKED = 40,             /* ICE_UNCHECKED  */
  YYSYMBOL_ICE_STRING_LITERAL = 41,        /* ICE_STRING_LITERAL  */
  YYSYMBOL_ICE_INTEGER_LITERAL = 42,       /* ICE_INTEGER_LITERAL  */
  YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 43, /* ICE_FLOATING_POINT_LITERAL  */
  YYSYMBOL_ICE_IDENTIFIER = 44,            /* ICE_IDENTIFIER  */
  YYSYMBOL_ICE_SCOPED_IDENTIFIER = 45,     /* ICE_SCOPED_IDENTIFIER  */
  YYSYMBOL_ICE_LOCAL_METADATA_OPEN = 46,   /* ICE_LOCAL_METADATA_OPEN  */
  YYSYMBOL_ICE_LOCAL_METADATA_CLOSE = 47,  /* ICE_LOCAL_METADATA_CLOSE  */
  YYSYMBOL_ICE_FILE_METADATA_OPEN = 48,    /* ICE_FILE_METADATA_OPEN  */
  YYSYMBOL_ICE_FILE_METADATA_IGNORE = 49,  /* ICE_FILE_METADATA_IGNORE  */
  YYSYMBOL_ICE_FILE_METADATA_CLOSE = 50,   /* ICE_FILE_METADATA_CLOSE  */
  YYSYMBOL_ICE_IDENT_OPEN = 51,            /* ICE_IDENT_OPEN  */
  YYSYMBOL_ICE_KEYWORD_OPEN = 52,          /* ICE_KEYWORD_OPEN  */
  YYSYMBOL_ICE_TAG_OPEN = 53,              /* ICE_TAG_OPEN  */
  YYSYMBOL_ICE_OPTIONAL_OPEN = 54,         /* ICE_OPTIONAL_OPEN  */
  YYSYMBOL_BAD_CHAR = 55,                  /* BAD_CHAR  */
  YYSYMBOL_56_ = 56,                       /* ';'  */
  YYSYMBOL_57_ = 57,                       /* '{'  */
  YYSYMBOL_58_ = 58,                       /* '}'  */
  YYSYMBOL_59_ = 59,                       /* ')'  */
  YYSYMBOL_60_ = 60,                       /* ':'  */
  YYSYMBOL_61_ = 61,                       /* '='  */
  YYSYMBOL_62_ = 62,                       /* ','  */
  YYSYMBOL_63_ = 63,                       /* '<'  */
  YYSYMBOL_64_ = 64,                       /* '>'  */
  YYSYMBOL_65_ = 65,                       /* '*'  */
  YYSYMBOL_66_ = 66,                       /* '?'  */
  YYSYMBOL_YYACCEPT = 67,                  /* $accept  */
  YYSYMBOL_start = 68,                     /* start  */
  YYSYMBOL_opt_semicolon = 69,             /* opt_semicolon  */
  YYSYMBOL_file_metadata = 70,             /* file_metadata  */
  YYSYMBOL_local_metadata = 71,            /* local_metadata  */
  YYSYMBOL_definitions = 72,               /* definitions  */
  YYSYMBOL_definition = 73,                /* definition  */
  YYSYMBOL_74_1 = 74,                      /* $@1  */
  YYSYMBOL_75_2 = 75,                      /* $@2  */
  YYSYMBOL_76_3 = 76,                      /* $@3  */
  YYSYMBOL_77_4 = 77,                      /* $@4  */
  YYSYMBOL_78_5 = 78,                      /* $@5  */
  YYSYMBOL_79_6 = 79,                      /* $@6  */
  YYSYMBOL_80_7 = 80,                      /* $@7  */
  YYSYMBOL_81_8 = 81,                      /* $@8  */
  YYSYMBOL_82_9 = 82,                      /* $@9  */
  YYSYMBOL_83_10 = 83,                     /* $@10  */
  YYSYMBOL_84_11 = 84,                     /* $@11  */
  YYSYMBOL_85_12 = 85,                     /* $@12  */
  YYSYMBOL_86_13 = 86,                     /* $@13  */
  YYSYMBOL_module_def = 87,                /* module_def  */
  YYSYMBOL_88_14 = 88,                     /* @14  */
  YYSYMBOL_89_15 = 89,                     /* @15  */
  YYSYMBOL_exception_id = 90,              /* exception_id  */
  YYSYMBOL_exception_decl = 91,            /* exception_decl  */
  YYSYMBOL_exception_def = 92,             /* exception_def  */
  YYSYMBOL_93_16 = 93,                     /* @16  */
  YYSYMBOL_exception_extends = 94,         /* exception_extends  */
  YYSYMBOL_tag = 95,                       /* tag  */
  YYSYMBOL_optional = 96,                  /* optional  */
  YYSYMBOL_struct_id = 97,                 /* struct_id  */
  YYSYMBOL_struct_decl = 98,               /* struct_decl  */
  YYSYMBOL_struct_def = 99,                /* struct_def  */
  YYSYMBOL_100_17 = 100,                   /* @17  */
  YYSYMBOL_class_name = 101,               /* class_name  */
  YYSYMBOL_class_id = 102,                 /* class_id  */
  YYSYMBOL_class_decl = 103,               /* class_decl  */
  YYSYMBOL_class_def = 104,                /* class_def  */
  YYSYMBOL_105_18 = 105,                   /* @18  */
  YYSYMBOL_class_extends = 106,            /* class_extends  */
  YYSYMBOL_extends = 107,                  /* extends  */
  YYSYMBOL_data_member = 108,              /* data_member  */
  YYSYMBOL_data_member_list = 109,         /* data_member_list  */
  YYSYMBOL_data_members = 110,             /* data_members  */
  YYSYMBOL_return_type = 111,              /* return_type  */
  YYSYMBOL_operation_preamble = 112,       /* operation_preamble  */
  YYSYMBOL_operation = 113,                /* operation  */
  YYSYMBOL_114_19 = 114,                   /* @19  */
  YYSYMBOL_115_20 = 115,                   /* @20  */
  YYSYMBOL_operation_list = 116,           /* operation_list  */
  YYSYMBOL_interface_id = 117,             /* interface_id  */
  YYSYMBOL_interface_decl = 118,           /* interface_decl  */
  YYSYMBOL_interface_def = 119,            /* interface_def  */
  YYSYMBOL_120_21 = 120,                   /* @21  */
  YYSYMBOL_interface_list = 121,           /* interface_list  */
  YYSYMBOL_interface_extends = 122,        /* interface_extends  */
  YYSYMBOL_exception_list = 123,           /* exception_list  */
  YYSYMBOL_exception = 124,                /* exception  */
  YYSYMBOL_sequence_def = 125,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 126,           /* dictionary_def  */
  YYSYMBOL_enum_start = 127,               /* enum_start  */
  YYSYMBOL_enum_id = 128,                  /* enum_id  */
  YYSYMBOL_enum_def = 129,                 /* enum_def  */
  YYSYMBOL_130_22 = 130,                   /* @22  */
  YYSYMBOL_131_23 = 131,                   /* @23  */
  YYSYMBOL_enum_underlying = 132,          /* enum_underlying  */
  YYSYMBOL_enumerator_list = 133,          /* enumerator_list  */
  YYSYMBOL_enumerator = 134,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 135,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 136,            /* out_qualifier  */
  YYSYMBOL_parameter = 137,                /* parameter  */
  YYSYMBOL_parameter_list = 138,           /* parameter_list  */
  YYSYMBOL_parameters = 139,               /* parameters  */
  YYSYMBOL_throws = 140,                   /* throws  */
  YYSYMBOL_scoped_name = 141,              /* scoped_name  */
  YYSYMBOL_builtin = 142,                  /* builtin  */
  YYSYMBOL_type = 143,                     /* type  */
  YYSYMBOL_tagged_type = 144,              /* tagged_type  */
  YYSYMBOL_member = 145,                   /* member  */
  YYSYMBOL_string_literal = 146,           /* string_literal  */
  YYSYMBOL_string_list = 147,              /* string_list  */
  YYSYMBOL_const_initializer = 148,        /* const_initializer  */
  YYSYMBOL_const_def = 149,                /* const_def  */
  YYSYMBOL_keyword = 150                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 61 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 361 "src/Slice/Grammar.cpp"

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
#define YYLAST   1049

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  84
/* YYNRULES -- Number of rules.  */
#define YYNRULES  236
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  331

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   310


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
       2,    59,    65,     2,    62,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    60,    56,
      63,    61,    64,    66,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    57,     2,    58,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   195,   195,   201,   202,   208,   212,   222,   226,   233,
     242,   250,   259,   266,   265,   271,   270,   275,   280,   279,
     285,   284,   289,   294,   293,   299,   298,   303,   308,   307,
     313,   312,   317,   322,   321,   327,   326,   331,   336,   335,
     340,   345,   344,   350,   349,   354,   358,   368,   367,   406,
     405,   483,   487,   498,   509,   508,   534,   542,   551,   569,
     643,   649,   660,   682,   760,   770,   785,   789,   800,   811,
     810,   851,   855,   866,   891,   981,   993,  1006,  1005,  1039,
    1073,  1082,  1083,  1089,  1104,  1126,  1127,  1128,  1132,  1138,
    1139,  1145,  1149,  1159,  1185,  1211,  1237,  1269,  1268,  1291,
    1290,  1313,  1322,  1326,  1327,  1333,  1337,  1348,  1362,  1361,
    1395,  1430,  1465,  1470,  1475,  1489,  1493,  1502,  1509,  1521,
    1533,  1544,  1552,  1566,  1576,  1592,  1596,  1605,  1621,  1635,
    1634,  1657,  1656,  1675,  1679,  1688,  1694,  1700,  1712,  1725,
    1732,  1742,  1746,  1784,  1788,  1797,  1816,  1817,  1823,  1824,
    1830,  1834,  1843,  1844,  1850,  1851,  1852,  1853,  1854,  1855,
    1856,  1857,  1858,  1859,  1860,  1861,  1862,  1863,  1864,  1869,
    1873,  1877,  1881,  1889,  1894,  1905,  1909,  1921,  1926,  1952,
    1984,  2012,  2027,  2041,  2052,  2059,  2066,  2073,  2084,  2090,
    2096,  2103,  2115,  2124,  2133,  2173,  2180,  2187,  2199,  2208,
    2222,  2223,  2224,  2225,  2226,  2227,  2228,  2229,  2230,  2231,
    2232,  2233,  2234,  2235,  2236,  2237,  2238,  2239,  2240,  2241,
    2242,  2243,  2244,  2245,  2246,  2247,  2248,  2249,  2250,  2251,
    2252,  2253,  2254,  2255,  2256,  2257,  2258
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
  "ICE_SHORT", "ICE_USHORT", "ICE_INT", "ICE_UINT", "ICE_VARINT",
  "ICE_VARUINT", "ICE_LONG", "ICE_ULONG", "ICE_VARLONG", "ICE_VARULONG",
  "ICE_FLOAT", "ICE_DOUBLE", "ICE_STRING", "ICE_OBJECT", "ICE_CONST",
  "ICE_FALSE", "ICE_TRUE", "ICE_IDEMPOTENT", "ICE_TAG", "ICE_OPTIONAL",
  "ICE_ANYCLASS", "ICE_VALUE", "ICE_UNCHECKED", "ICE_STRING_LITERAL",
  "ICE_INTEGER_LITERAL", "ICE_FLOATING_POINT_LITERAL", "ICE_IDENTIFIER",
  "ICE_SCOPED_IDENTIFIER", "ICE_LOCAL_METADATA_OPEN",
  "ICE_LOCAL_METADATA_CLOSE", "ICE_FILE_METADATA_OPEN",
  "ICE_FILE_METADATA_IGNORE", "ICE_FILE_METADATA_CLOSE", "ICE_IDENT_OPEN",
  "ICE_KEYWORD_OPEN", "ICE_TAG_OPEN", "ICE_OPTIONAL_OPEN", "BAD_CHAR",
  "';'", "'{'", "'}'", "')'", "':'", "'='", "','", "'<'", "'>'", "'*'",
  "'?'", "$accept", "start", "opt_semicolon", "file_metadata",
  "local_metadata", "definitions", "definition", "$@1", "$@2", "$@3",
  "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "$@12", "$@13",
  "module_def", "@14", "@15", "exception_id", "exception_decl",
  "exception_def", "@16", "exception_extends", "tag", "optional",
  "struct_id", "struct_decl", "struct_def", "@17", "class_name",
  "class_id", "class_decl", "class_def", "@18", "class_extends", "extends",
  "data_member", "data_member_list", "data_members", "return_type",
  "operation_preamble", "operation", "@19", "@20", "operation_list",
  "interface_id", "interface_decl", "interface_def", "@21",
  "interface_list", "interface_extends", "exception_list", "exception",
  "sequence_def", "dictionary_def", "enum_start", "enum_id", "enum_def",
  "@22", "@23", "enum_underlying", "enumerator_list", "enumerator",
  "enumerator_initializer", "out_qualifier", "parameter", "parameter_list",
  "parameters", "throws", "scoped_name", "builtin", "type", "tagged_type",
  "member", "string_literal", "string_list", "const_initializer",
  "const_def", "keyword", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,    59,   123,   125,    41,
      58,    61,    44,    60,    62,    42,    63
};
#endif

#define YYPACT_NINF (-272)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-150)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -272,    14,    18,  -272,   -22,   -22,   -22,  -272,   104,   -22,
    -272,   -32,   151,   152,   -46,    53,   400,   492,   534,   576,
     -42,    11,  -272,    -6,    37,   -22,  -272,  -272,    23,    33,
    -272,    35,    50,  -272,    24,     5,    61,  -272,    30,    65,
    -272,    76,    78,   618,   120,  -272,    95,  -272,  -272,   -22,
    -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,    51,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,    -6,    -6,   973,  -272,   -31,   138,  -272,  -272,  -272,
      82,   161,   138,   162,   164,   138,  -272,    82,   166,   138,
     148,  -272,   167,   138,   168,   169,  -272,   170,  -272,  1004,
    -272,   138,   172,  -272,   173,   174,   149,  -272,  -272,   159,
     973,   973,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,  -272,    77,   163,   176,
     119,   177,   -33,  -272,  -272,  -272,   179,  -272,  -272,  -272,
     236,  -272,  -272,   181,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,   178,   182,  -272,  -272,  -272,  -272,   660,  -272,   187,
    -272,  -272,  -272,  -272,  -272,  -272,   183,   184,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,   160,   155,   236,   189,  -272,
    -272,    57,    86,   895,  1004,  1004,   192,   298,   191,  -272,
     702,   207,   236,   148,   137,   208,   212,   188,  -272,   660,
      91,   157,   744,    -6,   155,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,   213,  -272,   217,  -272,   218,   219,  -272,   220,
     -22,  -272,  -272,  -272,  -272,   344,  -272,  -272,  -272,   155,
     225,  -272,   228,   816,   229,   165,  -272,   660,   230,  -272,
    -272,  -272,  -272,   973,  -272,  -272,  -272,  -272,  -272,  -272,
     -23,  -272,  -272,   137,  -272,   856,    96,    32,   235,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,   222,  -272,  -272,   118,
    -272,  -272,   233,  -272,   934,  -272,   234,   238,   137,   786,
    -272,  -272,  -272,  -272,   284,  -272,  -272,  -272,  -272,   286,
    -272,   286,   449,  -272,  -272,  -272,   239,  -272,  -272,   449,
    -272
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      12,     0,     9,     1,     0,     0,     0,    10,     0,   189,
     191,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   126,     9,     0,     0,    11,    13,    53,    27,
      28,    68,    32,    33,    76,    80,    17,    18,   107,    22,
      23,    37,    40,   131,   134,    41,    45,   188,     7,     0,
       5,     6,    46,    47,    49,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,    71,     0,    72,   105,   106,    51,    52,    66,
      67,     9,     9,     0,   125,     0,     4,    81,    82,    54,
       0,     0,     4,     0,     0,     4,    77,     0,     0,     4,
       0,   108,     0,     4,     0,     0,   127,     0,   128,     0,
     129,     4,     0,   190,     0,     0,     0,   152,   153,     0,
       0,     0,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   174,   175,   176,
     178,   177,     0,     8,     3,    14,     0,    56,    26,    29,
       0,    31,    34,     0,    79,    16,    19,   112,   113,   114,
     115,   111,     0,    21,    24,    36,    39,   140,   133,     0,
      42,    44,    12,    12,    73,    74,     0,     0,   169,   170,
     171,   172,   179,   180,   173,     0,     0,     0,     0,    61,
      65,     0,     0,     0,     0,     0,    87,     0,     0,   183,
     186,    83,     0,     0,     0,   137,     0,   136,   139,   140,
       9,     9,     0,     9,     0,   196,   197,   195,   192,   193,
     194,   199,     0,    86,     0,    60,     0,     0,    64,     0,
       0,   187,   181,   182,    85,     0,    70,   184,   185,     0,
       0,   110,     0,     0,     0,     0,   132,   140,     0,    48,
      50,   121,   122,     0,   198,    55,    58,    59,    62,    63,
       0,    84,    78,     0,    92,     0,     0,     0,   102,    91,
     109,   141,   138,   142,   135,   130,     0,     7,   103,     0,
      93,    95,     0,   143,     0,   146,   148,     0,     0,     0,
      94,    96,    99,   145,   144,    97,   101,   123,   124,   151,
     147,   151,     0,   100,    98,   150,   118,   119,   120,     0,
     117
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -272,  -272,    10,  -272,     2,    19,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,  -272,
     -15,  -272,  -176,  -134,    13,  -272,  -272,  -272,  -272,  -271,
    -272,  -272,  -272,  -272,    79,  -272,   -26,  -272,  -272,  -272,
    -272,  -272,  -272,  -272,  -272,  -272,  -185,  -272,  -272,  -272,
     -10,  -272,  -272,   -14,   -88,  -272,   -95,  -191,  -204,    66,
       1,  -221,  -272,   -16
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   165,     7,   213,     2,    26,   106,   118,   119,
     122,   123,   111,   112,   114,   115,   124,   125,   131,   132,
      27,   134,   135,    28,    29,    30,   166,   109,   214,   215,
      31,    32,    33,   113,    34,    35,    36,    37,   173,   116,
     110,   216,   217,   218,   286,   287,   288,   321,   319,   264,
      38,    39,    40,   182,   180,   121,   325,   326,    41,    42,
      43,    44,    45,   189,   127,   130,   226,   227,   292,   304,
     305,   306,   307,   323,   160,   161,   219,   220,   221,    10,
      11,   241,    46,   228
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      94,    96,    98,   100,     8,   139,    12,    13,   162,   251,
      52,   205,   298,   274,     3,    48,   163,   107,    -2,     9,
     117,   101,   167,   120,   297,   103,   105,   128,   206,   174,
      49,    49,   181,   302,   188,   107,   -75,   316,   281,    49,
       4,   255,   107,   303,   268,   196,   197,   104,  -144,  -144,
    -144,  -144,  -144,  -144,  -144,  -144,  -144,  -144,  -144,  -144,
    -144,  -144,  -144,  -144,     4,   108,     5,     6,  -144,  -144,
    -144,  -144,   289,   242,   102,    47,  -144,  -144,  -144,   255,
     -57,   -75,   294,   108,   -75,  -144,  -144,  -116,   260,   -25,
     108,  -149,   -69,   136,   289,   137,   138,    53,    54,   244,
     313,   137,   138,   140,   141,    14,   -30,    15,    16,    17,
      18,    19,    20,    21,    22,   133,   245,   -15,   240,   252,
     253,   -20,   169,   246,   249,   172,   137,   138,   247,   176,
     137,   138,   -35,   184,   -38,   181,    23,     4,   262,     5,
       6,   190,   198,   199,    24,   248,   240,   300,   301,   269,
      25,   -43,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,   310,
     311,   240,    -9,    -9,    -9,    -9,    -9,   293,   296,   177,
     129,    -9,    -9,     4,   202,   203,   178,   179,   235,   236,
      -9,    -9,   137,   138,   164,  -104,   237,   238,   239,   137,
     138,    50,    51,     4,   258,     5,     6,   291,   194,   137,
     138,   230,   231,    49,    49,   270,   272,   168,   195,   170,
     171,   234,   175,   183,   185,   186,   263,   187,   191,   200,
     192,   193,     8,     8,   327,   273,   207,   208,   222,   224,
     223,   327,   201,   204,   229,   243,   233,   232,   254,   256,
     267,   280,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   259,   265,
     266,   275,   209,   210,   158,   159,   276,   277,   278,   279,
     137,   138,     4,   282,   283,   263,   309,   290,   295,   211,
     212,   308,   312,   318,   -90,   303,   314,   315,   299,   208,
     322,   329,   261,   330,   320,     0,   328,   324,     0,     0,
     263,     0,     0,   328,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
       0,     0,     0,     0,   209,   210,   158,   159,     0,     0,
       0,     0,   137,   138,     4,   208,     0,     0,     0,     0,
       0,   211,   212,     0,     0,     0,   -89,     0,     0,     0,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,     0,     0,     0,     0,
     209,   210,   158,   159,     0,     0,     0,     0,   137,   138,
       4,     0,     0,     0,     0,     0,     0,   211,   212,     0,
       0,     0,   -88,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,    92,     0,     0,     0,     0,     0,
       0,    93,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,   137,   138,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,    95,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,    97,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
      99,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,   126,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,   225,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,   257,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,   271,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
     317,   284,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,     0,     0,
       0,   285,   209,   210,   158,   159,     0,     0,     0,     0,
     137,   138,    25,     0,     0,     0,     0,     0,     0,   211,
     212,   284,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,     0,     0,
       0,     0,   209,   210,   158,   159,     0,     0,     0,     0,
     137,   138,     0,     0,     0,     0,     0,     0,     0,   211,
     212,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,     0,     0,     0,
       0,   209,   210,   158,   159,     0,     0,     0,     0,   137,
     138,   250,     0,     0,     0,     0,     0,     0,   211,   212,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,     0,     0,     0,     0,
     209,   210,   158,   159,     0,     0,     0,     0,   137,   138,
       4,     0,     0,     0,     0,     0,     0,   211,   212,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,   157,     0,     0,     0,     0,     0,
       0,   158,   159,     0,     0,     0,     0,   137,   138,    25,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,     0,     0,     0,     0,
       0,     0,   158,   159,     0,     0,     0,     0,   137,   138
};

static const yytype_int16 yycheck[] =
{
      16,    17,    18,    19,     2,    93,     5,     6,   103,   213,
      56,    44,   283,   234,     0,    47,    47,    12,     0,    41,
      35,    63,   110,    38,    47,    23,    25,    43,    61,   117,
      62,    62,   120,     1,   129,    12,    12,   308,   259,    62,
      46,   217,    12,    11,   229,   140,   141,    10,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    46,    60,    48,    49,    36,    37,
      38,    39,   263,   207,    63,     9,    44,    45,    46,   255,
      57,    57,   267,    60,    60,    53,    54,    57,   222,    56,
      60,    59,    57,    42,   285,    44,    45,    44,    45,    42,
     304,    44,    45,   101,   102,     1,    56,     3,     4,     5,
       6,     7,     8,     9,    10,    49,    59,    56,   206,   214,
     215,    56,   112,   211,   212,   115,    44,    45,    42,   119,
      44,    45,    56,   123,    56,   223,    32,    46,     1,    48,
      49,   131,    65,    66,    40,    59,   234,    51,    52,    58,
      46,    56,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    51,
      52,   259,    35,    36,    37,    38,    39,   265,   273,    31,
      60,    44,    45,    46,    65,    66,    38,    39,    33,    34,
      53,    54,    44,    45,    56,    58,    41,    42,    43,    44,
      45,    50,    50,    46,   220,    48,    49,    42,    59,    44,
      45,   192,   193,    62,    62,    58,   232,    56,    59,    57,
      56,    61,    56,    56,    56,    56,   224,    57,    56,    66,
      57,    57,   230,   231,   322,   233,    57,     1,    57,    57,
      62,   329,    66,    66,    57,    56,    62,    64,    56,    58,
      62,   250,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    61,    61,
      58,    58,    36,    37,    38,    39,    59,    59,    59,    59,
      44,    45,    46,    58,    56,   283,    64,    58,    58,    53,
      54,    56,    59,   309,    58,    11,    62,    59,   285,     1,
      14,    62,   223,   329,   314,    -1,   322,   321,    -1,    -1,
     308,    -1,    -1,   329,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,    45,    46,     1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    -1,    -1,    58,    -1,    -1,    -1,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    -1,
      -1,    -1,    58,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,
      -1,    51,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    44,    45,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    -1,    -1,    -1,    -1,    44,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    -1,    -1,    -1,    -1,
      44,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      -1,    -1,    -1,    -1,    44,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    -1,    -1,    -1,    -1,    44,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    -1,    -1,    -1,    -1,
      44,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    -1,    -1,    -1,    -1,
      44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    -1,    36,    37,    38,    39,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,
      54,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      -1,    36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    38,    39,    -1,    -1,    -1,    -1,    44,    45,    46,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    39,    -1,    -1,    -1,    -1,    44,    45
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    68,    72,     0,    46,    48,    49,    70,    71,    41,
     146,   147,   147,   147,     1,     3,     4,     5,     6,     7,
       8,     9,    10,    32,    40,    46,    73,    87,    90,    91,
      92,    97,    98,    99,   101,   102,   103,   104,   117,   118,
     119,   125,   126,   127,   128,   129,   149,   146,    47,    62,
      50,    50,    56,    44,    45,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    44,    51,   150,    44,   150,    44,   150,    44,
     150,    63,    63,    71,    10,   147,    74,    12,    60,    94,
     107,    79,    80,   100,    81,    82,   106,   107,    75,    76,
     107,   122,    77,    78,    83,    84,    44,   131,   150,    60,
     132,    85,    86,   146,    88,    89,    42,    44,    45,   141,
      71,    71,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    38,    39,
     141,   142,   143,    47,    56,    69,    93,   141,    56,    69,
      57,    56,    69,   105,   141,    56,    69,    31,    38,    39,
     121,   141,   120,    56,    69,    56,    56,    57,   143,   130,
      69,    56,    57,    57,    59,    59,   143,   143,    65,    66,
      66,    66,    65,    66,    66,    44,    61,    57,     1,    36,
      37,    53,    54,    71,    95,    96,   108,   109,   110,   143,
     144,   145,    57,    62,    57,    44,   133,   134,   150,    57,
      72,    72,    64,    62,    61,    33,    34,    41,    42,    43,
     141,   148,   110,    56,    42,    59,   141,    42,    59,   141,
      46,   145,   143,   143,    56,   109,    58,    44,   150,    61,
     110,   121,     1,    71,   116,    61,    58,    62,   133,    58,
      58,    44,   150,    71,   148,    58,    59,    59,    59,    59,
     147,   148,    58,    56,    15,    35,   111,   112,   113,   144,
      58,    42,   135,   141,   133,    58,   143,    47,   116,   111,
      51,    52,     1,    11,   136,   137,   138,   139,    56,    64,
      51,    52,    59,   145,    62,    59,   116,    44,   150,   115,
     137,   114,    14,   140,   140,   123,   124,   141,   150,    62,
     123
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    67,    68,    69,    69,    70,    70,    71,    71,    71,
      72,    72,    72,    74,    73,    75,    73,    73,    76,    73,
      77,    73,    73,    78,    73,    79,    73,    73,    80,    73,
      81,    73,    73,    82,    73,    83,    73,    73,    84,    73,
      73,    85,    73,    86,    73,    73,    73,    88,    87,    89,
      87,    90,    90,    91,    93,    92,    94,    94,    95,    95,
      95,    95,    96,    96,    96,    96,    97,    97,    98,   100,
      99,   101,   101,   102,   102,   102,   103,   105,   104,   106,
     106,   107,   107,   108,   108,   109,   109,   109,   109,   110,
     110,   111,   111,   112,   112,   112,   112,   114,   113,   115,
     113,   116,   116,   116,   116,   117,   117,   118,   120,   119,
     121,   121,   121,   121,   121,   122,   122,   123,   123,   124,
     124,   125,   125,   126,   126,   127,   127,   128,   128,   130,
     129,   131,   129,   132,   132,   133,   133,   134,   134,   134,
     134,   135,   135,   136,   136,   137,   138,   138,   139,   139,
     140,   140,   141,   141,   142,   142,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   144,   144,   144,   145,   145,   145,   145,   146,   146,
     147,   147,   148,   148,   148,   148,   148,   148,   149,   149,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150,   150,   150,   150,
     150,   150,   150,   150,   150,   150,   150
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     3,     4,     0,
       2,     3,     0,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       1,     0,     3,     0,     3,     1,     2,     0,     6,     0,
       6,     2,     2,     1,     0,     6,     2,     0,     3,     3,
       2,     1,     3,     3,     2,     1,     2,     2,     1,     0,
       5,     2,     2,     4,     4,     1,     1,     0,     6,     2,
       0,     1,     1,     1,     3,     2,     2,     1,     2,     1,
       0,     1,     1,     2,     3,     2,     3,     0,     5,     0,
       5,     4,     2,     3,     0,     2,     2,     1,     0,     6,
       3,     1,     1,     1,     1,     2,     0,     3,     1,     1,
       1,     6,     6,     9,     9,     2,     1,     2,     2,     0,
       6,     0,     5,     2,     0,     3,     1,     1,     3,     1,
       0,     1,     1,     1,     0,     2,     1,     3,     1,     0,
       2,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     1,     2,     2,     1,     2,     2,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     6,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YY_LOCATION_PRINT
#  if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

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

#   define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

#  else
#   define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#  endif
# endif /* !defined YY_LOCATION_PRINT */


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
  YYUSE (yyoutput);
  YYUSE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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

  YY_LOCATION_PRINT (yyo, *yylocationp);
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
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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
  case 5: /* file_metadata: ICE_FILE_METADATA_OPEN string_list ICE_FILE_METADATA_CLOSE  */
#line 209 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1838 "src/Slice/Grammar.cpp"
    break;

  case 6: /* file_metadata: ICE_FILE_METADATA_IGNORE string_list ICE_FILE_METADATA_CLOSE  */
#line 213 "src/Slice/Grammar.y"
{
    unit->error("file metadata must appear before any definitions");
    yyval = yyvsp[-1]; // Dummy
}
#line 1847 "src/Slice/Grammar.cpp"
    break;

  case 7: /* local_metadata: ICE_LOCAL_METADATA_OPEN string_list ICE_LOCAL_METADATA_CLOSE  */
#line 223 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1855 "src/Slice/Grammar.cpp"
    break;

  case 8: /* local_metadata: local_metadata ICE_LOCAL_METADATA_OPEN string_list ICE_LOCAL_METADATA_CLOSE  */
#line 227 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata1 = StringListTokPtr::dynamicCast(yyvsp[-3]);
    StringListTokPtr metadata2 = StringListTokPtr::dynamicCast(yyvsp[-1]);
    metadata1->v.splice(metadata1->v.end(), metadata2->v);
    yyval = metadata1;
}
#line 1866 "src/Slice/Grammar.cpp"
    break;

  case 9: /* local_metadata: %empty  */
#line 234 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 1874 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: definitions file_metadata  */
#line 243 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addFileMetaData(metaData->v);
    }
}
#line 1886 "src/Slice/Grammar.cpp"
    break;

  case 11: /* definitions: definitions local_metadata definition  */
#line 251 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1899 "src/Slice/Grammar.cpp"
    break;

  case 13: /* $@1: %empty  */
#line 266 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1907 "src/Slice/Grammar.cpp"
    break;

  case 15: /* $@2: %empty  */
#line 271 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1915 "src/Slice/Grammar.cpp"
    break;

  case 17: /* definition: class_decl  */
#line 276 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 1923 "src/Slice/Grammar.cpp"
    break;

  case 18: /* $@3: %empty  */
#line 280 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1931 "src/Slice/Grammar.cpp"
    break;

  case 20: /* $@4: %empty  */
#line 285 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1939 "src/Slice/Grammar.cpp"
    break;

  case 22: /* definition: interface_decl  */
#line 290 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 1947 "src/Slice/Grammar.cpp"
    break;

  case 23: /* $@5: %empty  */
#line 294 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDefPtr::dynamicCast(yyvsp[0]));
}
#line 1955 "src/Slice/Grammar.cpp"
    break;

  case 25: /* $@6: %empty  */
#line 299 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1963 "src/Slice/Grammar.cpp"
    break;

  case 27: /* definition: exception_decl  */
#line 304 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 1971 "src/Slice/Grammar.cpp"
    break;

  case 28: /* $@7: %empty  */
#line 308 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 1979 "src/Slice/Grammar.cpp"
    break;

  case 30: /* $@8: %empty  */
#line 313 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1987 "src/Slice/Grammar.cpp"
    break;

  case 32: /* definition: struct_decl  */
#line 318 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 1995 "src/Slice/Grammar.cpp"
    break;

  case 33: /* $@9: %empty  */
#line 322 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 2003 "src/Slice/Grammar.cpp"
    break;

  case 35: /* $@10: %empty  */
#line 327 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2011 "src/Slice/Grammar.cpp"
    break;

  case 37: /* definition: sequence_def  */
#line 332 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2019 "src/Slice/Grammar.cpp"
    break;

  case 38: /* $@11: %empty  */
#line 336 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2027 "src/Slice/Grammar.cpp"
    break;

  case 40: /* definition: dictionary_def  */
#line 341 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2035 "src/Slice/Grammar.cpp"
    break;

  case 41: /* $@12: %empty  */
#line 345 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2043 "src/Slice/Grammar.cpp"
    break;

  case 43: /* $@13: %empty  */
#line 350 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2051 "src/Slice/Grammar.cpp"
    break;

  case 45: /* definition: const_def  */
#line 355 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2059 "src/Slice/Grammar.cpp"
    break;

  case 46: /* definition: error ';'  */
#line 359 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2067 "src/Slice/Grammar.cpp"
    break;

  case 47: /* @14: %empty  */
#line 368 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();

    ModulePtr module;
    if (UnitPtr ut = UnitPtr::dynamicCast(cont))
    {
        module = ut->createModule(ident->v);
    }
    else if (ModulePtr mod = ModulePtr::dynamicCast(cont))
    {
        module = mod->createModule(ident->v);
    }

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
#line 2097 "src/Slice/Grammar.cpp"
    break;

  case 48: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
#line 394 "src/Slice/Grammar.y"
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
#line 2113 "src/Slice/Grammar.cpp"
    break;

  case 49: /* @15: %empty  */
#line 406 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);

    // Split the 'scoped identifier' into separate module names.
    vector<string> modules;
    size_t startPos = 0;
    size_t endPos;
    while((endPos = ident->v.find("::", startPos)) != string::npos)
    {
        modules.push_back(ident->v.substr(startPos, (endPos - startPos)));
        startPos = endPos + 2;
    }
    modules.push_back(ident->v.substr(startPos));

    // Create the nested modules.
    ContainerPtr cont = unit->currentContainer();
    ModulePtr parent;
    size_t i = 0;
    if (UnitPtr ut = UnitPtr::dynamicCast(cont))
    {
        parent = ut->createModule(modules[i++]);
        if (parent)
        {
            ut->checkIntroduced(ident->v, parent);
            unit->pushContainer(parent);
        }
    }

    if (i == 0 || parent)
    {
        parent = unit->currentModule();
        for (; i < modules.size(); i++)
        {
            if (ModulePtr module = parent->createModule(modules[i]))
            {
                parent->checkIntroduced(ident->v, module);
                unit->pushContainer(module);
                parent = module;
            }
            else
            {
                // If an error occurs creating one of the modules, back up the entire chain.
                for(; i > 0; i--)
                {
                    unit->popContainer();
                }
                parent = nullptr;
                break;
            }
        }
    }

    yyval = parent;
}
#line 2172 "src/Slice/Grammar.cpp"
    break;

  case 50: /* module_def: ICE_MODULE ICE_SCOPED_IDENTIFIER @15 '{' definitions '}'  */
#line 461 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-4]);
        size_t startPos = 0;
        while((startPos = ident->v.find("::", startPos + 2)) != string::npos)
        {
            unit->popContainer();
        }
        yyval = unit->currentContainer();
        unit->popContainer();
    }
    else
    {
        yyval = 0;
    }
}
#line 2194 "src/Slice/Grammar.cpp"
    break;

  case 51: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 484 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2202 "src/Slice/Grammar.cpp"
    break;

  case 52: /* exception_id: ICE_EXCEPTION keyword  */
#line 488 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2212 "src/Slice/Grammar.cpp"
    break;

  case 53: /* exception_decl: exception_id  */
#line 499 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2221 "src/Slice/Grammar.cpp"
    break;

  case 54: /* @16: %empty  */
#line 509 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-1]);
    ExceptionPtr base = ExceptionPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    ExceptionPtr ex = cont->createException(ident->v, base);
    if(ex)
    {
        cont->checkIntroduced(ident->v, ex);
        unit->pushContainer(ex);
    }
    yyval = ex;
}
#line 2238 "src/Slice/Grammar.cpp"
    break;

  case 55: /* exception_def: exception_id exception_extends @16 '{' data_members '}'  */
#line 522 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2250 "src/Slice/Grammar.cpp"
    break;

  case 56: /* exception_extends: extends scoped_name  */
#line 535 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2262 "src/Slice/Grammar.cpp"
    break;

  case 57: /* exception_extends: %empty  */
#line 543 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2270 "src/Slice/Grammar.cpp"
    break;

  case 58: /* tag: ICE_TAG_OPEN ICE_INTEGER_LITERAL ')'  */
#line 552 "src/Slice/Grammar.y"
{
    IntegerTokPtr i = IntegerTokPtr::dynamicCast(yyvsp[-1]);

    int tag;
    if(i->v < 0 || i->v > INT32_MAX)
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
#line 2292 "src/Slice/Grammar.cpp"
    break;

  case 59: /* tag: ICE_TAG_OPEN scoped_name ')'  */
#line 570 "src/Slice/Grammar.y"
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

    std::int64_t tag = -1;
    EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
    ConstPtr constant = ConstPtr::dynamicCast(cl.front());
    if (constant)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(constant->type());
        if (b && b->isIntegralType())
        {
            tag = IceUtilInternal::strToInt64(constant->value().c_str(), 0, 0);
        }
    }
    else if(enumerator)
    {
        tag = enumerator->value();
    }

    if (tag < 0 || tag > INT32_MAX)
    {
        unit->error("cannot use value of `" + scoped->v + "' as a tag");
    }

    TaggedDefTokPtr m = new TaggedDefTok(static_cast<int>(tag));
    yyval = m;
}
#line 2370 "src/Slice/Grammar.cpp"
    break;

  case 60: /* tag: ICE_TAG_OPEN ')'  */
#line 644 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2380 "src/Slice/Grammar.cpp"
    break;

  case 61: /* tag: ICE_TAG  */
#line 650 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2390 "src/Slice/Grammar.cpp"
    break;

  case 62: /* optional: ICE_OPTIONAL_OPEN ICE_INTEGER_LITERAL ')'  */
#line 661 "src/Slice/Grammar.y"
{
    IntegerTokPtr i = IntegerTokPtr::dynamicCast(yyvsp[-1]);
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }

    int tag;
    if(i->v < 0 || i->v > INT32_MAX)
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
#line 2416 "src/Slice/Grammar.cpp"
    break;

  case 63: /* optional: ICE_OPTIONAL_OPEN scoped_name ')'  */
#line 683 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-1]);
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }

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

    std::int64_t tag = -1;
    EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(cl.front());
    ConstPtr constant = ConstPtr::dynamicCast(cl.front());
    if (constant)
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(constant->type());
        if(b && b->isIntegralType())
        {
            tag = IceUtilInternal::strToInt64(constant->value().c_str(), 0, 0);
        }
    }
    else if(enumerator)
    {
        tag = enumerator->value();
    }

    if (tag < 0 || tag > INT32_MAX)
    {
        unit->error("cannot use value of `" + scoped->v + "' as a tag");
    }

    TaggedDefTokPtr m = new TaggedDefTok(static_cast<int>(tag));
    yyval = m;
}
#line 2498 "src/Slice/Grammar.cpp"
    break;

  case 64: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 761 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2512 "src/Slice/Grammar.cpp"
    break;

  case 65: /* optional: ICE_OPTIONAL  */
#line 771 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2526 "src/Slice/Grammar.cpp"
    break;

  case 66: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 786 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2534 "src/Slice/Grammar.cpp"
    break;

  case 67: /* struct_id: ICE_STRUCT keyword  */
#line 790 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2544 "src/Slice/Grammar.cpp"
    break;

  case 68: /* struct_decl: struct_id  */
#line 801 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2553 "src/Slice/Grammar.cpp"
    break;

  case 69: /* @17: %empty  */
#line 811 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
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
#line 2575 "src/Slice/Grammar.cpp"
    break;

  case 70: /* struct_def: struct_id @17 '{' data_members '}'  */
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
    if (!st->hasDataMembers())
    {
        unit->error("struct `" + st->name() + "' must have at least one member"); // $$ is a dummy
    }
}
#line 2597 "src/Slice/Grammar.cpp"
    break;

  case 71: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 852 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2605 "src/Slice/Grammar.cpp"
    break;

  case 72: /* class_name: ICE_CLASS keyword  */
#line 856 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2615 "src/Slice/Grammar.cpp"
    break;

  case 73: /* class_id: ICE_CLASS ICE_IDENT_OPEN ICE_INTEGER_LITERAL ')'  */
#line 867 "src/Slice/Grammar.y"
{
    IceUtil::Int64 id = IntegerTokPtr::dynamicCast(yyvsp[-1])->v;
    if(id < 0)
    {
        unit->error("invalid compact id for class: id must be a positive integer");
    }
    else if(id > INT32_MAX)
    {
        unit->error("invalid compact id for class: value is out of range");
    }
    else
    {
        string typeId = unit->getTypeId(static_cast<int>(id));
        if(!typeId.empty())
        {
            unit->error("invalid compact id for class: already assigned to class `" + typeId + "'");
        }
    }

    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[-2])->v;
    classId->t = static_cast<int>(id);
    yyval = classId;
}
#line 2644 "src/Slice/Grammar.cpp"
    break;

  case 74: /* class_id: ICE_CLASS ICE_IDENT_OPEN scoped_name ')'  */
#line 892 "src/Slice/Grammar.y"
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
            if(l < 0 || l > INT32_MAX)
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
        if(!typeId.empty())
        {
            unit->error("invalid compact id for class: already assigned to class `" + typeId + "'");
        }
    }

    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[-2])->v;
    classId->t = id;
    yyval = classId;

}
#line 2738 "src/Slice/Grammar.cpp"
    break;

  case 75: /* class_id: class_name  */
#line 982 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2749 "src/Slice/Grammar.cpp"
    break;

  case 76: /* class_decl: class_name  */
#line 994 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2760 "src/Slice/Grammar.cpp"
    break;

  case 77: /* @18: %empty  */
#line 1006 "src/Slice/Grammar.y"
{
    ClassIdTokPtr ident = ClassIdTokPtr::dynamicCast(yyvsp[-1]);
    ModulePtr cont = unit->currentModule();
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
#line 2781 "src/Slice/Grammar.cpp"
    break;

  case 78: /* class_def: class_id class_extends @18 '{' data_members '}'  */
#line 1023 "src/Slice/Grammar.y"
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
#line 2797 "src/Slice/Grammar.cpp"
    break;

  case 79: /* class_extends: extends scoped_name  */
#line 1040 "src/Slice/Grammar.y"
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
#line 2835 "src/Slice/Grammar.cpp"
    break;

  case 80: /* class_extends: %empty  */
#line 1074 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2843 "src/Slice/Grammar.cpp"
    break;

  case 83: /* data_member: member  */
#line 1090 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);

    // Check if the container was created successfully. If it wasn't skip creating the data member and continue parsing.
    if (DataMemberContainerPtr cont = DataMemberContainerPtr::dynamicCast(unit->currentContainer()))
    {
        MemberPtr dm = cont->createDataMember(def->name, def->type, def->isTagged, def->tag);
        unit->currentContainer()->checkIntroduced(def->name, dm);
        if (dm && !def->metadata.empty())
        {
            dm->setMetaData(def->metadata);
        }
    }
}
#line 2862 "src/Slice/Grammar.cpp"
    break;

  case 84: /* data_member: member '=' const_initializer  */
#line 1105 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);

    // Check if the container was created successfully. If it wasn't skip creating the data member and continue parsing.
    if (DataMemberContainerPtr cont = DataMemberContainerPtr::dynamicCast(unit->currentContainer()))
    {
        MemberPtr dm = cont->createDataMember(def->name, def->type, def->isTagged, def->tag, value->v,
                                              value->valueAsString, value->valueAsLiteral);
        unit->currentContainer()->checkIntroduced(def->name, dm);
        if (dm && !def->metadata.empty())
        {
            dm->setMetaData(def->metadata);
        }
    }
}
#line 2883 "src/Slice/Grammar.cpp"
    break;

  case 87: /* data_member_list: data_member  */
#line 1129 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2891 "src/Slice/Grammar.cpp"
    break;

  case 91: /* return_type: tagged_type  */
#line 1146 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2899 "src/Slice/Grammar.cpp"
    break;

  case 92: /* return_type: ICE_VOID  */
#line 1150 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok;
    yyval = m;
}
#line 2908 "src/Slice/Grammar.cpp"
    break;

  case 93: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1160 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer());
    if(interface)
    {
        TaggedDefListTokPtr returnTypes = new TaggedDefListTok();
        returnTypes->v.push_back(returnType);

        if (OperationPtr op = interface->createOperation(name, returnTypes))
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
#line 2938 "src/Slice/Grammar.cpp"
    break;

  case 94: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1186 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer());
    if(interface)
    {
        TaggedDefListTokPtr returnTypes = new TaggedDefListTok();
        returnTypes->v.push_back(returnType);

        if (OperationPtr op = interface->createOperation(name, returnTypes, Operation::Idempotent))
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
#line 2968 "src/Slice/Grammar.cpp"
    break;

  case 95: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1212 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer());
    if(interface)
    {
        TaggedDefListTokPtr returnTypes = new TaggedDefListTok();
        returnTypes->v.push_back(returnType);

        if (OperationPtr op = interface->createOperation(name, returnTypes))
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
#line 2998 "src/Slice/Grammar.cpp"
    break;

  case 96: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1238 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnType = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer());
    if(interface)
    {
        TaggedDefListTokPtr returnTypes = new TaggedDefListTok();
        returnTypes->v.push_back(returnType);

        if (OperationPtr op = interface->createOperation(name, returnTypes, Operation::Idempotent))
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
#line 3028 "src/Slice/Grammar.cpp"
    break;

  case 97: /* @19: %empty  */
#line 1269 "src/Slice/Grammar.y"
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
#line 3044 "src/Slice/Grammar.cpp"
    break;

  case 98: /* operation: operation_preamble parameters ')' @19 throws  */
#line 1281 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3058 "src/Slice/Grammar.cpp"
    break;

  case 99: /* @20: %empty  */
#line 1291 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3070 "src/Slice/Grammar.cpp"
    break;

  case 100: /* operation: operation_preamble error ')' @20 throws  */
#line 1299 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3084 "src/Slice/Grammar.cpp"
    break;

  case 101: /* operation_list: local_metadata operation ';' operation_list  */
#line 1314 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    OperationPtr operation = OperationPtr::dynamicCast(yyvsp[-2]);
    if (operation && !metaData->v.empty())
    {
        operation->setMetaData(metaData->v);
    }
}
#line 3097 "src/Slice/Grammar.cpp"
    break;

  case 102: /* operation_list: local_metadata operation  */
#line 1323 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3105 "src/Slice/Grammar.cpp"
    break;

  case 105: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1334 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3113 "src/Slice/Grammar.cpp"
    break;

  case 106: /* interface_id: ICE_INTERFACE keyword  */
#line 1338 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3123 "src/Slice/Grammar.cpp"
    break;

  case 107: /* interface_decl: interface_id  */
#line 1349 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3135 "src/Slice/Grammar.cpp"
    break;

  case 108: /* @21: %empty  */
#line 1362 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-1]);
    ModulePtr cont = unit->currentModule();
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
#line 3156 "src/Slice/Grammar.cpp"
    break;

  case 109: /* interface_def: interface_id interface_extends @21 '{' operation_list '}'  */
#line 1379 "src/Slice/Grammar.y"
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
#line 3172 "src/Slice/Grammar.cpp"
    break;

  case 110: /* interface_list: scoped_name ',' interface_list  */
#line 1396 "src/Slice/Grammar.y"
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
#line 3211 "src/Slice/Grammar.cpp"
    break;

  case 111: /* interface_list: scoped_name  */
#line 1431 "src/Slice/Grammar.y"
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
#line 3250 "src/Slice/Grammar.cpp"
    break;

  case 112: /* interface_list: ICE_OBJECT  */
#line 1466 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new InterfaceListTok; // Dummy
}
#line 3259 "src/Slice/Grammar.cpp"
    break;

  case 113: /* interface_list: ICE_ANYCLASS  */
#line 1471 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type AnyClass");
    yyval = new ClassListTok; // Dummy
}
#line 3268 "src/Slice/Grammar.cpp"
    break;

  case 114: /* interface_list: ICE_VALUE  */
#line 1476 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3281 "src/Slice/Grammar.cpp"
    break;

  case 115: /* interface_extends: extends interface_list  */
#line 1490 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3289 "src/Slice/Grammar.cpp"
    break;

  case 116: /* interface_extends: %empty  */
#line 1494 "src/Slice/Grammar.y"
{
    yyval = new InterfaceListTok;
}
#line 3297 "src/Slice/Grammar.cpp"
    break;

  case 117: /* exception_list: exception ',' exception_list  */
#line 1503 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3308 "src/Slice/Grammar.cpp"
    break;

  case 118: /* exception_list: exception  */
#line 1510 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3319 "src/Slice/Grammar.cpp"
    break;

  case 119: /* exception: scoped_name  */
#line 1522 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v);
    if(!exception)
    {
        exception = unit->currentModule()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
    }
    cont->checkIntroduced(scoped->v, exception);
    yyval = exception;
}
#line 3335 "src/Slice/Grammar.cpp"
    break;

  case 120: /* exception: keyword  */
#line 1534 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentModule()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3345 "src/Slice/Grammar.cpp"
    break;

  case 121: /* sequence_def: ICE_SEQUENCE '<' local_metadata type '>' ICE_IDENTIFIER  */
#line 1545 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3357 "src/Slice/Grammar.cpp"
    break;

  case 122: /* sequence_def: ICE_SEQUENCE '<' local_metadata type '>' keyword  */
#line 1553 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3370 "src/Slice/Grammar.cpp"
    break;

  case 123: /* dictionary_def: ICE_DICTIONARY '<' local_metadata type ',' local_metadata type '>' ICE_IDENTIFIER  */
#line 1567 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3384 "src/Slice/Grammar.cpp"
    break;

  case 124: /* dictionary_def: ICE_DICTIONARY '<' local_metadata type ',' local_metadata type '>' keyword  */
#line 1577 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3399 "src/Slice/Grammar.cpp"
    break;

  case 125: /* enum_start: ICE_UNCHECKED ICE_ENUM  */
#line 1593 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(true);
}
#line 3407 "src/Slice/Grammar.cpp"
    break;

  case 126: /* enum_start: ICE_ENUM  */
#line 1597 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(false);
}
#line 3415 "src/Slice/Grammar.cpp"
    break;

  case 127: /* enum_id: enum_start ICE_IDENTIFIER  */
#line 1606 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[-1])->v;
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    EnumPtr en = cont->createEnum(ident->v, unchecked);
    if (en)
    {
        cont->checkIntroduced(ident->v, en);
    }
    else
    {
        en = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
    }
    yyval = en;
}
#line 3435 "src/Slice/Grammar.cpp"
    break;

  case 128: /* enum_id: enum_start keyword  */
#line 1622 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[-1])->v;
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
}
#line 3447 "src/Slice/Grammar.cpp"
    break;

  case 129: /* @22: %empty  */
#line 1635 "src/Slice/Grammar.y"
{
    EnumPtr en = EnumPtr::dynamicCast(yyvsp[-1]);
    en->initUnderlying(TypePtr::dynamicCast(yyvsp[0]));
    unit->pushContainer(en);
    yyval = en;
}
#line 3458 "src/Slice/Grammar.cpp"
    break;

  case 130: /* enum_def: enum_id enum_underlying @22 '{' enumerator_list '}'  */
#line 1642 "src/Slice/Grammar.y"
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
#line 3476 "src/Slice/Grammar.cpp"
    break;

  case 131: /* @23: %empty  */
#line 1657 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[0])->v;
    unit->error("missing enumeration name");
    ModulePtr cont = unit->currentModule();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3489 "src/Slice/Grammar.cpp"
    break;

  case 132: /* enum_def: enum_start @23 '{' enumerator_list '}'  */
#line 1666 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3498 "src/Slice/Grammar.cpp"
    break;

  case 133: /* enum_underlying: ':' type  */
#line 1676 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3506 "src/Slice/Grammar.cpp"
    break;

  case 134: /* enum_underlying: %empty  */
#line 1680 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 3514 "src/Slice/Grammar.cpp"
    break;

  case 135: /* enumerator_list: enumerator ',' enumerator_list  */
#line 1689 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 3524 "src/Slice/Grammar.cpp"
    break;

  case 137: /* enumerator: ICE_IDENTIFIER  */
#line 1701 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    EnumPtr cont = EnumPtr::dynamicCast(unit->currentContainer());
    EnumeratorPtr en = cont->createEnumerator(ident->v);
    if(en)
    {
        ens->v.push_front(en);
    }
    yyval = ens;
}
#line 3540 "src/Slice/Grammar.cpp"
    break;

  case 138: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1713 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    EnumPtr cont = EnumPtr::dynamicCast(unit->currentContainer());
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    if (intVal)
    {
       EnumeratorPtr en = cont->createEnumerator(ident->v, intVal->v);
       ens->v.push_front(en);
    }
    yyval = ens;
}
#line 3557 "src/Slice/Grammar.cpp"
    break;

  case 139: /* enumerator: keyword  */
#line 1726 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 3568 "src/Slice/Grammar.cpp"
    break;

  case 140: /* enumerator: %empty  */
#line 1733 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 3577 "src/Slice/Grammar.cpp"
    break;

  case 141: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1743 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3585 "src/Slice/Grammar.cpp"
    break;

  case 142: /* enumerator_initializer: scoped_name  */
#line 1747 "src/Slice/Grammar.y"
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
#line 3622 "src/Slice/Grammar.cpp"
    break;

  case 143: /* out_qualifier: ICE_OUT  */
#line 1785 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(true);
}
#line 3630 "src/Slice/Grammar.cpp"
    break;

  case 144: /* out_qualifier: %empty  */
#line 1789 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(false);
}
#line 3638 "src/Slice/Grammar.cpp"
    break;

  case 145: /* parameter: out_qualifier member  */
#line 1798 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-1]);
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);

    if (OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer()))
    {
        MemberPtr param = op->createParameter(def->name, def->type, isOutParam->v, def->isTagged, def->tag);
        unit->currentContainer()->checkIntroduced(def->name, param);
        if(param && !def->metadata.empty())
        {
            param->setMetaData(def->metadata);
        }
    }
}
#line 3657 "src/Slice/Grammar.cpp"
    break;

  case 150: /* throws: ICE_THROWS exception_list  */
#line 1831 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3665 "src/Slice/Grammar.cpp"
    break;

  case 151: /* throws: %empty  */
#line 1835 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 3673 "src/Slice/Grammar.cpp"
    break;

  case 154: /* builtin: ICE_BOOL  */
#line 1850 "src/Slice/Grammar.y"
           {}
#line 3679 "src/Slice/Grammar.cpp"
    break;

  case 155: /* builtin: ICE_BYTE  */
#line 1851 "src/Slice/Grammar.y"
           {}
#line 3685 "src/Slice/Grammar.cpp"
    break;

  case 156: /* builtin: ICE_SHORT  */
#line 1852 "src/Slice/Grammar.y"
            {}
#line 3691 "src/Slice/Grammar.cpp"
    break;

  case 157: /* builtin: ICE_USHORT  */
#line 1853 "src/Slice/Grammar.y"
             {}
#line 3697 "src/Slice/Grammar.cpp"
    break;

  case 158: /* builtin: ICE_INT  */
#line 1854 "src/Slice/Grammar.y"
          {}
#line 3703 "src/Slice/Grammar.cpp"
    break;

  case 159: /* builtin: ICE_UINT  */
#line 1855 "src/Slice/Grammar.y"
           {}
#line 3709 "src/Slice/Grammar.cpp"
    break;

  case 160: /* builtin: ICE_VARINT  */
#line 1856 "src/Slice/Grammar.y"
             {}
#line 3715 "src/Slice/Grammar.cpp"
    break;

  case 161: /* builtin: ICE_VARUINT  */
#line 1857 "src/Slice/Grammar.y"
              {}
#line 3721 "src/Slice/Grammar.cpp"
    break;

  case 162: /* builtin: ICE_LONG  */
#line 1858 "src/Slice/Grammar.y"
           {}
#line 3727 "src/Slice/Grammar.cpp"
    break;

  case 163: /* builtin: ICE_ULONG  */
#line 1859 "src/Slice/Grammar.y"
            {}
#line 3733 "src/Slice/Grammar.cpp"
    break;

  case 164: /* builtin: ICE_VARLONG  */
#line 1860 "src/Slice/Grammar.y"
              {}
#line 3739 "src/Slice/Grammar.cpp"
    break;

  case 165: /* builtin: ICE_VARULONG  */
#line 1861 "src/Slice/Grammar.y"
               {}
#line 3745 "src/Slice/Grammar.cpp"
    break;

  case 166: /* builtin: ICE_FLOAT  */
#line 1862 "src/Slice/Grammar.y"
            {}
#line 3751 "src/Slice/Grammar.cpp"
    break;

  case 167: /* builtin: ICE_DOUBLE  */
#line 1863 "src/Slice/Grammar.y"
             {}
#line 3757 "src/Slice/Grammar.cpp"
    break;

  case 168: /* builtin: ICE_STRING  */
#line 1864 "src/Slice/Grammar.y"
             {}
#line 3763 "src/Slice/Grammar.cpp"
    break;

  case 169: /* type: ICE_OBJECT '*'  */
#line 1870 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 3771 "src/Slice/Grammar.cpp"
    break;

  case 170: /* type: ICE_OBJECT '?'  */
#line 1874 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 3779 "src/Slice/Grammar.cpp"
    break;

  case 171: /* type: ICE_ANYCLASS '?'  */
#line 1878 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 3787 "src/Slice/Grammar.cpp"
    break;

  case 172: /* type: ICE_VALUE '?'  */
#line 1882 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 3799 "src/Slice/Grammar.cpp"
    break;

  case 173: /* type: builtin '?'  */
#line 1890 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[-1]);
    yyval = unit->optionalBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 3808 "src/Slice/Grammar.cpp"
    break;

  case 174: /* type: ICE_OBJECT  */
#line 1895 "src/Slice/Grammar.y"
{
    if (unit->compatMode())
    {
        yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
    }
    else
    {
        yyval = unit->builtin(Builtin::KindObject);
    }
}
#line 3823 "src/Slice/Grammar.cpp"
    break;

  case 175: /* type: ICE_ANYCLASS  */
#line 1906 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindAnyClass);
}
#line 3831 "src/Slice/Grammar.cpp"
    break;

  case 176: /* type: ICE_VALUE  */
#line 1910 "src/Slice/Grammar.y"
{
    if (unit->compatMode())
    {
        yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
    }
    else
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
        yyval = unit->builtin(Builtin::KindAnyClass);
    }
}
#line 3847 "src/Slice/Grammar.cpp"
    break;

  case 177: /* type: builtin  */
#line 1922 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 3856 "src/Slice/Grammar.cpp"
    break;

  case 178: /* type: scoped_name  */
#line 1927 "src/Slice/Grammar.y"
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
        if (ClassDeclPtr::dynamicCast(types.front()) && unit->compatMode())
        {
            yyval = new Optional(types.front());
        }
        else
        {
            yyval = types.front();
        }
    }
    else
    {
        yyval = 0;
    }
}
#line 3886 "src/Slice/Grammar.cpp"
    break;

  case 179: /* type: scoped_name '*'  */
#line 1953 "src/Slice/Grammar.y"
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
            *p = new Optional(interface);
        }
        yyval = types.front();
    }
    else
    {
        yyval = 0;
    }
}
#line 3922 "src/Slice/Grammar.cpp"
    break;

  case 180: /* type: scoped_name '?'  */
#line 1985 "src/Slice/Grammar.y"
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
#line 3949 "src/Slice/Grammar.cpp"
    break;

  case 181: /* tagged_type: tag type  */
#line 2013 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr taggedDef = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    OptionalPtr type = OptionalPtr::dynamicCast(yyvsp[0]);

    if (!type)
    {
        // Infer the type to be optional so parsing can continue without nullptrs.
        type = new Optional(TypePtr::dynamicCast(yyvsp[0]));
        unit->error("only optional types can be tagged");
    }

    taggedDef->type = type;
    yyval = taggedDef;
}
#line 3968 "src/Slice/Grammar.cpp"
    break;

  case 182: /* tagged_type: optional type  */
#line 2028 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr taggedDef = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    OptionalPtr type = OptionalPtr::dynamicCast(yyvsp[0]);

    if (!type)
    {
        // Infer the type to be optional for backwards compatibility.
        type = new Optional(TypePtr::dynamicCast(yyvsp[0]));
    }

    taggedDef->type = type;
    yyval = taggedDef;
}
#line 3986 "src/Slice/Grammar.cpp"
    break;

  case 183: /* tagged_type: type  */
#line 2042 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr taggedDef = new TaggedDefTok;
    taggedDef->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = taggedDef;
}
#line 3996 "src/Slice/Grammar.cpp"
    break;

  case 184: /* member: tagged_type ICE_IDENTIFIER  */
#line 2053 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    def->name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    checkIdentifier(def->name);
    yyval = def;
}
#line 4007 "src/Slice/Grammar.cpp"
    break;

  case 185: /* member: tagged_type keyword  */
#line 2060 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    def->name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    unit->error("keyword `" + def->name + "' cannot be used as an identifier");
    yyval = def;
}
#line 4018 "src/Slice/Grammar.cpp"
    break;

  case 186: /* member: tagged_type  */
#line 2067 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    def->name = IceUtil::generateUUID(); // Dummy
    unit->error("missing identifier");
    yyval = def;
}
#line 4029 "src/Slice/Grammar.cpp"
    break;

  case 187: /* member: local_metadata member  */
#line 2074 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    def->metadata = StringListTokPtr::dynamicCast(yyvsp[-1])->v;
    yyval = def;
}
#line 4039 "src/Slice/Grammar.cpp"
    break;

  case 188: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 2085 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4049 "src/Slice/Grammar.cpp"
    break;

  case 190: /* string_list: string_list ',' string_literal  */
#line 2097 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4060 "src/Slice/Grammar.cpp"
    break;

  case 191: /* string_list: string_literal  */
#line 2104 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4071 "src/Slice/Grammar.cpp"
    break;

  case 192: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2116 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4084 "src/Slice/Grammar.cpp"
    break;

  case 193: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2125 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4097 "src/Slice/Grammar.cpp"
    break;

  case 194: /* const_initializer: scoped_name  */
#line 2134 "src/Slice/Grammar.y"
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
#line 4141 "src/Slice/Grammar.cpp"
    break;

  case 195: /* const_initializer: ICE_STRING_LITERAL  */
#line 2174 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4152 "src/Slice/Grammar.cpp"
    break;

  case 196: /* const_initializer: ICE_FALSE  */
#line 2181 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4163 "src/Slice/Grammar.cpp"
    break;

  case 197: /* const_initializer: ICE_TRUE  */
#line 2188 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4174 "src/Slice/Grammar.cpp"
    break;

  case 198: /* const_def: ICE_CONST local_metadata type ICE_IDENTIFIER '=' const_initializer  */
#line 2200 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentModule()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4187 "src/Slice/Grammar.cpp"
    break;

  case 199: /* const_def: ICE_CONST local_metadata type '=' const_initializer  */
#line 2209 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentModule()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4200 "src/Slice/Grammar.cpp"
    break;

  case 200: /* keyword: ICE_MODULE  */
#line 2222 "src/Slice/Grammar.y"
             {}
#line 4206 "src/Slice/Grammar.cpp"
    break;

  case 201: /* keyword: ICE_CLASS  */
#line 2223 "src/Slice/Grammar.y"
            {}
#line 4212 "src/Slice/Grammar.cpp"
    break;

  case 202: /* keyword: ICE_INTERFACE  */
#line 2224 "src/Slice/Grammar.y"
                {}
#line 4218 "src/Slice/Grammar.cpp"
    break;

  case 203: /* keyword: ICE_EXCEPTION  */
#line 2225 "src/Slice/Grammar.y"
                {}
#line 4224 "src/Slice/Grammar.cpp"
    break;

  case 204: /* keyword: ICE_STRUCT  */
#line 2226 "src/Slice/Grammar.y"
             {}
#line 4230 "src/Slice/Grammar.cpp"
    break;

  case 205: /* keyword: ICE_SEQUENCE  */
#line 2227 "src/Slice/Grammar.y"
               {}
#line 4236 "src/Slice/Grammar.cpp"
    break;

  case 206: /* keyword: ICE_DICTIONARY  */
#line 2228 "src/Slice/Grammar.y"
                 {}
#line 4242 "src/Slice/Grammar.cpp"
    break;

  case 207: /* keyword: ICE_ENUM  */
#line 2229 "src/Slice/Grammar.y"
           {}
#line 4248 "src/Slice/Grammar.cpp"
    break;

  case 208: /* keyword: ICE_OUT  */
#line 2230 "src/Slice/Grammar.y"
          {}
#line 4254 "src/Slice/Grammar.cpp"
    break;

  case 209: /* keyword: ICE_EXTENDS  */
#line 2231 "src/Slice/Grammar.y"
              {}
#line 4260 "src/Slice/Grammar.cpp"
    break;

  case 210: /* keyword: ICE_IMPLEMENTS  */
#line 2232 "src/Slice/Grammar.y"
                 {}
#line 4266 "src/Slice/Grammar.cpp"
    break;

  case 211: /* keyword: ICE_THROWS  */
#line 2233 "src/Slice/Grammar.y"
             {}
#line 4272 "src/Slice/Grammar.cpp"
    break;

  case 212: /* keyword: ICE_VOID  */
#line 2234 "src/Slice/Grammar.y"
           {}
#line 4278 "src/Slice/Grammar.cpp"
    break;

  case 213: /* keyword: ICE_BOOL  */
#line 2235 "src/Slice/Grammar.y"
           {}
#line 4284 "src/Slice/Grammar.cpp"
    break;

  case 214: /* keyword: ICE_BYTE  */
#line 2236 "src/Slice/Grammar.y"
           {}
#line 4290 "src/Slice/Grammar.cpp"
    break;

  case 215: /* keyword: ICE_SHORT  */
#line 2237 "src/Slice/Grammar.y"
            {}
#line 4296 "src/Slice/Grammar.cpp"
    break;

  case 216: /* keyword: ICE_USHORT  */
#line 2238 "src/Slice/Grammar.y"
             {}
#line 4302 "src/Slice/Grammar.cpp"
    break;

  case 217: /* keyword: ICE_INT  */
#line 2239 "src/Slice/Grammar.y"
          {}
#line 4308 "src/Slice/Grammar.cpp"
    break;

  case 218: /* keyword: ICE_UINT  */
#line 2240 "src/Slice/Grammar.y"
           {}
#line 4314 "src/Slice/Grammar.cpp"
    break;

  case 219: /* keyword: ICE_VARINT  */
#line 2241 "src/Slice/Grammar.y"
             {}
#line 4320 "src/Slice/Grammar.cpp"
    break;

  case 220: /* keyword: ICE_VARUINT  */
#line 2242 "src/Slice/Grammar.y"
              {}
#line 4326 "src/Slice/Grammar.cpp"
    break;

  case 221: /* keyword: ICE_LONG  */
#line 2243 "src/Slice/Grammar.y"
           {}
#line 4332 "src/Slice/Grammar.cpp"
    break;

  case 222: /* keyword: ICE_ULONG  */
#line 2244 "src/Slice/Grammar.y"
            {}
#line 4338 "src/Slice/Grammar.cpp"
    break;

  case 223: /* keyword: ICE_VARLONG  */
#line 2245 "src/Slice/Grammar.y"
              {}
#line 4344 "src/Slice/Grammar.cpp"
    break;

  case 224: /* keyword: ICE_VARULONG  */
#line 2246 "src/Slice/Grammar.y"
               {}
#line 4350 "src/Slice/Grammar.cpp"
    break;

  case 225: /* keyword: ICE_FLOAT  */
#line 2247 "src/Slice/Grammar.y"
            {}
#line 4356 "src/Slice/Grammar.cpp"
    break;

  case 226: /* keyword: ICE_DOUBLE  */
#line 2248 "src/Slice/Grammar.y"
             {}
#line 4362 "src/Slice/Grammar.cpp"
    break;

  case 227: /* keyword: ICE_STRING  */
#line 2249 "src/Slice/Grammar.y"
             {}
#line 4368 "src/Slice/Grammar.cpp"
    break;

  case 228: /* keyword: ICE_OBJECT  */
#line 2250 "src/Slice/Grammar.y"
             {}
#line 4374 "src/Slice/Grammar.cpp"
    break;

  case 229: /* keyword: ICE_CONST  */
#line 2251 "src/Slice/Grammar.y"
            {}
#line 4380 "src/Slice/Grammar.cpp"
    break;

  case 230: /* keyword: ICE_FALSE  */
#line 2252 "src/Slice/Grammar.y"
            {}
#line 4386 "src/Slice/Grammar.cpp"
    break;

  case 231: /* keyword: ICE_TRUE  */
#line 2253 "src/Slice/Grammar.y"
           {}
#line 4392 "src/Slice/Grammar.cpp"
    break;

  case 232: /* keyword: ICE_IDEMPOTENT  */
#line 2254 "src/Slice/Grammar.y"
                 {}
#line 4398 "src/Slice/Grammar.cpp"
    break;

  case 233: /* keyword: ICE_TAG  */
#line 2255 "src/Slice/Grammar.y"
          {}
#line 4404 "src/Slice/Grammar.cpp"
    break;

  case 234: /* keyword: ICE_OPTIONAL  */
#line 2256 "src/Slice/Grammar.y"
               {}
#line 4410 "src/Slice/Grammar.cpp"
    break;

  case 235: /* keyword: ICE_ANYCLASS  */
#line 2257 "src/Slice/Grammar.y"
               {}
#line 4416 "src/Slice/Grammar.cpp"
    break;

  case 236: /* keyword: ICE_VALUE  */
#line 2258 "src/Slice/Grammar.y"
            {}
#line 4422 "src/Slice/Grammar.cpp"
    break;


#line 4426 "src/Slice/Grammar.cpp"

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
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 2261 "src/Slice/Grammar.y"

