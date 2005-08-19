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
#include <IceGrid/Grammar.h>

#ifdef _WIN32
// I get these warnings from some flex versions:
// warning C4003: not enough actual parameters for macro 'yywrap'
#   pragma warning( disable : 4003 )
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

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
    return ICE_GRID_HELP;
}

"quit"|"exit" {
    return ICE_GRID_EXIT;
}

"application" {
    return ICE_GRID_APPLICATION;
}

"server" {
    return ICE_GRID_SERVER;
}

"adapter" {
    return ICE_GRID_ADAPTER;
}

"add" {
    return ICE_GRID_ADD;
}

"remove" {
    return ICE_GRID_REMOVE;
}

"list" {
    return ICE_GRID_LIST;
}

"shutdown" {
    return ICE_GRID_SHUTDOWN;
}

"describe" {
    return ICE_GRID_DESCRIBE;
}

"state" {
    return ICE_GRID_STATE;
}

"pid" {
    return ICE_GRID_PID;
}

"endpoints" {
    return ICE_GRID_ENDPOINTS;
}

"start" {
    return ICE_GRID_START;
}

"patch" {
    return ICE_GRID_PATCH;
}

"stop" {
    return ICE_GRID_STOP;
}

"signal" {
    return ICE_GRID_SIGNAL;
}

"stdout" {
    return ICE_GRID_STDOUT;
}

"stderr" {
    return ICE_GRID_STDERR;
}

"node" {
    return ICE_GRID_NODE;
}

"ping" {
    return ICE_GRID_PING;
}

"activation" {
    return ICE_GRID_ACTIVATION;
}

"object" {
    return ICE_GRID_OBJECT;
}

"find" {
    return ICE_GRID_FIND;
}

"show" {
    return ICE_GRID_SHOW;
}

"copying" {
    return ICE_GRID_COPYING;
}

"warranty" {
    return ICE_GRID_WARRANTY;
}

"diff" {
    return ICE_GRID_DIFF;
}

"update" {
    return ICE_GRID_UPDATE;
}

"instantiate" {
    return ICE_GRID_INSTANTIATE;
}

"template" {
    return ICE_GRID_TEMPLATE;
}

"service" {
    return ICE_GRID_SERVICE;
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
    return ICE_GRID_STRING;
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
    return ICE_GRID_STRING;
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
    return ICE_GRID_STRING;
}

%%
