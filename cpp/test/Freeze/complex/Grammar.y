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

#ifdef WIN32
// I get this warning from some bison version:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
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
