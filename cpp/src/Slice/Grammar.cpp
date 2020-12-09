/* A Bison parser, made by GNU Bison 3.7.4.  */

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

/* Identify Bison output, and Bison version.  */
#define YYBISON 30704

/* Bison version string.  */
#define YYBISON_VERSION "3.7.4"

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

// TODO this function is only temporarily necessary to convert between the old and new syntax styles.
void convertMetadata(StringListTokPtr& metadata)
{
    for (auto& m : metadata->v)
    {
        auto pos = m.find('(');
        if (pos != string::npos)
        {
            m[pos] = ':';
            assert(m.back() == ')');
            m.pop_back();
        }
    }
}

const int QUALIFIER_NONE = 0;
const int QUALIFIER_OUT = 1;
const int QUALIFIER_STREAM = 2;


#line 187 "src/Slice/Grammar.cpp"

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
  YYSYMBOL_ICE_USING = 8,                  /* ICE_USING  */
  YYSYMBOL_ICE_SEQUENCE = 9,               /* ICE_SEQUENCE  */
  YYSYMBOL_ICE_DICTIONARY = 10,            /* ICE_DICTIONARY  */
  YYSYMBOL_ICE_ENUM = 11,                  /* ICE_ENUM  */
  YYSYMBOL_ICE_OUT = 12,                   /* ICE_OUT  */
  YYSYMBOL_ICE_STREAM = 13,                /* ICE_STREAM  */
  YYSYMBOL_ICE_EXTENDS = 14,               /* ICE_EXTENDS  */
  YYSYMBOL_ICE_IMPLEMENTS = 15,            /* ICE_IMPLEMENTS  */
  YYSYMBOL_ICE_THROWS = 16,                /* ICE_THROWS  */
  YYSYMBOL_ICE_VOID = 17,                  /* ICE_VOID  */
  YYSYMBOL_ICE_BOOL = 18,                  /* ICE_BOOL  */
  YYSYMBOL_ICE_BYTE = 19,                  /* ICE_BYTE  */
  YYSYMBOL_ICE_SHORT = 20,                 /* ICE_SHORT  */
  YYSYMBOL_ICE_USHORT = 21,                /* ICE_USHORT  */
  YYSYMBOL_ICE_INT = 22,                   /* ICE_INT  */
  YYSYMBOL_ICE_UINT = 23,                  /* ICE_UINT  */
  YYSYMBOL_ICE_VARINT = 24,                /* ICE_VARINT  */
  YYSYMBOL_ICE_VARUINT = 25,               /* ICE_VARUINT  */
  YYSYMBOL_ICE_LONG = 26,                  /* ICE_LONG  */
  YYSYMBOL_ICE_ULONG = 27,                 /* ICE_ULONG  */
  YYSYMBOL_ICE_VARLONG = 28,               /* ICE_VARLONG  */
  YYSYMBOL_ICE_VARULONG = 29,              /* ICE_VARULONG  */
  YYSYMBOL_ICE_FLOAT = 30,                 /* ICE_FLOAT  */
  YYSYMBOL_ICE_DOUBLE = 31,                /* ICE_DOUBLE  */
  YYSYMBOL_ICE_STRING = 32,                /* ICE_STRING  */
  YYSYMBOL_ICE_OBJECT = 33,                /* ICE_OBJECT  */
  YYSYMBOL_ICE_CONST = 34,                 /* ICE_CONST  */
  YYSYMBOL_ICE_FALSE = 35,                 /* ICE_FALSE  */
  YYSYMBOL_ICE_TRUE = 36,                  /* ICE_TRUE  */
  YYSYMBOL_ICE_IDEMPOTENT = 37,            /* ICE_IDEMPOTENT  */
  YYSYMBOL_ICE_TAG = 38,                   /* ICE_TAG  */
  YYSYMBOL_ICE_OPTIONAL = 39,              /* ICE_OPTIONAL  */
  YYSYMBOL_ICE_ANYCLASS = 40,              /* ICE_ANYCLASS  */
  YYSYMBOL_ICE_VALUE = 41,                 /* ICE_VALUE  */
  YYSYMBOL_ICE_UNCHECKED = 42,             /* ICE_UNCHECKED  */
  YYSYMBOL_ICE_STRING_LITERAL = 43,        /* ICE_STRING_LITERAL  */
  YYSYMBOL_ICE_INTEGER_LITERAL = 44,       /* ICE_INTEGER_LITERAL  */
  YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 45, /* ICE_FLOATING_POINT_LITERAL  */
  YYSYMBOL_ICE_IDENTIFIER = 46,            /* ICE_IDENTIFIER  */
  YYSYMBOL_ICE_SCOPED_IDENTIFIER = 47,     /* ICE_SCOPED_IDENTIFIER  */
  YYSYMBOL_ICE_LOCAL_METADATA_OPEN = 48,   /* ICE_LOCAL_METADATA_OPEN  */
  YYSYMBOL_ICE_LOCAL_METADATA_CLOSE = 49,  /* ICE_LOCAL_METADATA_CLOSE  */
  YYSYMBOL_ICE_FILE_METADATA_OPEN = 50,    /* ICE_FILE_METADATA_OPEN  */
  YYSYMBOL_ICE_FILE_METADATA_CLOSE = 51,   /* ICE_FILE_METADATA_CLOSE  */
  YYSYMBOL_BAD_CHAR = 52,                  /* BAD_CHAR  */
  YYSYMBOL_53_ = 53,                       /* ';'  */
  YYSYMBOL_54_ = 54,                       /* '{'  */
  YYSYMBOL_55_ = 55,                       /* '}'  */
  YYSYMBOL_56_ = 56,                       /* '('  */
  YYSYMBOL_57_ = 57,                       /* ')'  */
  YYSYMBOL_58_ = 58,                       /* ':'  */
  YYSYMBOL_59_ = 59,                       /* '='  */
  YYSYMBOL_60_ = 60,                       /* ','  */
  YYSYMBOL_61_ = 61,                       /* '<'  */
  YYSYMBOL_62_ = 62,                       /* '>'  */
  YYSYMBOL_63_ = 63,                       /* '*'  */
  YYSYMBOL_64_ = 64,                       /* '?'  */
  YYSYMBOL_YYACCEPT = 65,                  /* $accept  */
  YYSYMBOL_start = 66,                     /* start  */
  YYSYMBOL_opt_semicolon = 67,             /* opt_semicolon  */
  YYSYMBOL_file_metadata = 68,             /* file_metadata  */
  YYSYMBOL_local_metadata = 69,            /* local_metadata  */
  YYSYMBOL_definitions = 70,               /* definitions  */
  YYSYMBOL_definition = 71,                /* definition  */
  YYSYMBOL_72_1 = 72,                      /* $@1  */
  YYSYMBOL_73_2 = 73,                      /* $@2  */
  YYSYMBOL_74_3 = 74,                      /* $@3  */
  YYSYMBOL_75_4 = 75,                      /* $@4  */
  YYSYMBOL_76_5 = 76,                      /* $@5  */
  YYSYMBOL_77_6 = 77,                      /* $@6  */
  YYSYMBOL_78_7 = 78,                      /* $@7  */
  YYSYMBOL_79_8 = 79,                      /* $@8  */
  YYSYMBOL_80_9 = 80,                      /* $@9  */
  YYSYMBOL_81_10 = 81,                     /* $@10  */
  YYSYMBOL_82_11 = 82,                     /* $@11  */
  YYSYMBOL_83_12 = 83,                     /* $@12  */
  YYSYMBOL_84_13 = 84,                     /* $@13  */
  YYSYMBOL_85_14 = 85,                     /* $@14  */
  YYSYMBOL_module_def = 86,                /* module_def  */
  YYSYMBOL_87_15 = 87,                     /* @15  */
  YYSYMBOL_88_16 = 88,                     /* @16  */
  YYSYMBOL_exception_id = 89,              /* exception_id  */
  YYSYMBOL_exception_decl = 90,            /* exception_decl  */
  YYSYMBOL_exception_def = 91,             /* exception_def  */
  YYSYMBOL_92_17 = 92,                     /* @17  */
  YYSYMBOL_exception_extends = 93,         /* exception_extends  */
  YYSYMBOL_tag = 94,                       /* tag  */
  YYSYMBOL_optional = 95,                  /* optional  */
  YYSYMBOL_struct_id = 96,                 /* struct_id  */
  YYSYMBOL_struct_decl = 97,               /* struct_decl  */
  YYSYMBOL_struct_def = 98,                /* struct_def  */
  YYSYMBOL_99_18 = 99,                     /* @18  */
  YYSYMBOL_class_name = 100,               /* class_name  */
  YYSYMBOL_class_id = 101,                 /* class_id  */
  YYSYMBOL_class_decl = 102,               /* class_decl  */
  YYSYMBOL_class_def = 103,                /* class_def  */
  YYSYMBOL_104_19 = 104,                   /* @19  */
  YYSYMBOL_class_extends = 105,            /* class_extends  */
  YYSYMBOL_extends = 106,                  /* extends  */
  YYSYMBOL_data_member = 107,              /* data_member  */
  YYSYMBOL_data_member_list = 108,         /* data_member_list  */
  YYSYMBOL_data_members = 109,             /* data_members  */
  YYSYMBOL_return_tuple = 110,             /* return_tuple  */
  YYSYMBOL_return_type = 111,              /* return_type  */
  YYSYMBOL_operation_preamble = 112,       /* operation_preamble  */
  YYSYMBOL_operation = 113,                /* operation  */
  YYSYMBOL_114_20 = 114,                   /* @20  */
  YYSYMBOL_115_21 = 115,                   /* @21  */
  YYSYMBOL_operation_list = 116,           /* operation_list  */
  YYSYMBOL_interface_id = 117,             /* interface_id  */
  YYSYMBOL_interface_decl = 118,           /* interface_decl  */
  YYSYMBOL_interface_def = 119,            /* interface_def  */
  YYSYMBOL_120_22 = 120,                   /* @22  */
  YYSYMBOL_interface_list = 121,           /* interface_list  */
  YYSYMBOL_interface_extends = 122,        /* interface_extends  */
  YYSYMBOL_exception_list = 123,           /* exception_list  */
  YYSYMBOL_exception = 124,                /* exception  */
  YYSYMBOL_type_alias_def = 125,           /* type_alias_def  */
  YYSYMBOL_sequence_def = 126,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 127,           /* dictionary_def  */
  YYSYMBOL_enum_start = 128,               /* enum_start  */
  YYSYMBOL_enum_id = 129,                  /* enum_id  */
  YYSYMBOL_enum_def = 130,                 /* enum_def  */
  YYSYMBOL_131_23 = 131,                   /* @23  */
  YYSYMBOL_132_24 = 132,                   /* @24  */
  YYSYMBOL_enum_underlying = 133,          /* enum_underlying  */
  YYSYMBOL_enumerator_list_or_empty = 134, /* enumerator_list_or_empty  */
  YYSYMBOL_enumerator_list = 135,          /* enumerator_list  */
  YYSYMBOL_enumerator = 136,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 137,   /* enumerator_initializer  */
  YYSYMBOL_qualifier = 138,                /* qualifier  */
  YYSYMBOL_parameter = 139,                /* parameter  */
  YYSYMBOL_parameter_list = 140,           /* parameter_list  */
  YYSYMBOL_parameters = 141,               /* parameters  */
  YYSYMBOL_throws = 142,                   /* throws  */
  YYSYMBOL_scoped_name = 143,              /* scoped_name  */
  YYSYMBOL_unscoped_name = 144,            /* unscoped_name  */
  YYSYMBOL_builtin = 145,                  /* builtin  */
  YYSYMBOL_type = 146,                     /* type  */
  YYSYMBOL_tagged_type = 147,              /* tagged_type  */
  YYSYMBOL_member = 148,                   /* member  */
  YYSYMBOL_string_literal = 149,           /* string_literal  */
  YYSYMBOL_string_list = 150,              /* string_list  */
  YYSYMBOL_const_initializer = 151,        /* const_initializer  */
  YYSYMBOL_const_def = 152,                /* const_def  */
  YYSYMBOL_keyword = 153                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 61 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 383 "src/Slice/Grammar.cpp"

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
#define YYLAST   1089

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  89
/* YYNRULES -- Number of rules.  */
#define YYNRULES  253
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  360

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   307


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
      56,    57,    63,     2,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    58,    53,
      61,    59,    62,    64,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    54,     2,    55,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   209,   209,   215,   216,   222,   233,   239,   248,   257,
     265,   274,   281,   280,   286,   285,   290,   295,   294,   300,
     299,   304,   309,   308,   314,   313,   318,   323,   322,   328,
     327,   332,   337,   336,   342,   341,   346,   351,   350,   355,
     360,   359,   364,   369,   368,   374,   373,   378,   382,   392,
     391,   430,   429,   507,   511,   522,   533,   532,   558,   566,
     575,   593,   667,   673,   684,   706,   784,   794,   809,   813,
     824,   835,   834,   875,   879,   890,   915,  1005,  1017,  1030,
    1029,  1063,  1099,  1108,  1109,  1115,  1130,  1152,  1153,  1154,
    1158,  1164,  1165,  1171,  1188,  1209,  1234,  1243,  1248,  1257,
    1292,  1327,  1361,  1401,  1400,  1423,  1422,  1445,  1467,  1471,
    1472,  1478,  1482,  1493,  1507,  1506,  1540,  1577,  1614,  1619,
    1624,  1638,  1642,  1651,  1658,  1670,  1682,  1693,  1714,  1720,
    1729,  1740,  1757,  1771,  1791,  1795,  1804,  1820,  1834,  1833,
    1866,  1865,  1884,  1888,  1897,  1898,  1899,  1908,  1917,  1931,
    1945,  1960,  1980,  1984,  2022,  2026,  2030,  2035,  2040,  2049,
    2075,  2076,  2082,  2083,  2089,  2093,  2102,  2103,  2109,  2110,
    2121,  2122,  2123,  2124,  2125,  2126,  2127,  2128,  2129,  2130,
    2131,  2132,  2133,  2134,  2135,  2140,  2144,  2148,  2152,  2160,
    2165,  2176,  2180,  2192,  2197,  2223,  2255,  2279,  2300,  2320,
    2334,  2346,  2357,  2368,  2380,  2386,  2392,  2399,  2411,  2420,
    2429,  2469,  2476,  2483,  2495,  2507,  2524,  2525,  2526,  2527,
    2528,  2529,  2530,  2531,  2532,  2533,  2534,  2535,  2536,  2537,
    2538,  2539,  2540,  2541,  2542,  2543,  2544,  2545,  2546,  2547,
    2548,  2549,  2550,  2551,  2552,  2553,  2554,  2555,  2556,  2557,
    2558,  2559,  2560,  2561
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
  "ICE_CLASS", "ICE_INTERFACE", "ICE_EXCEPTION", "ICE_STRUCT", "ICE_USING",
  "ICE_SEQUENCE", "ICE_DICTIONARY", "ICE_ENUM", "ICE_OUT", "ICE_STREAM",
  "ICE_EXTENDS", "ICE_IMPLEMENTS", "ICE_THROWS", "ICE_VOID", "ICE_BOOL",
  "ICE_BYTE", "ICE_SHORT", "ICE_USHORT", "ICE_INT", "ICE_UINT",
  "ICE_VARINT", "ICE_VARUINT", "ICE_LONG", "ICE_ULONG", "ICE_VARLONG",
  "ICE_VARULONG", "ICE_FLOAT", "ICE_DOUBLE", "ICE_STRING", "ICE_OBJECT",
  "ICE_CONST", "ICE_FALSE", "ICE_TRUE", "ICE_IDEMPOTENT", "ICE_TAG",
  "ICE_OPTIONAL", "ICE_ANYCLASS", "ICE_VALUE", "ICE_UNCHECKED",
  "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENTIFIER", "ICE_SCOPED_IDENTIFIER",
  "ICE_LOCAL_METADATA_OPEN", "ICE_LOCAL_METADATA_CLOSE",
  "ICE_FILE_METADATA_OPEN", "ICE_FILE_METADATA_CLOSE", "BAD_CHAR", "';'",
  "'{'", "'}'", "'('", "')'", "':'", "'='", "','", "'<'", "'>'", "'*'",
  "'?'", "$accept", "start", "opt_semicolon", "file_metadata",
  "local_metadata", "definitions", "definition", "$@1", "$@2", "$@3",
  "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "$@12", "$@13",
  "$@14", "module_def", "@15", "@16", "exception_id", "exception_decl",
  "exception_def", "@17", "exception_extends", "tag", "optional",
  "struct_id", "struct_decl", "struct_def", "@18", "class_name",
  "class_id", "class_decl", "class_def", "@19", "class_extends", "extends",
  "data_member", "data_member_list", "data_members", "return_tuple",
  "return_type", "operation_preamble", "operation", "@20", "@21",
  "operation_list", "interface_id", "interface_decl", "interface_def",
  "@22", "interface_list", "interface_extends", "exception_list",
  "exception", "type_alias_def", "sequence_def", "dictionary_def",
  "enum_start", "enum_id", "enum_def", "@23", "@24", "enum_underlying",
  "enumerator_list_or_empty", "enumerator_list", "enumerator",
  "enumerator_initializer", "qualifier", "parameter", "parameter_list",
  "parameters", "throws", "scoped_name", "unscoped_name", "builtin",
  "type", "tagged_type", "member", "string_literal", "string_list",
  "const_initializer", "const_def", "keyword", YY_NULLPTR
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
     305,   306,   307,    59,   123,   125,    40,    41,    58,    61,
      44,    60,    62,    42,    63
};
#endif

#define YYPACT_NINF (-281)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-169)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -281,    30,    10,  -281,    56,    56,  -281,    81,    56,  -281,
      83,    76,    48,   124,   522,   657,   701,   745,    21,    43,
      65,  -281,    74,   123,    56,  -281,  -281,    25,   102,  -281,
     106,   112,  -281,    79,     7,   140,  -281,    86,   151,  -281,
     157,   165,   172,   789,   170,  -281,   176,  -281,  -281,    56,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,   174,  -281,   178,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,   177,    74,    74,   155,  -281,    93,   182,
    -281,  -281,  -281,   153,   184,   182,   185,   187,   182,  -281,
     153,   188,   182,    78,  -281,   189,   182,   190,   191,   193,
    -281,   194,  -281,  1042,  -281,   182,   196,  -281,   197,   199,
     145,    74,   155,   155,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,   152,
     183,   186,  -281,  -281,   156,   192,    49,  -281,  -281,  -281,
     201,  -281,  -281,  -281,   347,  -281,  -281,   203,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,   198,   205,  -281,  -281,  -281,
    -281,  -281,   -22,  -281,   206,  -281,  -281,  -281,  -281,   204,
     207,   155,   200,   208,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,   211,   162,   347,   210,   215,   216,   950,  1042,  1042,
     212,   388,   218,  -281,   833,   217,   347,    78,   306,   476,
     219,   220,  -281,   -22,   -32,   113,  -281,  -281,  -281,   877,
      74,   162,  -281,  -281,  -281,  -281,  -281,  -281,  -281,   222,
    -281,    84,   105,    56,  -281,  -281,  -281,  -281,   429,  -281,
    -281,  -281,   162,   223,  -281,   213,    61,   224,   225,  -281,
    -281,   109,   226,  -281,  -281,  -281,  -281,   155,  -281,  -281,
     228,  -281,   229,   231,  -281,   232,    99,  -281,  -281,   306,
     254,   263,  -281,    64,     0,   567,    23,   237,  1012,  -281,
     167,  -281,  -281,   230,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,   567,  -281,   -49,   981,  -281,   235,   238,   236,
     981,  -281,   239,   240,   306,  -281,  -281,  -281,  -281,   921,
     242,   245,  -281,   209,  -281,  -281,  -281,  -281,  -281,   209,
    -281,  -281,  -281,  -281,  -281,  -281,   981,   266,  -281,   266,
    -281,   612,  -281,  -281,  -281,   243,  -281,  -281,   612,  -281
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      11,     0,     8,     1,     0,     0,     9,     0,   205,   207,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   135,     8,     0,     0,    10,    12,    55,    26,    27,
      70,    31,    32,    78,    82,    16,    17,   113,    21,    22,
      39,    36,    42,   140,   143,    43,    47,   204,     6,     0,
       5,    48,    49,    51,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    73,   169,     0,    74,   111,   112,    53,    54,
      68,    69,   129,   128,     8,     8,     0,   134,     0,     4,
      83,    84,    56,     0,     0,     4,     0,     0,     4,    79,
       0,     0,     4,     0,   114,     0,     4,     0,     0,     0,
     136,     0,   137,     0,   138,     4,     0,   206,     0,     0,
       0,     8,     0,     0,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   190,
     191,   192,   166,   167,   194,   193,     0,     7,     3,    13,
       0,    58,    25,    28,     0,    30,    33,     0,    81,    15,
      18,   118,   119,   120,   121,   117,     0,    20,    23,    38,
      35,    41,     8,   142,     0,    44,    46,    11,    11,     0,
       0,     0,     0,     0,   185,   186,   187,   188,   195,   196,
     189,     0,     0,     0,     0,    63,    67,     0,     0,     0,
      89,     0,     0,   199,   202,    85,     0,     0,     0,     0,
       0,   144,   148,     8,     8,     8,    75,    76,   127,     0,
       8,     0,   212,   213,   211,   208,   209,   210,   215,     0,
      88,     0,     0,     0,   203,   197,   198,    87,     0,    72,
     200,   201,     0,     0,   116,     0,   158,     0,   149,   151,
     141,     8,     0,    50,    52,   130,   131,     0,   214,    57,
       0,    62,     0,     0,    66,     0,     0,    86,    80,     0,
     154,   155,    98,   158,   158,     0,     0,   108,     0,   115,
       0,   147,   139,     0,    60,    61,    64,    65,     6,   109,
     157,   156,     0,    97,     0,     0,   168,     0,     0,     0,
       0,   160,   162,     0,     0,    95,   152,   150,   153,     0,
       0,     0,    96,   158,    93,    99,   101,   105,   159,   158,
     103,   107,   132,   133,   100,   102,     0,   165,   161,   165,
      94,     0,   106,   104,   164,   124,   125,   126,     0,   123
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -281,  -281,    32,  -281,    -2,    26,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,
    -281,   132,  -281,  -153,  -116,  -281,     2,  -281,  -281,  -281,
    -281,  -230,  -281,  -281,  -281,  -281,    69,  -281,   -56,  -281,
    -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,  -281,    71,
    -281,    34,  -281,  -267,   -33,  -281,  -281,   -41,  -106,  -280,
    -281,  -105,    11,  -208,    67,     1,  -222,  -281,   -12
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   169,     6,   217,     2,    25,   109,   121,   122,
     125,   126,   114,   115,   117,   118,   128,   127,   129,   135,
     136,    26,   138,   139,    27,    28,    29,   170,   112,   218,
     219,    30,    31,    32,   116,    33,    34,    35,    36,   177,
     119,   113,   220,   221,   222,   314,   295,   296,   297,   349,
     347,   267,    37,    38,    39,   186,   184,   124,   354,   355,
      40,    41,    42,    43,    44,    45,   194,   131,   134,   230,
     231,   232,   327,   298,   321,   322,   323,   352,   164,    94,
     165,   223,   224,   225,     9,    10,   248,    46,   357
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       7,   166,    95,    97,    99,   101,    11,   171,   332,   254,
      -2,   333,   290,   291,   178,   317,     4,   185,     5,   278,
     106,   110,   102,   273,   319,   108,     4,   315,   193,   320,
       3,   132,   330,  -146,   200,   290,   291,   202,   203,   110,
     287,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,  -158,
    -158,  -158,  -158,  -158,  -158,  -158,  -158,   313,     4,   309,
       5,  -158,  -158,  -158,  -158,   111,   346,   103,   258,  -158,
    -158,  -158,   320,   290,   291,    47,   290,   291,   292,   -59,
    -163,   292,    12,   111,    13,    14,    15,    16,    17,    18,
      19,    20,    21,   -77,   341,   211,   238,   249,   293,     8,
     110,    51,   142,   143,   104,   258,   247,   334,   212,    24,
     263,   181,   338,   255,   256,    22,   137,   294,   182,   183,
     294,   185,     4,    23,   162,   163,   105,    50,   280,    24,
     162,   163,    48,   -77,   107,   247,    49,   -77,   350,   201,
    -122,   281,   167,    49,   111,   282,   285,   173,   308,   283,
     176,   162,   163,    49,   180,   -24,   247,     4,   188,    49,
     -71,     4,   284,     5,  -145,   -29,   120,   195,   274,   123,
      52,    53,   303,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   199,
     229,   162,   163,   -14,   328,   160,   161,   242,   243,   162,
     163,   162,   163,    24,   -19,   244,   245,   246,   162,   163,
     -37,   326,   261,   162,   163,   204,   205,   269,   -34,   208,
     209,   290,   291,   234,   235,   -40,   266,   276,   133,   -45,
    -168,   229,     7,     7,   140,   168,   141,   172,   277,   174,
     175,   179,   187,   189,   190,   356,   191,   206,   192,   196,
     207,   197,   356,   198,   286,   213,   210,   226,   227,   228,
     233,   236,   239,   250,   237,   257,   289,   310,   240,   229,
     241,   251,   252,   259,   270,   311,   262,   279,   288,   299,
     271,   302,   351,   318,   300,   304,   305,   266,   306,   307,
     324,   335,   329,   337,   336,   312,   264,   340,   344,   339,
     331,   345,   359,   358,   272,   301,   348,   265,   353,   325,
       0,     0,     0,     0,     0,     0,     0,   343,    -8,    -8,
       0,     0,   266,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
       0,     0,     0,    -8,    -8,    -8,    -8,    -8,   214,     0,
       0,     0,    -8,    -8,     4,     0,     0,     0,     0,     0,
       0,  -110,    -8,     0,     0,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,     0,     0,     0,     0,   215,   216,   160,   161,   214,
       0,     0,     0,   162,   163,     4,     0,     0,     0,     0,
       0,     0,   -92,     0,     0,     0,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     158,   159,     0,     0,     0,     0,   215,   216,   160,   161,
     214,     0,     0,     0,   162,   163,     4,     0,     0,     0,
       0,     0,     0,   -91,     0,     0,     0,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,     0,     0,     0,     0,   215,   216,   160,
     161,     0,     0,     0,     0,   162,   163,     4,     0,    54,
      55,    56,    57,    58,   -90,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,   268,     0,    24,    54,    55,    56,    57,    58,
       0,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,    92,    93,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,   316,    93,    54,    55,    56,    57,    58,
       0,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,   162,   163,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,    96,    54,    55,    56,    57,    58,     0,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,    98,    54,    55,
      56,    57,    58,     0,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,     0,     0,     0,
       0,   100,    54,    55,    56,    57,    58,     0,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,     0,     0,     0,     0,   130,    54,    55,    56,    57,
      58,     0,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,     0,     0,     0,     0,   260,
      54,    55,    56,    57,    58,     0,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,     0,
       0,     0,     0,   275,    54,    55,    56,    57,    58,     0,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,   342,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,     0,     0,     0,     0,   215,   216,
     160,   161,     0,     0,     0,     0,   162,   163,   253,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,     0,     0,     0,     0,   215,
     216,   160,   161,     0,     0,     0,     0,   162,   163,     4,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,     0,     0,     0,     0,
     215,   216,   160,   161,     0,     0,     0,     0,   162,   163,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,     0,     0,     0,     0,
       0,     0,   160,   161,     0,     0,     0,     0,   162,   163
};

static const yytype_int16 yycheck[] =
{
       2,   106,    14,    15,    16,    17,     5,   113,    57,   217,
       0,    60,    12,    13,   120,   295,    48,   123,    50,   241,
      22,    14,     1,    55,     1,    24,    48,   294,   133,   296,
       0,    43,   312,    55,   140,    12,    13,   142,   143,    14,
     262,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    57,    48,   289,
      50,    38,    39,    40,    41,    58,   333,    46,   221,    46,
      47,    48,   339,    12,    13,     8,    12,    13,    17,    54,
      57,    17,     1,    58,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    14,   324,    46,   201,   213,    37,    43,
      14,    53,   104,   105,    61,   258,   212,   315,    59,    48,
     226,    33,   320,   218,   219,    34,    49,    56,    40,    41,
      56,   227,    48,    42,    46,    47,    61,    51,    44,    48,
      46,    47,    49,    54,    11,   241,    60,    58,   346,   141,
      54,    57,    49,    60,    58,   251,   252,   115,    49,    44,
     118,    46,    47,    60,   122,    53,   262,    48,   126,    60,
      54,    48,    57,    50,    55,    53,    34,   135,    55,    37,
      46,    47,   277,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    44,
     192,    46,    47,    53,   300,    40,    41,    35,    36,    46,
      47,    46,    47,    48,    53,    43,    44,    45,    46,    47,
      53,    44,   224,    46,    47,    63,    64,   229,    53,    63,
      64,    12,    13,   197,   198,    53,   228,   239,    58,    53,
      56,   233,   234,   235,    56,    53,    59,    53,   240,    54,
      53,    53,    53,    53,    53,   351,    53,    64,    54,    53,
      64,    54,   358,    54,   253,    54,    64,    54,    60,    54,
      54,    57,    62,    53,    57,    53,    53,    13,    60,   271,
      59,    56,    56,    55,    55,    12,    59,    55,    55,    55,
      60,    55,    16,   295,    59,    57,    57,   289,    57,    57,
      53,    56,    62,    57,    56,   293,   227,    57,    56,    60,
     312,    56,   358,    60,   233,   271,   339,     1,   349,   298,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   329,    12,    13,
      -1,    -1,   324,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    -1,    -1,    37,    38,    39,    40,    41,     1,    -1,
      -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,    -1,
      -1,    55,    56,    -1,    -1,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    -1,    -1,    -1,    38,    39,    40,    41,     1,
      -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,    -1,
      -1,    -1,    55,    -1,    -1,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    -1,    -1,    -1,    38,    39,    40,    41,
       1,    -1,    -1,    -1,    46,    47,    48,    -1,    -1,    -1,
      -1,    -1,    -1,    55,    -1,    -1,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    -1,    -1,    -1,    -1,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,    47,    48,    -1,     3,
       4,     5,     6,     7,    55,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    -1,    -1,
      -1,    -1,    46,    -1,    48,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,    47,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    -1,    -1,    -1,
      -1,    46,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    -1,    -1,    -1,    -1,    46,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    -1,
      -1,    -1,    -1,    46,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    -1,    -1,    -1,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    -1,    -1,    -1,    38,
      39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,    48,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    -1,    -1,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    46,    47,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    41,    -1,    -1,    -1,    -1,    46,    47
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    66,    70,     0,    48,    50,    68,    69,    43,   149,
     150,   150,     1,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    34,    42,    48,    71,    86,    89,    90,    91,
      96,    97,    98,   100,   101,   102,   103,   117,   118,   119,
     125,   126,   127,   128,   129,   130,   152,   149,    49,    60,
      51,    53,    46,    47,     3,     4,     5,     6,     7,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    46,    47,   144,   153,    46,   153,    46,   153,
      46,   153,     1,    46,    61,    61,    69,    11,   150,    72,
      14,    58,    93,   106,    77,    78,    99,    79,    80,   105,
     106,    73,    74,   106,   122,    75,    76,    82,    81,    83,
      46,   132,   153,    58,   133,    84,    85,   149,    87,    88,
      56,    59,    69,    69,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      40,    41,    46,    47,   143,   145,   146,    49,    53,    67,
      92,   143,    53,    67,    54,    53,    67,   104,   143,    53,
      67,    33,    40,    41,   121,   143,   120,    53,    67,    53,
      53,    53,    54,   146,   131,    67,    53,    54,    54,    44,
     143,    69,   146,   146,    63,    64,    64,    64,    63,    64,
      64,    46,    59,    54,     1,    38,    39,    69,    94,    95,
     107,   108,   109,   146,   147,   148,    54,    60,    54,    69,
     134,   135,   136,    54,    70,    70,    57,    57,   146,    62,
      60,    59,    35,    36,    43,    44,    45,   143,   151,   109,
      53,    56,    56,    48,   148,   146,   146,    53,   108,    55,
      46,   153,    59,   109,   121,     1,    69,   116,    46,   153,
      55,    60,   134,    55,    55,    46,   153,    69,   151,    55,
      44,    57,   143,    44,    57,   143,   150,   151,    55,    53,
      12,    13,    17,    37,    56,   111,   112,   113,   138,    55,
      59,   136,    55,   146,    57,    57,    57,    57,    49,   116,
      13,    12,   111,    57,   110,   138,    46,   144,   153,     1,
     138,   139,   140,   141,    53,   147,    44,   137,   143,    62,
     144,   153,    57,    60,   148,    56,    56,    57,   148,    60,
      57,   116,    46,   153,    56,    56,   138,   115,   139,   114,
     148,    16,   142,   142,   123,   124,   143,   153,    60,   123
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    67,    67,    68,    69,    69,    69,    70,
      70,    70,    72,    71,    73,    71,    71,    74,    71,    75,
      71,    71,    76,    71,    77,    71,    71,    78,    71,    79,
      71,    71,    80,    71,    81,    71,    71,    82,    71,    71,
      83,    71,    71,    84,    71,    85,    71,    71,    71,    87,
      86,    88,    86,    89,    89,    90,    92,    91,    93,    93,
      94,    94,    94,    94,    95,    95,    95,    95,    96,    96,
      97,    99,    98,   100,   100,   101,   101,   101,   102,   104,
     103,   105,   105,   106,   106,   107,   107,   108,   108,   108,
     108,   109,   109,   110,   110,   111,   111,   111,   111,   112,
     112,   112,   112,   114,   113,   115,   113,   116,   116,   116,
     116,   117,   117,   118,   120,   119,   121,   121,   121,   121,
     121,   122,   122,   123,   123,   124,   124,   125,   125,   125,
     126,   126,   127,   127,   128,   128,   129,   129,   131,   130,
     132,   130,   133,   133,   134,   134,   134,   135,   135,   136,
     136,   136,   137,   137,   138,   138,   138,   138,   138,   139,
     140,   140,   141,   141,   142,   142,   143,   143,   144,   144,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   146,   146,   146,   147,   147,   147,
     148,   148,   148,   148,   149,   149,   150,   150,   151,   151,
     151,   151,   151,   151,   152,   152,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     4,     0,     2,
       3,     0,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     1,
       0,     3,     1,     0,     3,     0,     3,     1,     2,     0,
       6,     0,     6,     2,     2,     1,     0,     6,     2,     0,
       4,     4,     3,     1,     4,     4,     3,     1,     2,     2,
       1,     0,     5,     2,     2,     5,     5,     1,     1,     0,
       6,     2,     0,     1,     1,     1,     3,     2,     2,     1,
       2,     1,     0,     2,     4,     2,     3,     2,     1,     3,
       4,     3,     4,     0,     5,     0,     5,     4,     2,     3,
       0,     2,     2,     1,     0,     6,     3,     1,     1,     1,
       1,     2,     0,     3,     1,     1,     1,     5,     2,     2,
       6,     6,     9,     9,     2,     1,     2,     2,     0,     6,
       0,     5,     2,     0,     1,     2,     0,     3,     1,     2,
       4,     2,     1,     1,     1,     1,     2,     2,     0,     2,
       1,     3,     1,     0,     2,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     1,     2,     2,     2,     2,     1,
       2,     2,     1,     2,     2,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     6,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
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
#line 223 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    convertMetadata(metadata);
    yyval = metadata;
}
#line 1881 "src/Slice/Grammar.cpp"
    break;

  case 6: /* local_metadata: ICE_LOCAL_METADATA_OPEN string_list ICE_LOCAL_METADATA_CLOSE  */
#line 234 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    convertMetadata(metadata);
    yyval = metadata;
}
#line 1891 "src/Slice/Grammar.cpp"
    break;

  case 7: /* local_metadata: local_metadata ICE_LOCAL_METADATA_OPEN string_list ICE_LOCAL_METADATA_CLOSE  */
#line 240 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata1 = StringListTokPtr::dynamicCast(yyvsp[-3]);
    StringListTokPtr metadata2 = StringListTokPtr::dynamicCast(yyvsp[-1]);
    metadata1->v.splice(metadata1->v.end(), metadata2->v);

    convertMetadata(metadata1);
    yyval = metadata1;
}
#line 1904 "src/Slice/Grammar.cpp"
    break;

  case 8: /* local_metadata: %empty  */
#line 249 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 1912 "src/Slice/Grammar.cpp"
    break;

  case 9: /* definitions: definitions file_metadata  */
#line 258 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metadata->v.empty())
    {
        unit->addFileMetadata(metadata->v);
    }
}
#line 1924 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: definitions local_metadata definition  */
#line 266 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metadata->v.empty())
    {
        contained->setMetadata(metadata->v);
    }
}
#line 1937 "src/Slice/Grammar.cpp"
    break;

  case 12: /* $@1: %empty  */
#line 281 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1945 "src/Slice/Grammar.cpp"
    break;

  case 14: /* $@2: %empty  */
#line 286 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1953 "src/Slice/Grammar.cpp"
    break;

  case 16: /* definition: class_decl  */
#line 291 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 1961 "src/Slice/Grammar.cpp"
    break;

  case 17: /* $@3: %empty  */
#line 295 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1969 "src/Slice/Grammar.cpp"
    break;

  case 19: /* $@4: %empty  */
#line 300 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1977 "src/Slice/Grammar.cpp"
    break;

  case 21: /* definition: interface_decl  */
#line 305 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 1985 "src/Slice/Grammar.cpp"
    break;

  case 22: /* $@5: %empty  */
#line 309 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDefPtr::dynamicCast(yyvsp[0]));
}
#line 1993 "src/Slice/Grammar.cpp"
    break;

  case 24: /* $@6: %empty  */
#line 314 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2001 "src/Slice/Grammar.cpp"
    break;

  case 26: /* definition: exception_decl  */
#line 319 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 2009 "src/Slice/Grammar.cpp"
    break;

  case 27: /* $@7: %empty  */
#line 323 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 2017 "src/Slice/Grammar.cpp"
    break;

  case 29: /* $@8: %empty  */
#line 328 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2025 "src/Slice/Grammar.cpp"
    break;

  case 31: /* definition: struct_decl  */
#line 333 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 2033 "src/Slice/Grammar.cpp"
    break;

  case 32: /* $@9: %empty  */
#line 337 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 2041 "src/Slice/Grammar.cpp"
    break;

  case 34: /* $@10: %empty  */
#line 342 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2049 "src/Slice/Grammar.cpp"
    break;

  case 36: /* definition: sequence_def  */
#line 347 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2057 "src/Slice/Grammar.cpp"
    break;

  case 37: /* $@11: %empty  */
#line 351 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || TypeAliasPtr::dynamicCast(yyvsp[0]));
}
#line 2065 "src/Slice/Grammar.cpp"
    break;

  case 39: /* definition: type_alias_def  */
#line 356 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after type-alias");
}
#line 2073 "src/Slice/Grammar.cpp"
    break;

  case 40: /* $@12: %empty  */
#line 360 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2081 "src/Slice/Grammar.cpp"
    break;

  case 42: /* definition: dictionary_def  */
#line 365 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2089 "src/Slice/Grammar.cpp"
    break;

  case 43: /* $@13: %empty  */
#line 369 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2097 "src/Slice/Grammar.cpp"
    break;

  case 45: /* $@14: %empty  */
#line 374 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2105 "src/Slice/Grammar.cpp"
    break;

  case 47: /* definition: const_def  */
#line 379 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2113 "src/Slice/Grammar.cpp"
    break;

  case 48: /* definition: error ';'  */
#line 383 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2121 "src/Slice/Grammar.cpp"
    break;

  case 49: /* @15: %empty  */
#line 392 "src/Slice/Grammar.y"
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
#line 2151 "src/Slice/Grammar.cpp"
    break;

  case 50: /* module_def: ICE_MODULE ICE_IDENTIFIER @15 '{' definitions '}'  */
#line 418 "src/Slice/Grammar.y"
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
#line 2167 "src/Slice/Grammar.cpp"
    break;

  case 51: /* @16: %empty  */
#line 430 "src/Slice/Grammar.y"
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
#line 2226 "src/Slice/Grammar.cpp"
    break;

  case 52: /* module_def: ICE_MODULE ICE_SCOPED_IDENTIFIER @16 '{' definitions '}'  */
#line 485 "src/Slice/Grammar.y"
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
#line 2248 "src/Slice/Grammar.cpp"
    break;

  case 53: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 508 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2256 "src/Slice/Grammar.cpp"
    break;

  case 54: /* exception_id: ICE_EXCEPTION keyword  */
#line 512 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2266 "src/Slice/Grammar.cpp"
    break;

  case 55: /* exception_decl: exception_id  */
#line 523 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2275 "src/Slice/Grammar.cpp"
    break;

  case 56: /* @17: %empty  */
#line 533 "src/Slice/Grammar.y"
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
#line 2292 "src/Slice/Grammar.cpp"
    break;

  case 57: /* exception_def: exception_id exception_extends @17 '{' data_members '}'  */
#line 546 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2304 "src/Slice/Grammar.cpp"
    break;

  case 58: /* exception_extends: extends scoped_name  */
#line 559 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2316 "src/Slice/Grammar.cpp"
    break;

  case 59: /* exception_extends: %empty  */
#line 567 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2324 "src/Slice/Grammar.cpp"
    break;

  case 60: /* tag: ICE_TAG '(' ICE_INTEGER_LITERAL ')'  */
#line 576 "src/Slice/Grammar.y"
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
#line 2346 "src/Slice/Grammar.cpp"
    break;

  case 61: /* tag: ICE_TAG '(' scoped_name ')'  */
#line 594 "src/Slice/Grammar.y"
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
#line 2424 "src/Slice/Grammar.cpp"
    break;

  case 62: /* tag: ICE_TAG '(' ')'  */
#line 668 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2434 "src/Slice/Grammar.cpp"
    break;

  case 63: /* tag: ICE_TAG  */
#line 674 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2444 "src/Slice/Grammar.cpp"
    break;

  case 64: /* optional: ICE_OPTIONAL '(' ICE_INTEGER_LITERAL ')'  */
#line 685 "src/Slice/Grammar.y"
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
#line 2470 "src/Slice/Grammar.cpp"
    break;

  case 65: /* optional: ICE_OPTIONAL '(' scoped_name ')'  */
#line 707 "src/Slice/Grammar.y"
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
#line 2552 "src/Slice/Grammar.cpp"
    break;

  case 66: /* optional: ICE_OPTIONAL '(' ')'  */
#line 785 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2566 "src/Slice/Grammar.cpp"
    break;

  case 67: /* optional: ICE_OPTIONAL  */
#line 795 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2580 "src/Slice/Grammar.cpp"
    break;

  case 68: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 810 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2588 "src/Slice/Grammar.cpp"
    break;

  case 69: /* struct_id: ICE_STRUCT keyword  */
#line 814 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2598 "src/Slice/Grammar.cpp"
    break;

  case 70: /* struct_decl: struct_id  */
#line 825 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2607 "src/Slice/Grammar.cpp"
    break;

  case 71: /* @18: %empty  */
#line 835 "src/Slice/Grammar.y"
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
        st = cont->createStruct(IceUtil::generateUUID(), Dummy);
        assert(st);
        unit->pushContainer(st);
    }
    yyval = st;
}
#line 2629 "src/Slice/Grammar.cpp"
    break;

  case 72: /* struct_def: struct_id @18 '{' data_members '}'  */
#line 853 "src/Slice/Grammar.y"
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
#line 2651 "src/Slice/Grammar.cpp"
    break;

  case 73: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 876 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2659 "src/Slice/Grammar.cpp"
    break;

  case 74: /* class_name: ICE_CLASS keyword  */
#line 880 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2669 "src/Slice/Grammar.cpp"
    break;

  case 75: /* class_id: ICE_CLASS unscoped_name '(' ICE_INTEGER_LITERAL ')'  */
#line 891 "src/Slice/Grammar.y"
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
    classId->v = StringTokPtr::dynamicCast(yyvsp[-3])->v;
    classId->t = static_cast<int>(id);
    yyval = classId;
}
#line 2698 "src/Slice/Grammar.cpp"
    break;

  case 76: /* class_id: ICE_CLASS unscoped_name '(' scoped_name ')'  */
#line 916 "src/Slice/Grammar.y"
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
    classId->v = StringTokPtr::dynamicCast(yyvsp[-3])->v;
    classId->t = id;
    yyval = classId;

}
#line 2792 "src/Slice/Grammar.cpp"
    break;

  case 77: /* class_id: class_name  */
#line 1006 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2803 "src/Slice/Grammar.cpp"
    break;

  case 78: /* class_decl: class_name  */
#line 1018 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2814 "src/Slice/Grammar.cpp"
    break;

  case 79: /* @19: %empty  */
#line 1030 "src/Slice/Grammar.y"
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
#line 2835 "src/Slice/Grammar.cpp"
    break;

  case 80: /* class_def: class_id class_extends @19 '{' data_members '}'  */
#line 1047 "src/Slice/Grammar.y"
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
#line 2851 "src/Slice/Grammar.cpp"
    break;

  case 81: /* class_extends: extends scoped_name  */
#line 1064 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    yyval = 0;
    if (!types.empty())
    {
        ClassDeclPtr cl = ClassDeclPtr::dynamicCast(types.front());
        if (!cl)
        {
            if (auto alias = TypeAliasPtr::dynamicCast(types.front()))
            {
                cl = ClassDeclPtr::dynamicCast(alias->underlying());
            }
        }

        if (cl)
        {
            ClassDefPtr def = cl->definition();
            if (!def)
            {
                unit->error("`" + scoped->v + "' has been declared but not defined");
            }
            else
            {
                cont->checkIntroduced(scoped->v);
                yyval = def;
            }
        }
        else
        {
            unit->error("`" + scoped->v + "' is not a class");
        }
    }
}
#line 2891 "src/Slice/Grammar.cpp"
    break;

  case 82: /* class_extends: %empty  */
#line 1100 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2899 "src/Slice/Grammar.cpp"
    break;

  case 85: /* data_member: member  */
#line 1116 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);

    // Check if the container was created successfully. If it wasn't skip creating the data member and continue parsing.
    if (DataMemberContainerPtr cont = DataMemberContainerPtr::dynamicCast(unit->currentContainer()))
    {
        MemberPtr dm = cont->createDataMember(def->name, def->type, def->isTagged, def->tag);
        unit->currentContainer()->checkIntroduced(def->name, dm);
        if (dm && !def->metadata.empty())
        {
            dm->setMetadata(def->metadata);
        }
    }
}
#line 2918 "src/Slice/Grammar.cpp"
    break;

  case 86: /* data_member: member '=' const_initializer  */
#line 1131 "src/Slice/Grammar.y"
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
            dm->setMetadata(def->metadata);
        }
    }
}
#line 2939 "src/Slice/Grammar.cpp"
    break;

  case 89: /* data_member_list: data_member  */
#line 1155 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2947 "src/Slice/Grammar.cpp"
    break;

  case 93: /* return_tuple: qualifier member  */
#line 1172 "src/Slice/Grammar.y"
{
    IntegerTokPtr qualifier = IntegerTokPtr::dynamicCast(yyvsp[-1]);
    TaggedDefTokPtr returnMember = TaggedDefTokPtr::dynamicCast(yyvsp[0]);

    returnMember->isStream = qualifier->v & QUALIFIER_STREAM;

    bool isOutParam = qualifier->v & QUALIFIER_OUT;
    if (isOutParam)
    {
        unit->error("`" + returnMember->name + "': return members cannot be marked as out");
    }

    TaggedDefListTokPtr returnMembers = new TaggedDefListTok();
    returnMembers->v.push_back(returnMember);
    yyval = returnMembers;
}
#line 2968 "src/Slice/Grammar.cpp"
    break;

  case 94: /* return_tuple: return_tuple ',' qualifier member  */
#line 1189 "src/Slice/Grammar.y"
{
    IntegerTokPtr qualifier = IntegerTokPtr::dynamicCast(yyvsp[-1]);
    TaggedDefTokPtr returnMember = TaggedDefTokPtr::dynamicCast(yyvsp[0]);

    returnMember->isStream = qualifier->v & QUALIFIER_STREAM;

    bool isOutParam = qualifier->v & QUALIFIER_OUT;
    if (isOutParam)
    {
        unit->error("`" + returnMember->name + "': return members cannot be marked as out");
    }

    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-3]);
    returnMembers->v.push_back(returnMember);
    yyval = returnMembers;
}
#line 2989 "src/Slice/Grammar.cpp"
    break;

  case 95: /* return_type: qualifier tagged_type  */
#line 1210 "src/Slice/Grammar.y"
{
    IntegerTokPtr qualifier = IntegerTokPtr::dynamicCast(yyvsp[-1]);
    TaggedDefTokPtr returnMember = TaggedDefTokPtr::dynamicCast(yyvsp[0]);

    // For unnamed return types we infer their name to be 'returnValue'.
    returnMember->name = "returnValue";

    returnMember->isStream = qualifier->v & QUALIFIER_STREAM;

    bool isOutParam = qualifier->v & QUALIFIER_OUT;
    if (isOutParam)
    {
        unit->error("return value cannot be marked as out");
    }

    if (returnMember->isTagged)
    {
        checkForTaggableType(returnMember->type);
    }

    TaggedDefListTokPtr returnMembers = new TaggedDefListTok();
    returnMembers->v.push_back(returnMember);
    yyval = returnMembers;
}
#line 3018 "src/Slice/Grammar.cpp"
    break;

  case 96: /* return_type: '(' return_tuple ')'  */
#line 1235 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-1]);
    if (returnMembers->v.size() == 1)
    {
        unit->error("return tuples must contain at least 2 elements");
    }
    yyval = yyvsp[-1];
}
#line 3031 "src/Slice/Grammar.cpp"
    break;

  case 97: /* return_type: '(' ')'  */
#line 1244 "src/Slice/Grammar.y"
{
    unit->error("return tuples must contain at least 2 elements");
    yyval = new TaggedDefListTok();
}
#line 3040 "src/Slice/Grammar.cpp"
    break;

  case 98: /* return_type: ICE_VOID  */
#line 1249 "src/Slice/Grammar.y"
{
    yyval = new TaggedDefListTok();
}
#line 3048 "src/Slice/Grammar.cpp"
    break;

  case 99: /* operation_preamble: return_type unscoped_name '('  */
#line 1258 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-2]);
    string name = StringTokPtr::dynamicCast(yyvsp[-1])->v;

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
                                                     returnMember->tag, returnMember->isStream);
                if (p && !returnMember->metadata.empty())
                {
                    p->setMetadata(returnMember->metadata);
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
#line 3087 "src/Slice/Grammar.cpp"
    break;

  case 100: /* operation_preamble: ICE_IDEMPOTENT return_type unscoped_name '('  */
#line 1293 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-2]);
    string name = StringTokPtr::dynamicCast(yyvsp[-1])->v;

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
                                                     returnMember->tag, returnMember->isStream);
                if (p && !returnMember->metadata.empty())
                {
                    p->setMetadata(returnMember->metadata);
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
#line 3126 "src/Slice/Grammar.cpp"
    break;

  case 101: /* operation_preamble: return_type keyword '('  */
#line 1328 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-2]);
    string name = StringTokPtr::dynamicCast(yyvsp[-1])->v;
    if (InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer()))
    {
        if (OperationPtr op = interface->createOperation(name))
        {
            unit->pushContainer(op);

            // Set the return members for the operation.
            for (const auto& returnMember : returnMembers->v)
            {
                MemberPtr p = op->createReturnMember(returnMember->name, returnMember->type, returnMember->isTagged,
                                                     returnMember->tag, returnMember->isStream);
                if (p && !returnMember->metadata.empty())
                {
                    p->setMetadata(returnMember->metadata);
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
#line 3164 "src/Slice/Grammar.cpp"
    break;

  case 102: /* operation_preamble: ICE_IDEMPOTENT return_type keyword '('  */
#line 1362 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-2]);
    string name = StringTokPtr::dynamicCast(yyvsp[-1])->v;
    if (InterfaceDefPtr interface = InterfaceDefPtr::dynamicCast(unit->currentContainer()))
    {
        if (OperationPtr op = interface->createOperation(name, Operation::Idempotent))
        {
            unit->pushContainer(op);

            // Set the return members for the operation.
            for (const auto& returnMember : returnMembers->v)
            {
                MemberPtr p = op->createReturnMember(returnMember->name, returnMember->type, returnMember->isTagged,
                                                     returnMember->tag, returnMember->isStream);
                if (p && !returnMember->metadata.empty())
                {
                    p->setMetadata(returnMember->metadata);
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
#line 3202 "src/Slice/Grammar.cpp"
    break;

  case 103: /* @20: %empty  */
#line 1401 "src/Slice/Grammar.y"
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
#line 3218 "src/Slice/Grammar.cpp"
    break;

  case 104: /* operation: operation_preamble parameters ')' @20 throws  */
#line 1413 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3232 "src/Slice/Grammar.cpp"
    break;

  case 105: /* @21: %empty  */
#line 1423 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3244 "src/Slice/Grammar.cpp"
    break;

  case 106: /* operation: operation_preamble error ')' @21 throws  */
#line 1431 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3258 "src/Slice/Grammar.cpp"
    break;

  case 107: /* operation_list: local_metadata operation ';' operation_list  */
#line 1446 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    OperationPtr operation = OperationPtr::dynamicCast(yyvsp[-2]);
    if (operation && !metadata->v.empty())
    {
        operation->setMetadata(metadata->v);

        // If the operation had a single return type (not a return tuple), also apply the metadata to the return type.
        // TODO: once we introduce more concrete metadata validation, we could sort the metadata out between the return
        // type and the operation itself. So metadata relevant to operations would only be set for the operation, and
        // metadata only relevant to the return type would only be set on the return type.
        if (operation->hasSingleReturnType())
        {
            MemberPtr returnType = operation->returnType().front();
            StringList returnMetadata = returnType->getAllMetadata();

            // Merge any metadata specified on the operation into the return type.
            returnType->setMetadata(appendMetadata(metadata->v, returnMetadata));
        }
    }
}
#line 3284 "src/Slice/Grammar.cpp"
    break;

  case 108: /* operation_list: local_metadata operation  */
#line 1468 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3292 "src/Slice/Grammar.cpp"
    break;

  case 111: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1479 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3300 "src/Slice/Grammar.cpp"
    break;

  case 112: /* interface_id: ICE_INTERFACE keyword  */
#line 1483 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3310 "src/Slice/Grammar.cpp"
    break;

  case 113: /* interface_decl: interface_id  */
#line 1494 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3322 "src/Slice/Grammar.cpp"
    break;

  case 114: /* @22: %empty  */
#line 1507 "src/Slice/Grammar.y"
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
#line 3343 "src/Slice/Grammar.cpp"
    break;

  case 115: /* interface_def: interface_id interface_extends @22 '{' operation_list '}'  */
#line 1524 "src/Slice/Grammar.y"
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
#line 3359 "src/Slice/Grammar.cpp"
    break;

  case 116: /* interface_list: scoped_name ',' interface_list  */
#line 1541 "src/Slice/Grammar.y"
{
    InterfaceListTokPtr intfs = InterfaceListTokPtr::dynamicCast(yyvsp[0]);
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if (!types.empty())
    {
        InterfaceDeclPtr interface = InterfaceDeclPtr::dynamicCast(types.front());
        if (!interface)
        {
            if (auto alias = TypeAliasPtr::dynamicCast(types.front()))
            {
                interface = InterfaceDeclPtr::dynamicCast(alias->underlying());
            }
        }

        if (interface)
        {
            InterfaceDefPtr def = interface->definition();
            if (!def)
            {
                unit->error("`" + scoped->v + "' has been declared but not defined");
            }
            else
            {
                cont->checkIntroduced(scoped->v);
                intfs->v.push_front(def);
            }
        }
        else
        {
            unit->error("`" + scoped->v + "' is not an interface");
        }
    }
    yyval = intfs;
}
#line 3400 "src/Slice/Grammar.cpp"
    break;

  case 117: /* interface_list: scoped_name  */
#line 1578 "src/Slice/Grammar.y"
{
    InterfaceListTokPtr intfs = new InterfaceListTok;
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    TypeList types = cont->lookupType(scoped->v);
    if (!types.empty())
    {
        InterfaceDeclPtr interface = InterfaceDeclPtr::dynamicCast(types.front());
        if (!interface)
        {
            if (auto alias = TypeAliasPtr::dynamicCast(types.front()))
            {
                interface = InterfaceDeclPtr::dynamicCast(alias->underlying());
            }
        }

        if (interface)
        {
            InterfaceDefPtr def = interface->definition();
            if (!def)
            {
                unit->error("`" + scoped->v + "' has been declared but not defined"); // $$ is a dummy
            }
            else
            {
                cont->checkIntroduced(scoped->v);
                intfs->v.push_front(def);
            }
        }
        else
        {
            unit->error("`" + scoped->v + "' is not an interface"); // $$ is a dummy
        }
    }
    yyval = intfs;
}
#line 3441 "src/Slice/Grammar.cpp"
    break;

  case 118: /* interface_list: ICE_OBJECT  */
#line 1615 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new InterfaceListTok; // Dummy
}
#line 3450 "src/Slice/Grammar.cpp"
    break;

  case 119: /* interface_list: ICE_ANYCLASS  */
#line 1620 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type AnyClass");
    yyval = new ClassListTok; // Dummy
}
#line 3459 "src/Slice/Grammar.cpp"
    break;

  case 120: /* interface_list: ICE_VALUE  */
#line 1625 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3472 "src/Slice/Grammar.cpp"
    break;

  case 121: /* interface_extends: extends interface_list  */
#line 1639 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3480 "src/Slice/Grammar.cpp"
    break;

  case 122: /* interface_extends: %empty  */
#line 1643 "src/Slice/Grammar.y"
{
    yyval = new InterfaceListTok;
}
#line 3488 "src/Slice/Grammar.cpp"
    break;

  case 123: /* exception_list: exception ',' exception_list  */
#line 1652 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3499 "src/Slice/Grammar.cpp"
    break;

  case 124: /* exception_list: exception  */
#line 1659 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3510 "src/Slice/Grammar.cpp"
    break;

  case 125: /* exception: scoped_name  */
#line 1671 "src/Slice/Grammar.y"
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
#line 3526 "src/Slice/Grammar.cpp"
    break;

  case 126: /* exception: keyword  */
#line 1683 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentModule()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3536 "src/Slice/Grammar.cpp"
    break;

  case 127: /* type_alias_def: ICE_USING ICE_IDENTIFIER '=' local_metadata type  */
#line 1694 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-3]);
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);

    if (TypePtr type = TypePtr::dynamicCast(yyvsp[0]))
    {
        if (auto alias = TypeAliasPtr::dynamicCast(type))
        {
            appendMetadataInPlace(metadata->v, alias->typeMetadata());
            type = alias->underlying();
        }

        ModulePtr cont = unit->currentModule();
        yyval = cont->createTypeAlias(ident->v, type, metadata->v);
    }
    else
    {
        yyval = nullptr;
    }
}
#line 3561 "src/Slice/Grammar.cpp"
    break;

  case 128: /* type_alias_def: ICE_USING ICE_IDENTIFIER  */
#line 1715 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing underlying type for typealias `" + ident->v + "'");
    yyval = nullptr;
}
#line 3571 "src/Slice/Grammar.cpp"
    break;

  case 129: /* type_alias_def: ICE_USING error  */
#line 1721 "src/Slice/Grammar.y"
{
    unit->error("unable to resolve underlying type");
    yyval = nullptr;
}
#line 3580 "src/Slice/Grammar.cpp"
    break;

  case 130: /* sequence_def: ICE_SEQUENCE '<' local_metadata type '>' ICE_IDENTIFIER  */
#line 1730 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);

    unalias(type, metadata->v);

    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metadata->v);
}
#line 3595 "src/Slice/Grammar.cpp"
    break;

  case 131: /* sequence_def: ICE_SEQUENCE '<' local_metadata type '>' keyword  */
#line 1741 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);

    unalias(type, metadata->v);

    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metadata->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3611 "src/Slice/Grammar.cpp"
    break;

  case 132: /* dictionary_def: ICE_DICTIONARY '<' local_metadata type ',' local_metadata type '>' ICE_IDENTIFIER  */
#line 1758 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetadata = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);

    unalias(keyType, keyMetadata->v);
    unalias(valueType, valueMetadata->v);

    ModulePtr cont = unit->currentModule();
    yyval = cont->createDictionary(ident->v, keyType, keyMetadata->v, valueType, valueMetadata->v);
}
#line 3629 "src/Slice/Grammar.cpp"
    break;

  case 133: /* dictionary_def: ICE_DICTIONARY '<' local_metadata type ',' local_metadata type '>' keyword  */
#line 1772 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetadata = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);

    unalias(keyType, keyMetadata->v);
    unalias(valueType, valueMetadata->v);

    ModulePtr cont = unit->currentModule();
    yyval = cont->createDictionary(ident->v, keyType, keyMetadata->v, valueType, valueMetadata->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3648 "src/Slice/Grammar.cpp"
    break;

  case 134: /* enum_start: ICE_UNCHECKED ICE_ENUM  */
#line 1792 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(true);
}
#line 3656 "src/Slice/Grammar.cpp"
    break;

  case 135: /* enum_start: ICE_ENUM  */
#line 1796 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(false);
}
#line 3664 "src/Slice/Grammar.cpp"
    break;

  case 136: /* enum_id: enum_start ICE_IDENTIFIER  */
#line 1805 "src/Slice/Grammar.y"
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
#line 3684 "src/Slice/Grammar.cpp"
    break;

  case 137: /* enum_id: enum_start keyword  */
#line 1821 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[-1])->v;
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
}
#line 3696 "src/Slice/Grammar.cpp"
    break;

  case 138: /* @23: %empty  */
#line 1834 "src/Slice/Grammar.y"
{
    EnumPtr en = EnumPtr::dynamicCast(yyvsp[-1]);
    TypePtr underlying = TypePtr::dynamicCast(yyvsp[0]);

    if (auto alias = TypeAliasPtr::dynamicCast(underlying))
    {
        if (!alias->typeMetadata().empty())
        {
            unit->error("illegal metadata: typealias metadata `" + alias->typeMetadata().front() +
                        "' cannot be used in enum declarations");
        }
        underlying = alias->underlying();
    }

    en->initUnderlying(underlying);
    unit->pushContainer(en);
    yyval = en;
}
#line 3719 "src/Slice/Grammar.cpp"
    break;

  case 139: /* enum_def: enum_id enum_underlying @23 '{' enumerator_list_or_empty '}'  */
#line 1853 "src/Slice/Grammar.y"
{
    if (EnumPtr en = EnumPtr::dynamicCast(yyvsp[-3]))
    {
        EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast(yyvsp[-1]);
        if (enumerators->v.empty() && (!en->underlying() || !en->isUnchecked()))
        {
            unit->error("enum `" + en->name() + "' must have at least one enumerator");
        }
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 3736 "src/Slice/Grammar.cpp"
    break;

  case 140: /* @24: %empty  */
#line 1866 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[0])->v;
    unit->error("missing enumeration name");
    ModulePtr cont = unit->currentModule();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3749 "src/Slice/Grammar.cpp"
    break;

  case 141: /* enum_def: enum_start @24 '{' enumerator_list_or_empty '}'  */
#line 1875 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3758 "src/Slice/Grammar.cpp"
    break;

  case 142: /* enum_underlying: ':' type  */
#line 1885 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3766 "src/Slice/Grammar.cpp"
    break;

  case 143: /* enum_underlying: %empty  */
#line 1889 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 3774 "src/Slice/Grammar.cpp"
    break;

  case 146: /* enumerator_list_or_empty: %empty  */
#line 1900 "src/Slice/Grammar.y"
{
    yyval = new EnumeratorListTok;
}
#line 3782 "src/Slice/Grammar.cpp"
    break;

  case 147: /* enumerator_list: enumerator_list ',' enumerator  */
#line 1909 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    if (EnumeratorPtr en = EnumeratorPtr::dynamicCast(yyvsp[0]))
    {
        enumerators->v.push_back(en);
    }
    yyval = enumerators;
}
#line 3795 "src/Slice/Grammar.cpp"
    break;

  case 148: /* enumerator_list: enumerator  */
#line 1918 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr enumerators = new EnumeratorListTok;
    if (EnumeratorPtr en = EnumeratorPtr::dynamicCast(yyvsp[0]))
    {
        enumerators->v.push_back(en);
    }
    yyval = enumerators;
}
#line 3808 "src/Slice/Grammar.cpp"
    break;

  case 149: /* enumerator: local_metadata ICE_IDENTIFIER  */
#line 1932 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);

    EnumPtr cont = EnumPtr::dynamicCast(unit->currentContainer());
    EnumeratorPtr en = cont->createEnumerator(ident->v);

    if (en && !metadata->v.empty())
    {
        en->setMetadata(metadata->v);
    }
    yyval = en;
}
#line 3826 "src/Slice/Grammar.cpp"
    break;

  case 150: /* enumerator: local_metadata ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1946 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);

    EnumPtr cont = EnumPtr::dynamicCast(unit->currentContainer());
    EnumeratorPtr en = cont->createEnumerator(ident->v, intVal->v);

    if (en && !metadata->v.empty())
    {
        en->setMetadata(metadata->v);
    }
    yyval = en;
}
#line 3845 "src/Slice/Grammar.cpp"
    break;

  case 151: /* enumerator: local_metadata keyword  */
#line 1961 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");

    EnumPtr cont = EnumPtr::dynamicCast(unit->currentContainer());
    EnumeratorPtr en = cont->createEnumerator(ident->v);

    if (en && !metadata->v.empty())
    {
        en->setMetadata(metadata->v);
    }
    yyval = en;
}
#line 3864 "src/Slice/Grammar.cpp"
    break;

  case 152: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1981 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3872 "src/Slice/Grammar.cpp"
    break;

  case 153: /* enumerator_initializer: scoped_name  */
#line 1985 "src/Slice/Grammar.y"
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
#line 3909 "src/Slice/Grammar.cpp"
    break;

  case 154: /* qualifier: ICE_OUT  */
#line 2023 "src/Slice/Grammar.y"
{
    yyval = new IntegerTok(QUALIFIER_OUT);
}
#line 3917 "src/Slice/Grammar.cpp"
    break;

  case 155: /* qualifier: ICE_STREAM  */
#line 2027 "src/Slice/Grammar.y"
{
    yyval = new IntegerTok(QUALIFIER_STREAM);
}
#line 3925 "src/Slice/Grammar.cpp"
    break;

  case 156: /* qualifier: ICE_STREAM ICE_OUT  */
#line 2031 "src/Slice/Grammar.y"
{
    // Not allowed but we still allow the parsing to print an appropriate error message
    yyval = new IntegerTok(QUALIFIER_OUT | QUALIFIER_STREAM);
}
#line 3934 "src/Slice/Grammar.cpp"
    break;

  case 157: /* qualifier: ICE_OUT ICE_STREAM  */
#line 2036 "src/Slice/Grammar.y"
{
    // Not allowed but we still allow the parsing to print an appropriate error message
    yyval = new IntegerTok(QUALIFIER_OUT | QUALIFIER_STREAM);
}
#line 3943 "src/Slice/Grammar.cpp"
    break;

  case 158: /* qualifier: %empty  */
#line 2041 "src/Slice/Grammar.y"
{
    yyval = new IntegerTok(QUALIFIER_NONE);
}
#line 3951 "src/Slice/Grammar.cpp"
    break;

  case 159: /* parameter: qualifier member  */
#line 2050 "src/Slice/Grammar.y"
{
    IntegerTokPtr qualifier = IntegerTokPtr::dynamicCast(yyvsp[-1]);
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);

    if (OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer()))
    {
        bool isOutParam = qualifier->v & QUALIFIER_OUT;
        bool isStreamParam = qualifier->v & QUALIFIER_STREAM;
        if (isOutParam && isStreamParam)
        {
            unit->error("`" + def->name + "': stream parameter cannot be marked as out");
        }

        MemberPtr param = op->createParameter(def->name, def->type, isOutParam, def->isTagged, def->tag, isStreamParam);
        unit->currentContainer()->checkIntroduced(def->name, param);
        if(param && !def->metadata.empty())
        {
            param->setMetadata(def->metadata);
        }
    }
}
#line 3977 "src/Slice/Grammar.cpp"
    break;

  case 164: /* throws: ICE_THROWS exception_list  */
#line 2090 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3985 "src/Slice/Grammar.cpp"
    break;

  case 165: /* throws: %empty  */
#line 2094 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 3993 "src/Slice/Grammar.cpp"
    break;

  case 169: /* unscoped_name: ICE_SCOPED_IDENTIFIER  */
#line 2111 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("Identifier cannot be scoped: `" + (ident->v) + "'");
    yyval = yyvsp[0];
}
#line 4003 "src/Slice/Grammar.cpp"
    break;

  case 170: /* builtin: ICE_BOOL  */
#line 2121 "src/Slice/Grammar.y"
           {}
#line 4009 "src/Slice/Grammar.cpp"
    break;

  case 171: /* builtin: ICE_BYTE  */
#line 2122 "src/Slice/Grammar.y"
           {}
#line 4015 "src/Slice/Grammar.cpp"
    break;

  case 172: /* builtin: ICE_SHORT  */
#line 2123 "src/Slice/Grammar.y"
            {}
#line 4021 "src/Slice/Grammar.cpp"
    break;

  case 173: /* builtin: ICE_USHORT  */
#line 2124 "src/Slice/Grammar.y"
             {}
#line 4027 "src/Slice/Grammar.cpp"
    break;

  case 174: /* builtin: ICE_INT  */
#line 2125 "src/Slice/Grammar.y"
          {}
#line 4033 "src/Slice/Grammar.cpp"
    break;

  case 175: /* builtin: ICE_UINT  */
#line 2126 "src/Slice/Grammar.y"
           {}
#line 4039 "src/Slice/Grammar.cpp"
    break;

  case 176: /* builtin: ICE_VARINT  */
#line 2127 "src/Slice/Grammar.y"
             {}
#line 4045 "src/Slice/Grammar.cpp"
    break;

  case 177: /* builtin: ICE_VARUINT  */
#line 2128 "src/Slice/Grammar.y"
              {}
#line 4051 "src/Slice/Grammar.cpp"
    break;

  case 178: /* builtin: ICE_LONG  */
#line 2129 "src/Slice/Grammar.y"
           {}
#line 4057 "src/Slice/Grammar.cpp"
    break;

  case 179: /* builtin: ICE_ULONG  */
#line 2130 "src/Slice/Grammar.y"
            {}
#line 4063 "src/Slice/Grammar.cpp"
    break;

  case 180: /* builtin: ICE_VARLONG  */
#line 2131 "src/Slice/Grammar.y"
              {}
#line 4069 "src/Slice/Grammar.cpp"
    break;

  case 181: /* builtin: ICE_VARULONG  */
#line 2132 "src/Slice/Grammar.y"
               {}
#line 4075 "src/Slice/Grammar.cpp"
    break;

  case 182: /* builtin: ICE_FLOAT  */
#line 2133 "src/Slice/Grammar.y"
            {}
#line 4081 "src/Slice/Grammar.cpp"
    break;

  case 183: /* builtin: ICE_DOUBLE  */
#line 2134 "src/Slice/Grammar.y"
             {}
#line 4087 "src/Slice/Grammar.cpp"
    break;

  case 184: /* builtin: ICE_STRING  */
#line 2135 "src/Slice/Grammar.y"
             {}
#line 4093 "src/Slice/Grammar.cpp"
    break;

  case 185: /* type: ICE_OBJECT '*'  */
#line 2141 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 4101 "src/Slice/Grammar.cpp"
    break;

  case 186: /* type: ICE_OBJECT '?'  */
#line 2145 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 4109 "src/Slice/Grammar.cpp"
    break;

  case 187: /* type: ICE_ANYCLASS '?'  */
#line 2149 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 4117 "src/Slice/Grammar.cpp"
    break;

  case 188: /* type: ICE_VALUE '?'  */
#line 2153 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 4129 "src/Slice/Grammar.cpp"
    break;

  case 189: /* type: builtin '?'  */
#line 2161 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[-1]);
    yyval = unit->optionalBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 4138 "src/Slice/Grammar.cpp"
    break;

  case 190: /* type: ICE_OBJECT  */
#line 2166 "src/Slice/Grammar.y"
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
#line 4153 "src/Slice/Grammar.cpp"
    break;

  case 191: /* type: ICE_ANYCLASS  */
#line 2177 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindAnyClass);
}
#line 4161 "src/Slice/Grammar.cpp"
    break;

  case 192: /* type: ICE_VALUE  */
#line 2181 "src/Slice/Grammar.y"
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
#line 4177 "src/Slice/Grammar.cpp"
    break;

  case 193: /* type: builtin  */
#line 2193 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 4186 "src/Slice/Grammar.cpp"
    break;

  case 194: /* type: scoped_name  */
#line 2198 "src/Slice/Grammar.y"
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
#line 4216 "src/Slice/Grammar.cpp"
    break;

  case 195: /* type: scoped_name '*'  */
#line 2224 "src/Slice/Grammar.y"
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
#line 4252 "src/Slice/Grammar.cpp"
    break;

  case 196: /* type: scoped_name '?'  */
#line 2256 "src/Slice/Grammar.y"
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
        cont->checkIntroduced(scoped->v);
        yyval = new Optional(types.front());
    }
    else
    {
        yyval = 0;
    }
}
#line 4275 "src/Slice/Grammar.cpp"
    break;

  case 197: /* tagged_type: tag type  */
#line 2280 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr taggedDef = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    OptionalPtr type = OptionalPtr::dynamicCast(yyvsp[0]);

    if (!type)
    {
        // Infer the type to be optional so parsing can continue without nullptrs.
        type = new Optional(TypePtr::dynamicCast(yyvsp[0]));
        unit->error("only optional types can be tagged");
    }

    if (auto alias = TypeAliasPtr::dynamicCast(type->underlying()))
    {
        taggedDef->metadata = alias->typeMetadata();
        type = new Optional(alias->underlying());
    }

    taggedDef->type = type;
    yyval = taggedDef;
}
#line 4300 "src/Slice/Grammar.cpp"
    break;

  case 198: /* tagged_type: optional type  */
#line 2301 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr taggedDef = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    OptionalPtr type = OptionalPtr::dynamicCast(yyvsp[0]);

    if (!type)
    {
        // Infer the type to be optional for backwards compatibility.
        type = new Optional(TypePtr::dynamicCast(yyvsp[0]));
    }

    if (auto alias = TypeAliasPtr::dynamicCast(type->underlying()))
    {
        taggedDef->metadata = alias->typeMetadata();
        type = new Optional(alias->underlying());
    }

    taggedDef->type = type;
    yyval = taggedDef;
}
#line 4324 "src/Slice/Grammar.cpp"
    break;

  case 199: /* tagged_type: type  */
#line 2321 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr taggedDef = new TaggedDefTok;
    taggedDef->type = TypePtr::dynamicCast(yyvsp[0]);

    unalias(taggedDef->type, taggedDef->metadata);

    yyval = taggedDef;
}
#line 4337 "src/Slice/Grammar.cpp"
    break;

  case 200: /* member: tagged_type ICE_IDENTIFIER  */
#line 2335 "src/Slice/Grammar.y"
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
#line 4353 "src/Slice/Grammar.cpp"
    break;

  case 201: /* member: tagged_type keyword  */
#line 2347 "src/Slice/Grammar.y"
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
#line 4368 "src/Slice/Grammar.cpp"
    break;

  case 202: /* member: tagged_type  */
#line 2358 "src/Slice/Grammar.y"
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
#line 4383 "src/Slice/Grammar.cpp"
    break;

  case 203: /* member: local_metadata member  */
#line 2369 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    def->metadata = appendMetadata(metadata->v, def->metadata);
    yyval = def;
}
#line 4394 "src/Slice/Grammar.cpp"
    break;

  case 204: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 2381 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4404 "src/Slice/Grammar.cpp"
    break;

  case 206: /* string_list: string_list ',' string_literal  */
#line 2393 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4415 "src/Slice/Grammar.cpp"
    break;

  case 207: /* string_list: string_literal  */
#line 2400 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4426 "src/Slice/Grammar.cpp"
    break;

  case 208: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2412 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4439 "src/Slice/Grammar.cpp"
    break;

  case 209: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2421 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4452 "src/Slice/Grammar.cpp"
    break;

  case 210: /* const_initializer: scoped_name  */
#line 2430 "src/Slice/Grammar.y"
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
#line 4496 "src/Slice/Grammar.cpp"
    break;

  case 211: /* const_initializer: ICE_STRING_LITERAL  */
#line 2470 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4507 "src/Slice/Grammar.cpp"
    break;

  case 212: /* const_initializer: ICE_FALSE  */
#line 2477 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4518 "src/Slice/Grammar.cpp"
    break;

  case 213: /* const_initializer: ICE_TRUE  */
#line 2484 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4529 "src/Slice/Grammar.cpp"
    break;

  case 214: /* const_def: ICE_CONST local_metadata type ICE_IDENTIFIER '=' const_initializer  */
#line 2496 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);

    unalias(const_type, metadata->v);

    yyval = unit->currentModule()->createConst(ident->v, const_type, metadata->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4545 "src/Slice/Grammar.cpp"
    break;

  case 215: /* const_def: ICE_CONST local_metadata type '=' const_initializer  */
#line 2508 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");

    unalias(const_type, metadata->v);

    yyval = unit->currentModule()->createConst(IceUtil::generateUUID(), const_type, metadata->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4561 "src/Slice/Grammar.cpp"
    break;

  case 216: /* keyword: ICE_MODULE  */
#line 2524 "src/Slice/Grammar.y"
             {}
#line 4567 "src/Slice/Grammar.cpp"
    break;

  case 217: /* keyword: ICE_CLASS  */
#line 2525 "src/Slice/Grammar.y"
            {}
#line 4573 "src/Slice/Grammar.cpp"
    break;

  case 218: /* keyword: ICE_INTERFACE  */
#line 2526 "src/Slice/Grammar.y"
                {}
#line 4579 "src/Slice/Grammar.cpp"
    break;

  case 219: /* keyword: ICE_EXCEPTION  */
#line 2527 "src/Slice/Grammar.y"
                {}
#line 4585 "src/Slice/Grammar.cpp"
    break;

  case 220: /* keyword: ICE_STRUCT  */
#line 2528 "src/Slice/Grammar.y"
             {}
#line 4591 "src/Slice/Grammar.cpp"
    break;

  case 221: /* keyword: ICE_SEQUENCE  */
#line 2529 "src/Slice/Grammar.y"
               {}
#line 4597 "src/Slice/Grammar.cpp"
    break;

  case 222: /* keyword: ICE_DICTIONARY  */
#line 2530 "src/Slice/Grammar.y"
                 {}
#line 4603 "src/Slice/Grammar.cpp"
    break;

  case 223: /* keyword: ICE_ENUM  */
#line 2531 "src/Slice/Grammar.y"
           {}
#line 4609 "src/Slice/Grammar.cpp"
    break;

  case 224: /* keyword: ICE_OUT  */
#line 2532 "src/Slice/Grammar.y"
          {}
#line 4615 "src/Slice/Grammar.cpp"
    break;

  case 225: /* keyword: ICE_STREAM  */
#line 2533 "src/Slice/Grammar.y"
             {}
#line 4621 "src/Slice/Grammar.cpp"
    break;

  case 226: /* keyword: ICE_EXTENDS  */
#line 2534 "src/Slice/Grammar.y"
              {}
#line 4627 "src/Slice/Grammar.cpp"
    break;

  case 227: /* keyword: ICE_IMPLEMENTS  */
#line 2535 "src/Slice/Grammar.y"
                 {}
#line 4633 "src/Slice/Grammar.cpp"
    break;

  case 228: /* keyword: ICE_THROWS  */
#line 2536 "src/Slice/Grammar.y"
             {}
#line 4639 "src/Slice/Grammar.cpp"
    break;

  case 229: /* keyword: ICE_VOID  */
#line 2537 "src/Slice/Grammar.y"
           {}
#line 4645 "src/Slice/Grammar.cpp"
    break;

  case 230: /* keyword: ICE_BOOL  */
#line 2538 "src/Slice/Grammar.y"
           {}
#line 4651 "src/Slice/Grammar.cpp"
    break;

  case 231: /* keyword: ICE_BYTE  */
#line 2539 "src/Slice/Grammar.y"
           {}
#line 4657 "src/Slice/Grammar.cpp"
    break;

  case 232: /* keyword: ICE_SHORT  */
#line 2540 "src/Slice/Grammar.y"
            {}
#line 4663 "src/Slice/Grammar.cpp"
    break;

  case 233: /* keyword: ICE_USHORT  */
#line 2541 "src/Slice/Grammar.y"
             {}
#line 4669 "src/Slice/Grammar.cpp"
    break;

  case 234: /* keyword: ICE_INT  */
#line 2542 "src/Slice/Grammar.y"
          {}
#line 4675 "src/Slice/Grammar.cpp"
    break;

  case 235: /* keyword: ICE_UINT  */
#line 2543 "src/Slice/Grammar.y"
           {}
#line 4681 "src/Slice/Grammar.cpp"
    break;

  case 236: /* keyword: ICE_VARINT  */
#line 2544 "src/Slice/Grammar.y"
             {}
#line 4687 "src/Slice/Grammar.cpp"
    break;

  case 237: /* keyword: ICE_VARUINT  */
#line 2545 "src/Slice/Grammar.y"
              {}
#line 4693 "src/Slice/Grammar.cpp"
    break;

  case 238: /* keyword: ICE_LONG  */
#line 2546 "src/Slice/Grammar.y"
           {}
#line 4699 "src/Slice/Grammar.cpp"
    break;

  case 239: /* keyword: ICE_ULONG  */
#line 2547 "src/Slice/Grammar.y"
            {}
#line 4705 "src/Slice/Grammar.cpp"
    break;

  case 240: /* keyword: ICE_VARLONG  */
#line 2548 "src/Slice/Grammar.y"
              {}
#line 4711 "src/Slice/Grammar.cpp"
    break;

  case 241: /* keyword: ICE_VARULONG  */
#line 2549 "src/Slice/Grammar.y"
               {}
#line 4717 "src/Slice/Grammar.cpp"
    break;

  case 242: /* keyword: ICE_FLOAT  */
#line 2550 "src/Slice/Grammar.y"
            {}
#line 4723 "src/Slice/Grammar.cpp"
    break;

  case 243: /* keyword: ICE_DOUBLE  */
#line 2551 "src/Slice/Grammar.y"
             {}
#line 4729 "src/Slice/Grammar.cpp"
    break;

  case 244: /* keyword: ICE_STRING  */
#line 2552 "src/Slice/Grammar.y"
             {}
#line 4735 "src/Slice/Grammar.cpp"
    break;

  case 245: /* keyword: ICE_OBJECT  */
#line 2553 "src/Slice/Grammar.y"
             {}
#line 4741 "src/Slice/Grammar.cpp"
    break;

  case 246: /* keyword: ICE_CONST  */
#line 2554 "src/Slice/Grammar.y"
            {}
#line 4747 "src/Slice/Grammar.cpp"
    break;

  case 247: /* keyword: ICE_FALSE  */
#line 2555 "src/Slice/Grammar.y"
            {}
#line 4753 "src/Slice/Grammar.cpp"
    break;

  case 248: /* keyword: ICE_TRUE  */
#line 2556 "src/Slice/Grammar.y"
           {}
#line 4759 "src/Slice/Grammar.cpp"
    break;

  case 249: /* keyword: ICE_IDEMPOTENT  */
#line 2557 "src/Slice/Grammar.y"
                 {}
#line 4765 "src/Slice/Grammar.cpp"
    break;

  case 250: /* keyword: ICE_TAG  */
#line 2558 "src/Slice/Grammar.y"
          {}
#line 4771 "src/Slice/Grammar.cpp"
    break;

  case 251: /* keyword: ICE_OPTIONAL  */
#line 2559 "src/Slice/Grammar.y"
               {}
#line 4777 "src/Slice/Grammar.cpp"
    break;

  case 252: /* keyword: ICE_ANYCLASS  */
#line 2560 "src/Slice/Grammar.y"
               {}
#line 4783 "src/Slice/Grammar.cpp"
    break;

  case 253: /* keyword: ICE_VALUE  */
#line 2561 "src/Slice/Grammar.y"
            {}
#line 4789 "src/Slice/Grammar.cpp"
    break;


#line 4793 "src/Slice/Grammar.cpp"

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

#line 2564 "src/Slice/Grammar.y"

