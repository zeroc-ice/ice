%{

// **********************************************************************
//
// Copyright (c) 2001
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
%token ICE_PACK_APPLICATION
%token ICE_PACK_NODE
%token ICE_PACK_SERVER
%token ICE_PACK_ADAPTER
%token ICE_PACK_PING
%token ICE_PACK_ADD
%token ICE_PACK_REMOVE
%token ICE_PACK_LIST
%token ICE_PACK_SHUTDOWN
%token ICE_PACK_STRING
%token ICE_PACK_START
%token ICE_PACK_STOP
%token ICE_PACK_DESCRIBE
%token ICE_PACK_STATE
%token ICE_PACK_PID
%token ICE_PACK_ENDPOINTS
%token ICE_PACK_ACTIVATION

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
| ICE_PACK_APPLICATION ICE_PACK_ADD strings ';'
{
    parser->addApplication($3);
}
| ICE_PACK_APPLICATION ICE_PACK_REMOVE strings ';'
{
    parser->removeApplication($3);
}
| ICE_PACK_NODE ICE_PACK_PING strings ';'
{
    parser->pingNode($3);
}
| ICE_PACK_NODE ICE_PACK_SHUTDOWN strings ';'
{
    parser->shutdownNode($3);
}
| ICE_PACK_NODE ICE_PACK_LIST ';'
{
    parser->listAllNodes();
}
| ICE_PACK_SERVER ICE_PACK_ADD strings ';'
{
    parser->addServer($3);
}
| ICE_PACK_SERVER ICE_PACK_START strings ';'
{
    parser->startServer($3);
}
| ICE_PACK_SERVER ICE_PACK_STOP strings ';'
{
    parser->stopServer($3);
}
| ICE_PACK_SERVER ICE_PACK_DESCRIBE strings ';'
{
    parser->describeServer($3);
}
| ICE_PACK_SERVER ICE_PACK_STATE strings ';'
{
    parser->stateServer($3);
}
| ICE_PACK_SERVER ICE_PACK_PID strings ';'
{
    parser->pidServer($3);
}
| ICE_PACK_SERVER ICE_PACK_ACTIVATION strings ';'
{
    parser->activationServer($3);
}
| ICE_PACK_SERVER ICE_PACK_REMOVE strings ';'
{
    parser->removeServer($3);
}
| ICE_PACK_SERVER ICE_PACK_LIST ';'
{
    parser->listAllServers();
}
| ICE_PACK_ADAPTER ICE_PACK_ENDPOINTS strings ';'
{
    parser->endpointsAdapter($3);
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
