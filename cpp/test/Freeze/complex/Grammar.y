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

#include <NodeI.h>
#include <Parser.h>

using namespace std;
using namespace Ice;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%pure_parser

%token TOK_NUMBER

%%

start
: expr
{
    parser->setResult($$);
}

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
