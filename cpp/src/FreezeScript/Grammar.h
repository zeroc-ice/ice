/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
#define TOK_AND 258
#define TOK_OR 259
#define TOK_NOT 260
#define TOK_ADD 261
#define TOK_SUB 262
#define TOK_MUL 263
#define TOK_DIV 264
#define TOK_MOD 265
#define TOK_LPAREN 266
#define TOK_RPAREN 267
#define TOK_LBRACKET 268
#define TOK_RBRACKET 269
#define TOK_LESS_THAN 270
#define TOK_GREATER_THAN 271
#define TOK_LESS_EQUAL 272
#define TOK_GREATER_EQUAL 273
#define TOK_EQUAL 274
#define TOK_NEQ 275
#define TOK_TRUE 276
#define TOK_FALSE 277
#define TOK_NIL 278
#define TOK_SCOPE_DELIMITER 279
#define TOK_IDENTIFIER 280
#define TOK_STRING_LITERAL 281
#define TOK_INTEGER_LITERAL 282
#define TOK_FLOATING_POINT_LITERAL 283
#define UNARY_OP 284




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





