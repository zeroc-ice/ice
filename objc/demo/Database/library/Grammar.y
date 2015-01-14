%{

// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Parser.h>

void
yyerror(const char* s)
{
    [parser error: s];
}

%}

%pure_parser

%token TOK_HELP
%token TOK_EXIT
%token TOK_ADD_BOOK
%token TOK_FIND_ISBN
%token TOK_FIND_AUTHORS
%token TOK_FIND_TITLE
%token TOK_NEXT_FOUND_BOOK
%token TOK_PRINT_CURRENT
%token TOK_RENT_BOOK
%token TOK_RETURN_BOOK
%token TOK_REMOVE_CURRENT
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
    [parser usage];
}
| TOK_EXIT ';'
{
    return 0;
}
| TOK_ADD_BOOK strings ';'
{
    [parser addBook: $2];
}
| TOK_FIND_ISBN strings ';'
{
    [parser findIsbn: $2];
}
| TOK_FIND_AUTHORS strings ';'
{
    [parser findAuthors: $2];
}
| TOK_FIND_TITLE strings ';'
{
    [parser findTitle: $2];
}
| TOK_NEXT_FOUND_BOOK ';'
{
    [parser nextFoundBook];
}
| TOK_PRINT_CURRENT ';'
{
    [parser printCurrent];
}
| TOK_RENT_BOOK strings ';'
{
    [parser rentCurrent: $2];
}
| TOK_RETURN_BOOK ';'
{
    [parser returnCurrent];
}
| TOK_REMOVE_CURRENT ';'
{
    [parser removeCurrent];
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
    [$$ addObject: [$1 objectAtIndex:0]];
}
| TOK_STRING
{
    $$ = $1;
}
;

%%
