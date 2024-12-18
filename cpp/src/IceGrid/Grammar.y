%code top{

// Copyright (c) ZeroC, Inc.

// NOLINTBEGIN

}

%code requires{

#include <list>
#include <string>

// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
#define YYMAXDEPTH  10000      // 10000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

// Newer bison versions allow to disable stack resizing by defining yyoverflow.
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

}

%code{

// Forward declaration of the lexing function generated by flex, so bison knows about it.
// This must match the definition of 'yylex' in the generated scanner.
int yylex(YYSTYPE* yylvalp);

}

%{

#include "Ice/Ice.h"
#include "Parser.h"

#ifdef _MSC_VER
// warning C4127: conditional expression is constant
#    pragma warning(disable:4127)
// warning C4102: 'yyexhausted': unreferenced label
#    pragma warning(disable:4102)
// warning C4702: unreachable code
#    pragma warning(disable:4702)
#endif

// Avoid old style cast warnings in generated grammar
#ifdef __GNUC__
#    pragma GCC diagnostic ignored "-Wold-style-cast"
#    pragma GCC diagnostic ignored "-Wunused-label"

// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98753
#    pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif

// Avoid clang warnings in generate grammar
#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wconversion"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#    pragma clang diagnostic ignored "-Wunused-but-set-variable"
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

// Directs Bison to generate a re-entrant parser.
%define api.pure
// Specifies what type to back the tokens with (their semantic values).
%define api.value.type {std::list<std::string>}

// All keyword tokens. Make sure to modify the "keyword" rule in this
// file if the list of keywords is changed. Also make sure to add the
// keyword to the keyword table in Scanner.l.
%token ICEGRID_HELP
%token ICEGRID_EXIT
%token ICEGRID_APPLICATION
%token ICEGRID_NODE
%token ICEGRID_REGISTRY
%token ICEGRID_SERVER
%token ICEGRID_ADAPTER
%token ICEGRID_PING
%token ICEGRID_LOAD
%token ICEGRID_SOCKETS
%token ICEGRID_ADD
%token ICEGRID_REMOVE
%token ICEGRID_LIST
%token ICEGRID_SHUTDOWN
%token ICEGRID_STRING
%token ICEGRID_START
%token ICEGRID_STOP
%token ICEGRID_SIGNAL
%token ICEGRID_STDOUT
%token ICEGRID_STDERR
%token ICEGRID_DESCRIBE
%token ICEGRID_PROPERTIES
%token ICEGRID_PROPERTY
%token ICEGRID_STATE
%token ICEGRID_PID
%token ICEGRID_ENDPOINTS
%token ICEGRID_ACTIVATION
%token ICEGRID_OBJECT
%token ICEGRID_FIND
%token ICEGRID_SHOW
%token ICEGRID_COPYING
%token ICEGRID_WARRANTY
%token ICEGRID_DIFF
%token ICEGRID_UPDATE
%token ICEGRID_INSTANTIATE
%token ICEGRID_TEMPLATE
%token ICEGRID_SERVICE
%token ICEGRID_ENABLE
%token ICEGRID_DISABLE

%%

// ----------------------------------------------------------------------
start
// ----------------------------------------------------------------------
: commands
{
}
| %empty
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

// ----------------------------------------------------------------------
checkInterrupted
// ----------------------------------------------------------------------
: %empty
{
    parser->checkInterrupted();
}
;

// ----------------------------------------------------------------------
command
// ----------------------------------------------------------------------
: ICEGRID_HELP ';'
{
    parser->usage();
}
| ICEGRID_EXIT ';'
{
    return 0;
}
| ICEGRID_APPLICATION ICEGRID_ADD strings ';'
{
    parser->addApplication($3);
}
| ICEGRID_APPLICATION ICEGRID_ADD ICEGRID_HELP ';'
{
    parser->usage("application", "add");
}
| ICEGRID_APPLICATION ICEGRID_REMOVE strings ';'
{
    parser->removeApplication($3);
}
| ICEGRID_APPLICATION ICEGRID_REMOVE ICEGRID_HELP ';'
{
    parser->usage("application", "remove");
}
| ICEGRID_APPLICATION ICEGRID_DIFF strings ';'
{
    parser->diffApplication($3);
}
| ICEGRID_APPLICATION ICEGRID_DIFF ICEGRID_HELP ';'
{
    parser->usage("application", "diff");
}
| ICEGRID_APPLICATION ICEGRID_UPDATE strings ';'
{
    parser->updateApplication($3);
}
| ICEGRID_APPLICATION ICEGRID_UPDATE ICEGRID_HELP ';'
{
    parser->usage("application", "update");
}
| ICEGRID_APPLICATION ICEGRID_DESCRIBE strings ';'
{
    parser->describeApplication($3);
}
| ICEGRID_APPLICATION ICEGRID_DESCRIBE ICEGRID_HELP ';'
{
    parser->usage("application", "describe");
}
| ICEGRID_APPLICATION ICEGRID_LIST strings ';'
{
    parser->listAllApplications($3);
}
| ICEGRID_APPLICATION ICEGRID_LIST ICEGRID_HELP ';'
{
    parser->usage("application", "list");
}
| ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_DESCRIBE strings ';'
{
    parser->describeServerTemplate($4);
}
| ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_DESCRIBE ICEGRID_HELP ';'
{
    parser->usage("server template", "describe");
}
| ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_INSTANTIATE strings ';'
{
    parser->instantiateServerTemplate($4);
}
| ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_INSTANTIATE ICEGRID_HELP ';'
{
    parser->usage("server template", "instantiate");
}
| ICEGRID_SERVER ICEGRID_TEMPLATE ICEGRID_HELP ';'
{
    parser->usage("server template");
}
| ICEGRID_SERVICE ICEGRID_TEMPLATE ICEGRID_DESCRIBE strings ';'
{
    parser->describeServiceTemplate($4);
}
| ICEGRID_SERVICE ICEGRID_TEMPLATE ICEGRID_DESCRIBE ICEGRID_HELP ';'
{
    parser->usage("service template", "describe");
}
| ICEGRID_SERVICE ICEGRID_TEMPLATE ICEGRID_HELP ';'
{
    parser->usage("service template");
}
| ICEGRID_NODE ICEGRID_DESCRIBE strings ';'
{
    parser->describeNode($3);
}
| ICEGRID_NODE ICEGRID_DESCRIBE ICEGRID_HELP ';'
{
    parser->usage("node", "describe");
}
| ICEGRID_NODE ICEGRID_PING strings ';'
{
    parser->pingNode($3);
}
| ICEGRID_NODE ICEGRID_PING ICEGRID_HELP ';'
{
    parser->usage("node", "ping");
}
| ICEGRID_NODE ICEGRID_LOAD strings ';'
{
    parser->printLoadNode($3);
}
| ICEGRID_NODE ICEGRID_LOAD ICEGRID_HELP ';'
{
    parser->usage("node", "load");
}
| ICEGRID_NODE ICEGRID_SOCKETS strings ';'
{
    parser->printNodeProcessorSockets($3);
}
| ICEGRID_NODE ICEGRID_SOCKETS ICEGRID_HELP ';'
{
    parser->usage("node", "sockets");
}
| ICEGRID_NODE ICEGRID_SHUTDOWN strings ';'
{
    parser->shutdownNode($3);
}
| ICEGRID_NODE ICEGRID_SHUTDOWN ICEGRID_HELP ';'
{
    parser->usage("node", "shutdown");
}
| ICEGRID_NODE ICEGRID_LIST strings ';'
{
    parser->listAllNodes($3);
}
| ICEGRID_NODE ICEGRID_LIST ICEGRID_HELP ';'
{
    parser->usage("node", "list");
}
| ICEGRID_NODE ICEGRID_SHOW strings ';'
{
    parser->show("node", $3);
}
| ICEGRID_NODE ICEGRID_SHOW ICEGRID_HELP ';'
{
    parser->usage("node", "show");
}
| ICEGRID_REGISTRY ICEGRID_DESCRIBE strings ';'
{
    parser->describeRegistry($3);
}
| ICEGRID_REGISTRY ICEGRID_DESCRIBE ICEGRID_HELP ';'
{
    parser->usage("registry", "describe");
}
| ICEGRID_REGISTRY ICEGRID_PING strings ';'
{
    parser->pingRegistry($3);
}
| ICEGRID_REGISTRY ICEGRID_PING ICEGRID_HELP ';'
{
    parser->usage("registry", "ping");
}
| ICEGRID_REGISTRY ICEGRID_SHUTDOWN strings ';'
{
    parser->shutdownRegistry($3);
}
| ICEGRID_REGISTRY ICEGRID_SHUTDOWN ICEGRID_HELP ';'
{
    parser->usage("registry", "shutdown");
}
| ICEGRID_REGISTRY ICEGRID_LIST strings ';'
{
    parser->listAllRegistries($3);
}
| ICEGRID_REGISTRY ICEGRID_LIST ICEGRID_HELP ';'
{
    parser->usage("registry", "list");
}
| ICEGRID_REGISTRY ICEGRID_SHOW strings ';'
{
    parser->show("registry", $3);
}
| ICEGRID_REGISTRY ICEGRID_SHOW ICEGRID_HELP ';'
{
    parser->usage("registry", "show");
}
| ICEGRID_SERVER ICEGRID_REMOVE strings ';'
{
    parser->removeServer($3);
}
| ICEGRID_SERVER ICEGRID_REMOVE ICEGRID_HELP ';'
{
    parser->usage("server", "remove");
}
| ICEGRID_SERVER ICEGRID_DESCRIBE strings ';'
{
    parser->describeServer($3);
}
| ICEGRID_SERVER ICEGRID_DESCRIBE ICEGRID_HELP ';'
{
    parser->usage("server", "describe");
}
| ICEGRID_SERVER ICEGRID_START strings ';'
{
    parser->startServer($3);
}
| ICEGRID_SERVER ICEGRID_START ICEGRID_HELP ';'
{
    parser->usage("server", "start");
}
| ICEGRID_SERVER ICEGRID_STOP strings ';'
{
    parser->stopServer($3);
}
| ICEGRID_SERVER ICEGRID_STOP ICEGRID_HELP ';'
{
    parser->usage("server", "stop");
}
| ICEGRID_SERVER ICEGRID_SIGNAL strings ';'
{
    parser->signalServer($3);
}
| ICEGRID_SERVER ICEGRID_SIGNAL ICEGRID_HELP ';'
{
    parser->usage("server", "signal");
}
| ICEGRID_SERVER ICEGRID_STDOUT strings ';'
{
    parser->writeMessage($3, 1);
}
| ICEGRID_SERVER ICEGRID_STDOUT ICEGRID_HELP ';'
{
    parser->usage("server", "stdout");
}
| ICEGRID_SERVER ICEGRID_STDERR strings ';'
{
    parser->writeMessage($3, 2);
}
| ICEGRID_SERVER ICEGRID_STDERR ICEGRID_HELP ';'
{
    parser->usage("server", "stderr");
}
| ICEGRID_SERVER ICEGRID_STATE strings ';'
{
    parser->stateServer($3);
}
| ICEGRID_SERVER ICEGRID_STATE ICEGRID_HELP ';'
{
    parser->usage("server", "start");
}
| ICEGRID_SERVER ICEGRID_PID strings ';'
{
    parser->pidServer($3);
}
| ICEGRID_SERVER ICEGRID_PID ICEGRID_HELP ';'
{
    parser->usage("server", "pid");
}
| ICEGRID_SERVER ICEGRID_PROPERTIES strings ';'
{
    parser->propertiesServer($3, false);
}
| ICEGRID_SERVER ICEGRID_PROPERTIES ICEGRID_HELP ';'
{
    parser->usage("server", "properties");
}
| ICEGRID_SERVER ICEGRID_PROPERTY strings ';'
{
    parser->propertiesServer($3, true);
}
| ICEGRID_SERVER ICEGRID_PROPERTY ICEGRID_HELP ';'
{
    parser->usage("server", "property");
}
| ICEGRID_SERVER ICEGRID_ENABLE strings ';'
{
    parser->enableServer($3, true);
}
| ICEGRID_SERVER ICEGRID_ENABLE ICEGRID_HELP ';'
{
    parser->usage("server", "enable");
}
| ICEGRID_SERVER ICEGRID_DISABLE strings ';'
{
    parser->enableServer($3, false);
}
| ICEGRID_SERVER ICEGRID_DISABLE ICEGRID_HELP ';'
{
    parser->usage("server", "disable");
}
| ICEGRID_SERVER ICEGRID_LIST strings ';'
{
    parser->listAllServers($3);
}
| ICEGRID_SERVER ICEGRID_LIST ICEGRID_HELP ';'
{
    parser->usage("server", "list");
}
| ICEGRID_SERVER ICEGRID_SHOW strings ';'
{
    parser->show("server", $3);
}
| ICEGRID_SERVER ICEGRID_SHOW ICEGRID_HELP ';'
{
    parser->usage("server", "show");
}
| ICEGRID_SERVICE ICEGRID_START strings ';'
{
    parser->startService($3);
}
| ICEGRID_SERVICE ICEGRID_START ICEGRID_HELP ';'
{
    parser->usage("service", "start");
}
| ICEGRID_SERVICE ICEGRID_STOP strings ';'
{
    parser->stopService($3);
}
| ICEGRID_SERVICE ICEGRID_STOP ICEGRID_HELP ';'
{
    parser->usage("service", "stop");
}
| ICEGRID_SERVICE ICEGRID_DESCRIBE strings ';'
{
    parser->describeService($3);
}
| ICEGRID_SERVICE ICEGRID_DESCRIBE ICEGRID_HELP ';'
{
    parser->usage("service", "describe");
}
| ICEGRID_SERVICE ICEGRID_PROPERTIES strings ';'
{
    parser->propertiesService($3, false);
}
| ICEGRID_SERVICE ICEGRID_PROPERTIES ICEGRID_HELP ';'
{
    parser->usage("service", "properties");
}
| ICEGRID_SERVICE ICEGRID_PROPERTY strings ';'
{
    parser->propertiesService($3, true);
}
| ICEGRID_SERVICE ICEGRID_PROPERTY ICEGRID_HELP ';'
{
    parser->usage("service", "property");
}
| ICEGRID_SERVICE ICEGRID_LIST strings ';'
{
    parser->listServices($3);
}
| ICEGRID_SERVICE ICEGRID_LIST ICEGRID_HELP ';'
{
    parser->usage("service", "list");
}
| ICEGRID_ADAPTER ICEGRID_ENDPOINTS strings ';'
{
    parser->endpointsAdapter($3);
}
| ICEGRID_ADAPTER ICEGRID_ENDPOINTS ICEGRID_HELP ';'
{
    parser->usage("adapter", "endpoints");
}
| ICEGRID_ADAPTER ICEGRID_REMOVE strings ';'
{
    parser->removeAdapter($3);
}
| ICEGRID_ADAPTER ICEGRID_REMOVE ICEGRID_HELP ';'
{
    parser->usage("adapter", "remove");
}
| ICEGRID_ADAPTER ICEGRID_LIST strings ';'
{
    parser->listAllAdapters($3);
}
| ICEGRID_ADAPTER ICEGRID_LIST ICEGRID_HELP ';'
{
    parser->usage("adapter", "list");
}
| ICEGRID_OBJECT ICEGRID_ADD strings ';'
{
    parser->addObject($3);
}
| ICEGRID_OBJECT ICEGRID_ADD ICEGRID_HELP ';'
{
    parser->usage("object", "add");
}
| ICEGRID_OBJECT ICEGRID_REMOVE strings ';'
{
    parser->removeObject($3);
}
| ICEGRID_OBJECT ICEGRID_REMOVE ICEGRID_HELP ';'
{
    parser->usage("object", "remove");
}
| ICEGRID_OBJECT ICEGRID_FIND strings ';'
{
    parser->findObject($3);
}
| ICEGRID_OBJECT ICEGRID_FIND ICEGRID_HELP ';'
{
    parser->usage("object", "find");
}
| ICEGRID_OBJECT ICEGRID_LIST strings ';'
{
    parser->listObject($3);
}
| ICEGRID_OBJECT ICEGRID_LIST ICEGRID_HELP ';'
{
    parser->usage("object", "list");
}
| ICEGRID_OBJECT ICEGRID_DESCRIBE strings ';'
{
    parser->describeObject($3);
}
| ICEGRID_OBJECT ICEGRID_DESCRIBE ICEGRID_HELP ';'
{
    parser->usage("object", "describe");
}
| ICEGRID_SHOW ICEGRID_COPYING ';'
{
    parser->showCopying();
}
| ICEGRID_SHOW ICEGRID_WARRANTY ';'
{
    parser->showWarranty();
}
| ICEGRID_HELP keyword ';'
{
    parser->usage($2.front());
}
| ICEGRID_HELP keyword keyword ';'
{
    if (($2.front() == "server" || $2.front() == "service") && $3.front() == "template")
    {
        parser->usage($2.front() + " " + $3.front());
    }
    else
    {
        parser->usage($2.front(), $3.front());
    }
}
| ICEGRID_HELP keyword ICEGRID_STRING strings ';'
{
    parser->usage($2.front(), $3.front());
}
| ICEGRID_HELP keyword keyword keyword ';'
{
    if (($2.front() == "server" || $2.front() == "service") && $3.front() == "template")
    {
        parser->usage($2.front() + " " + $3.front(), $4.front());
    }
    else
    {
        parser->usage($2.front(), $3.front());
    }
}
| ICEGRID_HELP keyword keyword ICEGRID_STRING strings ';'
{
    if (($2.front() == "server" || $2.front() == "service") && $3.front() == "template")
    {
        parser->usage($2.front() + " " + $3.front(), $4.front());
    }
    else
    {
        parser->usage($2.front(), $3.front());
    }
}
| ICEGRID_HELP ICEGRID_STRING strings ';'
{
    parser->usage($2.front());
}
| ICEGRID_HELP error ';'
{
    parser->usage();
}
| keyword ICEGRID_HELP ';'
{
    parser->usage($1.front());
}
| keyword ICEGRID_STRING error ';'
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
| ICEGRID_STRING error ';'
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
: ICEGRID_STRING strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| keyword strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| ICEGRID_STRING ICEGRID_HELP strings
{
    $$ = $2;
    $$.push_front("help");
    $$.push_front($1.front());
}
| keyword ICEGRID_HELP strings
{
    $$ = $2;
    $$.push_front("help");
    $$.push_front($1.front());
}
| %empty
{
    $$ = YYSTYPE();
}
;

// ----------------------------------------------------------------------
keyword
// ----------------------------------------------------------------------
: ICEGRID_EXIT
{
}
| ICEGRID_APPLICATION
{
}
| ICEGRID_NODE
{
}
| ICEGRID_REGISTRY
{
}
| ICEGRID_SERVER
{
}
| ICEGRID_ADAPTER
{
}
| ICEGRID_PING
{
}
| ICEGRID_LOAD
{
}
| ICEGRID_SOCKETS
{
}
| ICEGRID_ADD
{
}
| ICEGRID_REMOVE
{
}
| ICEGRID_LIST
{
}
| ICEGRID_SHUTDOWN
{
}
| ICEGRID_START
{
}
| ICEGRID_STOP
{
}
| ICEGRID_SIGNAL
{
}
| ICEGRID_DESCRIBE
{
}
| ICEGRID_STATE
{
}
| ICEGRID_PID
{
}
| ICEGRID_PROPERTIES
{
}
| ICEGRID_PROPERTY
{
}
| ICEGRID_ENDPOINTS
{
}
| ICEGRID_ACTIVATION
{
}
| ICEGRID_OBJECT
{
}
| ICEGRID_FIND
{
}
| ICEGRID_SHOW
{
}
| ICEGRID_COPYING
{
}
| ICEGRID_WARRANTY
{
}
| ICEGRID_DIFF
{
}
| ICEGRID_UPDATE
{
}
| ICEGRID_INSTANTIATE
{
}
| ICEGRID_TEMPLATE
{
}
| ICEGRID_SERVICE
{
}
| ICEGRID_ENABLE
{
}
| ICEGRID_DISABLE
{
}
| ICEGRID_STDERR
{
}
| ICEGRID_STDOUT
{
}
;

%%

// NOLINTEND
