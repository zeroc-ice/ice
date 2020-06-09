/* A Bison parser, made by GNU Bison 3.6.2.  */

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
#define YYBISON_VERSION "3.6.2"

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

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED
# define YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int slice_debug;
#endif
/* "%code requires" blocks.  */
#line 12 "src/Slice/Grammar.y"


// Define a custom location type for storing the location (and filename) of tokens.
#define YYLTYPE Slice::TokenContext

// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
#define YYMAXDEPTH  10000
#define YYINITDEPTH YYMAXDEPTH

// Newer bison versions allow to disable stack resizing by defining yyoverflow.
#define yyoverflow(a, b, c, d, e, f, g, h) yyerror(a)


#line 221 "src/Slice/Grammar.cpp"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ICE_MODULE = 258,              /* ICE_MODULE  */
    ICE_CLASS = 259,               /* ICE_CLASS  */
    ICE_INTERFACE = 260,           /* ICE_INTERFACE  */
    ICE_EXCEPTION = 261,           /* ICE_EXCEPTION  */
    ICE_STRUCT = 262,              /* ICE_STRUCT  */
    ICE_SEQUENCE = 263,            /* ICE_SEQUENCE  */
    ICE_DICTIONARY = 264,          /* ICE_DICTIONARY  */
    ICE_ENUM = 265,                /* ICE_ENUM  */
    ICE_OUT = 266,                 /* ICE_OUT  */
    ICE_EXTENDS = 267,             /* ICE_EXTENDS  */
    ICE_IMPLEMENTS = 268,          /* ICE_IMPLEMENTS  */
    ICE_THROWS = 269,              /* ICE_THROWS  */
    ICE_VOID = 270,                /* ICE_VOID  */
    ICE_BOOL = 271,                /* ICE_BOOL  */
    ICE_BYTE = 272,                /* ICE_BYTE  */
    ICE_SHORT = 273,               /* ICE_SHORT  */
    ICE_USHORT = 274,              /* ICE_USHORT  */
    ICE_INT = 275,                 /* ICE_INT  */
    ICE_UINT = 276,                /* ICE_UINT  */
    ICE_VARINT = 277,              /* ICE_VARINT  */
    ICE_VARUINT = 278,             /* ICE_VARUINT  */
    ICE_LONG = 279,                /* ICE_LONG  */
    ICE_ULONG = 280,               /* ICE_ULONG  */
    ICE_VARLONG = 281,             /* ICE_VARLONG  */
    ICE_VARULONG = 282,            /* ICE_VARULONG  */
    ICE_FLOAT = 283,               /* ICE_FLOAT  */
    ICE_DOUBLE = 284,              /* ICE_DOUBLE  */
    ICE_STRING = 285,              /* ICE_STRING  */
    ICE_OBJECT = 286,              /* ICE_OBJECT  */
    ICE_CONST = 287,               /* ICE_CONST  */
    ICE_FALSE = 288,               /* ICE_FALSE  */
    ICE_TRUE = 289,                /* ICE_TRUE  */
    ICE_IDEMPOTENT = 290,          /* ICE_IDEMPOTENT  */
    ICE_TAG = 291,                 /* ICE_TAG  */
    ICE_OPTIONAL = 292,            /* ICE_OPTIONAL  */
    ICE_VALUE = 293,               /* ICE_VALUE  */
    ICE_STRING_LITERAL = 294,      /* ICE_STRING_LITERAL  */
    ICE_INTEGER_LITERAL = 295,     /* ICE_INTEGER_LITERAL  */
    ICE_FLOATING_POINT_LITERAL = 296, /* ICE_FLOATING_POINT_LITERAL  */
    ICE_IDENTIFIER = 297,          /* ICE_IDENTIFIER  */
    ICE_SCOPED_IDENTIFIER = 298,   /* ICE_SCOPED_IDENTIFIER  */
    ICE_LOCAL_METADATA_OPEN = 299, /* ICE_LOCAL_METADATA_OPEN  */
    ICE_LOCAL_METADATA_CLOSE = 300, /* ICE_LOCAL_METADATA_CLOSE  */
    ICE_FILE_METADATA_OPEN = 301,  /* ICE_FILE_METADATA_OPEN  */
    ICE_FILE_METADATA_IGNORE = 302, /* ICE_FILE_METADATA_IGNORE  */
    ICE_FILE_METADATA_CLOSE = 303, /* ICE_FILE_METADATA_CLOSE  */
    ICE_IDENT_OPEN = 304,          /* ICE_IDENT_OPEN  */
    ICE_KEYWORD_OPEN = 305,        /* ICE_KEYWORD_OPEN  */
    ICE_TAG_OPEN = 306,            /* ICE_TAG_OPEN  */
    ICE_OPTIONAL_OPEN = 307,       /* ICE_OPTIONAL_OPEN  */
    BAD_CHAR = 308                 /* BAD_CHAR  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef Slice::GrammarBasePtr YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int slice_parse (void);

#endif /* !YY_SLICE_SRC_SLICE_GRAMMAR_HPP_INCLUDED  */
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
  YYSYMBOL_ICE_VALUE = 38,                 /* ICE_VALUE  */
  YYSYMBOL_ICE_STRING_LITERAL = 39,        /* ICE_STRING_LITERAL  */
  YYSYMBOL_ICE_INTEGER_LITERAL = 40,       /* ICE_INTEGER_LITERAL  */
  YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 41, /* ICE_FLOATING_POINT_LITERAL  */
  YYSYMBOL_ICE_IDENTIFIER = 42,            /* ICE_IDENTIFIER  */
  YYSYMBOL_ICE_SCOPED_IDENTIFIER = 43,     /* ICE_SCOPED_IDENTIFIER  */
  YYSYMBOL_ICE_LOCAL_METADATA_OPEN = 44,   /* ICE_LOCAL_METADATA_OPEN  */
  YYSYMBOL_ICE_LOCAL_METADATA_CLOSE = 45,  /* ICE_LOCAL_METADATA_CLOSE  */
  YYSYMBOL_ICE_FILE_METADATA_OPEN = 46,    /* ICE_FILE_METADATA_OPEN  */
  YYSYMBOL_ICE_FILE_METADATA_IGNORE = 47,  /* ICE_FILE_METADATA_IGNORE  */
  YYSYMBOL_ICE_FILE_METADATA_CLOSE = 48,   /* ICE_FILE_METADATA_CLOSE  */
  YYSYMBOL_ICE_IDENT_OPEN = 49,            /* ICE_IDENT_OPEN  */
  YYSYMBOL_ICE_KEYWORD_OPEN = 50,          /* ICE_KEYWORD_OPEN  */
  YYSYMBOL_ICE_TAG_OPEN = 51,              /* ICE_TAG_OPEN  */
  YYSYMBOL_ICE_OPTIONAL_OPEN = 52,         /* ICE_OPTIONAL_OPEN  */
  YYSYMBOL_BAD_CHAR = 53,                  /* BAD_CHAR  */
  YYSYMBOL_54_ = 54,                       /* ';'  */
  YYSYMBOL_55_ = 55,                       /* '{'  */
  YYSYMBOL_56_ = 56,                       /* '}'  */
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
  YYSYMBOL_module_def = 85,                /* module_def  */
  YYSYMBOL_86_14 = 86,                     /* @14  */
  YYSYMBOL_87_15 = 87,                     /* @15  */
  YYSYMBOL_exception_id = 88,              /* exception_id  */
  YYSYMBOL_exception_decl = 89,            /* exception_decl  */
  YYSYMBOL_exception_def = 90,             /* exception_def  */
  YYSYMBOL_91_16 = 91,                     /* @16  */
  YYSYMBOL_exception_extends = 92,         /* exception_extends  */
  YYSYMBOL_exception_exports = 93,         /* exception_exports  */
  YYSYMBOL_type_id = 94,                   /* type_id  */
  YYSYMBOL_tag = 95,                       /* tag  */
  YYSYMBOL_optional = 96,                  /* optional  */
  YYSYMBOL_tagged_type_id = 97,            /* tagged_type_id  */
  YYSYMBOL_exception_export = 98,          /* exception_export  */
  YYSYMBOL_struct_id = 99,                 /* struct_id  */
  YYSYMBOL_struct_decl = 100,              /* struct_decl  */
  YYSYMBOL_struct_def = 101,               /* struct_def  */
  YYSYMBOL_102_17 = 102,                   /* @17  */
  YYSYMBOL_struct_exports = 103,           /* struct_exports  */
  YYSYMBOL_struct_export = 104,            /* struct_export  */
  YYSYMBOL_class_name = 105,               /* class_name  */
  YYSYMBOL_class_id = 106,                 /* class_id  */
  YYSYMBOL_class_decl = 107,               /* class_decl  */
  YYSYMBOL_class_def = 108,                /* class_def  */
  YYSYMBOL_109_18 = 109,                   /* @18  */
  YYSYMBOL_class_extends = 110,            /* class_extends  */
  YYSYMBOL_extends = 111,                  /* extends  */
  YYSYMBOL_class_exports = 112,            /* class_exports  */
  YYSYMBOL_data_member = 113,              /* data_member  */
  YYSYMBOL_struct_data_member = 114,       /* struct_data_member  */
  YYSYMBOL_return_type = 115,              /* return_type  */
  YYSYMBOL_operation_preamble = 116,       /* operation_preamble  */
  YYSYMBOL_operation = 117,                /* operation  */
  YYSYMBOL_118_19 = 118,                   /* @19  */
  YYSYMBOL_119_20 = 119,                   /* @20  */
  YYSYMBOL_class_export = 120,             /* class_export  */
  YYSYMBOL_interface_id = 121,             /* interface_id  */
  YYSYMBOL_interface_decl = 122,           /* interface_decl  */
  YYSYMBOL_interface_def = 123,            /* interface_def  */
  YYSYMBOL_124_21 = 124,                   /* @21  */
  YYSYMBOL_interface_list = 125,           /* interface_list  */
  YYSYMBOL_interface_extends = 126,        /* interface_extends  */
  YYSYMBOL_interface_exports = 127,        /* interface_exports  */
  YYSYMBOL_interface_export = 128,         /* interface_export  */
  YYSYMBOL_exception_list = 129,           /* exception_list  */
  YYSYMBOL_exception = 130,                /* exception  */
  YYSYMBOL_sequence_def = 131,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 132,           /* dictionary_def  */
  YYSYMBOL_enum_id = 133,                  /* enum_id  */
  YYSYMBOL_enum_def = 134,                 /* enum_def  */
  YYSYMBOL_135_22 = 135,                   /* @22  */
  YYSYMBOL_136_23 = 136,                   /* @23  */
  YYSYMBOL_enum_underlying = 137,          /* enum_underlying  */
  YYSYMBOL_enumerator_list = 138,          /* enumerator_list  */
  YYSYMBOL_enumerator = 139,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 140,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 141,            /* out_qualifier  */
  YYSYMBOL_parameters = 142,               /* parameters  */
  YYSYMBOL_throws = 143,                   /* throws  */
  YYSYMBOL_scoped_name = 144,              /* scoped_name  */
  YYSYMBOL_builtin = 145,                  /* builtin  */
  YYSYMBOL_type = 146,                     /* type  */
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


#line 483 "src/Slice/Grammar.cpp"

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
#define YYLAST   1112

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  87
/* YYNRULES -- Number of rules.  */
#define YYNRULES  251
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  361

#define YYMAXUTOK   308


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
       2,    57,    63,     2,    60,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    58,    54,
      61,    59,    62,    64,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    55,     2,    56,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53
};

#if YYDEBUG
  /* YYRLINEYYN -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   190,   190,   198,   201,   209,   213,   223,   227,   234,
     243,   251,   260,   269,   268,   274,   273,   278,   283,   282,
     288,   287,   292,   297,   296,   302,   301,   306,   311,   310,
     316,   315,   320,   325,   324,   330,   329,   334,   339,   338,
     343,   348,   347,   353,   352,   357,   361,   371,   370,   399,
     398,   459,   463,   474,   485,   484,   510,   518,   527,   536,
     539,   543,   551,   565,   583,   662,   668,   679,   701,   784,
     794,   809,   824,   845,   858,   864,   868,   879,   890,   889,
     930,   939,   942,   946,   954,   960,   964,   975,  1000,  1090,
    1102,  1115,  1114,  1148,  1182,  1191,  1194,  1202,  1211,  1214,
    1218,  1226,  1248,  1275,  1297,  1323,  1332,  1343,  1352,  1361,
    1370,  1379,  1389,  1403,  1416,  1433,  1439,  1449,  1473,  1498,
    1522,  1553,  1552,  1575,  1574,  1597,  1603,  1607,  1618,  1632,
    1631,  1665,  1700,  1735,  1740,  1750,  1754,  1763,  1772,  1775,
    1779,  1787,  1793,  1800,  1812,  1824,  1835,  1843,  1857,  1867,
    1883,  1887,  1899,  1898,  1930,  1929,  1947,  1951,  1960,  1966,
    1974,  1986,  1999,  2006,  2016,  2020,  2058,  2064,  2075,  2078,
    2094,  2110,  2122,  2134,  2145,  2161,  2165,  2174,  2177,  2185,
    2186,  2187,  2188,  2189,  2190,  2191,  2192,  2193,  2194,  2195,
    2196,  2197,  2198,  2199,  2204,  2208,  2212,  2216,  2221,  2232,
    2243,  2248,  2274,  2306,  2334,  2340,  2348,  2355,  2367,  2376,
    2385,  2425,  2432,  2439,  2451,  2460,  2474,  2475,  2476,  2477,
    2478,  2479,  2480,  2481,  2482,  2483,  2484,  2485,  2486,  2487,
    2488,  2489,  2490,  2491,  2492,  2493,  2494,  2495,  2496,  2497,
    2498,  2499,  2500,  2501,  2502,  2503,  2504,  2505,  2506,  2507,
    2508,  2509
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
  "ICE_VALUE", "ICE_STRING_LITERAL", "ICE_INTEGER_LITERAL",
  "ICE_FLOATING_POINT_LITERAL", "ICE_IDENTIFIER", "ICE_SCOPED_IDENTIFIER",
  "ICE_LOCAL_METADATA_OPEN", "ICE_LOCAL_METADATA_CLOSE",
  "ICE_FILE_METADATA_OPEN", "ICE_FILE_METADATA_IGNORE",
  "ICE_FILE_METADATA_CLOSE", "ICE_IDENT_OPEN", "ICE_KEYWORD_OPEN",
  "ICE_TAG_OPEN", "ICE_OPTIONAL_OPEN", "BAD_CHAR", "';'", "'{'", "'}'",
  "')'", "':'", "'='", "','", "'<'", "'>'", "'*'", "'?'", "$accept",
  "start", "opt_semicolon", "file_metadata", "local_metadata",
  "definitions", "definition", "$@1", "$@2", "$@3", "$@4", "$@5", "$@6",
  "$@7", "$@8", "$@9", "$@10", "$@11", "$@12", "$@13", "module_def", "@14",
  "@15", "exception_id", "exception_decl", "exception_def", "@16",
  "exception_extends", "exception_exports", "type_id", "tag", "optional",
  "tagged_type_id", "exception_export", "struct_id", "struct_decl",
  "struct_def", "@17", "struct_exports", "struct_export", "class_name",
  "class_id", "class_decl", "class_def", "@18", "class_extends", "extends",
  "class_exports", "data_member", "struct_data_member", "return_type",
  "operation_preamble", "operation", "@19", "@20", "class_export",
  "interface_id", "interface_decl", "interface_def", "@21",
  "interface_list", "interface_extends", "interface_exports",
  "interface_export", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_id", "enum_def", "@22", "@23", "enum_underlying",
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
     305,   306,   307,   308,    59,   123,   125,    41,    58,    61,
      44,    60,    62,    42,    63
};
#endif

#define YYPACT_NINF (-308)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-169)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACTSTATE-NUM -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -308,    16,    12,  -308,   -17,   -17,   -17,  -308,    98,   -17,
    -308,   -25,    34,    35,   -14,    76,   560,   648,   688,   728,
     -23,    17,   768,    28,   -17,  -308,  -308,     5,     1,  -308,
      32,    42,  -308,     6,     7,    46,  -308,    11,    56,  -308,
      61,    68,    66,  -308,    86,  -308,  -308,   -17,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,
      93,  -308,  -308,  -308,  -308,  -308,  -308,  -308,    28,    28,
    -308,    95,  -308,  1068,    -6,    99,  -308,  -308,  -308,    89,
     103,    99,   106,   128,    99,  -308,    89,   129,    99,    43,
    -308,   130,    99,   132,   133,   150,  -308,    99,   136,  -308,
     109,   140,   142,  -308,  -308,   146,  1068,  1068,   808,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,
    -308,  -308,  -308,  -308,    85,   141,    92,   144,   -27,  -308,
    -308,  -308,   149,  -308,  -308,  -308,   418,  -308,  -308,   154,
    -308,  -308,  -308,  -308,  -308,  -308,   137,   155,  -308,  -308,
    -308,  -308,  -308,   162,  -308,  -308,  -308,  -308,  -308,  -308,
     156,   159,   161,   166,   163,  -308,  -308,  -308,  -308,  -308,
    -308,  -308,   167,   173,   462,   171,  1031,   172,   506,    43,
     374,   808,    67,    82,   848,    28,   101,  -308,   808,   173,
    -308,  -308,  -308,  -308,  -308,  -308,  -308,   177,  1031,   176,
     418,  -308,  -308,   -12,    33,   168,   150,   150,   179,  -308,
     888,  -308,   180,  1031,   181,  -308,   182,   956,   183,   184,
    -308,  -308,  -308,  -308,  1068,  -308,  -308,  -308,  -308,  -308,
     462,  -308,   150,   150,   185,   188,  -308,   888,  -308,  -308,
     178,  -308,   186,   189,  -308,   191,   173,   190,   196,   193,
     418,  -308,  -308,   506,  -308,   199,  -308,   374,  -308,   994,
     150,   150,    96,   328,  -308,   200,  -308,  -308,  -308,   194,
    -308,  -308,  -308,   173,   462,  -308,  -308,  -308,  -308,  -308,
    -308,   173,   173,  -308,  -308,   506,  -308,   110,  -308,  -308,
    -308,  -308,   198,  -308,    28,   -11,   374,   928,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,  1031,  -308,   234,  -308,
    -308,  -308,   236,  -308,   888,   236,    28,   607,  -308,  -308,
    -308,  1031,  -308,   197,  -308,  -308,  -308,   888,   607,  -308,
    -308
};

  /* YYDEFACTSTATE-NUM -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      12,     0,     9,     1,     0,     0,     0,    10,     0,   205,
     207,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   154,     9,     0,    11,    13,    53,    27,    28,
      77,    32,    33,    90,    94,    17,    18,   128,    22,    23,
      37,    40,   157,    41,    45,   204,     7,     0,     5,     6,
      46,    47,    49,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   251,    85,
       0,    86,   126,   127,    51,    52,    75,    76,     9,     9,
     150,     0,   151,     0,     0,     4,    95,    96,    54,     0,
       0,     4,     0,     0,     4,    91,     0,     0,     4,     0,
     129,     0,     4,     0,     0,     0,   152,     4,     0,   206,
       0,     0,     0,   177,   178,     0,     0,     0,   163,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   198,   199,   201,   200,     0,     8,
       3,    14,     0,    56,    26,    29,     0,    31,    34,     0,
      93,    16,    19,   133,   134,   135,   132,     0,    21,    24,
      36,    39,   156,     0,    42,    44,    12,    12,    87,    88,
       0,     0,   160,     0,   159,   162,   194,   195,   196,   202,
     203,   197,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   163,     9,     9,     0,     9,     0,   155,   163,     0,
     212,   213,   211,   208,   209,   210,   215,     0,     0,     0,
       0,    66,    70,     0,     0,   105,     0,     0,    82,    84,
     112,    79,     0,     0,     0,   131,     0,     0,     0,     0,
      48,    50,   146,   147,     0,   164,   161,   165,   158,   214,
       0,    73,     0,     0,   101,    60,    74,   104,    55,    81,
       0,    65,     0,     0,    69,     0,     0,   107,     0,   109,
       0,    62,   111,     0,   125,    99,    92,     0,   116,     0,
       0,     0,     0,     0,   141,   139,   115,   130,   153,     0,
      59,    71,    72,     0,     0,   103,    63,    64,    67,    68,
     106,     0,     0,    80,    98,     0,   138,     0,   113,   114,
     117,   119,     0,   166,     9,     0,     0,     0,   102,    58,
     108,   110,    97,   118,   120,   123,     0,   121,   167,   137,
     148,   149,   176,   169,   173,   176,     9,     0,   124,   171,
     122,     0,   175,   143,   144,   145,   170,   174,     0,   172,
     142
};

  /* YYPGOTONTERM-NUM.  */
static const yytype_int16 yypgoto[] =
{
    -308,  -308,    -2,  -308,   -19,   -24,  -308,  -308,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -234,  -195,
    -197,  -196,  -307,  -308,  -308,  -308,  -308,  -308,  -223,  -308,
    -308,  -308,  -308,  -308,  -308,  -308,    54,  -262,    15,  -308,
     -30,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,  -308,
      51,  -308,  -274,  -308,   -95,  -308,  -308,  -308,  -308,  -308,
    -308,  -308,  -308,  -134,  -308,  -308,   -72,  -308,   -78,   -82,
    -308,   -89,    24,    19,  -214,  -308,   -16
};

  /* YYDEFGOTONTERM-NUM.  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   161,     7,     8,     2,    25,   105,   117,   118,
     121,   122,   110,   111,   113,   114,   123,   124,   127,   128,
      26,   130,   131,    27,    28,    29,   162,   108,   229,   261,
     262,   263,   264,   265,    30,    31,    32,   112,   207,   238,
      33,    34,    35,    36,   169,   115,   109,   244,   266,   239,
     292,   293,   294,   345,   342,   285,    37,    38,    39,   177,
     175,   120,   248,   295,   352,   353,    40,    41,    42,    43,
     183,   101,   126,   193,   194,   256,   324,   325,   348,   156,
     157,   278,    10,    11,   226,    44,   195
};

  /* YYTABLEYYPACT[STATE-NUM] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      91,    93,    95,    97,   103,   259,   102,   269,   135,   236,
     237,   235,    -2,   316,   158,   202,     3,   106,   -89,   106,
      46,   314,     9,   106,    12,    13,   300,   163,   270,   343,
     133,   134,   203,    45,   170,    47,   182,   176,    98,   159,
      50,   277,   279,   104,   356,   271,   337,   190,   191,   338,
     290,   291,   339,   332,    47,   -25,     4,   313,     5,     6,
     -57,   -89,   310,   107,   -89,   107,  -136,   301,   302,   107,
     329,   129,     4,   273,   173,   133,   134,   249,    99,   136,
     137,   174,    48,    49,   258,   133,   134,   -78,   116,   328,
     274,   119,   290,   291,    47,    47,   -30,   330,   331,    14,
     -15,    15,    16,    17,    18,    19,    20,    21,    22,   165,
     -20,     4,   168,     5,     6,   -35,   172,   240,    51,    52,
     179,   225,   -38,   250,   125,   184,     4,   176,     5,     6,
      23,   133,   134,   132,   257,   133,   134,   225,   251,   267,
     -43,   255,    24,   133,   134,   320,   321,   206,   196,   197,
     138,   272,   275,   160,   267,   199,   200,   164,   296,   333,
     334,   166,   212,   213,   186,   299,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   167,   171,   178,   228,   180,   181,   155,   243,
     185,   247,   133,   134,   225,   187,   254,   209,   253,   188,
     296,   318,   319,   189,   204,   198,   220,   221,   201,   208,
     210,   206,   222,   223,   224,   133,   134,   211,   214,   215,
     216,   225,   217,   218,   282,   230,   219,   276,   241,   225,
     225,   260,   268,   280,   283,   306,   287,   286,   281,   297,
     298,   228,   304,   307,   303,   323,   308,   344,   309,   311,
     347,   305,   312,   315,   326,   335,   327,   358,   284,   317,
     245,   206,   357,   360,   243,   354,   346,   350,   247,     0,
       0,     0,     0,     0,     0,     0,   354,     0,     0,     0,
       0,     0,     0,     0,     0,   228,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   243,     0,     0,     0,
       0,     0,     0,     0,     0,   336,     0,   247,     0,     0,
       0,   341,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   351,   349,   322,
       0,   355,     0,     0,     0,     0,     0,     0,     0,   323,
       0,   359,   355,     0,  -167,  -167,  -167,  -167,  -167,  -167,
    -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,  -167,
       0,     0,     0,     0,  -167,  -167,  -167,     0,     0,     0,
    -167,  -167,  -167,     0,     0,   246,     0,     0,     0,  -167,
    -167,     0,     0,     0,     0,  -168,     0,     0,  -168,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,     0,     0,     0,    -9,
      -9,    -9,    -9,     0,     0,     0,    -9,    -9,     4,   205,
       0,     0,     0,     0,     0,    -9,    -9,     0,     0,     0,
    -140,     0,     0,     0,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
       0,     0,     0,     0,    -9,    -9,    -9,     0,     0,     0,
      -9,    -9,     4,   227,     0,     0,     0,     0,     0,    -9,
      -9,     0,     0,     0,   -83,     0,     0,     0,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,     0,     0,     0,     0,    -9,    -9,
      -9,     0,     0,     0,    -9,    -9,     4,   242,     0,     0,
       0,     0,     0,    -9,    -9,     0,     0,     0,   -61,     0,
       0,     0,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,     0,     0,
       0,     0,    -9,    -9,    -9,     0,     0,     0,    -9,    -9,
       4,     0,     0,     0,     0,     0,     0,    -9,    -9,     0,
       0,     0,  -100,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,     0,
       0,     0,    89,     0,     0,     0,     0,     0,     0,    90,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,     0,     0,     0,   133,
     134,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
      92,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
      94,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
      96,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
     100,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
     192,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
     252,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
     281,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,     0,     0,     0,
     340,   288,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,     0,     0,
       0,   289,   231,   232,   155,     0,     0,     0,   133,   134,
      24,     0,     0,     0,     0,     0,     0,   233,   234,   288,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,     0,     0,     0,     0,
     231,   232,   155,     0,     0,     0,   133,   134,     0,     0,
       0,     0,     0,     0,     0,   233,   234,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,     0,     0,     0,     0,   231,   232,   155,
       0,     0,     0,   133,   134,    24,     0,     0,     0,     0,
       0,     0,   233,   234,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
       0,     0,     0,     0,     0,     0,   155,     0,     0,     0,
     133,   134,    24
};

static const yytype_int16 yycheck[] =
{
      16,    17,    18,    19,    23,   219,    22,   230,    90,   206,
     206,   206,     0,   287,   103,    42,     0,    12,    12,    12,
      45,   283,    39,    12,     5,     6,   260,   109,    40,   336,
      42,    43,    59,     9,   116,    60,   125,   119,    61,    45,
      54,   236,   237,    24,   351,    57,    57,   136,   137,    60,
     247,   247,   326,   315,    60,    54,    44,   280,    46,    47,
      55,    55,   276,    58,    58,    58,    55,   262,   263,    58,
     304,    47,    44,    40,    31,    42,    43,   211,    61,    98,
      99,    38,    48,    48,   218,    42,    43,    55,    34,   303,
      57,    37,   289,   289,    60,    60,    54,   311,   312,     1,
      54,     3,     4,     5,     6,     7,     8,     9,    10,   111,
      54,    44,   114,    46,    47,    54,   118,   206,    42,    43,
     122,   203,    54,    56,    58,   127,    44,   209,    46,    47,
      32,    42,    43,    40,   216,    42,    43,   219,    56,   228,
      54,    40,    44,    42,    43,    49,    50,   166,    63,    64,
      55,   233,   234,    54,   243,    63,    64,    54,   247,    49,
      50,    55,   186,   187,    55,   254,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    54,    54,    54,   204,    54,    54,    38,   208,
      54,   210,    42,    43,   276,    55,   215,    60,   214,    57,
     289,   290,   291,    57,    55,    64,    33,    34,    64,    55,
      55,   230,    39,    40,    41,    42,    43,    55,    62,    60,
      59,   303,    56,    60,   240,    54,    59,    59,    56,   311,
     312,    54,    56,    54,    54,    57,    54,    56,    42,    56,
      56,   260,    54,    57,    59,    11,    57,   336,    57,    59,
      14,   267,    59,    54,    54,    57,    62,    60,   243,   289,
     209,   280,   351,   358,   283,   347,   338,   345,   287,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   358,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   304,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   315,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   324,    -1,   326,    -1,    -1,
      -1,   327,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   346,   344,     1,
      -1,   347,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    11,
      -1,   357,   358,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    36,    37,    38,    -1,    -1,    -1,
      42,    43,    44,    -1,    -1,     1,    -1,    -1,    -1,    51,
      52,    -1,    -1,    -1,    -1,    57,    -1,    -1,    60,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    35,
      36,    37,    38,    -1,    -1,    -1,    42,    43,    44,     1,
      -1,    -1,    -1,    -1,    -1,    51,    52,    -1,    -1,    -1,
      56,    -1,    -1,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    36,    37,    38,    -1,    -1,    -1,
      42,    43,    44,     1,    -1,    -1,    -1,    -1,    -1,    51,
      52,    -1,    -1,    -1,    56,    -1,    -1,    -1,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    -1,    36,    37,
      38,    -1,    -1,    -1,    42,    43,    44,     1,    -1,    -1,
      -1,    -1,    -1,    51,    52,    -1,    -1,    -1,    56,    -1,
      -1,    -1,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    -1,    36,    37,    38,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    -1,
      -1,    -1,    56,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    49,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    42,
      43,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      42,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      42,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      42,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      42,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      42,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      42,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      42,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      42,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    -1,    -1,
      -1,    35,    36,    37,    38,    -1,    -1,    -1,    42,    43,
      44,    -1,    -1,    -1,    -1,    -1,    -1,    51,    52,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    -1,    -1,    -1,    -1,
      36,    37,    38,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,    52,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    -1,    -1,    -1,    -1,    36,    37,    38,
      -1,    -1,    -1,    42,    43,    44,    -1,    -1,    -1,    -1,
      -1,    -1,    51,    52,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      42,    43,    44
};

  /* YYSTOSSTATE-NUM -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    66,    70,     0,    44,    46,    47,    68,    69,    39,
     147,   148,   148,   148,     1,     3,     4,     5,     6,     7,
       8,     9,    10,    32,    44,    71,    85,    88,    89,    90,
      99,   100,   101,   105,   106,   107,   108,   121,   122,   123,
     131,   132,   133,   134,   150,   147,    45,    60,    48,    48,
      54,    42,    43,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    42,
      49,   151,    42,   151,    42,   151,    42,   151,    61,    61,
      42,   136,   151,    69,   148,    72,    12,    58,    92,   111,
      77,    78,   102,    79,    80,   110,   111,    73,    74,   111,
     126,    75,    76,    81,    82,    58,   137,    83,    84,   147,
      86,    87,    40,    42,    43,   144,    69,    69,    55,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    38,   144,   145,   146,    45,
      54,    67,    91,   144,    54,    67,    55,    54,    67,   109,
     144,    54,    67,    31,    38,   125,   144,   124,    54,    67,
      54,    54,   146,   135,    67,    54,    55,    55,    57,    57,
     146,   146,    42,   138,   139,   151,    63,    64,    64,    63,
      64,    64,    42,    59,    55,     1,    69,   103,    55,    60,
      55,    55,    70,    70,    62,    60,    59,    56,    60,    59,
      33,    34,    39,    40,    41,   144,   149,     1,    69,    93,
      54,    36,    37,    51,    52,    94,    95,    96,   104,   114,
     146,    56,     1,    69,   112,   125,     1,    69,   127,   138,
      56,    56,    42,   151,    69,    40,   140,   144,   138,   149,
      54,    94,    95,    96,    97,    98,   113,   146,    56,   103,
      40,    57,   144,    40,    57,   144,    59,    94,   146,    94,
      54,    42,   151,    54,   113,   120,    56,    54,    15,    35,
      95,    96,   115,   116,   117,   128,   146,    56,    56,   146,
      93,    94,    94,    59,    54,   151,    57,    57,    57,    57,
     149,    59,    59,   103,   112,    54,   127,   115,   146,   146,
      49,    50,     1,    11,   141,   142,    54,    62,   149,    93,
     149,   149,   112,    49,    50,    57,    69,    57,    60,   127,
      42,   151,   119,    97,   146,   118,   141,    14,   143,   151,
     143,    69,   129,   130,   144,   151,    97,   146,    60,   151,
     129
};

  /* YYR1YYN -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    67,    67,    68,    68,    69,    69,    69,
      70,    70,    70,    72,    71,    73,    71,    71,    74,    71,
      75,    71,    71,    76,    71,    77,    71,    71,    78,    71,
      79,    71,    71,    80,    71,    81,    71,    71,    82,    71,
      71,    83,    71,    84,    71,    71,    71,    86,    85,    87,
      85,    88,    88,    89,    91,    90,    92,    92,    93,    93,
      93,    93,    94,    95,    95,    95,    95,    96,    96,    96,
      96,    97,    97,    97,    98,    99,    99,   100,   102,   101,
     103,   103,   103,   103,   104,   105,   105,   106,   106,   106,
     107,   109,   108,   110,   110,   111,   111,   112,   112,   112,
     112,   113,   113,   113,   113,   114,   114,   114,   114,   114,
     114,   114,   114,   115,   115,   115,   115,   116,   116,   116,
     116,   118,   117,   119,   117,   120,   121,   121,   122,   124,
     123,   125,   125,   125,   125,   126,   126,   127,   127,   127,
     127,   128,   129,   129,   130,   130,   131,   131,   132,   132,
     133,   133,   135,   134,   136,   134,   137,   137,   138,   138,
     139,   139,   139,   139,   140,   140,   141,   141,   142,   142,
     142,   142,   142,   142,   142,   143,   143,   144,   144,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   146,   146,   146,   146,   146,   146,
     146,   146,   146,   146,   147,   147,   148,   148,   149,   149,
     149,   149,   149,   149,   150,   150,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151,   151,   151,   151,   151,   151,   151,   151,   151,
     151,   151
};

  /* YYR2YYN -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     3,     4,     0,
       2,     3,     0,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       0,     3,     1,     0,     3,     0,     3,     1,     0,     3,
       1,     0,     3,     0,     3,     1,     2,     0,     6,     0,
       6,     2,     2,     1,     0,     6,     2,     0,     4,     3,
       2,     0,     2,     3,     3,     2,     1,     3,     3,     2,
       1,     2,     2,     1,     1,     2,     2,     1,     0,     5,
       4,     3,     2,     0,     1,     2,     2,     4,     4,     1,
       1,     0,     6,     2,     0,     1,     1,     4,     3,     2,
       0,     1,     3,     2,     1,     1,     3,     2,     4,     2,
       4,     2,     1,     2,     2,     1,     1,     2,     3,     2,
       3,     0,     5,     0,     5,     1,     2,     2,     1,     0,
       6,     3,     1,     1,     1,     2,     0,     4,     3,     2,
       0,     1,     3,     1,     1,     1,     6,     6,     9,     9,
       2,     2,     0,     6,     0,     5,     2,     0,     3,     1,
       1,     3,     1,     0,     1,     1,     1,     0,     0,     3,
       5,     4,     6,     3,     5,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     1,     1,
       1,     1,     2,     2,     2,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     6,     5,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
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
/* The lookahead symbol.  */
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
    int yynerrs;

    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize;

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
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

  yynerrs = 0;
  yystate = 0;
  yyerrstatus = 0;

  yystacksize = YYINITDEPTH;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;


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
  case 2:
#line 191 "src/Slice/Grammar.y"
{
}
#line 2002 "src/Slice/Grammar.cpp"
    break;

  case 3:
#line 199 "src/Slice/Grammar.y"
{
}
#line 2009 "src/Slice/Grammar.cpp"
    break;

  case 4:
#line 202 "src/Slice/Grammar.y"
{
}
#line 2016 "src/Slice/Grammar.cpp"
    break;

  case 5:
#line 210 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 2024 "src/Slice/Grammar.cpp"
    break;

  case 6:
#line 214 "src/Slice/Grammar.y"
{
    unit->error("file metadata must appear before any definitions");
    yyval = yyvsp[-1]; // Dummy
}
#line 2033 "src/Slice/Grammar.cpp"
    break;

  case 7:
#line 224 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 2041 "src/Slice/Grammar.cpp"
    break;

  case 8:
#line 228 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata1 = StringListTokPtr::dynamicCast(yyvsp[-3]);
    StringListTokPtr metadata2 = StringListTokPtr::dynamicCast(yyvsp[-1]);
    metadata1->v.splice(metadata1->v.end(), metadata2->v);
    yyval = metadata1;
}
#line 2052 "src/Slice/Grammar.cpp"
    break;

  case 9:
#line 235 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 2060 "src/Slice/Grammar.cpp"
    break;

  case 10:
#line 244 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addFileMetaData(metaData->v);
    }
}
#line 2072 "src/Slice/Grammar.cpp"
    break;

  case 11:
#line 252 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2085 "src/Slice/Grammar.cpp"
    break;

  case 12:
#line 261 "src/Slice/Grammar.y"
{
}
#line 2092 "src/Slice/Grammar.cpp"
    break;

  case 13:
#line 269 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 2100 "src/Slice/Grammar.cpp"
    break;

  case 15:
#line 274 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 2108 "src/Slice/Grammar.cpp"
    break;

  case 17:
#line 279 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 2116 "src/Slice/Grammar.cpp"
    break;

  case 18:
#line 283 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 2124 "src/Slice/Grammar.cpp"
    break;

  case 20:
#line 288 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDeclPtr::dynamicCast(yyvsp[0]));
}
#line 2132 "src/Slice/Grammar.cpp"
    break;

  case 22:
#line 293 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 2140 "src/Slice/Grammar.cpp"
    break;

  case 23:
#line 297 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDefPtr::dynamicCast(yyvsp[0]));
}
#line 2148 "src/Slice/Grammar.cpp"
    break;

  case 25:
#line 302 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2156 "src/Slice/Grammar.cpp"
    break;

  case 27:
#line 307 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 2164 "src/Slice/Grammar.cpp"
    break;

  case 28:
#line 311 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 2172 "src/Slice/Grammar.cpp"
    break;

  case 30:
#line 316 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2180 "src/Slice/Grammar.cpp"
    break;

  case 32:
#line 321 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 2188 "src/Slice/Grammar.cpp"
    break;

  case 33:
#line 325 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 2196 "src/Slice/Grammar.cpp"
    break;

  case 35:
#line 330 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2204 "src/Slice/Grammar.cpp"
    break;

  case 37:
#line 335 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2212 "src/Slice/Grammar.cpp"
    break;

  case 38:
#line 339 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2220 "src/Slice/Grammar.cpp"
    break;

  case 40:
#line 344 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2228 "src/Slice/Grammar.cpp"
    break;

  case 41:
#line 348 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2236 "src/Slice/Grammar.cpp"
    break;

  case 43:
#line 353 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2244 "src/Slice/Grammar.cpp"
    break;

  case 45:
#line 358 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2252 "src/Slice/Grammar.cpp"
    break;

  case 46:
#line 362 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2260 "src/Slice/Grammar.cpp"
    break;

  case 47:
#line 371 "src/Slice/Grammar.y"
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
#line 2280 "src/Slice/Grammar.cpp"
    break;

  case 48:
#line 387 "src/Slice/Grammar.y"
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
#line 2296 "src/Slice/Grammar.cpp"
    break;

  case 49:
#line 399 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();

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
    for(size_t i = 0; i < modules.size(); i++)
    {
        ModulePtr module = cont->createModule(modules[i]);
        if(module)
        {
            cont->checkIntroduced(ident->v, module);
            unit->pushContainer(module);
            yyval = cont = module;
        }
        else
        {
            // If an error occurs creating one of the modules, back up the entire chain.
            for(; i > 0; i--)
            {
                unit->popContainer();
            }
            yyval = 0;
            break;
        }
    }
}
#line 2338 "src/Slice/Grammar.cpp"
    break;

  case 50:
#line 437 "src/Slice/Grammar.y"
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
#line 2360 "src/Slice/Grammar.cpp"
    break;

  case 51:
#line 460 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2368 "src/Slice/Grammar.cpp"
    break;

  case 52:
#line 464 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2378 "src/Slice/Grammar.cpp"
    break;

  case 53:
#line 475 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2387 "src/Slice/Grammar.cpp"
    break;

  case 54:
#line 485 "src/Slice/Grammar.y"
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
#line 2404 "src/Slice/Grammar.cpp"
    break;

  case 55:
#line 498 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2416 "src/Slice/Grammar.cpp"
    break;

  case 56:
#line 511 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2428 "src/Slice/Grammar.cpp"
    break;

  case 57:
#line 519 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2436 "src/Slice/Grammar.cpp"
    break;

  case 58:
#line 528 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2449 "src/Slice/Grammar.cpp"
    break;

  case 59:
#line 537 "src/Slice/Grammar.y"
{
}
#line 2456 "src/Slice/Grammar.cpp"
    break;

  case 60:
#line 540 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2464 "src/Slice/Grammar.cpp"
    break;

  case 61:
#line 544 "src/Slice/Grammar.y"
{
}
#line 2471 "src/Slice/Grammar.cpp"
    break;

  case 62:
#line 552 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    checkIdentifier(ident->v);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    yyval = typestring;
}
#line 2484 "src/Slice/Grammar.cpp"
    break;

  case 63:
#line 566 "src/Slice/Grammar.y"
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
#line 2506 "src/Slice/Grammar.cpp"
    break;

  case 64:
#line 584 "src/Slice/Grammar.y"
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
#line 2589 "src/Slice/Grammar.cpp"
    break;

  case 65:
#line 663 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2599 "src/Slice/Grammar.cpp"
    break;

  case 66:
#line 669 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2609 "src/Slice/Grammar.cpp"
    break;

  case 67:
#line 680 "src/Slice/Grammar.y"
{
    IntegerTokPtr i = IntegerTokPtr::dynamicCast(yyvsp[-1]);
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }

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
#line 2635 "src/Slice/Grammar.cpp"
    break;

  case 68:
#line 702 "src/Slice/Grammar.y"
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
#line 2722 "src/Slice/Grammar.cpp"
    break;

  case 69:
#line 785 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2736 "src/Slice/Grammar.cpp"
    break;

  case 70:
#line 795 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, string("The `optional' keyword is deprecated, use `tag' instead"));
    }
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2750 "src/Slice/Grammar.cpp"
    break;

  case 71:
#line 810 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

    OptionalPtr optional = OptionalPtr::dynamicCast(ts->v.first);
    if (!optional)
    {
       unit->error("Only optional types can be tagged.");
    }

    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2769 "src/Slice/Grammar.cpp"
    break;

  case 72:
#line 825 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);

    OptionalPtr optional = OptionalPtr::dynamicCast(ts->v.first);

    if (optional)
    {
        // Use the optional directly.
        m->type = ts->v.first;
        m->name = ts->v.second;
    }
    else
    {
        // Infer the type to be optional for backwards compatibility.
        m->type = new Optional(ts->v.first);
        m->name = ts->v.second;
    }
    yyval = m;
}
#line 2794 "src/Slice/Grammar.cpp"
    break;

  case 73:
#line 846 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = ts->v.first;
    m->name = ts->v.second;
    yyval = m;
}
#line 2806 "src/Slice/Grammar.cpp"
    break;

  case 75:
#line 865 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2814 "src/Slice/Grammar.cpp"
    break;

  case 76:
#line 869 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2824 "src/Slice/Grammar.cpp"
    break;

  case 77:
#line 880 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2833 "src/Slice/Grammar.cpp"
    break;

  case 78:
#line 890 "src/Slice/Grammar.y"
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
#line 2855 "src/Slice/Grammar.cpp"
    break;

  case 79:
#line 908 "src/Slice/Grammar.y"
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
#line 2877 "src/Slice/Grammar.cpp"
    break;

  case 80:
#line 931 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2890 "src/Slice/Grammar.cpp"
    break;

  case 81:
#line 940 "src/Slice/Grammar.y"
{
}
#line 2897 "src/Slice/Grammar.cpp"
    break;

  case 82:
#line 943 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2905 "src/Slice/Grammar.cpp"
    break;

  case 83:
#line 947 "src/Slice/Grammar.y"
{
}
#line 2912 "src/Slice/Grammar.cpp"
    break;

  case 85:
#line 961 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2920 "src/Slice/Grammar.cpp"
    break;

  case 86:
#line 965 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2930 "src/Slice/Grammar.cpp"
    break;

  case 87:
#line 976 "src/Slice/Grammar.y"
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
#line 2959 "src/Slice/Grammar.cpp"
    break;

  case 88:
#line 1001 "src/Slice/Grammar.y"
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
#line 3053 "src/Slice/Grammar.cpp"
    break;

  case 89:
#line 1091 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 3064 "src/Slice/Grammar.cpp"
    break;

  case 90:
#line 1103 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 3075 "src/Slice/Grammar.cpp"
    break;

  case 91:
#line 1115 "src/Slice/Grammar.y"
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
#line 3096 "src/Slice/Grammar.cpp"
    break;

  case 92:
#line 1132 "src/Slice/Grammar.y"
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
#line 3112 "src/Slice/Grammar.cpp"
    break;

  case 93:
#line 1149 "src/Slice/Grammar.y"
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
#line 3150 "src/Slice/Grammar.cpp"
    break;

  case 94:
#line 1183 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 3158 "src/Slice/Grammar.cpp"
    break;

  case 95:
#line 1192 "src/Slice/Grammar.y"
{
}
#line 3165 "src/Slice/Grammar.cpp"
    break;

  case 96:
#line 1195 "src/Slice/Grammar.y"
{
}
#line 3172 "src/Slice/Grammar.cpp"
    break;

  case 97:
#line 1203 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3185 "src/Slice/Grammar.cpp"
    break;

  case 98:
#line 1212 "src/Slice/Grammar.y"
{
}
#line 3192 "src/Slice/Grammar.cpp"
    break;

  case 99:
#line 1215 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3200 "src/Slice/Grammar.cpp"
    break;

  case 100:
#line 1219 "src/Slice/Grammar.y"
{
}
#line 3207 "src/Slice/Grammar.cpp"
    break;

  case 101:
#line 1227 "src/Slice/Grammar.y"
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
#line 3233 "src/Slice/Grammar.cpp"
    break;

  case 102:
#line 1249 "src/Slice/Grammar.y"
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
#line 3264 "src/Slice/Grammar.cpp"
    break;

  case 103:
#line 1276 "src/Slice/Grammar.y"
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
#line 3290 "src/Slice/Grammar.cpp"
    break;

  case 104:
#line 1298 "src/Slice/Grammar.y"
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
#line 3315 "src/Slice/Grammar.cpp"
    break;

  case 105:
#line 1324 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    DataMemberPtr dm = st->createDataMember(ts->v.second, ts->v.first, false, -1, 0, "", "");
    unit->currentContainer()->checkIntroduced(ts->v.second, dm);
    yyval = dm;
}
#line 3328 "src/Slice/Grammar.cpp"
    break;

  case 106:
#line 1333 "src/Slice/Grammar.y"
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
#line 3343 "src/Slice/Grammar.cpp"
    break;

  case 107:
#line 1344 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3356 "src/Slice/Grammar.cpp"
    break;

  case 108:
#line 1353 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3369 "src/Slice/Grammar.cpp"
    break;

  case 109:
#line 1362 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3382 "src/Slice/Grammar.cpp"
    break;

  case 110:
#line 1371 "src/Slice/Grammar.y"
{
    TypeStringTokPtr ts = TypeStringTokPtr::dynamicCast(yyvsp[-2]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(ts->v.second, ts->v.first, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("tagged data members are not supported in structs");
}
#line 3395 "src/Slice/Grammar.cpp"
    break;

  case 111:
#line 1380 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(name, type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3409 "src/Slice/Grammar.cpp"
    break;

  case 112:
#line 1390 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    assert(st);
    yyval = st->createDataMember(IceUtil::generateUUID(), type, false, 0, 0, "", ""); // Dummy
    assert(yyval);
    unit->error("missing data member name");
}
#line 3422 "src/Slice/Grammar.cpp"
    break;

  case 113:
#line 1404 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);

    OptionalPtr optional = OptionalPtr::dynamicCast(yyvsp[0]);
    if (!optional)
    {
        unit->error("Only optional types can be tagged.");
    }

    m->type = optional;
    yyval = m;
}
#line 3439 "src/Slice/Grammar.cpp"
    break;

  case 114:
#line 1417 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = TaggedDefTokPtr::dynamicCast(yyvsp[-1]);

    OptionalPtr optional = OptionalPtr::dynamicCast(yyvsp[0]);

    if (optional)
    {
        m->type = optional;
    }
    else
    {
        // Infer the type to be optional for backwards compatibility.
        m->type = new Optional(TypePtr::dynamicCast(yyvsp[0]));
    }
    yyval = m;
}
#line 3460 "src/Slice/Grammar.cpp"
    break;

  case 115:
#line 1434 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3470 "src/Slice/Grammar.cpp"
    break;

  case 116:
#line 1440 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    yyval = m;
}
#line 3479 "src/Slice/Grammar.cpp"
    break;

  case 117:
#line 1450 "src/Slice/Grammar.y"
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
#line 3507 "src/Slice/Grammar.cpp"
    break;

  case 118:
#line 1474 "src/Slice/Grammar.y"
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
#line 3536 "src/Slice/Grammar.cpp"
    break;

  case 119:
#line 1499 "src/Slice/Grammar.y"
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
#line 3564 "src/Slice/Grammar.cpp"
    break;

  case 120:
#line 1523 "src/Slice/Grammar.y"
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
#line 3593 "src/Slice/Grammar.cpp"
    break;

  case 121:
#line 1553 "src/Slice/Grammar.y"
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
#line 3609 "src/Slice/Grammar.cpp"
    break;

  case 122:
#line 1565 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3623 "src/Slice/Grammar.cpp"
    break;

  case 123:
#line 1575 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3635 "src/Slice/Grammar.cpp"
    break;

  case 124:
#line 1583 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3649 "src/Slice/Grammar.cpp"
    break;

  case 126:
#line 1604 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3657 "src/Slice/Grammar.cpp"
    break;

  case 127:
#line 1608 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3667 "src/Slice/Grammar.cpp"
    break;

  case 128:
#line 1619 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3679 "src/Slice/Grammar.cpp"
    break;

  case 129:
#line 1632 "src/Slice/Grammar.y"
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
#line 3700 "src/Slice/Grammar.cpp"
    break;

  case 130:
#line 1649 "src/Slice/Grammar.y"
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
#line 3716 "src/Slice/Grammar.cpp"
    break;

  case 131:
#line 1666 "src/Slice/Grammar.y"
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
#line 3755 "src/Slice/Grammar.cpp"
    break;

  case 132:
#line 1701 "src/Slice/Grammar.y"
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
#line 3794 "src/Slice/Grammar.cpp"
    break;

  case 133:
#line 1736 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new InterfaceListTok; // Dummy
}
#line 3803 "src/Slice/Grammar.cpp"
    break;

  case 134:
#line 1741 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3812 "src/Slice/Grammar.cpp"
    break;

  case 135:
#line 1751 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3820 "src/Slice/Grammar.cpp"
    break;

  case 136:
#line 1755 "src/Slice/Grammar.y"
{
    yyval = new InterfaceListTok;
}
#line 3828 "src/Slice/Grammar.cpp"
    break;

  case 137:
#line 1764 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3841 "src/Slice/Grammar.cpp"
    break;

  case 138:
#line 1773 "src/Slice/Grammar.y"
{
}
#line 3848 "src/Slice/Grammar.cpp"
    break;

  case 139:
#line 1776 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3856 "src/Slice/Grammar.cpp"
    break;

  case 140:
#line 1780 "src/Slice/Grammar.y"
{
}
#line 3863 "src/Slice/Grammar.cpp"
    break;

  case 142:
#line 1794 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3874 "src/Slice/Grammar.cpp"
    break;

  case 143:
#line 1801 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3885 "src/Slice/Grammar.cpp"
    break;

  case 144:
#line 1813 "src/Slice/Grammar.y"
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
#line 3901 "src/Slice/Grammar.cpp"
    break;

  case 145:
#line 1825 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentContainer()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3911 "src/Slice/Grammar.cpp"
    break;

  case 146:
#line 1836 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3923 "src/Slice/Grammar.cpp"
    break;

  case 147:
#line 1844 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3936 "src/Slice/Grammar.cpp"
    break;

  case 148:
#line 1858 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ContainerPtr cont = unit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3950 "src/Slice/Grammar.cpp"
    break;

  case 149:
#line 1868 "src/Slice/Grammar.y"
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
#line 3965 "src/Slice/Grammar.cpp"
    break;

  case 150:
#line 1884 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3973 "src/Slice/Grammar.cpp"
    break;

  case 151:
#line 1888 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = yyvsp[0]; // Dummy
}
#line 3983 "src/Slice/Grammar.cpp"
    break;

  case 152:
#line 1899 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-1]);
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v, TypePtr::dynamicCast(yyvsp[0]));
    if (en)
    {
        cont->checkIntroduced(ident->v, en);
    }
    else
    {
        en = cont->createEnum(IceUtil::generateUUID(), nullptr, Dummy);
    }
    unit->pushContainer(en);
    yyval = en;
}
#line 4003 "src/Slice/Grammar.cpp"
    break;

  case 153:
#line 1915 "src/Slice/Grammar.y"
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
#line 4021 "src/Slice/Grammar.cpp"
    break;

  case 154:
#line 1930 "src/Slice/Grammar.y"
{
    unit->error("missing enumeration name");
    ContainerPtr cont = unit->currentContainer();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), nullptr, Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 4033 "src/Slice/Grammar.cpp"
    break;

  case 155:
#line 1938 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 4042 "src/Slice/Grammar.cpp"
    break;

  case 156:
#line 1948 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4050 "src/Slice/Grammar.cpp"
    break;

  case 157:
#line 1952 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 4058 "src/Slice/Grammar.cpp"
    break;

  case 158:
#line 1961 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 4068 "src/Slice/Grammar.cpp"
    break;

  case 159:
#line 1967 "src/Slice/Grammar.y"
{
}
#line 4075 "src/Slice/Grammar.cpp"
    break;

  case 160:
#line 1975 "src/Slice/Grammar.y"
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
#line 4091 "src/Slice/Grammar.cpp"
    break;

  case 161:
#line 1987 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    ContainerPtr cont = unit->currentContainer();
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    if (intVal)
    {
       EnumeratorPtr en = cont->createEnumerator(ident->v, intVal->v);
       ens->v.push_front(en);
    }
    yyval = ens;
}
#line 4108 "src/Slice/Grammar.cpp"
    break;

  case 162:
#line 2000 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 4119 "src/Slice/Grammar.cpp"
    break;

  case 163:
#line 2007 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 4128 "src/Slice/Grammar.cpp"
    break;

  case 164:
#line 2017 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4136 "src/Slice/Grammar.cpp"
    break;

  case 165:
#line 2021 "src/Slice/Grammar.y"
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
#line 4173 "src/Slice/Grammar.cpp"
    break;

  case 166:
#line 2059 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = true;
    yyval = out;
}
#line 4183 "src/Slice/Grammar.cpp"
    break;

  case 167:
#line 2065 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = false;
    yyval = out;
}
#line 4193 "src/Slice/Grammar.cpp"
    break;

  case 168:
#line 2076 "src/Slice/Grammar.y"
{
}
#line 4200 "src/Slice/Grammar.cpp"
    break;

  case 169:
#line 2079 "src/Slice/Grammar.y"
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
#line 4220 "src/Slice/Grammar.cpp"
    break;

  case 170:
#line 2095 "src/Slice/Grammar.y"
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
#line 4240 "src/Slice/Grammar.cpp"
    break;

  case 171:
#line 2111 "src/Slice/Grammar.y"
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
#line 4256 "src/Slice/Grammar.cpp"
    break;

  case 172:
#line 2123 "src/Slice/Grammar.y"
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
#line 4272 "src/Slice/Grammar.cpp"
    break;

  case 173:
#line 2135 "src/Slice/Grammar.y"
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
#line 4287 "src/Slice/Grammar.cpp"
    break;

  case 174:
#line 2146 "src/Slice/Grammar.y"
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
#line 4302 "src/Slice/Grammar.cpp"
    break;

  case 175:
#line 2162 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4310 "src/Slice/Grammar.cpp"
    break;

  case 176:
#line 2166 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 4318 "src/Slice/Grammar.cpp"
    break;

  case 177:
#line 2175 "src/Slice/Grammar.y"
{
}
#line 4325 "src/Slice/Grammar.cpp"
    break;

  case 178:
#line 2178 "src/Slice/Grammar.y"
{
}
#line 4332 "src/Slice/Grammar.cpp"
    break;

  case 179:
#line 2185 "src/Slice/Grammar.y"
           {}
#line 4338 "src/Slice/Grammar.cpp"
    break;

  case 180:
#line 2186 "src/Slice/Grammar.y"
           {}
#line 4344 "src/Slice/Grammar.cpp"
    break;

  case 181:
#line 2187 "src/Slice/Grammar.y"
            {}
#line 4350 "src/Slice/Grammar.cpp"
    break;

  case 182:
#line 2188 "src/Slice/Grammar.y"
             {}
#line 4356 "src/Slice/Grammar.cpp"
    break;

  case 183:
#line 2189 "src/Slice/Grammar.y"
          {}
#line 4362 "src/Slice/Grammar.cpp"
    break;

  case 184:
#line 2190 "src/Slice/Grammar.y"
           {}
#line 4368 "src/Slice/Grammar.cpp"
    break;

  case 185:
#line 2191 "src/Slice/Grammar.y"
             {}
#line 4374 "src/Slice/Grammar.cpp"
    break;

  case 186:
#line 2192 "src/Slice/Grammar.y"
              {}
#line 4380 "src/Slice/Grammar.cpp"
    break;

  case 187:
#line 2193 "src/Slice/Grammar.y"
           {}
#line 4386 "src/Slice/Grammar.cpp"
    break;

  case 188:
#line 2194 "src/Slice/Grammar.y"
            {}
#line 4392 "src/Slice/Grammar.cpp"
    break;

  case 189:
#line 2195 "src/Slice/Grammar.y"
              {}
#line 4398 "src/Slice/Grammar.cpp"
    break;

  case 190:
#line 2196 "src/Slice/Grammar.y"
               {}
#line 4404 "src/Slice/Grammar.cpp"
    break;

  case 191:
#line 2197 "src/Slice/Grammar.y"
            {}
#line 4410 "src/Slice/Grammar.cpp"
    break;

  case 192:
#line 2198 "src/Slice/Grammar.y"
             {}
#line 4416 "src/Slice/Grammar.cpp"
    break;

  case 193:
#line 2199 "src/Slice/Grammar.y"
             {}
#line 4422 "src/Slice/Grammar.cpp"
    break;

  case 194:
#line 2205 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 4430 "src/Slice/Grammar.cpp"
    break;

  case 195:
#line 2209 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 4438 "src/Slice/Grammar.cpp"
    break;

  case 196:
#line 2213 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindValue);
}
#line 4446 "src/Slice/Grammar.cpp"
    break;

  case 197:
#line 2217 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[-1]);
    yyval = unit->optionalBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 4455 "src/Slice/Grammar.cpp"
    break;

  case 198:
#line 2222 "src/Slice/Grammar.y"
{
    if (unit->compatMode())
    {
        yyval = unit->optionalBuiltin(Builtin::KindValue);
    }
    else
    {
        yyval = unit->builtin(Builtin::KindObject);
    }
}
#line 4470 "src/Slice/Grammar.cpp"
    break;

  case 199:
#line 2233 "src/Slice/Grammar.y"
{
    if (unit->compatMode())
    {
        yyval = unit->optionalBuiltin(Builtin::KindValue);
    }
    else
    {
        yyval = unit->builtin(Builtin::KindValue);
    }
}
#line 4485 "src/Slice/Grammar.cpp"
    break;

  case 200:
#line 2244 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 4494 "src/Slice/Grammar.cpp"
    break;

  case 201:
#line 2249 "src/Slice/Grammar.y"
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
#line 4524 "src/Slice/Grammar.cpp"
    break;

  case 202:
#line 2275 "src/Slice/Grammar.y"
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
#line 4560 "src/Slice/Grammar.cpp"
    break;

  case 203:
#line 2307 "src/Slice/Grammar.y"
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
#line 4587 "src/Slice/Grammar.cpp"
    break;

  case 204:
#line 2335 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4597 "src/Slice/Grammar.cpp"
    break;

  case 205:
#line 2341 "src/Slice/Grammar.y"
{
}
#line 4604 "src/Slice/Grammar.cpp"
    break;

  case 206:
#line 2349 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4615 "src/Slice/Grammar.cpp"
    break;

  case 207:
#line 2356 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4626 "src/Slice/Grammar.cpp"
    break;

  case 208:
#line 2368 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4639 "src/Slice/Grammar.cpp"
    break;

  case 209:
#line 2377 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4652 "src/Slice/Grammar.cpp"
    break;

  case 210:
#line 2386 "src/Slice/Grammar.y"
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
#line 4696 "src/Slice/Grammar.cpp"
    break;

  case 211:
#line 2426 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4707 "src/Slice/Grammar.cpp"
    break;

  case 212:
#line 2433 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4718 "src/Slice/Grammar.cpp"
    break;

  case 213:
#line 2440 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4729 "src/Slice/Grammar.cpp"
    break;

  case 214:
#line 2452 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentContainer()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4742 "src/Slice/Grammar.cpp"
    break;

  case 215:
#line 2461 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentContainer()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4755 "src/Slice/Grammar.cpp"
    break;

  case 216:
#line 2474 "src/Slice/Grammar.y"
             {}
#line 4761 "src/Slice/Grammar.cpp"
    break;

  case 217:
#line 2475 "src/Slice/Grammar.y"
            {}
#line 4767 "src/Slice/Grammar.cpp"
    break;

  case 218:
#line 2476 "src/Slice/Grammar.y"
                {}
#line 4773 "src/Slice/Grammar.cpp"
    break;

  case 219:
#line 2477 "src/Slice/Grammar.y"
                {}
#line 4779 "src/Slice/Grammar.cpp"
    break;

  case 220:
#line 2478 "src/Slice/Grammar.y"
             {}
#line 4785 "src/Slice/Grammar.cpp"
    break;

  case 221:
#line 2479 "src/Slice/Grammar.y"
               {}
#line 4791 "src/Slice/Grammar.cpp"
    break;

  case 222:
#line 2480 "src/Slice/Grammar.y"
                 {}
#line 4797 "src/Slice/Grammar.cpp"
    break;

  case 223:
#line 2481 "src/Slice/Grammar.y"
           {}
#line 4803 "src/Slice/Grammar.cpp"
    break;

  case 224:
#line 2482 "src/Slice/Grammar.y"
          {}
#line 4809 "src/Slice/Grammar.cpp"
    break;

  case 225:
#line 2483 "src/Slice/Grammar.y"
              {}
#line 4815 "src/Slice/Grammar.cpp"
    break;

  case 226:
#line 2484 "src/Slice/Grammar.y"
                 {}
#line 4821 "src/Slice/Grammar.cpp"
    break;

  case 227:
#line 2485 "src/Slice/Grammar.y"
             {}
#line 4827 "src/Slice/Grammar.cpp"
    break;

  case 228:
#line 2486 "src/Slice/Grammar.y"
           {}
#line 4833 "src/Slice/Grammar.cpp"
    break;

  case 229:
#line 2487 "src/Slice/Grammar.y"
           {}
#line 4839 "src/Slice/Grammar.cpp"
    break;

  case 230:
#line 2488 "src/Slice/Grammar.y"
           {}
#line 4845 "src/Slice/Grammar.cpp"
    break;

  case 231:
#line 2489 "src/Slice/Grammar.y"
            {}
#line 4851 "src/Slice/Grammar.cpp"
    break;

  case 232:
#line 2490 "src/Slice/Grammar.y"
             {}
#line 4857 "src/Slice/Grammar.cpp"
    break;

  case 233:
#line 2491 "src/Slice/Grammar.y"
          {}
#line 4863 "src/Slice/Grammar.cpp"
    break;

  case 234:
#line 2492 "src/Slice/Grammar.y"
           {}
#line 4869 "src/Slice/Grammar.cpp"
    break;

  case 235:
#line 2493 "src/Slice/Grammar.y"
             {}
#line 4875 "src/Slice/Grammar.cpp"
    break;

  case 236:
#line 2494 "src/Slice/Grammar.y"
              {}
#line 4881 "src/Slice/Grammar.cpp"
    break;

  case 237:
#line 2495 "src/Slice/Grammar.y"
           {}
#line 4887 "src/Slice/Grammar.cpp"
    break;

  case 238:
#line 2496 "src/Slice/Grammar.y"
            {}
#line 4893 "src/Slice/Grammar.cpp"
    break;

  case 239:
#line 2497 "src/Slice/Grammar.y"
              {}
#line 4899 "src/Slice/Grammar.cpp"
    break;

  case 240:
#line 2498 "src/Slice/Grammar.y"
               {}
#line 4905 "src/Slice/Grammar.cpp"
    break;

  case 241:
#line 2499 "src/Slice/Grammar.y"
            {}
#line 4911 "src/Slice/Grammar.cpp"
    break;

  case 242:
#line 2500 "src/Slice/Grammar.y"
             {}
#line 4917 "src/Slice/Grammar.cpp"
    break;

  case 243:
#line 2501 "src/Slice/Grammar.y"
             {}
#line 4923 "src/Slice/Grammar.cpp"
    break;

  case 244:
#line 2502 "src/Slice/Grammar.y"
             {}
#line 4929 "src/Slice/Grammar.cpp"
    break;

  case 245:
#line 2503 "src/Slice/Grammar.y"
            {}
#line 4935 "src/Slice/Grammar.cpp"
    break;

  case 246:
#line 2504 "src/Slice/Grammar.y"
            {}
#line 4941 "src/Slice/Grammar.cpp"
    break;

  case 247:
#line 2505 "src/Slice/Grammar.y"
           {}
#line 4947 "src/Slice/Grammar.cpp"
    break;

  case 248:
#line 2506 "src/Slice/Grammar.y"
                 {}
#line 4953 "src/Slice/Grammar.cpp"
    break;

  case 249:
#line 2507 "src/Slice/Grammar.y"
          {}
#line 4959 "src/Slice/Grammar.cpp"
    break;

  case 250:
#line 2508 "src/Slice/Grammar.y"
               {}
#line 4965 "src/Slice/Grammar.cpp"
    break;

  case 251:
#line 2509 "src/Slice/Grammar.y"
            {}
#line 4971 "src/Slice/Grammar.cpp"
    break;


#line 4975 "src/Slice/Grammar.cpp"

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
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
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

#line 2512 "src/Slice/Grammar.y"

