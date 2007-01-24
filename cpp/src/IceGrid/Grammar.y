%{

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Parser.h>

#ifdef _MSC_VER
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
    parser->invalidCommand(s);
}

%}

%pure_parser

%token ICE_GRID_HELP
%token ICE_GRID_EXIT
%token ICE_GRID_APPLICATION
%token ICE_GRID_NODE
%token ICE_GRID_REGISTRY
%token ICE_GRID_SERVER
%token ICE_GRID_ADAPTER
%token ICE_GRID_PING
%token ICE_GRID_LOAD
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
| ICE_GRID_HELP ICE_GRID_APPLICATION ';'
{
    parser->usage("application");
}
| ICE_GRID_HELP ICE_GRID_NODE ';'
{
    parser->usage("node");
}
| ICE_GRID_HELP ICE_GRID_REGISTRY ';'
{
    parser->usage("registry");
}
| ICE_GRID_HELP ICE_GRID_SERVER ';'
{
    parser->usage("server");
}
| ICE_GRID_HELP ICE_GRID_ADAPTER ';'
{
    parser->usage("adapter");
}
| ICE_GRID_HELP ICE_GRID_OBJECT ';'
{
    parser->usage("object");
}
| ICE_GRID_HELP ICE_GRID_SERVER ICE_GRID_TEMPLATE ';'
{
    parser->usage("server template");
}
| ICE_GRID_HELP ICE_GRID_SERVICE ICE_GRID_TEMPLATE ';'
{
    parser->usage("servcice template");
}
| ICE_GRID_APPLICATION ICE_GRID_HELP ';'
{
    parser->usage("application");
}
| ICE_GRID_NODE ICE_GRID_HELP ';'
{
    parser->usage("node");
}
| ICE_GRID_REGISTRY ICE_GRID_HELP ';'
{
    parser->usage("registry");
}
| ICE_GRID_SERVER ICE_GRID_HELP ';'
{
    parser->usage("server");
}
| ICE_GRID_ADAPTER ICE_GRID_HELP ';'
{
    parser->usage("adapter");
}
| ICE_GRID_OBJECT ICE_GRID_HELP ';'
{
    parser->usage("object");
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_HELP ';'
{
    parser->usage("server template");
}
| ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_HELP ';'
{
    parser->usage("service template");
}
| ICE_GRID_EXIT ';'
{
    return 0;
}
| ICE_GRID_APPLICATION ICE_GRID_ADD optional_strings ';'
{
    parser->addApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_ADD ICE_GRID_HELP ';'
{
    parser->usage("application", "add");
}
| ICE_GRID_APPLICATION ICE_GRID_REMOVE optional_strings ';'
{
    parser->removeApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_REMOVE ICE_GRID_HELP ';'
{
    parser->usage("application", "remove");
}
| ICE_GRID_APPLICATION ICE_GRID_DIFF optional_strings ';'
{
    parser->diffApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_DIFF ICE_GRID_HELP ';'
{
    parser->usage("application", "diff");
}
| ICE_GRID_APPLICATION ICE_GRID_UPDATE optional_strings ';'
{
    parser->updateApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_UPDATE ICE_GRID_HELP ';'
{
    parser->usage("application", "update");
}
| ICE_GRID_APPLICATION ICE_GRID_DESCRIBE optional_strings ';'
{
    parser->describeApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("application", "describe");
}
| ICE_GRID_APPLICATION ICE_GRID_PATCH optional_strings ';'
{
    parser->patchApplication($3);
}
| ICE_GRID_APPLICATION ICE_GRID_PATCH ICE_GRID_HELP ';'
{
    parser->usage("application", "patch");
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE optional_strings ';'
{
    parser->describeServerTemplate($4);
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("server template", "describe");
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_INSTANTIATE optional_strings ';'
{
    parser->instantiateServerTemplate($4);
}
| ICE_GRID_SERVER ICE_GRID_TEMPLATE ICE_GRID_INSTANTIATE ICE_GRID_HELP ';'
{
    parser->usage("server template", "instantiate");
}
| ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE optional_strings ';'
{
    parser->describeServiceTemplate($4);
}
| ICE_GRID_SERVICE ICE_GRID_TEMPLATE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("service template", "describe");
}
| ICE_GRID_APPLICATION ICE_GRID_LIST ';'
{
    parser->listAllApplications();
}
| ICE_GRID_NODE ICE_GRID_DESCRIBE optional_strings ';'
{
    parser->describeNode($3);
}
| ICE_GRID_NODE ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("node", "describe");
}
| ICE_GRID_NODE ICE_GRID_PING optional_strings ';'
{
    parser->pingNode($3);
}
| ICE_GRID_NODE ICE_GRID_PING ICE_GRID_HELP ';'
{
    parser->usage("node", "ping");
}
| ICE_GRID_NODE ICE_GRID_LOAD optional_strings ';'
{
    parser->printLoadNode($3);
}
| ICE_GRID_NODE ICE_GRID_LOAD ICE_GRID_HELP ';'
{
    parser->usage("node", "lost");
}
| ICE_GRID_NODE ICE_GRID_SHUTDOWN optional_strings ';'
{
    parser->shutdownNode($3);
}
| ICE_GRID_NODE ICE_GRID_SHUTDOWN ICE_GRID_HELP ';'
{
    parser->usage("node", "shutdown");
}
| ICE_GRID_NODE ICE_GRID_LIST ';'
{
    parser->listAllNodes();
}
| ICE_GRID_NODE ICE_GRID_SHOW strings_and_log_filename ';'
{
    parser->showFile("node", $3);
}
| ICE_GRID_NODE ICE_GRID_SHOW ICE_GRID_HELP ';'
{
    parser->usage("node", "show");
}
| ICE_GRID_REGISTRY ICE_GRID_DESCRIBE optional_strings ';'
{
    parser->describeRegistry($3);
}
| ICE_GRID_REGISTRY ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("registry", "describe");
}
| ICE_GRID_REGISTRY ICE_GRID_PING optional_strings ';'
{
    parser->pingRegistry($3);
}
| ICE_GRID_REGISTRY ICE_GRID_PING ICE_GRID_HELP ';'
{
    parser->usage("registry", "ping");
}
| ICE_GRID_REGISTRY ICE_GRID_SHUTDOWN optional_strings ';'
{
    parser->shutdownRegistry($3);
}
| ICE_GRID_REGISTRY ICE_GRID_SHUTDOWN ICE_GRID_HELP ';'
{
    parser->usage("registry", "shutdown");
}
| ICE_GRID_REGISTRY ICE_GRID_LIST ';'
{
    parser->listAllRegistries();
}
| ICE_GRID_REGISTRY ICE_GRID_SHOW strings_and_log_filename ';'
{
    parser->showFile("registry", $3);
}
| ICE_GRID_REGISTRY ICE_GRID_SHOW ICE_GRID_HELP ';'
{
    parser->usage("registry", "show");
}
| ICE_GRID_SERVER ICE_GRID_REMOVE optional_strings ';'
{
    parser->removeServer($3);
}
| ICE_GRID_SERVER ICE_GRID_REMOVE ICE_GRID_HELP ';'
{
    parser->usage("server", "remove");
}
| ICE_GRID_SERVER ICE_GRID_DESCRIBE optional_strings ';'
{
    parser->describeServer($3);
}
| ICE_GRID_SERVER ICE_GRID_DESCRIBE ICE_GRID_HELP ';'
{
    parser->usage("server", "describe");
}
| ICE_GRID_SERVER ICE_GRID_START optional_strings ';'
{
    parser->startServer($3);
}
| ICE_GRID_SERVER ICE_GRID_START ICE_GRID_HELP ';'
{
    parser->usage("server", "start");
}
| ICE_GRID_SERVER ICE_GRID_STOP optional_strings ';'
{
    parser->stopServer($3);
}
| ICE_GRID_SERVER ICE_GRID_STOP ICE_GRID_HELP ';'
{
    parser->usage("server", "stop");
}
| ICE_GRID_SERVER ICE_GRID_PATCH optional_strings ';'
{
    parser->patchServer($3);
}
| ICE_GRID_SERVER ICE_GRID_PATCH ICE_GRID_HELP ';'
{
    parser->usage("server", "patch");
}
| ICE_GRID_SERVER ICE_GRID_SIGNAL optional_strings ';'
{
    parser->signalServer($3);
}
| ICE_GRID_SERVER ICE_GRID_SIGNAL ICE_GRID_HELP ';'
{
    parser->usage("server", "signal");
}
| ICE_GRID_SERVER ICE_GRID_STDOUT optional_strings ';'
{
    parser->writeMessage($3, 1);
}
| ICE_GRID_SERVER ICE_GRID_STDOUT ICE_GRID_HELP ';'
{
    parser->usage("server", "stdout");
}
| ICE_GRID_SERVER ICE_GRID_STDERR optional_strings ';'
{
    parser->writeMessage($3, 2);
}
| ICE_GRID_SERVER ICE_GRID_STDERR ICE_GRID_HELP ';'
{
    parser->usage("server", "stderr");
}
| ICE_GRID_SERVER ICE_GRID_STATE optional_strings ';'
{
    parser->stateServer($3);
}
| ICE_GRID_SERVER ICE_GRID_STATE ICE_GRID_HELP ';'
{
    parser->usage("server", "start");
}
| ICE_GRID_SERVER ICE_GRID_PID optional_strings ';'
{
    parser->pidServer($3);
}
| ICE_GRID_SERVER ICE_GRID_PID ICE_GRID_HELP ';'
{
    parser->usage("server", "pid");
}
| ICE_GRID_SERVER ICE_GRID_ENABLE optional_strings ';'
{
    parser->enableServer($3, true);
}
| ICE_GRID_SERVER ICE_GRID_ENABLE ICE_GRID_HELP ';'
{
    parser->usage("server", "enable");
}
| ICE_GRID_SERVER ICE_GRID_DISABLE optional_strings ';'
{
    parser->enableServer($3, false);
}
| ICE_GRID_SERVER ICE_GRID_DISABLE ICE_GRID_HELP ';'
{
    parser->usage("server", "disable");
}
| ICE_GRID_SERVER ICE_GRID_LIST ';'
{
    parser->listAllServers();
}
| ICE_GRID_SERVER ICE_GRID_SHOW strings_and_log_filename ';'
{
    parser->showFile("server", $3);
}
| ICE_GRID_SERVER ICE_GRID_SHOW ICE_GRID_HELP ';'
{
    parser->usage("server", "show");
}
| ICE_GRID_ADAPTER ICE_GRID_ENDPOINTS optional_strings ';'
{
    parser->endpointsAdapter($3);
}
| ICE_GRID_ADAPTER ICE_GRID_ENDPOINTS ICE_GRID_HELP ';'
{
    parser->usage("adapter", "endpoints");
}
| ICE_GRID_ADAPTER ICE_GRID_REMOVE optional_strings ';'
{
    parser->removeAdapter($3);
}
| ICE_GRID_ADAPTER ICE_GRID_REMOVE ICE_GRID_HELP ';'
{
    parser->usage("adapter", "remove");
}
| ICE_GRID_ADAPTER ICE_GRID_LIST ';'
{
    parser->listAllAdapters();
}
| ICE_GRID_OBJECT ICE_GRID_ADD optional_strings ';'
{
    parser->addObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_ADD ICE_GRID_HELP ';'
{
    parser->usage("object", "add");
}
| ICE_GRID_OBJECT ICE_GRID_REMOVE optional_strings ';'
{
    parser->removeObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_REMOVE ICE_GRID_HELP ';'
{
    parser->usage("object", "remove");
}
| ICE_GRID_OBJECT ICE_GRID_FIND optional_strings ';'
{
    parser->findObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_FIND ICE_GRID_HELP ';'
{
    parser->usage("object", "find");
}
| ICE_GRID_OBJECT ICE_GRID_LIST optional_strings ';'
{
    parser->listObject($3);
}
| ICE_GRID_OBJECT ICE_GRID_DESCRIBE optional_strings ';'
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
| ICE_GRID_APPLICATION optional_strings ';'
{
    if($2.empty())
    {
	parser->error("invalid command `application' (use `help application'\n"
		      "to get the list of available commands)");
    }
    else
    {
	parser->error("invalid command: `application " + $2.front() + "' (use `help application'\n"
		      "to get the list of available commands)");
    }
}
| ICE_GRID_SERVER optional_strings ';'
{
    if($2.empty())
    {
	parser->error("invalid command `server' (use `help server'\n"
		      "to get the list of available commands)");
    }
    else
    {
	parser->error("invalid command: `server " + $2.front() + "' (use `help server'\n"
		      "to get the list of available commands)");
    }
}
| ICE_GRID_NODE optional_strings ';'
{
    if($2.empty())
    {
	parser->error("invalid command `node' (use `help node'\n"
		      "to get the list of available commands)");
    }
    else
    {
	parser->error("invalid command: `node " + $2.front() + "' (use `help node'\n"
		      "to get the list of available commands)");
    }
}
| ICE_GRID_REGISTRY optional_strings ';'
{
    if($2.empty())
    {
	parser->error("invalid command `registry' (use `help registry'\n"
		      "to get the list of available commands)");
    }
    else
    {
	parser->error("invalid command: `registry " + $2.front() + "' (use `help registry'\n"
		      "to get the list of available commands)");
    }
}
| ICE_GRID_SERVICE optional_strings ';'
{
    if($2.empty())
    {
	parser->invalidCommand("invalid command `service' ");
    }
    else
    {
	parser->invalidCommand("invalid command: `service " + $2.front() + "'");
    }
}
| ICE_GRID_OBJECT optional_strings ';'
{
    if($2.empty())
    {
	parser->error("invalid command `object' (use `help object'\n"
		      "to get the list of available commands)");
    }
    else
    {
	parser->error("invalid command: `object " + $2.front() + "' (use `help object'\n"
		      "to get the list of available commands)");
    }
}
| ICE_GRID_ADAPTER optional_strings ';'
{
    if($2.empty())
    {
	parser->error("invalid command `adapter' (use `help adapter'\n"
		      "to get the list of available commands)");
    }
    else
    {
	parser->error("invalid command: `adapter " + $2.front() + "' (use `help adapter'\n"
		      "to get the list of available commands)");
    }
}
| strings ';'
{
    parser->error("invalid command `" + $1.front() + "' (use `help'\n"
		  "to get the list of available commands)");
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
strings_and_log_filename
// ----------------------------------------------------------------------
: ICE_GRID_STRING strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| strings ICE_GRID_STDERR
{
    $$ = $1;
    $$.push_back("stderr");
}
| strings ICE_GRID_STDOUT
{
    $$ = $1;
    $$.push_back("stdout");
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
