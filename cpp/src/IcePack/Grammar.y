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
#include <IcePack/Parser.h>

#ifdef _WIN32
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
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
%token ICE_PACK_SIGNAL
%token ICE_PACK_STDOUT
%token ICE_PACK_STDERR
%token ICE_PACK_DESCRIBE
%token ICE_PACK_STATE
%token ICE_PACK_PID
%token ICE_PACK_ENDPOINTS
%token ICE_PACK_ACTIVATION
%token ICE_PACK_OBJECT
%token ICE_PACK_FIND
%token ICE_PACK_SHOW
%token ICE_PACK_COPYING
%token ICE_PACK_WARRANTY
%token ICE_PACK_DIFF
%token ICE_PACK_UPDATE

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
| ICE_PACK_APPLICATION ICE_PACK_DIFF strings ';'
{
    parser->diffApplication($3);
}
| ICE_PACK_APPLICATION ICE_PACK_UPDATE strings ';'
{
    parser->updateApplication($3);
}
| ICE_PACK_APPLICATION ICE_PACK_DESCRIBE strings ';'
{
    parser->describeApplication($3);
}
| ICE_PACK_APPLICATION ICE_PACK_LIST ';'
{
    parser->listAllApplications();
}
| ICE_PACK_NODE ICE_PACK_PING strings ';'
{
    parser->pingNode($3);
}
| ICE_PACK_NODE ICE_PACK_SHUTDOWN strings ';'
{
    parser->shutdownNode($3);
}
| ICE_PACK_NODE ICE_PACK_REMOVE strings ';'
{
    parser->removeNode($3);
}
| ICE_PACK_NODE ICE_PACK_LIST ';'
{
    parser->listAllNodes();
}
| ICE_PACK_SERVER ICE_PACK_ADD strings ';'
{
    parser->addServer($3);
}
| ICE_PACK_SERVER ICE_PACK_UPDATE strings ';'
{
    parser->updateServer($3);
}
| ICE_PACK_SERVER ICE_PACK_DESCRIBE strings ';'
{
    parser->describeServer($3);
}
| ICE_PACK_SERVER ICE_PACK_START strings ';'
{
    parser->startServer($3);
}
| ICE_PACK_SERVER ICE_PACK_STOP strings ';'
{
    parser->stopServer($3);
}
| ICE_PACK_SERVER ICE_PACK_SIGNAL strings ';'
{
    parser->signalServer($3);
}
| ICE_PACK_SERVER ICE_PACK_STDOUT strings ';'
{
    parser->writeMessage($3, 1);
}
| ICE_PACK_SERVER ICE_PACK_STDERR strings ';'
{
    parser->writeMessage($3, 2);
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
| ICE_PACK_OBJECT ICE_PACK_ADD strings ';'
{
    parser->addObject($3);
}
| ICE_PACK_OBJECT ICE_PACK_REMOVE strings ';'
{
    parser->removeObject($3);
}
| ICE_PACK_OBJECT ICE_PACK_FIND strings ';'
{
    parser->findObject($3);
}
| ICE_PACK_OBJECT ICE_PACK_LIST optional_strings ';'
{
    parser->listObject($3);
}
| ICE_PACK_OBJECT ICE_PACK_DESCRIBE optional_strings ';'
{
    parser->describeObject($3);
}
| ICE_PACK_SHUTDOWN ';'
{
    parser->shutdown();
}
| ICE_PACK_SHOW ICE_PACK_COPYING ';'
{
    parser->showCopying();
}
| ICE_PACK_SHOW ICE_PACK_WARRANTY ';'
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
: ICE_PACK_STRING strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| ICE_PACK_STRING
{
    $$ = $1;
}
;

// ----------------------------------------------------------------------
optional_strings
// ----------------------------------------------------------------------
: ICE_PACK_STRING optional_strings
{
    $$ = $2;
    $$.push_front($1.front());
}
|
{
    $$ = YYSTYPE()
}
;

%%
