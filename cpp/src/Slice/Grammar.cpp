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

#line 27 "src/Slice/Grammar.y"

// Defines the rule bison uses to reduce token locations. Bison asks that the macro should
// be one-line, and treatable as a single statement when followed by a semi-colon.
// `N` is the number of tokens that are being combined, and (Cur) is their combined location.
#define YYLLOC_DEFAULT(Cur, Rhs, N)                                                                                    \
    do                                                                                                                 \
        if (N == 1)                                                                                                    \
        {                                                                                                              \
            (Cur) = (YYRHSLOC((Rhs), 1));                                                                              \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            if (N)                                                                                                     \
            {                                                                                                          \
                (Cur).firstLine = (YYRHSLOC((Rhs), 1)).firstLine;                                                      \
                (Cur).firstColumn = (YYRHSLOC((Rhs), 1)).firstColumn;                                                  \
            }                                                                                                          \
            else                                                                                                       \
            {                                                                                                          \
                (Cur).firstLine = (YYRHSLOC((Rhs), 0)).lastLine;                                                       \
                (Cur).firstColumn = (YYRHSLOC((Rhs), 0)).lastColumn;                                                   \
            }                                                                                                          \
            (Cur).filename = (YYRHSLOC((Rhs), N)).filename;                                                            \
            (Cur).lastLine = (YYRHSLOC((Rhs), N)).lastLine;                                                            \
            (Cur).lastColumn = (YYRHSLOC((Rhs), N)).lastColumn;                                                        \
        }                                                                                                              \
    while (0)

#line 106 "src/Slice/Grammar.cpp"

/* Substitute the variable and function names.  */
#define yyparse slice_parse
#define yylex slice_lex
#define yyerror slice_error
#define yydebug slice_debug
#define yynerrs slice_nerrs

/* First part of user prologue.  */
#line 66 "src/Slice/Grammar.y"

#include "Ice/UUID.h"
#include "Parser.h"

#include <cstring>
#include <limits>

#ifdef _MSC_VER
// warning C4102: 'yyoverflowlab' : unreferenced label
#    pragma warning(disable : 4102)
// warning C4127: conditional expression is constant
#    pragma warning(disable : 4127)
// warning C4702: unreachable code
#    pragma warning(disable : 4702)
#endif

// Avoid old style cast warnings in generated grammar
#ifdef __GNUC__
#    pragma GCC diagnostic ignored "-Wold-style-cast"
#    pragma GCC diagnostic ignored "-Wunused-label"

// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98753
#    pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif

// Avoid clang warnings in generated grammar
#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wconversion"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#    pragma clang diagnostic ignored "-Wunused-but-set-variable"
#    pragma clang diagnostic ignored "-Wunused-label"
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

#line 171 "src/Slice/Grammar.cpp"

#ifndef YY_CAST
#    ifdef __cplusplus
#        define YY_CAST(Type, Val) static_cast<Type>(Val)
#        define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type>(Val)
#    else
#        define YY_CAST(Type, Val) ((Type)(Val))
#        define YY_REINTERPRET_CAST(Type, Val) ((Type)(Val))
#    endif
#endif
#ifndef YY_NULLPTR
#    if defined __cplusplus
#        if 201103L <= __cplusplus
#            define YY_NULLPTR nullptr
#        else
#            define YY_NULLPTR 0
#        endif
#    else
#        define YY_NULLPTR ((void*)0)
#    endif
#endif

#include "Grammar.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
    YYSYMBOL_YYEMPTY = -2,
    YYSYMBOL_YYEOF = 0,                       /* "end of file"  */
    YYSYMBOL_YYerror = 1,                     /* error  */
    YYSYMBOL_YYUNDEF = 2,                     /* "invalid token"  */
    YYSYMBOL_ICE_MODULE = 3,                  /* ICE_MODULE  */
    YYSYMBOL_ICE_CLASS = 4,                   /* ICE_CLASS  */
    YYSYMBOL_ICE_INTERFACE = 5,               /* ICE_INTERFACE  */
    YYSYMBOL_ICE_EXCEPTION = 6,               /* ICE_EXCEPTION  */
    YYSYMBOL_ICE_STRUCT = 7,                  /* ICE_STRUCT  */
    YYSYMBOL_ICE_SEQUENCE = 8,                /* ICE_SEQUENCE  */
    YYSYMBOL_ICE_DICTIONARY = 9,              /* ICE_DICTIONARY  */
    YYSYMBOL_ICE_ENUM = 10,                   /* ICE_ENUM  */
    YYSYMBOL_ICE_OUT = 11,                    /* ICE_OUT  */
    YYSYMBOL_ICE_EXTENDS = 12,                /* ICE_EXTENDS  */
    YYSYMBOL_ICE_THROWS = 13,                 /* ICE_THROWS  */
    YYSYMBOL_ICE_VOID = 14,                   /* ICE_VOID  */
    YYSYMBOL_ICE_BOOL = 15,                   /* ICE_BOOL  */
    YYSYMBOL_ICE_BYTE = 16,                   /* ICE_BYTE  */
    YYSYMBOL_ICE_SHORT = 17,                  /* ICE_SHORT  */
    YYSYMBOL_ICE_INT = 18,                    /* ICE_INT  */
    YYSYMBOL_ICE_LONG = 19,                   /* ICE_LONG  */
    YYSYMBOL_ICE_FLOAT = 20,                  /* ICE_FLOAT  */
    YYSYMBOL_ICE_DOUBLE = 21,                 /* ICE_DOUBLE  */
    YYSYMBOL_ICE_STRING = 22,                 /* ICE_STRING  */
    YYSYMBOL_ICE_OBJECT = 23,                 /* ICE_OBJECT  */
    YYSYMBOL_ICE_CONST = 24,                  /* ICE_CONST  */
    YYSYMBOL_ICE_FALSE = 25,                  /* ICE_FALSE  */
    YYSYMBOL_ICE_TRUE = 26,                   /* ICE_TRUE  */
    YYSYMBOL_ICE_IDEMPOTENT = 27,             /* ICE_IDEMPOTENT  */
    YYSYMBOL_ICE_OPTIONAL = 28,               /* ICE_OPTIONAL  */
    YYSYMBOL_ICE_VALUE = 29,                  /* ICE_VALUE  */
    YYSYMBOL_ICE_STRING_LITERAL = 30,         /* ICE_STRING_LITERAL  */
    YYSYMBOL_ICE_INTEGER_LITERAL = 31,        /* ICE_INTEGER_LITERAL  */
    YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 32, /* ICE_FLOATING_POINT_LITERAL  */
    YYSYMBOL_ICE_IDENTIFIER = 33,             /* ICE_IDENTIFIER  */
    YYSYMBOL_ICE_SCOPED_IDENTIFIER = 34,      /* ICE_SCOPED_IDENTIFIER  */
    YYSYMBOL_ICE_METADATA_OPEN = 35,          /* ICE_METADATA_OPEN  */
    YYSYMBOL_ICE_METADATA_CLOSE = 36,         /* ICE_METADATA_CLOSE  */
    YYSYMBOL_ICE_GLOBAL_METADATA_OPEN = 37,   /* ICE_GLOBAL_METADATA_OPEN  */
    YYSYMBOL_ICE_GLOBAL_METADATA_IGNORE = 38, /* ICE_GLOBAL_METADATA_IGNORE  */
    YYSYMBOL_ICE_GLOBAL_METADATA_CLOSE = 39,  /* ICE_GLOBAL_METADATA_CLOSE  */
    YYSYMBOL_ICE_IDENT_OPEN = 40,             /* ICE_IDENT_OPEN  */
    YYSYMBOL_ICE_KEYWORD_OPEN = 41,           /* ICE_KEYWORD_OPEN  */
    YYSYMBOL_ICE_OPTIONAL_OPEN = 42,          /* ICE_OPTIONAL_OPEN  */
    YYSYMBOL_BAD_CHAR = 43,                   /* BAD_CHAR  */
    YYSYMBOL_44_ = 44,                        /* ';'  */
    YYSYMBOL_45_ = 45,                        /* '{'  */
    YYSYMBOL_46_ = 46,                        /* '}'  */
    YYSYMBOL_47_ = 47,                        /* ')'  */
    YYSYMBOL_48_ = 48,                        /* ':'  */
    YYSYMBOL_49_ = 49,                        /* '='  */
    YYSYMBOL_50_ = 50,                        /* ','  */
    YYSYMBOL_51_ = 51,                        /* '<'  */
    YYSYMBOL_52_ = 52,                        /* '>'  */
    YYSYMBOL_53_ = 53,                        /* '*'  */
    YYSYMBOL_YYACCEPT = 54,                   /* $accept  */
    YYSYMBOL_start = 55,                      /* start  */
    YYSYMBOL_opt_semicolon = 56,              /* opt_semicolon  */
    YYSYMBOL_global_meta_data = 57,           /* global_meta_data  */
    YYSYMBOL_meta_data = 58,                  /* meta_data  */
    YYSYMBOL_definitions = 59,                /* definitions  */
    YYSYMBOL_definition = 60,                 /* definition  */
    YYSYMBOL_61_1 = 61,                       /* $@1  */
    YYSYMBOL_62_2 = 62,                       /* $@2  */
    YYSYMBOL_63_3 = 63,                       /* $@3  */
    YYSYMBOL_64_4 = 64,                       /* $@4  */
    YYSYMBOL_65_5 = 65,                       /* $@5  */
    YYSYMBOL_66_6 = 66,                       /* $@6  */
    YYSYMBOL_67_7 = 67,                       /* $@7  */
    YYSYMBOL_68_8 = 68,                       /* $@8  */
    YYSYMBOL_69_9 = 69,                       /* $@9  */
    YYSYMBOL_70_10 = 70,                      /* $@10  */
    YYSYMBOL_71_11 = 71,                      /* $@11  */
    YYSYMBOL_72_12 = 72,                      /* $@12  */
    YYSYMBOL_73_13 = 73,                      /* $@13  */
    YYSYMBOL_module_def = 74,                 /* module_def  */
    YYSYMBOL_75_14 = 75,                      /* @14  */
    YYSYMBOL_76_15 = 76,                      /* @15  */
    YYSYMBOL_exception_id = 77,               /* exception_id  */
    YYSYMBOL_exception_decl = 78,             /* exception_decl  */
    YYSYMBOL_exception_def = 79,              /* exception_def  */
    YYSYMBOL_80_16 = 80,                      /* @16  */
    YYSYMBOL_exception_extends = 81,          /* exception_extends  */
    YYSYMBOL_type_id = 82,                    /* type_id  */
    YYSYMBOL_optional = 83,                   /* optional  */
    YYSYMBOL_optional_type_id = 84,           /* optional_type_id  */
    YYSYMBOL_struct_id = 85,                  /* struct_id  */
    YYSYMBOL_struct_decl = 86,                /* struct_decl  */
    YYSYMBOL_struct_def = 87,                 /* struct_def  */
    YYSYMBOL_88_17 = 88,                      /* @17  */
    YYSYMBOL_class_name = 89,                 /* class_name  */
    YYSYMBOL_class_id = 90,                   /* class_id  */
    YYSYMBOL_class_decl = 91,                 /* class_decl  */
    YYSYMBOL_class_def = 92,                  /* class_def  */
    YYSYMBOL_93_18 = 93,                      /* @18  */
    YYSYMBOL_class_extends = 94,              /* class_extends  */
    YYSYMBOL_extends = 95,                    /* extends  */
    YYSYMBOL_data_members = 96,               /* data_members  */
    YYSYMBOL_data_member = 97,                /* data_member  */
    YYSYMBOL_return_type = 98,                /* return_type  */
    YYSYMBOL_operation_preamble = 99,         /* operation_preamble  */
    YYSYMBOL_operation = 100,                 /* operation  */
    YYSYMBOL_101_19 = 101,                    /* @19  */
    YYSYMBOL_102_20 = 102,                    /* @20  */
    YYSYMBOL_interface_id = 103,              /* interface_id  */
    YYSYMBOL_interface_decl = 104,            /* interface_decl  */
    YYSYMBOL_interface_def = 105,             /* interface_def  */
    YYSYMBOL_106_21 = 106,                    /* @21  */
    YYSYMBOL_interface_list = 107,            /* interface_list  */
    YYSYMBOL_interface_extends = 108,         /* interface_extends  */
    YYSYMBOL_operations = 109,                /* operations  */
    YYSYMBOL_exception_list = 110,            /* exception_list  */
    YYSYMBOL_exception = 111,                 /* exception  */
    YYSYMBOL_sequence_def = 112,              /* sequence_def  */
    YYSYMBOL_dictionary_def = 113,            /* dictionary_def  */
    YYSYMBOL_enum_id = 114,                   /* enum_id  */
    YYSYMBOL_enum_def = 115,                  /* enum_def  */
    YYSYMBOL_116_22 = 116,                    /* @22  */
    YYSYMBOL_117_23 = 117,                    /* @23  */
    YYSYMBOL_enumerator_list = 118,           /* enumerator_list  */
    YYSYMBOL_enumerator = 119,                /* enumerator  */
    YYSYMBOL_enumerator_initializer = 120,    /* enumerator_initializer  */
    YYSYMBOL_out_qualifier = 121,             /* out_qualifier  */
    YYSYMBOL_parameters = 122,                /* parameters  */
    YYSYMBOL_throws = 123,                    /* throws  */
    YYSYMBOL_scoped_name = 124,               /* scoped_name  */
    YYSYMBOL_builtin = 125,                   /* builtin  */
    YYSYMBOL_type = 126,                      /* type  */
    YYSYMBOL_string_literal = 127,            /* string_literal  */
    YYSYMBOL_string_list = 128,               /* string_list  */
    YYSYMBOL_const_initializer = 129,         /* const_initializer  */
    YYSYMBOL_const_def = 130,                 /* const_def  */
    YYSYMBOL_keyword = 131                    /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;

/* Unqualified %code blocks.  */
#line 58 "src/Slice/Grammar.y"

// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);

#line 345 "src/Slice/Grammar.cpp"

#ifdef short
#    undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
#    include <limits.h> /* INFRINGES ON USER NAME SPACE */
#    if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#        include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#        define YY_STDINT_H
#    endif
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
#    undef UINT_LEAST8_MAX
#    undef UINT_LEAST16_MAX
#    define UINT_LEAST8_MAX 255
#    define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
#    if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#        define YYPTRDIFF_T __PTRDIFF_TYPE__
#        define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
#    elif defined PTRDIFF_MAX
#        ifndef ptrdiff_t
#            include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#        endif
#        define YYPTRDIFF_T ptrdiff_t
#        define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
#    else
#        define YYPTRDIFF_T long
#        define YYPTRDIFF_MAXIMUM LONG_MAX
#    endif
#endif

#ifndef YYSIZE_T
#    ifdef __SIZE_TYPE__
#        define YYSIZE_T __SIZE_TYPE__
#    elif defined size_t
#        define YYSIZE_T size_t
#    elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#        include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#        define YYSIZE_T size_t
#    else
#        define YYSIZE_T unsigned
#    endif
#endif

#define YYSIZE_MAXIMUM                                                                                                 \
    YY_CAST(YYPTRDIFF_T, (YYPTRDIFF_MAXIMUM < YY_CAST(YYSIZE_T, -1) ? YYPTRDIFF_MAXIMUM : YY_CAST(YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST(YYPTRDIFF_T, sizeof(X))

/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
#    if defined YYENABLE_NLS && YYENABLE_NLS
#        if ENABLE_NLS
#            include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#            define YY_(Msgid) dgettext("bison-runtime", Msgid)
#        endif
#    endif
#    ifndef YY_
#        define YY_(Msgid) Msgid
#    endif
#endif

#ifndef YY_ATTRIBUTE_PURE
#    if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#        define YY_ATTRIBUTE_PURE __attribute__((__pure__))
#    else
#        define YY_ATTRIBUTE_PURE
#    endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
#    if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#        define YY_ATTRIBUTE_UNUSED __attribute__((__unused__))
#    else
#        define YY_ATTRIBUTE_UNUSED
#    endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if !defined lint || defined __GNUC__
#    define YY_USE(E) ((void)(E))
#else
#    define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && !defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
#    if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#        define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                                                                    \
            _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wuninitialized\"")
#    else
#        define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                                                                    \
            _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wuninitialized\"")                       \
                _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#    endif
#    define YY_IGNORE_MAYBE_UNINITIALIZED_END _Pragma("GCC diagnostic pop")
#else
#    define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#    define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#    define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
#    define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && !defined __ICC && 6 <= __GNUC__
#    define YY_IGNORE_USELESS_CAST_BEGIN                                                                               \
        _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wuseless-cast\"")
#    define YY_IGNORE_USELESS_CAST_END _Pragma("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
#    define YY_IGNORE_USELESS_CAST_BEGIN
#    define YY_IGNORE_USELESS_CAST_END
#endif

#define YY_ASSERT(E) ((void)(0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

#    ifdef YYSTACK_USE_ALLOCA
#        if YYSTACK_USE_ALLOCA
#            ifdef __GNUC__
#                define YYSTACK_ALLOC __builtin_alloca
#            elif defined __BUILTIN_VA_ARG_INCR
#                include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#            elif defined _AIX
#                define YYSTACK_ALLOC __alloca
#            elif defined _MSC_VER
#                include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#                define alloca _alloca
#            else
#                define YYSTACK_ALLOC alloca
#                if !defined _ALLOCA_H && !defined EXIT_SUCCESS
#                    include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
/* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#                    ifndef EXIT_SUCCESS
#                        define EXIT_SUCCESS 0
#                    endif
#                endif
#            endif
#        endif
#    endif

#    ifdef YYSTACK_ALLOC
/* Pacify GCC's 'empty if-body' warning.  */
#        define YYSTACK_FREE(Ptr)                                                                                      \
            do                                                                                                         \
            { /* empty */                                                                                              \
                ;                                                                                                      \
            } while (0)
#        ifndef YYSTACK_ALLOC_MAXIMUM
/* The OS might guarantee only one guard page at the bottom of the stack,
   and a page size can be as small as 4096 bytes.  So we cannot safely
   invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
   to allow for a few compiler-allocated temporary stack slots.  */
#            define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#        endif
#    else
#        define YYSTACK_ALLOC YYMALLOC
#        define YYSTACK_FREE YYFREE
#        ifndef YYSTACK_ALLOC_MAXIMUM
#            define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#        endif
#        if (                                                                                                          \
            defined __cplusplus && !defined EXIT_SUCCESS &&                                                            \
            !((defined YYMALLOC || defined malloc) && (defined YYFREE || defined free)))
#            include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#            ifndef EXIT_SUCCESS
#                define EXIT_SUCCESS 0
#            endif
#        endif
#        ifndef YYMALLOC
#            define YYMALLOC malloc
#            if !defined malloc && !defined EXIT_SUCCESS
void* malloc(YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#            endif
#        endif
#        ifndef YYFREE
#            define YYFREE free
#            if !defined free && !defined EXIT_SUCCESS
void free(void*); /* INFRINGES ON USER NAME SPACE */
#            endif
#        endif
#    endif
#endif /* !defined yyoverflow */

#if (                                                                                                                  \
    !defined yyoverflow && (!defined __cplusplus || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL &&               \
                                                     defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
    yy_state_t yyss_alloc;
    YYSTYPE yyvs_alloc;
    YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
#    define YYSTACK_GAP_MAXIMUM (YYSIZEOF(union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
#    define YYSTACK_BYTES(N)                                                                                           \
        ((N) * (YYSIZEOF(yy_state_t) + YYSIZEOF(YYSTYPE) + YYSIZEOF(YYLTYPE)) + 2 * YYSTACK_GAP_MAXIMUM)

#    define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
#    define YYSTACK_RELOCATE(Stack_alloc, Stack)                                                                       \
        do                                                                                                             \
        {                                                                                                              \
            YYPTRDIFF_T yynewbytes;                                                                                    \
            YYCOPY(&yyptr->Stack_alloc, Stack, yysize);                                                                \
            Stack = &yyptr->Stack_alloc;                                                                               \
            yynewbytes = yystacksize * YYSIZEOF(*Stack) + YYSTACK_GAP_MAXIMUM;                                         \
            yyptr += yynewbytes / YYSIZEOF(*yyptr);                                                                    \
        } while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
#    ifndef YYCOPY
#        if defined __GNUC__ && 1 < __GNUC__
#            define YYCOPY(Dst, Src, Count) __builtin_memcpy(Dst, Src, YY_CAST(YYSIZE_T, (Count)) * sizeof(*(Src)))
#        else
#            define YYCOPY(Dst, Src, Count)                                                                            \
                do                                                                                                     \
                {                                                                                                      \
                    YYPTRDIFF_T yyi;                                                                                   \
                    for (yyi = 0; yyi < (Count); yyi++)                                                                \
                        (Dst)[yyi] = (Src)[yyi];                                                                       \
                } while (0)
#        endif
#    endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL 3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST 653

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS 54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS 78
/* YYNRULES -- Number of rules.  */
#define YYNRULES 202
/* YYNSTATES -- Number of states.  */
#define YYNSTATES 293

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK 298

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                                                               \
    (0 <= (YYX) && (YYX) <= YYMAXUTOK ? YY_CAST(yysymbol_kind_t, yytranslate[YYX]) : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] = {
    0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  47, 53, 2,  50, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  48, 44, 51, 49, 52, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  45, 2,  46, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] = {
    0,    185,  185,  193,  196,  204,  208,  218,  222,  231,  239,  248,  257,  256,  262,  261,  266,  271,  270,
    276,  275,  280,  285,  284,  290,  289,  294,  299,  298,  304,  303,  308,  313,  312,  318,  317,  322,  327,
    326,  331,  336,  335,  341,  340,  345,  349,  359,  358,  387,  386,  468,  472,  483,  494,  493,  519,  527,
    536,  549,  567,  643,  649,  660,  677,  690,  694,  705,  716,  715,  754,  758,  769,  794,  882,  894,  907,
    906,  940,  974,  983,  986,  994,  1003, 1006, 1010, 1018, 1048, 1082, 1104, 1130, 1145, 1151, 1161, 1185, 1215,
    1239, 1274, 1273, 1296, 1295, 1318, 1322, 1333, 1347, 1346, 1380, 1415, 1450, 1455, 1465, 1469, 1478, 1487, 1490,
    1494, 1502, 1509, 1521, 1533, 1544, 1552, 1566, 1576, 1592, 1596, 1608, 1607, 1639, 1638, 1656, 1662, 1670, 1682,
    1702, 1709, 1719, 1723, 1764, 1770, 1781, 1784, 1800, 1816, 1828, 1840, 1851, 1867, 1871, 1880, 1883, 1891, 1892,
    1893, 1894, 1895, 1896, 1897, 1898, 1899, 1900, 1905, 1909, 1914, 1945, 1981, 1987, 1995, 2002, 2014, 2023, 2032,
    2067, 2074, 2081, 2093, 2102, 2116, 2117, 2118, 2119, 2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127, 2128, 2129,
    2130, 2131, 2132, 2133, 2134, 2135, 2136, 2137, 2138, 2139, 2140, 2141, 2142};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST(yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char* yysymbol_name(yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char* const yytname[] = {
    "\"end of file\"",
    "error",
    "\"invalid token\"",
    "ICE_MODULE",
    "ICE_CLASS",
    "ICE_INTERFACE",
    "ICE_EXCEPTION",
    "ICE_STRUCT",
    "ICE_SEQUENCE",
    "ICE_DICTIONARY",
    "ICE_ENUM",
    "ICE_OUT",
    "ICE_EXTENDS",
    "ICE_THROWS",
    "ICE_VOID",
    "ICE_BOOL",
    "ICE_BYTE",
    "ICE_SHORT",
    "ICE_INT",
    "ICE_LONG",
    "ICE_FLOAT",
    "ICE_DOUBLE",
    "ICE_STRING",
    "ICE_OBJECT",
    "ICE_CONST",
    "ICE_FALSE",
    "ICE_TRUE",
    "ICE_IDEMPOTENT",
    "ICE_OPTIONAL",
    "ICE_VALUE",
    "ICE_STRING_LITERAL",
    "ICE_INTEGER_LITERAL",
    "ICE_FLOATING_POINT_LITERAL",
    "ICE_IDENTIFIER",
    "ICE_SCOPED_IDENTIFIER",
    "ICE_METADATA_OPEN",
    "ICE_METADATA_CLOSE",
    "ICE_GLOBAL_METADATA_OPEN",
    "ICE_GLOBAL_METADATA_IGNORE",
    "ICE_GLOBAL_METADATA_CLOSE",
    "ICE_IDENT_OPEN",
    "ICE_KEYWORD_OPEN",
    "ICE_OPTIONAL_OPEN",
    "BAD_CHAR",
    "';'",
    "'{'",
    "'}'",
    "')'",
    "':'",
    "'='",
    "','",
    "'<'",
    "'>'",
    "'*'",
    "$accept",
    "start",
    "opt_semicolon",
    "global_meta_data",
    "meta_data",
    "definitions",
    "definition",
    "$@1",
    "$@2",
    "$@3",
    "$@4",
    "$@5",
    "$@6",
    "$@7",
    "$@8",
    "$@9",
    "$@10",
    "$@11",
    "$@12",
    "$@13",
    "module_def",
    "@14",
    "@15",
    "exception_id",
    "exception_decl",
    "exception_def",
    "@16",
    "exception_extends",
    "type_id",
    "optional",
    "optional_type_id",
    "struct_id",
    "struct_decl",
    "struct_def",
    "@17",
    "class_name",
    "class_id",
    "class_decl",
    "class_def",
    "@18",
    "class_extends",
    "extends",
    "data_members",
    "data_member",
    "return_type",
    "operation_preamble",
    "operation",
    "@19",
    "@20",
    "interface_id",
    "interface_decl",
    "interface_def",
    "@21",
    "interface_list",
    "interface_extends",
    "operations",
    "exception_list",
    "exception",
    "sequence_def",
    "dictionary_def",
    "enum_id",
    "enum_def",
    "@22",
    "@23",
    "enumerator_list",
    "enumerator",
    "enumerator_initializer",
    "out_qualifier",
    "parameters",
    "throws",
    "scoped_name",
    "builtin",
    "type",
    "string_literal",
    "string_list",
    "const_initializer",
    "const_def",
    "keyword",
    YY_NULLPTR};

static const char*
yysymbol_name(yysymbol_kind_t yysymbol)
{
    return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-211)

#define yypact_value_is_default(Yyn) ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-140)

#define yytable_value_is_error(Yyn) 0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] = {
    -211, 2,    36,   -211, -10,  -10,  -10,  -211, 115,  -10,  -211, -22,  9,    58,   -20,  97,   276,  346,  377,
    408,  -29,  16,   439,  50,   -211, -211, 13,   48,   -211, 57,   65,   -211, 15,   1,    70,   -211, 56,   94,
    -211, 96,   108,  -211, -211, 118,  -211, -211, -10,  -211, -211, -211, -211, -211, -211, -211, -211, -211, -211,
    -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211,
    -211, -211, -211, -211, 156,  -211, -211, -211, -211, -211, -211, -211, 50,   50,   -211, 124,  -211, 224,  132,
    -211, -211, -211, 117,  136,  132,  137,  147,  132,  -211, 117,  155,  132,  77,   -211, 175,  132,  178,  180,
    182,  132,  184,  -211, 185,  186,  189,  -211, -211, 190,  224,  224,  470,  -211, -211, -211, -211, -211, -211,
    -211, -211, 179,  -211, 195,  -211, 21,   -211, -211, 204,  -211, -211, -211, 126,  -211, -211, 205,  -211, -211,
    -211, -211, -211, -211, 183,  206,  -211, -211, -211, -211, 470,  -211, -211, -211, -211, -211, -211, 202,  210,
    207,  209,  212,  -211, -211, -211, 214,  103,  126,  221,  611,  220,  126,  77,   61,   222,  128,  188,  501,
    50,   161,  -211, 470,  103,  -211, -211, -211, -211, -211, -211, -211, 223,  126,  -211, -16,  -211, 224,  225,
    226,  532,  -211, 227,  -211, 228,  187,  229,  -211, -211, -211, -211, -211, 224,  -211, -211, -211, -211, -211,
    -211, -211, 259,  -211, 260,  -211, 238,  103,  126,  -211, -211, -211, 61,   -211, 583,  224,  172,  22,   264,
    -211, -211, 258,  -211, -211, -211, -211, -211, 177,  -211, -211, -211, 265,  -211, 50,   123,  61,   563,  -211,
    -211, -211, 611,  -211, 300,  -211, -211, -211, 301,  -211, 532,  301,  50,   314,  -211, -211, -211, 611,  -211,
    263,  -211, -211, -211, 532,  314,  -211, -211};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] = {
    11,  0,   8,   1,   0,   0,   0,   9,   0,   165, 167, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   127,
    8,   10,  12,  52,  26,  27,  66,  31,  32,  74,  78,  16,  17,  102, 21,  22,  36,  39,  125, 40,  44,  164, 7,
    0,   5,   6,   45,  46,  48,  176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192,
    193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 69,  0,   70,  100, 101, 50,  51,  64,  65,  8,   8,   123, 0,
    124, 0,   4,   79,  80,  53,  0,   0,   4,   0,   0,   4,   75,  0,   0,   4,   0,   103, 0,   4,   0,   0,   0,
    4,   0,   166, 0,   0,   0,   148, 149, 0,   0,   0,   134, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 162,
    161, 0,   3,   13,  0,   55,  25,  28,  0,   30,  33,  0,   77,  15,  18,  107, 108, 109, 106, 0,   20,  23,  35,
    38,  134, 41,  43,  11,  11,  71,  72,  0,   0,   131, 0,   130, 133, 160, 163, 0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   8,   8,   0,   8,   0,   128, 134, 0,   172, 173, 171, 168, 169, 170, 175, 0,   0,   61,  0,   63,
    0,   85,  83,  88,  68,  0,   105, 0,   0,   0,   126, 47,  49,  119, 120, 0,   135, 132, 136, 129, 174, 54,  82,
    0,   60,  0,   62,  0,   0,   0,   57,  87,  76,  0,   91,  0,   0,   0,   0,   113, 90,  104, 0,   58,  59,  86,
    81,  112, 0,   89,  92,  94,  0,   137, 8,   0,   0,   0,   93,  95,  98,  0,   96,  138, 111, 121, 122, 147, 140,
    144, 147, 8,   0,   99,  142, 97,  0,   146, 116, 117, 118, 141, 145, 0,   143, 115};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] = {-211, -211, 64,   -211, -2,   -8,   -211, -211, -211, -211, -211, -211, -211,
                                       -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211, -211,
                                       -211, -211, 28,   -149, -170, -211, -211, -211, -211, -211, -211, -211, -211,
                                       -211, -211, 148,  -171, -211, 73,   -211, -211, -211, -211, -211, -211, -211,
                                       -211, 160,  -211, -210, 55,   -211, -211, -211, -211, -211, -211, -211, -146,
                                       -211, -211, 76,   -211, 99,   -79,  -211, -90,  0,    191,  -183, -211, -12};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] = {0,   1,   141, 7,   181, 2,   24,  94,  106, 107, 110, 111, 99,  100, 102, 103,
                                         112, 113, 115, 116, 25,  118, 119, 26,  27,  28,  142, 97,  206, 207, 208, 29,
                                         30,  31,  101, 32,  33,  34,  35,  149, 104, 98,  182, 209, 244, 245, 246, 277,
                                         274, 36,  37,  38,  157, 155, 109, 216, 284, 285, 39,  40,  41,  42,  114, 91,
                                         172, 173, 224, 261, 262, 280, 137, 138, 247, 10,  11,  201, 43,  174};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] = {
    8,    123,  3,    139,  81,   83,   85,  87,   202, 44,   92,   227,  212,  95,  45,  230,  186,  121,  122,  143,
    9,    93,   88,   259,  49,   95,   150, -73,  46,  156,  254,  231,  229,  260, 169, 170,  -2,   -138, -138, -138,
    -138, -138, -138, -138, -138, -138, 117, 226,  47,  96,   -138, -138, 252,  271, 177, -138, -138, -138, -56,  46,
    -73,  96,   214,  -73,  -138, 253,  243, 89,   95,  -139, 178,  4,    -139, 5,   6,   -8,   -8,   -8,   -8,   -8,
    -8,   -8,   -8,   -8,   -8,   4,    124, 125,  -8,  -8,   -8,   210,  -24,  243, -8,  -8,   4,    48,   275,  200,
    153,  -110, -67,  -8,   96,   156,  154, -114, 46,  -29,  121,  122,  225,  288, -14, 200,  14,   234,  15,   16,
    17,   18,   19,   20,   21,   22,   232, 180,  195, 196,  50,   51,   249,  197, 198, 199,  121,  122,  -19,  23,
    -34,  -8,   -8,   -8,   -8,   -8,   -8,  -8,   -8,  -8,   121,  122,  -37,  256, -8,  -8,   200,  187,  188,  -8,
    -8,   4,    -42,  4,    145,  5,    6,   148,  -8,  126,  269,  152,  -84,  270, 218, 159,  140,  221,  276,  163,
    144,  105,  146,  215,  108,  8,    8,   120,  222, 121,  122,  147,  223,  289, 121, 122,  12,   13,   238,  151,
    286,  241,  127,  128,  129,  130,  131, 132,  133, 134,  135,  286,  257,  258, 242, 204,  136,  265,  266,  158,
    121,  122,  160,  4,    161,  5,    6,   162,  164, 205,  165,  166,  175,  184, 219, 233,  167,  168,  215,  127,
    128,  129,  130,  131,  132,  133,  134, 135,  176, 179,  183,  185,  273,  136, 189, 192,  191,  121,  122,  268,
    190,  215,  193,  194,  281,  203,  211, 287,  217, 228,  236,  237,  240,  239, 235, 248,  283,  291,  287,  52,
    53,   54,   55,   56,   57,   58,   59,  60,   61,  62,   63,   64,   65,   66,  67,  68,   69,   70,   71,   72,
    73,   74,   75,   76,   77,   78,   250, 251,  263, 79,   264,  260,  267,  290, 279, 255,  80,   52,   53,   54,
    55,   56,   57,   58,   59,   60,   61,  62,   63,  64,   65,   66,   67,   68,  69,  70,   71,   72,   73,   74,
    75,   76,   77,   78,   213,  292,  278, 121,  122, 52,   53,   54,   55,   56,  57,  58,   59,   60,   61,   62,
    63,   64,   65,   66,   67,   68,   69,  70,   71,  72,   73,   74,   75,   76,  77,  78,   282,  0,    0,    82,
    52,   53,   54,   55,   56,   57,   58,  59,   60,  61,   62,   63,   64,   65,  66,  67,   68,   69,   70,   71,
    72,   73,   74,   75,   76,   77,   78,  0,    0,   0,    84,   52,   53,   54,  55,  56,   57,   58,   59,   60,
    61,   62,   63,   64,   65,   66,   67,  68,   69,  70,   71,   72,   73,   74,  75,  76,   77,   78,   0,    0,
    0,    86,   52,   53,   54,   55,   56,  57,   58,  59,   60,   61,   62,   63,  64,  65,   66,   67,   68,   69,
    70,   71,   72,   73,   74,   75,   76,  77,   78,  0,    0,    0,    90,   52,  53,  54,   55,   56,   57,   58,
    59,   60,   61,   62,   63,   64,   65,  66,   67,  68,   69,   70,   71,   72,  73,  74,   75,   76,   77,   78,
    0,    0,    0,    171,  52,   53,   54,  55,   56,  57,   58,   59,   60,   61,  62,  63,   64,   65,   66,   67,
    68,   69,   70,   71,   72,   73,   74,  75,   76,  77,   78,   0,    0,    0,   220, 52,   53,   54,   55,   56,
    57,   58,   59,   60,   61,   62,   63,  64,   65,  66,   67,   68,   69,   70,  71,  72,   73,   74,   75,   76,
    77,   78,   0,    0,    0,    237,  52,  53,   54,  55,   56,   57,   58,   59,  60,  61,   62,   63,   64,   65,
    66,   67,   68,   69,   70,   71,   72,  73,   74,  75,   76,   77,   78,   0,   0,   0,    272,  241,  127,  128,
    129,  130,  131,  132,  133,  134,  135, 0,    0,   0,    0,    204,  136,  0,   0,   0,    121,  122,  0,    0,
    0,    0,    0,    0,    0,    205,  127, 128,  129, 130,  131,  132,  133,  134, 135, 0,    0,    0,    0,    204,
    136,  0,    0,    0,    121,  122,  0,   0,    0,   0,    0,    0,    0,    205};

static const yytype_int16 yycheck[] = {
    2,   80,  0,   93,  16,  17,  18,  19,  179, 9,   22,  194, 183, 12,  36,  31,  162, 33,  34,  98,  30,  23,  51,
    1,   44,  12,  105, 12,  50,  108, 240, 47,  203, 11,  124, 125, 0,   15,  16,  17,  18,  19,  20,  21,  22,  23,
    46,  193, 39,  48,  28,  29,  235, 263, 33,  33,  34,  35,  45,  50,  45,  48,  1,   48,  42,  236, 215, 51,  12,
    47,  49,  35,  50,  37,  38,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  35,  88,  89,  27,  28,  29,  181,
    44,  242, 33,  34,  35,  39,  268, 178, 23,  45,  45,  42,  48,  184, 29,  46,  50,  44,  33,  34,  191, 283, 44,
    194, 1,   207, 3,   4,   5,   6,   7,   8,   9,   10,  205, 1,   25,  26,  33,  34,  222, 30,  31,  32,  33,  34,
    44,  24,  44,  15,  16,  17,  18,  19,  20,  21,  22,  23,  33,  34,  44,  243, 28,  29,  235, 165, 166, 33,  34,
    35,  44,  35,  100, 37,  38,  103, 42,  45,  47,  107, 46,  50,  46,  111, 44,  189, 268, 115, 44,  33,  45,  185,
    36,  187, 188, 31,  190, 33,  34,  44,  31,  283, 33,  34,  5,   6,   210, 44,  279, 14,  15,  16,  17,  18,  19,
    20,  21,  22,  23,  290, 40,  41,  27,  28,  29,  40,  41,  44,  33,  34,  44,  35,  44,  37,  38,  45,  44,  42,
    45,  45,  53,  50,  46,  207, 47,  47,  240, 15,  16,  17,  18,  19,  20,  21,  22,  23,  53,  45,  45,  45,  264,
    29,  52,  46,  49,  33,  34,  261, 50,  263, 50,  49,  276, 44,  46,  279, 46,  46,  44,  33,  44,  46,  49,  46,
    278, 289, 290, 3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
    23,  24,  25,  26,  27,  28,  29,  47,  47,  44,  33,  52,  11,  47,  50,  13,  242, 40,  3,   4,   5,   6,   7,
    8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  184,
    290, 270, 33,  34,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
    22,  23,  24,  25,  26,  27,  28,  29,  277, -1,  -1,  33,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,
    14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  -1,  -1,  -1,  33,  3,   4,   5,
    6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,
    29,  -1,  -1,  -1,  33,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
    21,  22,  23,  24,  25,  26,  27,  28,  29,  -1,  -1,  -1,  33,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,
    13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  -1,  -1,  -1,  33,  3,   4,
    5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
    28,  29,  -1,  -1,  -1,  33,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
    20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  -1,  -1,  -1,  33,  3,   4,   5,   6,   7,   8,   9,   10,  11,
    12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  -1,  -1,  -1,  33,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  -1,  -1,  -1,  -1,  28,  29,  -1,  -1,  -1,  33,  34,  -1,  -1,  -1,
    -1,  -1,  -1,  -1,  42,  15,  16,  17,  18,  19,  20,  21,  22,  23,  -1,  -1,  -1,  -1,  28,  29,  -1,  -1,  -1,
    33,  34,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  42};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] = {
    0,   55,  59,  0,   35,  37,  38,  57, 58,  30,  127, 128, 128, 128, 1,   3,   4,   5,   6,   7,   8,   9,   10,
    24,  60,  74,  77,  78,  79,  85,  86, 87,  89,  90,  91,  92,  103, 104, 105, 112, 113, 114, 115, 130, 127, 36,
    50,  39,  39,  44,  33,  34,  3,   4,  5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
    20,  21,  22,  23,  24,  25,  26,  27, 28,  29,  33,  40,  131, 33,  131, 33,  131, 33,  131, 51,  51,  33,  117,
    131, 58,  61,  12,  48,  81,  95,  66, 67,  88,  68,  69,  94,  95,  62,  63,  95,  108, 64,  65,  70,  71,  116,
    72,  73,  127, 75,  76,  31,  33,  34, 124, 58,  58,  45,  15,  16,  17,  18,  19,  20,  21,  22,  23,  29,  124,
    125, 126, 44,  56,  80,  124, 44,  56, 45,  44,  56,  93,  124, 44,  56,  23,  29,  107, 124, 106, 44,  56,  44,
    44,  45,  56,  44,  45,  45,  47,  47, 126, 126, 33,  118, 119, 131, 53,  53,  33,  49,  45,  1,   58,  96,  45,
    50,  45,  118, 59,  59,  52,  50,  49, 46,  50,  49,  25,  26,  30,  31,  32,  124, 129, 96,  44,  28,  42,  82,
    83,  84,  97,  126, 46,  96,  107, 1,  58,  109, 46,  46,  46,  33,  131, 58,  31,  120, 124, 118, 129, 46,  96,
    31,  47,  124, 82,  126, 49,  44,  33, 131, 46,  44,  14,  27,  83,  98,  99,  100, 126, 46,  126, 47,  47,  129,
    96,  109, 98,  126, 40,  41,  1,   11, 121, 122, 44,  52,  40,  41,  47,  58,  47,  50,  109, 33,  131, 102, 84,
    126, 101, 121, 13,  123, 131, 123, 58, 110, 111, 124, 131, 84,  126, 50,  131, 110};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] = {
    0,   54,  55,  56,  56,  57,  57,  58,  58,  59,  59,  59,  61,  60,  62,  60,  60,  63,  60,  64,  60,  60,  65,
    60,  66,  60,  60,  67,  60,  68,  60,  60,  69,  60,  70,  60,  60,  71,  60,  60,  72,  60,  73,  60,  60,  60,
    75,  74,  76,  74,  77,  77,  78,  80,  79,  81,  81,  82,  83,  83,  83,  83,  84,  84,  85,  85,  86,  88,  87,
    89,  89,  90,  90,  90,  91,  93,  92,  94,  94,  95,  95,  96,  96,  96,  96,  97,  97,  97,  97,  98,  98,  98,
    99,  99,  99,  99,  101, 100, 102, 100, 103, 103, 104, 106, 105, 107, 107, 107, 107, 108, 108, 109, 109, 109, 109,
    110, 110, 111, 111, 112, 112, 113, 113, 114, 114, 116, 115, 117, 115, 118, 118, 119, 119, 119, 119, 120, 120, 121,
    121, 122, 122, 122, 122, 122, 122, 122, 123, 123, 124, 124, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 126,
    126, 126, 126, 127, 127, 128, 128, 129, 129, 129, 129, 129, 129, 130, 130, 131, 131, 131, 131, 131, 131, 131, 131,
    131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] = {
    0, 2, 1, 1, 0, 3, 3, 3, 0, 2, 3, 0, 0, 3, 0, 3, 1, 0, 3, 0, 3, 1, 0, 3, 0, 3, 1, 0, 3, 0, 3, 1, 0, 3,
    0, 3, 1, 0, 3, 1, 0, 3, 0, 3, 1, 2, 0, 6, 0, 6, 2, 2, 1, 0, 6, 2, 0, 2, 3, 3, 2, 1, 2, 1, 2, 2, 1, 0,
    5, 2, 2, 4, 4, 1, 1, 0, 6, 2, 0, 1, 1, 4, 3, 2, 0, 1, 3, 2, 1, 2, 1, 1, 2, 3, 2, 3, 0, 5, 0, 5, 2, 2,
    1, 0, 6, 3, 1, 1, 1, 2, 0, 4, 3, 2, 0, 3, 1, 1, 1, 6, 6, 9, 9, 2, 2, 0, 5, 0, 5, 3, 1, 1, 3, 1, 0, 1,
    1, 1, 0, 0, 3, 5, 4, 6, 3, 5, 2, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 2, 1, 3, 1, 1, 1,
    1, 1, 1, 1, 6, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

enum
{
    YYENOMEM = -2
};

#define yyerrok (yyerrstatus = 0)
#define yyclearin (yychar = YYEMPTY)

#define YYACCEPT goto yyacceptlab
#define YYABORT goto yyabortlab
#define YYERROR goto yyerrorlab
#define YYNOMEM goto yyexhaustedlab

#define YYRECOVERING() (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                                                                         \
    do                                                                                                                 \
        if (yychar == YYEMPTY)                                                                                         \
        {                                                                                                              \
            yychar = (Token);                                                                                          \
            yylval = (Value);                                                                                          \
            YYPOPSTACK(yylen);                                                                                         \
            yystate = *yyssp;                                                                                          \
            goto yybackup;                                                                                             \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            yyerror(YY_("syntax error: cannot back up"));                                                              \
            YYERROR;                                                                                                   \
        }                                                                                                              \
    while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
#    define YYLLOC_DEFAULT(Current, Rhs, N)                                                                            \
        do                                                                                                             \
            if (N)                                                                                                     \
            {                                                                                                          \
                (Current).first_line = YYRHSLOC(Rhs, 1).first_line;                                                    \
                (Current).first_column = YYRHSLOC(Rhs, 1).first_column;                                                \
                (Current).last_line = YYRHSLOC(Rhs, N).last_line;                                                      \
                (Current).last_column = YYRHSLOC(Rhs, N).last_column;                                                  \
            }                                                                                                          \
            else                                                                                                       \
            {                                                                                                          \
                (Current).first_line = (Current).last_line = YYRHSLOC(Rhs, 0).last_line;                               \
                (Current).first_column = (Current).last_column = YYRHSLOC(Rhs, 0).last_column;                         \
            }                                                                                                          \
        while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])

/* Enable debugging if requested.  */
#if YYDEBUG

#    ifndef YYFPRINTF
#        include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#        define YYFPRINTF fprintf
#    endif

#    define YYDPRINTF(Args)                                                                                            \
        do                                                                                                             \
        {                                                                                                              \
            if (yydebug)                                                                                               \
                YYFPRINTF Args;                                                                                        \
        } while (0)

/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#    ifndef YYLOCATION_PRINT

#        if defined YY_LOCATION_PRINT

/* Temporary convenience wrapper in case some people defined the
   undocumented and private YY_LOCATION_PRINT macros.  */
#            define YYLOCATION_PRINT(File, Loc) YY_LOCATION_PRINT(File, *(Loc))

#        elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_(FILE* yyo, YYLTYPE const* const yylocp)
{
    int res = 0;
    int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
    if (0 <= yylocp->first_line)
    {
        res += YYFPRINTF(yyo, "%d", yylocp->first_line);
        if (0 <= yylocp->first_column)
            res += YYFPRINTF(yyo, ".%d", yylocp->first_column);
    }
    if (0 <= yylocp->last_line)
    {
        if (yylocp->first_line < yylocp->last_line)
        {
            res += YYFPRINTF(yyo, "-%d", yylocp->last_line);
            if (0 <= end_col)
                res += YYFPRINTF(yyo, ".%d", end_col);
        }
        else if (0 <= end_col && yylocp->first_column < end_col)
            res += YYFPRINTF(yyo, "-%d", end_col);
    }
    return res;
}

#            define YYLOCATION_PRINT yy_location_print_

/* Temporary convenience wrapper in case some people defined the
   undocumented and private YY_LOCATION_PRINT macros.  */
#            define YY_LOCATION_PRINT(File, Loc) YYLOCATION_PRINT(File, &(Loc))

#        else

#            define YYLOCATION_PRINT(File, Loc) ((void)0)
/* Temporary convenience wrapper in case some people defined the
   undocumented and private YY_LOCATION_PRINT macros.  */
#            define YY_LOCATION_PRINT YYLOCATION_PRINT

#        endif
#    endif /* !defined YYLOCATION_PRINT */

#    define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                                                              \
        do                                                                                                             \
        {                                                                                                              \
            if (yydebug)                                                                                               \
            {                                                                                                          \
                YYFPRINTF(stderr, "%s ", Title);                                                                       \
                yy_symbol_print(stderr, Kind, Value, Location);                                                        \
                YYFPRINTF(stderr, "\n");                                                                               \
            }                                                                                                          \
        } while (0)

/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print(
    FILE* yyo,
    yysymbol_kind_t yykind,
    YYSTYPE const* const yyvaluep,
    YYLTYPE const* const yylocationp)
{
    FILE* yyoutput = yyo;
    YY_USE(yyoutput);
    YY_USE(yylocationp);
    if (!yyvaluep)
        return;
    YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
    YY_USE(yykind);
    YY_IGNORE_MAYBE_UNINITIALIZED_END
}

/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print(FILE* yyo, yysymbol_kind_t yykind, YYSTYPE const* const yyvaluep, YYLTYPE const* const yylocationp)
{
    YYFPRINTF(yyo, "%s %s (", yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name(yykind));

    YYLOCATION_PRINT(yyo, yylocationp);
    YYFPRINTF(yyo, ": ");
    yy_symbol_value_print(yyo, yykind, yyvaluep, yylocationp);
    YYFPRINTF(yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print(yy_state_t* yybottom, yy_state_t* yytop)
{
    YYFPRINTF(stderr, "Stack now");
    for (; yybottom <= yytop; yybottom++)
    {
        int yybot = *yybottom;
        YYFPRINTF(stderr, " %d", yybot);
    }
    YYFPRINTF(stderr, "\n");
}

#    define YY_STACK_PRINT(Bottom, Top)                                                                                \
        do                                                                                                             \
        {                                                                                                              \
            if (yydebug)                                                                                               \
                yy_stack_print((Bottom), (Top));                                                                       \
        } while (0)

/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print(yy_state_t* yyssp, YYSTYPE* yyvsp, YYLTYPE* yylsp, int yyrule)
{
    int yylno = yyrline[yyrule];
    int yynrhs = yyr2[yyrule];
    int yyi;
    YYFPRINTF(stderr, "Reducing stack by rule %d (line %d):\n", yyrule - 1, yylno);
    /* The symbols being reduced.  */
    for (yyi = 0; yyi < yynrhs; yyi++)
    {
        YYFPRINTF(stderr, "   $%d = ", yyi + 1);
        yy_symbol_print(
            stderr,
            YY_ACCESSING_SYMBOL(+yyssp[yyi + 1 - yynrhs]),
            &yyvsp[(yyi + 1) - (yynrhs)],
            &(yylsp[(yyi + 1) - (yynrhs)]));
        YYFPRINTF(stderr, "\n");
    }
}

#    define YY_REDUCE_PRINT(Rule)                                                                                      \
        do                                                                                                             \
        {                                                                                                              \
            if (yydebug)                                                                                               \
                yy_reduce_print(yyssp, yyvsp, yylsp, Rule);                                                            \
        } while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
#    define YYDPRINTF(Args) ((void)0)
#    define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
#    define YY_STACK_PRINT(Bottom, Top)
#    define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
#    define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
#    define YYMAXDEPTH 10000
#endif

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct(const char* yymsg, yysymbol_kind_t yykind, YYSTYPE* yyvaluep, YYLTYPE* yylocationp)
{
    YY_USE(yyvaluep);
    YY_USE(yylocationp);
    if (!yymsg)
        yymsg = "Deleting";
    YY_SYMBOL_PRINT(yymsg, yykind, yyvaluep, yylocationp);

    YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
    YY_USE(yykind);
    YY_IGNORE_MAYBE_UNINITIALIZED_END
}

/*----------.
| yyparse.  |
`----------*/

int
yyparse(void)
{
    /* Lookahead token kind.  */
    int yychar;

    /* The semantic value of the lookahead symbol.  */
    /* Default value used for initialization, for pacifying older GCCs
       or non-GCC compilers.  */
    YY_INITIAL_VALUE(static YYSTYPE yyval_default;)
    YYSTYPE yylval YY_INITIAL_VALUE(= yyval_default);

    /* Location data for the lookahead symbol.  */
    static YYLTYPE yyloc_default
#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
        = {1, 1, 1, 1}
#endif
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
    yy_state_t* yyss = yyssa;
    yy_state_t* yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE* yyvs = yyvsa;
    YYSTYPE* yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE* yyls = yylsa;
    YYLTYPE* yylsp = yyls;

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

#define YYPOPSTACK(N) (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

    /* The number of symbols on the RHS of the reduced rule.
       Keep to zero when no symbol should be popped.  */
    int yylen = 0;

    YYDPRINTF((stderr, "Starting parse\n"));

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
    YYDPRINTF((stderr, "Entering state %d\n", yystate));
    YY_ASSERT(0 <= yystate && yystate < YYNSTATES);
    YY_IGNORE_USELESS_CAST_BEGIN
    *yyssp = YY_CAST(yy_state_t, yystate);
    YY_IGNORE_USELESS_CAST_END
    YY_STACK_PRINT(yyss, yyssp);

    if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
        YYNOMEM;
#else
    {
        /* Get the current used size of the three stacks, in elements.  */
        YYPTRDIFF_T yysize = yyssp - yyss + 1;

#    if defined yyoverflow
        {
            /* Give user a chance to reallocate the stack.  Use copies of
               these so that the &'s don't force the real ones into
               memory.  */
            yy_state_t* yyss1 = yyss;
            YYSTYPE* yyvs1 = yyvs;
            YYLTYPE* yyls1 = yyls;

            /* Each stack pointer address is followed by the size of the
               data in use in that stack, in bytes.  This used to be a
               conditional around just the two extra args, but that might
               be undefined if yyoverflow is a macro.  */
            yyoverflow(
                YY_("memory exhausted"),
                &yyss1,
                yysize * YYSIZEOF(*yyssp),
                &yyvs1,
                yysize * YYSIZEOF(*yyvsp),
                &yyls1,
                yysize * YYSIZEOF(*yylsp),
                &yystacksize);
            yyss = yyss1;
            yyvs = yyvs1;
            yyls = yyls1;
        }
#    else /* defined YYSTACK_RELOCATE */
        /* Extend the stack our own way.  */
        if (YYMAXDEPTH <= yystacksize)
            YYNOMEM;
        yystacksize *= 2;
        if (YYMAXDEPTH < yystacksize)
            yystacksize = YYMAXDEPTH;

        {
            yy_state_t* yyss1 = yyss;
            union yyalloc* yyptr =
                YY_CAST(union yyalloc*, YYSTACK_ALLOC(YY_CAST(YYSIZE_T, YYSTACK_BYTES(yystacksize))));
            if (!yyptr)
                YYNOMEM;
            YYSTACK_RELOCATE(yyss_alloc, yyss);
            YYSTACK_RELOCATE(yyvs_alloc, yyvs);
            YYSTACK_RELOCATE(yyls_alloc, yyls);
#        undef YYSTACK_RELOCATE
            if (yyss1 != yyssa)
                YYSTACK_FREE(yyss1);
        }
#    endif

        yyssp = yyss + yysize - 1;
        yyvsp = yyvs + yysize - 1;
        yylsp = yyls + yysize - 1;

        YY_IGNORE_USELESS_CAST_BEGIN
        YYDPRINTF((stderr, "Stack size increased to %ld\n", YY_CAST(long, yystacksize)));
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
    if (yypact_value_is_default(yyn))
        goto yydefault;

    /* Not known => get a lookahead token if don't already have one.  */

    /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
    if (yychar == YYEMPTY)
    {
        YYDPRINTF((stderr, "Reading a token\n"));
        yychar = yylex(&yylval, &yylloc);
    }

    if (yychar <= YYEOF)
    {
        yychar = YYEOF;
        yytoken = YYSYMBOL_YYEOF;
        YYDPRINTF((stderr, "Now at end of input.\n"));
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
        yytoken = YYTRANSLATE(yychar);
        YY_SYMBOL_PRINT("Next token is", yytoken, &yylval, &yylloc);
    }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
        goto yydefault;
    yyn = yytable[yyn];
    if (yyn <= 0)
    {
        if (yytable_value_is_error(yyn))
            goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
    }

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus)
        yyerrstatus--;

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT("Shifting", yytoken, &yylval, &yylloc);
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
    yyval = yyvsp[1 - yylen];

    /* Default location. */
    YYLLOC_DEFAULT(yyloc, (yylsp - yylen), yylen);
    yyerror_range[1] = yyloc;
    YY_REDUCE_PRINT(yyn);
    switch (yyn)
    {
        case 2: /* start: definitions  */
#line 186 "src/Slice/Grammar.y"
        {
        }
#line 1730 "src/Slice/Grammar.cpp"
        break;

        case 3: /* opt_semicolon: ';'  */
#line 194 "src/Slice/Grammar.y"
        {
        }
#line 1737 "src/Slice/Grammar.cpp"
        break;

        case 4: /* opt_semicolon: %empty  */
#line 197 "src/Slice/Grammar.y"
        {
        }
#line 1744 "src/Slice/Grammar.cpp"
        break;

        case 5: /* global_meta_data: ICE_GLOBAL_METADATA_OPEN string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 205 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[-1];
        }
#line 1752 "src/Slice/Grammar.cpp"
        break;

        case 6: /* global_meta_data: ICE_GLOBAL_METADATA_IGNORE string_list ICE_GLOBAL_METADATA_CLOSE  */
#line 209 "src/Slice/Grammar.y"
        {
            currentUnit->error("global metadata must appear before any definitions");
            yyval = yyvsp[-1]; // Dummy
        }
#line 1761 "src/Slice/Grammar.cpp"
        break;

        case 7: /* meta_data: ICE_METADATA_OPEN string_list ICE_METADATA_CLOSE  */
#line 219 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[-1];
        }
#line 1769 "src/Slice/Grammar.cpp"
        break;

        case 8: /* meta_data: %empty  */
#line 223 "src/Slice/Grammar.y"
        {
            yyval = make_shared<StringListTok>();
        }
#line 1777 "src/Slice/Grammar.cpp"
        break;

        case 9: /* definitions: definitions global_meta_data  */
#line 232 "src/Slice/Grammar.y"
        {
            auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[0]);
            if (!metaData->v.empty())
            {
                currentUnit->addGlobalMetaData(metaData->v);
            }
        }
#line 1789 "src/Slice/Grammar.cpp"
        break;

        case 10: /* definitions: definitions meta_data definition  */
#line 240 "src/Slice/Grammar.y"
        {
            auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-1]);
            auto contained = dynamic_pointer_cast<Contained>(yyvsp[0]);
            if (contained && !metaData->v.empty())
            {
                contained->setMetaData(metaData->v);
            }
        }
#line 1802 "src/Slice/Grammar.cpp"
        break;

        case 11: /* definitions: %empty  */
#line 249 "src/Slice/Grammar.y"
        {
        }
#line 1809 "src/Slice/Grammar.cpp"
        break;

        case 12: /* $@1: %empty  */
#line 257 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Module>(yyvsp[0]));
        }
#line 1817 "src/Slice/Grammar.cpp"
        break;

        case 14: /* $@2: %empty  */
#line 262 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<ClassDecl>(yyvsp[0]));
        }
#line 1825 "src/Slice/Grammar.cpp"
        break;

        case 16: /* definition: class_decl  */
#line 267 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after class forward declaration");
        }
#line 1833 "src/Slice/Grammar.cpp"
        break;

        case 17: /* $@3: %empty  */
#line 271 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<ClassDef>(yyvsp[0]));
        }
#line 1841 "src/Slice/Grammar.cpp"
        break;

        case 19: /* $@4: %empty  */
#line 276 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<InterfaceDecl>(yyvsp[0]));
        }
#line 1849 "src/Slice/Grammar.cpp"
        break;

        case 21: /* definition: interface_decl  */
#line 281 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after interface forward declaration");
        }
#line 1857 "src/Slice/Grammar.cpp"
        break;

        case 22: /* $@5: %empty  */
#line 285 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<InterfaceDef>(yyvsp[0]));
        }
#line 1865 "src/Slice/Grammar.cpp"
        break;

        case 24: /* $@6: %empty  */
#line 290 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr);
        }
#line 1873 "src/Slice/Grammar.cpp"
        break;

        case 26: /* definition: exception_decl  */
#line 295 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after exception forward declaration");
        }
#line 1881 "src/Slice/Grammar.cpp"
        break;

        case 27: /* $@7: %empty  */
#line 299 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Exception>(yyvsp[0]));
        }
#line 1889 "src/Slice/Grammar.cpp"
        break;

        case 29: /* $@8: %empty  */
#line 304 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr);
        }
#line 1897 "src/Slice/Grammar.cpp"
        break;

        case 31: /* definition: struct_decl  */
#line 309 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after struct forward declaration");
        }
#line 1905 "src/Slice/Grammar.cpp"
        break;

        case 32: /* $@9: %empty  */
#line 313 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Struct>(yyvsp[0]));
        }
#line 1913 "src/Slice/Grammar.cpp"
        break;

        case 34: /* $@10: %empty  */
#line 318 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Sequence>(yyvsp[0]));
        }
#line 1921 "src/Slice/Grammar.cpp"
        break;

        case 36: /* definition: sequence_def  */
#line 323 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after sequence definition");
        }
#line 1929 "src/Slice/Grammar.cpp"
        break;

        case 37: /* $@11: %empty  */
#line 327 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Dictionary>(yyvsp[0]));
        }
#line 1937 "src/Slice/Grammar.cpp"
        break;

        case 39: /* definition: dictionary_def  */
#line 332 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after dictionary definition");
        }
#line 1945 "src/Slice/Grammar.cpp"
        break;

        case 40: /* $@12: %empty  */
#line 336 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Enum>(yyvsp[0]));
        }
#line 1953 "src/Slice/Grammar.cpp"
        break;

        case 42: /* $@13: %empty  */
#line 341 "src/Slice/Grammar.y"
        {
            assert(yyvsp[0] == nullptr || dynamic_pointer_cast<Const>(yyvsp[0]));
        }
#line 1961 "src/Slice/Grammar.cpp"
        break;

        case 44: /* definition: const_def  */
#line 346 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after const definition");
        }
#line 1969 "src/Slice/Grammar.cpp"
        break;

        case 45: /* definition: error ';'  */
#line 350 "src/Slice/Grammar.y"
        {
            yyerrok;
        }
#line 1977 "src/Slice/Grammar.cpp"
        break;

        case 46: /* @14: %empty  */
#line 359 "src/Slice/Grammar.y"
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
#line 1997 "src/Slice/Grammar.cpp"
        break;

        case 47: /* module_def: ICE_MODULE ICE_IDENTIFIER @14 '{' definitions '}'  */
#line 375 "src/Slice/Grammar.y"
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
#line 2013 "src/Slice/Grammar.cpp"
        break;

        case 48: /* @15: %empty  */
#line 387 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);

            // Reject scoped identifiers starting with "::". This is generally indicates global scope, but is invalid
            // here.
            size_t startPos = 0;
            if (ident->v.find("::") == 0)
            {
                currentUnit->error("illegal identifier: module identifiers cannot start with '::' prefix");
                startPos += 2; // Skip the leading "::".
            }

            // Split the scoped-identifier token into separate module names.
            size_t endPos;
            vector<string> modules;
            while ((endPos = ident->v.find("::", startPos)) != string::npos)
            {
                modules.push_back(ident->v.substr(startPos, (endPos - startPos)));
                startPos = endPos + 2; // Skip the "::" separator.
            }
            modules.push_back(ident->v.substr(startPos));

            // Create the nested modules.
            ContainerPtr cont = currentUnit->currentContainer();
            for (size_t i = 0; i < modules.size(); i++)
            {
                const auto currentModuleName = modules[i];
                ModulePtr module = cont->createModule(currentModuleName);
                if (module)
                {
                    cont->checkIntroduced(currentModuleName, module);
                    currentUnit->pushContainer(module);
                    yyval = cont = module;
                }
                else
                {
                    // If an error occurs while creating one of the modules, we have to stop. But, to eagerly report as
                    // many errors as possible, we still 'create' any remaining modules, which will run _some_
                    // validation on them.
                    for (size_t j = (i + 1); j < modules.size(); j++)
                    {
                        cont->createModule(modules[j]); // Dummy
                    }

                    // Then we roll back the chain, ie. pop the successfully-created-modules off the container stack.
                    for (; i > 0; i--)
                    {
                        currentUnit->popContainer();
                    }
                    yyval = nullptr;
                    break;
                }
            }
        }
#line 2070 "src/Slice/Grammar.cpp"
        break;

        case 49: /* module_def: ICE_MODULE ICE_SCOPED_IDENTIFIER @15 '{' definitions '}'  */
#line 440 "src/Slice/Grammar.y"
        {
            if (yyvsp[-3])
            {
                // We need to pop '(N+1)' modules off the container stack, to navigate out of the nested module.
                // Where `N` is the number of scope separators ("::").
                size_t startPos = 0;
                auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-4]);
                while ((startPos = ident->v.find("::", startPos)) != string::npos)
                {
                    currentUnit->popContainer();
                    startPos += 2; // Skip the "::" separator.
                }

                // Set the 'return value' to the outer-most module, before we pop it off the stack.
                // Whichever module we return, is the one that metadata will be applied to.
                yyval = currentUnit->currentContainer();
                currentUnit->popContainer();
            }
            else
            {
                yyval = nullptr;
            }
        }
#line 2098 "src/Slice/Grammar.cpp"
        break;

        case 50: /* exception_id: ICE_EXCEPTION ICE_IDENTIFIER  */
#line 469 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[0];
        }
#line 2106 "src/Slice/Grammar.cpp"
        break;

        case 51: /* exception_id: ICE_EXCEPTION keyword  */
#line 473 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            currentUnit->error("keyword `" + ident->v + "' cannot be used as exception name");
            yyval = yyvsp[0]; // Dummy
        }
#line 2116 "src/Slice/Grammar.cpp"
        break;

        case 52: /* exception_decl: exception_id  */
#line 484 "src/Slice/Grammar.y"
        {
            currentUnit->error("exceptions cannot be forward declared");
            yyval = nullptr;
        }
#line 2125 "src/Slice/Grammar.cpp"
        break;

        case 53: /* @16: %empty  */
#line 494 "src/Slice/Grammar.y"
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
#line 2142 "src/Slice/Grammar.cpp"
        break;

        case 54: /* exception_def: exception_id exception_extends @16 '{' data_members '}'  */
#line 507 "src/Slice/Grammar.y"
        {
            if (yyvsp[-3])
            {
                currentUnit->popContainer();
            }
            yyval = yyvsp[-3];
        }
#line 2154 "src/Slice/Grammar.cpp"
        break;

        case 55: /* exception_extends: extends scoped_name  */
#line 520 "src/Slice/Grammar.y"
        {
            auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            ContainerPtr cont = currentUnit->currentContainer();
            ContainedPtr contained = cont->lookupException(scoped->v);
            cont->checkIntroduced(scoped->v);
            yyval = contained;
        }
#line 2166 "src/Slice/Grammar.cpp"
        break;

        case 56: /* exception_extends: %empty  */
#line 528 "src/Slice/Grammar.y"
        {
            yyval = nullptr;
        }
#line 2174 "src/Slice/Grammar.cpp"
        break;

        case 57: /* type_id: type ICE_IDENTIFIER  */
#line 537 "src/Slice/Grammar.y"
        {
            auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto typestring = make_shared<TypeStringTok>();
            typestring->v = make_pair(type, ident->v);
            yyval = typestring;
        }
#line 2186 "src/Slice/Grammar.cpp"
        break;

        case 58: /* optional: ICE_OPTIONAL_OPEN ICE_INTEGER_LITERAL ')'  */
#line 550 "src/Slice/Grammar.y"
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

            auto m = make_shared<OptionalDefTok>(tag);
            yyval = m;
        }
#line 2208 "src/Slice/Grammar.cpp"
        break;

        case 59: /* optional: ICE_OPTIONAL_OPEN scoped_name ')'  */
#line 568 "src/Slice/Grammar.y"
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

            auto m = make_shared<OptionalDefTok>(tag);
            yyval = m;
        }
#line 2288 "src/Slice/Grammar.cpp"
        break;

        case 60: /* optional: ICE_OPTIONAL_OPEN ')'  */
#line 644 "src/Slice/Grammar.y"
        {
            currentUnit->error("missing tag");
            auto m = make_shared<OptionalDefTok>(-1); // Dummy
            yyval = m;
        }
#line 2298 "src/Slice/Grammar.cpp"
        break;

        case 61: /* optional: ICE_OPTIONAL  */
#line 650 "src/Slice/Grammar.y"
        {
            currentUnit->error("missing tag");
            auto m = make_shared<OptionalDefTok>(-1); // Dummy
            yyval = m;
        }
#line 2308 "src/Slice/Grammar.cpp"
        break;

        case 62: /* optional_type_id: optional type_id  */
#line 661 "src/Slice/Grammar.y"
        {
            auto m = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
            auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);
            m->type = ts->v.first;
            m->name = ts->v.second;

            // It's safe to perform this check in the parser, since we already have enough information to know whether a
            // type can be optional. This is because the only types that can be forward declared (classes/interfaces)
            // have constant values for `usesClasses` (true/false respectively).
            if (m->type->usesClasses())
            {
                currentUnit->error("types that use classes cannot be marked with 'optional'");
            }

            yyval = m;
        }
#line 2329 "src/Slice/Grammar.cpp"
        break;

        case 63: /* optional_type_id: type_id  */
#line 678 "src/Slice/Grammar.y"
        {
            auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);
            auto m = make_shared<OptionalDefTok>(-1);
            m->type = ts->v.first;
            m->name = ts->v.second;
            yyval = m;
        }
#line 2341 "src/Slice/Grammar.cpp"
        break;

        case 64: /* struct_id: ICE_STRUCT ICE_IDENTIFIER  */
#line 691 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[0];
        }
#line 2349 "src/Slice/Grammar.cpp"
        break;

        case 65: /* struct_id: ICE_STRUCT keyword  */
#line 695 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            currentUnit->error("keyword `" + ident->v + "' cannot be used as struct name");
            yyval = yyvsp[0]; // Dummy
        }
#line 2359 "src/Slice/Grammar.cpp"
        break;

        case 66: /* struct_decl: struct_id  */
#line 706 "src/Slice/Grammar.y"
        {
            currentUnit->error("structs cannot be forward declared");
            yyval = nullptr; // Dummy
        }
#line 2368 "src/Slice/Grammar.cpp"
        break;

        case 67: /* @17: %empty  */
#line 716 "src/Slice/Grammar.y"
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
                st = cont->createStruct(Ice::generateUUID()); // Dummy
                assert(st);
                currentUnit->pushContainer(st);
            }
            yyval = st;
        }
#line 2390 "src/Slice/Grammar.cpp"
        break;

        case 68: /* struct_def: struct_id @17 '{' data_members '}'  */
#line 734 "src/Slice/Grammar.y"
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
#line 2410 "src/Slice/Grammar.cpp"
        break;

        case 69: /* class_name: ICE_CLASS ICE_IDENTIFIER  */
#line 755 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[0];
        }
#line 2418 "src/Slice/Grammar.cpp"
        break;

        case 70: /* class_name: ICE_CLASS keyword  */
#line 759 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            currentUnit->error("keyword `" + ident->v + "' cannot be used as class name");
            yyval = yyvsp[0]; // Dummy
        }
#line 2428 "src/Slice/Grammar.cpp"
        break;

        case 71: /* class_id: ICE_CLASS ICE_IDENT_OPEN ICE_INTEGER_LITERAL ')'  */
#line 770 "src/Slice/Grammar.y"
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
#line 2457 "src/Slice/Grammar.cpp"
        break;

        case 72: /* class_id: ICE_CLASS ICE_IDENT_OPEN scoped_name ')'  */
#line 795 "src/Slice/Grammar.y"
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
#line 2549 "src/Slice/Grammar.cpp"
        break;

        case 73: /* class_id: class_name  */
#line 883 "src/Slice/Grammar.y"
        {
            auto classId = make_shared<ClassIdTok>();
            classId->v = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
            classId->t = -1;
            yyval = classId;
        }
#line 2560 "src/Slice/Grammar.cpp"
        break;

        case 74: /* class_decl: class_name  */
#line 895 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            ContainerPtr cont = currentUnit->currentContainer();
            ClassDeclPtr cl = cont->createClassDecl(ident->v);
            yyval = cl;
        }
#line 2571 "src/Slice/Grammar.cpp"
        break;

        case 75: /* @18: %empty  */
#line 907 "src/Slice/Grammar.y"
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
#line 2592 "src/Slice/Grammar.cpp"
        break;

        case 76: /* class_def: class_id class_extends @18 '{' data_members '}'  */
#line 924 "src/Slice/Grammar.y"
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
#line 2608 "src/Slice/Grammar.cpp"
        break;

        case 77: /* class_extends: extends scoped_name  */
#line 941 "src/Slice/Grammar.y"
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
#line 2646 "src/Slice/Grammar.cpp"
        break;

        case 78: /* class_extends: %empty  */
#line 975 "src/Slice/Grammar.y"
        {
            yyval = nullptr;
        }
#line 2654 "src/Slice/Grammar.cpp"
        break;

        case 79: /* extends: ICE_EXTENDS  */
#line 984 "src/Slice/Grammar.y"
        {
        }
#line 2661 "src/Slice/Grammar.cpp"
        break;

        case 80: /* extends: ':'  */
#line 987 "src/Slice/Grammar.y"
        {
        }
#line 2668 "src/Slice/Grammar.cpp"
        break;

        case 81: /* data_members: meta_data data_member ';' data_members  */
#line 995 "src/Slice/Grammar.y"
        {
            auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
            auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
            if (contained && !metaData->v.empty())
            {
                contained->setMetaData(metaData->v);
            }
        }
#line 2681 "src/Slice/Grammar.cpp"
        break;

        case 82: /* data_members: error ';' data_members  */
#line 1004 "src/Slice/Grammar.y"
        {
        }
#line 2688 "src/Slice/Grammar.cpp"
        break;

        case 83: /* data_members: meta_data data_member  */
#line 1007 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after definition");
        }
#line 2696 "src/Slice/Grammar.cpp"
        break;

        case 84: /* data_members: %empty  */
#line 1011 "src/Slice/Grammar.y"
        {
        }
#line 2703 "src/Slice/Grammar.cpp"
        break;

        case 85: /* data_member: optional_type_id  */
#line 1019 "src/Slice/Grammar.y"
        {
            auto def = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
            auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
            DataMemberPtr dm;
            if (cl)
            {
                dm = cl->createDataMember(def->name, def->type, def->isOptional, def->tag, 0, "", "");
            }
            auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
            if (st)
            {
                if (def->isOptional)
                {
                    currentUnit->error("optional data members are not supported in structs");
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
                dm = ex->createDataMember(def->name, def->type, def->isOptional, def->tag, 0, "", "");
            }
            currentUnit->currentContainer()->checkIntroduced(def->name, dm);
            yyval = dm;
        }
#line 2737 "src/Slice/Grammar.cpp"
        break;

        case 86: /* data_member: optional_type_id '=' const_initializer  */
#line 1049 "src/Slice/Grammar.y"
        {
            auto def = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-2]);
            auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
            auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
            DataMemberPtr dm;
            if (cl)
            {
                dm = cl->createDataMember(
                    def->name,
                    def->type,
                    def->isOptional,
                    def->tag,
                    value->v,
                    value->valueAsString,
                    value->valueAsLiteral);
            }
            auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
            if (st)
            {
                if (def->isOptional)
                {
                    currentUnit->error("optional data members are not supported in structs");
                    dm = st->createDataMember(def->name, def->type, false, 0, 0, "", ""); // Dummy
                }
                else
                {
                    dm = st->createDataMember(
                        def->name,
                        def->type,
                        false,
                        -1,
                        value->v,
                        value->valueAsString,
                        value->valueAsLiteral);
                }
            }
            auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
            if (ex)
            {
                dm = ex->createDataMember(
                    def->name,
                    def->type,
                    def->isOptional,
                    def->tag,
                    value->v,
                    value->valueAsString,
                    value->valueAsLiteral);
            }
            currentUnit->currentContainer()->checkIntroduced(def->name, dm);
            yyval = dm;
        }
#line 2775 "src/Slice/Grammar.cpp"
        break;

        case 87: /* data_member: type keyword  */
#line 1083 "src/Slice/Grammar.y"
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
#line 2801 "src/Slice/Grammar.cpp"
        break;

        case 88: /* data_member: type  */
#line 1105 "src/Slice/Grammar.y"
        {
            auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
            auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer());
            if (cl)
            {
                yyval = cl->createDataMember(Ice::generateUUID(), type, false, 0, 0, "", ""); // Dummy
            }
            auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer());
            if (st)
            {
                yyval = st->createDataMember(Ice::generateUUID(), type, false, 0, 0, "", ""); // Dummy
            }
            auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer());
            if (ex)
            {
                yyval = ex->createDataMember(Ice::generateUUID(), type, false, 0, 0, "", ""); // Dummy
            }
            assert(yyval);
            currentUnit->error("missing data member name");
        }
#line 2826 "src/Slice/Grammar.cpp"
        break;

        case 89: /* return_type: optional type  */
#line 1131 "src/Slice/Grammar.y"
        {
            auto m = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
            m->type = dynamic_pointer_cast<Type>(yyvsp[0]);

            // It's safe to perform this check in the parser, since we already have enough information to know whether a
            // type can be optional. This is because the only types that can be forward declared (classes/interfaces)
            // have constant values for `usesClasses` (true/false respectively).
            if (m->type->usesClasses())
            {
                currentUnit->error("types that use classes cannot be marked with 'optional'");
            }

            yyval = m;
        }
#line 2845 "src/Slice/Grammar.cpp"
        break;

        case 90: /* return_type: type  */
#line 1146 "src/Slice/Grammar.y"
        {
            auto m = make_shared<OptionalDefTok>(-1);
            m->type = dynamic_pointer_cast<Type>(yyvsp[0]);
            yyval = m;
        }
#line 2855 "src/Slice/Grammar.cpp"
        break;

        case 91: /* return_type: ICE_VOID  */
#line 1152 "src/Slice/Grammar.y"
        {
            auto m = make_shared<OptionalDefTok>(-1);
            yyval = m;
        }
#line 2864 "src/Slice/Grammar.cpp"
        break;

        case 92: /* operation_preamble: return_type ICE_IDENT_OPEN  */
#line 1162 "src/Slice/Grammar.y"
        {
            auto returnType = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
            string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
            auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
            if (interface)
            {
                OperationPtr op =
                    interface->createOperation(name, returnType->type, returnType->isOptional, returnType->tag);
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
#line 2892 "src/Slice/Grammar.cpp"
        break;

        case 93: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_IDENT_OPEN  */
#line 1186 "src/Slice/Grammar.y"
        {
            auto returnType = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
            string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
            auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
            if (interface)
            {
                OperationPtr op = interface->createOperation(
                    name,
                    returnType->type,
                    returnType->isOptional,
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
#line 2926 "src/Slice/Grammar.cpp"
        break;

        case 94: /* operation_preamble: return_type ICE_KEYWORD_OPEN  */
#line 1216 "src/Slice/Grammar.y"
        {
            auto returnType = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
            string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
            auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
            if (interface)
            {
                OperationPtr op =
                    interface->createOperation(name, returnType->type, returnType->isOptional, returnType->tag);
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
#line 2954 "src/Slice/Grammar.cpp"
        break;

        case 95: /* operation_preamble: ICE_IDEMPOTENT return_type ICE_KEYWORD_OPEN  */
#line 1240 "src/Slice/Grammar.y"
        {
            auto returnType = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
            string name = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
            auto interface = dynamic_pointer_cast<InterfaceDef>(currentUnit->currentContainer());
            if (interface)
            {
                OperationPtr op = interface->createOperation(
                    name,
                    returnType->type,
                    returnType->isOptional,
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
#line 2987 "src/Slice/Grammar.cpp"
        break;

        case 96: /* @19: %empty  */
#line 1274 "src/Slice/Grammar.y"
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
#line 3003 "src/Slice/Grammar.cpp"
        break;

        case 97: /* operation: operation_preamble parameters ')' @19 throws  */
#line 1286 "src/Slice/Grammar.y"
        {
            auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
            auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
            assert(el);
            if (op)
            {
                op->setExceptionList(el->v);
            }
        }
#line 3017 "src/Slice/Grammar.cpp"
        break;

        case 98: /* @20: %empty  */
#line 1296 "src/Slice/Grammar.y"
        {
            if (yyvsp[-2])
            {
                currentUnit->popContainer();
            }
            yyerrok;
        }
#line 3029 "src/Slice/Grammar.cpp"
        break;

        case 99: /* operation: operation_preamble error ')' @20 throws  */
#line 1304 "src/Slice/Grammar.y"
        {
            auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
            auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
            assert(el);
            if (op)
            {
                op->setExceptionList(el->v); // Dummy
            }
        }
#line 3043 "src/Slice/Grammar.cpp"
        break;

        case 100: /* interface_id: ICE_INTERFACE ICE_IDENTIFIER  */
#line 1319 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[0];
        }
#line 3051 "src/Slice/Grammar.cpp"
        break;

        case 101: /* interface_id: ICE_INTERFACE keyword  */
#line 1323 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            currentUnit->error("keyword `" + ident->v + "' cannot be used as interface name");
            yyval = yyvsp[0]; // Dummy
        }
#line 3061 "src/Slice/Grammar.cpp"
        break;

        case 102: /* interface_decl: interface_id  */
#line 1334 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto cont = currentUnit->currentContainer();
            InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
            cont->checkIntroduced(ident->v, cl);
            yyval = cl;
        }
#line 3073 "src/Slice/Grammar.cpp"
        break;

        case 103: /* @21: %empty  */
#line 1347 "src/Slice/Grammar.y"
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
#line 3094 "src/Slice/Grammar.cpp"
        break;

        case 104: /* interface_def: interface_id interface_extends @21 '{' operations '}'  */
#line 1364 "src/Slice/Grammar.y"
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
#line 3110 "src/Slice/Grammar.cpp"
        break;

        case 105: /* interface_list: scoped_name ',' interface_list  */
#line 1381 "src/Slice/Grammar.y"
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
#line 3149 "src/Slice/Grammar.cpp"
        break;

        case 106: /* interface_list: scoped_name  */
#line 1416 "src/Slice/Grammar.y"
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
#line 3188 "src/Slice/Grammar.cpp"
        break;

        case 107: /* interface_list: ICE_OBJECT  */
#line 1451 "src/Slice/Grammar.y"
        {
            currentUnit->error("illegal inheritance from type Object");
            yyval = make_shared<InterfaceListTok>(); // Dummy
        }
#line 3197 "src/Slice/Grammar.cpp"
        break;

        case 108: /* interface_list: ICE_VALUE  */
#line 1456 "src/Slice/Grammar.y"
        {
            currentUnit->error("illegal inheritance from type Value");
            yyval = make_shared<ClassListTok>(); // Dummy
        }
#line 3206 "src/Slice/Grammar.cpp"
        break;

        case 109: /* interface_extends: extends interface_list  */
#line 1466 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[0];
        }
#line 3214 "src/Slice/Grammar.cpp"
        break;

        case 110: /* interface_extends: %empty  */
#line 1470 "src/Slice/Grammar.y"
        {
            yyval = make_shared<InterfaceListTok>();
        }
#line 3222 "src/Slice/Grammar.cpp"
        break;

        case 111: /* operations: meta_data operation ';' operations  */
#line 1479 "src/Slice/Grammar.y"
        {
            auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
            auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
            if (contained && !metaData->v.empty())
            {
                contained->setMetaData(metaData->v);
            }
        }
#line 3235 "src/Slice/Grammar.cpp"
        break;

        case 112: /* operations: error ';' operations  */
#line 1488 "src/Slice/Grammar.y"
        {
        }
#line 3242 "src/Slice/Grammar.cpp"
        break;

        case 113: /* operations: meta_data operation  */
#line 1491 "src/Slice/Grammar.y"
        {
            currentUnit->error("`;' missing after definition");
        }
#line 3250 "src/Slice/Grammar.cpp"
        break;

        case 114: /* operations: %empty  */
#line 1495 "src/Slice/Grammar.y"
        {
        }
#line 3257 "src/Slice/Grammar.cpp"
        break;

        case 115: /* exception_list: exception ',' exception_list  */
#line 1503 "src/Slice/Grammar.y"
        {
            auto exception = dynamic_pointer_cast<Exception>(yyvsp[-2]);
            auto exceptionList = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
            exceptionList->v.push_front(exception);
            yyval = exceptionList;
        }
#line 3268 "src/Slice/Grammar.cpp"
        break;

        case 116: /* exception_list: exception  */
#line 1510 "src/Slice/Grammar.y"
        {
            auto exception = dynamic_pointer_cast<Exception>(yyvsp[0]);
            auto exceptionList = make_shared<ExceptionListTok>();
            exceptionList->v.push_front(exception);
            yyval = exceptionList;
        }
#line 3279 "src/Slice/Grammar.cpp"
        break;

        case 117: /* exception: scoped_name  */
#line 1522 "src/Slice/Grammar.y"
        {
            auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            ContainerPtr cont = currentUnit->currentContainer();
            ExceptionPtr exception = cont->lookupException(scoped->v);
            if (!exception)
            {
                exception = cont->createException(Ice::generateUUID(), 0, Dummy); // Dummy
            }
            cont->checkIntroduced(scoped->v, exception);
            yyval = exception;
        }
#line 3295 "src/Slice/Grammar.cpp"
        break;

        case 118: /* exception: keyword  */
#line 1534 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            currentUnit->error("keyword `" + ident->v + "' cannot be used as exception name");
            yyval = currentUnit->currentContainer()->createException(Ice::generateUUID(), 0, Dummy); // Dummy
        }
#line 3305 "src/Slice/Grammar.cpp"
        break;

        case 119: /* sequence_def: ICE_SEQUENCE '<' meta_data type '>' ICE_IDENTIFIER  */
#line 1545 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
            auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
            ContainerPtr cont = currentUnit->currentContainer();
            yyval = cont->createSequence(ident->v, type, metaData->v);
        }
#line 3317 "src/Slice/Grammar.cpp"
        break;

        case 120: /* sequence_def: ICE_SEQUENCE '<' meta_data type '>' keyword  */
#line 1553 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
            auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
            ContainerPtr cont = currentUnit->currentContainer();
            yyval = cont->createSequence(ident->v, type, metaData->v); // Dummy
            currentUnit->error("keyword `" + ident->v + "' cannot be used as sequence name");
        }
#line 3330 "src/Slice/Grammar.cpp"
        break;

        case 121: /* dictionary_def: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' ICE_IDENTIFIER  */
#line 1567 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto keyMetaData = dynamic_pointer_cast<StringListTok>(yyvsp[-6]);
            auto keyType = dynamic_pointer_cast<Type>(yyvsp[-5]);
            auto valueMetaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
            auto valueType = dynamic_pointer_cast<Type>(yyvsp[-2]);
            ContainerPtr cont = currentUnit->currentContainer();
            yyval = cont->createDictionary(ident->v, keyType, keyMetaData->v, valueType, valueMetaData->v);
        }
#line 3344 "src/Slice/Grammar.cpp"
        break;

        case 122: /* dictionary_def: ICE_DICTIONARY '<' meta_data type ',' meta_data type '>' keyword  */
#line 1577 "src/Slice/Grammar.y"
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
#line 3359 "src/Slice/Grammar.cpp"
        break;

        case 123: /* enum_id: ICE_ENUM ICE_IDENTIFIER  */
#line 1593 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[0];
        }
#line 3367 "src/Slice/Grammar.cpp"
        break;

        case 124: /* enum_id: ICE_ENUM keyword  */
#line 1597 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            currentUnit->error("keyword `" + ident->v + "' cannot be used as enumeration name");
            yyval = yyvsp[0]; // Dummy
        }
#line 3377 "src/Slice/Grammar.cpp"
        break;

        case 125: /* @22: %empty  */
#line 1608 "src/Slice/Grammar.y"
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
                en = cont->createEnum(Ice::generateUUID(), Dummy);
            }
            currentUnit->pushContainer(en);
            yyval = en;
        }
#line 3397 "src/Slice/Grammar.cpp"
        break;

        case 126: /* enum_def: enum_id @22 '{' enumerator_list '}'  */
#line 1624 "src/Slice/Grammar.y"
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
#line 3415 "src/Slice/Grammar.cpp"
        break;

        case 127: /* @23: %empty  */
#line 1639 "src/Slice/Grammar.y"
        {
            currentUnit->error("missing enumeration name");
            ContainerPtr cont = currentUnit->currentContainer();
            EnumPtr en = cont->createEnum(Ice::generateUUID(), Dummy);
            currentUnit->pushContainer(en);
            yyval = en;
        }
#line 3427 "src/Slice/Grammar.cpp"
        break;

        case 128: /* enum_def: ICE_ENUM @23 '{' enumerator_list '}'  */
#line 1647 "src/Slice/Grammar.y"
        {
            currentUnit->popContainer();
            yyval = yyvsp[-4];
        }
#line 3436 "src/Slice/Grammar.cpp"
        break;

        case 129: /* enumerator_list: enumerator ',' enumerator_list  */
#line 1657 "src/Slice/Grammar.y"
        {
            auto ens = dynamic_pointer_cast<EnumeratorListTok>(yyvsp[-2]);
            ens->v.splice(ens->v.end(), dynamic_pointer_cast<EnumeratorListTok>(yyvsp[0])->v);
            yyval = ens;
        }
#line 3446 "src/Slice/Grammar.cpp"
        break;

        case 130: /* enumerator_list: enumerator  */
#line 1663 "src/Slice/Grammar.y"
        {
        }
#line 3453 "src/Slice/Grammar.cpp"
        break;

        case 131: /* enumerator: ICE_IDENTIFIER  */
#line 1671 "src/Slice/Grammar.y"
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
#line 3469 "src/Slice/Grammar.cpp"
        break;

        case 132: /* enumerator: ICE_IDENTIFIER '=' enumerator_initializer  */
#line 1683 "src/Slice/Grammar.y"
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
#line 3493 "src/Slice/Grammar.cpp"
        break;

        case 133: /* enumerator: keyword  */
#line 1703 "src/Slice/Grammar.y"
        {
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            currentUnit->error("keyword `" + ident->v + "' cannot be used as enumerator");
            auto ens = make_shared<EnumeratorListTok>(); // Dummy
            yyval = ens;
        }
#line 3504 "src/Slice/Grammar.cpp"
        break;

        case 134: /* enumerator: %empty  */
#line 1710 "src/Slice/Grammar.y"
        {
            auto ens = make_shared<EnumeratorListTok>();
            yyval = ens; // Dummy
        }
#line 3513 "src/Slice/Grammar.cpp"
        break;

        case 135: /* enumerator_initializer: ICE_INTEGER_LITERAL  */
#line 1720 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[0];
        }
#line 3521 "src/Slice/Grammar.cpp"
        break;

        case 136: /* enumerator_initializer: scoped_name  */
#line 1724 "src/Slice/Grammar.y"
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
#line 3561 "src/Slice/Grammar.cpp"
        break;

        case 137: /* out_qualifier: ICE_OUT  */
#line 1765 "src/Slice/Grammar.y"
        {
            auto out = make_shared<BoolTok>();
            out->v = true;
            yyval = out;
        }
#line 3571 "src/Slice/Grammar.cpp"
        break;

        case 138: /* out_qualifier: %empty  */
#line 1771 "src/Slice/Grammar.y"
        {
            auto out = make_shared<BoolTok>();
            out->v = false;
            yyval = out;
        }
#line 3581 "src/Slice/Grammar.cpp"
        break;

        case 139: /* parameters: %empty  */
#line 1782 "src/Slice/Grammar.y"
        {
        }
#line 3588 "src/Slice/Grammar.cpp"
        break;

        case 140: /* parameters: out_qualifier meta_data optional_type_id  */
#line 1785 "src/Slice/Grammar.y"
        {
            auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
            auto tsp = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
            auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
            if (op)
            {
                ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isOptional, tsp->tag);
                currentUnit->currentContainer()->checkIntroduced(tsp->name, pd);
                auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-1]);
                if (!metaData->v.empty())
                {
                    pd->setMetaData(metaData->v);
                }
            }
        }
#line 3608 "src/Slice/Grammar.cpp"
        break;

        case 141: /* parameters: parameters ',' out_qualifier meta_data optional_type_id  */
#line 1801 "src/Slice/Grammar.y"
        {
            auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
            auto tsp = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
            auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
            if (op)
            {
                ParamDeclPtr pd = op->createParamDecl(tsp->name, tsp->type, isOutParam->v, tsp->isOptional, tsp->tag);
                currentUnit->currentContainer()->checkIntroduced(tsp->name, pd);
                auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-1]);
                if (!metaData->v.empty())
                {
                    pd->setMetaData(metaData->v);
                }
            }
        }
#line 3628 "src/Slice/Grammar.cpp"
        break;

        case 142: /* parameters: out_qualifier meta_data type keyword  */
#line 1817 "src/Slice/Grammar.y"
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
#line 3644 "src/Slice/Grammar.cpp"
        break;

        case 143: /* parameters: parameters ',' out_qualifier meta_data type keyword  */
#line 1829 "src/Slice/Grammar.y"
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
#line 3660 "src/Slice/Grammar.cpp"
        break;

        case 144: /* parameters: out_qualifier meta_data type  */
#line 1841 "src/Slice/Grammar.y"
        {
            auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
            auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
            auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
            if (op)
            {
                op->createParamDecl(Ice::generateUUID(), type, isOutParam->v, false, 0); // Dummy
                currentUnit->error("missing parameter name");
            }
        }
#line 3675 "src/Slice/Grammar.cpp"
        break;

        case 145: /* parameters: parameters ',' out_qualifier meta_data type  */
#line 1852 "src/Slice/Grammar.y"
        {
            auto isOutParam = dynamic_pointer_cast<BoolTok>(yyvsp[-2]);
            auto type = dynamic_pointer_cast<Type>(yyvsp[0]);
            auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
            if (op)
            {
                op->createParamDecl(Ice::generateUUID(), type, isOutParam->v, false, 0); // Dummy
                currentUnit->error("missing parameter name");
            }
        }
#line 3690 "src/Slice/Grammar.cpp"
        break;

        case 146: /* throws: ICE_THROWS exception_list  */
#line 1868 "src/Slice/Grammar.y"
        {
            yyval = yyvsp[0];
        }
#line 3698 "src/Slice/Grammar.cpp"
        break;

        case 147: /* throws: %empty  */
#line 1872 "src/Slice/Grammar.y"
        {
            yyval = make_shared<ExceptionListTok>();
        }
#line 3706 "src/Slice/Grammar.cpp"
        break;

        case 148: /* scoped_name: ICE_IDENTIFIER  */
#line 1881 "src/Slice/Grammar.y"
        {
        }
#line 3713 "src/Slice/Grammar.cpp"
        break;

        case 149: /* scoped_name: ICE_SCOPED_IDENTIFIER  */
#line 1884 "src/Slice/Grammar.y"
        {
        }
#line 3720 "src/Slice/Grammar.cpp"
        break;

        case 150: /* builtin: ICE_BOOL  */
#line 1891 "src/Slice/Grammar.y"
        {
        }
#line 3726 "src/Slice/Grammar.cpp"
        break;

        case 151: /* builtin: ICE_BYTE  */
#line 1892 "src/Slice/Grammar.y"
        {
        }
#line 3732 "src/Slice/Grammar.cpp"
        break;

        case 152: /* builtin: ICE_SHORT  */
#line 1893 "src/Slice/Grammar.y"
        {
        }
#line 3738 "src/Slice/Grammar.cpp"
        break;

        case 153: /* builtin: ICE_INT  */
#line 1894 "src/Slice/Grammar.y"
        {
        }
#line 3744 "src/Slice/Grammar.cpp"
        break;

        case 154: /* builtin: ICE_LONG  */
#line 1895 "src/Slice/Grammar.y"
        {
        }
#line 3750 "src/Slice/Grammar.cpp"
        break;

        case 155: /* builtin: ICE_FLOAT  */
#line 1896 "src/Slice/Grammar.y"
        {
        }
#line 3756 "src/Slice/Grammar.cpp"
        break;

        case 156: /* builtin: ICE_DOUBLE  */
#line 1897 "src/Slice/Grammar.y"
        {
        }
#line 3762 "src/Slice/Grammar.cpp"
        break;

        case 157: /* builtin: ICE_STRING  */
#line 1898 "src/Slice/Grammar.y"
        {
        }
#line 3768 "src/Slice/Grammar.cpp"
        break;

        case 158: /* builtin: ICE_OBJECT  */
#line 1899 "src/Slice/Grammar.y"
        {
        }
#line 3774 "src/Slice/Grammar.cpp"
        break;

        case 159: /* builtin: ICE_VALUE  */
#line 1900 "src/Slice/Grammar.y"
        {
        }
#line 3780 "src/Slice/Grammar.cpp"
        break;

        case 160: /* type: ICE_OBJECT '*'  */
#line 1906 "src/Slice/Grammar.y"
        {
            yyval = currentUnit->builtin(Builtin::KindObjectProxy);
        }
#line 3788 "src/Slice/Grammar.cpp"
        break;

        case 161: /* type: builtin  */
#line 1910 "src/Slice/Grammar.y"
        {
            auto typeName = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            yyval = currentUnit->builtin(Builtin::kindFromString(typeName->v).value());
        }
#line 3797 "src/Slice/Grammar.cpp"
        break;

        case 162: /* type: scoped_name  */
#line 1915 "src/Slice/Grammar.y"
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
#line 3832 "src/Slice/Grammar.cpp"
        break;

        case 163: /* type: scoped_name '*'  */
#line 1946 "src/Slice/Grammar.y"
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
#line 3867 "src/Slice/Grammar.cpp"
        break;

        case 164: /* string_literal: ICE_STRING_LITERAL string_literal  */
#line 1982 "src/Slice/Grammar.y"
        {
            auto str1 = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
            auto str2 = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            str1->v += str2->v;
        }
#line 3877 "src/Slice/Grammar.cpp"
        break;

        case 165: /* string_literal: ICE_STRING_LITERAL  */
#line 1988 "src/Slice/Grammar.y"
        {
        }
#line 3884 "src/Slice/Grammar.cpp"
        break;

        case 166: /* string_list: string_list ',' string_literal  */
#line 1996 "src/Slice/Grammar.y"
        {
            auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto stringList = dynamic_pointer_cast<StringListTok>(yyvsp[-2]);
            stringList->v.push_back(str->v);
            yyval = stringList;
        }
#line 3895 "src/Slice/Grammar.cpp"
        break;

        case 167: /* string_list: string_literal  */
#line 2003 "src/Slice/Grammar.y"
        {
            auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto stringList = make_shared<StringListTok>();
            stringList->v.push_back(str->v);
            yyval = stringList;
        }
#line 3906 "src/Slice/Grammar.cpp"
        break;

        case 168: /* const_initializer: ICE_INTEGER_LITERAL  */
#line 2015 "src/Slice/Grammar.y"
        {
            BuiltinPtr type = currentUnit->builtin(Builtin::KindLong);
            auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
            ostringstream sstr;
            sstr << intVal->v;
            auto def = make_shared<ConstDefTok>(type, sstr.str(), intVal->literal);
            yyval = def;
        }
#line 3919 "src/Slice/Grammar.cpp"
        break;

        case 169: /* const_initializer: ICE_FLOATING_POINT_LITERAL  */
#line 2024 "src/Slice/Grammar.y"
        {
            BuiltinPtr type = currentUnit->builtin(Builtin::KindDouble);
            auto floatVal = dynamic_pointer_cast<FloatingTok>(yyvsp[0]);
            ostringstream sstr;
            sstr << floatVal->v;
            auto def = make_shared<ConstDefTok>(type, sstr.str(), floatVal->literal);
            yyval = def;
        }
#line 3932 "src/Slice/Grammar.cpp"
        break;

        case 170: /* const_initializer: scoped_name  */
#line 2033 "src/Slice/Grammar.y"
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
#line 3971 "src/Slice/Grammar.cpp"
        break;

        case 171: /* const_initializer: ICE_STRING_LITERAL  */
#line 2068 "src/Slice/Grammar.y"
        {
            BuiltinPtr type = currentUnit->builtin(Builtin::KindString);
            auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto def = make_shared<ConstDefTok>(type, literal->v, literal->literal);
            yyval = def;
        }
#line 3982 "src/Slice/Grammar.cpp"
        break;

        case 172: /* const_initializer: ICE_FALSE  */
#line 2075 "src/Slice/Grammar.y"
        {
            BuiltinPtr type = currentUnit->builtin(Builtin::KindBool);
            auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto def = make_shared<ConstDefTok>(type, "false", "false");
            yyval = def;
        }
#line 3993 "src/Slice/Grammar.cpp"
        break;

        case 173: /* const_initializer: ICE_TRUE  */
#line 2082 "src/Slice/Grammar.y"
        {
            BuiltinPtr type = currentUnit->builtin(Builtin::KindBool);
            auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
            auto def = make_shared<ConstDefTok>(type, "true", "true");
            yyval = def;
        }
#line 4004 "src/Slice/Grammar.cpp"
        break;

        case 174: /* const_def: ICE_CONST meta_data type ICE_IDENTIFIER '=' const_initializer  */
#line 2094 "src/Slice/Grammar.y"
        {
            auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-4]);
            auto const_type = dynamic_pointer_cast<Type>(yyvsp[-3]);
            auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
            auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
            yyval = currentUnit->currentContainer()->createConst(
                ident->v,
                const_type,
                metaData->v,
                value->v,
                value->valueAsString,
                value->valueAsLiteral);
        }
#line 4017 "src/Slice/Grammar.cpp"
        break;

        case 175: /* const_def: ICE_CONST meta_data type '=' const_initializer  */
#line 2103 "src/Slice/Grammar.y"
        {
            auto metaData = dynamic_pointer_cast<StringListTok>(yyvsp[-3]);
            auto const_type = dynamic_pointer_cast<Type>(yyvsp[-2]);
            auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
            currentUnit->error("missing constant name");
            yyval = currentUnit->currentContainer()->createConst(
                Ice::generateUUID(),
                const_type,
                metaData->v,
                value->v,
                value->valueAsString,
                value->valueAsLiteral,
                Dummy); // Dummy
        }
#line 4030 "src/Slice/Grammar.cpp"
        break;

        case 176: /* keyword: ICE_MODULE  */
#line 2116 "src/Slice/Grammar.y"
        {
        }
#line 4036 "src/Slice/Grammar.cpp"
        break;

        case 177: /* keyword: ICE_CLASS  */
#line 2117 "src/Slice/Grammar.y"
        {
        }
#line 4042 "src/Slice/Grammar.cpp"
        break;

        case 178: /* keyword: ICE_INTERFACE  */
#line 2118 "src/Slice/Grammar.y"
        {
        }
#line 4048 "src/Slice/Grammar.cpp"
        break;

        case 179: /* keyword: ICE_EXCEPTION  */
#line 2119 "src/Slice/Grammar.y"
        {
        }
#line 4054 "src/Slice/Grammar.cpp"
        break;

        case 180: /* keyword: ICE_STRUCT  */
#line 2120 "src/Slice/Grammar.y"
        {
        }
#line 4060 "src/Slice/Grammar.cpp"
        break;

        case 181: /* keyword: ICE_SEQUENCE  */
#line 2121 "src/Slice/Grammar.y"
        {
        }
#line 4066 "src/Slice/Grammar.cpp"
        break;

        case 182: /* keyword: ICE_DICTIONARY  */
#line 2122 "src/Slice/Grammar.y"
        {
        }
#line 4072 "src/Slice/Grammar.cpp"
        break;

        case 183: /* keyword: ICE_ENUM  */
#line 2123 "src/Slice/Grammar.y"
        {
        }
#line 4078 "src/Slice/Grammar.cpp"
        break;

        case 184: /* keyword: ICE_OUT  */
#line 2124 "src/Slice/Grammar.y"
        {
        }
#line 4084 "src/Slice/Grammar.cpp"
        break;

        case 185: /* keyword: ICE_EXTENDS  */
#line 2125 "src/Slice/Grammar.y"
        {
        }
#line 4090 "src/Slice/Grammar.cpp"
        break;

        case 186: /* keyword: ICE_THROWS  */
#line 2126 "src/Slice/Grammar.y"
        {
        }
#line 4096 "src/Slice/Grammar.cpp"
        break;

        case 187: /* keyword: ICE_VOID  */
#line 2127 "src/Slice/Grammar.y"
        {
        }
#line 4102 "src/Slice/Grammar.cpp"
        break;

        case 188: /* keyword: ICE_BOOL  */
#line 2128 "src/Slice/Grammar.y"
        {
        }
#line 4108 "src/Slice/Grammar.cpp"
        break;

        case 189: /* keyword: ICE_BYTE  */
#line 2129 "src/Slice/Grammar.y"
        {
        }
#line 4114 "src/Slice/Grammar.cpp"
        break;

        case 190: /* keyword: ICE_SHORT  */
#line 2130 "src/Slice/Grammar.y"
        {
        }
#line 4120 "src/Slice/Grammar.cpp"
        break;

        case 191: /* keyword: ICE_INT  */
#line 2131 "src/Slice/Grammar.y"
        {
        }
#line 4126 "src/Slice/Grammar.cpp"
        break;

        case 192: /* keyword: ICE_LONG  */
#line 2132 "src/Slice/Grammar.y"
        {
        }
#line 4132 "src/Slice/Grammar.cpp"
        break;

        case 193: /* keyword: ICE_FLOAT  */
#line 2133 "src/Slice/Grammar.y"
        {
        }
#line 4138 "src/Slice/Grammar.cpp"
        break;

        case 194: /* keyword: ICE_DOUBLE  */
#line 2134 "src/Slice/Grammar.y"
        {
        }
#line 4144 "src/Slice/Grammar.cpp"
        break;

        case 195: /* keyword: ICE_STRING  */
#line 2135 "src/Slice/Grammar.y"
        {
        }
#line 4150 "src/Slice/Grammar.cpp"
        break;

        case 196: /* keyword: ICE_OBJECT  */
#line 2136 "src/Slice/Grammar.y"
        {
        }
#line 4156 "src/Slice/Grammar.cpp"
        break;

        case 197: /* keyword: ICE_CONST  */
#line 2137 "src/Slice/Grammar.y"
        {
        }
#line 4162 "src/Slice/Grammar.cpp"
        break;

        case 198: /* keyword: ICE_FALSE  */
#line 2138 "src/Slice/Grammar.y"
        {
        }
#line 4168 "src/Slice/Grammar.cpp"
        break;

        case 199: /* keyword: ICE_TRUE  */
#line 2139 "src/Slice/Grammar.y"
        {
        }
#line 4174 "src/Slice/Grammar.cpp"
        break;

        case 200: /* keyword: ICE_IDEMPOTENT  */
#line 2140 "src/Slice/Grammar.y"
        {
        }
#line 4180 "src/Slice/Grammar.cpp"
        break;

        case 201: /* keyword: ICE_OPTIONAL  */
#line 2141 "src/Slice/Grammar.y"
        {
        }
#line 4186 "src/Slice/Grammar.cpp"
        break;

        case 202: /* keyword: ICE_VALUE  */
#line 2142 "src/Slice/Grammar.y"
        {
        }
#line 4192 "src/Slice/Grammar.cpp"
        break;

#line 4196 "src/Slice/Grammar.cpp"

        default:
            break;
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
    YY_SYMBOL_PRINT("-> $$ =", YY_CAST(yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

    YYPOPSTACK(yylen);
    yylen = 0;

    *++yyvsp = yyval;
    *++yylsp = yyloc;

    /* Now 'shift' the result of the reduction.  Determine what state
       that goes to, based on the state we popped back to and the rule
       number reduced by.  */
    {
        const int yylhs = yyr1[yyn] - YYNTOKENS;
        const int yyi = yypgoto[yylhs] + *yyssp;
        yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp ? yytable[yyi] : yydefgoto[yylhs]);
    }

    goto yynewstate;

/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE(yychar);
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus)
    {
        ++yynerrs;
        yyerror(YY_("syntax error"));
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
            yydestruct("Error: discarding", yytoken, &yylval, &yylloc);
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
    YYPOPSTACK(yylen);
    yylen = 0;
    YY_STACK_PRINT(yyss, yyssp);
    yystate = *yyssp;
    goto yyerrlab1;

/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
    yyerrstatus = 3; /* Each real token shifted decrements this.  */

    /* Pop stack until we find a state that shifts the error token.  */
    for (;;)
    {
        yyn = yypact[yystate];
        if (!yypact_value_is_default(yyn))
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
        yydestruct("Error: popping", YY_ACCESSING_SYMBOL(yystate), yyvsp, yylsp);
        YYPOPSTACK(1);
        yystate = *yyssp;
        YY_STACK_PRINT(yyss, yyssp);
    }

    YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
    *++yyvsp = yylval;
    YY_IGNORE_MAYBE_UNINITIALIZED_END

    yyerror_range[2] = yylloc;
    ++yylsp;
    YYLLOC_DEFAULT(*yylsp, yyerror_range, 2);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT("Shifting", YY_ACCESSING_SYMBOL(yyn), yyvsp, yylsp);

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
    yyerror(YY_("memory exhausted"));
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
        yytoken = YYTRANSLATE(yychar);
        yydestruct("Cleanup: discarding lookahead", yytoken, &yylval, &yylloc);
    }
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    YYPOPSTACK(yylen);
    YY_STACK_PRINT(yyss, yyssp);
    while (yyssp != yyss)
    {
        yydestruct("Cleanup: popping", YY_ACCESSING_SYMBOL(+*yyssp), yyvsp, yylsp);
        YYPOPSTACK(1);
    }
#ifndef yyoverflow
    if (yyss != yyssa)
        YYSTACK_FREE(yyss);
#endif

    return yyresult;
}

#line 2145 "src/Slice/Grammar.y"
