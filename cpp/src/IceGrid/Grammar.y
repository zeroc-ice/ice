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
#include <IceGrid/Parser.h>

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
using namespace IceGrid;

void
yyerror(const char* s)
{
    parser->invalidCommand(s);
}

%}

%pure-parser

//
// All keyword tokens. Make sure to modify the "keyword" rule in this
// file if the list of keywords is changed. Also make sure to add the
// keyword to the keyword table in Scanner.l.
//
%token ICE_GRID_HELP
%token ICE_GRID_EXIT
%token ICE_GRID_APPLICATION
%token ICE_GRID_NODE
%token ICE_GRID_REGISTRY
%token ICE_GRID_SERVER
%token ICE_GRID_ADAPTER
%token ICE_GRID_PING
%token ICE_GRID_LOAD
%token ICE_GRID_SOCKETS
%token ICE_GRID_ADD
%token ICE_GRID_REMOVE
%token ICE_GRID_LIST
%token ICE_GRID_SHUTDOWN
%token ICE_GRID_STRING
%token ICE_GRID_START
%token ICE_GRID_STOP
%token ICE_GRID_PATCH
%token ICE_GRID_SIGNAL
%token ICE_GRID_STDOUT
%token ICE_GRID_STDERR
%token ICE_GRID_DESCRIBE
%token ICE_GRID_PROPERTIES
%token ICE_GRID_PROPERTY
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
%token ICE_GRID_ENABLE
%token ICE_GRID_DISABLE

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
: commands checkInterrupted command
{
}
| checkInterrupted command
{
}
;

checkInterrupted
:
{
    parser->checkInterrupted();
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
| ICE_GRID_APPLICATION ICE_GRID_ADD ICE_GRID_HELP ';'
{
    parser->usage("application", "add");
}
| ICE_GRID_APPLICATION ICE_GRID_REMOVE strings ';'
{
    parser->removeApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_REMOVE ICE_GRID_HELP ';'
{
    parser->usage("application", "remove");
}
| ICE_GRID_APPLICATION ICE_GRID_DIFF strings ';'
{
    parser->diffApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_DIFF ICE_GRID_HELP ';'
{
    parser->usage("application", "diff");
}
| ICE_GRID_APPLICATION ICE_GRID_UPDATE strings ';'
{
    parser->updateApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_UPDATE ICE_GRID_HELP ';'
{
    parser->usage("application", "update");
}
| ICE_GRID_APPLICATION ICE_GRID_DESCRIBE strings ';'
{
    parser->describeApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("application", "describe");
}
| ICE_GRID_APPLICATION ICE_GRID_PATCH strings ';'
{
    parser->patchApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_PATCH ICE_GRID_HELP ';'
{
    parser->usage("application", "patch");
}
| ICE_GRID_APPLICATION ICE_GRID_LIST strings ';'
{
    parser->listAllApplications($3);
}
| ICE_GRID_APPLICATION ICE_GRID_LIST ICE_GRID_HELP ';'
{
    parser->usage("application", "list");
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE strings ';'
{
    parser->describeServerTemplate($4);
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("server template", "describe");
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_INSTANTIATE strings ';'
{
    parser->instantiateServerTemplate($4);
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_INSTANTIATE ICE_GRID_HELP ';'
{
    parser->usage("server template", "instantiate");
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_HELP ';'
{
    parser->usage("server template");
}
| ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE strings ';'
{
    parser->describeServiceTemplate($4);
}
| ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("service template", "describe");
}
| ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_HELP ';'
{
    parser->usage("service template");
}
| ICE_GRID_NODE ICE_GRID_DESCRIBE strings ';'
{
    parser->describeNode($3);
}
| ICE_GRID_NODE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("node", "describe");
}
| ICE_GRID_NODE ICE_GRID_PING strings ';'
{
    parser->pingNode($3);
}
| ICE_GRID_NODE ICE_GRID_PING ICE_GRID_HELP ';'
{
    parser->usage("node", "ping");
}
| ICE_GRID_NODE ICE_GRID_LOAD strings ';'
{
    parser->printLoadNode($3);
}
| ICE_GRID_NODE ICE_GRID_LOAD ICE_GRID_HELP ';'
{
    parser->usage("node", "load");
}
| ICE_GRID_NODE ICE_GRID_SOCKETS strings ';'
{
    parser->printNodeProcessorSockets($3);
}
| ICE_GRID_NODE ICE_GRID_SOCKETS ICE_GRID_HELP ';'
{
    parser->usage("node", "sockets");
}
| ICE_GRID_NODE ICE_GRID_SHUTDOWN strings ';'
{
    parser->shutdownNode($3);
}
| ICE_GRID_NODE ICE_GRID_SHUTDOWN ICE_GRID_HELP ';'
{
    parser->usage("node", "shutdown");
}
| ICE_GRID_NODE ICE_GRID_LIST strings ';'
{
    parser->listAllNodes($3);
}
| ICE_GRID_NODE ICE_GRID_LIST ICE_GRID_HELP ';'
{
    parser->usage("node", "list");
}
| ICE_GRID_NODE ICE_GRID_SHOW strings ';'
{
    parser->show("node", $3);
}
| ICE_GRID_NODE ICE_GRID_SHOW ICE_GRID_HELP ';'
{
    parser->usage("node", "show");
}
| ICE_GRID_REGISTRY ICE_GRID_DESCRIBE strings ';'
{
    parser->describeRegistry($3);
}
| ICE_GRID_REGISTRY ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("registry", "describe");
}
| ICE_GRID_REGISTRY ICE_GRID_PING strings ';'
{
    parser->pingRegistry($3);
}
| ICE_GRID_REGISTRY ICE_GRID_PING ICE_GRID_HELP ';'
{
    parser->usage("registry", "ping");
}
| ICE_GRID_REGISTRY ICE_GRID_SHUTDOWN strings ';'
{
    parser->shutdownRegistry($3);
}
| ICE_GRID_REGISTRY ICE_GRID_SHUTDOWN ICE_GRID_HELP ';'
{
    parser->usage("registry", "shutdown");
}
| ICE_GRID_REGISTRY ICE_GRID_LIST strings ';'
{
    parser->listAllRegistries($3);
}
| ICE_GRID_REGISTRY ICE_GRID_LIST ICE_GRID_HELP ';'
{
    parser->usage("registry", "list");
}
| ICE_GRID_REGISTRY ICE_GRID_SHOW strings ';'
{
    parser->show("registry", $3);
}
| ICE_GRID_REGISTRY ICE_GRID_SHOW ICE_GRID_HELP ';'
{
    parser->usage("registry", "show");
}
| ICE_GRID_SERVER ICE_GRID_REMOVE strings ';'
{
    parser->removeServer($3);
}
| ICE_GRID_SERVER ICE_GRID_REMOVE ICE_GRID_HELP ';'
{
    parser->usage("server", "remove");
}
| ICE_GRID_SERVER ICE_GRID_DESCRIBE strings ';'
{
    parser->describeServer($3);
}
| ICE_GRID_SERVER ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("server", "describe");
}
| ICE_GRID_SERVER ICE_GRID_START strings ';'
{
    parser->startServer($3);
}
| ICE_GRID_SERVER ICE_GRID_START ICE_GRID_HELP ';'
{
    parser->usage("server", "start");
}
| ICE_GRID_SERVER ICE_GRID_STOP strings ';'
{
    parser->stopServer($3);
}
| ICE_GRID_SERVER ICE_GRID_STOP ICE_GRID_HELP ';'
{
    parser->usage("server", "stop");
}
| ICE_GRID_SERVER ICE_GRID_PATCH strings ';'
{
    parser->patchServer($3);
}
| ICE_GRID_SERVER ICE_GRID_PATCH ICE_GRID_HELP ';'
{
    parser->usage("server", "patch");
}
| ICE_GRID_SERVER ICE_GRID_SIGNAL strings ';'
{
    parser->signalServer($3);
}
| ICE_GRID_SERVER ICE_GRID_SIGNAL ICE_GRID_HELP ';'
{
    parser->usage("server", "signal");
}
| ICE_GRID_SERVER ICE_GRID_STDOUT strings ';'
{
    parser->writeMessage($3, 1);
}
| ICE_GRID_SERVER ICE_GRID_STDOUT ICE_GRID_HELP ';'
{
    parser->usage("server", "stdout");
}
| ICE_GRID_SERVER ICE_GRID_STDERR strings ';'
{
    parser->writeMessage($3, 2);
}
| ICE_GRID_SERVER ICE_GRID_STDERR ICE_GRID_HELP ';'
{
    parser->usage("server", "stderr");
}
| ICE_GRID_SERVER ICE_GRID_STATE strings ';'
{
    parser->stateServer($3);
}
| ICE_GRID_SERVER ICE_GRID_STATE ICE_GRID_HELP ';'
{
    parser->usage("server", "start");
}
| ICE_GRID_SERVER ICE_GRID_PID strings ';'
{
    parser->pidServer($3);
}
| ICE_GRID_SERVER ICE_GRID_PID ICE_GRID_HELP ';'
{
    parser->usage("server", "pid");
}
| ICE_GRID_SERVER ICE_GRID_PROPERTIES strings ';'
{
    parser->propertiesServer($3, false);
}
| ICE_GRID_SERVER ICE_GRID_PROPERTIES ICE_GRID_HELP ';'
{
    parser->usage("server", "properties");
}
| ICE_GRID_SERVER ICE_GRID_PROPERTY strings ';'
{
    parser->propertiesServer($3, true);
}
| ICE_GRID_SERVER ICE_GRID_PROPERTY ICE_GRID_HELP ';'
{
    parser->usage("server", "property");
}
| ICE_GRID_SERVER ICE_GRID_ENABLE strings ';'
{
    parser->enableServer($3, true);
}
| ICE_GRID_SERVER ICE_GRID_ENABLE ICE_GRID_HELP ';'
{
    parser->usage("server", "enable");
}
| ICE_GRID_SERVER ICE_GRID_DISABLE strings ';'
{
    parser->enableServer($3, false);
}
| ICE_GRID_SERVER ICE_GRID_DISABLE ICE_GRID_HELP ';'
{
    parser->usage("server", "disable");
}
| ICE_GRID_SERVER ICE_GRID_LIST strings ';'
{
    parser->listAllServers($3);
}
| ICE_GRID_SERVER ICE_GRID_LIST ICE_GRID_HELP ';'
{
    parser->usage("server", "list");
}
| ICE_GRID_SERVER ICE_GRID_SHOW strings ';'
{
    parser->show("server", $3);
}
| ICE_GRID_SERVER ICE_GRID_SHOW ICE_GRID_HELP ';'
{
    parser->usage("server", "show");
}
| ICE_GRID_SERVICE ICE_GRID_START strings ';'
{
    parser->startService($3);
}
| ICE_GRID_SERVICE ICE_GRID_START ICE_GRID_HELP ';'
{
    parser->usage("service", "start");
}
| ICE_GRID_SERVICE ICE_GRID_STOP strings ';'
{
    parser->stopService($3);
}
| ICE_GRID_SERVICE ICE_GRID_STOP ICE_GRID_HELP ';'
{
    parser->usage("service", "stop");
}
| ICE_GRID_SERVICE ICE_GRID_DESCRIBE strings ';'
{
    parser->describeService($3);
}
| ICE_GRID_SERVICE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("service", "describe");
}
| ICE_GRID_SERVICE ICE_GRID_PROPERTIES strings ';'
{
    parser->propertiesService($3, false);
}
| ICE_GRID_SERVICE ICE_GRID_PROPERTIES ICE_GRID_HELP ';'
{
    parser->usage("service", "properties");
}
| ICE_GRID_SERVICE ICE_GRID_PROPERTY strings ';'
{
    parser->propertiesService($3, true);
}
| ICE_GRID_SERVICE ICE_GRID_PROPERTY ICE_GRID_HELP ';'
{
    parser->usage("service", "property");
}
| ICE_GRID_SERVICE ICE_GRID_LIST strings ';'
{
    parser->listServices($3);
}
| ICE_GRID_SERVICE ICE_GRID_LIST ICE_GRID_HELP ';'
{
    parser->usage("service", "list");
}
| ICE_GRID_ADAPTER ICE_GRID_ENDPOINTS strings ';'
{
    parser->endpointsAdapter($3);
}
| ICE_GRID_ADAPTER ICE_GRID_ENDPOINTS ICE_GRID_HELP ';'
{
    parser->usage("adapter", "endpoints");
}
| ICE_GRID_ADAPTER ICE_GRID_REMOVE strings ';'
{
    parser->removeAdapter($3);
}
| ICE_GRID_ADAPTER ICE_GRID_REMOVE ICE_GRID_HELP ';'
{
    parser->usage("adapter", "remove");
}
| ICE_GRID_ADAPTER ICE_GRID_LIST strings ';'
{
    parser->listAllAdapters($3);
}
| ICE_GRID_ADAPTER ICE_GRID_LIST ICE_GRID_HELP ';'
{
    parser->usage("adapter", "list");
}
| ICE_GRID_OBJECT ICE_GRID_ADD strings ';'
{
    parser->addObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_ADD ICE_GRID_HELP ';'
{
    parser->usage("object", "add");
}
| ICE_GRID_OBJECT ICE_GRID_REMOVE strings ';'
{
    parser->removeObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_REMOVE ICE_GRID_HELP ';'
{
    parser->usage("object", "remove");
}
| ICE_GRID_OBJECT ICE_GRID_FIND strings ';'
{
    parser->findObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_FIND ICE_GRID_HELP ';'
{
    parser->usage("object", "find");
}
| ICE_GRID_OBJECT ICE_GRID_LIST strings ';'
{
    parser->listObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_LIST ICE_GRID_HELP ';'
{
    parser->usage("object", "list");
}
| ICE_GRID_OBJECT ICE_GRID_DESCRIBE strings ';'
{
    parser->describeObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("object", "describe");
}
| ICE_GRID_SHOW ICE_GRID_COPYING ';'
{
    parser->showCopying();
}
| ICE_GRID_SHOW ICE_GRID_WARRANTY ';'
{
    parser->showWarranty();
}
| ICE_GRID_HELP keyword ';'
{
    parser->usage($2.front());
}
| ICE_GRID_HELP keyword keyword ';'
{
    if(($2.front() == "server" || $2.front() == "service") && $3.front() == "template")
    {
        parser->usage($2.front() + " " + $3.front());
    }
    else
    {
        parser->usage($2.front(), $3.front());
    }
}
| ICE_GRID_HELP keyword ICE_GRID_STRING strings ';'
{
    parser->usage($2.front(), $3.front());
}
| ICE_GRID_HELP keyword keyword keyword ';'
{
    if(($2.front() == "server" || $2.front() == "service") && $3.front() == "template")
    {
        parser->usage($2.front() + " " + $3.front(), $4.front());
    }
    else
    {
        parser->usage($2.front(), $3.front());
    }
}
| ICE_GRID_HELP keyword keyword ICE_GRID_STRING strings ';'
{
    if(($2.front() == "server" || $2.front() == "service") && $3.front() == "template")
    {
        parser->usage($2.front() + " " + $3.front(), $4.front());
    }
    else
    {
        parser->usage($2.front(), $3.front());
    }
}
| ICE_GRID_HELP ICE_GRID_STRING strings ';'
{
    parser->usage($2.front());
}
| ICE_GRID_HELP error ';'
{
    parser->usage();
}
| keyword ICE_GRID_HELP ';'
{
    parser->usage($1.front());
}
| keyword ICE_GRID_STRING error ';'
{
    $1.push_back($2.front());
    parser->invalidCommand($1);
    yyerrok;
}
| keyword error ';'
{
    parser->invalidCommand($1);
    yyerrok;
}
| ICE_GRID_STRING error ';'
{
    parser->invalidCommand($1);
    yyerrok;
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
| keyword strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| ICE_GRID_STRING ICE_GRID_HELP strings
{
    $$ = $2;
    $$.push_front("help");
    $$.push_front($1.front());
}
| keyword ICE_GRID_HELP strings
{
    $$ = $2;
    $$.push_front("help");
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
: ICE_GRID_EXIT
{
}
| ICE_GRID_APPLICATION
{
}
| ICE_GRID_NODE
{
}
| ICE_GRID_REGISTRY
{
}
| ICE_GRID_SERVER
{
}
| ICE_GRID_ADAPTER
{
}
| ICE_GRID_PING
{
}
| ICE_GRID_LOAD
{
}
| ICE_GRID_SOCKETS
{
}
| ICE_GRID_ADD
{
}
| ICE_GRID_REMOVE
{
}
| ICE_GRID_LIST
{
}
| ICE_GRID_SHUTDOWN
{
}
| ICE_GRID_START
{
}
| ICE_GRID_STOP
{
}
| ICE_GRID_PATCH
{
}
| ICE_GRID_SIGNAL
{
}
| ICE_GRID_DESCRIBE
{
}
| ICE_GRID_STATE
{
}
| ICE_GRID_PID
{
}
| ICE_GRID_PROPERTIES
{
}
| ICE_GRID_PROPERTY
{
}
| ICE_GRID_ENDPOINTS
{
}
| ICE_GRID_ACTIVATION
{
}
| ICE_GRID_OBJECT
{
}
| ICE_GRID_FIND
{
}
| ICE_GRID_SHOW
{
}
| ICE_GRID_COPYING
{
}
| ICE_GRID_WARRANTY
{
}
| ICE_GRID_DIFF
{
}
| ICE_GRID_UPDATE
{
}
| ICE_GRID_INSTANTIATE
{
}
| ICE_GRID_TEMPLATE
{
}
| ICE_GRID_SERVICE
{
}
| ICE_GRID_ENABLE
{
}
| ICE_GRID_DISABLE
{
}
| ICE_GRID_STDERR
{
}
| ICE_GRID_STDOUT
{
}
;
