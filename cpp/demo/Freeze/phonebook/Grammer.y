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
#include <Parser.h>

using namespace std;
using namespace Ice;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%pure_parser

%token TOK_HELP
%token TOK_EXIT
%token TOK_ADD_ENTRIES
%token TOK_FIND_ENTRIES
%token TOK_NEXT_FOUND_ENTRY
%token TOK_PRINT_CURRENT
%token TOK_REMOVE_CURRENT
%token TOK_LIST_NAMES
%token TOK_STRING

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
: TOK_HELP ';'
{
    parser->usage();
}
| TOK_EXIT ';'
{
    return 0;
}
| TOK_ADD_ENTRIES strings ';'
{
    parser->addEntries($2);
}
| TOK_FIND_ENTRIES strings ';'
{
    parser->findEntries($2);
}
| TOK_NEXT_FOUND_ENTRY ';'
{
    parser->nextFoundEntry();
}
| TOK_PRINT_CURRENT ';'
{
    parser->printCurrent();
}
| TOK_REMOVE_CURRENT ';'
{
    parser->removeCurrent();
}
| TOK_LIST_NAMES ';'
{
    parser->listNames();
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
: TOK_STRING strings
{
    $$ = $2;
    $$.push_front($1.front());
}
| TOK_STRING
{
    $$ = $1
}
;

%%
