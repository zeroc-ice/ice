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
#include <IcePack/Parser.h>
#include <IcePack/Grammer.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

%}

WS	[ \t\v\f]
NL	[\n\r]
S	[ \t]
D	[0-9]
L	[a-zA-Z_]

%option noyywrap

%%

^"#"{S}*{D}+{S}*$ {
    parser->scanPosition(yytext);
}

^"#"{S}*{D}+{S}+"\""[^\"]*"\"".*$ {
    parser->scanPosition(yytext);
}

^"#"{S}*"line"{S}+{D}+{S}*$ {
    parser->scanPosition(yytext);
}

^"#"{S}*"line"{S}+{D}+{S}+"\""[^\"]*"\"".*$ {
    parser->scanPosition(yytext);
}

"//" {
    // C++-style comment
    int c;
    do
    {
	c = yyinput();
	if (c == '\n')
	{
	    parser->nextLine();
	}
    }
    while (c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    while (true)
    {
	int c = yyinput();
	if (c == '\n')
	{
	    parser->nextLine();
	}
	else if (c == '*')
	{
	    int next = yyinput();
	    if (next == '/')
	    {
		break;
	    }
	    else
	    {
		unput(next);
	    }
	}
	else if (c == EOF)
	{
	    parser->warning("EOF in comment");
	    break;
	}
    }
}

\" {
    // "..."-type string
    string s;
    while (true)
    {
	char c = static_cast<char>(yyinput());
	if (c == '"')
	{
	    break;
	}
	else if (c == EOF)
	{
	    parser->warning("EOF in string");
	    break;
	}
	else if (c == '\n')
	{
	    s += c;
	    parser->nextLine();
	}
	else if (c == '\\')
	{
	    char next = static_cast<char>(yyinput());
	    switch (next)
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
    return ICE_PACK_STRING;
}

\' {
    // '...'-type string
    string s;
    while (true)
    {
	char c = static_cast<char>(yyinput());
	if (c == '\'')
	{
	    break;
	}
	else if (c == EOF)
	{
	    parser->warning("EOF in string");
	    break;
	}
	else if (c == '\n')
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
    return ICE_PACK_STRING;
}

"help" {
    return ICE_PACK_HELP;
}

"quit"|"exit" {
    return ICE_PACK_EXIT;
}

"add" {
    return ICE_PACK_ADD;
}

"remove" {
    return ICE_PACK_REMOVE;
}

"list" {
    return ICE_PACK_LIST;
}

"shutdown" {
    return ICE_PACK_SHUTDOWN;
}

{WS}*(\\{WS}*{NL})? {
    int len = strlen(yytext);
    for (int i = 0; i < len; ++i)
    {
	if (yytext[i] == '\\')
	{
	    parser->continueLine();
	}
	else if (yytext[i] == '\n')
	{
	    parser->nextLine();
	}
    }
}

{NL}|; {
    int len = strlen(yytext);
    for (int i = 0; i < len; ++i)
    {
	if (yytext[i] == '\n')
	{
	    parser->nextLine();
	}
    }
    return ';';
}

. {
    return yytext[0];
}

%%
