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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



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
     ICE_GRID_ADD = 267,
     ICE_GRID_REMOVE = 268,
     ICE_GRID_LIST = 269,
     ICE_GRID_SHUTDOWN = 270,
     ICE_GRID_STRING = 271,
     ICE_GRID_START = 272,
     ICE_GRID_STOP = 273,
     ICE_GRID_PATCH = 274,
     ICE_GRID_SIGNAL = 275,
     ICE_GRID_STDOUT = 276,
     ICE_GRID_STDERR = 277,
     ICE_GRID_DESCRIBE = 278,
     ICE_GRID_PROPERTIES = 279,
     ICE_GRID_PROPERTY = 280,
     ICE_GRID_STATE = 281,
     ICE_GRID_PID = 282,
     ICE_GRID_ENDPOINTS = 283,
     ICE_GRID_ACTIVATION = 284,
     ICE_GRID_OBJECT = 285,
     ICE_GRID_FIND = 286,
     ICE_GRID_SHOW = 287,
     ICE_GRID_COPYING = 288,
     ICE_GRID_WARRANTY = 289,
     ICE_GRID_DIFF = 290,
     ICE_GRID_UPDATE = 291,
     ICE_GRID_INSTANTIATE = 292,
     ICE_GRID_TEMPLATE = 293,
     ICE_GRID_SERVICE = 294,
     ICE_GRID_ENABLE = 295,
     ICE_GRID_DISABLE = 296
   };
#endif
#define ICE_GRID_HELP 258
#define ICE_GRID_EXIT 259
#define ICE_GRID_APPLICATION 260
#define ICE_GRID_NODE 261
#define ICE_GRID_REGISTRY 262
#define ICE_GRID_SERVER 263
#define ICE_GRID_ADAPTER 264
#define ICE_GRID_PING 265
#define ICE_GRID_LOAD 266
#define ICE_GRID_ADD 267
#define ICE_GRID_REMOVE 268
#define ICE_GRID_LIST 269
#define ICE_GRID_SHUTDOWN 270
#define ICE_GRID_STRING 271
#define ICE_GRID_START 272
#define ICE_GRID_STOP 273
#define ICE_GRID_PATCH 274
#define ICE_GRID_SIGNAL 275
#define ICE_GRID_STDOUT 276
#define ICE_GRID_STDERR 277
#define ICE_GRID_DESCRIBE 278
#define ICE_GRID_PROPERTIES 279
#define ICE_GRID_PROPERTY 280
#define ICE_GRID_STATE 281
#define ICE_GRID_PID 282
#define ICE_GRID_ENDPOINTS 283
#define ICE_GRID_ACTIVATION 284
#define ICE_GRID_OBJECT 285
#define ICE_GRID_FIND 286
#define ICE_GRID_SHOW 287
#define ICE_GRID_COPYING 288
#define ICE_GRID_WARRANTY 289
#define ICE_GRID_DIFF 290
#define ICE_GRID_UPDATE 291
#define ICE_GRID_INSTANTIATE 292
#define ICE_GRID_TEMPLATE 293
#define ICE_GRID_SERVICE 294
#define ICE_GRID_ENABLE 295
#define ICE_GRID_DISABLE 296




/* Copy the first part of user declarations.  */
#line 1 "Grammar.y"


// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Parser.h>

#ifdef _MSC_VER
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

void
yyerror(const char* s)
{
    parser->invalidCommand(s);
}



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 203 "Grammar.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2510

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  43
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  7
/* YYNRULES -- Number of rules. */
#define YYNRULES  169
/* YYNRULES -- Number of states. */
#define YYNSTATES  361

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   296

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
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
      35,    36,    37,    38,    39,    40,    41
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     6,    10,    13,    14,    17,    20,
      25,    30,    35,    40,    45,    50,    55,    60,    65,    70,
      75,    80,    85,    90,    96,   102,   108,   114,   119,   125,
     131,   136,   141,   146,   151,   156,   161,   166,   171,   176,
     181,   186,   191,   196,   201,   206,   211,   216,   221,   226,
     231,   236,   241,   246,   251,   256,   261,   266,   271,   276,
     281,   286,   291,   296,   301,   306,   311,   316,   321,   326,
     331,   336,   341,   346,   351,   356,   361,   366,   371,   376,
     381,   386,   391,   396,   401,   406,   411,   416,   421,   426,
     431,   436,   441,   446,   451,   456,   461,   466,   471,   476,
     481,   486,   491,   496,   501,   506,   511,   516,   521,   526,
     531,   536,   541,   546,   550,   554,   558,   563,   569,   575,
     582,   587,   591,   595,   600,   604,   608,   611,   613,   616,
     619,   623,   627,   628,   630,   632,   634,   636,   638,   640,
     642,   644,   646,   648,   650,   652,   654,   656,   658,   660,
     662,   664,   666,   668,   670,   672,   674,   676,   678,   680,
     682,   684,   686,   688,   690,   692,   694,   696,   698,   700
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      44,     0,    -1,    45,    -1,    -1,    45,    46,    47,    -1,
      46,    47,    -1,    -1,     3,    42,    -1,     4,    42,    -1,
       5,    12,    48,    42,    -1,     5,    12,     3,    42,    -1,
       5,    13,    48,    42,    -1,     5,    13,     3,    42,    -1,
       5,    35,    48,    42,    -1,     5,    35,     3,    42,    -1,
       5,    36,    48,    42,    -1,     5,    36,     3,    42,    -1,
       5,    23,    48,    42,    -1,     5,    23,     3,    42,    -1,
       5,    19,    48,    42,    -1,     5,    19,     3,    42,    -1,
       5,    14,    48,    42,    -1,     5,    14,     3,    42,    -1,
       8,    38,    23,    48,    42,    -1,     8,    38,    23,     3,
      42,    -1,     8,    38,    37,    48,    42,    -1,     8,    38,
      37,     3,    42,    -1,     8,    38,     3,    42,    -1,    39,
      38,    23,    48,    42,    -1,    39,    38,    23,     3,    42,
      -1,    39,    38,     3,    42,    -1,     6,    23,    48,    42,
      -1,     6,    23,     3,    42,    -1,     6,    10,    48,    42,
      -1,     6,    10,     3,    42,    -1,     6,    11,    48,    42,
      -1,     6,    11,     3,    42,    -1,     6,    15,    48,    42,
      -1,     6,    15,     3,    42,    -1,     6,    14,    48,    42,
      -1,     6,    14,     3,    42,    -1,     6,    32,    48,    42,
      -1,     6,    32,     3,    42,    -1,     7,    23,    48,    42,
      -1,     7,    23,     3,    42,    -1,     7,    10,    48,    42,
      -1,     7,    10,     3,    42,    -1,     7,    15,    48,    42,
      -1,     7,    15,     3,    42,    -1,     7,    14,    48,    42,
      -1,     7,    14,     3,    42,    -1,     7,    32,    48,    42,
      -1,     7,    32,     3,    42,    -1,     8,    13,    48,    42,
      -1,     8,    13,     3,    42,    -1,     8,    23,    48,    42,
      -1,     8,    23,     3,    42,    -1,     8,    17,    48,    42,
      -1,     8,    17,     3,    42,    -1,     8,    18,    48,    42,
      -1,     8,    18,     3,    42,    -1,     8,    19,    48,    42,
      -1,     8,    19,     3,    42,    -1,     8,    20,    48,    42,
      -1,     8,    20,     3,    42,    -1,     8,    21,    48,    42,
      -1,     8,    21,     3,    42,    -1,     8,    22,    48,    42,
      -1,     8,    22,     3,    42,    -1,     8,    26,    48,    42,
      -1,     8,    26,     3,    42,    -1,     8,    27,    48,    42,
      -1,     8,    27,     3,    42,    -1,     8,    24,    48,    42,
      -1,     8,    24,     3,    42,    -1,     8,    25,    48,    42,
      -1,     8,    25,     3,    42,    -1,     8,    40,    48,    42,
      -1,     8,    40,     3,    42,    -1,     8,    41,    48,    42,
      -1,     8,    41,     3,    42,    -1,     8,    14,    48,    42,
      -1,     8,    14,     3,    42,    -1,     8,    32,    48,    42,
      -1,     8,    32,     3,    42,    -1,    39,    17,    48,    42,
      -1,    39,    17,     3,    42,    -1,    39,    18,    48,    42,
      -1,    39,    18,     3,    42,    -1,    39,    23,    48,    42,
      -1,    39,    23,     3,    42,    -1,    39,    24,    48,    42,
      -1,    39,    24,     3,    42,    -1,    39,    25,    48,    42,
      -1,    39,    25,     3,    42,    -1,    39,    14,    48,    42,
      -1,    39,    14,     3,    42,    -1,     9,    28,    48,    42,
      -1,     9,    28,     3,    42,    -1,     9,    13,    48,    42,
      -1,     9,    13,     3,    42,    -1,     9,    14,    48,    42,
      -1,     9,    14,     3,    42,    -1,    30,    12,    48,    42,
      -1,    30,    12,     3,    42,    -1,    30,    13,    48,    42,
      -1,    30,    13,     3,    42,    -1,    30,    31,    48,    42,
      -1,    30,    31,     3,    42,    -1,    30,    14,    48,    42,
      -1,    30,    14,     3,    42,    -1,    30,    23,    48,    42,
      -1,    30,    23,     3,    42,    -1,    32,    33,    42,    -1,
      32,    34,    42,    -1,     3,    49,    42,    -1,     3,    49,
      49,    42,    -1,     3,    49,    16,    48,    42,    -1,     3,
      49,    49,    49,    42,    -1,     3,    49,    49,    16,    48,
      42,    -1,     3,    16,    48,    42,    -1,     3,     1,    42,
      -1,    49,     3,    42,    -1,    49,    16,     1,    42,    -1,
      49,     1,    42,    -1,    16,     1,    42,    -1,     1,    42,
      -1,    42,    -1,    16,    48,    -1,    49,    48,    -1,    16,
       3,    48,    -1,    49,     3,    48,    -1,    -1,     4,    -1,
       5,    -1,     6,    -1,     7,    -1,     8,    -1,     9,    -1,
      10,    -1,    11,    -1,    12,    -1,    13,    -1,    14,    -1,
      15,    -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,
      23,    -1,    26,    -1,    27,    -1,    24,    -1,    25,    -1,
      28,    -1,    29,    -1,    30,    -1,    31,    -1,    32,    -1,
      33,    -1,    34,    -1,    35,    -1,    36,    -1,    37,    -1,
      38,    -1,    39,    -1,    40,    -1,    41,    -1,    22,    -1,
      21,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    87,    87,    91,    98,   101,   108,   116,   120,   124,
     128,   132,   136,   140,   144,   148,   152,   156,   160,   164,
     168,   172,   176,   180,   184,   188,   192,   196,   200,   204,
     208,   212,   216,   220,   224,   228,   232,   236,   240,   244,
     248,   252,   256,   260,   264,   268,   272,   276,   280,   284,
     288,   292,   296,   300,   304,   308,   312,   316,   320,   324,
     328,   332,   336,   340,   344,   348,   352,   356,   360,   364,
     368,   372,   376,   380,   384,   388,   392,   396,   400,   404,
     408,   412,   416,   420,   424,   428,   432,   436,   440,   444,
     448,   452,   456,   460,   464,   468,   472,   476,   480,   484,
     488,   492,   496,   500,   504,   508,   512,   516,   520,   524,
     528,   532,   536,   540,   544,   548,   552,   563,   567,   578,
     589,   593,   597,   601,   607,   612,   617,   621,   630,   635,
     640,   646,   653,   661,   664,   667,   670,   673,   676,   679,
     682,   685,   688,   691,   694,   697,   700,   703,   706,   709,
     712,   715,   718,   721,   724,   727,   730,   733,   736,   739,
     742,   745,   748,   751,   754,   757,   760,   763,   766,   769
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ICE_GRID_HELP", "ICE_GRID_EXIT",
  "ICE_GRID_APPLICATION", "ICE_GRID_NODE", "ICE_GRID_REGISTRY",
  "ICE_GRID_SERVER", "ICE_GRID_ADAPTER", "ICE_GRID_PING", "ICE_GRID_LOAD",
  "ICE_GRID_ADD", "ICE_GRID_REMOVE", "ICE_GRID_LIST", "ICE_GRID_SHUTDOWN",
  "ICE_GRID_STRING", "ICE_GRID_START", "ICE_GRID_STOP", "ICE_GRID_PATCH",
  "ICE_GRID_SIGNAL", "ICE_GRID_STDOUT", "ICE_GRID_STDERR",
  "ICE_GRID_DESCRIBE", "ICE_GRID_PROPERTIES", "ICE_GRID_PROPERTY",
  "ICE_GRID_STATE", "ICE_GRID_PID", "ICE_GRID_ENDPOINTS",
  "ICE_GRID_ACTIVATION", "ICE_GRID_OBJECT", "ICE_GRID_FIND",
  "ICE_GRID_SHOW", "ICE_GRID_COPYING", "ICE_GRID_WARRANTY",
  "ICE_GRID_DIFF", "ICE_GRID_UPDATE", "ICE_GRID_INSTANTIATE",
  "ICE_GRID_TEMPLATE", "ICE_GRID_SERVICE", "ICE_GRID_ENABLE",
  "ICE_GRID_DISABLE", "';'", "$accept", "start", "commands",
  "checkInterrupted", "command", "strings", "keyword", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    43,    44,    44,    45,    45,    46,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    48,    48,
      48,    48,    48,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     0,     3,     2,     0,     2,     2,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     5,     5,     5,     5,     4,     5,     5,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     3,     3,     3,     4,     5,     5,     6,
       4,     3,     3,     4,     3,     3,     2,     1,     2,     2,
       3,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       6,     0,     6,     0,     1,     0,     0,     0,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
       0,   145,   146,   147,   148,   169,   168,   149,   152,   153,
     150,   151,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   127,     5,     0,     4,
     126,     0,   133,   134,   135,   136,   137,   138,   132,   156,
     158,   165,     7,     0,     8,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,   132,   132,   132,
     132,   132,   132,   132,   132,   132,   132,     0,   132,   132,
     132,   132,   132,     0,   132,   132,   132,   132,   132,     0,
       0,   132,   132,   132,   132,   132,   132,     0,     0,     0,
       0,   121,   132,     0,   132,   132,   115,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   132,   132,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   125,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   113,   114,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   132,   124,   122,     0,   132,   128,   120,   132,   129,
       0,   132,   116,     0,    10,     9,    12,    11,    22,    21,
      20,    19,    18,    17,    14,    13,    16,    15,    34,    33,
      36,    35,    40,    39,    38,    37,    32,    31,    42,    41,
      46,    45,    50,    49,    48,    47,    44,    43,    52,    51,
      54,    53,    82,    81,    58,    57,    60,    59,    62,    61,
      64,    63,    66,    65,    68,    67,    56,    55,    74,    73,
      76,    75,    70,    69,    72,    71,    84,    83,    27,     0,
       0,     0,     0,    78,    77,    80,    79,   100,    99,   102,
     101,    98,    97,   104,   103,   106,   105,   110,   109,   112,
     111,   108,   107,    96,    95,    86,    85,    88,    87,    90,
      89,    92,    91,    94,    93,    30,     0,     0,   123,   130,
     131,   117,     0,   118,    24,    23,    26,    25,    29,    28,
     119
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     2,     3,    47,   123,   124
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -66
static const short yypact[] =
{
      38,    53,    55,    60,   -66,    60,    20,   176,    62,    96,
     140,   142,   121,    98,   -66,   -66,   -66,   -66,   -66,   -66,
     112,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,    93,   -66,    11,   -66,   -66,   -66,
     -66,   -66,   -66,  2388,   -66,   -66,   -66,   -66,   117,   -66,
     -66,    75,   -66,   -66,   -66,   -66,   -66,   -66,  2360,   -66,
     -66,   -66,   -66,   215,   -66,   255,   294,   333,   372,   411,
     450,   489,   528,   567,   606,   645,   684,   723,   762,   801,
     840,   879,   918,   957,   996,  1035,  1074,  1113,  1152,  1191,
    1230,  1269,  1308,  1347,  1386,  1425,  1464,   100,  1503,  1542,
    1581,  1620,  1659,    79,  1698,  1737,  1776,  1815,  1854,    80,
      83,  1893,  1932,  1971,  2010,  2049,  2088,    29,    85,    88,
     135,   -66,  2127,   107,  2166,  2360,   -66,  2204,   116,   118,
     122,   125,   126,   127,   129,   133,   137,  2361,  2362,  2365,
    2366,  2367,  2368,  2372,  2373,  2374,  2375,  2376,  2377,  2378,
    2379,  2380,  2381,  2382,  2383,  2385,  2386,  2387,  2389,  2390,
    2391,  2392,  2393,  2394,  2395,  2396,  2397,  2398,  2399,  2400,
    2401,  2402,  2403,  2404,  2405,  2406,  2407,  2408,  2409,  2410,
    2411,  2412,  2413,  2414,  2415,  2416,  2417,  2418,  2419,  2420,
    2421,  2422,  2423,  2244,  2283,  2424,  2425,  2426,  2427,  2428,
    2429,  2430,  2431,  2432,  2433,   -66,  2434,  2435,  2436,  2437,
    2438,  2439,  2440,  2441,  2442,  2443,   -66,   -66,  2444,  2445,
    2446,  2447,  2448,  2449,  2450,  2451,  2452,  2453,  2454,  2455,
    2456,  2322,   -66,   -66,  2457,  2360,   -66,   -66,  2360,   -66,
    2458,  2360,   -66,  2459,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,  2460,
    2461,  2462,  2463,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66,   -66,   -66,   -66,   -66,   -66,  2464,  2465,   -66,   -66,
     -66,   -66,  2466,   -66,   -66,   -66,   -66,   -66,   -66,   -66,
     -66
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
     -66,   -66,   -66,  2507,  2505,   -65,    51
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -4
static const short yytable[] =
{
     129,   131,   133,   135,   137,   139,   141,   143,   145,   147,
     149,   151,   153,   155,   157,   159,   161,   163,   165,   167,
     169,   171,   173,   175,   177,   179,   181,   183,   185,   187,
     189,   191,   230,   196,   198,   200,   202,   204,    -3,   207,
     209,   211,   213,   215,   109,   110,   219,   221,   223,   225,
     227,   229,   231,     4,    48,    -2,    48,   236,    63,   239,
     240,     6,    50,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,   192,    64,   104,   105,   106,    65,    66,
      67,   100,   101,   103,   127,    68,   107,   121,   118,    69,
     119,   205,   216,   193,   108,   217,   102,   232,   310,   312,
     233,    70,    71,   120,    83,    84,   234,   194,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,   237,
      72,    73,    78,    96,    74,    75,    79,    80,   244,    97,
     245,    98,    99,    76,   246,    81,   347,   247,   248,   249,
     349,   250,    77,   350,    82,   251,   352,    51,   243,   252,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,    58,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,    62,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,   125,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   126,   128,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,   122,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   130,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
     122,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   132,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,   122,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   134,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,   122,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   136,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,   122,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   138,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,   122,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   140,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,   122,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   142,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,   122,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     144,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,   122,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   146,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,   122,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   148,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,   122,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   150,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
     122,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   152,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,   122,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   154,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,   122,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   156,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,   122,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   158,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,   122,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   160,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,   122,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   162,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,   122,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     164,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,   122,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   166,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,   122,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   168,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,   122,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   170,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
     122,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   172,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,   122,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   174,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,   122,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   176,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,   122,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   178,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,   122,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   180,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,   122,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   182,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,   122,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     184,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,   122,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   186,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,   122,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   188,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,   122,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   190,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
     122,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   195,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,   122,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   197,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,   122,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   199,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,   122,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   201,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,   122,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   203,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,   122,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   206,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,   122,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     208,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,   122,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   210,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,   122,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,   212,    52,
      53,    54,    55,    56,    57,    14,    15,    16,    17,    18,
      19,   122,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    59,    35,    60,    37,    38,
      39,    40,    41,    42,    61,    44,    45,   214,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
     122,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   218,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,   122,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   220,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,   122,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,   222,    52,    53,    54,    55,    56,
      57,    14,    15,    16,    17,    18,    19,   122,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    59,    35,    60,    37,    38,    39,    40,    41,    42,
      61,    44,    45,   224,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,   122,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   226,    52,    53,    54,    55,    56,    57,    14,
      15,    16,    17,    18,    19,   122,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    59,
      35,    60,    37,    38,    39,    40,    41,    42,    61,    44,
      45,   228,    52,    53,    54,    55,    56,    57,    14,    15,
      16,    17,    18,    19,   122,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    59,    35,
      60,    37,    38,    39,    40,    41,    42,    61,    44,    45,
     235,    52,    53,    54,    55,    56,    57,    14,    15,    16,
      17,    18,    19,   122,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    59,    35,    60,
      37,    38,    39,    40,    41,    42,    61,    44,    45,   238,
      52,    53,    54,    55,    56,    57,    14,    15,    16,    17,
      18,    19,   122,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    59,    35,    60,    37,
      38,    39,    40,    41,    42,    61,    44,    45,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
     241,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   242,   309,    52,    53,
      54,    55,    56,    57,    14,    15,    16,    17,    18,    19,
     122,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    59,    35,    60,    37,    38,    39,
      40,    41,    42,    61,    44,    45,   311,    52,    53,    54,
      55,    56,    57,    14,    15,    16,    17,    18,    19,   122,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    59,    35,    60,    37,    38,    39,    40,
      41,    42,    61,    44,    45,   346,    52,    53,    54,    55,
      56,    57,    14,    15,    16,    17,    18,    19,   122,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    59,    35,    60,    37,    38,    39,    40,    41,
      42,    61,    44,    45,    52,    53,    54,    55,    56,    57,
      14,    15,    16,    17,    18,    19,   122,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      59,    35,    60,    37,    38,    39,    40,    41,    42,    61,
      44,    45,   111,   253,   254,   112,   113,   255,   256,   257,
     258,   114,   115,   116,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   117,   271,   272,   273,
       0,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   348,
     351,   353,   354,   355,   356,   357,   358,   359,   360,     5,
      49
};

static const short yycheck[] =
{
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,     3,    98,    99,   100,   101,   102,     0,   104,
     105,   106,   107,   108,    33,    34,   111,   112,   113,   114,
     115,   116,    23,     0,     3,     0,     5,   122,     7,   124,
     125,     1,    42,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,     3,    42,    12,    13,    14,    12,    13,
      14,    13,    14,     1,    63,    19,    23,    42,     1,    23,
       3,    42,    42,    23,    31,    42,    28,    42,   193,   194,
      42,    35,    36,    16,    13,    14,     1,    37,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    42,
      10,    11,    10,    32,    14,    15,    14,    15,    42,    38,
      42,    40,    41,    23,    42,    23,   231,    42,    42,    42,
     235,    42,    32,   238,    32,    42,   241,     1,   127,    42,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    14,    42,    42,    17,    18,    42,    42,    42,
      42,    23,    24,    25,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    38,    42,    42,    42,
      -1,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,     2,
       5
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    44,    45,    46,     0,    46,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    47,    49,    47,
      42,     1,     4,     5,     6,     7,     8,     9,    16,    30,
      32,    39,    42,    49,    42,    12,    13,    14,    19,    23,
      35,    36,    10,    11,    14,    15,    23,    32,    10,    14,
      15,    23,    32,    13,    14,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    32,    38,    40,    41,
      13,    14,    28,     1,    12,    13,    14,    23,    31,    33,
      34,    14,    17,    18,    23,    24,    25,    38,     1,     3,
      16,    42,    16,    48,    49,    16,    42,    49,     3,    48,
       3,    48,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    48,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    48,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    48,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    48,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    48,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    48,     3,    23,    37,     3,    48,     3,    48,     3,
      48,     3,    48,     3,    48,    42,     3,    48,     3,    48,
       3,    48,     3,    48,     3,    48,    42,    42,     3,    48,
       3,    48,     3,    48,     3,    48,     3,    48,     3,    48,
       3,    23,    42,    42,     1,     3,    48,    42,     3,    48,
      48,    16,    42,    49,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,     3,
      48,     3,    48,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,    42,    42,    42,    42,
      42,    42,    42,    42,    42,    42,     3,    48,    42,    48,
      48,    42,    48,    42,    42,    42,    42,    42,    42,    42,
      42
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 88 "Grammar.y"
    {
;}
    break;

  case 3:
#line 91 "Grammar.y"
    {
;}
    break;

  case 4:
#line 99 "Grammar.y"
    {
;}
    break;

  case 5:
#line 102 "Grammar.y"
    {
;}
    break;

  case 6:
#line 108 "Grammar.y"
    {
    parser->checkInterrupted();
;}
    break;

  case 7:
#line 117 "Grammar.y"
    {
    parser->usage();
;}
    break;

  case 8:
#line 121 "Grammar.y"
    {
    return 0;
;}
    break;

  case 9:
#line 125 "Grammar.y"
    {
    parser->addApplication(yyvsp[-1]);
;}
    break;

  case 10:
#line 129 "Grammar.y"
    {
    parser->usage("application", "add");
;}
    break;

  case 11:
#line 133 "Grammar.y"
    {
    parser->removeApplication(yyvsp[-1]);
;}
    break;

  case 12:
#line 137 "Grammar.y"
    {
    parser->usage("application", "remove");
;}
    break;

  case 13:
#line 141 "Grammar.y"
    {
    parser->diffApplication(yyvsp[-1]);
;}
    break;

  case 14:
#line 145 "Grammar.y"
    {
    parser->usage("application", "diff");
;}
    break;

  case 15:
#line 149 "Grammar.y"
    {
    parser->updateApplication(yyvsp[-1]);
;}
    break;

  case 16:
#line 153 "Grammar.y"
    {
    parser->usage("application", "update");
;}
    break;

  case 17:
#line 157 "Grammar.y"
    {
    parser->describeApplication(yyvsp[-1]);
;}
    break;

  case 18:
#line 161 "Grammar.y"
    {
    parser->usage("application", "describe");
;}
    break;

  case 19:
#line 165 "Grammar.y"
    {
    parser->patchApplication(yyvsp[-1]);
;}
    break;

  case 20:
#line 169 "Grammar.y"
    {
    parser->usage("application", "patch");
;}
    break;

  case 21:
#line 173 "Grammar.y"
    {
    parser->listAllApplications(yyvsp[-1]);
;}
    break;

  case 22:
#line 177 "Grammar.y"
    {
    parser->usage("application", "list");
;}
    break;

  case 23:
#line 181 "Grammar.y"
    {
    parser->describeServerTemplate(yyvsp[-1]);
;}
    break;

  case 24:
#line 185 "Grammar.y"
    {
    parser->usage("server template", "describe");
;}
    break;

  case 25:
#line 189 "Grammar.y"
    {
    parser->instantiateServerTemplate(yyvsp[-1]);
;}
    break;

  case 26:
#line 193 "Grammar.y"
    {
    parser->usage("server template", "instantiate");
;}
    break;

  case 27:
#line 197 "Grammar.y"
    {
    parser->usage("server template");
;}
    break;

  case 28:
#line 201 "Grammar.y"
    {
    parser->describeServiceTemplate(yyvsp[-1]);
;}
    break;

  case 29:
#line 205 "Grammar.y"
    {
    parser->usage("service template", "describe");
;}
    break;

  case 30:
#line 209 "Grammar.y"
    {
    parser->usage("service template");
;}
    break;

  case 31:
#line 213 "Grammar.y"
    {
    parser->describeNode(yyvsp[-1]);
;}
    break;

  case 32:
#line 217 "Grammar.y"
    {
    parser->usage("node", "describe");
;}
    break;

  case 33:
#line 221 "Grammar.y"
    {
    parser->pingNode(yyvsp[-1]);
;}
    break;

  case 34:
#line 225 "Grammar.y"
    {
    parser->usage("node", "ping");
;}
    break;

  case 35:
#line 229 "Grammar.y"
    {
    parser->printLoadNode(yyvsp[-1]);
;}
    break;

  case 36:
#line 233 "Grammar.y"
    {
    parser->usage("node", "lost");
;}
    break;

  case 37:
#line 237 "Grammar.y"
    {
    parser->shutdownNode(yyvsp[-1]);
;}
    break;

  case 38:
#line 241 "Grammar.y"
    {
    parser->usage("node", "shutdown");
;}
    break;

  case 39:
#line 245 "Grammar.y"
    {
    parser->listAllNodes(yyvsp[-1]);
;}
    break;

  case 40:
#line 249 "Grammar.y"
    {
    parser->usage("node", "list");
;}
    break;

  case 41:
#line 253 "Grammar.y"
    {
    parser->showFile("node", yyvsp[-1]);
;}
    break;

  case 42:
#line 257 "Grammar.y"
    {
    parser->usage("node", "show");
;}
    break;

  case 43:
#line 261 "Grammar.y"
    {
    parser->describeRegistry(yyvsp[-1]);
;}
    break;

  case 44:
#line 265 "Grammar.y"
    {
    parser->usage("registry", "describe");
;}
    break;

  case 45:
#line 269 "Grammar.y"
    {
    parser->pingRegistry(yyvsp[-1]);
;}
    break;

  case 46:
#line 273 "Grammar.y"
    {
    parser->usage("registry", "ping");
;}
    break;

  case 47:
#line 277 "Grammar.y"
    {
    parser->shutdownRegistry(yyvsp[-1]);
;}
    break;

  case 48:
#line 281 "Grammar.y"
    {
    parser->usage("registry", "shutdown");
;}
    break;

  case 49:
#line 285 "Grammar.y"
    {
    parser->listAllRegistries(yyvsp[-1]);
;}
    break;

  case 50:
#line 289 "Grammar.y"
    {
    parser->usage("registry", "list");
;}
    break;

  case 51:
#line 293 "Grammar.y"
    {
    parser->showFile("registry", yyvsp[-1]);
;}
    break;

  case 52:
#line 297 "Grammar.y"
    {
    parser->usage("registry", "show");
;}
    break;

  case 53:
#line 301 "Grammar.y"
    {
    parser->removeServer(yyvsp[-1]);
;}
    break;

  case 54:
#line 305 "Grammar.y"
    {
    parser->usage("server", "remove");
;}
    break;

  case 55:
#line 309 "Grammar.y"
    {
    parser->describeServer(yyvsp[-1]);
;}
    break;

  case 56:
#line 313 "Grammar.y"
    {
    parser->usage("server", "describe");
;}
    break;

  case 57:
#line 317 "Grammar.y"
    {
    parser->startServer(yyvsp[-1]);
;}
    break;

  case 58:
#line 321 "Grammar.y"
    {
    parser->usage("server", "start");
;}
    break;

  case 59:
#line 325 "Grammar.y"
    {
    parser->stopServer(yyvsp[-1]);
;}
    break;

  case 60:
#line 329 "Grammar.y"
    {
    parser->usage("server", "stop");
;}
    break;

  case 61:
#line 333 "Grammar.y"
    {
    parser->patchServer(yyvsp[-1]);
;}
    break;

  case 62:
#line 337 "Grammar.y"
    {
    parser->usage("server", "patch");
;}
    break;

  case 63:
#line 341 "Grammar.y"
    {
    parser->signalServer(yyvsp[-1]);
;}
    break;

  case 64:
#line 345 "Grammar.y"
    {
    parser->usage("server", "signal");
;}
    break;

  case 65:
#line 349 "Grammar.y"
    {
    parser->writeMessage(yyvsp[-1], 1);
;}
    break;

  case 66:
#line 353 "Grammar.y"
    {
    parser->usage("server", "stdout");
;}
    break;

  case 67:
#line 357 "Grammar.y"
    {
    parser->writeMessage(yyvsp[-1], 2);
;}
    break;

  case 68:
#line 361 "Grammar.y"
    {
    parser->usage("server", "stderr");
;}
    break;

  case 69:
#line 365 "Grammar.y"
    {
    parser->stateServer(yyvsp[-1]);
;}
    break;

  case 70:
#line 369 "Grammar.y"
    {
    parser->usage("server", "start");
;}
    break;

  case 71:
#line 373 "Grammar.y"
    {
    parser->pidServer(yyvsp[-1]);
;}
    break;

  case 72:
#line 377 "Grammar.y"
    {
    parser->usage("server", "pid");
;}
    break;

  case 73:
#line 381 "Grammar.y"
    {
    parser->propertiesServer(yyvsp[-1], false);
;}
    break;

  case 74:
#line 385 "Grammar.y"
    {
    parser->usage("server", "properties");
;}
    break;

  case 75:
#line 389 "Grammar.y"
    {
    parser->propertiesServer(yyvsp[-1], true);
;}
    break;

  case 76:
#line 393 "Grammar.y"
    {
    parser->usage("server", "property");
;}
    break;

  case 77:
#line 397 "Grammar.y"
    {
    parser->enableServer(yyvsp[-1], true);
;}
    break;

  case 78:
#line 401 "Grammar.y"
    {
    parser->usage("server", "enable");
;}
    break;

  case 79:
#line 405 "Grammar.y"
    {
    parser->enableServer(yyvsp[-1], false);
;}
    break;

  case 80:
#line 409 "Grammar.y"
    {
    parser->usage("server", "disable");
;}
    break;

  case 81:
#line 413 "Grammar.y"
    {
    parser->listAllServers(yyvsp[-1]);
;}
    break;

  case 82:
#line 417 "Grammar.y"
    {
    parser->usage("server", "list");
;}
    break;

  case 83:
#line 421 "Grammar.y"
    {
    parser->showFile("server", yyvsp[-1]);
;}
    break;

  case 84:
#line 425 "Grammar.y"
    {
    parser->usage("server", "show");
;}
    break;

  case 85:
#line 429 "Grammar.y"
    {
    parser->startService(yyvsp[-1]);
;}
    break;

  case 86:
#line 433 "Grammar.y"
    {
    parser->usage("service", "start");
;}
    break;

  case 87:
#line 437 "Grammar.y"
    {
    parser->stopService(yyvsp[-1]);
;}
    break;

  case 88:
#line 441 "Grammar.y"
    {
    parser->usage("service", "stop");
;}
    break;

  case 89:
#line 445 "Grammar.y"
    {
    parser->describeService(yyvsp[-1]);
;}
    break;

  case 90:
#line 449 "Grammar.y"
    {
    parser->usage("service", "describe");
;}
    break;

  case 91:
#line 453 "Grammar.y"
    {
    parser->propertiesService(yyvsp[-1], false);
;}
    break;

  case 92:
#line 457 "Grammar.y"
    {
    parser->usage("service", "properties");
;}
    break;

  case 93:
#line 461 "Grammar.y"
    {
    parser->propertiesService(yyvsp[-1], true);
;}
    break;

  case 94:
#line 465 "Grammar.y"
    {
    parser->usage("service", "property");
;}
    break;

  case 95:
#line 469 "Grammar.y"
    {
    parser->listServices(yyvsp[-1]);
;}
    break;

  case 96:
#line 473 "Grammar.y"
    {
    parser->usage("service", "list");
;}
    break;

  case 97:
#line 477 "Grammar.y"
    {
    parser->endpointsAdapter(yyvsp[-1]);
;}
    break;

  case 98:
#line 481 "Grammar.y"
    {
    parser->usage("adapter", "endpoints");
;}
    break;

  case 99:
#line 485 "Grammar.y"
    {
    parser->removeAdapter(yyvsp[-1]);
;}
    break;

  case 100:
#line 489 "Grammar.y"
    {
    parser->usage("adapter", "remove");
;}
    break;

  case 101:
#line 493 "Grammar.y"
    {
    parser->listAllAdapters(yyvsp[-1]);
;}
    break;

  case 102:
#line 497 "Grammar.y"
    {
    parser->usage("adapter", "list");
;}
    break;

  case 103:
#line 501 "Grammar.y"
    {
    parser->addObject(yyvsp[-1]);
;}
    break;

  case 104:
#line 505 "Grammar.y"
    {
    parser->usage("object", "add");
;}
    break;

  case 105:
#line 509 "Grammar.y"
    {
    parser->removeObject(yyvsp[-1]);
;}
    break;

  case 106:
#line 513 "Grammar.y"
    {
    parser->usage("object", "remove");
;}
    break;

  case 107:
#line 517 "Grammar.y"
    {
    parser->findObject(yyvsp[-1]);
;}
    break;

  case 108:
#line 521 "Grammar.y"
    {
    parser->usage("object", "find");
;}
    break;

  case 109:
#line 525 "Grammar.y"
    {
    parser->listObject(yyvsp[-1]);
;}
    break;

  case 110:
#line 529 "Grammar.y"
    {
    parser->usage("object", "list");
;}
    break;

  case 111:
#line 533 "Grammar.y"
    {
    parser->describeObject(yyvsp[-1]);
;}
    break;

  case 112:
#line 537 "Grammar.y"
    {
    parser->usage("object", "describe");
;}
    break;

  case 113:
#line 541 "Grammar.y"
    {
    parser->showCopying();
;}
    break;

  case 114:
#line 545 "Grammar.y"
    {
    parser->showWarranty();
;}
    break;

  case 115:
#line 549 "Grammar.y"
    {
    parser->usage(yyvsp[-1].front());
;}
    break;

  case 116:
#line 553 "Grammar.y"
    {
    if((yyvsp[-2].front() == "server" || yyvsp[-2].front() == "service") && yyvsp[-1].front() == "template")
    {
        parser->usage(yyvsp[-2].front() + " " + yyvsp[-1].front());
    }
    else
    {
        parser->usage(yyvsp[-2].front(), yyvsp[-1].front());
    }
;}
    break;

  case 117:
#line 564 "Grammar.y"
    {
    parser->usage(yyvsp[-3].front(), yyvsp[-2].front());
;}
    break;

  case 118:
#line 568 "Grammar.y"
    {
    if((yyvsp[-3].front() == "server" || yyvsp[-3].front() == "service") && yyvsp[-2].front() == "template")
    {
        parser->usage(yyvsp[-3].front() + " " + yyvsp[-2].front(), yyvsp[-1].front());
    }
    else
    {
        parser->usage(yyvsp[-3].front(), yyvsp[-2].front());
    }
;}
    break;

  case 119:
#line 579 "Grammar.y"
    {
    if((yyvsp[-4].front() == "server" || yyvsp[-4].front() == "service") && yyvsp[-3].front() == "template")
    {
        parser->usage(yyvsp[-4].front() + " " + yyvsp[-3].front(), yyvsp[-2].front());
    }
    else
    {
        parser->usage(yyvsp[-4].front(), yyvsp[-3].front());
    }
;}
    break;

  case 120:
#line 590 "Grammar.y"
    {
    parser->usage(yyvsp[-2].front());
;}
    break;

  case 121:
#line 594 "Grammar.y"
    {
    parser->usage();
;}
    break;

  case 122:
#line 598 "Grammar.y"
    {
    parser->usage(yyvsp[-2].front());
;}
    break;

  case 123:
#line 602 "Grammar.y"
    {
    yyvsp[-3].push_back(yyvsp[-2].front());
    parser->invalidCommand(yyvsp[-3]);
    yyerrok;
;}
    break;

  case 124:
#line 608 "Grammar.y"
    {
    parser->invalidCommand(yyvsp[-2]);
    yyerrok;
;}
    break;

  case 125:
#line 613 "Grammar.y"
    {
    parser->invalidCommand(yyvsp[-2]);
    yyerrok;
;}
    break;

  case 126:
#line 618 "Grammar.y"
    {
    yyerrok;
;}
    break;

  case 127:
#line 622 "Grammar.y"
    {
;}
    break;

  case 128:
#line 631 "Grammar.y"
    {
    yyval = yyvsp[0];
    yyval.push_front(yyvsp[-1].front());
;}
    break;

  case 129:
#line 636 "Grammar.y"
    {
    yyval = yyvsp[0];
    yyval.push_front(yyvsp[-1].front());
;}
    break;

  case 130:
#line 641 "Grammar.y"
    {
    yyval = yyvsp[-1];
    yyval.push_front("help");
    yyval.push_front(yyvsp[-2].front());
;}
    break;

  case 131:
#line 647 "Grammar.y"
    {
    yyval = yyvsp[-1];
    yyval.push_front("help");
    yyval.push_front(yyvsp[-2].front());
;}
    break;

  case 132:
#line 653 "Grammar.y"
    {
    yyval = YYSTYPE();
;}
    break;

  case 133:
#line 662 "Grammar.y"
    {
;}
    break;

  case 134:
#line 665 "Grammar.y"
    {
;}
    break;

  case 135:
#line 668 "Grammar.y"
    {
;}
    break;

  case 136:
#line 671 "Grammar.y"
    {
;}
    break;

  case 137:
#line 674 "Grammar.y"
    {
;}
    break;

  case 138:
#line 677 "Grammar.y"
    {
;}
    break;

  case 139:
#line 680 "Grammar.y"
    {
;}
    break;

  case 140:
#line 683 "Grammar.y"
    {
;}
    break;

  case 141:
#line 686 "Grammar.y"
    {
;}
    break;

  case 142:
#line 689 "Grammar.y"
    {
;}
    break;

  case 143:
#line 692 "Grammar.y"
    {
;}
    break;

  case 144:
#line 695 "Grammar.y"
    {
;}
    break;

  case 145:
#line 698 "Grammar.y"
    {
;}
    break;

  case 146:
#line 701 "Grammar.y"
    {
;}
    break;

  case 147:
#line 704 "Grammar.y"
    {
;}
    break;

  case 148:
#line 707 "Grammar.y"
    {
;}
    break;

  case 149:
#line 710 "Grammar.y"
    {
;}
    break;

  case 150:
#line 713 "Grammar.y"
    {
;}
    break;

  case 151:
#line 716 "Grammar.y"
    {
;}
    break;

  case 152:
#line 719 "Grammar.y"
    {
;}
    break;

  case 153:
#line 722 "Grammar.y"
    {
;}
    break;

  case 154:
#line 725 "Grammar.y"
    {
;}
    break;

  case 155:
#line 728 "Grammar.y"
    {
;}
    break;

  case 156:
#line 731 "Grammar.y"
    {
;}
    break;

  case 157:
#line 734 "Grammar.y"
    {
;}
    break;

  case 158:
#line 737 "Grammar.y"
    {
;}
    break;

  case 159:
#line 740 "Grammar.y"
    {
;}
    break;

  case 160:
#line 743 "Grammar.y"
    {
;}
    break;

  case 161:
#line 746 "Grammar.y"
    {
;}
    break;

  case 162:
#line 749 "Grammar.y"
    {
;}
    break;

  case 163:
#line 752 "Grammar.y"
    {
;}
    break;

  case 164:
#line 755 "Grammar.y"
    {
;}
    break;

  case 165:
#line 758 "Grammar.y"
    {
;}
    break;

  case 166:
#line 761 "Grammar.y"
    {
;}
    break;

  case 167:
#line 764 "Grammar.y"
    {
;}
    break;

  case 168:
#line 767 "Grammar.y"
    {
;}
    break;

  case 169:
#line 770 "Grammar.y"
    {
;}
    break;


    }

/* Line 1000 of yacc.c.  */
#line 3000 "Grammar.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
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

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}



