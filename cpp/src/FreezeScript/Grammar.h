/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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

#ifndef YY_FREEZE_SCRIPT_GRAMMAR_TAB_H_INCLUDED
# define YY_FREEZE_SCRIPT_GRAMMAR_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int freeze_script_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_AND = 258,
    TOK_OR = 259,
    TOK_NOT = 260,
    TOK_ADD = 261,
    TOK_SUB = 262,
    TOK_MUL = 263,
    TOK_DIV = 264,
    TOK_MOD = 265,
    TOK_LPAREN = 266,
    TOK_RPAREN = 267,
    TOK_LBRACKET = 268,
    TOK_RBRACKET = 269,
    TOK_LESS_THAN = 270,
    TOK_GREATER_THAN = 271,
    TOK_LESS_EQUAL = 272,
    TOK_GREATER_EQUAL = 273,
    TOK_EQUAL = 274,
    TOK_NEQ = 275,
    TOK_TRUE = 276,
    TOK_FALSE = 277,
    TOK_NIL = 278,
    TOK_SCOPE_DELIMITER = 279,
    TOK_IDENTIFIER = 280,
    TOK_STRING_LITERAL = 281,
    TOK_INTEGER_LITERAL = 282,
    TOK_FLOATING_POINT_LITERAL = 283,
    UNARY_OP = 284
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int freeze_script_parse (void);

#endif /* !YY_FREEZE_SCRIPT_GRAMMAR_TAB_H_INCLUDED  */
