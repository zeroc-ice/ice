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
#include <IceStorm/Parser.h>

#ifdef _MSC_VER
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
#endif

using namespace std;
using namespace Ice;
using namespace IceStorm;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%pure_parser

%token ICE_STORM_HELP
%token ICE_STORM_EXIT
%token ICE_STORM_CREATE
%token ICE_STORM_DESTROY
%token ICE_STORM_LIST
%token ICE_STORM_LINK
%token ICE_STORM_UNLINK
%token ICE_STORM_GRAPH
%token ICE_STORM_STRING
%token ICE_STORM_SHOW
%token ICE_STORM_COPYING
%token ICE_STORM_WARRANTY

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
: ICE_STORM_HELP ';'
{
    parser->usage();
}
| ICE_STORM_EXIT ';'
{
    return 0;
}
| ICE_STORM_CREATE strings ';'
{
    parser->create($2);
}
| ICE_STORM_DESTROY strings ';'
{
    parser->destroy($2);
}
| ICE_STORM_LINK strings ';'
{
    parser->link($2);
}
| ICE_STORM_UNLINK strings ';'
{
    parser->unlink($2);
}
| ICE_STORM_GRAPH strings ';'
{
    parser->graph($2);
}
| ICE_STORM_LIST ';'
{
    std::list<std::string> args;
    parser->dolist(args);
}
| ICE_STORM_LIST strings ';'
{
    parser->dolist($2);
}
| ICE_STORM_SHOW ICE_STORM_COPYING ';'
{
    parser->showCopying();
}
| ICE_STORM_SHOW ICE_STORM_WARRANTY ';'
{
    parser->showWarranty();
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
: ICE_STORM_STRING strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| ICE_STORM_STRING
{
    $$ = $1;
}
;

%%
