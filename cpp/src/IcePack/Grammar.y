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
%token ICE_PACK_SERVER
%token ICE_PACK_ADAPTER
%token ICE_PACK_ADD
%token ICE_PACK_REMOVE
%token ICE_PACK_LIST
%token ICE_PACK_SHUTDOWN
%token ICE_PACK_STRING
%token ICE_PACK_START
%token ICE_PACK_DESCRIBE
%token ICE_PACK_STATE
%token ICE_PACK_ENDPOINTS
%token ICE_PACK_ADAPTERS
%token ICE_PACK_OPTIONS
%token ICE_PACK_OPEN_BRACKET
%token ICE_PACK_CLOSE_BRACKET

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
| ICE_PACK_SERVER ICE_PACK_ADD server_add_options ';'
{
}
| ICE_PACK_SERVER ICE_PACK_START strings ';'
{
    parser->startServer($3);
}
| ICE_PACK_SERVER ICE_PACK_DESCRIBE strings ';'
{
    parser->describeServer($3);
}
| ICE_PACK_SERVER ICE_PACK_STATE strings ';'
{
    parser->stateServer($3);
}
| ICE_PACK_SERVER ICE_PACK_REMOVE strings ';'
{
    parser->removeServer($3);
}
| ICE_PACK_SERVER ICE_PACK_LIST ';'
{
    parser->listAllServers();
}
| ICE_PACK_ADAPTER ICE_PACK_ADD strings ';'
{
    parser->addAdapter($3);
}
| ICE_PACK_ADAPTER ICE_PACK_ENDPOINTS strings ';'
{
    parser->endpointsAdapter($3);
}
| ICE_PACK_ADAPTER ICE_PACK_REMOVE strings ';'
{
    parser->removeAdapter($3);
}
| ICE_PACK_ADAPTER ICE_PACK_LIST ';'
{
    parser->listAllAdapters();
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
server_add_options
// ----------------------------------------------------------------------
: strings adapters options
{
    parser->addServer($1, $2, $3);
}
| strings options adapters
{
    parser->addServer($1, $3, $2);
}
| strings adapters
{
    parser->addServer($1, $2, list<string>());
}
| strings options
{
    parser->addServer($1, list<string>(), $2);
}
| strings
{
    parser->addServer($1, list<string>(), list<string>());
};

// ----------------------------------------------------------------------
adapters
// ----------------------------------------------------------------------
: ICE_PACK_ADAPTERS ICE_PACK_OPEN_BRACKET strings ICE_PACK_CLOSE_BRACKET
{
    $$ = $3;
}
| ICE_PACK_ADAPTERS ICE_PACK_OPEN_BRACKET ICE_PACK_CLOSE_BRACKET
{
    $$ = list<string>();
};

// ----------------------------------------------------------------------
options
// ----------------------------------------------------------------------
: ICE_PACK_OPTIONS ICE_PACK_OPEN_BRACKET strings ICE_PACK_CLOSE_BRACKET
{
    $$ = $3;
}
| ICE_PACK_OPTIONS ICE_PACK_OPEN_BRACKET ICE_PACK_CLOSE_BRACKET
{
    $$ = list<string>();    
};

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
