%{

// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Parser.h>
#include <Grammar.h>
#include <NodeI.h>

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
