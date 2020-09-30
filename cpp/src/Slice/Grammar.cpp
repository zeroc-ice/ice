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
  YYSYMBOL_ICE_FILE_METADATA_CLOSE = 49,   /* ICE_FILE_METADATA_CLOSE  */
  YYSYMBOL_ICE_IDENT_OPEN = 50,            /* ICE_IDENT_OPEN  */
  YYSYMBOL_ICE_KEYWORD_OPEN = 51,          /* ICE_KEYWORD_OPEN  */
  YYSYMBOL_ICE_TAG_OPEN = 52,              /* ICE_TAG_OPEN  */
  YYSYMBOL_ICE_OPTIONAL_OPEN = 53,         /* ICE_OPTIONAL_OPEN  */
  YYSYMBOL_BAD_CHAR = 54,                  /* BAD_CHAR  */
  YYSYMBOL_55_ = 55,                       /* ';'  */
  YYSYMBOL_56_ = 56,                       /* '{'  */
  YYSYMBOL_57_ = 57,                       /* '}'  */
  YYSYMBOL_58_ = 58,                       /* ')'  */
  YYSYMBOL_59_ = 59,                       /* ':'  */
  YYSYMBOL_60_ = 60,                       /* '='  */
  YYSYMBOL_61_ = 61,                       /* ','  */
  YYSYMBOL_62_ = 62,                       /* '('  */
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
  YYSYMBOL_return_tuple = 111,             /* return_tuple  */
  YYSYMBOL_return_type = 112,              /* return_type  */
  YYSYMBOL_operation_preamble = 113,       /* operation_preamble  */
  YYSYMBOL_operation = 114,                /* operation  */
  YYSYMBOL_115_19 = 115,                   /* @19  */
  YYSYMBOL_116_20 = 116,                   /* @20  */
  YYSYMBOL_operation_list = 117,           /* operation_list  */
  YYSYMBOL_interface_id = 118,             /* interface_id  */
  YYSYMBOL_interface_decl = 119,           /* interface_decl  */
  YYSYMBOL_interface_def = 120,            /* interface_def  */
  YYSYMBOL_121_21 = 121,                   /* @21  */
  YYSYMBOL_interface_list = 122,           /* interface_list  */
  YYSYMBOL_interface_extends = 123,        /* interface_extends  */
  YYSYMBOL_exception_list = 124,           /* exception_list  */
  YYSYMBOL_exception = 125,                /* exception  */
  YYSYMBOL_sequence_def = 126,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 127,           /* dictionary_def  */
  YYSYMBOL_enum_start = 128,               /* enum_start  */
  YYSYMBOL_enum_id = 129,                  /* enum_id  */
  YYSYMBOL_enum_def = 130,                 /* enum_def  */
  YYSYMBOL_131_22 = 131,                   /* @22  */
  YYSYMBOL_132_23 = 132,                   /* @23  */
  YYSYMBOL_enum_underlying = 133,          /* enum_underlying  */
  YYSYMBOL_enumerator_list = 134,          /* enumerator_list  */
  YYSYMBOL_enumerator = 135,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 136,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 137,            /* out_qualifier  */
  YYSYMBOL_parameter = 138,                /* parameter  */
  YYSYMBOL_parameter_list = 139,           /* parameter_list  */
  YYSYMBOL_parameters = 140,               /* parameters  */
  YYSYMBOL_throws = 141,                   /* throws  */
  YYSYMBOL_scoped_name = 142,              /* scoped_name  */
  YYSYMBOL_builtin = 143,                  /* builtin  */
  YYSYMBOL_type = 144,                     /* type  */
  YYSYMBOL_tagged_type = 145,              /* tagged_type  */
  YYSYMBOL_member = 146,                   /* member  */
  YYSYMBOL_string_literal = 147,           /* string_literal  */
  YYSYMBOL_string_list = 148,              /* string_list  */
  YYSYMBOL_const_initializer = 149,        /* const_initializer  */
  YYSYMBOL_const_def = 150,                /* const_def  */
  YYSYMBOL_keyword = 151                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 61 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 362 "src/Slice/Grammar.cpp"

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
#define YYLAST   1065

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  85
/* YYNRULES -- Number of rules.  */
#define YYNRULES  239
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  337

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   309


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
      62,    58,    65,     2,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    59,    55,
      63,    60,    64,    66,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    56,     2,    57,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   194,   194,   200,   201,   207,   216,   220,   227,   236,
     244,   253,   260,   259,   265,   264,   269,   274,   273,   279,
     278,   283,   288,   287,   293,   292,   297,   302,   301,   307,
     306,   311,   316,   315,   321,   320,   325,   330,   329,   334,
     339,   338,   344,   343,   348,   352,   362,   361,   400,   399,
     477,   481,   492,   503,   502,   528,   536,   545,   563,   637,
     643,   654,   676,   754,   764,   779,   783,   794,   805,   804,
     845,   849,   860,   885,   975,   987,  1000,   999,  1033,  1067,
    1076,  1077,  1083,  1098,  1120,  1121,  1122,  1126,  1132,  1133,
    1139,  1151,  1167,  1182,  1191,  1196,  1205,  1240,  1275,  1309,
    1349,  1348,  1371,  1370,  1393,  1411,  1415,  1416,  1422,  1426,
    1437,  1451,  1450,  1484,  1519,  1554,  1559,  1564,  1578,  1582,
    1591,  1598,  1610,  1622,  1633,  1641,  1655,  1665,  1681,  1685,
    1694,  1710,  1724,  1723,  1746,  1745,  1764,  1768,  1777,  1783,
    1789,  1801,  1814,  1821,  1831,  1835,  1873,  1877,  1886,  1905,
    1906,  1912,  1913,  1919,  1923,  1932,  1933,  1939,  1940,  1941,
    1942,  1943,  1944,  1945,  1946,  1947,  1948,  1949,  1950,  1951,
    1952,  1953,  1958,  1962,  1966,  1970,  1978,  1983,  1994,  1998,
    2010,  2015,  2041,  2073,  2101,  2116,  2130,  2141,  2153,  2164,
    2175,  2186,  2192,  2198,  2205,  2217,  2226,  2235,  2275,  2282,
    2289,  2301,  2310,  2324,  2325,  2326,  2327,  2328,  2329,  2330,
    2331,  2332,  2333,  2334,  2335,  2336,  2337,  2338,  2339,  2340,
    2341,  2342,  2343,  2344,  2345,  2346,  2347,  2348,  2349,  2350,
    2351,  2352,  2353,  2354,  2355,  2356,  2357,  2358,  2359,  2360
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
  "ICE_FILE_METADATA_CLOSE", "ICE_IDENT_OPEN", "ICE_KEYWORD_OPEN",
  "ICE_TAG_OPEN", "ICE_OPTIONAL_OPEN", "BAD_CHAR", "';'", "'{'", "'}'",
  "')'", "':'", "'='", "','", "'('", "'<'", "'>'", "'*'", "'?'", "$accept",
  "start", "opt_semicolon", "file_metadata", "local_metadata",
  "definitions", "definition", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6",
  "$@7", "$@8", "$@9", "$@10", "$@11", "$@12", "$@13", "module_def", "@14",
  "@15", "exception_id", "exception_decl", "exception_def", "@16",
  "exception_extends", "tag", "optional", "struct_id", "struct_decl",
  "struct_def", "@17", "class_name", "class_id", "class_decl", "class_def",
  "@18", "class_extends", "extends", "data_member", "data_member_list",
  "data_members", "return_tuple", "return_type", "operation_preamble",
  "operation", "@19", "@20", "operation_list", "interface_id",
  "interface_decl", "interface_def", "@21", "interface_list",
  "interface_extends", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_start", "enum_id", "enum_def", "@22", "@23",
  "enum_underlying", "enumerator_list", "enumerator",
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
     305,   306,   307,   308,   309,    59,   123,   125,    41,    58,
      61,    44,    40,    60,    62,    42,    63
};
#endif

#define YYPACT_NINF (-277)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-153)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -277,    13,    18,  -277,   -26,   -26,  -277,   201,   -26,  -277,
      49,    38,   -34,    -1,   403,   590,   632,   674,    41,    54,
    -277,    -9,    55,   -26,  -277,  -277,    16,    36,  -277,    73,
      72,  -277,    23,    14,    79,  -277,    24,    88,  -277,    95,
     115,   716,   125,  -277,   132,  -277,  -277,   -26,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
     -25,  -277,  -277,  -277,  -277,  -277,  -277,  -277,    -9,    -9,
     989,  -277,    61,   144,  -277,  -277,  -277,    71,   145,   144,
     156,   146,   144,  -277,    71,   158,   144,   100,  -277,   159,
     144,   163,   164,  -277,   165,  -277,  1020,  -277,   144,   167,
    -277,   175,   178,   139,  -277,  -277,   162,   989,   989,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,    83,   169,   171,   103,   172,   -30,
    -277,  -277,  -277,   184,  -277,  -277,  -277,   235,  -277,  -277,
     186,  -277,  -277,  -277,  -277,  -277,  -277,  -277,   137,   188,
    -277,  -277,  -277,  -277,   758,  -277,   189,  -277,  -277,  -277,
    -277,  -277,  -277,   182,   206,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,   179,   183,   235,   194,  -277,  -277,    48,    53,
     913,  1020,  1020,   213,   306,   212,  -277,   800,   210,   235,
     100,    32,   215,   219,   216,  -277,   758,    84,    94,   842,
      -9,   183,  -277,  -277,  -277,  -277,  -277,  -277,  -277,   221,
    -277,   225,  -277,   226,   227,  -277,   228,   -26,  -277,  -277,
    -277,  -277,   348,  -277,  -277,  -277,   183,   232,  -277,   236,
     439,   233,   141,  -277,   758,   237,  -277,  -277,  -277,  -277,
     989,  -277,  -277,  -277,  -277,  -277,  -277,    67,  -277,  -277,
      32,  -277,   487,    28,   140,   136,   238,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,   231,  -277,  -277,   142,  -277,  -277,
      75,   951,  -277,  -277,   239,   951,  -277,   240,   241,    32,
     884,  -277,  -277,  -277,   287,  -277,  -277,  -277,   287,  -277,
    -277,  -277,  -277,   951,   286,  -277,   286,  -277,   547,  -277,
    -277,  -277,   242,  -277,  -277,   547,  -277
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      11,     0,     8,     1,     0,     0,     9,     0,   192,   194,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     129,     8,     0,     0,    10,    12,    52,    26,    27,    67,
      31,    32,    75,    79,    16,    17,   110,    21,    22,    36,
      39,   134,   137,    40,    44,   191,     6,     0,     5,    45,
      46,    48,   203,   204,   205,   206,   207,   208,   209,   210,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,    70,
       0,    71,   108,   109,    50,    51,    65,    66,     8,     8,
       0,   128,     0,     4,    80,    81,    53,     0,     0,     4,
       0,     0,     4,    76,     0,     0,     4,     0,   111,     0,
       4,     0,     0,   130,     0,   131,     0,   132,     4,     0,
     193,     0,     0,     0,   155,   156,     0,     0,     0,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   177,   178,   179,   181,   180,     0,
       7,     3,    13,     0,    55,    25,    28,     0,    30,    33,
       0,    78,    15,    18,   115,   116,   117,   118,   114,     0,
      20,    23,    35,    38,   143,   136,     0,    41,    43,    11,
      11,    72,    73,     0,     0,   172,   173,   174,   175,   182,
     183,   176,     0,     0,     0,     0,    60,    64,     0,     0,
       0,     0,     0,    86,     0,     0,   186,   189,    82,     0,
       0,     0,   140,     0,   139,   142,   143,     8,     8,     0,
       8,     0,   199,   200,   198,   195,   196,   197,   202,     0,
      85,     0,    59,     0,     0,    63,     0,     0,   190,   184,
     185,    84,     0,    69,   187,   188,     0,     0,   113,     0,
       0,     0,     0,   135,   143,     0,    47,    49,   124,   125,
       0,   201,    54,    57,    58,    61,    62,     0,    83,    77,
       0,    95,     0,   147,     0,     0,   105,    92,   112,   144,
     141,   145,   138,   133,     0,     6,   106,     0,   146,    94,
       0,     0,    96,    98,     0,     0,   149,   151,     0,     0,
       0,    97,    99,    93,   147,    90,   102,   148,   147,   100,
     104,   126,   127,     0,   154,   150,   154,    91,     0,   103,
     101,   153,   121,   122,   123,     0,   120
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -277,  -277,    -7,  -277,     2,     6,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,  -277,
     143,  -277,  -140,  -194,  -277,    20,  -277,  -277,  -277,  -277,
    -269,  -277,  -277,  -277,  -277,    85,  -277,   -31,  -277,  -277,
    -277,  -277,  -277,  -277,  -277,  -277,  -277,  -138,  -277,  -277,
    -276,   -12,  -277,  -277,   -18,   -85,  -277,   -92,  -251,  -198,
      34,     1,  -215,  -277,   -14
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   162,     6,   210,     2,    24,   103,   115,   116,
     119,   120,   108,   109,   111,   112,   121,   122,   128,   129,
      25,   131,   132,    26,    27,    28,   163,   106,   211,   212,
      29,    30,    31,   110,    32,    33,    34,    35,   170,   113,
     107,   213,   214,   215,   300,   284,   285,   286,   326,   324,
     261,    36,    37,    38,   179,   177,   118,   331,   332,    39,
      40,    41,    42,    43,   186,   124,   127,   223,   224,   290,
     305,   306,   307,   308,   329,   157,   158,   216,   217,   218,
       9,    10,   238,    44,   225
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      91,    93,    95,    97,     7,   136,    11,   301,   159,   287,
     239,   296,   248,     3,   202,     8,   271,   133,    -2,   134,
     135,    49,   164,   100,   102,   257,   104,   125,   104,   171,
     203,   287,   178,   259,   185,   -74,   104,     4,   323,   298,
     320,   278,    45,    50,    51,   193,   194,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,     4,   101,     5,    -8,    -8,    -8,
      -8,    -8,   -56,   105,   252,   105,    -8,    -8,     4,   -74,
    -119,   130,   -74,   105,    -8,    -8,   299,    48,   265,  -107,
     241,   -24,   134,   135,    -8,   244,    46,   134,   135,    47,
     137,   138,   166,   315,    98,   169,   242,   317,   160,   173,
      47,   245,   252,   181,   295,   134,   135,    99,   237,   249,
     250,   187,    47,   243,   246,   327,   292,   -29,    47,   -68,
       4,   174,     5,   313,   -14,   178,   314,   304,   175,   176,
       4,   266,     5,   -19,   134,   135,   237,   298,   195,   196,
     -34,   267,  -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,
    -147,  -147,  -147,  -147,  -147,  -147,  -147,  -147,   199,   200,
     -37,   237,  -147,  -147,  -147,  -147,   114,   291,   294,   117,
    -147,  -147,  -147,   289,   126,   134,   135,   -42,  -147,  -147,
     302,   303,   311,   312,  -152,   227,   228,   191,   220,   161,
     165,   168,    12,   255,    13,    14,    15,    16,    17,    18,
      19,    20,   167,   172,   180,   269,   232,   233,   182,   183,
     192,   184,   188,   260,   234,   235,   236,   134,   135,     7,
       7,   189,   270,    21,   190,   197,   205,   198,   201,   231,
     204,    22,   219,   333,   221,   226,   229,    23,   277,   240,
     333,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   230,   251,   253,
     256,   206,   207,   155,   156,   262,   263,   264,   272,   134,
     135,     4,   260,   273,   274,   275,   276,   208,   209,   279,
     288,   280,   -89,   309,   293,   310,   322,   316,   298,   319,
     328,   318,   297,   335,   336,   258,   325,   205,   330,     0,
       0,   260,     0,     0,   334,     0,     0,     0,     0,     0,
       0,   334,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,     0,     0,
       0,     0,   206,   207,   155,   156,     0,     0,     0,   205,
     134,   135,     4,     0,     0,     0,     0,     0,   208,   209,
       0,     0,     0,   -88,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,     0,     0,     0,   206,   207,   155,   156,     0,     0,
       0,     0,   134,   135,     4,     0,     0,     0,     0,     0,
     208,   209,     0,     0,     0,   -87,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,     0,     0,     0,     0,    89,     0,     0,
       0,     0,     0,    90,   281,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,     0,     0,   282,   206,   207,   155,   156,     0,
       0,     0,     0,   134,   135,    23,     0,     0,     0,     0,
       0,   208,   209,     0,     0,     0,     0,     0,     0,     0,
       0,   283,   281,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,     0,
       0,     0,     0,   206,   207,   155,   156,     0,     0,     0,
       0,   134,   135,     0,     0,     0,     0,     0,     0,   208,
     209,     0,     0,     0,     0,     0,     0,     0,     0,   283,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
       0,   134,   135,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
       0,     0,     0,     0,    92,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     0,     0,     0,     0,    94,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,     0,     0,     0,     0,    96,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,     0,     0,     0,     0,
     123,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,     0,     0,
       0,     0,   222,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
       0,     0,     0,     0,   254,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     0,     0,     0,     0,   268,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,     0,     0,     0,     0,   321,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,   153,   154,     0,     0,     0,     0,   206,
     207,   155,   156,     0,     0,     0,     0,   134,   135,   247,
       0,     0,     0,     0,     0,   208,   209,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,     0,     0,     0,     0,   206,   207,   155,
     156,     0,     0,     0,     0,   134,   135,     4,     0,     0,
       0,     0,     0,   208,   209,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,     0,     0,     0,     0,     0,     0,   155,   156,     0,
       0,     0,     0,   134,   135,    23,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,     0,     0,     0,     0,     0,     0,   155,   156,
       0,     0,     0,     0,   134,   135
};

static const yytype_int16 yycheck[] =
{
      14,    15,    16,    17,     2,    90,     5,   283,   100,   260,
     204,   280,   210,     0,    44,    41,   231,    42,     0,    44,
      45,    55,   107,    21,    23,   219,    12,    41,    12,   114,
      60,   282,   117,     1,   126,    12,    12,    46,   314,    11,
     309,   256,     8,    44,    45,   137,   138,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    46,    10,    48,    35,    36,    37,
      38,    39,    56,    59,   214,    59,    44,    45,    46,    56,
      56,    47,    59,    59,    52,    53,    58,    49,   226,    57,
      42,    55,    44,    45,    62,    42,    47,    44,    45,    61,
      98,    99,   109,   301,    63,   112,    58,   305,    47,   116,
      61,    58,   252,   120,    47,    44,    45,    63,   203,   211,
     212,   128,    61,   208,   209,   323,   264,    55,    61,    56,
      46,    31,    48,    58,    55,   220,    61,     1,    38,    39,
      46,    57,    48,    55,    44,    45,   231,    11,    65,    66,
      55,    57,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    65,    66,
      55,   256,    36,    37,    38,    39,    33,   262,   270,    36,
      44,    45,    46,    42,    59,    44,    45,    55,    52,    53,
      50,    51,    50,    51,    58,   189,   190,    58,    61,    55,
      55,    55,     1,   217,     3,     4,     5,     6,     7,     8,
       9,    10,    56,    55,    55,   229,    33,    34,    55,    55,
      58,    56,    55,   221,    41,    42,    43,    44,    45,   227,
     228,    56,   230,    32,    56,    66,     1,    66,    66,    60,
      56,    40,    56,   328,    56,    56,    64,    46,   247,    55,
     335,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    61,    55,    57,
      60,    36,    37,    38,    39,    60,    57,    61,    57,    44,
      45,    46,   280,    58,    58,    58,    58,    52,    53,    57,
      57,    55,    57,    55,    57,    64,   310,    58,    11,    58,
      14,    61,   282,    61,   335,   220,   318,     1,   326,    -1,
      -1,   309,    -1,    -1,   328,    -1,    -1,    -1,    -1,    -1,
      -1,   335,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    -1,    36,    37,    38,    39,    -1,    -1,    -1,     1,
      44,    45,    46,    -1,    -1,    -1,    -1,    -1,    52,    53,
      -1,    -1,    -1,    57,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
      52,    53,    -1,    -1,    -1,    57,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    50,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    35,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    44,    45,     3,     4,     5,     6,     7,     8,     9,
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
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    44,    45,    46,
      -1,    -1,    -1,    -1,    -1,    52,    53,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    36,    37,    38,
      39,    -1,    -1,    -1,    -1,    44,    45,    46,    -1,    -1,
      -1,    -1,    -1,    52,    53,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    -1,
      -1,    -1,    -1,    44,    45,    46,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,
      -1,    -1,    -1,    -1,    44,    45
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    68,    72,     0,    46,    48,    70,    71,    41,   147,
     148,   148,     1,     3,     4,     5,     6,     7,     8,     9,
      10,    32,    40,    46,    73,    87,    90,    91,    92,    97,
      98,    99,   101,   102,   103,   104,   118,   119,   120,   126,
     127,   128,   129,   130,   150,   147,    47,    61,    49,    55,
      44,    45,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    44,
      50,   151,    44,   151,    44,   151,    44,   151,    63,    63,
      71,    10,   148,    74,    12,    59,    94,   107,    79,    80,
     100,    81,    82,   106,   107,    75,    76,   107,   123,    77,
      78,    83,    84,    44,   132,   151,    59,   133,    85,    86,
     147,    88,    89,    42,    44,    45,   142,    71,    71,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    38,    39,   142,   143,   144,
      47,    55,    69,    93,   142,    55,    69,    56,    55,    69,
     105,   142,    55,    69,    31,    38,    39,   122,   142,   121,
      55,    69,    55,    55,    56,   144,   131,    69,    55,    56,
      56,    58,    58,   144,   144,    65,    66,    66,    66,    65,
      66,    66,    44,    60,    56,     1,    36,    37,    52,    53,
      71,    95,    96,   108,   109,   110,   144,   145,   146,    56,
      61,    56,    44,   134,   135,   151,    56,    72,    72,    64,
      61,    60,    33,    34,    41,    42,    43,   142,   149,   110,
      55,    42,    58,   142,    42,    58,   142,    46,   146,   144,
     144,    55,   109,    57,    44,   151,    60,   110,   122,     1,
      71,   117,    60,    57,    61,   134,    57,    57,    44,   151,
      71,   149,    57,    58,    58,    58,    58,   148,   149,    57,
      55,    15,    35,    62,   112,   113,   114,   145,    57,    42,
     136,   142,   134,    57,   144,    47,   117,   112,    11,    58,
     111,   137,    50,    51,     1,   137,   138,   139,   140,    55,
      64,    50,    51,    58,    61,   146,    58,   146,    61,    58,
     117,    44,   151,   137,   116,   138,   115,   146,    14,   141,
     141,   124,   125,   142,   151,    61,   124
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    67,    68,    69,    69,    70,    71,    71,    71,    72,
      72,    72,    74,    73,    75,    73,    73,    76,    73,    77,
      73,    73,    78,    73,    79,    73,    73,    80,    73,    81,
      73,    73,    82,    73,    83,    73,    73,    84,    73,    73,
      85,    73,    86,    73,    73,    73,    88,    87,    89,    87,
      90,    90,    91,    93,    92,    94,    94,    95,    95,    95,
      95,    96,    96,    96,    96,    97,    97,    98,   100,    99,
     101,   101,   102,   102,   102,   103,   105,   104,   106,   106,
     107,   107,   108,   108,   109,   109,   109,   109,   110,   110,
     111,   111,   112,   112,   112,   112,   113,   113,   113,   113,
     115,   114,   116,   114,   117,   117,   117,   117,   118,   118,
     119,   121,   120,   122,   122,   122,   122,   122,   123,   123,
     124,   124,   125,   125,   126,   126,   127,   127,   128,   128,
     129,   129,   131,   130,   132,   130,   133,   133,   134,   134,
     135,   135,   135,   135,   136,   136,   137,   137,   138,   139,
     139,   140,   140,   141,   141,   142,   142,   143,   143,   143,
     143,   143,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   144,   144,   144,   144,   144,   144,   144,   144,
     144,   144,   144,   144,   145,   145,   145,   146,   146,   146,
     146,   147,   147,   148,   148,   149,   149,   149,   149,   149,
     149,   150,   150,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     4,     0,     2,
       3,     0,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     1,
       0,     3,     0,     3,     1,     2,     0,     6,     0,     6,
       2,     2,     1,     0,     6,     2,     0,     3,     3,     2,
       1,     3,     3,     2,     1,     2,     2,     1,     0,     5,
       2,     2,     4,     4,     1,     1,     0,     6,     2,     0,
       1,     1,     1,     3,     2,     2,     1,     2,     1,     0,
       2,     4,     1,     3,     2,     1,     2,     3,     2,     3,
       0,     5,     0,     5,     4,     2,     3,     0,     2,     2,
       1,     0,     6,     3,     1,     1,     1,     1,     2,     0,
       3,     1,     1,     1,     6,     6,     9,     9,     2,     1,
       2,     2,     0,     6,     0,     5,     2,     0,     3,     1,
       1,     3,     1,     0,     1,     1,     1,     0,     2,     1,
       3,     1,     0,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     2,     2,     2,     2,     1,     2,     2,     1,
       2,     2,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     6,     5,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
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
#line 208 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1842 "src/Slice/Grammar.cpp"
    break;

  case 6: /* local_metadata: ICE_LOCAL_METADATA_OPEN string_list ICE_LOCAL_METADATA_CLOSE  */
#line 217 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1850 "src/Slice/Grammar.cpp"
    break;

  case 7: /* local_metadata: local_metadata ICE_LOCAL_METADATA_OPEN string_list ICE_LOCAL_METADATA_CLOSE  */
#line 221 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata1 = StringListTokPtr::dynamicCast(yyvsp[-3]);
    StringListTokPtr metadata2 = StringListTokPtr::dynamicCast(yyvsp[-1]);
    metadata1->v.splice(metadata1->v.end(), metadata2->v);
    yyval = metadata1;
}
#line 1861 "src/Slice/Grammar.cpp"
    break;

  case 8: /* local_metadata: %empty  */
#line 228 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 1869 "src/Slice/Grammar.cpp"
    break;

  case 9: /* definitions: definitions file_metadata  */
#line 237 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addFileMetaData(metaData->v);
    }
}
#line 1881 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: definitions local_metadata definition  */
#line 245 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 1894 "src/Slice/Grammar.cpp"
    break;

  case 12: /* $@1: %empty  */
#line 260 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1902 "src/Slice/Grammar.cpp"
    break;

  case 14: /* $@2: %empty  */
#line 265 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1910 "src/Slice/Grammar.cpp"
    break;

  case 16: /* definition: class_decl  */
#line 270 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 1918 "src/Slice/Grammar.cpp"
    break;

  case 17: /* $@3: %empty  */
#line 274 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1926 "src/Slice/Grammar.cpp"
    break;

  case 19: /* $@4: %empty  */
#line 279 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1934 "src/Slice/Grammar.cpp"
    break;

  case 21: /* definition: interface_decl  */
#line 284 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 1942 "src/Slice/Grammar.cpp"
    break;

  case 22: /* $@5: %empty  */
#line 288 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDefPtr::dynamicCast(yyvsp[0]));
}
#line 1950 "src/Slice/Grammar.cpp"
    break;

  case 24: /* $@6: %empty  */
#line 293 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1958 "src/Slice/Grammar.cpp"
    break;

  case 26: /* definition: exception_decl  */
#line 298 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 1966 "src/Slice/Grammar.cpp"
    break;

  case 27: /* $@7: %empty  */
#line 302 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 1974 "src/Slice/Grammar.cpp"
    break;

  case 29: /* $@8: %empty  */
#line 307 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1982 "src/Slice/Grammar.cpp"
    break;

  case 31: /* definition: struct_decl  */
#line 312 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 1990 "src/Slice/Grammar.cpp"
    break;

  case 32: /* $@9: %empty  */
#line 316 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 1998 "src/Slice/Grammar.cpp"
    break;

  case 34: /* $@10: %empty  */
#line 321 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2006 "src/Slice/Grammar.cpp"
    break;

  case 36: /* definition: sequence_def  */
#line 326 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2014 "src/Slice/Grammar.cpp"
    break;

  case 37: /* $@11: %empty  */
#line 330 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2022 "src/Slice/Grammar.cpp"
    break;

  case 39: /* definition: dictionary_def  */
#line 335 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2030 "src/Slice/Grammar.cpp"
    break;

  case 40: /* $@12: %empty  */
#line 339 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2038 "src/Slice/Grammar.cpp"
    break;

  case 42: /* $@13: %empty  */
#line 344 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2046 "src/Slice/Grammar.cpp"
    break;

  case 44: /* definition: const_def  */
#line 349 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2054 "src/Slice/Grammar.cpp"
    break;

  case 45: /* definition: error ';'  */
#line 353 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2062 "src/Slice/Grammar.cpp"
    break;

  case 46: /* @14: %empty  */
#line 362 "src/Slice/Grammar.y"
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
#line 2092 "src/Slice/Grammar.cpp"
    break;

  case 47: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
#line 388 "src/Slice/Grammar.y"
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
#line 2108 "src/Slice/Grammar.cpp"
    break;

  case 48: /* @15: %empty  */
#line 400 "src/Slice/Grammar.y"
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
#line 2167 "src/Slice/Grammar.cpp"
    break;

  case 49: /* module_def: ICE_MODULE ICE_SCOPED_IDENTIFIER @15 '{' definitions '}'  */
#line 455 "src/Slice/Grammar.y"
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
#line 2189 "src/Slice/Grammar.cpp"
    break;

  case 50: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 478 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2197 "src/Slice/Grammar.cpp"
    break;

  case 51: /* exception_id: ICE_EXCEPTION keyword  */
#line 482 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2207 "src/Slice/Grammar.cpp"
    break;

  case 52: /* exception_decl: exception_id  */
#line 493 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2216 "src/Slice/Grammar.cpp"
    break;

  case 53: /* @16: %empty  */
#line 503 "src/Slice/Grammar.y"
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
#line 2233 "src/Slice/Grammar.cpp"
    break;

  case 54: /* exception_def: exception_id exception_extends @16 '{' data_members '}'  */
#line 516 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2245 "src/Slice/Grammar.cpp"
    break;

  case 55: /* exception_extends: extends scoped_name  */
#line 529 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2257 "src/Slice/Grammar.cpp"
    break;

  case 56: /* exception_extends: %empty  */
#line 537 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2265 "src/Slice/Grammar.cpp"
    break;

  case 57: /* tag: ICE_TAG_OPEN ICE_INTEGER_LITERAL ')'  */
#line 546 "src/Slice/Grammar.y"
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
#line 2287 "src/Slice/Grammar.cpp"
    break;

  case 58: /* tag: ICE_TAG_OPEN scoped_name ')'  */
#line 564 "src/Slice/Grammar.y"
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
#line 2365 "src/Slice/Grammar.cpp"
    break;

  case 59: /* tag: ICE_TAG_OPEN ')'  */
#line 638 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2375 "src/Slice/Grammar.cpp"
    break;

  case 60: /* tag: ICE_TAG  */
#line 644 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2385 "src/Slice/Grammar.cpp"
    break;

  case 61: /* optional: ICE_OPTIONAL_OPEN ICE_INTEGER_LITERAL ')'  */
#line 655 "src/Slice/Grammar.y"
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
#line 2411 "src/Slice/Grammar.cpp"
    break;

  case 62: /* optional: ICE_OPTIONAL_OPEN scoped_name ')'  */
#line 677 "src/Slice/Grammar.y"
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
#line 2493 "src/Slice/Grammar.cpp"
    break;

  case 63: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 755 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2507 "src/Slice/Grammar.cpp"
    break;

  case 64: /* optional: ICE_OPTIONAL  */
#line 765 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2521 "src/Slice/Grammar.cpp"
    break;

  case 65: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 780 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2529 "src/Slice/Grammar.cpp"
    break;

  case 66: /* struct_id: ICE_STRUCT keyword  */
#line 784 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2539 "src/Slice/Grammar.cpp"
    break;

  case 67: /* struct_decl: struct_id  */
#line 795 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2548 "src/Slice/Grammar.cpp"
    break;

  case 68: /* @17: %empty  */
#line 805 "src/Slice/Grammar.y"
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
#line 2570 "src/Slice/Grammar.cpp"
    break;

  case 69: /* struct_def: struct_id @17 '{' data_members '}'  */
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
    if (!st->hasDataMembers())
    {
        unit->error("struct `" + st->name() + "' must have at least one member"); // $$ is a dummy
    }
}
#line 2592 "src/Slice/Grammar.cpp"
    break;

  case 70: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 846 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2600 "src/Slice/Grammar.cpp"
    break;

  case 71: /* class_name: ICE_CLASS keyword  */
#line 850 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2610 "src/Slice/Grammar.cpp"
    break;

  case 72: /* class_id: ICE_CLASS ICE_IDENT_OPEN ICE_INTEGER_LITERAL ')'  */
#line 861 "src/Slice/Grammar.y"
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
#line 2639 "src/Slice/Grammar.cpp"
    break;

  case 73: /* class_id: ICE_CLASS ICE_IDENT_OPEN scoped_name ')'  */
#line 886 "src/Slice/Grammar.y"
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
#line 2733 "src/Slice/Grammar.cpp"
    break;

  case 74: /* class_id: class_name  */
#line 976 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2744 "src/Slice/Grammar.cpp"
    break;

  case 75: /* class_decl: class_name  */
#line 988 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2755 "src/Slice/Grammar.cpp"
    break;

  case 76: /* @18: %empty  */
#line 1000 "src/Slice/Grammar.y"
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
#line 2776 "src/Slice/Grammar.cpp"
    break;

  case 77: /* class_def: class_id class_extends @18 '{' data_members '}'  */
#line 1017 "src/Slice/Grammar.y"
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
#line 2792 "src/Slice/Grammar.cpp"
    break;

  case 78: /* class_extends: extends scoped_name  */
#line 1034 "src/Slice/Grammar.y"
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
#line 2830 "src/Slice/Grammar.cpp"
    break;

  case 79: /* class_extends: %empty  */
#line 1068 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2838 "src/Slice/Grammar.cpp"
    break;

  case 82: /* data_member: member  */
#line 1084 "src/Slice/Grammar.y"
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
#line 2857 "src/Slice/Grammar.cpp"
    break;

  case 83: /* data_member: member '=' const_initializer  */
#line 1099 "src/Slice/Grammar.y"
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
#line 2878 "src/Slice/Grammar.cpp"
    break;

  case 86: /* data_member_list: data_member  */
#line 1123 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2886 "src/Slice/Grammar.cpp"
    break;

  case 90: /* return_tuple: out_qualifier member  */
#line 1140 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnMember = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    if (BoolTokPtr::dynamicCast(yyvsp[-1])->v)
    {
        unit->error("`" + returnMember->name + "': return members cannot be marked as out");
    }

    TaggedDefListTokPtr returnMembers = new TaggedDefListTok();
    returnMembers->v.push_back(returnMember);
    yyval = returnMembers;
}
#line 2902 "src/Slice/Grammar.cpp"
    break;

  case 91: /* return_tuple: return_tuple ',' out_qualifier member  */
#line 1152 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnMember = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    if (BoolTokPtr::dynamicCast(yyvsp[-1])->v)
    {
        unit->error("`" + returnMember->name + "': return members cannot be marked as out");
    }

    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-3]);
    returnMembers->v.push_back(returnMember);
    yyval = returnMembers;
}
#line 2918 "src/Slice/Grammar.cpp"
    break;

  case 92: /* return_type: tagged_type  */
#line 1168 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr returnMember = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    // For unnamed return types we infer their name to be 'returnValue'.
    returnMember->name = "returnValue";

    if (returnMember->isTagged)
    {
        checkForTaggableType(returnMember->type);
    }

    TaggedDefListTokPtr returnMembers = new TaggedDefListTok();
    returnMembers->v.push_back(returnMember);
    yyval = returnMembers;
}
#line 2937 "src/Slice/Grammar.cpp"
    break;

  case 93: /* return_type: '(' return_tuple ')'  */
#line 1183 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-1]);
    if (returnMembers->v.size() == 1)
    {
        unit->error("return tuples must contain at least 2 elements");
    }
    yyval = yyvsp[-1];
}
#line 2950 "src/Slice/Grammar.cpp"
    break;

  case 94: /* return_type: '(' ')'  */
#line 1192 "src/Slice/Grammar.y"
{
    unit->error("return tuples must contain at least 2 elements");
    yyval = new TaggedDefListTok();
}
#line 2959 "src/Slice/Grammar.cpp"
    break;

  case 95: /* return_type: ICE_VOID  */
#line 1197 "src/Slice/Grammar.y"
{
    yyval = new TaggedDefListTok();
}
#line 2967 "src/Slice/Grammar.cpp"
    break;

  case 96: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1206 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;

    if (InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer()))
    {
        if (OperationPtr op = interface->createOperation(name))
        {
            interface->checkIntroduced(name, op);
            unit->pushContainer(op);

            // Set the return members for the operation.
            for (const auto& returnMember : returnMembers->v)
            {
                MemberPtr p = op->createReturnMember(returnMember->name, returnMember->type, returnMember->isTagged,
                                                     returnMember->tag);
                if (p && !returnMember->metadata.empty())
                {
                    p->setMetaData(returnMember->metadata);
                }
            }

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
#line 3006 "src/Slice/Grammar.cpp"
    break;

  case 97: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1241 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;

    if (InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer()))
    {
        if (OperationPtr op = interface->createOperation(name, Operation::Idempotent))
        {
            interface->checkIntroduced(name, op);
            unit->pushContainer(op);

            // Set the return members for the operation.
            for (const auto& returnMember : returnMembers->v)
            {
                MemberPtr p = op->createReturnMember(returnMember->name, returnMember->type, returnMember->isTagged,
                                                     returnMember->tag);
                if (p && !returnMember->metadata.empty())
                {
                    p->setMetaData(returnMember->metadata);
                }
            }

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
#line 3045 "src/Slice/Grammar.cpp"
    break;

  case 98: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1276 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    if (InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer()))
    {
        if (OperationPtr op = interface->createOperation(name))
        {
            unit->pushContainer(op);

            // Set the return members for the operation.
            for (const auto& returnMember : returnMembers->v)
            {
                MemberPtr p = op->createReturnMember(returnMember->name, returnMember->type, returnMember->isTagged,
                                                     returnMember->tag);
                if (p && !returnMember->metadata.empty())
                {
                    p->setMetaData(returnMember->metadata);
                }
            }

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
#line 3083 "src/Slice/Grammar.cpp"
    break;

  case 99: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1310 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    if (InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer()))
    {
        if (OperationPtr op = interface->createOperation(name, Operation::Idempotent))
        {
            unit->pushContainer(op);

            // Set the return members for the operation.
            for (const auto& returnMember : returnMembers->v)
            {
                MemberPtr p = op->createReturnMember(returnMember->name, returnMember->type, returnMember->isTagged,
                                                     returnMember->tag);
                if (p && !returnMember->metadata.empty())
                {
                    p->setMetaData(returnMember->metadata);
                }
            }

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
#line 3121 "src/Slice/Grammar.cpp"
    break;

  case 100: /* @19: %empty  */
#line 1349 "src/Slice/Grammar.y"
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
#line 3137 "src/Slice/Grammar.cpp"
    break;

  case 101: /* operation: operation_preamble parameters ')' @19 throws  */
#line 1361 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3151 "src/Slice/Grammar.cpp"
    break;

  case 102: /* @20: %empty  */
#line 1371 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3163 "src/Slice/Grammar.cpp"
    break;

  case 103: /* operation: operation_preamble error ')' @20 throws  */
#line 1379 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3177 "src/Slice/Grammar.cpp"
    break;

  case 104: /* operation_list: local_metadata operation ';' operation_list  */
#line 1394 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    OperationPtr operation = OperationPtr::dynamicCast(yyvsp[-2]);
    if (operation && !metaData->v.empty())
    {
        operation->setMetaData(metaData->v);

        // If the operation had a single return type (not a return tuple), also apply the metadata to the return type.
        // TODO: once we introduce more concrete metadata validation, we could sort the metadata out between the return
        // type and the operation itself. So metadata relevant to operations would only be set for the operation, and
        // metadata only relevant to the return type would only be set on the return type.
        if (operation->hasSingleReturnType())
        {
            operation->returnType().front()->setMetaData(metaData->v);
        }
    }
}
#line 3199 "src/Slice/Grammar.cpp"
    break;

  case 105: /* operation_list: local_metadata operation  */
#line 1412 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3207 "src/Slice/Grammar.cpp"
    break;

  case 108: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1423 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3215 "src/Slice/Grammar.cpp"
    break;

  case 109: /* interface_id: ICE_INTERFACE keyword  */
#line 1427 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3225 "src/Slice/Grammar.cpp"
    break;

  case 110: /* interface_decl: interface_id  */
#line 1438 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3237 "src/Slice/Grammar.cpp"
    break;

  case 111: /* @21: %empty  */
#line 1451 "src/Slice/Grammar.y"
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
#line 3258 "src/Slice/Grammar.cpp"
    break;

  case 112: /* interface_def: interface_id interface_extends @21 '{' operation_list '}'  */
#line 1468 "src/Slice/Grammar.y"
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
#line 3274 "src/Slice/Grammar.cpp"
    break;

  case 113: /* interface_list: scoped_name ',' interface_list  */
#line 1485 "src/Slice/Grammar.y"
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
#line 3313 "src/Slice/Grammar.cpp"
    break;

  case 114: /* interface_list: scoped_name  */
#line 1520 "src/Slice/Grammar.y"
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
#line 3352 "src/Slice/Grammar.cpp"
    break;

  case 115: /* interface_list: ICE_OBJECT  */
#line 1555 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new InterfaceListTok; // Dummy
}
#line 3361 "src/Slice/Grammar.cpp"
    break;

  case 116: /* interface_list: ICE_ANYCLASS  */
#line 1560 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type AnyClass");
    yyval = new ClassListTok; // Dummy
}
#line 3370 "src/Slice/Grammar.cpp"
    break;

  case 117: /* interface_list: ICE_VALUE  */
#line 1565 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3383 "src/Slice/Grammar.cpp"
    break;

  case 118: /* interface_extends: extends interface_list  */
#line 1579 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3391 "src/Slice/Grammar.cpp"
    break;

  case 119: /* interface_extends: %empty  */
#line 1583 "src/Slice/Grammar.y"
{
    yyval = new InterfaceListTok;
}
#line 3399 "src/Slice/Grammar.cpp"
    break;

  case 120: /* exception_list: exception ',' exception_list  */
#line 1592 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3410 "src/Slice/Grammar.cpp"
    break;

  case 121: /* exception_list: exception  */
#line 1599 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3421 "src/Slice/Grammar.cpp"
    break;

  case 122: /* exception: scoped_name  */
#line 1611 "src/Slice/Grammar.y"
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
#line 3437 "src/Slice/Grammar.cpp"
    break;

  case 123: /* exception: keyword  */
#line 1623 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentModule()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3447 "src/Slice/Grammar.cpp"
    break;

  case 124: /* sequence_def: ICE_SEQUENCE '<' local_metadata type '>' ICE_IDENTIFIER  */
#line 1634 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3459 "src/Slice/Grammar.cpp"
    break;

  case 125: /* sequence_def: ICE_SEQUENCE '<' local_metadata type '>' keyword  */
#line 1642 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3472 "src/Slice/Grammar.cpp"
    break;

  case 126: /* dictionary_def: ICE_DICTIONARY '<' local_metadata type ',' local_metadata type '>' ICE_IDENTIFIER  */
#line 1656 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3486 "src/Slice/Grammar.cpp"
    break;

  case 127: /* dictionary_def: ICE_DICTIONARY '<' local_metadata type ',' local_metadata type '>' keyword  */
#line 1666 "src/Slice/Grammar.y"
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
#line 3501 "src/Slice/Grammar.cpp"
    break;

  case 128: /* enum_start: ICE_UNCHECKED ICE_ENUM  */
#line 1682 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(true);
}
#line 3509 "src/Slice/Grammar.cpp"
    break;

  case 129: /* enum_start: ICE_ENUM  */
#line 1686 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(false);
}
#line 3517 "src/Slice/Grammar.cpp"
    break;

  case 130: /* enum_id: enum_start ICE_IDENTIFIER  */
#line 1695 "src/Slice/Grammar.y"
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
#line 3537 "src/Slice/Grammar.cpp"
    break;

  case 131: /* enum_id: enum_start keyword  */
#line 1711 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[-1])->v;
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
}
#line 3549 "src/Slice/Grammar.cpp"
    break;

  case 132: /* @22: %empty  */
#line 1724 "src/Slice/Grammar.y"
{
    EnumPtr en = EnumPtr::dynamicCast(yyvsp[-1]);
    en->initUnderlying(TypePtr::dynamicCast(yyvsp[0]));
    unit->pushContainer(en);
    yyval = en;
}
#line 3560 "src/Slice/Grammar.cpp"
    break;

  case 133: /* enum_def: enum_id enum_underlying @22 '{' enumerator_list '}'  */
#line 1731 "src/Slice/Grammar.y"
{
    EnumPtr en = EnumPtr::dynamicCast(yyvsp[-3]);
    if(en)
    {
        EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast(yyvsp[-1]);
        if(enumerators->v.empty() && !en->underlying() && en->isUnchecked())
        {
            unit->error("enum `" + en->name() + "' must have at least one enumerator");
        }
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 3578 "src/Slice/Grammar.cpp"
    break;

  case 134: /* @23: %empty  */
#line 1746 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[0])->v;
    unit->error("missing enumeration name");
    ModulePtr cont = unit->currentModule();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3591 "src/Slice/Grammar.cpp"
    break;

  case 135: /* enum_def: enum_start @23 '{' enumerator_list '}'  */
#line 1755 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3600 "src/Slice/Grammar.cpp"
    break;

  case 136: /* enum_underlying: ':' type  */
#line 1765 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3608 "src/Slice/Grammar.cpp"
    break;

  case 137: /* enum_underlying: %empty  */
#line 1769 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 3616 "src/Slice/Grammar.cpp"
    break;

  case 138: /* enumerator_list: enumerator ',' enumerator_list  */
#line 1778 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 3626 "src/Slice/Grammar.cpp"
    break;

  case 140: /* enumerator: ICE_IDENTIFIER  */
#line 1790 "src/Slice/Grammar.y"
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
#line 3642 "src/Slice/Grammar.cpp"
    break;

  case 141: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1802 "src/Slice/Grammar.y"
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
#line 3659 "src/Slice/Grammar.cpp"
    break;

  case 142: /* enumerator: keyword  */
#line 1815 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 3670 "src/Slice/Grammar.cpp"
    break;

  case 143: /* enumerator: %empty  */
#line 1822 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 3679 "src/Slice/Grammar.cpp"
    break;

  case 144: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1832 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3687 "src/Slice/Grammar.cpp"
    break;

  case 145: /* enumerator_initializer: scoped_name  */
#line 1836 "src/Slice/Grammar.y"
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
#line 3724 "src/Slice/Grammar.cpp"
    break;

  case 146: /* out_qualifier: ICE_OUT  */
#line 1874 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(true);
}
#line 3732 "src/Slice/Grammar.cpp"
    break;

  case 147: /* out_qualifier: %empty  */
#line 1878 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(false);
}
#line 3740 "src/Slice/Grammar.cpp"
    break;

  case 148: /* parameter: out_qualifier member  */
#line 1887 "src/Slice/Grammar.y"
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
#line 3759 "src/Slice/Grammar.cpp"
    break;

  case 153: /* throws: ICE_THROWS exception_list  */
#line 1920 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3767 "src/Slice/Grammar.cpp"
    break;

  case 154: /* throws: %empty  */
#line 1924 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 3775 "src/Slice/Grammar.cpp"
    break;

  case 157: /* builtin: ICE_BOOL  */
#line 1939 "src/Slice/Grammar.y"
           {}
#line 3781 "src/Slice/Grammar.cpp"
    break;

  case 158: /* builtin: ICE_BYTE  */
#line 1940 "src/Slice/Grammar.y"
           {}
#line 3787 "src/Slice/Grammar.cpp"
    break;

  case 159: /* builtin: ICE_SHORT  */
#line 1941 "src/Slice/Grammar.y"
            {}
#line 3793 "src/Slice/Grammar.cpp"
    break;

  case 160: /* builtin: ICE_USHORT  */
#line 1942 "src/Slice/Grammar.y"
             {}
#line 3799 "src/Slice/Grammar.cpp"
    break;

  case 161: /* builtin: ICE_INT  */
#line 1943 "src/Slice/Grammar.y"
          {}
#line 3805 "src/Slice/Grammar.cpp"
    break;

  case 162: /* builtin: ICE_UINT  */
#line 1944 "src/Slice/Grammar.y"
           {}
#line 3811 "src/Slice/Grammar.cpp"
    break;

  case 163: /* builtin: ICE_VARINT  */
#line 1945 "src/Slice/Grammar.y"
             {}
#line 3817 "src/Slice/Grammar.cpp"
    break;

  case 164: /* builtin: ICE_VARUINT  */
#line 1946 "src/Slice/Grammar.y"
              {}
#line 3823 "src/Slice/Grammar.cpp"
    break;

  case 165: /* builtin: ICE_LONG  */
#line 1947 "src/Slice/Grammar.y"
           {}
#line 3829 "src/Slice/Grammar.cpp"
    break;

  case 166: /* builtin: ICE_ULONG  */
#line 1948 "src/Slice/Grammar.y"
            {}
#line 3835 "src/Slice/Grammar.cpp"
    break;

  case 167: /* builtin: ICE_VARLONG  */
#line 1949 "src/Slice/Grammar.y"
              {}
#line 3841 "src/Slice/Grammar.cpp"
    break;

  case 168: /* builtin: ICE_VARULONG  */
#line 1950 "src/Slice/Grammar.y"
               {}
#line 3847 "src/Slice/Grammar.cpp"
    break;

  case 169: /* builtin: ICE_FLOAT  */
#line 1951 "src/Slice/Grammar.y"
            {}
#line 3853 "src/Slice/Grammar.cpp"
    break;

  case 170: /* builtin: ICE_DOUBLE  */
#line 1952 "src/Slice/Grammar.y"
             {}
#line 3859 "src/Slice/Grammar.cpp"
    break;

  case 171: /* builtin: ICE_STRING  */
#line 1953 "src/Slice/Grammar.y"
             {}
#line 3865 "src/Slice/Grammar.cpp"
    break;

  case 172: /* type: ICE_OBJECT '*'  */
#line 1959 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 3873 "src/Slice/Grammar.cpp"
    break;

  case 173: /* type: ICE_OBJECT '?'  */
#line 1963 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 3881 "src/Slice/Grammar.cpp"
    break;

  case 174: /* type: ICE_ANYCLASS '?'  */
#line 1967 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 3889 "src/Slice/Grammar.cpp"
    break;

  case 175: /* type: ICE_VALUE '?'  */
#line 1971 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 3901 "src/Slice/Grammar.cpp"
    break;

  case 176: /* type: builtin '?'  */
#line 1979 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[-1]);
    yyval = unit->optionalBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 3910 "src/Slice/Grammar.cpp"
    break;

  case 177: /* type: ICE_OBJECT  */
#line 1984 "src/Slice/Grammar.y"
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
#line 3925 "src/Slice/Grammar.cpp"
    break;

  case 178: /* type: ICE_ANYCLASS  */
#line 1995 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindAnyClass);
}
#line 3933 "src/Slice/Grammar.cpp"
    break;

  case 179: /* type: ICE_VALUE  */
#line 1999 "src/Slice/Grammar.y"
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
#line 3949 "src/Slice/Grammar.cpp"
    break;

  case 180: /* type: builtin  */
#line 2011 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 3958 "src/Slice/Grammar.cpp"
    break;

  case 181: /* type: scoped_name  */
#line 2016 "src/Slice/Grammar.y"
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
#line 3988 "src/Slice/Grammar.cpp"
    break;

  case 182: /* type: scoped_name '*'  */
#line 2042 "src/Slice/Grammar.y"
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
#line 4024 "src/Slice/Grammar.cpp"
    break;

  case 183: /* type: scoped_name '?'  */
#line 2074 "src/Slice/Grammar.y"
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
#line 4051 "src/Slice/Grammar.cpp"
    break;

  case 184: /* tagged_type: tag type  */
#line 2102 "src/Slice/Grammar.y"
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
#line 4070 "src/Slice/Grammar.cpp"
    break;

  case 185: /* tagged_type: optional type  */
#line 2117 "src/Slice/Grammar.y"
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
#line 4088 "src/Slice/Grammar.cpp"
    break;

  case 186: /* tagged_type: type  */
#line 2131 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr taggedDef = new TaggedDefTok;
    taggedDef->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = taggedDef;
}
#line 4098 "src/Slice/Grammar.cpp"
    break;

  case 187: /* member: tagged_type ICE_IDENTIFIER  */
#line 2142 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    def->name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    checkIdentifier(def->name);
    if (def->isTagged)
    {
        checkForTaggableType(def->type, def->name);
    }

    yyval = def;
}
#line 4114 "src/Slice/Grammar.cpp"
    break;

  case 188: /* member: tagged_type keyword  */
#line 2154 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    def->name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    if (def->isTagged)
    {
        checkForTaggableType(def->type, def->name);
    }
    unit->error("keyword `" + def->name + "' cannot be used as an identifier");
    yyval = def;
}
#line 4129 "src/Slice/Grammar.cpp"
    break;

  case 189: /* member: tagged_type  */
#line 2165 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    def->name = IceUtil::generateUUID(); // Dummy
    if (def->isTagged)
    {
        checkForTaggableType(def->type);
    }
    unit->error("missing identifier");
    yyval = def;
}
#line 4144 "src/Slice/Grammar.cpp"
    break;

  case 190: /* member: local_metadata member  */
#line 2176 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    def->metadata = StringListTokPtr::dynamicCast(yyvsp[-1])->v;
    yyval = def;
}
#line 4154 "src/Slice/Grammar.cpp"
    break;

  case 191: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 2187 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4164 "src/Slice/Grammar.cpp"
    break;

  case 193: /* string_list: string_list ',' string_literal  */
#line 2199 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4175 "src/Slice/Grammar.cpp"
    break;

  case 194: /* string_list: string_literal  */
#line 2206 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4186 "src/Slice/Grammar.cpp"
    break;

  case 195: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2218 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4199 "src/Slice/Grammar.cpp"
    break;

  case 196: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2227 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4212 "src/Slice/Grammar.cpp"
    break;

  case 197: /* const_initializer: scoped_name  */
#line 2236 "src/Slice/Grammar.y"
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
#line 4256 "src/Slice/Grammar.cpp"
    break;

  case 198: /* const_initializer: ICE_STRING_LITERAL  */
#line 2276 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4267 "src/Slice/Grammar.cpp"
    break;

  case 199: /* const_initializer: ICE_FALSE  */
#line 2283 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4278 "src/Slice/Grammar.cpp"
    break;

  case 200: /* const_initializer: ICE_TRUE  */
#line 2290 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4289 "src/Slice/Grammar.cpp"
    break;

  case 201: /* const_def: ICE_CONST local_metadata type ICE_IDENTIFIER '=' const_initializer  */
#line 2302 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentModule()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4302 "src/Slice/Grammar.cpp"
    break;

  case 202: /* const_def: ICE_CONST local_metadata type '=' const_initializer  */
#line 2311 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentModule()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4315 "src/Slice/Grammar.cpp"
    break;

  case 203: /* keyword: ICE_MODULE  */
#line 2324 "src/Slice/Grammar.y"
             {}
#line 4321 "src/Slice/Grammar.cpp"
    break;

  case 204: /* keyword: ICE_CLASS  */
#line 2325 "src/Slice/Grammar.y"
            {}
#line 4327 "src/Slice/Grammar.cpp"
    break;

  case 205: /* keyword: ICE_INTERFACE  */
#line 2326 "src/Slice/Grammar.y"
                {}
#line 4333 "src/Slice/Grammar.cpp"
    break;

  case 206: /* keyword: ICE_EXCEPTION  */
#line 2327 "src/Slice/Grammar.y"
                {}
#line 4339 "src/Slice/Grammar.cpp"
    break;

  case 207: /* keyword: ICE_STRUCT  */
#line 2328 "src/Slice/Grammar.y"
             {}
#line 4345 "src/Slice/Grammar.cpp"
    break;

  case 208: /* keyword: ICE_SEQUENCE  */
#line 2329 "src/Slice/Grammar.y"
               {}
#line 4351 "src/Slice/Grammar.cpp"
    break;

  case 209: /* keyword: ICE_DICTIONARY  */
#line 2330 "src/Slice/Grammar.y"
                 {}
#line 4357 "src/Slice/Grammar.cpp"
    break;

  case 210: /* keyword: ICE_ENUM  */
#line 2331 "src/Slice/Grammar.y"
           {}
#line 4363 "src/Slice/Grammar.cpp"
    break;

  case 211: /* keyword: ICE_OUT  */
#line 2332 "src/Slice/Grammar.y"
          {}
#line 4369 "src/Slice/Grammar.cpp"
    break;

  case 212: /* keyword: ICE_EXTENDS  */
#line 2333 "src/Slice/Grammar.y"
              {}
#line 4375 "src/Slice/Grammar.cpp"
    break;

  case 213: /* keyword: ICE_IMPLEMENTS  */
#line 2334 "src/Slice/Grammar.y"
                 {}
#line 4381 "src/Slice/Grammar.cpp"
    break;

  case 214: /* keyword: ICE_THROWS  */
#line 2335 "src/Slice/Grammar.y"
             {}
#line 4387 "src/Slice/Grammar.cpp"
    break;

  case 215: /* keyword: ICE_VOID  */
#line 2336 "src/Slice/Grammar.y"
           {}
#line 4393 "src/Slice/Grammar.cpp"
    break;

  case 216: /* keyword: ICE_BOOL  */
#line 2337 "src/Slice/Grammar.y"
           {}
#line 4399 "src/Slice/Grammar.cpp"
    break;

  case 217: /* keyword: ICE_BYTE  */
#line 2338 "src/Slice/Grammar.y"
           {}
#line 4405 "src/Slice/Grammar.cpp"
    break;

  case 218: /* keyword: ICE_SHORT  */
#line 2339 "src/Slice/Grammar.y"
            {}
#line 4411 "src/Slice/Grammar.cpp"
    break;

  case 219: /* keyword: ICE_USHORT  */
#line 2340 "src/Slice/Grammar.y"
             {}
#line 4417 "src/Slice/Grammar.cpp"
    break;

  case 220: /* keyword: ICE_INT  */
#line 2341 "src/Slice/Grammar.y"
          {}
#line 4423 "src/Slice/Grammar.cpp"
    break;

  case 221: /* keyword: ICE_UINT  */
#line 2342 "src/Slice/Grammar.y"
           {}
#line 4429 "src/Slice/Grammar.cpp"
    break;

  case 222: /* keyword: ICE_VARINT  */
#line 2343 "src/Slice/Grammar.y"
             {}
#line 4435 "src/Slice/Grammar.cpp"
    break;

  case 223: /* keyword: ICE_VARUINT  */
#line 2344 "src/Slice/Grammar.y"
              {}
#line 4441 "src/Slice/Grammar.cpp"
    break;

  case 224: /* keyword: ICE_LONG  */
#line 2345 "src/Slice/Grammar.y"
           {}
#line 4447 "src/Slice/Grammar.cpp"
    break;

  case 225: /* keyword: ICE_ULONG  */
#line 2346 "src/Slice/Grammar.y"
            {}
#line 4453 "src/Slice/Grammar.cpp"
    break;

  case 226: /* keyword: ICE_VARLONG  */
#line 2347 "src/Slice/Grammar.y"
              {}
#line 4459 "src/Slice/Grammar.cpp"
    break;

  case 227: /* keyword: ICE_VARULONG  */
#line 2348 "src/Slice/Grammar.y"
               {}
#line 4465 "src/Slice/Grammar.cpp"
    break;

  case 228: /* keyword: ICE_FLOAT  */
#line 2349 "src/Slice/Grammar.y"
            {}
#line 4471 "src/Slice/Grammar.cpp"
    break;

  case 229: /* keyword: ICE_DOUBLE  */
#line 2350 "src/Slice/Grammar.y"
             {}
#line 4477 "src/Slice/Grammar.cpp"
    break;

  case 230: /* keyword: ICE_STRING  */
#line 2351 "src/Slice/Grammar.y"
             {}
#line 4483 "src/Slice/Grammar.cpp"
    break;

  case 231: /* keyword: ICE_OBJECT  */
#line 2352 "src/Slice/Grammar.y"
             {}
#line 4489 "src/Slice/Grammar.cpp"
    break;

  case 232: /* keyword: ICE_CONST  */
#line 2353 "src/Slice/Grammar.y"
            {}
#line 4495 "src/Slice/Grammar.cpp"
    break;

  case 233: /* keyword: ICE_FALSE  */
#line 2354 "src/Slice/Grammar.y"
            {}
#line 4501 "src/Slice/Grammar.cpp"
    break;

  case 234: /* keyword: ICE_TRUE  */
#line 2355 "src/Slice/Grammar.y"
           {}
#line 4507 "src/Slice/Grammar.cpp"
    break;

  case 235: /* keyword: ICE_IDEMPOTENT  */
#line 2356 "src/Slice/Grammar.y"
                 {}
#line 4513 "src/Slice/Grammar.cpp"
    break;

  case 236: /* keyword: ICE_TAG  */
#line 2357 "src/Slice/Grammar.y"
          {}
#line 4519 "src/Slice/Grammar.cpp"
    break;

  case 237: /* keyword: ICE_OPTIONAL  */
#line 2358 "src/Slice/Grammar.y"
               {}
#line 4525 "src/Slice/Grammar.cpp"
    break;

  case 238: /* keyword: ICE_ANYCLASS  */
#line 2359 "src/Slice/Grammar.y"
               {}
#line 4531 "src/Slice/Grammar.cpp"
    break;

  case 239: /* keyword: ICE_VALUE  */
#line 2360 "src/Slice/Grammar.y"
            {}
#line 4537 "src/Slice/Grammar.cpp"
    break;


#line 4541 "src/Slice/Grammar.cpp"

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

#line 2363 "src/Slice/Grammar.y"

