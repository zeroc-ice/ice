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

using namespace std;
using namespace Ice;
using namespace IcePack;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%token ICE_PACK_EXIT
%token ICE_PACK_ADD
%token ICE_PACK_REMOVE
%token ICE_PACK_SHUTDOWN
%token ICE_PACK_REFERENCE

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
: ICE_PACK_EXIT ';'
{
    return 0;
}
| ICE_PACK_ADD references ';'
{
    parser->add($2);
}
| ICE_PACK_REMOVE references ';'
{
    parser->remove($2);
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
references
// ----------------------------------------------------------------------
: ICE_PACK_REFERENCE references
{
    $1.front().erase(0, 1);
    $1.front().erase($1.front().size() - 2);
    $$ = $2;
    $$.push_front($1.front());
}
| ICE_PACK_REFERENCE
{
    $1.front().erase(0, 1);
    $1.front().erase($1.front().size() - 2);
    $$ = $1
}
;

%%
