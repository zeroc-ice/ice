%{

// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
