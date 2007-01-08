%{

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Parser.h>
#include <Grammar.h>
#include <NodeI.h>

#if defined(_MSC_VER) && defined(ICE_64)
//
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
// Puts a pointer-difference into an int
//
#   pragma warning( 4 : 4244 )
#endif

using namespace std;
using namespace Ice;

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

%}

WS	[ \t\v\f\r]
NL	[\n]
DIGIT   [0-9]

%option noyywrap
%option never-interactive

%%

{DIGIT}+ {
    *yylvalp = new Complex::NumberNodeI(atoi(yytext));
    return TOK_NUMBER;
}

WS+    /* eat whitespace */

"("|")"|"+"|"*" { return *yytext; }

%%
