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
#define YY_YY_SRC_ICEGRID_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
#    define YYDEBUG 1
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
#define YYMAXDEPTH 10000       // 10000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

// Newer bison versions allow to disable stack resizing by defining yyoverflow.
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

#line 66 "src/IceGrid/Grammar.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
#    define YYTOKENTYPE
enum yytokentype
{
    YYEMPTY = -2,
    YYEOF = 0,                  /* "end of file"  */
    YYerror = 256,              /* error  */
    YYUNDEF = 257,              /* "invalid token"  */
    ICE_GRID_HELP = 258,        /* ICE_GRID_HELP  */
    ICE_GRID_EXIT = 259,        /* ICE_GRID_EXIT  */
    ICE_GRID_APPLICATION = 260, /* ICE_GRID_APPLICATION  */
    ICE_GRID_NODE = 261,        /* ICE_GRID_NODE  */
    ICE_GRID_REGISTRY = 262,    /* ICE_GRID_REGISTRY  */
    ICE_GRID_SERVER = 263,      /* ICE_GRID_SERVER  */
    ICE_GRID_ADAPTER = 264,     /* ICE_GRID_ADAPTER  */
    ICE_GRID_PING = 265,        /* ICE_GRID_PING  */
    ICE_GRID_LOAD = 266,        /* ICE_GRID_LOAD  */
    ICE_GRID_SOCKETS = 267,     /* ICE_GRID_SOCKETS  */
    ICE_GRID_ADD = 268,         /* ICE_GRID_ADD  */
    ICE_GRID_REMOVE = 269,      /* ICE_GRID_REMOVE  */
    ICE_GRID_LIST = 270,        /* ICE_GRID_LIST  */
    ICE_GRID_SHUTDOWN = 271,    /* ICE_GRID_SHUTDOWN  */
    ICE_GRID_STRING = 272,      /* ICE_GRID_STRING  */
    ICE_GRID_START = 273,       /* ICE_GRID_START  */
    ICE_GRID_STOP = 274,        /* ICE_GRID_STOP  */
    ICE_GRID_SIGNAL = 275,      /* ICE_GRID_SIGNAL  */
    ICE_GRID_STDOUT = 276,      /* ICE_GRID_STDOUT  */
    ICE_GRID_STDERR = 277,      /* ICE_GRID_STDERR  */
    ICE_GRID_DESCRIBE = 278,    /* ICE_GRID_DESCRIBE  */
    ICE_GRID_PROPERTIES = 279,  /* ICE_GRID_PROPERTIES  */
    ICE_GRID_PROPERTY = 280,    /* ICE_GRID_PROPERTY  */
    ICE_GRID_STATE = 281,       /* ICE_GRID_STATE  */
    ICE_GRID_PID = 282,         /* ICE_GRID_PID  */
    ICE_GRID_ENDPOINTS = 283,   /* ICE_GRID_ENDPOINTS  */
    ICE_GRID_ACTIVATION = 284,  /* ICE_GRID_ACTIVATION  */
    ICE_GRID_OBJECT = 285,      /* ICE_GRID_OBJECT  */
    ICE_GRID_FIND = 286,        /* ICE_GRID_FIND  */
    ICE_GRID_SHOW = 287,        /* ICE_GRID_SHOW  */
    ICE_GRID_COPYING = 288,     /* ICE_GRID_COPYING  */
    ICE_GRID_WARRANTY = 289,    /* ICE_GRID_WARRANTY  */
    ICE_GRID_DIFF = 290,        /* ICE_GRID_DIFF  */
    ICE_GRID_UPDATE = 291,      /* ICE_GRID_UPDATE  */
    ICE_GRID_INSTANTIATE = 292, /* ICE_GRID_INSTANTIATE  */
    ICE_GRID_TEMPLATE = 293,    /* ICE_GRID_TEMPLATE  */
    ICE_GRID_SERVICE = 294,     /* ICE_GRID_SERVICE  */
    ICE_GRID_ENABLE = 295,      /* ICE_GRID_ENABLE  */
    ICE_GRID_DISABLE = 296      /* ICE_GRID_DISABLE  */
};
typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef std::list<std::string> YYSTYPE;
#    define YYSTYPE_IS_TRIVIAL 1
#    define YYSTYPE_IS_DECLARED 1
#endif

int yyparse(void);

#endif /* !YY_YY_SRC_ICEGRID_GRAMMAR_H_INCLUDED  */
