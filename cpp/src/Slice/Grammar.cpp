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


#line 183 "src/Slice/Grammar.cpp"

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
  YYSYMBOL_BAD_CHAR = 50,                  /* BAD_CHAR  */
  YYSYMBOL_51_ = 51,                       /* ';'  */
  YYSYMBOL_52_ = 52,                       /* '{'  */
  YYSYMBOL_53_ = 53,                       /* '}'  */
  YYSYMBOL_54_ = 54,                       /* '('  */
  YYSYMBOL_55_ = 55,                       /* ')'  */
  YYSYMBOL_56_ = 56,                       /* ':'  */
  YYSYMBOL_57_ = 57,                       /* '='  */
  YYSYMBOL_58_ = 58,                       /* ','  */
  YYSYMBOL_59_ = 59,                       /* '<'  */
  YYSYMBOL_60_ = 60,                       /* '>'  */
  YYSYMBOL_61_ = 61,                       /* '*'  */
  YYSYMBOL_62_ = 62,                       /* '?'  */
  YYSYMBOL_YYACCEPT = 63,                  /* $accept  */
  YYSYMBOL_start = 64,                     /* start  */
  YYSYMBOL_opt_semicolon = 65,             /* opt_semicolon  */
  YYSYMBOL_file_metadata = 66,             /* file_metadata  */
  YYSYMBOL_local_metadata = 67,            /* local_metadata  */
  YYSYMBOL_definitions = 68,               /* definitions  */
  YYSYMBOL_definition = 69,                /* definition  */
  YYSYMBOL_70_1 = 70,                      /* $@1  */
  YYSYMBOL_71_2 = 71,                      /* $@2  */
  YYSYMBOL_72_3 = 72,                      /* $@3  */
  YYSYMBOL_73_4 = 73,                      /* $@4  */
  YYSYMBOL_74_5 = 74,                      /* $@5  */
  YYSYMBOL_75_6 = 75,                      /* $@6  */
  YYSYMBOL_76_7 = 76,                      /* $@7  */
  YYSYMBOL_77_8 = 77,                      /* $@8  */
  YYSYMBOL_78_9 = 78,                      /* $@9  */
  YYSYMBOL_79_10 = 79,                     /* $@10  */
  YYSYMBOL_80_11 = 80,                     /* $@11  */
  YYSYMBOL_81_12 = 81,                     /* $@12  */
  YYSYMBOL_82_13 = 82,                     /* $@13  */
  YYSYMBOL_module_def = 83,                /* module_def  */
  YYSYMBOL_84_14 = 84,                     /* @14  */
  YYSYMBOL_85_15 = 85,                     /* @15  */
  YYSYMBOL_exception_id = 86,              /* exception_id  */
  YYSYMBOL_exception_decl = 87,            /* exception_decl  */
  YYSYMBOL_exception_def = 88,             /* exception_def  */
  YYSYMBOL_89_16 = 89,                     /* @16  */
  YYSYMBOL_exception_extends = 90,         /* exception_extends  */
  YYSYMBOL_tag = 91,                       /* tag  */
  YYSYMBOL_optional = 92,                  /* optional  */
  YYSYMBOL_struct_id = 93,                 /* struct_id  */
  YYSYMBOL_struct_decl = 94,               /* struct_decl  */
  YYSYMBOL_struct_def = 95,                /* struct_def  */
  YYSYMBOL_96_17 = 96,                     /* @17  */
  YYSYMBOL_class_name = 97,                /* class_name  */
  YYSYMBOL_class_id = 98,                  /* class_id  */
  YYSYMBOL_class_decl = 99,                /* class_decl  */
  YYSYMBOL_class_def = 100,                /* class_def  */
  YYSYMBOL_101_18 = 101,                   /* @18  */
  YYSYMBOL_class_extends = 102,            /* class_extends  */
  YYSYMBOL_extends = 103,                  /* extends  */
  YYSYMBOL_data_member = 104,              /* data_member  */
  YYSYMBOL_data_member_list = 105,         /* data_member_list  */
  YYSYMBOL_data_members = 106,             /* data_members  */
  YYSYMBOL_return_tuple = 107,             /* return_tuple  */
  YYSYMBOL_return_type = 108,              /* return_type  */
  YYSYMBOL_operation_preamble = 109,       /* operation_preamble  */
  YYSYMBOL_operation = 110,                /* operation  */
  YYSYMBOL_111_19 = 111,                   /* @19  */
  YYSYMBOL_112_20 = 112,                   /* @20  */
  YYSYMBOL_operation_list = 113,           /* operation_list  */
  YYSYMBOL_interface_id = 114,             /* interface_id  */
  YYSYMBOL_interface_decl = 115,           /* interface_decl  */
  YYSYMBOL_interface_def = 116,            /* interface_def  */
  YYSYMBOL_117_21 = 117,                   /* @21  */
  YYSYMBOL_interface_list = 118,           /* interface_list  */
  YYSYMBOL_interface_extends = 119,        /* interface_extends  */
  YYSYMBOL_exception_list = 120,           /* exception_list  */
  YYSYMBOL_exception = 121,                /* exception  */
  YYSYMBOL_sequence_def = 122,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 123,           /* dictionary_def  */
  YYSYMBOL_enum_start = 124,               /* enum_start  */
  YYSYMBOL_enum_id = 125,                  /* enum_id  */
  YYSYMBOL_enum_def = 126,                 /* enum_def  */
  YYSYMBOL_127_22 = 127,                   /* @22  */
  YYSYMBOL_128_23 = 128,                   /* @23  */
  YYSYMBOL_enum_underlying = 129,          /* enum_underlying  */
  YYSYMBOL_enumerator_list_or_empty = 130, /* enumerator_list_or_empty  */
  YYSYMBOL_enumerator_list = 131,          /* enumerator_list  */
  YYSYMBOL_enumerator = 132,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 133,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 134,            /* out_qualifier  */
  YYSYMBOL_parameter = 135,                /* parameter  */
  YYSYMBOL_parameter_list = 136,           /* parameter_list  */
  YYSYMBOL_parameters = 137,               /* parameters  */
  YYSYMBOL_throws = 138,                   /* throws  */
  YYSYMBOL_scoped_name = 139,              /* scoped_name  */
  YYSYMBOL_unscoped_name = 140,            /* unscoped_name  */
  YYSYMBOL_builtin = 141,                  /* builtin  */
  YYSYMBOL_type = 142,                     /* type  */
  YYSYMBOL_tagged_type = 143,              /* tagged_type  */
  YYSYMBOL_member = 144,                   /* member  */
  YYSYMBOL_string_literal = 145,           /* string_literal  */
  YYSYMBOL_string_list = 146,              /* string_list  */
  YYSYMBOL_const_initializer = 147,        /* const_initializer  */
  YYSYMBOL_const_def = 148,                /* const_def  */
  YYSYMBOL_keyword = 149                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 61 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 375 "src/Slice/Grammar.cpp"

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
#define YYLAST   1096

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  63
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  87
/* YYNRULES -- Number of rules.  */
#define YYNRULES  243
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  346

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   305


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
      54,    55,    61,     2,    58,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    56,    51,
      59,    57,    60,    62,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    52,     2,    53,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   203,   203,   209,   210,   216,   227,   233,   242,   251,
     259,   268,   275,   274,   280,   279,   284,   289,   288,   294,
     293,   298,   303,   302,   308,   307,   312,   317,   316,   322,
     321,   326,   331,   330,   336,   335,   340,   345,   344,   349,
     354,   353,   359,   358,   363,   367,   377,   376,   415,   414,
     492,   496,   507,   518,   517,   543,   551,   560,   578,   652,
     658,   669,   691,   769,   779,   794,   798,   809,   820,   819,
     860,   864,   875,   900,   990,  1002,  1015,  1014,  1048,  1082,
    1091,  1092,  1098,  1113,  1135,  1136,  1137,  1141,  1147,  1148,
    1154,  1166,  1182,  1197,  1206,  1211,  1220,  1255,  1290,  1324,
    1364,  1363,  1386,  1385,  1408,  1426,  1430,  1431,  1437,  1441,
    1452,  1466,  1465,  1499,  1534,  1569,  1574,  1579,  1593,  1597,
    1606,  1613,  1625,  1637,  1648,  1656,  1670,  1680,  1696,  1700,
    1709,  1725,  1739,  1738,  1759,  1758,  1777,  1781,  1790,  1791,
    1792,  1801,  1810,  1824,  1838,  1853,  1873,  1877,  1915,  1919,
    1928,  1947,  1948,  1954,  1955,  1961,  1965,  1974,  1975,  1981,
    1982,  1993,  1994,  1995,  1996,  1997,  1998,  1999,  2000,  2001,
    2002,  2003,  2004,  2005,  2006,  2007,  2012,  2016,  2020,  2024,
    2032,  2037,  2048,  2052,  2064,  2069,  2095,  2127,  2155,  2170,
    2184,  2195,  2207,  2218,  2229,  2240,  2246,  2252,  2259,  2271,
    2280,  2289,  2329,  2336,  2343,  2355,  2364,  2378,  2379,  2380,
    2381,  2382,  2383,  2384,  2385,  2386,  2387,  2388,  2389,  2390,
    2391,  2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,  2400,
    2401,  2402,  2403,  2404,  2405,  2406,  2407,  2408,  2409,  2410,
    2411,  2412,  2413,  2414
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
  "ICE_FILE_METADATA_CLOSE", "BAD_CHAR", "';'", "'{'", "'}'", "'('", "')'",
  "':'", "'='", "','", "'<'", "'>'", "'*'", "'?'", "$accept", "start",
  "opt_semicolon", "file_metadata", "local_metadata", "definitions",
  "definition", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8",
  "$@9", "$@10", "$@11", "$@12", "$@13", "module_def", "@14", "@15",
  "exception_id", "exception_decl", "exception_def", "@16",
  "exception_extends", "tag", "optional", "struct_id", "struct_decl",
  "struct_def", "@17", "class_name", "class_id", "class_decl", "class_def",
  "@18", "class_extends", "extends", "data_member", "data_member_list",
  "data_members", "return_tuple", "return_type", "operation_preamble",
  "operation", "@19", "@20", "operation_list", "interface_id",
  "interface_decl", "interface_def", "@21", "interface_list",
  "interface_extends", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_start", "enum_id", "enum_def", "@22", "@23",
  "enum_underlying", "enumerator_list_or_empty", "enumerator_list",
  "enumerator", "enumerator_initializer", "out_qualifier", "parameter",
  "parameter_list", "parameters", "throws", "scoped_name", "unscoped_name",
  "builtin", "type", "tagged_type", "member", "string_literal",
  "string_list", "const_initializer", "const_def", "keyword", YY_NULLPTR
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
     305,    59,   123,   125,    40,    41,    58,    61,    44,    60,
      62,    42,    63
};
#endif

#define YYPACT_NINF (-274)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-160)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -274,    41,    15,  -274,   -15,   -15,  -274,    82,   -15,  -274,
      47,   -37,    28,    81,   468,   597,   639,   681,    40,    49,
    -274,   103,   158,   -15,  -274,  -274,    24,   126,  -274,   139,
     150,  -274,    25,     6,   154,  -274,    26,   155,  -274,   157,
     159,   723,   153,  -274,   160,  -274,  -274,   -15,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,   161,
    -274,   164,  -274,  -274,  -274,  -274,  -274,  -274,  -274,   103,
     103,  1020,  -274,    48,   162,  -274,  -274,  -274,   102,   163,
     162,   167,   169,   162,  -274,   102,   170,   162,    65,  -274,
     174,   162,   175,   176,  -274,   177,  -274,  1051,  -274,   162,
     179,  -274,   180,   181,   125,  1020,  1020,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,
    -274,  -274,   123,   172,   178,  -274,  -274,   136,   182,   -33,
    -274,  -274,  -274,   184,  -274,  -274,  -274,   135,  -274,  -274,
     185,  -274,  -274,  -274,  -274,  -274,  -274,  -274,   173,   186,
    -274,  -274,  -274,  -274,   -24,  -274,   187,  -274,  -274,  -274,
    -274,   190,   191,   152,   183,  -274,  -274,  -274,  -274,  -274,
    -274,  -274,   192,   100,   135,   196,   194,   197,   958,  1051,
    1051,   199,   334,   200,  -274,   765,   195,   135,    65,   295,
     424,   201,   198,  -274,   -24,   130,   146,  -274,  -274,   807,
     103,   100,  -274,  -274,  -274,  -274,  -274,  -274,  -274,   202,
    -274,    93,   145,   -15,  -274,  -274,  -274,  -274,   373,  -274,
    -274,  -274,   100,   204,  -274,   207,   879,   208,   203,  -274,
    -274,    67,   209,  -274,  -274,  -274,  -274,  1020,  -274,  -274,
     210,  -274,   211,   212,  -274,   213,    53,  -274,  -274,   295,
    -274,   919,     9,   511,    29,   218,  -274,  -274,   151,  -274,
    -274,   214,  -274,  -274,  -274,  -274,  -274,  -274,   511,  -274,
    -274,    66,   989,  -274,   217,   219,   220,   989,  -274,   205,
     221,   295,  -274,  -274,  -274,   849,   224,   225,  -274,   253,
    -274,  -274,  -274,  -274,  -274,   253,  -274,  -274,  -274,  -274,
    -274,  -274,   989,   258,  -274,   258,  -274,   554,  -274,  -274,
    -274,   222,  -274,  -274,   554,  -274
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      11,     0,     8,     1,     0,     0,     9,     0,   196,   198,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     129,     8,     0,     0,    10,    12,    52,    26,    27,    67,
      31,    32,    75,    79,    16,    17,   110,    21,    22,    36,
      39,   134,   137,    40,    44,   195,     6,     0,     5,    45,
      46,    48,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,    70,
     160,     0,    71,   108,   109,    50,    51,    65,    66,     8,
       8,     0,   128,     0,     4,    80,    81,    53,     0,     0,
       4,     0,     0,     4,    76,     0,     0,     4,     0,   111,
       0,     4,     0,     0,   130,     0,   131,     0,   132,     4,
       0,   197,     0,     0,     0,     0,     0,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   181,   182,   183,   157,   158,   185,   184,     0,
       7,     3,    13,     0,    55,    25,    28,     0,    30,    33,
       0,    78,    15,    18,   115,   116,   117,   118,   114,     0,
      20,    23,    35,    38,     8,   136,     0,    41,    43,    11,
      11,     0,     0,     0,     0,   176,   177,   178,   179,   186,
     187,   180,     0,     0,     0,     0,    60,    64,     0,     0,
       0,    86,     0,     0,   190,   193,    82,     0,     0,     0,
       0,     0,   138,   142,     8,     8,     8,    72,    73,     0,
       8,     0,   203,   204,   202,   199,   200,   201,   206,     0,
      85,     0,     0,     0,   194,   188,   189,    84,     0,    69,
     191,   192,     0,     0,   113,     0,     0,     0,   143,   145,
     135,     8,     0,    47,    49,   124,   125,     0,   205,    54,
       0,    59,     0,     0,    63,     0,     0,    83,    77,     0,
      95,     0,   149,     0,     0,   105,    92,   112,     0,   141,
     133,     0,    57,    58,    61,    62,     6,   106,     0,   148,
      94,     0,     0,   159,     0,     0,     0,     0,   151,   153,
       0,     0,   146,   144,   147,     0,     0,     0,    93,   149,
      90,    96,    98,   102,   150,   149,   100,   104,   126,   127,
      97,    99,     0,   156,   152,   156,    91,     0,   103,   101,
     155,   121,   122,   123,     0,   120
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -274,  -274,     2,  -274,    -2,    14,  -274,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,  -274,
      94,  -274,  -179,  -190,  -274,     1,  -274,  -274,  -274,  -274,
    -272,  -274,  -274,  -274,  -274,    63,  -274,   -61,  -274,  -274,
    -274,  -274,  -274,  -274,  -274,  -274,  -274,    60,  -274,    27,
    -274,  -248,   -39,  -274,  -274,   -48,  -102,  -273,  -274,   -92,
    -239,  -200,    23,     0,  -159,  -274,   -13
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   162,     6,   208,     2,    24,   104,   116,   117,
     120,   121,   109,   110,   112,   113,   122,   123,   129,   130,
      25,   132,   133,    26,    27,    28,   163,   107,   209,   210,
      29,    30,    31,   111,    32,    33,    34,    35,   170,   114,
     108,   211,   212,   213,   301,   283,   284,   285,   335,   333,
     257,    36,    37,    38,   179,   177,   119,   340,   341,    39,
      40,    41,    42,    43,   186,   125,   128,   221,   222,   223,
     313,   307,   308,   309,   310,   338,   157,    91,   158,   214,
     215,   216,     9,    10,   238,    44,   343
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
       7,    92,    94,    96,    98,    11,   164,   297,   244,   159,
     304,   202,    48,   171,   239,    -2,   178,   286,   105,   101,
     299,    47,     4,   103,   203,   316,     8,   253,   126,  -140,
     306,    45,   192,   248,   302,   185,   105,   -74,   105,   327,
     299,     3,   286,   193,   194,  -149,  -149,  -149,  -149,  -149,
    -149,  -149,  -149,  -149,  -149,  -149,  -149,  -149,  -149,  -149,
    -149,     4,   106,     5,   300,  -149,  -149,  -149,  -149,   248,
     131,   332,   268,  -149,  -149,  -149,   -56,   -74,  -119,    49,
     106,   -74,   106,    12,  -154,    13,    14,    15,    16,    17,
      18,    19,    20,   277,    46,   160,   174,   135,   136,    99,
     296,   237,   320,   175,   176,    47,    47,   324,   100,   155,
     156,    47,   166,     4,    21,   169,   178,   245,   246,   173,
    -139,   318,    22,   181,   319,    50,    51,   115,    23,   237,
     118,   187,   336,   232,   233,   270,   205,   155,   156,   272,
     275,   234,   235,   236,   155,   156,   155,   156,   271,     4,
     237,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   191,   102,   155,
     156,   206,   207,   153,   154,   291,     4,   -24,     5,   155,
     156,     4,   220,   263,   195,   196,   314,   273,   -89,   155,
     156,   -68,     4,   312,     5,   155,   156,   199,   200,   264,
     274,   -29,   251,   225,   226,   -14,   -19,   259,   -34,   127,
     -37,   -42,   229,   161,   165,  -159,   266,   256,   134,   167,
     168,   172,   220,     7,     7,   180,   182,   183,   267,   184,
     188,   218,   189,   190,   197,   342,   204,   217,   219,   224,
     198,   230,   342,   276,   201,   227,   228,   240,   241,   231,
     247,   242,   252,   249,   260,   269,   261,   278,   279,   220,
     288,   287,   290,   325,   299,   292,   293,   294,   295,   311,
     305,   321,   337,   322,   315,   323,   326,   256,   330,   331,
     344,   254,   298,   345,   262,   317,   334,   339,   289,     0,
       0,     0,     0,     0,     0,     0,   255,     0,     0,     0,
       0,     0,   329,     0,     0,     0,     0,     0,     0,   256,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,    -8,
      -8,    -8,    -8,    -8,    -8,    -8,    -8,     0,     0,     0,
      -8,    -8,    -8,    -8,    -8,   205,     0,     0,     0,    -8,
      -8,     4,     0,     0,     0,     0,     0,     0,  -107,    -8,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,     0,     0,     0,     0,
     206,   207,   153,   154,   205,     0,     0,     0,   155,   156,
       4,     0,     0,     0,     0,     0,     0,   -88,     0,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,     0,     0,     0,     0,   206,
     207,   153,   154,     0,     0,     0,     0,   155,   156,     4,
       0,     0,     0,     0,     0,     0,   -87,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,     0,     0,     0,     0,   258,     0,
      23,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,     0,     0,
       0,     0,    89,    90,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,     0,     0,     0,     0,   303,    90,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,     0,     0,     0,     0,   155,   156,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
       0,    93,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,     0,
       0,     0,     0,    95,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,     0,     0,     0,     0,    97,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,     0,     0,     0,     0,   124,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,     0,     0,     0,     0,   250,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
       0,   265,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,     0,
       0,     0,     0,   328,   280,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,     0,     0,     0,   281,   206,   207,   153,   154,     0,
       0,     0,     0,   155,   156,    23,     0,     0,     0,     0,
       0,     0,     0,   282,   280,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,     0,     0,     0,     0,   206,   207,   153,   154,     0,
       0,     0,     0,   155,   156,     0,     0,     0,     0,     0,
       0,     0,     0,   282,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
       0,     0,     0,     0,   206,   207,   153,   154,     0,     0,
       0,     0,   155,   156,   243,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,     0,     0,     0,     0,   206,   207,   153,   154,     0,
       0,     0,     0,   155,   156,     4,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,     0,     0,     0,     0,     0,     0,   153,   154,
       0,     0,     0,     0,   155,   156,    23,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   152,     0,     0,     0,     0,     0,     0,   153,
     154,     0,     0,     0,     0,   155,   156
};

static const yytype_int16 yycheck[] =
{
       2,    14,    15,    16,    17,     5,   108,   279,   208,   101,
     283,    44,    49,   115,   204,     0,   118,   256,    12,    21,
      11,    58,    46,    23,    57,   298,    41,   217,    41,    53,
       1,     8,   134,   212,   282,   127,    12,    12,    12,   311,
      11,     0,   281,   135,   136,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    46,    56,    48,    55,    36,    37,    38,    39,   248,
      47,   319,   231,    44,    45,    46,    52,    52,    52,    51,
      56,    56,    56,     1,    55,     3,     4,     5,     6,     7,
       8,     9,    10,   252,    47,    47,    31,    99,   100,    59,
      47,   203,   302,    38,    39,    58,    58,   307,    59,    44,
      45,    58,   110,    46,    32,   113,   218,   209,   210,   117,
      53,    55,    40,   121,    58,    44,    45,    33,    46,   231,
      36,   129,   332,    33,    34,    42,     1,    44,    45,   241,
     242,    41,    42,    43,    44,    45,    44,    45,    55,    46,
     252,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    42,    10,    44,
      45,    36,    37,    38,    39,   267,    46,    51,    48,    44,
      45,    46,   184,    53,    61,    62,   288,    42,    53,    44,
      45,    52,    46,    42,    48,    44,    45,    61,    62,    53,
      55,    51,   215,   189,   190,    51,    51,   220,    51,    56,
      51,    51,    60,    51,    51,    54,   229,   219,    54,    52,
      51,    51,   224,   225,   226,    51,    51,    51,   230,    52,
      51,    58,    52,    52,    62,   337,    52,    52,    52,    52,
      62,    58,   344,   243,    62,    55,    55,    51,    54,    57,
      51,    54,    57,    53,    53,    53,    58,    53,    51,   261,
      57,    53,    53,    58,    11,    55,    55,    55,    55,    51,
     283,    54,    14,    54,    60,    55,    55,   279,    54,    54,
      58,   218,   281,   344,   224,   298,   325,   335,   261,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,    -1,
      -1,    -1,   315,    -1,    -1,    -1,    -1,    -1,    -1,   311,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    -1,    -1,    -1,
      35,    36,    37,    38,    39,     1,    -1,    -1,    -1,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      36,    37,    38,    39,     1,    -1,    -1,    -1,    44,    45,
      46,    -1,    -1,    -1,    -1,    -1,    -1,    53,    -1,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    44,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    53,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,    -1,
      46,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,    45,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,    -1,    -1,    44,    45,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,    45,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    44,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    44,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    -1,    -1,    -1,    -1,    44,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    44,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    44,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    44,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    35,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    54,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,    45,    46,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    44,    45,    46,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,
      -1,    -1,    -1,    -1,    44,    45,    46,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    -1,    -1,    -1,    -1,    44,    45
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    64,    68,     0,    46,    48,    66,    67,    41,   145,
     146,   146,     1,     3,     4,     5,     6,     7,     8,     9,
      10,    32,    40,    46,    69,    83,    86,    87,    88,    93,
      94,    95,    97,    98,    99,   100,   114,   115,   116,   122,
     123,   124,   125,   126,   148,   145,    47,    58,    49,    51,
      44,    45,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    44,
      45,   140,   149,    44,   149,    44,   149,    44,   149,    59,
      59,    67,    10,   146,    70,    12,    56,    90,   103,    75,
      76,    96,    77,    78,   102,   103,    71,    72,   103,   119,
      73,    74,    79,    80,    44,   128,   149,    56,   129,    81,
      82,   145,    84,    85,    54,    67,    67,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    38,    39,    44,    45,   139,   141,   142,
      47,    51,    65,    89,   139,    51,    65,    52,    51,    65,
     101,   139,    51,    65,    31,    38,    39,   118,   139,   117,
      51,    65,    51,    51,    52,   142,   127,    65,    51,    52,
      52,    42,   139,   142,   142,    61,    62,    62,    62,    61,
      62,    62,    44,    57,    52,     1,    36,    37,    67,    91,
      92,   104,   105,   106,   142,   143,   144,    52,    58,    52,
      67,   130,   131,   132,    52,    68,    68,    55,    55,    60,
      58,    57,    33,    34,    41,    42,    43,   139,   147,   106,
      51,    54,    54,    46,   144,   142,   142,    51,   105,    53,
      44,   149,    57,   106,   118,     1,    67,   113,    44,   149,
      53,    58,   130,    53,    53,    44,   149,    67,   147,    53,
      42,    55,   139,    42,    55,   139,   146,   147,    53,    51,
      15,    35,    54,   108,   109,   110,   143,    53,    57,   132,
      53,   142,    55,    55,    55,    55,    47,   113,   108,    11,
      55,   107,   134,    44,   140,   149,     1,   134,   135,   136,
     137,    51,    42,   133,   139,    60,   140,   149,    55,    58,
     144,    54,    54,    55,   144,    58,    55,   113,    44,   149,
      54,    54,   134,   112,   135,   111,   144,    14,   138,   138,
     120,   121,   139,   149,    58,   120
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    63,    64,    65,    65,    66,    67,    67,    67,    68,
      68,    68,    70,    69,    71,    69,    69,    72,    69,    73,
      69,    69,    74,    69,    75,    69,    69,    76,    69,    77,
      69,    69,    78,    69,    79,    69,    69,    80,    69,    69,
      81,    69,    82,    69,    69,    69,    84,    83,    85,    83,
      86,    86,    87,    89,    88,    90,    90,    91,    91,    91,
      91,    92,    92,    92,    92,    93,    93,    94,    96,    95,
      97,    97,    98,    98,    98,    99,   101,   100,   102,   102,
     103,   103,   104,   104,   105,   105,   105,   105,   106,   106,
     107,   107,   108,   108,   108,   108,   109,   109,   109,   109,
     111,   110,   112,   110,   113,   113,   113,   113,   114,   114,
     115,   117,   116,   118,   118,   118,   118,   118,   119,   119,
     120,   120,   121,   121,   122,   122,   123,   123,   124,   124,
     125,   125,   127,   126,   128,   126,   129,   129,   130,   130,
     130,   131,   131,   132,   132,   132,   133,   133,   134,   134,
     135,   136,   136,   137,   137,   138,   138,   139,   139,   140,
     140,   141,   141,   141,   141,   141,   141,   141,   141,   141,
     141,   141,   141,   141,   141,   141,   142,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   143,   143,
     143,   144,   144,   144,   144,   145,   145,   146,   146,   147,
     147,   147,   147,   147,   147,   148,   148,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     4,     0,     2,
       3,     0,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     3,     1,
       0,     3,     0,     3,     1,     2,     0,     6,     0,     6,
       2,     2,     1,     0,     6,     2,     0,     4,     4,     3,
       1,     4,     4,     3,     1,     2,     2,     1,     0,     5,
       2,     2,     5,     5,     1,     1,     0,     6,     2,     0,
       1,     1,     1,     3,     2,     2,     1,     2,     1,     0,
       2,     4,     1,     3,     2,     1,     3,     4,     3,     4,
       0,     5,     0,     5,     4,     2,     3,     0,     2,     2,
       1,     0,     6,     3,     1,     1,     1,     1,     2,     0,
       3,     1,     1,     1,     6,     6,     9,     9,     2,     1,
       2,     2,     0,     6,     0,     5,     2,     0,     1,     2,
       0,     3,     1,     2,     4,     2,     1,     1,     1,     0,
       2,     1,     3,     1,     0,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       1,     2,     2,     1,     2,     2,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     6,     5,     1,     1,     1,
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
#line 217 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    convertMetadata(metadata);
    yyval = metadata;
}
#line 1868 "src/Slice/Grammar.cpp"
    break;

  case 6: /* local_metadata: ICE_LOCAL_METADATA_OPEN string_list ICE_LOCAL_METADATA_CLOSE  */
#line 228 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    convertMetadata(metadata);
    yyval = metadata;
}
#line 1878 "src/Slice/Grammar.cpp"
    break;

  case 7: /* local_metadata: local_metadata ICE_LOCAL_METADATA_OPEN string_list ICE_LOCAL_METADATA_CLOSE  */
#line 234 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata1 = StringListTokPtr::dynamicCast(yyvsp[-3]);
    StringListTokPtr metadata2 = StringListTokPtr::dynamicCast(yyvsp[-1]);
    metadata1->v.splice(metadata1->v.end(), metadata2->v);

    convertMetadata(metadata1);
    yyval = metadata1;
}
#line 1891 "src/Slice/Grammar.cpp"
    break;

  case 8: /* local_metadata: %empty  */
#line 243 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 1899 "src/Slice/Grammar.cpp"
    break;

  case 9: /* definitions: definitions file_metadata  */
#line 252 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metadata->v.empty())
    {
        unit->addFileMetadata(metadata->v);
    }
}
#line 1911 "src/Slice/Grammar.cpp"
    break;

  case 10: /* definitions: definitions local_metadata definition  */
#line 260 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metadata->v.empty())
    {
        contained->setMetadata(metadata->v);
    }
}
#line 1924 "src/Slice/Grammar.cpp"
    break;

  case 12: /* $@1: %empty  */
#line 275 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 1932 "src/Slice/Grammar.cpp"
    break;

  case 14: /* $@2: %empty  */
#line 280 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1940 "src/Slice/Grammar.cpp"
    break;

  case 16: /* definition: class_decl  */
#line 285 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 1948 "src/Slice/Grammar.cpp"
    break;

  case 17: /* $@3: %empty  */
#line 289 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 1956 "src/Slice/Grammar.cpp"
    break;

  case 19: /* $@4: %empty  */
#line 294 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDeclPtr::dynamicCast(yyvsp[0]));
}
#line 1964 "src/Slice/Grammar.cpp"
    break;

  case 21: /* definition: interface_decl  */
#line 299 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 1972 "src/Slice/Grammar.cpp"
    break;

  case 22: /* $@5: %empty  */
#line 303 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDefPtr::dynamicCast(yyvsp[0]));
}
#line 1980 "src/Slice/Grammar.cpp"
    break;

  case 24: /* $@6: %empty  */
#line 308 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 1988 "src/Slice/Grammar.cpp"
    break;

  case 26: /* definition: exception_decl  */
#line 313 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 1996 "src/Slice/Grammar.cpp"
    break;

  case 27: /* $@7: %empty  */
#line 317 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 2004 "src/Slice/Grammar.cpp"
    break;

  case 29: /* $@8: %empty  */
#line 322 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2012 "src/Slice/Grammar.cpp"
    break;

  case 31: /* definition: struct_decl  */
#line 327 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 2020 "src/Slice/Grammar.cpp"
    break;

  case 32: /* $@9: %empty  */
#line 331 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 2028 "src/Slice/Grammar.cpp"
    break;

  case 34: /* $@10: %empty  */
#line 336 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2036 "src/Slice/Grammar.cpp"
    break;

  case 36: /* definition: sequence_def  */
#line 341 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2044 "src/Slice/Grammar.cpp"
    break;

  case 37: /* $@11: %empty  */
#line 345 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2052 "src/Slice/Grammar.cpp"
    break;

  case 39: /* definition: dictionary_def  */
#line 350 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2060 "src/Slice/Grammar.cpp"
    break;

  case 40: /* $@12: %empty  */
#line 354 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2068 "src/Slice/Grammar.cpp"
    break;

  case 42: /* $@13: %empty  */
#line 359 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2076 "src/Slice/Grammar.cpp"
    break;

  case 44: /* definition: const_def  */
#line 364 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2084 "src/Slice/Grammar.cpp"
    break;

  case 45: /* definition: error ';'  */
#line 368 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2092 "src/Slice/Grammar.cpp"
    break;

  case 46: /* @14: %empty  */
#line 377 "src/Slice/Grammar.y"
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
#line 2122 "src/Slice/Grammar.cpp"
    break;

  case 47: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
#line 403 "src/Slice/Grammar.y"
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
#line 2138 "src/Slice/Grammar.cpp"
    break;

  case 48: /* @15: %empty  */
#line 415 "src/Slice/Grammar.y"
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
#line 2197 "src/Slice/Grammar.cpp"
    break;

  case 49: /* module_def: ICE_MODULE ICE_SCOPED_IDENTIFIER @15 '{' definitions '}'  */
#line 470 "src/Slice/Grammar.y"
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
#line 2219 "src/Slice/Grammar.cpp"
    break;

  case 50: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 493 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2227 "src/Slice/Grammar.cpp"
    break;

  case 51: /* exception_id: ICE_EXCEPTION keyword  */
#line 497 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2237 "src/Slice/Grammar.cpp"
    break;

  case 52: /* exception_decl: exception_id  */
#line 508 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2246 "src/Slice/Grammar.cpp"
    break;

  case 53: /* @16: %empty  */
#line 518 "src/Slice/Grammar.y"
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
#line 2263 "src/Slice/Grammar.cpp"
    break;

  case 54: /* exception_def: exception_id exception_extends @16 '{' data_members '}'  */
#line 531 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2275 "src/Slice/Grammar.cpp"
    break;

  case 55: /* exception_extends: extends scoped_name  */
#line 544 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2287 "src/Slice/Grammar.cpp"
    break;

  case 56: /* exception_extends: %empty  */
#line 552 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2295 "src/Slice/Grammar.cpp"
    break;

  case 57: /* tag: ICE_TAG '(' ICE_INTEGER_LITERAL ')'  */
#line 561 "src/Slice/Grammar.y"
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
#line 2317 "src/Slice/Grammar.cpp"
    break;

  case 58: /* tag: ICE_TAG '(' scoped_name ')'  */
#line 579 "src/Slice/Grammar.y"
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
#line 2395 "src/Slice/Grammar.cpp"
    break;

  case 59: /* tag: ICE_TAG '(' ')'  */
#line 653 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2405 "src/Slice/Grammar.cpp"
    break;

  case 60: /* tag: ICE_TAG  */
#line 659 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2415 "src/Slice/Grammar.cpp"
    break;

  case 61: /* optional: ICE_OPTIONAL '(' ICE_INTEGER_LITERAL ')'  */
#line 670 "src/Slice/Grammar.y"
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
#line 2441 "src/Slice/Grammar.cpp"
    break;

  case 62: /* optional: ICE_OPTIONAL '(' scoped_name ')'  */
#line 692 "src/Slice/Grammar.y"
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
#line 2523 "src/Slice/Grammar.cpp"
    break;

  case 63: /* optional: ICE_OPTIONAL '(' ')'  */
#line 770 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2537 "src/Slice/Grammar.cpp"
    break;

  case 64: /* optional: ICE_OPTIONAL  */
#line 780 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok; // Dummy
    yyval = m;
}
#line 2551 "src/Slice/Grammar.cpp"
    break;

  case 65: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 795 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2559 "src/Slice/Grammar.cpp"
    break;

  case 66: /* struct_id: ICE_STRUCT keyword  */
#line 799 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2569 "src/Slice/Grammar.cpp"
    break;

  case 67: /* struct_decl: struct_id  */
#line 810 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2578 "src/Slice/Grammar.cpp"
    break;

  case 68: /* @17: %empty  */
#line 820 "src/Slice/Grammar.y"
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
#line 2600 "src/Slice/Grammar.cpp"
    break;

  case 69: /* struct_def: struct_id @17 '{' data_members '}'  */
#line 838 "src/Slice/Grammar.y"
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
#line 2622 "src/Slice/Grammar.cpp"
    break;

  case 70: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 861 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2630 "src/Slice/Grammar.cpp"
    break;

  case 71: /* class_name: ICE_CLASS keyword  */
#line 865 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2640 "src/Slice/Grammar.cpp"
    break;

  case 72: /* class_id: ICE_CLASS unscoped_name '(' ICE_INTEGER_LITERAL ')'  */
#line 876 "src/Slice/Grammar.y"
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
#line 2669 "src/Slice/Grammar.cpp"
    break;

  case 73: /* class_id: ICE_CLASS unscoped_name '(' scoped_name ')'  */
#line 901 "src/Slice/Grammar.y"
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
#line 2763 "src/Slice/Grammar.cpp"
    break;

  case 74: /* class_id: class_name  */
#line 991 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2774 "src/Slice/Grammar.cpp"
    break;

  case 75: /* class_decl: class_name  */
#line 1003 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2785 "src/Slice/Grammar.cpp"
    break;

  case 76: /* @18: %empty  */
#line 1015 "src/Slice/Grammar.y"
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
#line 2806 "src/Slice/Grammar.cpp"
    break;

  case 77: /* class_def: class_id class_extends @18 '{' data_members '}'  */
#line 1032 "src/Slice/Grammar.y"
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
#line 2822 "src/Slice/Grammar.cpp"
    break;

  case 78: /* class_extends: extends scoped_name  */
#line 1049 "src/Slice/Grammar.y"
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
#line 2860 "src/Slice/Grammar.cpp"
    break;

  case 79: /* class_extends: %empty  */
#line 1083 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2868 "src/Slice/Grammar.cpp"
    break;

  case 82: /* data_member: member  */
#line 1099 "src/Slice/Grammar.y"
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
#line 2887 "src/Slice/Grammar.cpp"
    break;

  case 83: /* data_member: member '=' const_initializer  */
#line 1114 "src/Slice/Grammar.y"
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
#line 2908 "src/Slice/Grammar.cpp"
    break;

  case 86: /* data_member_list: data_member  */
#line 1138 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2916 "src/Slice/Grammar.cpp"
    break;

  case 90: /* return_tuple: out_qualifier member  */
#line 1155 "src/Slice/Grammar.y"
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
#line 2932 "src/Slice/Grammar.cpp"
    break;

  case 91: /* return_tuple: return_tuple ',' out_qualifier member  */
#line 1167 "src/Slice/Grammar.y"
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
#line 2948 "src/Slice/Grammar.cpp"
    break;

  case 92: /* return_type: tagged_type  */
#line 1183 "src/Slice/Grammar.y"
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
#line 2967 "src/Slice/Grammar.cpp"
    break;

  case 93: /* return_type: '(' return_tuple ')'  */
#line 1198 "src/Slice/Grammar.y"
{
    TaggedDefListTokPtr returnMembers = TaggedDefListTokPtr::dynamicCast(yyvsp[-1]);
    if (returnMembers->v.size() == 1)
    {
        unit->error("return tuples must contain at least 2 elements");
    }
    yyval = yyvsp[-1];
}
#line 2980 "src/Slice/Grammar.cpp"
    break;

  case 94: /* return_type: '(' ')'  */
#line 1207 "src/Slice/Grammar.y"
{
    unit->error("return tuples must contain at least 2 elements");
    yyval = new TaggedDefListTok();
}
#line 2989 "src/Slice/Grammar.cpp"
    break;

  case 95: /* return_type: ICE_VOID  */
#line 1212 "src/Slice/Grammar.y"
{
    yyval = new TaggedDefListTok();
}
#line 2997 "src/Slice/Grammar.cpp"
    break;

  case 96: /* operation_preamble: return_type unscoped_name '('  */
#line 1221 "src/Slice/Grammar.y"
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
                                                     returnMember->tag);
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
#line 3036 "src/Slice/Grammar.cpp"
    break;

  case 97: /* operation_preamble: ICE_IDEMPOTENT return_type unscoped_name '('  */
#line 1256 "src/Slice/Grammar.y"
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
                                                     returnMember->tag);
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
#line 3075 "src/Slice/Grammar.cpp"
    break;

  case 98: /* operation_preamble: return_type keyword '('  */
#line 1291 "src/Slice/Grammar.y"
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
                                                     returnMember->tag);
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
#line 3113 "src/Slice/Grammar.cpp"
    break;

  case 99: /* operation_preamble: ICE_IDEMPOTENT return_type keyword '('  */
#line 1325 "src/Slice/Grammar.y"
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
                                                     returnMember->tag);
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
#line 3151 "src/Slice/Grammar.cpp"
    break;

  case 100: /* @19: %empty  */
#line 1364 "src/Slice/Grammar.y"
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
#line 3167 "src/Slice/Grammar.cpp"
    break;

  case 101: /* operation: operation_preamble parameters ')' @19 throws  */
#line 1376 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3181 "src/Slice/Grammar.cpp"
    break;

  case 102: /* @20: %empty  */
#line 1386 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3193 "src/Slice/Grammar.cpp"
    break;

  case 103: /* operation: operation_preamble error ')' @20 throws  */
#line 1394 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3207 "src/Slice/Grammar.cpp"
    break;

  case 104: /* operation_list: local_metadata operation ';' operation_list  */
#line 1409 "src/Slice/Grammar.y"
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
            operation->returnType().front()->setMetadata(metadata->v);
        }
    }
}
#line 3229 "src/Slice/Grammar.cpp"
    break;

  case 105: /* operation_list: local_metadata operation  */
#line 1427 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3237 "src/Slice/Grammar.cpp"
    break;

  case 108: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1438 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3245 "src/Slice/Grammar.cpp"
    break;

  case 109: /* interface_id: ICE_INTERFACE keyword  */
#line 1442 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3255 "src/Slice/Grammar.cpp"
    break;

  case 110: /* interface_decl: interface_id  */
#line 1453 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3267 "src/Slice/Grammar.cpp"
    break;

  case 111: /* @21: %empty  */
#line 1466 "src/Slice/Grammar.y"
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
#line 3288 "src/Slice/Grammar.cpp"
    break;

  case 112: /* interface_def: interface_id interface_extends @21 '{' operation_list '}'  */
#line 1483 "src/Slice/Grammar.y"
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
#line 3304 "src/Slice/Grammar.cpp"
    break;

  case 113: /* interface_list: scoped_name ',' interface_list  */
#line 1500 "src/Slice/Grammar.y"
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
#line 3343 "src/Slice/Grammar.cpp"
    break;

  case 114: /* interface_list: scoped_name  */
#line 1535 "src/Slice/Grammar.y"
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
#line 3382 "src/Slice/Grammar.cpp"
    break;

  case 115: /* interface_list: ICE_OBJECT  */
#line 1570 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new InterfaceListTok; // Dummy
}
#line 3391 "src/Slice/Grammar.cpp"
    break;

  case 116: /* interface_list: ICE_ANYCLASS  */
#line 1575 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type AnyClass");
    yyval = new ClassListTok; // Dummy
}
#line 3400 "src/Slice/Grammar.cpp"
    break;

  case 117: /* interface_list: ICE_VALUE  */
#line 1580 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3413 "src/Slice/Grammar.cpp"
    break;

  case 118: /* interface_extends: extends interface_list  */
#line 1594 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3421 "src/Slice/Grammar.cpp"
    break;

  case 119: /* interface_extends: %empty  */
#line 1598 "src/Slice/Grammar.y"
{
    yyval = new InterfaceListTok;
}
#line 3429 "src/Slice/Grammar.cpp"
    break;

  case 120: /* exception_list: exception ',' exception_list  */
#line 1607 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3440 "src/Slice/Grammar.cpp"
    break;

  case 121: /* exception_list: exception  */
#line 1614 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3451 "src/Slice/Grammar.cpp"
    break;

  case 122: /* exception: scoped_name  */
#line 1626 "src/Slice/Grammar.y"
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
#line 3467 "src/Slice/Grammar.cpp"
    break;

  case 123: /* exception: keyword  */
#line 1638 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentModule()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3477 "src/Slice/Grammar.cpp"
    break;

  case 124: /* sequence_def: ICE_SEQUENCE '<' local_metadata type '>' ICE_IDENTIFIER  */
#line 1649 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metadata->v);
}
#line 3489 "src/Slice/Grammar.cpp"
    break;

  case 125: /* sequence_def: ICE_SEQUENCE '<' local_metadata type '>' keyword  */
#line 1657 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metadata->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3502 "src/Slice/Grammar.cpp"
    break;

  case 126: /* dictionary_def: ICE_DICTIONARY '<' local_metadata type ',' local_metadata type '>' ICE_IDENTIFIER  */
#line 1671 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetadata = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createDictionary(ident->v, keyType, keyMetadata->v, valueType, valueMetadata->v);
}
#line 3516 "src/Slice/Grammar.cpp"
    break;

  case 127: /* dictionary_def: ICE_DICTIONARY '<' local_metadata type ',' local_metadata type '>' keyword  */
#line 1681 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetadata = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createDictionary(ident->v, keyType, keyMetadata->v, valueType, valueMetadata->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as dictionary name");
}
#line 3531 "src/Slice/Grammar.cpp"
    break;

  case 128: /* enum_start: ICE_UNCHECKED ICE_ENUM  */
#line 1697 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(true);
}
#line 3539 "src/Slice/Grammar.cpp"
    break;

  case 129: /* enum_start: ICE_ENUM  */
#line 1701 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(false);
}
#line 3547 "src/Slice/Grammar.cpp"
    break;

  case 130: /* enum_id: enum_start ICE_IDENTIFIER  */
#line 1710 "src/Slice/Grammar.y"
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
#line 3567 "src/Slice/Grammar.cpp"
    break;

  case 131: /* enum_id: enum_start keyword  */
#line 1726 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[-1])->v;
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
}
#line 3579 "src/Slice/Grammar.cpp"
    break;

  case 132: /* @22: %empty  */
#line 1739 "src/Slice/Grammar.y"
{
    EnumPtr en = EnumPtr::dynamicCast(yyvsp[-1]);
    en->initUnderlying(TypePtr::dynamicCast(yyvsp[0]));
    unit->pushContainer(en);
    yyval = en;
}
#line 3590 "src/Slice/Grammar.cpp"
    break;

  case 133: /* enum_def: enum_id enum_underlying @22 '{' enumerator_list_or_empty '}'  */
#line 1746 "src/Slice/Grammar.y"
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
#line 3607 "src/Slice/Grammar.cpp"
    break;

  case 134: /* @23: %empty  */
#line 1759 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[0])->v;
    unit->error("missing enumeration name");
    ModulePtr cont = unit->currentModule();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3620 "src/Slice/Grammar.cpp"
    break;

  case 135: /* enum_def: enum_start @23 '{' enumerator_list_or_empty '}'  */
#line 1768 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3629 "src/Slice/Grammar.cpp"
    break;

  case 136: /* enum_underlying: ':' type  */
#line 1778 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3637 "src/Slice/Grammar.cpp"
    break;

  case 137: /* enum_underlying: %empty  */
#line 1782 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 3645 "src/Slice/Grammar.cpp"
    break;

  case 140: /* enumerator_list_or_empty: %empty  */
#line 1793 "src/Slice/Grammar.y"
{
    yyval = new EnumeratorListTok;
}
#line 3653 "src/Slice/Grammar.cpp"
    break;

  case 141: /* enumerator_list: enumerator_list ',' enumerator  */
#line 1802 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr enumerators = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    if (EnumeratorPtr en = EnumeratorPtr::dynamicCast(yyvsp[0]))
    {
        enumerators->v.push_back(en);
    }
    yyval = enumerators;
}
#line 3666 "src/Slice/Grammar.cpp"
    break;

  case 142: /* enumerator_list: enumerator  */
#line 1811 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr enumerators = new EnumeratorListTok;
    if (EnumeratorPtr en = EnumeratorPtr::dynamicCast(yyvsp[0]))
    {
        enumerators->v.push_back(en);
    }
    yyval = enumerators;
}
#line 3679 "src/Slice/Grammar.cpp"
    break;

  case 143: /* enumerator: local_metadata ICE_IDENTIFIER  */
#line 1825 "src/Slice/Grammar.y"
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
#line 3697 "src/Slice/Grammar.cpp"
    break;

  case 144: /* enumerator: local_metadata ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1839 "src/Slice/Grammar.y"
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
#line 3716 "src/Slice/Grammar.cpp"
    break;

  case 145: /* enumerator: local_metadata keyword  */
#line 1854 "src/Slice/Grammar.y"
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
#line 3735 "src/Slice/Grammar.cpp"
    break;

  case 146: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1874 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3743 "src/Slice/Grammar.cpp"
    break;

  case 147: /* enumerator_initializer: scoped_name  */
#line 1878 "src/Slice/Grammar.y"
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
#line 3780 "src/Slice/Grammar.cpp"
    break;

  case 148: /* out_qualifier: ICE_OUT  */
#line 1916 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(true);
}
#line 3788 "src/Slice/Grammar.cpp"
    break;

  case 149: /* out_qualifier: %empty  */
#line 1920 "src/Slice/Grammar.y"
{
    yyval = new BoolTok(false);
}
#line 3796 "src/Slice/Grammar.cpp"
    break;

  case 150: /* parameter: out_qualifier member  */
#line 1929 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-1]);
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);

    if (OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer()))
    {
        MemberPtr param = op->createParameter(def->name, def->type, isOutParam->v, def->isTagged, def->tag);
        unit->currentContainer()->checkIntroduced(def->name, param);
        if(param && !def->metadata.empty())
        {
            param->setMetadata(def->metadata);
        }
    }
}
#line 3815 "src/Slice/Grammar.cpp"
    break;

  case 155: /* throws: ICE_THROWS exception_list  */
#line 1962 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3823 "src/Slice/Grammar.cpp"
    break;

  case 156: /* throws: %empty  */
#line 1966 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 3831 "src/Slice/Grammar.cpp"
    break;

  case 160: /* unscoped_name: ICE_SCOPED_IDENTIFIER  */
#line 1983 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("Identifier cannot be scoped: `" + (ident->v) + "'");
    yyval = yyvsp[0];
}
#line 3841 "src/Slice/Grammar.cpp"
    break;

  case 161: /* builtin: ICE_BOOL  */
#line 1993 "src/Slice/Grammar.y"
           {}
#line 3847 "src/Slice/Grammar.cpp"
    break;

  case 162: /* builtin: ICE_BYTE  */
#line 1994 "src/Slice/Grammar.y"
           {}
#line 3853 "src/Slice/Grammar.cpp"
    break;

  case 163: /* builtin: ICE_SHORT  */
#line 1995 "src/Slice/Grammar.y"
            {}
#line 3859 "src/Slice/Grammar.cpp"
    break;

  case 164: /* builtin: ICE_USHORT  */
#line 1996 "src/Slice/Grammar.y"
             {}
#line 3865 "src/Slice/Grammar.cpp"
    break;

  case 165: /* builtin: ICE_INT  */
#line 1997 "src/Slice/Grammar.y"
          {}
#line 3871 "src/Slice/Grammar.cpp"
    break;

  case 166: /* builtin: ICE_UINT  */
#line 1998 "src/Slice/Grammar.y"
           {}
#line 3877 "src/Slice/Grammar.cpp"
    break;

  case 167: /* builtin: ICE_VARINT  */
#line 1999 "src/Slice/Grammar.y"
             {}
#line 3883 "src/Slice/Grammar.cpp"
    break;

  case 168: /* builtin: ICE_VARUINT  */
#line 2000 "src/Slice/Grammar.y"
              {}
#line 3889 "src/Slice/Grammar.cpp"
    break;

  case 169: /* builtin: ICE_LONG  */
#line 2001 "src/Slice/Grammar.y"
           {}
#line 3895 "src/Slice/Grammar.cpp"
    break;

  case 170: /* builtin: ICE_ULONG  */
#line 2002 "src/Slice/Grammar.y"
            {}
#line 3901 "src/Slice/Grammar.cpp"
    break;

  case 171: /* builtin: ICE_VARLONG  */
#line 2003 "src/Slice/Grammar.y"
              {}
#line 3907 "src/Slice/Grammar.cpp"
    break;

  case 172: /* builtin: ICE_VARULONG  */
#line 2004 "src/Slice/Grammar.y"
               {}
#line 3913 "src/Slice/Grammar.cpp"
    break;

  case 173: /* builtin: ICE_FLOAT  */
#line 2005 "src/Slice/Grammar.y"
            {}
#line 3919 "src/Slice/Grammar.cpp"
    break;

  case 174: /* builtin: ICE_DOUBLE  */
#line 2006 "src/Slice/Grammar.y"
             {}
#line 3925 "src/Slice/Grammar.cpp"
    break;

  case 175: /* builtin: ICE_STRING  */
#line 2007 "src/Slice/Grammar.y"
             {}
#line 3931 "src/Slice/Grammar.cpp"
    break;

  case 176: /* type: ICE_OBJECT '*'  */
#line 2013 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 3939 "src/Slice/Grammar.cpp"
    break;

  case 177: /* type: ICE_OBJECT '?'  */
#line 2017 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 3947 "src/Slice/Grammar.cpp"
    break;

  case 178: /* type: ICE_ANYCLASS '?'  */
#line 2021 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 3955 "src/Slice/Grammar.cpp"
    break;

  case 179: /* type: ICE_VALUE '?'  */
#line 2025 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 3967 "src/Slice/Grammar.cpp"
    break;

  case 180: /* type: builtin '?'  */
#line 2033 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[-1]);
    yyval = unit->optionalBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 3976 "src/Slice/Grammar.cpp"
    break;

  case 181: /* type: ICE_OBJECT  */
#line 2038 "src/Slice/Grammar.y"
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
#line 3991 "src/Slice/Grammar.cpp"
    break;

  case 182: /* type: ICE_ANYCLASS  */
#line 2049 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindAnyClass);
}
#line 3999 "src/Slice/Grammar.cpp"
    break;

  case 183: /* type: ICE_VALUE  */
#line 2053 "src/Slice/Grammar.y"
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
#line 4015 "src/Slice/Grammar.cpp"
    break;

  case 184: /* type: builtin  */
#line 2065 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 4024 "src/Slice/Grammar.cpp"
    break;

  case 185: /* type: scoped_name  */
#line 2070 "src/Slice/Grammar.y"
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
#line 4054 "src/Slice/Grammar.cpp"
    break;

  case 186: /* type: scoped_name '*'  */
#line 2096 "src/Slice/Grammar.y"
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
#line 4090 "src/Slice/Grammar.cpp"
    break;

  case 187: /* type: scoped_name '?'  */
#line 2128 "src/Slice/Grammar.y"
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
#line 4117 "src/Slice/Grammar.cpp"
    break;

  case 188: /* tagged_type: tag type  */
#line 2156 "src/Slice/Grammar.y"
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
#line 4136 "src/Slice/Grammar.cpp"
    break;

  case 189: /* tagged_type: optional type  */
#line 2171 "src/Slice/Grammar.y"
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
#line 4154 "src/Slice/Grammar.cpp"
    break;

  case 190: /* tagged_type: type  */
#line 2185 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr taggedDef = new TaggedDefTok;
    taggedDef->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = taggedDef;
}
#line 4164 "src/Slice/Grammar.cpp"
    break;

  case 191: /* member: tagged_type ICE_IDENTIFIER  */
#line 2196 "src/Slice/Grammar.y"
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
#line 4180 "src/Slice/Grammar.cpp"
    break;

  case 192: /* member: tagged_type keyword  */
#line 2208 "src/Slice/Grammar.y"
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
#line 4195 "src/Slice/Grammar.cpp"
    break;

  case 193: /* member: tagged_type  */
#line 2219 "src/Slice/Grammar.y"
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
#line 4210 "src/Slice/Grammar.cpp"
    break;

  case 194: /* member: local_metadata member  */
#line 2230 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    def->metadata = StringListTokPtr::dynamicCast(yyvsp[-1])->v;
    yyval = def;
}
#line 4220 "src/Slice/Grammar.cpp"
    break;

  case 195: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 2241 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4230 "src/Slice/Grammar.cpp"
    break;

  case 197: /* string_list: string_list ',' string_literal  */
#line 2253 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4241 "src/Slice/Grammar.cpp"
    break;

  case 198: /* string_list: string_literal  */
#line 2260 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4252 "src/Slice/Grammar.cpp"
    break;

  case 199: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2272 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4265 "src/Slice/Grammar.cpp"
    break;

  case 200: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2281 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4278 "src/Slice/Grammar.cpp"
    break;

  case 201: /* const_initializer: scoped_name  */
#line 2290 "src/Slice/Grammar.y"
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
#line 4322 "src/Slice/Grammar.cpp"
    break;

  case 202: /* const_initializer: ICE_STRING_LITERAL  */
#line 2330 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4333 "src/Slice/Grammar.cpp"
    break;

  case 203: /* const_initializer: ICE_FALSE  */
#line 2337 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4344 "src/Slice/Grammar.cpp"
    break;

  case 204: /* const_initializer: ICE_TRUE  */
#line 2344 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4355 "src/Slice/Grammar.cpp"
    break;

  case 205: /* const_def: ICE_CONST local_metadata type ICE_IDENTIFIER '=' const_initializer  */
#line 2356 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentModule()->createConst(ident->v, const_type, metadata->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4368 "src/Slice/Grammar.cpp"
    break;

  case 206: /* const_def: ICE_CONST local_metadata type '=' const_initializer  */
#line 2365 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentModule()->createConst(IceUtil::generateUUID(), const_type, metadata->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4381 "src/Slice/Grammar.cpp"
    break;

  case 207: /* keyword: ICE_MODULE  */
#line 2378 "src/Slice/Grammar.y"
             {}
#line 4387 "src/Slice/Grammar.cpp"
    break;

  case 208: /* keyword: ICE_CLASS  */
#line 2379 "src/Slice/Grammar.y"
            {}
#line 4393 "src/Slice/Grammar.cpp"
    break;

  case 209: /* keyword: ICE_INTERFACE  */
#line 2380 "src/Slice/Grammar.y"
                {}
#line 4399 "src/Slice/Grammar.cpp"
    break;

  case 210: /* keyword: ICE_EXCEPTION  */
#line 2381 "src/Slice/Grammar.y"
                {}
#line 4405 "src/Slice/Grammar.cpp"
    break;

  case 211: /* keyword: ICE_STRUCT  */
#line 2382 "src/Slice/Grammar.y"
             {}
#line 4411 "src/Slice/Grammar.cpp"
    break;

  case 212: /* keyword: ICE_SEQUENCE  */
#line 2383 "src/Slice/Grammar.y"
               {}
#line 4417 "src/Slice/Grammar.cpp"
    break;

  case 213: /* keyword: ICE_DICTIONARY  */
#line 2384 "src/Slice/Grammar.y"
                 {}
#line 4423 "src/Slice/Grammar.cpp"
    break;

  case 214: /* keyword: ICE_ENUM  */
#line 2385 "src/Slice/Grammar.y"
           {}
#line 4429 "src/Slice/Grammar.cpp"
    break;

  case 215: /* keyword: ICE_OUT  */
#line 2386 "src/Slice/Grammar.y"
          {}
#line 4435 "src/Slice/Grammar.cpp"
    break;

  case 216: /* keyword: ICE_EXTENDS  */
#line 2387 "src/Slice/Grammar.y"
              {}
#line 4441 "src/Slice/Grammar.cpp"
    break;

  case 217: /* keyword: ICE_IMPLEMENTS  */
#line 2388 "src/Slice/Grammar.y"
                 {}
#line 4447 "src/Slice/Grammar.cpp"
    break;

  case 218: /* keyword: ICE_THROWS  */
#line 2389 "src/Slice/Grammar.y"
             {}
#line 4453 "src/Slice/Grammar.cpp"
    break;

  case 219: /* keyword: ICE_VOID  */
#line 2390 "src/Slice/Grammar.y"
           {}
#line 4459 "src/Slice/Grammar.cpp"
    break;

  case 220: /* keyword: ICE_BOOL  */
#line 2391 "src/Slice/Grammar.y"
           {}
#line 4465 "src/Slice/Grammar.cpp"
    break;

  case 221: /* keyword: ICE_BYTE  */
#line 2392 "src/Slice/Grammar.y"
           {}
#line 4471 "src/Slice/Grammar.cpp"
    break;

  case 222: /* keyword: ICE_SHORT  */
#line 2393 "src/Slice/Grammar.y"
            {}
#line 4477 "src/Slice/Grammar.cpp"
    break;

  case 223: /* keyword: ICE_USHORT  */
#line 2394 "src/Slice/Grammar.y"
             {}
#line 4483 "src/Slice/Grammar.cpp"
    break;

  case 224: /* keyword: ICE_INT  */
#line 2395 "src/Slice/Grammar.y"
          {}
#line 4489 "src/Slice/Grammar.cpp"
    break;

  case 225: /* keyword: ICE_UINT  */
#line 2396 "src/Slice/Grammar.y"
           {}
#line 4495 "src/Slice/Grammar.cpp"
    break;

  case 226: /* keyword: ICE_VARINT  */
#line 2397 "src/Slice/Grammar.y"
             {}
#line 4501 "src/Slice/Grammar.cpp"
    break;

  case 227: /* keyword: ICE_VARUINT  */
#line 2398 "src/Slice/Grammar.y"
              {}
#line 4507 "src/Slice/Grammar.cpp"
    break;

  case 228: /* keyword: ICE_LONG  */
#line 2399 "src/Slice/Grammar.y"
           {}
#line 4513 "src/Slice/Grammar.cpp"
    break;

  case 229: /* keyword: ICE_ULONG  */
#line 2400 "src/Slice/Grammar.y"
            {}
#line 4519 "src/Slice/Grammar.cpp"
    break;

  case 230: /* keyword: ICE_VARLONG  */
#line 2401 "src/Slice/Grammar.y"
              {}
#line 4525 "src/Slice/Grammar.cpp"
    break;

  case 231: /* keyword: ICE_VARULONG  */
#line 2402 "src/Slice/Grammar.y"
               {}
#line 4531 "src/Slice/Grammar.cpp"
    break;

  case 232: /* keyword: ICE_FLOAT  */
#line 2403 "src/Slice/Grammar.y"
            {}
#line 4537 "src/Slice/Grammar.cpp"
    break;

  case 233: /* keyword: ICE_DOUBLE  */
#line 2404 "src/Slice/Grammar.y"
             {}
#line 4543 "src/Slice/Grammar.cpp"
    break;

  case 234: /* keyword: ICE_STRING  */
#line 2405 "src/Slice/Grammar.y"
             {}
#line 4549 "src/Slice/Grammar.cpp"
    break;

  case 235: /* keyword: ICE_OBJECT  */
#line 2406 "src/Slice/Grammar.y"
             {}
#line 4555 "src/Slice/Grammar.cpp"
    break;

  case 236: /* keyword: ICE_CONST  */
#line 2407 "src/Slice/Grammar.y"
            {}
#line 4561 "src/Slice/Grammar.cpp"
    break;

  case 237: /* keyword: ICE_FALSE  */
#line 2408 "src/Slice/Grammar.y"
            {}
#line 4567 "src/Slice/Grammar.cpp"
    break;

  case 238: /* keyword: ICE_TRUE  */
#line 2409 "src/Slice/Grammar.y"
           {}
#line 4573 "src/Slice/Grammar.cpp"
    break;

  case 239: /* keyword: ICE_IDEMPOTENT  */
#line 2410 "src/Slice/Grammar.y"
                 {}
#line 4579 "src/Slice/Grammar.cpp"
    break;

  case 240: /* keyword: ICE_TAG  */
#line 2411 "src/Slice/Grammar.y"
          {}
#line 4585 "src/Slice/Grammar.cpp"
    break;

  case 241: /* keyword: ICE_OPTIONAL  */
#line 2412 "src/Slice/Grammar.y"
               {}
#line 4591 "src/Slice/Grammar.cpp"
    break;

  case 242: /* keyword: ICE_ANYCLASS  */
#line 2413 "src/Slice/Grammar.y"
               {}
#line 4597 "src/Slice/Grammar.cpp"
    break;

  case 243: /* keyword: ICE_VALUE  */
#line 2414 "src/Slice/Grammar.y"
            {}
#line 4603 "src/Slice/Grammar.cpp"
    break;


#line 4607 "src/Slice/Grammar.cpp"

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

#line 2417 "src/Slice/Grammar.y"

