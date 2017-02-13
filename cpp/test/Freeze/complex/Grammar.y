%{

// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <NodeI.h>
#include <Parser.h>

#ifdef _MSC_VER
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
