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
     TOK_HELP = 258,
     TOK_EXIT = 259,
     TOK_ADD_BOOK = 260,
     TOK_FIND_ISBN = 261,
     TOK_FIND_AUTHORS = 262,
     TOK_NEXT_FOUND_BOOK = 263,
     TOK_PRINT_CURRENT = 264,
     TOK_RENT_BOOK = 265,
     TOK_RETURN_BOOK = 266,
     TOK_REMOVE_CURRENT = 267,
     TOK_SET_EVICTOR_SIZE = 268,
     TOK_SHUTDOWN = 269,
     TOK_STRING = 270
   };
#endif
#define TOK_HELP 258
#define TOK_EXIT 259
#define TOK_ADD_BOOK 260
#define TOK_FIND_ISBN 261
#define TOK_FIND_AUTHORS 262
#define TOK_NEXT_FOUND_BOOK 263
#define TOK_PRINT_CURRENT 264
#define TOK_RENT_BOOK 265
#define TOK_RETURN_BOOK 266
#define TOK_REMOVE_CURRENT 267
#define TOK_SET_EVICTOR_SIZE 268
#define TOK_SHUTDOWN 269
#define TOK_STRING 270




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





