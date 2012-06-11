/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ICE_MODULE = 258,
     ICE_CLASS = 259,
     ICE_INTERFACE = 260,
     ICE_EXCEPTION = 261,
     ICE_STRUCT = 262,
     ICE_SEQUENCE = 263,
     ICE_DICTIONARY = 264,
     ICE_ENUM = 265,
     ICE_OUT = 266,
     ICE_EXTENDS = 267,
     ICE_IMPLEMENTS = 268,
     ICE_THROWS = 269,
     ICE_VOID = 270,
     ICE_BYTE = 271,
     ICE_BOOL = 272,
     ICE_SHORT = 273,
     ICE_INT = 274,
     ICE_LONG = 275,
     ICE_FLOAT = 276,
     ICE_DOUBLE = 277,
     ICE_STRING = 278,
     ICE_OBJECT = 279,
     ICE_LOCAL_OBJECT = 280,
     ICE_LOCAL = 281,
     ICE_CONST = 282,
     ICE_FALSE = 283,
     ICE_TRUE = 284,
     ICE_IDEMPOTENT = 285,
     ICE_OPTIONAL = 286,
     ICE_SCOPE_DELIMITER = 287,
     ICE_IDENTIFIER = 288,
     ICE_STRING_LITERAL = 289,
     ICE_INTEGER_LITERAL = 290,
     ICE_FLOATING_POINT_LITERAL = 291,
     ICE_IDENT_OP = 292,
     ICE_KEYWORD_OP = 293,
     ICE_OPTIONAL_OP = 294,
     ICE_METADATA_OPEN = 295,
     ICE_METADATA_CLOSE = 296,
     ICE_GLOBAL_METADATA_OPEN = 297,
     ICE_GLOBAL_METADATA_CLOSE = 298,
     BAD_CHAR = 299
   };
#endif
/* Tokens.  */
#define ICE_MODULE 258
#define ICE_CLASS 259
#define ICE_INTERFACE 260
#define ICE_EXCEPTION 261
#define ICE_STRUCT 262
#define ICE_SEQUENCE 263
#define ICE_DICTIONARY 264
#define ICE_ENUM 265
#define ICE_OUT 266
#define ICE_EXTENDS 267
#define ICE_IMPLEMENTS 268
#define ICE_THROWS 269
#define ICE_VOID 270
#define ICE_BYTE 271
#define ICE_BOOL 272
#define ICE_SHORT 273
#define ICE_INT 274
#define ICE_LONG 275
#define ICE_FLOAT 276
#define ICE_DOUBLE 277
#define ICE_STRING 278
#define ICE_OBJECT 279
#define ICE_LOCAL_OBJECT 280
#define ICE_LOCAL 281
#define ICE_CONST 282
#define ICE_FALSE 283
#define ICE_TRUE 284
#define ICE_IDEMPOTENT 285
#define ICE_OPTIONAL 286
#define ICE_SCOPE_DELIMITER 287
#define ICE_IDENTIFIER 288
#define ICE_STRING_LITERAL 289
#define ICE_INTEGER_LITERAL 290
#define ICE_FLOATING_POINT_LITERAL 291
#define ICE_IDENT_OP 292
#define ICE_KEYWORD_OP 293
#define ICE_OPTIONAL_OP 294
#define ICE_METADATA_OPEN 295
#define ICE_METADATA_CLOSE 296
#define ICE_GLOBAL_METADATA_OPEN 297
#define ICE_GLOBAL_METADATA_CLOSE 298
#define BAD_CHAR 299




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



