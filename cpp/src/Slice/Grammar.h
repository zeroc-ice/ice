/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_SLICE_SRC_SLICE_GRAMMAR_H_INCLUDED
# define YY_SLICE_SRC_SLICE_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int slice_debug;
#endif
/* "%code requires" blocks.  */
#line 9 "src/Slice/Grammar.y"


// Included first to get 'TokenContext' which we need to define YYLTYPE before flex does.
#include "GrammarUtil.h"

// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
#define YYMAXDEPTH  10000      // 10000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

// Newer bison versions allow to disable stack resizing by defining yyoverflow.
#define yyoverflow(a, b, c, d, e, f, g, h) yyerror(a)


#line 67 "src/Slice/Grammar.h"

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
    ICE_THROWS = 268,              /* ICE_THROWS  */
    ICE_VOID = 269,                /* ICE_VOID  */
    ICE_BOOL = 270,                /* ICE_BOOL  */
    ICE_BYTE = 271,                /* ICE_BYTE  */
    ICE_SHORT = 272,               /* ICE_SHORT  */
    ICE_INT = 273,                 /* ICE_INT  */
    ICE_LONG = 274,                /* ICE_LONG  */
    ICE_FLOAT = 275,               /* ICE_FLOAT  */
    ICE_DOUBLE = 276,              /* ICE_DOUBLE  */
    ICE_STRING = 277,              /* ICE_STRING  */
    ICE_OBJECT = 278,              /* ICE_OBJECT  */
    ICE_CONST = 279,               /* ICE_CONST  */
    ICE_FALSE = 280,               /* ICE_FALSE  */
    ICE_TRUE = 281,                /* ICE_TRUE  */
    ICE_IDEMPOTENT = 282,          /* ICE_IDEMPOTENT  */
    ICE_OPTIONAL = 283,            /* ICE_OPTIONAL  */
    ICE_VALUE = 284,               /* ICE_VALUE  */
    ICE_STRING_LITERAL = 285,      /* ICE_STRING_LITERAL  */
    ICE_INTEGER_LITERAL = 286,     /* ICE_INTEGER_LITERAL  */
    ICE_FLOATING_POINT_LITERAL = 287, /* ICE_FLOATING_POINT_LITERAL  */
    ICE_IDENTIFIER = 288,          /* ICE_IDENTIFIER  */
    ICE_SCOPED_IDENTIFIER = 289,   /* ICE_SCOPED_IDENTIFIER  */
    ICE_METADATA_OPEN = 290,       /* ICE_METADATA_OPEN  */
    ICE_METADATA_CLOSE = 291,      /* ICE_METADATA_CLOSE  */
    ICE_FILE_METADATA_OPEN = 292,  /* ICE_FILE_METADATA_OPEN  */
    ICE_FILE_METADATA_CLOSE = 293, /* ICE_FILE_METADATA_CLOSE  */
    ICE_IDENT_OPEN = 294,          /* ICE_IDENT_OPEN  */
    ICE_KEYWORD_OPEN = 295,        /* ICE_KEYWORD_OPEN  */
    ICE_OPTIONAL_OPEN = 296,       /* ICE_OPTIONAL_OPEN  */
    BAD_TOKEN = 297                /* BAD_TOKEN  */
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
typedef Slice::TokenContext YYLTYPE;




int slice_parse (void);


#endif /* !YY_SLICE_SRC_SLICE_GRAMMAR_H_INCLUDED  */
