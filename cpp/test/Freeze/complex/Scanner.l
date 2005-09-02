%{

// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Parser.h>
#include <Grammar.h>
#include <NodeI.h>

#ifdef _MSC_VER
// I get these warnings from some flex versions:
// warning C4003: not enough actual parameters for macro 'yywrap'
#   pragma warning( disable : 4003 )
#endif

using namespace std;
using namespace Ice;

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

%}

WS	[ \t\v\f\r]
NL	[\n]
DIGIT   [0-9]

%option noyywrap

%%

{DIGIT}+ {
    *yylvalp = new Complex::NumberNodeI(atoi(yytext));
    return TOK_NUMBER;
}

WS+    /* eat whitespace */

"("|")"|"+"|"*" { return *yytext; }

%%
