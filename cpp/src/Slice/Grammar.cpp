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


// Copyright (c) ZeroC, Inc.

// NOLINTBEGIN

#line 27 "src/Slice/Grammar.y"


// Defines the rule bison uses to reduce token locations. Bison asks that the macro should
// be one-line, and treatable as a single statement when followed by a semi-colon.
// `N` is the number of tokens that are being combined, and (Cur) is their combined location.
#define YYLLOC_DEFAULT(Cur, Rhs, N)                               \
do                                                                \
    if (N == 1)                                                   \
    {                                                             \
        (Cur) = (YYRHSLOC((Rhs), 1));                             \
    }                                                             \
    else                                                          \
    {                                                             \
        if (N)                                                    \
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


#line 106 "src/Slice/Grammar.cpp"

/* Substitute the variable and function names.  */
#define yyparse         slice_parse
#define yylex           slice_lex
#define yyerror         slice_error
#define yydebug         slice_debug
#define yynerrs         slice_nerrs

/* First part of user prologue.  */
#line 66 "src/Slice/Grammar.y"


#include "Ice/UUID.h"
#include "Parser.h"
#include "GrammarUtil.h"

#include <cstring>
#include <limits>

#ifdef _MSC_VER
// warning C4102: 'yyoverflowlab' : unreferenced label
#    pragma warning(disable:4102)
// warning C4127: conditional expression is constant
#    pragma warning(disable:4127)
// warning C4702: unreachable code
#    pragma warning(disable:4702)
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
    // We want 'unit' to emit the error so its error count is incremented.
    currentUnit->error(s);
}

namespace
{
    /// Resolves a `Type` named `name` from the current scope and returns it.
    /// If no such type could be found, this returns `nullptr` instead.
    /// @param name The (possibly scoped) name to resolve.
    /// @param expectInterfaceType `true` if the type ends with '*' (indicated a proxy), false otherwise.
    ///        This function will automatically emit errors for interfaces missing '*', or non-interfaces using '*'.
    [[nodiscard]] TypePtr lookupTypeByName(const string& name, bool expectInterfaceType);

    /// Resolves an `InterfaceDef` named `name` from the current scope and returns it.
    /// If no such interface could be found, this returns `nullptr` instead.
    /// @param name The (possibly scoped) name to resolve.
    [[nodiscard]] InterfaceDefPtr lookupInterfaceByName(const string& name);

    /// Checks if the provided integer token's value is within the range ['0' ... 'int32_t::max'] (inclusive).
    /// If it is within this range, this function will return `true`.
    /// Otherwise this will return `false`, and automatically emit an error stating the issue.
    /// @param token An integer token to check the value of.
    /// @param kindString A string describing what the integer is being used for ("tag", "compact id", etc.).
    ///                   It is only used to emit a more descriptive error message.
    bool checkIntegerBounds(const IntegerTokPtr& token, string_view kindString);
}


#line 186 "src/Slice/Grammar.cpp"

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
  YYSYMBOL_ICE_MODULE = 3,                 /* "module keyword"  */
  YYSYMBOL_ICE_CLASS = 4,                  /* "class keyword"  */
  YYSYMBOL_ICE_INTERFACE = 5,              /* "interface keyword"  */
  YYSYMBOL_ICE_EXCEPTION = 6,              /* "exception keyword"  */
  YYSYMBOL_ICE_STRUCT = 7,                 /* "struct keyword"  */
  YYSYMBOL_ICE_SEQUENCE = 8,               /* "sequence keyword"  */
  YYSYMBOL_ICE_DICTIONARY = 9,             /* "dictionary keyword"  */
  YYSYMBOL_ICE_ENUM = 10,                  /* "enum keyword"  */
  YYSYMBOL_ICE_OUT = 11,                   /* "out keyword"  */
  YYSYMBOL_ICE_EXTENDS = 12,               /* "extends keyword"  */
  YYSYMBOL_ICE_THROWS = 13,                /* "throws keyword"  */
  YYSYMBOL_ICE_VOID = 14,                  /* "void keyword"  */
  YYSYMBOL_ICE_BOOL = 15,                  /* "bool keyword"  */
  YYSYMBOL_ICE_BYTE = 16,                  /* "byte keyword"  */
  YYSYMBOL_ICE_SHORT = 17,                 /* "short keyword"  */
  YYSYMBOL_ICE_INT = 18,                   /* "int keyword"  */
  YYSYMBOL_ICE_LONG = 19,                  /* "long keyword"  */
  YYSYMBOL_ICE_FLOAT = 20,                 /* "float keyword"  */
  YYSYMBOL_ICE_DOUBLE = 21,                /* "double keyword"  */
  YYSYMBOL_ICE_STRING = 22,                /* "string keyword"  */
  YYSYMBOL_ICE_OBJECT = 23,                /* "Object keyword"  */
  YYSYMBOL_ICE_CONST = 24,                 /* "const keyword"  */
  YYSYMBOL_ICE_FALSE = 25,                 /* "false keyword"  */
  YYSYMBOL_ICE_TRUE = 26,                  /* "true keyword"  */
  YYSYMBOL_ICE_IDEMPOTENT = 27,            /* "idempotent keyword"  */
  YYSYMBOL_ICE_OPTIONAL = 28,              /* "optional keyword"  */
  YYSYMBOL_ICE_VALUE = 29,                 /* "Value keyword"  */
  YYSYMBOL_ICE_STRING_LITERAL = 30,        /* "string literal"  */
  YYSYMBOL_ICE_INTEGER_LITERAL = 31,       /* "integer literal"  */
  YYSYMBOL_ICE_FLOATING_POINT_LITERAL = 32, /* "floating-point literal"  */
  YYSYMBOL_ICE_IDENTIFIER = 33,            /* "identifier"  */
  YYSYMBOL_ICE_SCOPED_IDENTIFIER = 34,     /* "scoped identifier"  */
  YYSYMBOL_ICE_METADATA_OPEN = 35,         /* "["  */
  YYSYMBOL_ICE_METADATA_CLOSE = 36,        /* "]"  */
  YYSYMBOL_ICE_FILE_METADATA_OPEN = 37,    /* "[["  */
  YYSYMBOL_ICE_FILE_METADATA_CLOSE = 38,   /* "]]"  */
  YYSYMBOL_ICE_IDENT_OPEN = 39,            /* "identifier("  */
  YYSYMBOL_ICE_KEYWORD_OPEN = 40,          /* "keyword("  */
  YYSYMBOL_ICE_OPTIONAL_OPEN = 41,         /* "optional("  */
  YYSYMBOL_BAD_TOKEN = 42,                 /* "invalid character"  */
  YYSYMBOL_EMPTY_PREC = 43,                /* EMPTY_PREC  */
  YYSYMBOL_44_ = 44,                       /* ';'  */
  YYSYMBOL_45_ = 45,                       /* '{'  */
  YYSYMBOL_46_ = 46,                       /* '}'  */
  YYSYMBOL_47_ = 47,                       /* ')'  */
  YYSYMBOL_48_ = 48,                       /* ':'  */
  YYSYMBOL_49_ = 49,                       /* '='  */
  YYSYMBOL_50_ = 50,                       /* ','  */
  YYSYMBOL_51_ = 51,                       /* '<'  */
  YYSYMBOL_52_ = 52,                       /* '>'  */
  YYSYMBOL_53_ = 53,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 54,                  /* $accept  */
  YYSYMBOL_start = 55,                     /* start  */
  YYSYMBOL_opt_semicolon = 56,             /* opt_semicolon  */
  YYSYMBOL_file_metadata = 57,             /* file_metadata  */
  YYSYMBOL_local_metadata = 58,            /* local_metadata  */
  YYSYMBOL_metadata = 59,                  /* metadata  */
  YYSYMBOL_metadata_directives = 60,       /* metadata_directives  */
  YYSYMBOL_definitions = 61,               /* definitions  */
  YYSYMBOL_definition = 62,                /* definition  */
  YYSYMBOL_module_def = 63,                /* module_def  */
  YYSYMBOL_64_1 = 64,                      /* @1  */
  YYSYMBOL_65_2 = 65,                      /* @2  */
  YYSYMBOL_exception_decl = 66,            /* exception_decl  */
  YYSYMBOL_exception_def = 67,             /* exception_def  */
  YYSYMBOL_68_3 = 68,                      /* @3  */
  YYSYMBOL_exception_extends = 69,         /* exception_extends  */
  YYSYMBOL_type_id = 70,                   /* type_id  */
  YYSYMBOL_optional = 71,                  /* optional  */
  YYSYMBOL_optional_type_id = 72,          /* optional_type_id  */
  YYSYMBOL_struct_decl = 73,               /* struct_decl  */
  YYSYMBOL_struct_def = 74,                /* struct_def  */
  YYSYMBOL_75_4 = 75,                      /* @4  */
  YYSYMBOL_class_name = 76,                /* class_name  */
  YYSYMBOL_class_id = 77,                  /* class_id  */
  YYSYMBOL_class_decl = 78,                /* class_decl  */
  YYSYMBOL_class_def = 79,                 /* class_def  */
  YYSYMBOL_80_5 = 80,                      /* @5  */
  YYSYMBOL_class_extends = 81,             /* class_extends  */
  YYSYMBOL_extends = 82,                   /* extends  */
  YYSYMBOL_data_members = 83,              /* data_members  */
  YYSYMBOL_data_member = 84,               /* data_member  */
  YYSYMBOL_return_type = 85,               /* return_type  */
  YYSYMBOL_idempotent_modifier = 86,       /* idempotent_modifier  */
  YYSYMBOL_operation_preamble = 87,        /* operation_preamble  */
  YYSYMBOL_operation = 88,                 /* operation  */
  YYSYMBOL_89_6 = 89,                      /* @6  */
  YYSYMBOL_90_7 = 90,                      /* @7  */
  YYSYMBOL_interface_id = 91,              /* interface_id  */
  YYSYMBOL_interface_decl = 92,            /* interface_decl  */
  YYSYMBOL_interface_def = 93,             /* interface_def  */
  YYSYMBOL_94_8 = 94,                      /* @8  */
  YYSYMBOL_interface_list = 95,            /* interface_list  */
  YYSYMBOL_interface_extends = 96,         /* interface_extends  */
  YYSYMBOL_operations = 97,                /* operations  */
  YYSYMBOL_exception_list = 98,            /* exception_list  */
  YYSYMBOL_exception = 99,                 /* exception  */
  YYSYMBOL_sequence_def = 100,             /* sequence_def  */
  YYSYMBOL_dictionary_def = 101,           /* dictionary_def  */
  YYSYMBOL_enum_def = 102,                 /* enum_def  */
  YYSYMBOL_103_9 = 103,                    /* @9  */
  YYSYMBOL_enumerators = 104,              /* enumerators  */
  YYSYMBOL_enumerator_list = 105,          /* enumerator_list  */
  YYSYMBOL_enumerator = 106,               /* enumerator  */
  YYSYMBOL_parameter = 107,                /* parameter  */
  YYSYMBOL_parameters = 108,               /* parameters  */
  YYSYMBOL_parameter_list = 109,           /* parameter_list  */
  YYSYMBOL_throws = 110,                   /* throws  */
  YYSYMBOL_scoped_name = 111,              /* scoped_name  */
  YYSYMBOL_builtin = 112,                  /* builtin  */
  YYSYMBOL_type = 113,                     /* type  */
  YYSYMBOL_integer_constant = 114,         /* integer_constant  */
  YYSYMBOL_string_literal = 115,           /* string_literal  */
  YYSYMBOL_metadata_list = 116,            /* metadata_list  */
  YYSYMBOL_const_initializer = 117,        /* const_initializer  */
  YYSYMBOL_const_def = 118,                /* const_def  */
  YYSYMBOL_definition_name = 119,          /* definition_name  */
  YYSYMBOL_definition_name_open = 120,     /* definition_name_open  */
  YYSYMBOL_keyword = 121                   /* keyword  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;



/* Unqualified %code blocks.  */
#line 58 "src/Slice/Grammar.y"


// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' (or 'slice_lex') in the generated scanner.
int slice_lex(YYSTYPE* lvalp, YYLTYPE* llocp);


#line 350 "src/Slice/Grammar.cpp"

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

#if 1

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
#endif /* 1 */

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
#define YYLAST   536

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  68
/* YYNRULES -- Number of rules.  */
#define YYNRULES  177
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  267

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   298


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
       2,    47,    53,     2,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    48,    44,
      51,    49,    52,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    45,     2,    46,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   211,   211,   219,   222,   230,   234,   238,   248,   252,
     256,   266,   269,   278,   282,   294,   302,   311,   319,   323,
     327,   331,   335,   339,   343,   347,   351,   355,   359,   363,
     367,   371,   381,   380,   398,   397,   462,   473,   472,   493,
     501,   510,   521,   534,   540,   551,   568,   581,   592,   591,
     618,   622,   633,   653,   665,   678,   677,   711,   739,   748,
     751,   759,   768,   771,   775,   783,   811,   845,   860,   866,
     876,   880,   889,   919,   918,   950,   949,   972,   976,   987,
    1001,  1000,  1034,  1044,  1054,  1059,  1069,  1073,  1082,  1091,
    1094,  1098,  1106,  1116,  1131,  1142,  1159,  1172,  1188,  1187,
    1212,  1213,  1217,  1226,  1238,  1255,  1261,  1277,  1289,  1302,
    1310,  1324,  1325,  1331,  1332,  1338,  1342,  1351,  1354,  1362,
    1363,  1364,  1365,  1366,  1367,  1368,  1369,  1370,  1371,  1376,
    1380,  1385,  1390,  1400,  1404,  1489,  1495,  1503,  1513,  1528,
    1537,  1546,  1581,  1588,  1595,  1607,  1621,  1626,  1634,  1646,
    1651,  1664,  1665,  1666,  1667,  1668,  1669,  1670,  1671,  1672,
    1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,  1681,  1682,
    1683,  1684,  1685,  1686,  1687,  1688,  1689,  1690
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  static const char *const yy_sname[] =
  {
  "end of file", "error", "invalid token", "module keyword",
  "class keyword", "interface keyword", "exception keyword",
  "struct keyword", "sequence keyword", "dictionary keyword",
  "enum keyword", "out keyword", "extends keyword", "throws keyword",
  "void keyword", "bool keyword", "byte keyword", "short keyword",
  "int keyword", "long keyword", "float keyword", "double keyword",
  "string keyword", "Object keyword", "const keyword", "false keyword",
  "true keyword", "idempotent keyword", "optional keyword",
  "Value keyword", "string literal", "integer literal",
  "floating-point literal", "identifier", "scoped identifier", "[", "]",
  "[[", "]]", "identifier(", "keyword(", "optional(", "invalid character",
  "EMPTY_PREC", "';'", "'{'", "'}'", "')'", "':'", "'='", "','", "'<'",
  "'>'", "'*'", "$accept", "start", "opt_semicolon", "file_metadata",
  "local_metadata", "metadata", "metadata_directives", "definitions",
  "definition", "module_def", "@1", "@2", "exception_decl",
  "exception_def", "@3", "exception_extends", "type_id", "optional",
  "optional_type_id", "struct_decl", "struct_def", "@4", "class_name",
  "class_id", "class_decl", "class_def", "@5", "class_extends", "extends",
  "data_members", "data_member", "return_type", "idempotent_modifier",
  "operation_preamble", "operation", "@6", "@7", "interface_id",
  "interface_decl", "interface_def", "@8", "interface_list",
  "interface_extends", "operations", "exception_list", "exception",
  "sequence_def", "dictionary_def", "enum_def", "@9", "enumerators",
  "enumerator_list", "enumerator", "parameter", "parameters",
  "parameter_list", "throws", "scoped_name", "builtin", "type",
  "integer_constant", "string_literal", "metadata_list",
  "const_initializer", "const_def", "definition_name",
  "definition_name_open", "keyword", YY_NULLPTR
  };
  return yy_sname[yysymbol];
}
#endif

#define YYPACT_NINF (-210)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-113)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -210,    17,    14,  -210,    32,    10,  -210,  -210,   506,     3,
      -1,   -11,  -210,  -210,   -18,     6,  -210,   -16,    20,   321,
     283,   385,   416,   416,    15,    30,   416,     3,  -210,    41,
      42,    41,    56,    41,    62,    11,    63,    41,     8,    74,
      41,    75,    77,    41,    80,  -210,  -210,  -210,  -210,   -11,
    -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,
    -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,
    -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,
    -210,  -210,  -210,  -210,  -210,  -210,  -210,   119,  -210,  -210,
    -210,     9,    53,     3,     3,  -210,   502,  -210,  -210,  -210,
    -210,  -210,  -210,  -210,  -210,  -210,    27,  -210,  -210,   115,
    -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,    58,    86,
    -210,  -210,  -210,  -210,    78,  -210,    27,    89,   502,   502,
      91,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,    90,
    -210,   103,  -210,   416,   102,  -210,  -210,  -210,   107,  -210,
     113,  -210,  -210,  -210,   116,  -210,   239,    87,   110,    12,
    -210,  -210,   125,   239,    27,   202,    -7,   105,   239,   118,
     171,   129,   416,     3,   447,   131,   130,   138,   133,  -210,
     137,   155,   149,  -210,  -210,   150,   239,  -210,    99,  -210,
     502,   134,   140,   416,  -210,  -210,   502,   148,  -210,  -210,
    -210,    45,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,
     202,  -210,   467,    54,   154,  -210,  -210,  -210,  -210,   161,
    -210,   138,   239,  -210,   157,   119,   447,  -210,  -210,   502,
       2,  -210,   164,    94,    94,  -210,  -210,   166,   152,   202,
    -210,  -210,  -210,   416,  -210,  -210,  -210,  -210,  -210,  -210,
    -210,  -210,    94,  -210,  -210,   201,   201,  -210,   353,  -210,
    -210,   165,  -210,  -210,  -210,   353,  -210
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      17,     0,    12,     1,     0,     0,    15,    13,     0,    11,
       0,   136,    10,   138,     0,     0,     7,     0,     0,   148,
       0,     0,   148,   148,     0,     0,   148,    12,    16,     4,
       0,     4,     0,     4,    53,    58,     0,     4,    79,     0,
       4,     0,     0,     4,     0,    14,     9,   135,     8,     0,
       6,     5,    31,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,   173,   174,   175,   176,   177,
     146,    34,    32,   147,    50,   149,   150,     0,    51,    77,
      78,    36,    47,    12,    12,    98,     0,     3,    18,    23,
      24,    25,    26,    59,    60,    55,     0,    19,    20,     0,
      80,    21,    22,    27,    28,    29,    30,   137,     0,     0,
     133,   117,   118,   134,     0,    37,     0,     0,     0,     0,
       0,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   131,   130,   148,     0,    57,    84,    85,    86,    83,
       0,    17,    17,    52,     0,    39,     0,     0,     0,    12,
     129,   132,     0,     0,     0,     0,    12,    12,     0,     0,
       0,     0,   148,    12,     0,     0,   100,     0,     0,    82,
       0,    71,     0,    35,    33,     0,     0,    44,     0,    46,
       0,    65,    63,   148,    49,    96,     0,   105,   104,   107,
      99,    12,   143,   144,   142,   139,   140,   141,   145,    56,
       0,    70,     0,     0,    90,    81,    38,    62,    43,     0,
      45,     0,     0,    41,     0,     0,     0,    89,    69,     0,
       0,    68,     0,     0,     0,   108,   113,     0,   111,     0,
      42,    66,    61,   148,   106,   103,    67,    72,    75,   109,
     110,    73,     0,    88,    97,   116,   116,   114,     0,    76,
      74,   115,    93,    94,    95,     0,    92
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -210,  -210,    59,  -210,    -6,     0,  -210,   -73,  -210,  -210,
    -210,  -210,  -210,  -210,  -210,  -210,    43,    22,    71,  -210,
    -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,   -28,  -155,
    -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,  -210,
    -210,  -210,  -210,  -194,  -210,   -33,  -210,  -210,  -210,  -210,
    -210,  -210,    16,  -209,  -210,  -210,   -12,   -80,  -210,   -92,
    -173,     1,   242,    28,  -210,   -17,    21,   -20
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,    98,     6,     7,   170,     9,     2,    28,    29,
     119,   118,    30,    31,   154,   125,   189,   190,   235,    32,
      33,   127,    34,    35,    36,    37,   144,   105,   106,   171,
     192,   230,   212,   213,   214,   256,   255,    38,    39,    40,
     150,   148,   110,   182,   261,   262,    41,    42,    43,   130,
     175,   176,   198,   236,   237,   238,   259,   141,   142,   193,
     124,    13,    14,   208,    44,    82,    87,    83
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      88,    90,     8,    45,   143,    91,    92,   123,   178,    95,
     109,    15,    47,   185,    -2,   219,   227,     3,    48,    11,
     103,   103,    51,   103,   249,   250,   145,    96,     4,   149,
       5,   217,    49,    10,    49,    46,   157,   158,     4,   183,
      11,    85,    86,   257,    50,   253,   155,     4,    16,     4,
     117,     5,   244,   -87,   -40,   232,   104,   104,  -102,   104,
     121,   122,    11,   126,    52,   233,    93,   242,    12,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   166,   167,
       4,    94,   187,   140,   179,    97,    99,   121,   122,     4,
     100,  -101,   102,   128,   129,   188,   108,   207,   -48,   112,
     101,  -112,   115,   151,   224,   233,   -54,   107,   123,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   111,   113,
     231,   114,   187,   140,   116,   153,   162,   121,   122,     4,
     120,   152,   121,   122,   156,   188,   159,   246,   146,   172,
       4,   207,     5,   160,   147,   123,   218,   163,   121,   122,
     120,   184,   121,   122,   199,   195,   161,   164,   165,   174,
     173,   168,   186,   202,   203,   181,     8,     8,   204,   205,
     206,   121,   122,   196,   177,   194,   223,   200,   263,   209,
     201,   210,   211,   221,   222,   263,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   215,   216,   225,   239,   187,
     140,   226,   252,   180,   121,   122,   199,   234,   240,   243,
     181,   248,   188,   251,   258,   265,   -12,   -12,   -12,   -12,
     -12,   -12,   -12,   -12,   -12,   -12,   254,   234,   234,   -12,
     -12,   -12,   266,   220,   229,   -12,   -12,     4,   264,   181,
     169,   191,   245,   -12,   260,   264,   234,    17,   -91,   241,
       0,   247,     0,     0,   -12,   -12,   -12,   -12,   -12,   -12,
     -12,   -12,   -12,     0,     0,     0,     0,   -12,   -12,     0,
       0,     0,   -12,   -12,     4,     0,     0,     0,     0,     0,
     -12,     0,     0,     0,     0,   -64,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,    84,     0,     0,     0,
       0,     0,    85,    86,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,     0,     0,     0,    80,    81,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,     0,     0,     0,   121,   122,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,     0,     0,     0,    89,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,     0,     0,     0,    80,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,     0,     0,     0,
     197,   228,   131,   132,   133,   134,   135,   136,   137,   138,
     139,     0,     0,     0,     0,   187,   140,     0,     0,     0,
     121,   122,     0,     0,     0,     0,     0,    18,   188,    19,
      20,    21,    22,    23,    24,    25,    26,   131,   132,   133,
     134,   135,   136,   137,   138,   139,     0,     0,     0,     0,
      27,   140,     0,     0,     0,   121,   122
};

static const yytype_int16 yycheck[] =
{
      20,    21,     2,     9,    96,    22,    23,    87,   163,    26,
      38,     1,    11,   168,     0,   188,   210,     0,    36,    30,
      12,    12,    38,    12,   233,   234,   106,    27,    35,   109,
      37,   186,    50,     1,    50,    36,   128,   129,    35,    46,
      30,    39,    40,   252,    38,   239,   126,    35,    38,    35,
      49,    37,   225,    45,    45,     1,    48,    48,    46,    48,
      33,    34,    30,    91,    44,    11,    51,   222,    36,    15,
      16,    17,    18,    19,    20,    21,    22,    23,   151,   152,
      35,    51,    28,    29,   164,    44,    44,    33,    34,    35,
      31,    46,    33,    93,    94,    41,    37,   177,    45,    40,
      44,    47,    43,    45,   196,    11,    44,    44,   188,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    44,    44,
     212,    44,    28,    29,    44,    47,   143,    33,    34,    35,
      31,    45,    33,    34,    45,    41,    45,   229,    23,    52,
      35,   221,    37,    53,    29,   225,    47,    45,    33,    34,
      31,    46,    33,    34,   174,   172,    53,    50,    45,   159,
      50,    45,    44,    25,    26,   165,   166,   167,    30,    31,
      32,    33,    34,   173,    49,    46,   193,    46,   258,    46,
      50,    44,    27,    49,    44,   265,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    46,    46,    49,    44,    28,
      29,   201,    50,     1,    33,    34,   226,   213,    47,    52,
     210,    47,    41,    47,    13,    50,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,   243,   233,   234,    27,
      28,    29,   265,   190,   212,    33,    34,    35,   258,   239,
       1,   170,   226,    41,   256,   265,   252,     5,    46,   221,
      -1,   230,    -1,    -1,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    -1,    -1,    28,    29,    -1,
      -1,    -1,    33,    34,    35,    -1,    -1,    -1,    -1,    -1,
      41,    -1,    -1,    -1,    -1,    46,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    -1,    33,    -1,    -1,    -1,
      -1,    -1,    39,    40,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    -1,    -1,    33,    34,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    -1,    -1,    -1,    33,    34,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    -1,    33,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    -1,    -1,    33,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    -1,
      33,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    -1,    -1,    -1,    -1,    28,    29,    -1,    -1,    -1,
      33,    34,    -1,    -1,    -1,    -1,    -1,     1,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    -1,    -1,
      24,    29,    -1,    -1,    -1,    33,    34
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    55,    61,     0,    35,    37,    57,    58,    59,    60,
       1,    30,    36,   115,   116,     1,    38,   116,     1,     3,
       4,     5,     6,     7,     8,     9,    10,    24,    62,    63,
      66,    67,    73,    74,    76,    77,    78,    79,    91,    92,
      93,   100,   101,   102,   118,    58,    36,   115,    36,    50,
      38,    38,    44,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      33,    34,   119,   121,    33,    39,    40,   120,   121,    33,
     121,   119,   119,    51,    51,   119,    59,    44,    56,    44,
      56,    44,    56,    12,    48,    81,    82,    44,    56,    82,
      96,    44,    56,    44,    44,    56,    44,   115,    65,    64,
      31,    33,    34,   111,   114,    69,    82,    75,    59,    59,
     103,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      29,   111,   112,   113,    80,   111,    23,    29,    95,   111,
      94,    45,    45,    47,    68,   111,    45,   113,   113,    45,
      53,    53,   119,    45,    50,    45,    61,    61,    45,     1,
      59,    83,    52,    50,    59,   104,   105,    49,    83,   111,
       1,    59,    97,    46,    46,    83,    44,    28,    41,    70,
      71,    72,    84,   113,    46,   119,    59,    33,   106,   121,
      46,    50,    25,    26,    30,    31,    32,   111,   117,    46,
      44,    27,    86,    87,    88,    46,    46,    83,    47,   114,
      70,    49,    44,   119,   113,    49,    59,    97,    14,    71,
      85,   113,     1,    11,    58,    72,   107,   108,   109,    44,
      47,   117,    83,    52,   114,   106,   113,   120,    47,   107,
     107,    47,    50,    97,   119,    90,    89,   107,    13,   110,
     110,    98,    99,   111,   121,    50,    99
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    54,    55,    56,    56,    57,    57,    57,    58,    58,
      58,    59,    59,    60,    60,    61,    61,    61,    62,    62,
      62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
      62,    62,    64,    63,    65,    63,    66,    68,    67,    69,
      69,    70,    71,    71,    71,    72,    72,    73,    75,    74,
      76,    76,    77,    77,    78,    80,    79,    81,    81,    82,
      82,    83,    83,    83,    83,    84,    84,    85,    85,    85,
      86,    86,    87,    89,    88,    90,    88,    91,    91,    92,
      94,    93,    95,    95,    95,    95,    96,    96,    97,    97,
      97,    97,    98,    98,    99,    99,   100,   101,   103,   102,
     104,   104,   104,   105,   105,   106,   106,   106,   107,   107,
     107,   108,   108,   109,   109,   110,   110,   111,   111,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   113,
     113,   113,   113,   114,   114,   115,   115,   116,   116,   117,
     117,   117,   117,   117,   117,   118,   119,   119,   119,   120,
     120,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     0,     3,     3,     2,     3,     3,
       2,     1,     0,     1,     2,     2,     3,     0,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     0,     6,     0,     6,     2,     0,     7,     2,
       0,     2,     3,     2,     1,     2,     1,     2,     0,     6,
       2,     2,     4,     1,     1,     0,     6,     2,     0,     1,
       1,     4,     3,     2,     0,     1,     3,     2,     1,     1,
       1,     0,     3,     0,     5,     0,     5,     2,     2,     1,
       0,     6,     3,     1,     1,     1,     2,     0,     4,     3,
       2,     0,     3,     1,     1,     1,     6,     9,     0,     6,
       1,     2,     0,     4,     2,     1,     3,     1,     1,     2,
       2,     1,     0,     1,     3,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     1,     2,     1,     1,     2,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     6,     1,     1,     0,     1,
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif



static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yystrlen (yysymbol_name (yyarg[yyi]));
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp = yystpcpy (yyp, yysymbol_name (yyarg[yyi++]));
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


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

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

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
#line 212 "src/Slice/Grammar.y"
{
}
#line 1911 "src/Slice/Grammar.cpp"
    break;

  case 3: /* opt_semicolon: ';'  */
#line 220 "src/Slice/Grammar.y"
{
}
#line 1918 "src/Slice/Grammar.cpp"
    break;

  case 4: /* opt_semicolon: %empty  */
#line 223 "src/Slice/Grammar.y"
{
}
#line 1925 "src/Slice/Grammar.cpp"
    break;

  case 5: /* file_metadata: "[[" metadata_list "]]"  */
#line 231 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1933 "src/Slice/Grammar.cpp"
    break;

  case 6: /* file_metadata: "[[" error "]]"  */
#line 235 "src/Slice/Grammar.y"
{
    yyval = make_shared<MetadataListTok>();
}
#line 1941 "src/Slice/Grammar.cpp"
    break;

  case 7: /* file_metadata: "[[" "]]"  */
#line 239 "src/Slice/Grammar.y"
{
    currentUnit->warning(WarningCategory::All, "No directives were provided in metadata list");
    yyval = make_shared<MetadataListTok>();
}
#line 1950 "src/Slice/Grammar.cpp"
    break;

  case 8: /* local_metadata: "[" metadata_list "]"  */
#line 249 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 1958 "src/Slice/Grammar.cpp"
    break;

  case 9: /* local_metadata: "[" error "]"  */
#line 253 "src/Slice/Grammar.y"
{
    yyval = make_shared<MetadataListTok>();
}
#line 1966 "src/Slice/Grammar.cpp"
    break;

  case 10: /* local_metadata: "[" "]"  */
#line 257 "src/Slice/Grammar.y"
{
    currentUnit->warning(WarningCategory::All, "No directives were provided in metadata list");
    yyval = make_shared<MetadataListTok>();
}
#line 1975 "src/Slice/Grammar.cpp"
    break;

  case 11: /* metadata: metadata_directives  */
#line 267 "src/Slice/Grammar.y"
{
}
#line 1982 "src/Slice/Grammar.cpp"
    break;

  case 12: /* metadata: %empty  */
#line 270 "src/Slice/Grammar.y"
{
    yyval = make_shared<MetadataListTok>();
}
#line 1990 "src/Slice/Grammar.cpp"
    break;

  case 13: /* metadata_directives: local_metadata  */
#line 279 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 1998 "src/Slice/Grammar.cpp"
    break;

  case 14: /* metadata_directives: metadata_directives local_metadata  */
#line 283 "src/Slice/Grammar.y"
{
    auto metadata1 = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto metadata2 = dynamic_pointer_cast<MetadataListTok>(yyvsp[0]);
    metadata1->v.splice(metadata1->v.end(), std::move(metadata2->v));
    yyval = metadata1;
}
#line 2009 "src/Slice/Grammar.cpp"
    break;

  case 15: /* definitions: definitions file_metadata  */
#line 295 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[0]);
    if (!metadata->v.empty())
    {
        currentUnit->addFileMetadata(std::move(metadata->v));
    }
}
#line 2021 "src/Slice/Grammar.cpp"
    break;

  case 16: /* definitions: definitions metadata definition  */
#line 303 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[0]);
    if (contained && !metadata->v.empty())
    {
        contained->appendMetadata(std::move(metadata->v));
    }
}
#line 2034 "src/Slice/Grammar.cpp"
    break;

  case 17: /* definitions: %empty  */
#line 312 "src/Slice/Grammar.y"
{
}
#line 2041 "src/Slice/Grammar.cpp"
    break;

  case 18: /* definition: module_def opt_semicolon  */
#line 320 "src/Slice/Grammar.y"
{
    assert(dynamic_pointer_cast<Module>(yyvsp[-1]));
}
#line 2049 "src/Slice/Grammar.cpp"
    break;

  case 19: /* definition: class_decl ';'  */
#line 324 "src/Slice/Grammar.y"
{
    assert(yyvsp[-1] == nullptr || dynamic_pointer_cast<ClassDecl>(yyvsp[-1]));
}
#line 2057 "src/Slice/Grammar.cpp"
    break;

  case 20: /* definition: class_def opt_semicolon  */
#line 328 "src/Slice/Grammar.y"
{
    assert(yyvsp[-1] == nullptr || dynamic_pointer_cast<ClassDef>(yyvsp[-1]));
}
#line 2065 "src/Slice/Grammar.cpp"
    break;

  case 21: /* definition: interface_decl ';'  */
#line 332 "src/Slice/Grammar.y"
{
    assert(yyvsp[-1] == nullptr || dynamic_pointer_cast<InterfaceDecl>(yyvsp[-1]));
}
#line 2073 "src/Slice/Grammar.cpp"
    break;

  case 22: /* definition: interface_def opt_semicolon  */
#line 336 "src/Slice/Grammar.y"
{
    assert(yyvsp[-1] == nullptr || dynamic_pointer_cast<InterfaceDef>(yyvsp[-1]));
}
#line 2081 "src/Slice/Grammar.cpp"
    break;

  case 23: /* definition: exception_decl ';'  */
#line 340 "src/Slice/Grammar.y"
{
    assert(yyvsp[-1] == nullptr);
}
#line 2089 "src/Slice/Grammar.cpp"
    break;

  case 24: /* definition: exception_def opt_semicolon  */
#line 344 "src/Slice/Grammar.y"
{
    assert(dynamic_pointer_cast<Exception>(yyvsp[-1]));
}
#line 2097 "src/Slice/Grammar.cpp"
    break;

  case 25: /* definition: struct_decl ';'  */
#line 348 "src/Slice/Grammar.y"
{
    assert(yyvsp[-1] == nullptr);
}
#line 2105 "src/Slice/Grammar.cpp"
    break;

  case 26: /* definition: struct_def opt_semicolon  */
#line 352 "src/Slice/Grammar.y"
{
    assert(dynamic_pointer_cast<Struct>(yyvsp[-1]));
}
#line 2113 "src/Slice/Grammar.cpp"
    break;

  case 27: /* definition: sequence_def ';'  */
#line 356 "src/Slice/Grammar.y"
{
    assert(dynamic_pointer_cast<Sequence>(yyvsp[-1]));
}
#line 2121 "src/Slice/Grammar.cpp"
    break;

  case 28: /* definition: dictionary_def ';'  */
#line 360 "src/Slice/Grammar.y"
{
    assert(dynamic_pointer_cast<Dictionary>(yyvsp[-1]));
}
#line 2129 "src/Slice/Grammar.cpp"
    break;

  case 29: /* definition: enum_def opt_semicolon  */
#line 364 "src/Slice/Grammar.y"
{
    assert(dynamic_pointer_cast<Enum>(yyvsp[-1]));
}
#line 2137 "src/Slice/Grammar.cpp"
    break;

  case 30: /* definition: const_def ';'  */
#line 368 "src/Slice/Grammar.y"
{
    assert(dynamic_pointer_cast<Const>(yyvsp[-1]));
}
#line 2145 "src/Slice/Grammar.cpp"
    break;

  case 31: /* definition: error ';'  */
#line 372 "src/Slice/Grammar.y"
{
    yyerrok;
}
#line 2153 "src/Slice/Grammar.cpp"
    break;

  case 32: /* @1: %empty  */
#line 381 "src/Slice/Grammar.y"
{
    currentUnit->setSeenDefinition();

    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ModulePtr module = cont->createModule(ident->v, false);

    cont->checkHasChangedMeaning(ident->v, module);
    currentUnit->pushContainer(module);
    yyval = module;
}
#line 2169 "src/Slice/Grammar.cpp"
    break;

  case 33: /* module_def: "module keyword" definition_name @1 '{' definitions '}'  */
#line 393 "src/Slice/Grammar.y"
{
    currentUnit->popContainer();
    yyval = yyvsp[-3];
}
#line 2178 "src/Slice/Grammar.cpp"
    break;

  case 34: /* @2: %empty  */
#line 398 "src/Slice/Grammar.y"
{
    currentUnit->setSeenDefinition();

    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);

    // Reject scoped identifiers starting with "::". This generally indicates global scope, but is invalid here.
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
        ModulePtr module = cont->createModule(currentModuleName, true);

        cont->checkHasChangedMeaning(currentModuleName, module);
        currentUnit->pushContainer(module);
        yyval = cont = module;
    }
}
#line 2218 "src/Slice/Grammar.cpp"
    break;

  case 35: /* module_def: "module keyword" "scoped identifier" @2 '{' definitions '}'  */
#line 434 "src/Slice/Grammar.y"
{
    // We need to pop '(N+1)' modules off the container stack, to navigate out of the nested module.
    // Where `N` is the number of scope separators ("::").
    size_t startPos = 0;
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-4]);

    // Skip over any leading "::". This is invalid syntax of course, but the parser still needs to properly handle it.
    if (ident->v.find("::") == 0)
    {
        startPos += 2; // Skip the leading "::".
    }

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
#line 2246 "src/Slice/Grammar.cpp"
    break;

  case 36: /* exception_decl: "exception keyword" definition_name  */
#line 463 "src/Slice/Grammar.y"
{
    currentUnit->error("exceptions cannot be forward declared");
    yyval = nullptr;
}
#line 2255 "src/Slice/Grammar.cpp"
    break;

  case 37: /* @3: %empty  */
#line 473 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    auto base = dynamic_pointer_cast<Exception>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ExceptionPtr ex = cont->createException(ident->v, base);

    cont->checkHasChangedMeaning(ident->v, ex);
    currentUnit->pushContainer(ex);
    yyval = ex;
}
#line 2270 "src/Slice/Grammar.cpp"
    break;

  case 38: /* exception_def: "exception keyword" definition_name exception_extends @3 '{' data_members '}'  */
#line 484 "src/Slice/Grammar.y"
{
    currentUnit->popContainer();
    yyval = yyvsp[-3];
}
#line 2279 "src/Slice/Grammar.cpp"
    break;

  case 39: /* exception_extends: extends scoped_name  */
#line 494 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ContainedPtr contained = cont->lookupException(scoped->v, true);
    cont->checkHasChangedMeaning(scoped->v);
    yyval = contained;
}
#line 2291 "src/Slice/Grammar.cpp"
    break;

  case 40: /* exception_extends: %empty  */
#line 502 "src/Slice/Grammar.y"
{
    yyval = nullptr;
}
#line 2299 "src/Slice/Grammar.cpp"
    break;

  case 41: /* type_id: type definition_name  */
#line 511 "src/Slice/Grammar.y"
{
    auto type = dynamic_pointer_cast<Type>(yyvsp[-1]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    yyval = make_shared<TypeStringTok>(type, ident->v);
}
#line 2309 "src/Slice/Grammar.cpp"
    break;

  case 42: /* optional: "optional(" integer_constant ')'  */
#line 522 "src/Slice/Grammar.y"
{
    auto integer = dynamic_pointer_cast<IntegerTok>(yyvsp[-1]);
    int32_t tag = -1;

    if (integer && checkIntegerBounds(integer, "tag"))
    {
        tag = static_cast<int32_t>(integer->v);
    }

    auto m = make_shared<OptionalDefTok>(tag);
    yyval = m;
}
#line 2326 "src/Slice/Grammar.cpp"
    break;

  case 43: /* optional: "optional(" ')'  */
#line 535 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<OptionalDefTok>(-1); // Dummy
    yyval = m;
}
#line 2336 "src/Slice/Grammar.cpp"
    break;

  case 44: /* optional: "optional keyword"  */
#line 541 "src/Slice/Grammar.y"
{
    currentUnit->error("missing tag");
    auto m = make_shared<OptionalDefTok>(-1); // Dummy
    yyval = m;
}
#line 2346 "src/Slice/Grammar.cpp"
    break;

  case 45: /* optional_type_id: optional type_id  */
#line 552 "src/Slice/Grammar.y"
{
    auto m = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
    auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);
    m->type = ts->type;
    m->name = ts->name;

    // It's safe to perform this check in the parser, since we already have enough information to know whether a type
    // can be optional. This is because the only types that can be forward declared (classes/interfaces) have constant
    // values for `usesClasses` (true/false respectively).
    if (m->type && m->type->usesClasses())
    {
        currentUnit->error("types that use classes cannot be marked with 'optional'");
    }

    yyval = m;
}
#line 2367 "src/Slice/Grammar.cpp"
    break;

  case 46: /* optional_type_id: type_id  */
#line 569 "src/Slice/Grammar.y"
{
    auto ts = dynamic_pointer_cast<TypeStringTok>(yyvsp[0]);
    auto m = make_shared<OptionalDefTok>(-1);
    m->type = ts->type;
    m->name = ts->name;
    yyval = m;
}
#line 2379 "src/Slice/Grammar.cpp"
    break;

  case 47: /* struct_decl: "struct keyword" definition_name  */
#line 582 "src/Slice/Grammar.y"
{
    currentUnit->error("structs cannot be forward declared");
    yyval = nullptr; // Dummy
}
#line 2388 "src/Slice/Grammar.cpp"
    break;

  case 48: /* @4: %empty  */
#line 592 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    StructPtr st = cont->createStruct(ident->v);

    cont->checkHasChangedMeaning(ident->v, st);
    currentUnit->pushContainer(st);
    yyval = st;
}
#line 2402 "src/Slice/Grammar.cpp"
    break;

  case 49: /* struct_def: "struct keyword" definition_name @4 '{' data_members '}'  */
#line 602 "src/Slice/Grammar.y"
{
    currentUnit->popContainer();

    // Empty structures are not allowed
    auto st = dynamic_pointer_cast<Struct>(yyvsp[-3]);
    if (st->dataMembers().empty())
    {
        currentUnit->error("struct '" + st->name() + "' must have at least one member");
    }
    yyval = st;
}
#line 2418 "src/Slice/Grammar.cpp"
    break;

  case 50: /* class_name: "class keyword" "identifier"  */
#line 619 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2426 "src/Slice/Grammar.cpp"
    break;

  case 51: /* class_name: "class keyword" keyword  */
#line 623 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as class name");
    yyval = yyvsp[0]; // Dummy
}
#line 2436 "src/Slice/Grammar.cpp"
    break;

  case 52: /* class_id: "class keyword" definition_name_open integer_constant ')'  */
#line 634 "src/Slice/Grammar.y"
{
    auto integer = dynamic_pointer_cast<IntegerTok>(yyvsp[-1]);
    int32_t id = -1;

    if (integer && checkIntegerBounds(integer, "compact id"))
    {
        id = static_cast<int32_t>(integer->v);
        string typeId = currentUnit->getTypeId(id);
        if (!typeId.empty())
        {
            currentUnit->error("invalid compact id for class: already assigned to class '" + typeId + "'");
        }
    }

    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[-2])->v;
    classId->t = id;
    yyval = classId;
}
#line 2460 "src/Slice/Grammar.cpp"
    break;

  case 53: /* class_id: class_name  */
#line 654 "src/Slice/Grammar.y"
{
    auto classId = make_shared<ClassIdTok>();
    classId->v = dynamic_pointer_cast<StringTok>(yyvsp[0])->v;
    classId->t = -1;
    yyval = classId;
}
#line 2471 "src/Slice/Grammar.cpp"
    break;

  case 54: /* class_decl: class_name  */
#line 666 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ClassDeclPtr cl = cont->createClassDecl(ident->v);
    yyval = cl;
}
#line 2482 "src/Slice/Grammar.cpp"
    break;

  case 55: /* @5: %empty  */
#line 678 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<ClassIdTok>(yyvsp[-1]);
    ContainerPtr cont = currentUnit->currentContainer();
    auto base = dynamic_pointer_cast<ClassDef>(yyvsp[0]);
    ClassDefPtr cl = cont->createClassDef(ident->v, ident->t, base);
    if (cl)
    {
        cont->checkHasChangedMeaning(ident->v, cl);
        currentUnit->pushContainer(cl);
        yyval = cl;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2503 "src/Slice/Grammar.cpp"
    break;

  case 56: /* class_def: class_id class_extends @5 '{' data_members '}'  */
#line 695 "src/Slice/Grammar.y"
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
#line 2519 "src/Slice/Grammar.cpp"
    break;

  case 57: /* class_extends: extends scoped_name  */
#line 712 "src/Slice/Grammar.y"
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
            currentUnit->error("'" + scoped->v + "' is not a class");
        }
        else
        {
            ClassDefPtr def = cl->definition();
            if (!def)
            {
                currentUnit->error("'" + scoped->v + "' has been declared but not defined");
            }
            else
            {
                cont->checkHasChangedMeaning(scoped->v);
                yyval = def;
            }
        }
    }
}
#line 2551 "src/Slice/Grammar.cpp"
    break;

  case 58: /* class_extends: %empty  */
#line 740 "src/Slice/Grammar.y"
{
    yyval = nullptr;
}
#line 2559 "src/Slice/Grammar.cpp"
    break;

  case 59: /* extends: "extends keyword"  */
#line 749 "src/Slice/Grammar.y"
{
}
#line 2566 "src/Slice/Grammar.cpp"
    break;

  case 60: /* extends: ':'  */
#line 752 "src/Slice/Grammar.y"
{
}
#line 2573 "src/Slice/Grammar.cpp"
    break;

  case 61: /* data_members: metadata data_member ';' data_members  */
#line 760 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
    if (contained && !metadata->v.empty())
    {
        contained->appendMetadata(std::move(metadata->v));
    }
}
#line 2586 "src/Slice/Grammar.cpp"
    break;

  case 62: /* data_members: error ';' data_members  */
#line 769 "src/Slice/Grammar.y"
{
}
#line 2593 "src/Slice/Grammar.cpp"
    break;

  case 63: /* data_members: metadata data_member  */
#line 772 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after definition");
}
#line 2601 "src/Slice/Grammar.cpp"
    break;

  case 64: /* data_members: %empty  */
#line 776 "src/Slice/Grammar.y"
{
}
#line 2608 "src/Slice/Grammar.cpp"
    break;

  case 65: /* data_member: optional_type_id  */
#line 784 "src/Slice/Grammar.y"
{
    auto def = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
    DataMemberPtr dm;

    if (auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer()))
    {
        dm = cl->createDataMember(def->name, def->type, def->isOptional, def->tag, nullptr, std::nullopt);
    }
    else if (auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer()))
    {
        if (def->isOptional)
        {
            currentUnit->error("optional data members are not supported in structs");
        }
        dm = st->createDataMember(def->name, def->type, nullptr, std::nullopt);
    }
    else if (auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer()))
    {
        dm = ex->createDataMember(def->name, def->type, def->isOptional, def->tag, nullptr, std::nullopt);
    }

    if (dm)
    {
        currentUnit->currentContainer()->checkHasChangedMeaning(def->name, dm);
    }
    yyval = dm;
}
#line 2640 "src/Slice/Grammar.cpp"
    break;

  case 66: /* data_member: optional_type_id '=' const_initializer  */
#line 812 "src/Slice/Grammar.y"
{
    auto def = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    DataMemberPtr dm;

    if (auto cl = dynamic_pointer_cast<ClassDef>(currentUnit->currentContainer()))
    {
        dm = cl->createDataMember(def->name, def->type, def->isOptional, def->tag, value->v, value->valueAsString);
    }
    else if (auto st = dynamic_pointer_cast<Struct>(currentUnit->currentContainer()))
    {
        if (def->isOptional)
        {
            currentUnit->error("optional data members are not supported in structs");
        }
        dm = st->createDataMember(def->name, def->type, value->v, value->valueAsString);
    }
    else if (auto ex = dynamic_pointer_cast<Exception>(currentUnit->currentContainer()))
    {
        dm = ex->createDataMember(def->name, def->type, def->isOptional, def->tag, value->v, value->valueAsString);
    }

    if (dm)
    {
        currentUnit->currentContainer()->checkHasChangedMeaning(def->name, dm);
    }
    yyval = dm;
}
#line 2673 "src/Slice/Grammar.cpp"
    break;

  case 67: /* return_type: optional type  */
#line 846 "src/Slice/Grammar.y"
{
    auto m = dynamic_pointer_cast<OptionalDefTok>(yyvsp[-1]);
    m->type = dynamic_pointer_cast<Type>(yyvsp[0]);

    // It's safe to perform this check in the parser, since we already have enough information to know whether a type
    // can be optional. This is because the only types that can be forward declared (classes/interfaces) have constant
    // values for `usesClasses` (true/false respectively).
    if (m->type && m->type->usesClasses())
    {
        currentUnit->error("types that use classes cannot be marked with 'optional'");
    }

    yyval = m;
}
#line 2692 "src/Slice/Grammar.cpp"
    break;

  case 68: /* return_type: type  */
#line 861 "src/Slice/Grammar.y"
{
    auto m = make_shared<OptionalDefTok>(-1);
    m->type = dynamic_pointer_cast<Type>(yyvsp[0]);
    yyval = m;
}
#line 2702 "src/Slice/Grammar.cpp"
    break;

  case 69: /* return_type: "void keyword"  */
#line 867 "src/Slice/Grammar.y"
{
    auto m = make_shared<OptionalDefTok>(-1);
    yyval = m;
}
#line 2711 "src/Slice/Grammar.cpp"
    break;

  case 70: /* idempotent_modifier: "idempotent keyword"  */
#line 877 "src/Slice/Grammar.y"
{
    yyval = make_shared<BoolTok>(true);
}
#line 2719 "src/Slice/Grammar.cpp"
    break;

  case 71: /* idempotent_modifier: %empty  */
#line 881 "src/Slice/Grammar.y"
{
    yyval = make_shared<BoolTok>(false);
}
#line 2727 "src/Slice/Grammar.cpp"
    break;

  case 72: /* operation_preamble: idempotent_modifier return_type definition_name_open  */
#line 890 "src/Slice/Grammar.y"
{
    bool isIdempotent = dynamic_pointer_cast<BoolTok>(yyvsp[-2])->v;
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
            isIdempotent ? Operation::Idempotent : Operation::Normal);

        interface->checkHasChangedMeaning(name, op);
        currentUnit->pushContainer(op);
        yyval = op;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2755 "src/Slice/Grammar.cpp"
    break;

  case 73: /* @6: %empty  */
#line 919 "src/Slice/Grammar.y"
{
    if (yyvsp[-2])
    {
        // Check that all out parameters come before all in parameters.
        auto op = dynamic_pointer_cast<Operation>(yyvsp[-2]);
        bool seenOutParam = false;
        for (const auto& param : op->parameters())
        {
            const bool isOutParam = param->isOutParam();
            if (!isOutParam && seenOutParam)
            {
                currentUnit->error("parameter '" + param->name() + "': in-parameters cannot come after out-parameters");
            }
            seenOutParam |= isOutParam;
        }

        currentUnit->popContainer();
    }
    yyval = yyvsp[-2];
}
#line 2780 "src/Slice/Grammar.cpp"
    break;

  case 74: /* operation: operation_preamble parameters ')' @6 throws  */
#line 940 "src/Slice/Grammar.y"
{
    auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
    auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    assert(el);
    if (op)
    {
        op->setExceptionList(el->v);
    }
}
#line 2794 "src/Slice/Grammar.cpp"
    break;

  case 75: /* @7: %empty  */
#line 950 "src/Slice/Grammar.y"
{
    if (yyvsp[-2])
    {
        currentUnit->popContainer();
    }
    yyerrok;
}
#line 2806 "src/Slice/Grammar.cpp"
    break;

  case 76: /* operation: operation_preamble error ')' @7 throws  */
#line 958 "src/Slice/Grammar.y"
{
    auto op = dynamic_pointer_cast<Operation>(yyvsp[-1]);
    auto el = dynamic_pointer_cast<ExceptionListTok>(yyvsp[0]);
    assert(el);
    if (op)
    {
        op->setExceptionList(el->v); // Dummy
    }
}
#line 2820 "src/Slice/Grammar.cpp"
    break;

  case 77: /* interface_id: "interface keyword" "identifier"  */
#line 973 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2828 "src/Slice/Grammar.cpp"
    break;

  case 78: /* interface_id: "interface keyword" keyword  */
#line 977 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as interface name");
    yyval = yyvsp[0]; // Dummy
}
#line 2838 "src/Slice/Grammar.cpp"
    break;

  case 79: /* interface_decl: interface_id  */
#line 988 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto cont = currentUnit->currentContainer();
    InterfaceDeclPtr cl = cont->createInterfaceDecl(ident->v);
    cont->checkHasChangedMeaning(ident->v, cl);
    yyval = cl;
}
#line 2850 "src/Slice/Grammar.cpp"
    break;

  case 80: /* @8: %empty  */
#line 1001 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    ContainerPtr cont = currentUnit->currentContainer();
    auto bases = dynamic_pointer_cast<InterfaceListTok>(yyvsp[0]);
    InterfaceDefPtr interface = cont->createInterfaceDef(ident->v, bases->v);
    if (interface)
    {
        cont->checkHasChangedMeaning(ident->v, interface);
        currentUnit->pushContainer(interface);
        yyval = interface;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 2871 "src/Slice/Grammar.cpp"
    break;

  case 81: /* interface_def: interface_id interface_extends @8 '{' operations '}'  */
#line 1018 "src/Slice/Grammar.y"
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
#line 2887 "src/Slice/Grammar.cpp"
    break;

  case 82: /* interface_list: interface_list ',' scoped_name  */
#line 1035 "src/Slice/Grammar.y"
{
    auto interfaces = dynamic_pointer_cast<InterfaceListTok>(yyvsp[-2]);
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    if (auto interfaceDef = lookupInterfaceByName(scoped->v))
    {
        interfaces->v.push_back(interfaceDef);
    }
    yyval = interfaces;
}
#line 2901 "src/Slice/Grammar.cpp"
    break;

  case 83: /* interface_list: scoped_name  */
#line 1045 "src/Slice/Grammar.y"
{
    auto interfaces = make_shared<InterfaceListTok>();
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    if (auto interfaceDef = lookupInterfaceByName(scoped->v))
    {
        interfaces->v.push_back(interfaceDef);
    }
    yyval = interfaces;
}
#line 2915 "src/Slice/Grammar.cpp"
    break;

  case 84: /* interface_list: "Object keyword"  */
#line 1055 "src/Slice/Grammar.y"
{
    currentUnit->error("illegal inheritance from type Object");
    yyval = make_shared<InterfaceListTok>(); // Dummy
}
#line 2924 "src/Slice/Grammar.cpp"
    break;

  case 85: /* interface_list: "Value keyword"  */
#line 1060 "src/Slice/Grammar.y"
{
    currentUnit->error("illegal inheritance from type Value");
    yyval = make_shared<InterfaceListTok>(); // Dummy
}
#line 2933 "src/Slice/Grammar.cpp"
    break;

  case 86: /* interface_extends: extends interface_list  */
#line 1070 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 2941 "src/Slice/Grammar.cpp"
    break;

  case 87: /* interface_extends: %empty  */
#line 1074 "src/Slice/Grammar.y"
{
    yyval = make_shared<InterfaceListTok>();
}
#line 2949 "src/Slice/Grammar.cpp"
    break;

  case 88: /* operations: metadata operation ';' operations  */
#line 1083 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto contained = dynamic_pointer_cast<Contained>(yyvsp[-2]);
    if (contained && !metadata->v.empty())
    {
        contained->appendMetadata(std::move(metadata->v));
    }
}
#line 2962 "src/Slice/Grammar.cpp"
    break;

  case 89: /* operations: error ';' operations  */
#line 1092 "src/Slice/Grammar.y"
{
}
#line 2969 "src/Slice/Grammar.cpp"
    break;

  case 90: /* operations: metadata operation  */
#line 1095 "src/Slice/Grammar.y"
{
    currentUnit->error("';' missing after definition");
}
#line 2977 "src/Slice/Grammar.cpp"
    break;

  case 91: /* operations: %empty  */
#line 1099 "src/Slice/Grammar.y"
{
}
#line 2984 "src/Slice/Grammar.cpp"
    break;

  case 92: /* exception_list: exception_list ',' exception  */
#line 1107 "src/Slice/Grammar.y"
{
    auto exceptionList = dynamic_pointer_cast<ExceptionListTok>(yyvsp[-2]);
    auto exception = dynamic_pointer_cast<Exception>(yyvsp[0]);
    if (exception)
    {
        exceptionList->v.push_back(exception);
    }
    yyval = exceptionList;
}
#line 2998 "src/Slice/Grammar.cpp"
    break;

  case 93: /* exception_list: exception  */
#line 1117 "src/Slice/Grammar.y"
{
    auto exceptionList = make_shared<ExceptionListTok>();
    auto exception = dynamic_pointer_cast<Exception>(yyvsp[0]);
    if (exception)
    {
        exceptionList->v.push_back(exception);
    }
    yyval = exceptionList;
}
#line 3012 "src/Slice/Grammar.cpp"
    break;

  case 94: /* exception: scoped_name  */
#line 1132 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ExceptionPtr exception = cont->lookupException(scoped->v, true);
    if (exception)
    {
        cont->checkHasChangedMeaning(scoped->v, exception);
    }
    yyval = exception;
}
#line 3027 "src/Slice/Grammar.cpp"
    break;

  case 95: /* exception: keyword  */
#line 1143 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as a name");
    ContainerPtr cont = currentUnit->currentContainer();
    ExceptionPtr exception = cont->lookupException(ident->v, false);
    if (exception)
    {
        cont->checkHasChangedMeaning(ident->v, exception);
    }
    yyval = exception;
}
#line 3043 "src/Slice/Grammar.cpp"
    break;

  case 96: /* sequence_def: "sequence keyword" '<' metadata type '>' definition_name  */
#line 1160 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto type = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createSequence(ident->v, type, std::move(metadata->v));
}
#line 3055 "src/Slice/Grammar.cpp"
    break;

  case 97: /* dictionary_def: "dictionary keyword" '<' metadata type ',' metadata type '>' definition_name  */
#line 1173 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto keyMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-6]);
    auto keyType = dynamic_pointer_cast<Type>(yyvsp[-5]);
    auto valueMetadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-3]);
    auto valueType = dynamic_pointer_cast<Type>(yyvsp[-2]);
    ContainerPtr cont = currentUnit->currentContainer();
    yyval = cont->createDictionary(ident->v, keyType, std::move(keyMetadata->v), valueType, std::move(valueMetadata->v));
}
#line 3069 "src/Slice/Grammar.cpp"
    break;

  case 98: /* @9: %empty  */
#line 1188 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    EnumPtr en = cont->createEnum(ident->v);
    cont->checkHasChangedMeaning(ident->v, en);
    currentUnit->pushContainer(en);
    yyval = en;
}
#line 3082 "src/Slice/Grammar.cpp"
    break;

  case 99: /* enum_def: "enum keyword" definition_name @9 '{' enumerators '}'  */
#line 1197 "src/Slice/Grammar.y"
{
    auto en = dynamic_pointer_cast<Enum>(yyvsp[-3]);
    auto enumerators = dynamic_pointer_cast<EnumeratorListTok>(yyvsp[-1]);
    if (enumerators->v.empty())
    {
        currentUnit->error("enum '" + en->name() + "' must have at least one enumerator");
    }
    currentUnit->popContainer();
    yyval = en;
}
#line 3097 "src/Slice/Grammar.cpp"
    break;

  case 101: /* enumerators: enumerator_list ','  */
#line 1214 "src/Slice/Grammar.y"
{
    yyval = yyvsp[-1];
}
#line 3105 "src/Slice/Grammar.cpp"
    break;

  case 102: /* enumerators: %empty  */
#line 1218 "src/Slice/Grammar.y"
{
    yyval = make_shared<EnumeratorListTok>(); // Empty list
}
#line 3113 "src/Slice/Grammar.cpp"
    break;

  case 103: /* enumerator_list: enumerator_list ',' metadata enumerator  */
#line 1227 "src/Slice/Grammar.y"
{
    auto enumeratorList = dynamic_pointer_cast<EnumeratorListTok>(yyvsp[-3]);
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto enumerator = dynamic_pointer_cast<Enumerator>(yyvsp[0]);
    if (enumerator && !metadata->v.empty())
    {
        enumerator->appendMetadata(std::move(metadata->v));
    }
    enumeratorList->v.push_back(enumerator);
    yyval = enumeratorList;
}
#line 3129 "src/Slice/Grammar.cpp"
    break;

  case 104: /* enumerator_list: metadata enumerator  */
#line 1239 "src/Slice/Grammar.y"
{
    auto enumeratorList = make_shared<EnumeratorListTok>();
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
    auto enumerator = dynamic_pointer_cast<Enumerator>(yyvsp[0]);
    if (enumerator && !metadata->v.empty())
    {
        enumerator->appendMetadata(std::move(metadata->v));
    }
    enumeratorList->v.push_back(enumerator);
    yyval = enumeratorList;
}
#line 3145 "src/Slice/Grammar.cpp"
    break;

  case 105: /* enumerator: "identifier"  */
#line 1256 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    yyval = cont->createEnumerator(ident->v, nullopt);
}
#line 3155 "src/Slice/Grammar.cpp"
    break;

  case 106: /* enumerator: "identifier" '=' integer_constant  */
#line 1262 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
    if (intVal)
    {
        // We report numbers that are out of range, but always create the enumerator no matter what.
        checkIntegerBounds(intVal, "enumerator value");
        yyval = cont->createEnumerator(ident->v, static_cast<int32_t>(intVal->v));
    }
    else
    {
        yyval = cont->createEnumerator(ident->v, nullopt); // Dummy
    }
}
#line 3175 "src/Slice/Grammar.cpp"
    break;

  case 107: /* enumerator: keyword  */
#line 1278 "src/Slice/Grammar.y"
{
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    EnumPtr cont = dynamic_pointer_cast<Enum>(currentUnit->currentContainer());
    currentUnit->error("keyword '" + ident->v + "' cannot be used as enumerator");
    yyval = cont->createEnumerator(ident->v, nullopt); // Dummy
}
#line 3186 "src/Slice/Grammar.cpp"
    break;

  case 108: /* parameter: optional_type_id  */
#line 1290 "src/Slice/Grammar.y"
{
    auto tsp = dynamic_pointer_cast<OptionalDefTok>(yyvsp[0]);
    ParameterPtr param;

    auto op = dynamic_pointer_cast<Operation>(currentUnit->currentContainer());
    if (op)
    {
        param = op->createParameter(tsp->name, tsp->type, tsp->isOptional, tsp->tag);
        currentUnit->currentContainer()->checkHasChangedMeaning(tsp->name, param);
    }
    yyval = param;
}
#line 3203 "src/Slice/Grammar.cpp"
    break;

  case 109: /* parameter: "out keyword" parameter  */
#line 1303 "src/Slice/Grammar.y"
{
    if (auto param = dynamic_pointer_cast<Parameter>(yyvsp[0]))
    {
        param->setIsOutParam();
    }
    yyval = yyvsp[0];
}
#line 3215 "src/Slice/Grammar.cpp"
    break;

  case 110: /* parameter: local_metadata parameter  */
#line 1311 "src/Slice/Grammar.y"
{
    if (auto param = dynamic_pointer_cast<Parameter>(yyvsp[0]))
    {
        auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-1]);
        param->appendMetadata(std::move(metadata->v));
    }
    yyval = yyvsp[0];
}
#line 3228 "src/Slice/Grammar.cpp"
    break;

  case 115: /* throws: "throws keyword" exception_list  */
#line 1339 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3236 "src/Slice/Grammar.cpp"
    break;

  case 116: /* throws: %empty  */
#line 1343 "src/Slice/Grammar.y"
{
    yyval = make_shared<ExceptionListTok>();
}
#line 3244 "src/Slice/Grammar.cpp"
    break;

  case 117: /* scoped_name: "identifier"  */
#line 1352 "src/Slice/Grammar.y"
{
}
#line 3251 "src/Slice/Grammar.cpp"
    break;

  case 118: /* scoped_name: "scoped identifier"  */
#line 1355 "src/Slice/Grammar.y"
{
}
#line 3258 "src/Slice/Grammar.cpp"
    break;

  case 119: /* builtin: "bool keyword"  */
#line 1362 "src/Slice/Grammar.y"
           {}
#line 3264 "src/Slice/Grammar.cpp"
    break;

  case 120: /* builtin: "byte keyword"  */
#line 1363 "src/Slice/Grammar.y"
           {}
#line 3270 "src/Slice/Grammar.cpp"
    break;

  case 121: /* builtin: "short keyword"  */
#line 1364 "src/Slice/Grammar.y"
            {}
#line 3276 "src/Slice/Grammar.cpp"
    break;

  case 122: /* builtin: "int keyword"  */
#line 1365 "src/Slice/Grammar.y"
          {}
#line 3282 "src/Slice/Grammar.cpp"
    break;

  case 123: /* builtin: "long keyword"  */
#line 1366 "src/Slice/Grammar.y"
           {}
#line 3288 "src/Slice/Grammar.cpp"
    break;

  case 124: /* builtin: "float keyword"  */
#line 1367 "src/Slice/Grammar.y"
            {}
#line 3294 "src/Slice/Grammar.cpp"
    break;

  case 125: /* builtin: "double keyword"  */
#line 1368 "src/Slice/Grammar.y"
             {}
#line 3300 "src/Slice/Grammar.cpp"
    break;

  case 126: /* builtin: "string keyword"  */
#line 1369 "src/Slice/Grammar.y"
             {}
#line 3306 "src/Slice/Grammar.cpp"
    break;

  case 127: /* builtin: "Object keyword"  */
#line 1370 "src/Slice/Grammar.y"
             {}
#line 3312 "src/Slice/Grammar.cpp"
    break;

  case 128: /* builtin: "Value keyword"  */
#line 1371 "src/Slice/Grammar.y"
            {}
#line 3318 "src/Slice/Grammar.cpp"
    break;

  case 129: /* type: "Object keyword" '*'  */
#line 1377 "src/Slice/Grammar.y"
{
    yyval = currentUnit->createBuiltin(Builtin::KindObjectProxy);
}
#line 3326 "src/Slice/Grammar.cpp"
    break;

  case 130: /* type: builtin  */
#line 1381 "src/Slice/Grammar.y"
{
    auto typeName = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    yyval = currentUnit->createBuiltin(Builtin::kindFromString(typeName->v).value());
}
#line 3335 "src/Slice/Grammar.cpp"
    break;

  case 131: /* type: scoped_name  */
#line 1386 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    yyval = lookupTypeByName(scoped->v, false);
}
#line 3344 "src/Slice/Grammar.cpp"
    break;

  case 132: /* type: scoped_name '*'  */
#line 1391 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    yyval = lookupTypeByName(scoped->v, true);
}
#line 3353 "src/Slice/Grammar.cpp"
    break;

  case 133: /* integer_constant: "integer literal"  */
#line 1401 "src/Slice/Grammar.y"
{
    yyval = yyvsp[0];
}
#line 3361 "src/Slice/Grammar.cpp"
    break;

  case 134: /* integer_constant: scoped_name  */
#line 1405 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ContainerPtr cont = currentUnit->currentContainer();
    ContainedList cl = cont->lookupContained(scoped->v, false);

    if (cl.empty())
    {
        EnumeratorList enumerators = cont->enumerators(scoped->v);
        if (enumerators.size() == 1)
        {
            // We found the enumerator the user must of been referencing.
            cl.push_back(enumerators.front());
            scoped->v = enumerators.front()->scoped();
        }
        else if (enumerators.size() > 1)
        {
            // There are multiple enumerators and it's ambiguous which to use.
            bool first = true;
            ostringstream os;
            os << "enumerator '" << scoped->v << "' could designate";
            for (const auto& p : enumerators)
            {
                if (!first)
                {
                    os << " or";
                }
                first = false;

                os << " '" << p->scoped() << "'";
            }
            currentUnit->error(os.str());

            // Use the first enumerator we found, so parsing can continue.
            cl.push_back(enumerators.front());
        }
    }

    optional<int64_t> integerValue;
    if (cl.empty())
    {
        // If we couldn't find any Slice types matching the provided name, report an error.
        currentUnit->error("'" + scoped->v + "' is not defined");
    }
    else
    {
        cont->checkHasChangedMeaning(scoped->v);
        if (auto constant = dynamic_pointer_cast<Const>(cl.front()))
        {
            auto b = dynamic_pointer_cast<Builtin>(constant->type());
            if (b && b->isIntegralType())
            {
                integerValue = std::stoll(constant->value(), nullptr, 0);
            }
        }
        else if (auto enumerator = dynamic_pointer_cast<Enumerator>(cl.front()))
        {
            integerValue = enumerator->value();
        }

        // If the provided name resolved to a non-integer-constant Slice type, report an error.
        if (!integerValue)
        {
            currentUnit->error(cl.front()->kindOf() + " '" + scoped->v + "' cannot be used as an integer constant");
        }
    }

    if (integerValue)
    {
        // Return the value that we resolved.
        auto tok = make_shared<IntegerTok>();
        tok->v = *integerValue;
        tok->literal = scoped->v;
        yyval = tok;
    }
    else
    {
        yyval = nullptr;
    }
}
#line 3445 "src/Slice/Grammar.cpp"
    break;

  case 135: /* string_literal: "string literal" string_literal  */
#line 1490 "src/Slice/Grammar.y"
{
    auto str1 = dynamic_pointer_cast<StringTok>(yyvsp[-1]);
    auto str2 = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    str1->v += str2->v;
}
#line 3455 "src/Slice/Grammar.cpp"
    break;

  case 136: /* string_literal: "string literal"  */
#line 1496 "src/Slice/Grammar.y"
{
}
#line 3462 "src/Slice/Grammar.cpp"
    break;

  case 137: /* metadata_list: metadata_list ',' string_literal  */
#line 1504 "src/Slice/Grammar.y"
{
    auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadataList = dynamic_pointer_cast<MetadataListTok>(yyvsp[-2]);

    auto metadata = make_shared<Metadata>(str->v, currentUnit->currentFile(), currentUnit->currentLine());
    metadataList->v.push_back(metadata);

    yyval = metadataList;
}
#line 3476 "src/Slice/Grammar.cpp"
    break;

  case 138: /* metadata_list: string_literal  */
#line 1514 "src/Slice/Grammar.y"
{
    auto str = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto metadataList = make_shared<MetadataListTok>();

    auto metadata = make_shared<Metadata>(str->v, currentUnit->currentFile(), currentUnit->currentLine());
    metadataList->v.push_back(metadata);

    yyval = metadataList;
}
#line 3490 "src/Slice/Grammar.cpp"
    break;

  case 139: /* const_initializer: "integer literal"  */
#line 1529 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindLong);
    auto intVal = dynamic_pointer_cast<IntegerTok>(yyvsp[0]);
    ostringstream sstr;
    sstr << intVal->v;
    auto def = make_shared<ConstDefTok>(type, sstr.str());
    yyval = def;
}
#line 3503 "src/Slice/Grammar.cpp"
    break;

  case 140: /* const_initializer: "floating-point literal"  */
#line 1538 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindDouble);
    auto floatVal = dynamic_pointer_cast<FloatingTok>(yyvsp[0]);
    ostringstream sstr;
    sstr << floatVal->v;
    auto def = make_shared<ConstDefTok>(type, sstr.str());
    yyval = def;
}
#line 3516 "src/Slice/Grammar.cpp"
    break;

  case 141: /* const_initializer: scoped_name  */
#line 1547 "src/Slice/Grammar.y"
{
    auto scoped = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    ConstDefTokPtr def;
    ContainedList cl = currentUnit->currentContainer()->lookupContained(scoped->v, false);
    if (cl.empty())
    {
        // Could be an enumerator
        def = make_shared<ConstDefTok>(nullptr, scoped->v);
    }
    else
    {
        auto enumerator = dynamic_pointer_cast<Enumerator>(cl.front());
        auto constant = dynamic_pointer_cast<Const>(cl.front());
        if (enumerator)
        {
            currentUnit->currentContainer()->checkHasChangedMeaning(scoped->v, enumerator);
            def = make_shared<ConstDefTok>(enumerator, scoped->v);
        }
        else if (constant)
        {
            currentUnit->currentContainer()->checkHasChangedMeaning(scoped->v, constant);
            def = make_shared<ConstDefTok>(constant, constant->value());
        }
        else
        {
            def = make_shared<ConstDefTok>();
            string msg = "illegal initializer: '" + scoped->v + "' is ";
            string kindOf = cl.front()->kindOf();
            msg += getArticleFor(kindOf) + " " + kindOf;
            currentUnit->error(msg); // $$ is dummy
        }
    }
    yyval = def;
}
#line 3555 "src/Slice/Grammar.cpp"
    break;

  case 142: /* const_initializer: "string literal"  */
#line 1582 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindString);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, literal->v);
    yyval = def;
}
#line 3566 "src/Slice/Grammar.cpp"
    break;

  case 143: /* const_initializer: "false keyword"  */
#line 1589 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindBool);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, "false");
    yyval = def;
}
#line 3577 "src/Slice/Grammar.cpp"
    break;

  case 144: /* const_initializer: "true keyword"  */
#line 1596 "src/Slice/Grammar.y"
{
    BuiltinPtr type = currentUnit->createBuiltin(Builtin::KindBool);
    auto literal = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    auto def = make_shared<ConstDefTok>(type, "true");
    yyval = def;
}
#line 3588 "src/Slice/Grammar.cpp"
    break;

  case 145: /* const_def: "const keyword" metadata type definition_name '=' const_initializer  */
#line 1608 "src/Slice/Grammar.y"
{
    auto metadata = dynamic_pointer_cast<MetadataListTok>(yyvsp[-4]);
    auto const_type = dynamic_pointer_cast<Type>(yyvsp[-3]);
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[-2]);
    auto value = dynamic_pointer_cast<ConstDefTok>(yyvsp[0]);
    yyval = currentUnit->currentContainer()->createConst(ident->v, const_type, std::move(metadata->v), value->v,
                                                      value->valueAsString);
}
#line 3601 "src/Slice/Grammar.cpp"
    break;

  case 146: /* definition_name: "identifier"  */
#line 1622 "src/Slice/Grammar.y"
{
    // All good, this is a valid identifier.
    yyval = yyvsp[0];
}
#line 3610 "src/Slice/Grammar.cpp"
    break;

  case 147: /* definition_name: keyword  */
#line 1627 "src/Slice/Grammar.y"
{
    // If an un-escaped keyword was used as an identifier, we emit an error,
    // but continue along, pretending like the user escaped the keyword.
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as a name");
    yyval = ident;
}
#line 3622 "src/Slice/Grammar.cpp"
    break;

  case 148: /* definition_name: %empty  */
#line 1635 "src/Slice/Grammar.y"
{
    // If the user forgot to give a name to a Slice definition, we emit an error,
    // but continue along, returning an empty string instead of an identifier.
    currentUnit->error("missing name");
    yyval = make_shared<StringTok>();
}
#line 3633 "src/Slice/Grammar.cpp"
    break;

  case 149: /* definition_name_open: "identifier("  */
#line 1647 "src/Slice/Grammar.y"
{
    // All good, this is a valid identifier.
    yyval = yyvsp[0];
}
#line 3642 "src/Slice/Grammar.cpp"
    break;

  case 150: /* definition_name_open: "keyword("  */
#line 1652 "src/Slice/Grammar.y"
{
    // If an un-escaped keyword was used as an identifier, we emit an error,
    // but continue along, pretending like the user escaped the keyword.
    auto ident = dynamic_pointer_cast<StringTok>(yyvsp[0]);
    currentUnit->error("keyword '" + ident->v + "' cannot be used as a name");
    yyval = ident;
}
#line 3654 "src/Slice/Grammar.cpp"
    break;

  case 151: /* keyword: "module keyword"  */
#line 1664 "src/Slice/Grammar.y"
             {}
#line 3660 "src/Slice/Grammar.cpp"
    break;

  case 152: /* keyword: "class keyword"  */
#line 1665 "src/Slice/Grammar.y"
            {}
#line 3666 "src/Slice/Grammar.cpp"
    break;

  case 153: /* keyword: "interface keyword"  */
#line 1666 "src/Slice/Grammar.y"
                {}
#line 3672 "src/Slice/Grammar.cpp"
    break;

  case 154: /* keyword: "exception keyword"  */
#line 1667 "src/Slice/Grammar.y"
                {}
#line 3678 "src/Slice/Grammar.cpp"
    break;

  case 155: /* keyword: "struct keyword"  */
#line 1668 "src/Slice/Grammar.y"
             {}
#line 3684 "src/Slice/Grammar.cpp"
    break;

  case 156: /* keyword: "sequence keyword"  */
#line 1669 "src/Slice/Grammar.y"
               {}
#line 3690 "src/Slice/Grammar.cpp"
    break;

  case 157: /* keyword: "dictionary keyword"  */
#line 1670 "src/Slice/Grammar.y"
                 {}
#line 3696 "src/Slice/Grammar.cpp"
    break;

  case 158: /* keyword: "enum keyword"  */
#line 1671 "src/Slice/Grammar.y"
           {}
#line 3702 "src/Slice/Grammar.cpp"
    break;

  case 159: /* keyword: "out keyword"  */
#line 1672 "src/Slice/Grammar.y"
          {}
#line 3708 "src/Slice/Grammar.cpp"
    break;

  case 160: /* keyword: "extends keyword"  */
#line 1673 "src/Slice/Grammar.y"
              {}
#line 3714 "src/Slice/Grammar.cpp"
    break;

  case 161: /* keyword: "throws keyword"  */
#line 1674 "src/Slice/Grammar.y"
             {}
#line 3720 "src/Slice/Grammar.cpp"
    break;

  case 162: /* keyword: "void keyword"  */
#line 1675 "src/Slice/Grammar.y"
           {}
#line 3726 "src/Slice/Grammar.cpp"
    break;

  case 163: /* keyword: "bool keyword"  */
#line 1676 "src/Slice/Grammar.y"
           {}
#line 3732 "src/Slice/Grammar.cpp"
    break;

  case 164: /* keyword: "byte keyword"  */
#line 1677 "src/Slice/Grammar.y"
           {}
#line 3738 "src/Slice/Grammar.cpp"
    break;

  case 165: /* keyword: "short keyword"  */
#line 1678 "src/Slice/Grammar.y"
            {}
#line 3744 "src/Slice/Grammar.cpp"
    break;

  case 166: /* keyword: "int keyword"  */
#line 1679 "src/Slice/Grammar.y"
          {}
#line 3750 "src/Slice/Grammar.cpp"
    break;

  case 167: /* keyword: "long keyword"  */
#line 1680 "src/Slice/Grammar.y"
           {}
#line 3756 "src/Slice/Grammar.cpp"
    break;

  case 168: /* keyword: "float keyword"  */
#line 1681 "src/Slice/Grammar.y"
            {}
#line 3762 "src/Slice/Grammar.cpp"
    break;

  case 169: /* keyword: "double keyword"  */
#line 1682 "src/Slice/Grammar.y"
             {}
#line 3768 "src/Slice/Grammar.cpp"
    break;

  case 170: /* keyword: "string keyword"  */
#line 1683 "src/Slice/Grammar.y"
             {}
#line 3774 "src/Slice/Grammar.cpp"
    break;

  case 171: /* keyword: "Object keyword"  */
#line 1684 "src/Slice/Grammar.y"
             {}
#line 3780 "src/Slice/Grammar.cpp"
    break;

  case 172: /* keyword: "const keyword"  */
#line 1685 "src/Slice/Grammar.y"
            {}
#line 3786 "src/Slice/Grammar.cpp"
    break;

  case 173: /* keyword: "false keyword"  */
#line 1686 "src/Slice/Grammar.y"
            {}
#line 3792 "src/Slice/Grammar.cpp"
    break;

  case 174: /* keyword: "true keyword"  */
#line 1687 "src/Slice/Grammar.y"
           {}
#line 3798 "src/Slice/Grammar.cpp"
    break;

  case 175: /* keyword: "idempotent keyword"  */
#line 1688 "src/Slice/Grammar.y"
                 {}
#line 3804 "src/Slice/Grammar.cpp"
    break;

  case 176: /* keyword: "optional keyword"  */
#line 1689 "src/Slice/Grammar.y"
               {}
#line 3810 "src/Slice/Grammar.cpp"
    break;

  case 177: /* keyword: "Value keyword"  */
#line 1690 "src/Slice/Grammar.y"
            {}
#line 3816 "src/Slice/Grammar.cpp"
    break;


#line 3820 "src/Slice/Grammar.cpp"

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
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
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
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1693 "src/Slice/Grammar.y"


// NOLINTEND

namespace
{
    TypePtr lookupTypeByName(const string& name, bool expectInterfaceType)
    {
        ContainerPtr cont = currentUnit->currentContainer();
        TypeList types = cont->lookupType(name);
        if (types.empty())
        {
            return nullptr;
        }

        TypePtr firstType = types.front();
        auto interface = dynamic_pointer_cast<InterfaceDecl>(firstType);
        if (interface && !expectInterfaceType)
        {
            string msg = "add a '*' after the interface name to specify its proxy type: '" + name + "*'";
            currentUnit->error(msg);
        }
        if (!interface && expectInterfaceType)
        {
            string msg = "'" + name + "' must be an interface";
            currentUnit->error(msg);
        }

        cont->checkHasChangedMeaning(name);
        return firstType;
    }

    InterfaceDefPtr lookupInterfaceByName(const string& name)
    {
        ContainerPtr cont = currentUnit->currentContainer();
        InterfaceDefPtr interfaceDef = cont->lookupInterfaceDef(name, true);
        if (interfaceDef)
        {
            cont->checkHasChangedMeaning(name);
        }
        return interfaceDef;
    }

    bool checkIntegerBounds(const IntegerTokPtr& token, string_view kindString)
    {
        const int32_t max_value = std::numeric_limits<int32_t>::max();
        optional<string> errorReason;

        // Check if the integer is between the allowed bounds.
        if (token->v < 0)
        {
            errorReason = "cannot be negative";
        }
        else if (token->v > max_value)
        {
            errorReason = "must be less than or equal to '" + std::to_string(max_value) + "'";
        }

        // Report an error if the integer wasn't within bounds.
        if (errorReason)
        {
            ostringstream msg;
            msg << "invalid " << kindString << " '" << token->literal << "'";
            if (isalpha(token->literal[0]))
            {
                msg << " (" << token->v << ")";
            }
            msg << ": " << kindString << "s " << *errorReason;
            currentUnit->error(msg.str());
            return false;
        }
        else
        {
            return true;
        }
    }
}
