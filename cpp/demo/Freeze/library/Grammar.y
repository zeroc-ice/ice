%{

// **********************************************************************
//
// Copyright (c) 2002
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
#include <Parser.h>

#ifdef _WIN32
// I get this warning from some bison version:
// warning C4102: 'yyoverflowlab' : unreferenced label
#   pragma warning( disable : 4102 )
#endif

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
%token TOK_ADD_BOOK
%token TOK_FIND_ISBN
%token TOK_FIND_AUTHORS
%token TOK_NEXT_FOUND_BOOK
%token TOK_PRINT_CURRENT
%token TOK_RENT_BOOK
%token TOK_RETURN_BOOK
%token TOK_REMOVE_CURRENT
%token TOK_SET_EVICTOR_SIZE
%token TOK_SHUTDOWN
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
| TOK_ADD_BOOK strings ';'
{
    parser->addBook($2);
}
| TOK_FIND_ISBN strings ';'
{
    parser->findIsbn($2);
}
| TOK_FIND_AUTHORS strings ';'
{
    parser->findAuthors($2);
}
| TOK_NEXT_FOUND_BOOK ';'
{
    parser->nextFoundBook();
}
| TOK_PRINT_CURRENT ';'
{
    parser->printCurrent();
}
| TOK_RENT_BOOK strings ';'
{
    parser->rentCurrent($2);
}
| TOK_RETURN_BOOK ';'
{
    parser->returnCurrent();
}
| TOK_REMOVE_CURRENT ';'
{
    parser->removeCurrent();
}
| TOK_SET_EVICTOR_SIZE strings ';'
{
    parser->setEvictorSize($2);
}
| TOK_SHUTDOWN ';'
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
