%{

// **********************************************************************
//
// Copyright (c) 2002
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

#include <NodeI.h>
#include <Parser.h>

#ifdef _WIN32
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
#endif

using namespace std;
using namespace Ice;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%pure_parser
%expect 4

%token TOK_NUMBER

%%

start
: expr
{
    parser->setResult($$);
}
;

expr
: TOK_NUMBER
{
    $$ = $1;
}
| expr '+' expr
{
    $$ = new Complex::AddNodeI($1, $3);
}
| '(' expr ')'
{
    $$ = $2;
}
| expr '*' expr
{
    $$ = new Complex::MultiplyNodeI($1, $3);
}
;

%%
