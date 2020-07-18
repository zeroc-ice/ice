/* A Bison parser, made by GNU Bison 3.6.4.  */

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
#define YYBISON_VERSION "3.6.4"

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
    ICE_ANYCLASS = 293,            /* ICE_ANYCLASS  */
    ICE_VALUE = 294,               /* ICE_VALUE  */
    ICE_UNCHECKED = 295,           /* ICE_UNCHECKED  */
    ICE_STRING_LITERAL = 296,      /* ICE_STRING_LITERAL  */
    ICE_INTEGER_LITERAL = 297,     /* ICE_INTEGER_LITERAL  */
    ICE_FLOATING_POINT_LITERAL = 298, /* ICE_FLOATING_POINT_LITERAL  */
    ICE_IDENTIFIER = 299,          /* ICE_IDENTIFIER  */
    ICE_SCOPED_IDENTIFIER = 300,   /* ICE_SCOPED_IDENTIFIER  */
    ICE_LOCAL_METADATA_OPEN = 301, /* ICE_LOCAL_METADATA_OPEN  */
    ICE_LOCAL_METADATA_CLOSE = 302, /* ICE_LOCAL_METADATA_CLOSE  */
    ICE_FILE_METADATA_OPEN = 303,  /* ICE_FILE_METADATA_OPEN  */
    ICE_FILE_METADATA_IGNORE = 304, /* ICE_FILE_METADATA_IGNORE  */
    ICE_FILE_METADATA_CLOSE = 305, /* ICE_FILE_METADATA_CLOSE  */
    ICE_IDENT_OPEN = 306,          /* ICE_IDENT_OPEN  */
    ICE_KEYWORD_OPEN = 307,        /* ICE_KEYWORD_OPEN  */
    ICE_TAG_OPEN = 308,            /* ICE_TAG_OPEN  */
    ICE_OPTIONAL_OPEN = 309,       /* ICE_OPTIONAL_OPEN  */
    BAD_CHAR = 310                 /* BAD_CHAR  */
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
  YYSYMBOL_exception_exports = 95,         /* exception_exports  */
  YYSYMBOL_type_id = 96,                   /* type_id  */
  YYSYMBOL_tag = 97,                       /* tag  */
  YYSYMBOL_optional = 98,                  /* optional  */
  YYSYMBOL_tagged_type_id = 99,            /* tagged_type_id  */
  YYSYMBOL_exception_export = 100,         /* exception_export  */
  YYSYMBOL_struct_id = 101,                /* struct_id  */
  YYSYMBOL_struct_decl = 102,              /* struct_decl  */
  YYSYMBOL_struct_def = 103,               /* struct_def  */
  YYSYMBOL_104_17 = 104,                   /* @17  */
  YYSYMBOL_struct_exports = 105,           /* struct_exports  */
  YYSYMBOL_struct_export = 106,            /* struct_export  */
  YYSYMBOL_class_name = 107,               /* class_name  */
  YYSYMBOL_class_id = 108,                 /* class_id  */
  YYSYMBOL_class_decl = 109,               /* class_decl  */
  YYSYMBOL_class_def = 110,                /* class_def  */
  YYSYMBOL_111_18 = 111,                   /* @18  */
  YYSYMBOL_class_extends = 112,            /* class_extends  */
  YYSYMBOL_extends = 113,                  /* extends  */
  YYSYMBOL_class_exports = 114,            /* class_exports  */
  YYSYMBOL_data_member = 115,              /* data_member  */
  YYSYMBOL_return_type = 116,              /* return_type  */
  YYSYMBOL_operation_preamble = 117,       /* operation_preamble  */
  YYSYMBOL_operation = 118,                /* operation  */
  YYSYMBOL_119_19 = 119,                   /* @19  */
  YYSYMBOL_120_20 = 120,                   /* @20  */
  YYSYMBOL_class_export = 121,             /* class_export  */
  YYSYMBOL_interface_id = 122,             /* interface_id  */
  YYSYMBOL_interface_decl = 123,           /* interface_decl  */
  YYSYMBOL_interface_def = 124,            /* interface_def  */
  YYSYMBOL_125_21 = 125,                   /* @21  */
  YYSYMBOL_interface_list = 126,           /* interface_list  */
  YYSYMBOL_interface_extends = 127,        /* interface_extends  */
  YYSYMBOL_interface_exports = 128,        /* interface_exports  */
  YYSYMBOL_interface_export = 129,         /* interface_export  */
  YYSYMBOL_exception_list = 130,           /* exception_list  */
  YYSYMBOL_exception = 131,                /* exception  */
  YYSYMBOL_sequence_def = 132,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 133,           /* dictionary_def  */
  YYSYMBOL_enum_start = 134,               /* enum_start  */
  YYSYMBOL_enum_id = 135,                  /* enum_id  */
  YYSYMBOL_enum_def = 136,                 /* enum_def  */
  YYSYMBOL_137_22 = 137,                   /* @22  */
  YYSYMBOL_138_23 = 138,                   /* @23  */
  YYSYMBOL_enum_underlying = 139,          /* enum_underlying  */
  YYSYMBOL_enumerator_list = 140,          /* enumerator_list  */
  YYSYMBOL_enumerator = 141,               /* enumerator  */
  YYSYMBOL_enumerator_initializer = 142,   /* enumerator_initializer  */
  YYSYMBOL_out_qualifier = 143,            /* out_qualifier  */
  YYSYMBOL_parameters = 144,               /* parameters  */
  YYSYMBOL_throws = 145,                   /* throws  */
  YYSYMBOL_scoped_name = 146,              /* scoped_name  */
  YYSYMBOL_builtin = 147,                  /* builtin  */
  YYSYMBOL_type = 148,                     /* type  */
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


#line 487 "src/Slice/Grammar.cpp"

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
#define YYLAST   1034

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  87
/* YYNRULES -- Number of rules.  */
#define YYNRULES  249
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  355

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
       0,   192,   192,   200,   203,   211,   215,   225,   229,   236,
     245,   253,   262,   271,   270,   276,   275,   280,   285,   284,
     290,   289,   294,   299,   298,   304,   303,   308,   313,   312,
     318,   317,   322,   327,   326,   332,   331,   336,   341,   340,
     345,   350,   349,   355,   354,   359,   363,   373,   372,   411,
     410,   488,   492,   503,   514,   513,   539,   547,   556,   565,
     568,   572,   580,   594,   612,   686,   692,   703,   725,   803,
     813,   828,   843,   864,   877,   883,   887,   898,   909,   908,
     949,   958,   961,   965,   973,   979,   983,   994,  1019,  1109,
    1121,  1134,  1133,  1167,  1201,  1210,  1213,  1221,  1230,  1233,
    1237,  1245,  1267,  1294,  1316,  1342,  1355,  1372,  1378,  1388,
    1412,  1437,  1461,  1492,  1491,  1514,  1513,  1536,  1542,  1546,
    1557,  1571,  1570,  1604,  1639,  1674,  1679,  1684,  1698,  1702,
    1711,  1720,  1723,  1727,  1735,  1741,  1748,  1760,  1772,  1783,
    1791,  1805,  1815,  1831,  1837,  1848,  1864,  1878,  1877,  1900,
    1899,  1918,  1922,  1931,  1937,  1945,  1957,  1970,  1977,  1987,
    1991,  2029,  2035,  2046,  2049,  2065,  2081,  2093,  2105,  2116,
    2132,  2136,  2145,  2148,  2156,  2157,  2158,  2159,  2160,  2161,
    2162,  2163,  2164,  2165,  2166,  2167,  2168,  2169,  2170,  2175,
    2179,  2183,  2187,  2195,  2200,  2211,  2215,  2227,  2232,  2258,
    2290,  2318,  2324,  2332,  2339,  2351,  2360,  2369,  2409,  2416,
    2423,  2435,  2444,  2458,  2459,  2460,  2461,  2462,  2463,  2464,
    2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,  2473,  2474,
    2475,  2476,  2477,  2478,  2479,  2480,  2481,  2482,  2483,  2484,
    2485,  2486,  2487,  2488,  2489,  2490,  2491,  2492,  2493,  2494
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
  "exception_def", "@16", "exception_extends", "exception_exports",
  "type_id", "tag", "optional", "tagged_type_id", "exception_export",
  "struct_id", "struct_decl", "struct_def", "@17", "struct_exports",
  "struct_export", "class_name", "class_id", "class_decl", "class_def",
  "@18", "class_extends", "extends", "class_exports", "data_member",
  "return_type", "operation_preamble", "operation", "@19", "@20",
  "class_export", "interface_id", "interface_decl", "interface_def", "@21",
  "interface_list", "interface_extends", "interface_exports",
  "interface_export", "exception_list", "exception", "sequence_def",
  "dictionary_def", "enum_start", "enum_id", "enum_def", "@22", "@23",
  "enum_underlying", "enumerator_list", "enumerator",
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

#define YYPACT_NINF (-275)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-164)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -275,    17,    19,  -275,    -4,    -4,    -4,  -275,   167,    -4,
    -275,   -31,    34,    77,   -14,   113,   424,   516,   558,   600,
      30,    69,  -275,    88,   131,    -4,  -275,  -275,    16,    89,
    -275,    97,   104,  -275,    31,    14,   106,  -275,    86,   111,
    -275,   124,   128,   642,   127,  -275,   137,  -275,  -275,    -4,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,   141,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,    88,    88,   958,  -275,   -23,   146,  -275,  -275,  -275,
     164,   150,   146,   132,   158,   146,  -275,   164,   160,   146,
     173,  -275,   165,   146,   172,   186,  -275,   153,  -275,   989,
    -275,   146,   187,  -275,   190,   191,   185,  -275,  -275,   192,
     958,   958,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,   154,   180,   183,
     159,   193,   -21,  -275,  -275,  -275,   198,  -275,  -275,  -275,
     253,  -275,  -275,   199,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,   188,   200,  -275,  -275,  -275,  -275,   684,  -275,   201,
    -275,  -275,  -275,  -275,  -275,  -275,   197,   203,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,   205,   189,   322,   206,   919,
     210,   368,   173,    84,   225,   229,   226,  -275,   684,   107,
     133,   726,    88,   189,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,   238,   919,   237,   253,  -275,  -275,   -24,   119,  -275,
     989,   989,   235,   244,  -275,   768,  -275,   245,   919,   247,
    -275,   254,   840,   251,   156,  -275,   684,   255,  -275,  -275,
    -275,  -275,   958,  -275,   322,   256,  -275,  -275,  -275,   257,
    -275,   258,   259,  -275,   260,  -275,   270,  -275,   189,   253,
    -275,  -275,   368,  -275,   264,  -275,    84,  -275,   880,   989,
     989,   175,    33,  -275,   268,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,   239,  -275,   322,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,   368,  -275,   184,  -275,  -275,  -275,  -275,   267,
    -275,    88,   -47,    84,   810,  -275,  -275,  -275,  -275,  -275,
     919,  -275,   304,  -275,  -275,  -275,   313,  -275,   768,   313,
      88,   473,  -275,  -275,  -275,   919,  -275,   266,  -275,  -275,
    -275,   768,   473,  -275,  -275
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      12,     0,     9,     1,     0,     0,     0,    10,     0,   202,
     204,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   144,     9,     0,     0,    11,    13,    53,    27,
      28,    77,    32,    33,    90,    94,    17,    18,   120,    22,
      23,    37,    40,   149,   152,    41,    45,   201,     7,     0,
       5,     6,    46,    47,    49,   213,   214,   215,   216,   217,
     218,   219,   220,   221,   222,   223,   224,   225,   226,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,    85,     0,    86,   118,   119,    51,    52,    75,
      76,     9,     9,     0,   143,     0,     4,    95,    96,    54,
       0,     0,     4,     0,     0,     4,    91,     0,     0,     4,
       0,   121,     0,     4,     0,     0,   145,     0,   146,     0,
     147,     4,     0,   203,     0,     0,     0,   172,   173,     0,
       0,     0,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   194,   195,   196,
     198,   197,     0,     8,     3,    14,     0,    56,    26,    29,
       0,    31,    34,     0,    93,    16,    19,   125,   126,   127,
     128,   124,     0,    21,    24,    36,    39,   158,   151,     0,
      42,    44,    12,    12,    87,    88,     0,     0,   189,   190,
     191,   192,   199,   200,   193,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   155,     0,   154,   157,   158,     9,
       9,     0,     9,     0,   209,   210,   208,   205,   206,   207,
     212,     0,     0,     0,     0,    66,    70,     0,     0,    73,
       0,     0,   101,    82,    84,   104,    79,     0,     0,     0,
     123,     0,     0,     0,     0,   150,   158,     0,    48,    50,
     139,   140,     0,   211,     0,    60,    74,    55,    81,     0,
      65,     0,     0,    69,     0,    71,     0,    72,     0,     0,
      62,   103,     0,   117,    99,    92,     0,   108,     0,     0,
       0,     0,     0,   134,   132,   107,   122,   159,   156,   160,
     153,   148,     0,    59,     0,    63,    64,    67,    68,   102,
      80,    98,     0,   131,     0,   105,   106,   109,   111,     0,
     161,     9,     0,     0,     0,    58,    97,   110,   112,   115,
       0,   113,   162,   130,   141,   142,   171,   164,   168,   171,
       9,     0,   116,   166,   114,     0,   170,   136,   137,   138,
     165,   169,     0,   167,   135
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -275,  -275,    21,  -275,   -19,    46,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -258,     0,
    -163,  -162,  -214,  -275,  -275,  -275,  -275,  -275,  -204,  -275,
    -275,  -275,  -275,  -275,  -275,  -275,    59,  -274,  -223,    41,
    -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,  -275,   118,
    -275,  -206,  -275,   -20,  -275,  -275,  -275,  -275,  -275,  -275,
    -275,  -275,  -275,  -211,  -275,  -275,    -1,  -275,    -6,   -88,
    -275,   -93,    32,     8,  -212,  -275,   -16
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,   165,     7,     8,     2,    26,   106,   118,   119,
     122,   123,   111,   112,   114,   115,   124,   125,   131,   132,
      27,   134,   135,    28,    29,    30,   166,   109,   233,   239,
     240,   241,   242,   265,    31,    32,    33,   113,   210,   243,
      34,    35,    36,    37,   173,   116,   110,   249,   244,   291,
     292,   293,   339,   336,   284,    38,    39,    40,   182,   180,
     121,   253,   294,   346,   347,    41,    42,    43,    44,    45,
     189,   127,   130,   215,   216,   298,   321,   322,   342,   160,
     161,   245,    10,    11,   230,    46,   217
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      94,    96,    98,   100,   103,   139,   303,   257,   311,   266,
     162,   263,   331,    12,    13,   332,    48,     3,   269,    -2,
     137,   138,   167,   205,   163,   283,   107,   128,   107,   174,
     268,    49,   181,   105,   319,   270,   188,     9,   326,    49,
     206,    47,    52,   -89,   320,   300,   325,   196,   197,  -162,
    -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,  -162,
    -162,  -162,  -162,  -162,  -162,     4,   309,     5,     6,  -162,
    -162,  -162,  -162,   -57,   108,   310,   108,  -162,  -162,  -162,
     313,   133,   140,   141,    50,   251,  -162,  -162,   -89,   289,
     290,   -89,  -163,   101,   117,  -163,    49,   120,   107,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,   337,   333,   229,    -9,
      -9,    -9,    -9,    -9,   181,   289,   290,    51,    -9,    -9,
       4,   350,   102,   169,     4,   229,   172,    -9,    -9,    49,
     176,   104,  -133,  -129,   184,   -25,   108,   276,   276,   271,
     274,   209,   190,     4,   -78,     5,     6,    53,    54,   295,
     -30,   272,   -15,   137,   138,   258,   299,   -20,    14,   302,
      15,    16,    17,    18,    19,    20,    21,    22,   273,     4,
     -35,     5,     6,   136,   -38,   137,   138,   129,   232,   170,
     229,   259,   248,   -43,   252,   295,   315,   316,   297,    23,
     137,   138,   164,   262,   177,   261,   168,    24,   137,   138,
     187,   178,   179,    25,   171,   209,   175,   137,   138,   198,
     199,   183,   224,   225,   202,   203,   317,   318,   185,   281,
     226,   227,   228,   137,   138,   327,   328,   338,   219,   220,
     275,   277,   186,   191,   194,   232,   200,   192,   193,   201,
     212,   195,   351,   348,   208,   207,   211,   213,   218,   204,
     209,   221,   234,   248,   348,   222,   223,   252,   246,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,   232,   254,   255,   256,    -9,
      -9,    -9,    -9,   248,   264,   267,   278,    -9,    -9,     4,
     279,   282,   330,   324,   252,   285,    -9,    -9,   335,   296,
     286,   -83,   304,   301,   280,   320,   305,   306,   307,   308,
     312,   345,   343,   231,   323,   349,   329,   341,   352,   314,
     250,   340,   354,   344,     0,   353,   349,     0,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,     0,     0,     0,     0,    -9,    -9,
      -9,    -9,     0,     0,     0,     0,    -9,    -9,     4,   247,
       0,     0,     0,     0,     0,    -9,    -9,     0,     0,     0,
     -61,     0,     0,     0,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
       0,     0,     0,     0,    -9,    -9,    -9,    -9,     0,     0,
       0,     0,    -9,    -9,     4,     0,     0,     0,     0,     0,
       0,    -9,    -9,     0,     0,     0,  -100,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,    92,     0,
       0,     0,     0,     0,     0,    93,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,     0,     0,     0,     0,   137,   138,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
      95,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,    97,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,    99,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,     0,     0,     0,     0,   126,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,     0,     0,     0,     0,   214,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,     0,     0,     0,     0,
     260,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,     0,     0,
       0,     0,   280,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
       0,     0,     0,     0,   334,   287,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,     0,     0,     0,   288,   235,   236,   158,   159,
       0,     0,     0,     0,   137,   138,    25,     0,     0,     0,
       0,     0,     0,   237,   238,   287,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,     0,     0,     0,     0,   235,   236,   158,   159,
       0,     0,     0,     0,   137,   138,     0,     0,     0,     0,
       0,     0,     0,   237,   238,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,     0,     0,     0,     0,   235,   236,   158,   159,     0,
       0,     0,     0,   137,   138,    25,     0,     0,     0,     0,
       0,     0,   237,   238,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
       0,     0,     0,     0,     0,     0,   158,   159,     0,     0,
       0,     0,   137,   138,    25,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,   154,   155,   156,
     157,     0,     0,     0,     0,     0,     0,   158,   159,     0,
       0,     0,     0,   137,   138
};

static const yytype_int16 yycheck[] =
{
      16,    17,    18,    19,    23,    93,   264,   218,   282,   232,
     103,   223,    59,     5,     6,    62,    47,     0,    42,     0,
      44,    45,   110,    44,    47,   248,    12,    43,    12,   117,
     234,    62,   120,    25,     1,    59,   129,    41,   312,    62,
      61,     9,    56,    12,    11,   256,   304,   140,   141,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    46,   278,    48,    49,    36,
      37,    38,    39,    57,    60,   279,    60,    44,    45,    46,
     286,    49,   101,   102,    50,     1,    53,    54,    57,   252,
     252,    60,    59,    63,    35,    62,    62,    38,    12,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,   330,   323,   206,    35,
      36,    37,    38,    39,   212,   288,   288,    50,    44,    45,
      46,   345,    63,   112,    46,   223,   115,    53,    54,    62,
     119,    10,    58,    57,   123,    56,    60,   240,   241,   237,
     238,   170,   131,    46,    57,    48,    49,    44,    45,   252,
      56,    42,    56,    44,    45,    58,   254,    56,     1,   262,
       3,     4,     5,     6,     7,     8,     9,    10,    59,    46,
      56,    48,    49,    42,    56,    44,    45,    60,   207,    57,
     278,    58,   211,    56,   213,   288,   289,   290,    42,    32,
      44,    45,    56,   222,    31,   221,    56,    40,    44,    45,
      57,    38,    39,    46,    56,   234,    56,    44,    45,    65,
      66,    56,    33,    34,    65,    66,    51,    52,    56,   245,
      41,    42,    43,    44,    45,    51,    52,   330,   192,   193,
     240,   241,    56,    56,    59,   264,    66,    57,    57,    66,
      62,    59,   345,   341,     1,    57,    57,    57,    57,    66,
     279,    64,    56,   282,   352,    62,    61,   286,    58,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,   304,    61,    58,    62,    36,
      37,    38,    39,   312,    56,    58,    61,    44,    45,    46,
      56,    56,   321,    64,   323,    58,    53,    54,   324,    58,
      56,    58,    56,    58,    44,    11,    59,    59,    59,    59,
      56,   340,   338,     1,    56,   341,    59,    14,    62,   288,
     212,   332,   352,   339,    -1,   351,   352,    -1,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    -1,    -1,    -1,    -1,    36,    37,
      38,    39,    -1,    -1,    -1,    -1,    44,    45,    46,     1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    -1,    -1,    -1,
      58,    -1,    -1,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
      -1,    53,    54,    -1,    -1,    -1,    58,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    -1,    -1,    44,    -1,
      -1,    -1,    -1,    -1,    -1,    51,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    44,    45,     3,
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
      44,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    -1,    -1,
      -1,    -1,    44,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      -1,    -1,    -1,    -1,    44,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    35,    36,    37,    38,    39,
      -1,    -1,    -1,    -1,    44,    45,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    36,    37,    38,    39,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    36,    37,    38,    39,    -1,
      -1,    -1,    -1,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,    -1,    53,    54,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    -1,    -1,
      -1,    -1,    44,    45,    46,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    -1,
      -1,    -1,    -1,    44,    45
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    68,    72,     0,    46,    48,    49,    70,    71,    41,
     149,   150,   150,   150,     1,     3,     4,     5,     6,     7,
       8,     9,    10,    32,    40,    46,    73,    87,    90,    91,
      92,   101,   102,   103,   107,   108,   109,   110,   122,   123,
     124,   132,   133,   134,   135,   136,   152,   149,    47,    62,
      50,    50,    56,    44,    45,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    44,    51,   153,    44,   153,    44,   153,    44,
     153,    63,    63,    71,    10,   150,    74,    12,    60,    94,
     113,    79,    80,   104,    81,    82,   112,   113,    75,    76,
     113,   127,    77,    78,    83,    84,    44,   138,   153,    60,
     139,    85,    86,   149,    88,    89,    42,    44,    45,   146,
      71,    71,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    38,    39,
     146,   147,   148,    47,    56,    69,    93,   146,    56,    69,
      57,    56,    69,   111,   146,    56,    69,    31,    38,    39,
     126,   146,   125,    56,    69,    56,    56,    57,   148,   137,
      69,    56,    57,    57,    59,    59,   148,   148,    65,    66,
      66,    66,    65,    66,    66,    44,    61,    57,     1,    71,
     105,    57,    62,    57,    44,   140,   141,   153,    57,    72,
      72,    64,    62,    61,    33,    34,    41,    42,    43,   146,
     151,     1,    71,    95,    56,    36,    37,    53,    54,    96,
      97,    98,    99,   106,   115,   148,    58,     1,    71,   114,
     126,     1,    71,   128,    61,    58,    62,   140,    58,    58,
      44,   153,    71,   151,    56,   100,   115,    58,   105,    42,
      59,   146,    42,    59,   146,    96,   148,    96,    61,    56,
      44,   153,    56,   115,   121,    58,    56,    15,    35,    97,
      98,   116,   117,   118,   129,   148,    58,    42,   142,   146,
     140,    58,   148,    95,    56,    59,    59,    59,    59,   151,
     105,   114,    56,   128,   116,   148,   148,    51,    52,     1,
      11,   143,   144,    56,    64,    95,   114,    51,    52,    59,
      71,    59,    62,   128,    44,   153,   120,    99,   148,   119,
     143,    14,   145,   153,   145,    71,   130,   131,   146,   153,
      99,   148,    62,   153,   130
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
      95,    95,    96,    97,    97,    97,    97,    98,    98,    98,
      98,    99,    99,    99,   100,   101,   101,   102,   104,   103,
     105,   105,   105,   105,   106,   107,   107,   108,   108,   108,
     109,   111,   110,   112,   112,   113,   113,   114,   114,   114,
     114,   115,   115,   115,   115,   116,   116,   116,   116,   117,
     117,   117,   117,   119,   118,   120,   118,   121,   122,   122,
     123,   125,   124,   126,   126,   126,   126,   126,   127,   127,
     128,   128,   128,   128,   129,   130,   130,   131,   131,   132,
     132,   133,   133,   134,   134,   135,   135,   137,   136,   138,
     136,   139,   139,   140,   140,   141,   141,   141,   141,   142,
     142,   143,   143,   144,   144,   144,   144,   144,   144,   144,
     145,   145,   146,   146,   147,   147,   147,   147,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   147,   147,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   148,   148,
     148,   149,   149,   150,   150,   151,   151,   151,   151,   151,
     151,   152,   152,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
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
       0,     1,     3,     2,     1,     2,     2,     1,     1,     2,
       3,     2,     3,     0,     5,     0,     5,     1,     2,     2,
       1,     0,     6,     3,     1,     1,     1,     1,     2,     0,
       4,     3,     2,     0,     1,     3,     1,     1,     1,     6,
       6,     9,     9,     2,     1,     2,     2,     0,     6,     0,
       5,     2,     0,     3,     1,     1,     3,     1,     0,     1,
       1,     1,     0,     0,     3,     5,     4,     6,     3,     5,
       2,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     2,     1,     1,     1,     1,     1,     2,
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
#line 193 "src/Slice/Grammar.y"
{
}
#line 1985 "src/Slice/Grammar.cpp"
    break;

  case 3:
#line 201 "src/Slice/Grammar.y"
{
}
#line 1992 "src/Slice/Grammar.cpp"
    break;

  case 4:
#line 204 "src/Slice/Grammar.y"
{
}
#line 1999 "src/Slice/Grammar.cpp"
    break;

  case 5:
#line 212 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 2007 "src/Slice/Grammar.cpp"
    break;

  case 6:
#line 216 "src/Slice/Grammar.y"
{
    unit->error("file metadata must appear before any definitions");
    yyval = yyvsp[-1]; // Dummy
}
#line 2016 "src/Slice/Grammar.cpp"
    break;

  case 7:
#line 226 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 2024 "src/Slice/Grammar.cpp"
    break;

  case 8:
#line 230 "src/Slice/Grammar.y"
{
    StringListTokPtr metadata1 = StringListTokPtr::dynamicCast(yyvsp[-3]);
    StringListTokPtr metadata2 = StringListTokPtr::dynamicCast(yyvsp[-1]);
    metadata1->v.splice(metadata1->v.end(), metadata2->v);
    yyval = metadata1;
}
#line 2035 "src/Slice/Grammar.cpp"
    break;

  case 9:
#line 237 "src/Slice/Grammar.y"
{
    yyval = new StringListTok;
}
#line 2043 "src/Slice/Grammar.cpp"
    break;

  case 10:
#line 246 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[0]);
    if(!metaData->v.empty())
    {
        unit->addFileMetaData(metaData->v);
    }
}
#line 2055 "src/Slice/Grammar.cpp"
    break;

  case 11:
#line 254 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[0]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2068 "src/Slice/Grammar.cpp"
    break;

  case 12:
#line 263 "src/Slice/Grammar.y"
{
}
#line 2075 "src/Slice/Grammar.cpp"
    break;

  case 13:
#line 271 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ModulePtr::dynamicCast(yyvsp[0]));
}
#line 2083 "src/Slice/Grammar.cpp"
    break;

  case 15:
#line 276 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDeclPtr::dynamicCast(yyvsp[0]));
}
#line 2091 "src/Slice/Grammar.cpp"
    break;

  case 17:
#line 281 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after class forward declaration");
}
#line 2099 "src/Slice/Grammar.cpp"
    break;

  case 18:
#line 285 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ClassDefPtr::dynamicCast(yyvsp[0]));
}
#line 2107 "src/Slice/Grammar.cpp"
    break;

  case 20:
#line 290 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDeclPtr::dynamicCast(yyvsp[0]));
}
#line 2115 "src/Slice/Grammar.cpp"
    break;

  case 22:
#line 295 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after interface forward declaration");
}
#line 2123 "src/Slice/Grammar.cpp"
    break;

  case 23:
#line 299 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || InterfaceDefPtr::dynamicCast(yyvsp[0]));
}
#line 2131 "src/Slice/Grammar.cpp"
    break;

  case 25:
#line 304 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2139 "src/Slice/Grammar.cpp"
    break;

  case 27:
#line 309 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after exception forward declaration");
}
#line 2147 "src/Slice/Grammar.cpp"
    break;

  case 28:
#line 313 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ExceptionPtr::dynamicCast(yyvsp[0]));
}
#line 2155 "src/Slice/Grammar.cpp"
    break;

  case 30:
#line 318 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0);
}
#line 2163 "src/Slice/Grammar.cpp"
    break;

  case 32:
#line 323 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after struct forward declaration");
}
#line 2171 "src/Slice/Grammar.cpp"
    break;

  case 33:
#line 327 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || StructPtr::dynamicCast(yyvsp[0]));
}
#line 2179 "src/Slice/Grammar.cpp"
    break;

  case 35:
#line 332 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || SequencePtr::dynamicCast(yyvsp[0]));
}
#line 2187 "src/Slice/Grammar.cpp"
    break;

  case 37:
#line 337 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after sequence definition");
}
#line 2195 "src/Slice/Grammar.cpp"
    break;

  case 38:
#line 341 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || DictionaryPtr::dynamicCast(yyvsp[0]));
}
#line 2203 "src/Slice/Grammar.cpp"
    break;

  case 40:
#line 346 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after dictionary definition");
}
#line 2211 "src/Slice/Grammar.cpp"
    break;

  case 41:
#line 350 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || EnumPtr::dynamicCast(yyvsp[0]));
}
#line 2219 "src/Slice/Grammar.cpp"
    break;

  case 43:
#line 355 "src/Slice/Grammar.y"
{
    assert(yyvsp[0] == 0 || ConstPtr::dynamicCast(yyvsp[0]));
}
#line 2227 "src/Slice/Grammar.cpp"
    break;

  case 45:
#line 360 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after const definition");
}
#line 2235 "src/Slice/Grammar.cpp"
    break;

  case 46:
#line 364 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2243 "src/Slice/Grammar.cpp"
    break;

  case 47:
#line 373 "src/Slice/Grammar.y"
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
#line 2273 "src/Slice/Grammar.cpp"
    break;

  case 48:
#line 399 "src/Slice/Grammar.y"
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
#line 2289 "src/Slice/Grammar.cpp"
    break;

  case 49:
#line 411 "src/Slice/Grammar.y"
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
#line 2348 "src/Slice/Grammar.cpp"
    break;

  case 50:
#line 466 "src/Slice/Grammar.y"
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
#line 2370 "src/Slice/Grammar.cpp"
    break;

  case 51:
#line 489 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2378 "src/Slice/Grammar.cpp"
    break;

  case 52:
#line 493 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = yyvsp[0]; // Dummy
}
#line 2388 "src/Slice/Grammar.cpp"
    break;

  case 53:
#line 504 "src/Slice/Grammar.y"
{
    unit->error("exceptions cannot be forward declared");
    yyval = 0;
}
#line 2397 "src/Slice/Grammar.cpp"
    break;

  case 54:
#line 514 "src/Slice/Grammar.y"
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
#line 2414 "src/Slice/Grammar.cpp"
    break;

  case 55:
#line 527 "src/Slice/Grammar.y"
{
    if(yyvsp[-3])
    {
        unit->popContainer();
    }
    yyval = yyvsp[-3];
}
#line 2426 "src/Slice/Grammar.cpp"
    break;

  case 56:
#line 540 "src/Slice/Grammar.y"
{
    StringTokPtr scoped = StringTokPtr::dynamicCast(yyvsp[0]);
    ContainerPtr cont = unit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v);
    cont->checkIntroduced(scoped->v);
    yyval = contained;
}
#line 2438 "src/Slice/Grammar.cpp"
    break;

  case 57:
#line 548 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 2446 "src/Slice/Grammar.cpp"
    break;

  case 58:
#line 557 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 2459 "src/Slice/Grammar.cpp"
    break;

  case 59:
#line 566 "src/Slice/Grammar.y"
{
}
#line 2466 "src/Slice/Grammar.cpp"
    break;

  case 60:
#line 569 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2474 "src/Slice/Grammar.cpp"
    break;

  case 61:
#line 573 "src/Slice/Grammar.y"
{
}
#line 2481 "src/Slice/Grammar.cpp"
    break;

  case 62:
#line 581 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    checkIdentifier(ident->v);
    TypeStringTokPtr typestring = new TypeStringTok;
    typestring->v = make_pair(type, ident->v);
    yyval = typestring;
}
#line 2494 "src/Slice/Grammar.cpp"
    break;

  case 63:
#line 595 "src/Slice/Grammar.y"
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
#line 2516 "src/Slice/Grammar.cpp"
    break;

  case 64:
#line 613 "src/Slice/Grammar.y"
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
#line 2594 "src/Slice/Grammar.cpp"
    break;

  case 65:
#line 687 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2604 "src/Slice/Grammar.cpp"
    break;

  case 66:
#line 693 "src/Slice/Grammar.y"
{
    unit->error("missing tag");
    TaggedDefTokPtr m = new TaggedDefTok(-1); // Dummy
    yyval = m;
}
#line 2614 "src/Slice/Grammar.cpp"
    break;

  case 67:
#line 704 "src/Slice/Grammar.y"
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
#line 2640 "src/Slice/Grammar.cpp"
    break;

  case 68:
#line 726 "src/Slice/Grammar.y"
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
#line 2722 "src/Slice/Grammar.cpp"
    break;

  case 69:
#line 804 "src/Slice/Grammar.y"
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
#line 814 "src/Slice/Grammar.y"
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
#line 829 "src/Slice/Grammar.y"
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
#line 844 "src/Slice/Grammar.y"
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
#line 865 "src/Slice/Grammar.y"
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
#line 884 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2814 "src/Slice/Grammar.cpp"
    break;

  case 76:
#line 888 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as struct name");
    yyval = yyvsp[0]; // Dummy
}
#line 2824 "src/Slice/Grammar.cpp"
    break;

  case 77:
#line 899 "src/Slice/Grammar.y"
{
    unit->error("structs cannot be forward declared");
    yyval = 0; // Dummy
}
#line 2833 "src/Slice/Grammar.cpp"
    break;

  case 78:
#line 909 "src/Slice/Grammar.y"
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
#line 2855 "src/Slice/Grammar.cpp"
    break;

  case 79:
#line 927 "src/Slice/Grammar.y"
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
#line 950 "src/Slice/Grammar.y"
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
#line 959 "src/Slice/Grammar.y"
{
}
#line 2897 "src/Slice/Grammar.cpp"
    break;

  case 82:
#line 962 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 2905 "src/Slice/Grammar.cpp"
    break;

  case 83:
#line 966 "src/Slice/Grammar.y"
{
}
#line 2912 "src/Slice/Grammar.cpp"
    break;

  case 85:
#line 980 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2920 "src/Slice/Grammar.cpp"
    break;

  case 86:
#line 984 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2930 "src/Slice/Grammar.cpp"
    break;

  case 87:
#line 995 "src/Slice/Grammar.y"
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
#line 2959 "src/Slice/Grammar.cpp"
    break;

  case 88:
#line 1020 "src/Slice/Grammar.y"
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
#line 3053 "src/Slice/Grammar.cpp"
    break;

  case 89:
#line 1110 "src/Slice/Grammar.y"
{
    ClassIdTokPtr classId = new ClassIdTok();
    classId->v = StringTokPtr::dynamicCast(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 3064 "src/Slice/Grammar.cpp"
    break;

  case 90:
#line 1122 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 3075 "src/Slice/Grammar.cpp"
    break;

  case 91:
#line 1134 "src/Slice/Grammar.y"
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
#line 3096 "src/Slice/Grammar.cpp"
    break;

  case 92:
#line 1151 "src/Slice/Grammar.y"
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
#line 1168 "src/Slice/Grammar.y"
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
#line 1202 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 3158 "src/Slice/Grammar.cpp"
    break;

  case 95:
#line 1211 "src/Slice/Grammar.y"
{
}
#line 3165 "src/Slice/Grammar.cpp"
    break;

  case 96:
#line 1214 "src/Slice/Grammar.y"
{
}
#line 3172 "src/Slice/Grammar.cpp"
    break;

  case 97:
#line 1222 "src/Slice/Grammar.y"
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
#line 1231 "src/Slice/Grammar.y"
{
}
#line 3192 "src/Slice/Grammar.cpp"
    break;

  case 99:
#line 1234 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3200 "src/Slice/Grammar.cpp"
    break;

  case 100:
#line 1238 "src/Slice/Grammar.y"
{
}
#line 3207 "src/Slice/Grammar.cpp"
    break;

  case 101:
#line 1246 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr def = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    DataMemberPtr dm;
    if(cl)
    {
        dm = cl->createDataMember(def->name, def->type, def->isTagged, def->tag);
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        dm = st->createDataMember(def->name, def->type, def->isTagged);
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        dm = ex->createDataMember(def->name, def->type, def->isTagged, def->tag);
    }
    unit->currentContainer()->checkIntroduced(def->name, dm);
    yyval = dm;
}
#line 3233 "src/Slice/Grammar.cpp"
    break;

  case 102:
#line 1268 "src/Slice/Grammar.y"
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
        dm = st->createDataMember(def->name, def->type, def->isTagged, value->v,
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
#line 1295 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    string name = StringTokPtr::dynamicCast(yyvsp[0])->v;
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        yyval = cl->createDataMember(name, type, false, 0); // Dummy
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        yyval = st->createDataMember(name, type, false); // Dummy
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        yyval = ex->createDataMember(name, type, false, 0); // Dummy
    }
    assert(yyval);
    unit->error("keyword `" + name + "' cannot be used as data member name");
}
#line 3290 "src/Slice/Grammar.cpp"
    break;

  case 104:
#line 1317 "src/Slice/Grammar.y"
{
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    ClassDefPtr cl = ClassDefPtr::dynamicCast(unit->currentContainer());
    if(cl)
    {
        yyval = cl->createDataMember(IceUtil::generateUUID(), type, false, 0); // Dummy
    }
    StructPtr st = StructPtr::dynamicCast(unit->currentContainer());
    if(st)
    {
        yyval = st->createDataMember(IceUtil::generateUUID(), type, false); // Dummy
    }
    ExceptionPtr ex = ExceptionPtr::dynamicCast(unit->currentContainer());
    if(ex)
    {
        yyval = ex->createDataMember(IceUtil::generateUUID(), type, false, 0); // Dummy
    }
    assert(yyval);
    unit->error("missing data member name");
}
#line 3315 "src/Slice/Grammar.cpp"
    break;

  case 105:
#line 1343 "src/Slice/Grammar.y"
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
#line 3332 "src/Slice/Grammar.cpp"
    break;

  case 106:
#line 1356 "src/Slice/Grammar.y"
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
#line 3353 "src/Slice/Grammar.cpp"
    break;

  case 107:
#line 1373 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    m->type = TypePtr::dynamicCast(yyvsp[0]);
    yyval = m;
}
#line 3363 "src/Slice/Grammar.cpp"
    break;

  case 108:
#line 1379 "src/Slice/Grammar.y"
{
    TaggedDefTokPtr m = new TaggedDefTok(-1);
    yyval = m;
}
#line 3372 "src/Slice/Grammar.cpp"
    break;

  case 109:
#line 1389 "src/Slice/Grammar.y"
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
#line 3400 "src/Slice/Grammar.cpp"
    break;

  case 110:
#line 1413 "src/Slice/Grammar.y"
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
#line 3429 "src/Slice/Grammar.cpp"
    break;

  case 111:
#line 1438 "src/Slice/Grammar.y"
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
#line 3457 "src/Slice/Grammar.cpp"
    break;

  case 112:
#line 1462 "src/Slice/Grammar.y"
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
            yyval = 0;
        }
    }
    else
    {
        yyval = 0;
    }
}
#line 3486 "src/Slice/Grammar.cpp"
    break;

  case 113:
#line 1492 "src/Slice/Grammar.y"
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
#line 3502 "src/Slice/Grammar.cpp"
    break;

  case 114:
#line 1504 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v);
    }
}
#line 3516 "src/Slice/Grammar.cpp"
    break;

  case 115:
#line 1514 "src/Slice/Grammar.y"
{
    if(yyvsp[-2])
    {
        unit->popContainer();
    }
    yyerrok;
}
#line 3528 "src/Slice/Grammar.cpp"
    break;

  case 116:
#line 1522 "src/Slice/Grammar.y"
{
    OperationPtr op = OperationPtr::dynamicCast(yyvsp[-1]);
    ExceptionListTokPtr el = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    assert(el);
    if(op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 3542 "src/Slice/Grammar.cpp"
    break;

  case 118:
#line 1543 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3550 "src/Slice/Grammar.cpp"
    break;

  case 119:
#line 1547 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 3560 "src/Slice/Grammar.cpp"
    break;

  case 120:
#line 1558 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkIntroduced(ident->v, cl);
    yyval = cl;
}
#line 3572 "src/Slice/Grammar.cpp"
    break;

  case 121:
#line 1571 "src/Slice/Grammar.y"
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
#line 3593 "src/Slice/Grammar.cpp"
    break;

  case 122:
#line 1588 "src/Slice/Grammar.y"
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
#line 3609 "src/Slice/Grammar.cpp"
    break;

  case 123:
#line 1605 "src/Slice/Grammar.y"
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
#line 3648 "src/Slice/Grammar.cpp"
    break;

  case 124:
#line 1640 "src/Slice/Grammar.y"
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
#line 3687 "src/Slice/Grammar.cpp"
    break;

  case 125:
#line 1675 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type Object");
    yyval = new InterfaceListTok; // Dummy
}
#line 3696 "src/Slice/Grammar.cpp"
    break;

  case 126:
#line 1680 "src/Slice/Grammar.y"
{
    unit->error("illegal inheritance from type AnyClass");
    yyval = new ClassListTok; // Dummy
}
#line 3705 "src/Slice/Grammar.cpp"
    break;

  case 127:
#line 1685 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    unit->error("illegal inheritance from type Value");
    yyval = new ClassListTok; // Dummy
}
#line 3718 "src/Slice/Grammar.cpp"
    break;

  case 128:
#line 1699 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3726 "src/Slice/Grammar.cpp"
    break;

  case 129:
#line 1703 "src/Slice/Grammar.y"
{
    yyval = new InterfaceListTok;
}
#line 3734 "src/Slice/Grammar.cpp"
    break;

  case 130:
#line 1712 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    ContainedPtr contained = ContainedPtr::dynamicCast(yyvsp[-2]);
    if(contained && !metaData->v.empty())
    {
        contained->setMetaData(metaData->v);
    }
}
#line 3747 "src/Slice/Grammar.cpp"
    break;

  case 131:
#line 1721 "src/Slice/Grammar.y"
{
}
#line 3754 "src/Slice/Grammar.cpp"
    break;

  case 132:
#line 1724 "src/Slice/Grammar.y"
{
    unit->error("`;' missing after definition");
}
#line 3762 "src/Slice/Grammar.cpp"
    break;

  case 133:
#line 1728 "src/Slice/Grammar.y"
{
}
#line 3769 "src/Slice/Grammar.cpp"
    break;

  case 135:
#line 1742 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[-2]);
    ExceptionListTokPtr exceptionList = ExceptionListTokPtr::dynamicCast(yyvsp[0]);
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3780 "src/Slice/Grammar.cpp"
    break;

  case 136:
#line 1749 "src/Slice/Grammar.y"
{
    ExceptionPtr exception = ExceptionPtr::dynamicCast(yyvsp[0]);
    ExceptionListTokPtr exceptionList = new ExceptionListTok;
    exceptionList->v.push_front(exception);
    yyval = exceptionList;
}
#line 3791 "src/Slice/Grammar.cpp"
    break;

  case 137:
#line 1761 "src/Slice/Grammar.y"
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
#line 3807 "src/Slice/Grammar.cpp"
    break;

  case 138:
#line 1773 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as exception name");
    yyval = unit->currentModule()->createException(IceUtil::generateUUID(), 0, Dummy); // Dummy
}
#line 3817 "src/Slice/Grammar.cpp"
    break;

  case 139:
#line 1784 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metaData->v);
}
#line 3829 "src/Slice/Grammar.cpp"
    break;

  case 140:
#line 1792 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
    unit->error("keyword `" + ident->v + "' cannot be used as sequence name");
}
#line 3842 "src/Slice/Grammar.cpp"
    break;

  case 141:
#line 1806 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr keyMetaData = StringListTokPtr::dynamicCast(yyvsp[-6]);
    TypePtr keyType = TypePtr::dynamicCast(yyvsp[-5]);
    StringListTokPtr valueMetaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr valueType = TypePtr::dynamicCast(yyvsp[-2]);
    ModulePtr cont = unit->currentModule();
    yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
}
#line 3856 "src/Slice/Grammar.cpp"
    break;

  case 142:
#line 1816 "src/Slice/Grammar.y"
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
#line 3871 "src/Slice/Grammar.cpp"
    break;

  case 143:
#line 1832 "src/Slice/Grammar.y"
{
    BoolTokPtr unchecked = new BoolTok;
    unchecked->v = true;
    yyval = unchecked;
}
#line 3881 "src/Slice/Grammar.cpp"
    break;

  case 144:
#line 1838 "src/Slice/Grammar.y"
{
    BoolTokPtr unchecked = new BoolTok;
    unchecked->v = false;
    yyval = unchecked;
}
#line 3891 "src/Slice/Grammar.cpp"
    break;

  case 145:
#line 1849 "src/Slice/Grammar.y"
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
#line 3911 "src/Slice/Grammar.cpp"
    break;

  case 146:
#line 1865 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[-1])->v;
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    ModulePtr cont = unit->currentModule();
    unit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
    yyval = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
}
#line 3923 "src/Slice/Grammar.cpp"
    break;

  case 147:
#line 1878 "src/Slice/Grammar.y"
{
    EnumPtr en = EnumPtr::dynamicCast(yyvsp[-1]);
    en->initUnderlying(TypePtr::dynamicCast(yyvsp[0]));
    unit->pushContainer(en);
    yyval = en;
}
#line 3934 "src/Slice/Grammar.cpp"
    break;

  case 148:
#line 1885 "src/Slice/Grammar.y"
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
#line 3952 "src/Slice/Grammar.cpp"
    break;

  case 149:
#line 1900 "src/Slice/Grammar.y"
{
    bool unchecked = BoolTokPtr::dynamicCast(yyvsp[0])->v;
    unit->error("missing enumeration name");
    ModulePtr cont = unit->currentModule();
    EnumPtr en = cont->createEnum(IceUtil::generateUUID(), unchecked, Dummy);
    unit->pushContainer(en);
    yyval = en;
}
#line 3965 "src/Slice/Grammar.cpp"
    break;

  case 150:
#line 1909 "src/Slice/Grammar.y"
{
    unit->popContainer();
    yyval = yyvsp[-4];
}
#line 3974 "src/Slice/Grammar.cpp"
    break;

  case 151:
#line 1919 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3982 "src/Slice/Grammar.cpp"
    break;

  case 152:
#line 1923 "src/Slice/Grammar.y"
{
    yyval = 0;
}
#line 3990 "src/Slice/Grammar.cpp"
    break;

  case 153:
#line 1932 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = EnumeratorListTokPtr::dynamicCast(yyvsp[-2]);
    ens->v.splice(ens->v.end(), EnumeratorListTokPtr::dynamicCast(yyvsp[0])->v);
    yyval = ens;
}
#line 4000 "src/Slice/Grammar.cpp"
    break;

  case 154:
#line 1938 "src/Slice/Grammar.y"
{
}
#line 4007 "src/Slice/Grammar.cpp"
    break;

  case 155:
#line 1946 "src/Slice/Grammar.y"
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
#line 4023 "src/Slice/Grammar.cpp"
    break;

  case 156:
#line 1958 "src/Slice/Grammar.y"
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
#line 4040 "src/Slice/Grammar.cpp"
    break;

  case 157:
#line 1971 "src/Slice/Grammar.y"
{
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    unit->error("keyword `" + ident->v + "' cannot be used as enumerator");
    EnumeratorListTokPtr ens = new EnumeratorListTok; // Dummy
    yyval = ens;
}
#line 4051 "src/Slice/Grammar.cpp"
    break;

  case 158:
#line 1978 "src/Slice/Grammar.y"
{
    EnumeratorListTokPtr ens = new EnumeratorListTok;
    yyval = ens; // Dummy
}
#line 4060 "src/Slice/Grammar.cpp"
    break;

  case 159:
#line 1988 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4068 "src/Slice/Grammar.cpp"
    break;

  case 160:
#line 1992 "src/Slice/Grammar.y"
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
#line 4105 "src/Slice/Grammar.cpp"
    break;

  case 161:
#line 2030 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = true;
    yyval = out;
}
#line 4115 "src/Slice/Grammar.cpp"
    break;

  case 162:
#line 2036 "src/Slice/Grammar.y"
{
    BoolTokPtr out = new BoolTok;
    out->v = false;
    yyval = out;
}
#line 4125 "src/Slice/Grammar.cpp"
    break;

  case 163:
#line 2047 "src/Slice/Grammar.y"
{
}
#line 4132 "src/Slice/Grammar.cpp"
    break;

  case 164:
#line 2050 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    TaggedDefTokPtr tsp = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        DataMemberPtr param = op->createParameter(tsp->name, tsp->type, isOutParam->v, tsp->isTagged, tsp->tag);
        unit->currentContainer()->checkIntroduced(tsp->name, param);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
        if(!metaData->v.empty())
        {
            param->setMetaData(metaData->v);
        }
    }
}
#line 4152 "src/Slice/Grammar.cpp"
    break;

  case 165:
#line 2066 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    TaggedDefTokPtr tsp = TaggedDefTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        DataMemberPtr param = op->createParameter(tsp->name, tsp->type, isOutParam->v, tsp->isTagged, tsp->tag);
        unit->currentContainer()->checkIntroduced(tsp->name, param);
        StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-1]);
        if(!metaData->v.empty())
        {
            param->setMetaData(metaData->v);
        }
    }
}
#line 4172 "src/Slice/Grammar.cpp"
    break;

  case 166:
#line 2082 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParameter(ident->v, type, isOutParam->v, false, 0); // Dummy
        unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
#line 4188 "src/Slice/Grammar.cpp"
    break;

  case 167:
#line 2094 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[-1]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParameter(ident->v, type, isOutParam->v, false, 0); // Dummy
        unit->error("keyword `" + ident->v + "' cannot be used as parameter name");
    }
}
#line 4204 "src/Slice/Grammar.cpp"
    break;

  case 168:
#line 2106 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParameter(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        unit->error("missing parameter name");
    }
}
#line 4219 "src/Slice/Grammar.cpp"
    break;

  case 169:
#line 2117 "src/Slice/Grammar.y"
{
    BoolTokPtr isOutParam = BoolTokPtr::dynamicCast(yyvsp[-2]);
    TypePtr type = TypePtr::dynamicCast(yyvsp[0]);
    OperationPtr op = OperationPtr::dynamicCast(unit->currentContainer());
    if(op)
    {
        op->createParameter(IceUtil::generateUUID(), type, isOutParam->v, false, 0); // Dummy
        unit->error("missing parameter name");
    }
}
#line 4234 "src/Slice/Grammar.cpp"
    break;

  case 170:
#line 2133 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 4242 "src/Slice/Grammar.cpp"
    break;

  case 171:
#line 2137 "src/Slice/Grammar.y"
{
    yyval = new ExceptionListTok;
}
#line 4250 "src/Slice/Grammar.cpp"
    break;

  case 172:
#line 2146 "src/Slice/Grammar.y"
{
}
#line 4257 "src/Slice/Grammar.cpp"
    break;

  case 173:
#line 2149 "src/Slice/Grammar.y"
{
}
#line 4264 "src/Slice/Grammar.cpp"
    break;

  case 174:
#line 2156 "src/Slice/Grammar.y"
           {}
#line 4270 "src/Slice/Grammar.cpp"
    break;

  case 175:
#line 2157 "src/Slice/Grammar.y"
           {}
#line 4276 "src/Slice/Grammar.cpp"
    break;

  case 176:
#line 2158 "src/Slice/Grammar.y"
            {}
#line 4282 "src/Slice/Grammar.cpp"
    break;

  case 177:
#line 2159 "src/Slice/Grammar.y"
             {}
#line 4288 "src/Slice/Grammar.cpp"
    break;

  case 178:
#line 2160 "src/Slice/Grammar.y"
          {}
#line 4294 "src/Slice/Grammar.cpp"
    break;

  case 179:
#line 2161 "src/Slice/Grammar.y"
           {}
#line 4300 "src/Slice/Grammar.cpp"
    break;

  case 180:
#line 2162 "src/Slice/Grammar.y"
             {}
#line 4306 "src/Slice/Grammar.cpp"
    break;

  case 181:
#line 2163 "src/Slice/Grammar.y"
              {}
#line 4312 "src/Slice/Grammar.cpp"
    break;

  case 182:
#line 2164 "src/Slice/Grammar.y"
           {}
#line 4318 "src/Slice/Grammar.cpp"
    break;

  case 183:
#line 2165 "src/Slice/Grammar.y"
            {}
#line 4324 "src/Slice/Grammar.cpp"
    break;

  case 184:
#line 2166 "src/Slice/Grammar.y"
              {}
#line 4330 "src/Slice/Grammar.cpp"
    break;

  case 185:
#line 2167 "src/Slice/Grammar.y"
               {}
#line 4336 "src/Slice/Grammar.cpp"
    break;

  case 186:
#line 2168 "src/Slice/Grammar.y"
            {}
#line 4342 "src/Slice/Grammar.cpp"
    break;

  case 187:
#line 2169 "src/Slice/Grammar.y"
             {}
#line 4348 "src/Slice/Grammar.cpp"
    break;

  case 188:
#line 2170 "src/Slice/Grammar.y"
             {}
#line 4354 "src/Slice/Grammar.cpp"
    break;

  case 189:
#line 2176 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 4362 "src/Slice/Grammar.cpp"
    break;

  case 190:
#line 2180 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindObject);
}
#line 4370 "src/Slice/Grammar.cpp"
    break;

  case 191:
#line 2184 "src/Slice/Grammar.y"
{
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 4378 "src/Slice/Grammar.cpp"
    break;

  case 192:
#line 2188 "src/Slice/Grammar.y"
{
    if (!unit->compatMode())
    {
        unit->warning(Deprecated, "the `Value' keyword is deprecated, use `AnyClass' instead");
    }
    yyval = unit->optionalBuiltin(Builtin::KindAnyClass);
}
#line 4390 "src/Slice/Grammar.cpp"
    break;

  case 193:
#line 2196 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[-1]);
    yyval = unit->optionalBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 4399 "src/Slice/Grammar.cpp"
    break;

  case 194:
#line 2201 "src/Slice/Grammar.y"
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
#line 4414 "src/Slice/Grammar.cpp"
    break;

  case 195:
#line 2212 "src/Slice/Grammar.y"
{
    yyval = unit->builtin(Builtin::KindAnyClass);
}
#line 4422 "src/Slice/Grammar.cpp"
    break;

  case 196:
#line 2216 "src/Slice/Grammar.y"
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
#line 4438 "src/Slice/Grammar.cpp"
    break;

  case 197:
#line 2228 "src/Slice/Grammar.y"
{
    StringTokPtr typeName = StringTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->builtin(Builtin::kindFromString(typeName->v).value());
}
#line 4447 "src/Slice/Grammar.cpp"
    break;

  case 198:
#line 2233 "src/Slice/Grammar.y"
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
#line 4477 "src/Slice/Grammar.cpp"
    break;

  case 199:
#line 2259 "src/Slice/Grammar.y"
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
#line 4513 "src/Slice/Grammar.cpp"
    break;

  case 200:
#line 2291 "src/Slice/Grammar.y"
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
#line 4540 "src/Slice/Grammar.cpp"
    break;

  case 201:
#line 2319 "src/Slice/Grammar.y"
{
    StringTokPtr str1 = StringTokPtr::dynamicCast(yyvsp[-1]);
    StringTokPtr str2 = StringTokPtr::dynamicCast(yyvsp[0]);
    str1->v += str2->v;
}
#line 4550 "src/Slice/Grammar.cpp"
    break;

  case 202:
#line 2325 "src/Slice/Grammar.y"
{
}
#line 4557 "src/Slice/Grammar.cpp"
    break;

  case 203:
#line 2333 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = StringListTokPtr::dynamicCast(yyvsp[-2]);
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4568 "src/Slice/Grammar.cpp"
    break;

  case 204:
#line 2340 "src/Slice/Grammar.y"
{
    StringTokPtr str = StringTokPtr::dynamicCast(yyvsp[0]);
    StringListTokPtr stringList = new StringListTok;
    stringList->v.push_back(str->v);
    yyval = stringList;
}
#line 4579 "src/Slice/Grammar.cpp"
    break;

  case 205:
#line 2352 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindLong);
    IntegerTokPtr intVal = IntegerTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), intVal->literal);
    yyval = def;
}
#line 4592 "src/Slice/Grammar.cpp"
    break;

  case 206:
#line 2361 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindDouble);
    FloatingTokPtr floatVal = FloatingTokPtr::dynamicCast(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    ConstDefTokPtr def = new ConstDefTok(type, sstr.str(), floatVal->literal);
    yyval = def;
}
#line 4605 "src/Slice/Grammar.cpp"
    break;

  case 207:
#line 2370 "src/Slice/Grammar.y"
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
#line 4649 "src/Slice/Grammar.cpp"
    break;

  case 208:
#line 2410 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindString);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, literal->v, literal->literal);
    yyval = def;
}
#line 4660 "src/Slice/Grammar.cpp"
    break;

  case 209:
#line 2417 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "false", "false");
    yyval = def;
}
#line 4671 "src/Slice/Grammar.cpp"
    break;

  case 210:
#line 2424 "src/Slice/Grammar.y"
{
    BuiltinPtr type = unit->builtin(Builtin::KindBool);
    StringTokPtr literal = StringTokPtr::dynamicCast(yyvsp[0]);
    ConstDefTokPtr def = new ConstDefTok(type, "true", "true");
    yyval = def;
}
#line 4682 "src/Slice/Grammar.cpp"
    break;

  case 211:
#line 2436 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-4]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-3]);
    StringTokPtr ident = StringTokPtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    yyval = unit->currentModule()->createConst(ident->v, const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral);
}
#line 4695 "src/Slice/Grammar.cpp"
    break;

  case 212:
#line 2445 "src/Slice/Grammar.y"
{
    StringListTokPtr metaData = StringListTokPtr::dynamicCast(yyvsp[-3]);
    TypePtr const_type = TypePtr::dynamicCast(yyvsp[-2]);
    ConstDefTokPtr value = ConstDefTokPtr::dynamicCast(yyvsp[0]);
    unit->error("missing constant name");
    yyval = unit->currentModule()->createConst(IceUtil::generateUUID(), const_type, metaData->v, value->v,
                                               value->valueAsString, value->valueAsLiteral, Dummy); // Dummy
}
#line 4708 "src/Slice/Grammar.cpp"
    break;

  case 213:
#line 2458 "src/Slice/Grammar.y"
             {}
#line 4714 "src/Slice/Grammar.cpp"
    break;

  case 214:
#line 2459 "src/Slice/Grammar.y"
            {}
#line 4720 "src/Slice/Grammar.cpp"
    break;

  case 215:
#line 2460 "src/Slice/Grammar.y"
                {}
#line 4726 "src/Slice/Grammar.cpp"
    break;

  case 216:
#line 2461 "src/Slice/Grammar.y"
                {}
#line 4732 "src/Slice/Grammar.cpp"
    break;

  case 217:
#line 2462 "src/Slice/Grammar.y"
             {}
#line 4738 "src/Slice/Grammar.cpp"
    break;

  case 218:
#line 2463 "src/Slice/Grammar.y"
               {}
#line 4744 "src/Slice/Grammar.cpp"
    break;

  case 219:
#line 2464 "src/Slice/Grammar.y"
                 {}
#line 4750 "src/Slice/Grammar.cpp"
    break;

  case 220:
#line 2465 "src/Slice/Grammar.y"
           {}
#line 4756 "src/Slice/Grammar.cpp"
    break;

  case 221:
#line 2466 "src/Slice/Grammar.y"
          {}
#line 4762 "src/Slice/Grammar.cpp"
    break;

  case 222:
#line 2467 "src/Slice/Grammar.y"
              {}
#line 4768 "src/Slice/Grammar.cpp"
    break;

  case 223:
#line 2468 "src/Slice/Grammar.y"
                 {}
#line 4774 "src/Slice/Grammar.cpp"
    break;

  case 224:
#line 2469 "src/Slice/Grammar.y"
             {}
#line 4780 "src/Slice/Grammar.cpp"
    break;

  case 225:
#line 2470 "src/Slice/Grammar.y"
           {}
#line 4786 "src/Slice/Grammar.cpp"
    break;

  case 226:
#line 2471 "src/Slice/Grammar.y"
           {}
#line 4792 "src/Slice/Grammar.cpp"
    break;

  case 227:
#line 2472 "src/Slice/Grammar.y"
           {}
#line 4798 "src/Slice/Grammar.cpp"
    break;

  case 228:
#line 2473 "src/Slice/Grammar.y"
            {}
#line 4804 "src/Slice/Grammar.cpp"
    break;

  case 229:
#line 2474 "src/Slice/Grammar.y"
             {}
#line 4810 "src/Slice/Grammar.cpp"
    break;

  case 230:
#line 2475 "src/Slice/Grammar.y"
          {}
#line 4816 "src/Slice/Grammar.cpp"
    break;

  case 231:
#line 2476 "src/Slice/Grammar.y"
           {}
#line 4822 "src/Slice/Grammar.cpp"
    break;

  case 232:
#line 2477 "src/Slice/Grammar.y"
             {}
#line 4828 "src/Slice/Grammar.cpp"
    break;

  case 233:
#line 2478 "src/Slice/Grammar.y"
              {}
#line 4834 "src/Slice/Grammar.cpp"
    break;

  case 234:
#line 2479 "src/Slice/Grammar.y"
           {}
#line 4840 "src/Slice/Grammar.cpp"
    break;

  case 235:
#line 2480 "src/Slice/Grammar.y"
            {}
#line 4846 "src/Slice/Grammar.cpp"
    break;

  case 236:
#line 2481 "src/Slice/Grammar.y"
              {}
#line 4852 "src/Slice/Grammar.cpp"
    break;

  case 237:
#line 2482 "src/Slice/Grammar.y"
               {}
#line 4858 "src/Slice/Grammar.cpp"
    break;

  case 238:
#line 2483 "src/Slice/Grammar.y"
            {}
#line 4864 "src/Slice/Grammar.cpp"
    break;

  case 239:
#line 2484 "src/Slice/Grammar.y"
             {}
#line 4870 "src/Slice/Grammar.cpp"
    break;

  case 240:
#line 2485 "src/Slice/Grammar.y"
             {}
#line 4876 "src/Slice/Grammar.cpp"
    break;

  case 241:
#line 2486 "src/Slice/Grammar.y"
             {}
#line 4882 "src/Slice/Grammar.cpp"
    break;

  case 242:
#line 2487 "src/Slice/Grammar.y"
            {}
#line 4888 "src/Slice/Grammar.cpp"
    break;

  case 243:
#line 2488 "src/Slice/Grammar.y"
            {}
#line 4894 "src/Slice/Grammar.cpp"
    break;

  case 244:
#line 2489 "src/Slice/Grammar.y"
           {}
#line 4900 "src/Slice/Grammar.cpp"
    break;

  case 245:
#line 2490 "src/Slice/Grammar.y"
                 {}
#line 4906 "src/Slice/Grammar.cpp"
    break;

  case 246:
#line 2491 "src/Slice/Grammar.y"
          {}
#line 4912 "src/Slice/Grammar.cpp"
    break;

  case 247:
#line 2492 "src/Slice/Grammar.y"
               {}
#line 4918 "src/Slice/Grammar.cpp"
    break;

  case 248:
#line 2493 "src/Slice/Grammar.y"
               {}
#line 4924 "src/Slice/Grammar.cpp"
    break;

  case 249:
#line 2494 "src/Slice/Grammar.y"
            {}
#line 4930 "src/Slice/Grammar.cpp"
    break;


#line 4934 "src/Slice/Grammar.cpp"

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

#line 2497 "src/Slice/Grammar.y"

