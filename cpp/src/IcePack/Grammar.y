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
#include <IcePack/Parser.h>

#ifdef _WIN32
// I get this warning from some bison version:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
#endif

using namespace std;
using namespace Ice;
using namespace IcePack;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%pure_parser

%token ICE_PACK_HELP
%token ICE_PACK_EXIT
%token ICE_PACK_ADD
%token ICE_PACK_REMOVE
%token ICE_PACK_LIST
%token ICE_PACK_SHUTDOWN
%token ICE_PACK_STRING

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
: ICE_PACK_HELP ';'
{
    parser->usage();
}
| ICE_PACK_EXIT ';'
{
    return 0;
}
| ICE_PACK_ADD strings ';'
{
    parser->add($2);
}
| ICE_PACK_REMOVE strings ';'
{
    parser->remove($2);
}
| ICE_PACK_LIST ';'
{
    parser->listAll();
}
| ICE_PACK_SHUTDOWN ';'
{
    parser->shutdown();
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
: ICE_PACK_STRING strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| ICE_PACK_STRING
{
    $$ = $1
}
;

%%
