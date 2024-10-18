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

#ifndef YY_YY_SRC_ICEGRID_GRAMMAR_H_INCLUDED
# define YY_YY_SRC_ICEGRID_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 9 "src/IceGrid/Grammar.y"


#include <list>
#include <string>

// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
#define YYMAXDEPTH  10000      // 10000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

// Newer bison versions allow to disable stack resizing by defining yyoverflow.
#define yyoverflow(a, b, c, d, e, f) yyerror(a)


#line 67 "src/IceGrid/Grammar.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ICEGRID_HELP = 258,            /* ICEGRID_HELP  */
    ICEGRID_EXIT = 259,            /* ICEGRID_EXIT  */
    ICEGRID_APPLICATION = 260,     /* ICEGRID_APPLICATION  */
    ICEGRID_NODE = 261,            /* ICEGRID_NODE  */
    ICEGRID_REGISTRY = 262,        /* ICEGRID_REGISTRY  */
    ICEGRID_SERVER = 263,          /* ICEGRID_SERVER  */
    ICEGRID_ADAPTER = 264,         /* ICEGRID_ADAPTER  */
    ICEGRID_PING = 265,            /* ICEGRID_PING  */
    ICEGRID_LOAD = 266,            /* ICEGRID_LOAD  */
    ICEGRID_SOCKETS = 267,         /* ICEGRID_SOCKETS  */
    ICEGRID_ADD = 268,             /* ICEGRID_ADD  */
    ICEGRID_REMOVE = 269,          /* ICEGRID_REMOVE  */
    ICEGRID_LIST = 270,            /* ICEGRID_LIST  */
    ICEGRID_SHUTDOWN = 271,        /* ICEGRID_SHUTDOWN  */
    ICEGRID_STRING = 272,          /* ICEGRID_STRING  */
    ICEGRID_START = 273,           /* ICEGRID_START  */
    ICEGRID_STOP = 274,            /* ICEGRID_STOP  */
    ICEGRID_SIGNAL = 275,          /* ICEGRID_SIGNAL  */
    ICEGRID_STDOUT = 276,          /* ICEGRID_STDOUT  */
    ICEGRID_STDERR = 277,          /* ICEGRID_STDERR  */
    ICEGRID_DESCRIBE = 278,        /* ICEGRID_DESCRIBE  */
    ICEGRID_PROPERTIES = 279,      /* ICEGRID_PROPERTIES  */
    ICEGRID_PROPERTY = 280,        /* ICEGRID_PROPERTY  */
    ICEGRID_STATE = 281,           /* ICEGRID_STATE  */
    ICEGRID_PID = 282,             /* ICEGRID_PID  */
    ICEGRID_ENDPOINTS = 283,       /* ICEGRID_ENDPOINTS  */
    ICEGRID_ACTIVATION = 284,      /* ICEGRID_ACTIVATION  */
    ICEGRID_OBJECT = 285,          /* ICEGRID_OBJECT  */
    ICEGRID_FIND = 286,            /* ICEGRID_FIND  */
    ICEGRID_SHOW = 287,            /* ICEGRID_SHOW  */
    ICEGRID_COPYING = 288,         /* ICEGRID_COPYING  */
    ICEGRID_WARRANTY = 289,        /* ICEGRID_WARRANTY  */
    ICEGRID_DIFF = 290,            /* ICEGRID_DIFF  */
    ICEGRID_UPDATE = 291,          /* ICEGRID_UPDATE  */
    ICEGRID_INSTANTIATE = 292,     /* ICEGRID_INSTANTIATE  */
    ICEGRID_TEMPLATE = 293,        /* ICEGRID_TEMPLATE  */
    ICEGRID_SERVICE = 294,         /* ICEGRID_SERVICE  */
    ICEGRID_ENABLE = 295,          /* ICEGRID_ENABLE  */
    ICEGRID_DISABLE = 296          /* ICEGRID_DISABLE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef std::list<std::string> YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int yyparse (void);


#endif /* !YY_YY_SRC_ICEGRID_GRAMMAR_H_INCLUDED  */
