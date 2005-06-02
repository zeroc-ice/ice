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
#include <IceGrid/Parser.h>

#ifdef _WIN32
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%pure_parser

%token ICE_GRID_HELP
%token ICE_GRID_EXIT
%token ICE_GRID_APPLICATION
%token ICE_GRID_NODE
%token ICE_GRID_SERVER
%token ICE_GRID_ADAPTER
%token ICE_GRID_PING
%token ICE_GRID_ADD
%token ICE_GRID_REMOVE
%token ICE_GRID_LIST
%token ICE_GRID_SHUTDOWN
%token ICE_GRID_STRING
%token ICE_GRID_START
%token ICE_GRID_STOP
%token ICE_GRID_SIGNAL
%token ICE_GRID_STDOUT
%token ICE_GRID_STDERR
%token ICE_GRID_DESCRIBE
%token ICE_GRID_STATE
%token ICE_GRID_PID
%token ICE_GRID_ENDPOINTS
%token ICE_GRID_ACTIVATION
%token ICE_GRID_OBJECT
%token ICE_GRID_FIND
%token ICE_GRID_SHOW
%token ICE_GRID_COPYING
%token ICE_GRID_WARRANTY
%token ICE_GRID_DIFF
%token ICE_GRID_UPDATE
%token ICE_GRID_INSTANTIATE
%token ICE_GRID_TEMPLATE
%token ICE_GRID_SERVICE

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
: ICE_GRID_HELP ';'
{
    parser->usage();
}
| ICE_GRID_EXIT ';'
{
    return 0;
}
| ICE_GRID_APPLICATION ICE_GRID_ADD strings ';'
{
    parser->addApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_REMOVE strings ';'
{
    parser->removeApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_DIFF strings ';'
{
    parser->diffApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_UPDATE strings ';'
{
    parser->updateApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_DESCRIBE strings ';'
{
    parser->describeApplication($3);
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE strings ';'
{
    parser->describeServerTemplate($4);
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_INSTANTIATE strings ';'
{
    parser->instantiateServerTemplate($4);
}
| ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE strings ';'
{
    parser->describeServiceTemplate($4);
}
| ICE_GRID_APPLICATION ICE_GRID_LIST ';'
{
    parser->listAllApplications();
}
| ICE_GRID_NODE ICE_GRID_PING strings ';'
{
    parser->pingNode($3);
}
| ICE_GRID_NODE ICE_GRID_SHUTDOWN strings ';'
{
    parser->shutdownNode($3);
}
| ICE_GRID_NODE ICE_GRID_REMOVE strings ';'
{
    parser->removeNode($3);
}
| ICE_GRID_NODE ICE_GRID_LIST ';'
{
    parser->listAllNodes();
}
| ICE_GRID_SERVER ICE_GRID_DESCRIBE strings ';'
{
    parser->describeServer($3);
}
| ICE_GRID_SERVER ICE_GRID_START strings ';'
{
    parser->startServer($3);
}
| ICE_GRID_SERVER ICE_GRID_STOP strings ';'
{
    parser->stopServer($3);
}
| ICE_GRID_SERVER ICE_GRID_SIGNAL strings ';'
{
    parser->signalServer($3);
}
| ICE_GRID_SERVER ICE_GRID_STDOUT strings ';'
{
    parser->writeMessage($3, 1);
}
| ICE_GRID_SERVER ICE_GRID_STDERR strings ';'
{
    parser->writeMessage($3, 2);
}
| ICE_GRID_SERVER ICE_GRID_STATE strings ';'
{
    parser->stateServer($3);
}
| ICE_GRID_SERVER ICE_GRID_PID strings ';'
{
    parser->pidServer($3);
}
| ICE_GRID_SERVER ICE_GRID_ACTIVATION strings ';'
{
    parser->activationServer($3);
}
| ICE_GRID_SERVER ICE_GRID_LIST ';'
{
    parser->listAllServers();
}
| ICE_GRID_ADAPTER ICE_GRID_ENDPOINTS strings ';'
{
    parser->endpointsAdapter($3);
}
| ICE_GRID_ADAPTER ICE_GRID_LIST ';'
{
    parser->listAllAdapters();
}
| ICE_GRID_OBJECT ICE_GRID_ADD strings ';'
{
    parser->addObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_REMOVE strings ';'
{
    parser->removeObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_FIND strings ';'
{
    parser->findObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_LIST optional_strings ';'
{
    parser->listObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_DESCRIBE optional_strings ';'
{
    parser->describeObject($3);
}
| ICE_GRID_SHUTDOWN ';'
{
    parser->shutdown();
}
| ICE_GRID_SHOW ICE_GRID_COPYING ';'
{
    parser->showCopying();
}
| ICE_GRID_SHOW ICE_GRID_WARRANTY ';'
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
: ICE_GRID_STRING strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| ICE_GRID_STRING
{
    $$ = $1;
}
;

// ----------------------------------------------------------------------
optional_strings
// ----------------------------------------------------------------------
: ICE_GRID_STRING optional_strings
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
