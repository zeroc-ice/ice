%{

// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Parser.h>

#ifdef _MSC_VER
// I get these warnings from some bison versions:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
// warning C4065: switch statement contains 'default' but no 'case' labels
#   pragma warning( disable : 4065 )
#endif

using namespace std;

void
yyerror(const char* s)
{
    parser->error(s);
}

%}

%pure_parser

%token TOK_HELP
%token TOK_EXIT
%token TOK_STRING
%token TOK_LIST
%token TOK_LIST_RECURSIVE
%token TOK_CREATE_FILE
%token TOK_CREATE_DIR
%token TOK_PWD
%token TOK_CD
%token TOK_CAT
%token TOK_WRITE
%token TOK_RM

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
| TOK_LIST
{
    parser->list(false);
}
| TOK_LIST_RECURSIVE
{
    parser->list(true);
}
| TOK_CREATE_FILE strings
{
    parser->createFile($2);
}
| TOK_CREATE_DIR strings
{
    parser->createDir($2);
}
| TOK_PWD
{
    parser->pwd();
}
| TOK_CD
{
    parser->cd("/");
}
| TOK_CD strings
{
    parser->cd($2.front());
}
| TOK_CAT TOK_STRING
{
    parser->cat($2.front());
}
| TOK_WRITE strings
{
    parser->write($2);
}
| TOK_RM strings
{
    parser->destroy($2);
}
| error ';'
{
    parser->usage();
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
    $$ = $1;
}
;

%%
