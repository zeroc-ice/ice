%{

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Parser.h>
#include <Grammar.h>

#if defined(_MSC_VER) && defined(ICE_64)
//
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
// Puts a pointer-difference into an int
//
#   pragma warning( 4 : 4244 )
#endif

using namespace std;

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

%}

WS	[ \t\v\f\r]
NL	[\n]

%option noyywrap
%option never-interactive

%%

"//" {
    // C++-style comment
    int c;
    do
    {
	c = yyinput();
	if(c == '\n')
	{
	    parser->nextLine();
	}
    }
    while(c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    while(true)
    {
	int c = yyinput();
	if(c == '\n')
	{
	    parser->nextLine();
	}
	else if(c == '*')
	{
	    int next = yyinput();
	    if(next == '/')
	    {
		break;
	    }
	    else
	    {
		unput(next);
	    }
	}
	else if(c == EOF)
	{
	    parser->warning("EOF in comment");
	    break;
	}
    }
}

"help" {
    return TOK_HELP;
}

"quit"|"exit" {
    return TOK_EXIT;
}

"add" {
    return TOK_ADD_CONTACTS;
}

"find" {
    return TOK_FIND_CONTACTS;
}

"next" {
    return TOK_NEXT_FOUND_CONTACT;
}

"current" {
    return TOK_PRINT_CURRENT;
}

"name" {
    return TOK_SET_CURRENT_NAME;
}

"address" {
    return TOK_SET_CURRENT_ADDRESS;
}

"phone" {
    return TOK_SET_CURRENT_PHONE;
}

"remove" {
    return TOK_REMOVE_CURRENT;
}

"size" {
    return TOK_SET_EVICTOR_SIZE;
}

"shutdown" {
    return TOK_SHUTDOWN;
}

{WS}*(\\{WS}*{NL})? {
    size_t len = strlen(yytext);
    for(size_t i = 0; i < len; ++i)
    {
	if(yytext[i] == '\\')
	{
	    parser->continueLine();
	}
	else if(yytext[i] == '\n')
	{
	    parser->nextLine();
	}
    }
}

{NL}|; {
    size_t len = strlen(yytext);
    for(size_t i = 0; i < len; ++i)
    {
	if(yytext[i] == '\n')
	{
	    parser->nextLine();
	}
    }
    return ';';
}

\" {
    // "..."-type strings
    string s;
    while(true)
    {
	char c = static_cast<char>(yyinput());
	if(c == '"')
	{
	    break;
	}
	else if(c == EOF)
	{
	    parser->warning("EOF in string");
	    break;
	}
	else if(c == '\n')
	{
	    s += c;
	    parser->nextLine();
	}
	else if(c == '\\')
	{
	    char next = static_cast<char>(yyinput());
	    switch(next)
	    {
		case '\\':
		case '"':
		{
		    s += next;
		    break;
		}
	    
		case 'n':
		{
		    s += '\n';
		    break;
		}
	    
		case 'r':
		{
		    s += '\r';
		    break;
		}

		case 't':
		{
		    s += '\t';
		    break;
		}
	    
		case 'v':
		{
		    s += '\v';
		    break;
		}
	    
		case 'f':
		{
		    s += '\f';
		    break;
		}
	    
		default:
		{
		    s += c;
		    unput(next);
		}
	    }
	}
	else
	{
	    s += c;
	}
    }
    yylvalp->clear();
    yylvalp->push_back(s);
    return TOK_STRING;
}

\' {
    // '...'-type strings
    string s;
    while(true)
    {
	char c = static_cast<char>(yyinput());
	if(c == '\'')
	{
	    break;
	}
	else if(c == EOF)
	{
	    parser->warning("EOF in string");
	    break;
	}
	else if(c == '\n')
	{
	    s += c;
	    parser->nextLine();
	}
	else
	{
	    s += c;
	}
    }
    yylvalp->clear();
    yylvalp->push_back(s);
    return TOK_STRING;
}

. {
    // Simple strings
    string s;
    s += yytext[0];
    while(true)
    {
	char c = static_cast<char>(yyinput());
	if(c == EOF)
	{
	    break;
	}
	else if(isspace(c) || c == ';')
	{
	    unput(c);
	    break;
	}
	
	s += c;
    }
    yylvalp->clear();
    yylvalp->push_back(s);
    return TOK_STRING;
}

%%
