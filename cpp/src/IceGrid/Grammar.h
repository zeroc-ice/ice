/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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

#ifndef YY_YY_SRC_ICEGRID_GRAMMAR_HPP_INCLUDED
# define YY_YY_SRC_ICEGRID_GRAMMAR_HPP_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ICE_GRID_HELP = 258,
     ICE_GRID_EXIT = 259,
     ICE_GRID_APPLICATION = 260,
     ICE_GRID_NODE = 261,
     ICE_GRID_REGISTRY = 262,
     ICE_GRID_SERVER = 263,
     ICE_GRID_ADAPTER = 264,
     ICE_GRID_PING = 265,
     ICE_GRID_LOAD = 266,
     ICE_GRID_SOCKETS = 267,
     ICE_GRID_ADD = 268,
     ICE_GRID_REMOVE = 269,
     ICE_GRID_LIST = 270,
     ICE_GRID_SHUTDOWN = 271,
     ICE_GRID_STRING = 272,
     ICE_GRID_START = 273,
     ICE_GRID_STOP = 274,
     ICE_GRID_PATCH = 275,
     ICE_GRID_SIGNAL = 276,
     ICE_GRID_STDOUT = 277,
     ICE_GRID_STDERR = 278,
     ICE_GRID_DESCRIBE = 279,
     ICE_GRID_PROPERTIES = 280,
     ICE_GRID_PROPERTY = 281,
     ICE_GRID_STATE = 282,
     ICE_GRID_PID = 283,
     ICE_GRID_ENDPOINTS = 284,
     ICE_GRID_ACTIVATION = 285,
     ICE_GRID_OBJECT = 286,
     ICE_GRID_FIND = 287,
     ICE_GRID_SHOW = 288,
     ICE_GRID_COPYING = 289,
     ICE_GRID_WARRANTY = 290,
     ICE_GRID_DIFF = 291,
     ICE_GRID_UPDATE = 292,
     ICE_GRID_INSTANTIATE = 293,
     ICE_GRID_TEMPLATE = 294,
     ICE_GRID_SERVICE = 295,
     ICE_GRID_ENABLE = 296,
     ICE_GRID_DISABLE = 297
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_SRC_ICEGRID_GRAMMAR_HPP_INCLUDED  */
