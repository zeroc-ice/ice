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
#include <IceStorm/Parser.h>
#include <IceStorm/Grammar.h>

#if defined(_MSC_VER) && defined(ICE_64)
//
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
// Puts a pointer-difference into an int
//
#   pragma warning( 4 : 4244 )
#endif

using namespace std;
using namespace Ice;
using namespace IceStorm;

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

%}

WS	[ \t\v\f\r]
NL	[\n]

%option noyywrap

%%

^"#"[[:blank:]]*[[:digit:]]+[[:blank:]]*$ {
    parser->scanPosition(yytext);
}

^"#"[[:blank:]]*[[:digit:]]+[[:blank:]]+"\""[^\"]*"\"".*$ {
    parser->scanPosition(yytext);
}

^"#"[[:blank:]]*"line"[[:blank:]]+[[:digit:]]+[[:blank:]]*$ {
    parser->scanPosition(yytext);
}

^"#"[[:blank:]]*"line"[[:blank:]]+[[:digit:]]+[[:blank:]]+"\""[^\"]*"\"".*$ {
    parser->scanPosition(yytext);
}

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
    return ICE_STORM_HELP;
}

"quit"|"exit" {
    return ICE_STORM_EXIT;
}

"create" {
    return ICE_STORM_CREATE;
}

"destroy" {
    return ICE_STORM_DESTROY;
}

"list" {
    return ICE_STORM_LIST;
}

"link" {
    return ICE_STORM_LINK;
}

"unlink" {
    return ICE_STORM_UNLINK;
}

"graph" {
    return ICE_STORM_GRAPH;
}

"show" {
    return ICE_STORM_SHOW;
}

"copying" {
    return ICE_STORM_COPYING;
}

"warranty" {
    return ICE_STORM_WARRANTY;
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
    return ICE_STORM_STRING;
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
    return ICE_STORM_STRING;
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
    return ICE_STORM_STRING;
}

%%
