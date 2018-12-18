%{

// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/Parser.h>

#ifdef _MSC_VER
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning(disable:4102)
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning(disable:4065)
// warning C4244: '=': conversion from 'int' to 'yytype_int16', possible loss of data
#   pragma warning(disable:4244)
// warning C4702: unreachable code
#   pragma warning(disable:4702)
#endif

//
// Avoid old style cast warnings in generated grammar
//
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wold-style-cast"
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

%pure-parser

//
// All keyword tokens. Make sure to modify the "keyword" rule in this
// file if the list of keywords is changed. Also make sure to add the
// keyword to the keyword table in Scanner.l.
//
%token ICE_STORM_HELP
%token ICE_STORM_EXIT
%token ICE_STORM_CURRENT
%token ICE_STORM_CREATE
%token ICE_STORM_DESTROY
%token ICE_STORM_LINK
%token ICE_STORM_UNLINK
%token ICE_STORM_LINKS
%token ICE_STORM_TOPICS
%token ICE_STORM_REPLICA
%token ICE_STORM_SUBSCRIBERS
%token ICE_STORM_STRING

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
| ICE_STORM_CURRENT strings ';'
{
    parser->current($2);
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
| ICE_STORM_LINKS strings ';'
{
    parser->links($2);
}
| ICE_STORM_TOPICS strings ';'
{
    parser->topics($2);
}
| ICE_STORM_REPLICA strings ';'
{
    parser->replica($2);
}
| ICE_STORM_SUBSCRIBERS strings ';'
{
    parser->subscribers($2);
}
| ICE_STORM_STRING error ';'
{
    parser->invalidCommand("unknown command `" + $1.front() + "' (type `help' for more info)");
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
| keyword strings
{
    $$ = $2;
    $$.push_front($1.front());
}
|
{
    $$ = YYSTYPE();
}
;

// ----------------------------------------------------------------------
keyword
// ----------------------------------------------------------------------
: ICE_STORM_HELP
{
}
| ICE_STORM_EXIT
{
}
| ICE_STORM_CURRENT
{
}
| ICE_STORM_CREATE
{
}
| ICE_STORM_DESTROY
{
}
| ICE_STORM_LINK
{
}
| ICE_STORM_UNLINK
{
}
| ICE_STORM_LINKS
{
}
| ICE_STORM_TOPICS
{
}

%%
