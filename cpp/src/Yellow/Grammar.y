%{

// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Yellow/Parser.h>

#ifdef _WIN32
// I get this warning from some bison version:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
#endif

using namespace std;
using namespace Ice;
using namespace Yellow;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%pure_parser

%token YELLOW_HELP
%token YELLOW_EXIT
%token YELLOW_ADD
%token YELLOW_REMOVE
%token YELLOW_LOOKUP
%token YELLOW_LOOKUP_ALL
%token YELLOW_STRING

%%

// ----------------------------------------------------------------------
start
// ----------------------------------------------------------------------
: commands
{
}
|
{
}
;

// ----------------------------------------------------------------------
commands
// ----------------------------------------------------------------------
: commands command
{
}
| command
{
}
;

// ----------------------------------------------------------------------
command
// ----------------------------------------------------------------------
: YELLOW_HELP ';'
{
    parser->usage();
}
| YELLOW_EXIT ';'
{
    return 0;
}
| YELLOW_ADD strings ';'
{
    parser->addOffer($2);
}
| YELLOW_REMOVE strings ';'
{
    parser->removeOffer($2);
}
| YELLOW_LOOKUP strings ';'
{
    parser->lookup($2);
}
| YELLOW_LOOKUP_ALL strings ';'
{
    parser->lookupAll($2);
}
| error ';'
{
    yyerrok;
}
| ';'
{
}
;

// ----------------------------------------------------------------------
strings
// ----------------------------------------------------------------------
: YELLOW_STRING strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| YELLOW_STRING
{
    $$ = $1
}
;

%%
