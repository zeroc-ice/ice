%{

// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Parser.h>
#include <IceGrid/Grammar.h>

#if defined(_MSC_VER) && defined(ICE_64)
//
// '=' : conversion from 'size_t' to 'int', possible loss of data
// The result of fread() is a size_t and gets inserted into an int
//
#   pragma warning( 4 : 4267 )
//
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
// Puts a pointer-difference into an int
//
#   pragma warning( 4 : 4244 )
#endif

#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wsign-compare"
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

#ifdef _MSC_VER
#   ifdef yywrap
#      undef yywrap
#      define yywrap() 1
#   endif
#   define YY_NO_UNISTD_H
#endif

#ifdef __SUNPRO_CC
#   ifdef yywrap
#      undef yywrap
#      define yywrap() 1
#   endif
#   ifdef ICE_64
#       pragma error_messages(off,truncwarn)
#   endif
#endif

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

namespace IceGrid
{

typedef std::map<std::string, int> StringTokenMap;
static StringTokenMap keywordMap;

void initScanner();
std::string parseDoubleQuotedString();
std::string parseSingleQuotedString();

}
#define	YY_USER_INIT initScanner();

%}

WS	[ \t\v\f\r]
NL	[\n]
keyword [[:alpha:]]*

%option noyywrap
%option always-interactive

%%

"//" {
    // C++-style comment
    int c;
    do
    {
	c = yyinput();
    }
    while(c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    while(true)
    {
	int c = yyinput();
	if(c == '*')
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

{WS}*(\\{WS}*{NL})? {
    size_t len = strlen(yytext);
    for(size_t i = 0; i < len; ++i)
    {
	if(yytext[i] == '\\')
	{
	    parser->continueLine();
	}
    }
}

{NL}|; {
    return ';';
}

\" {
    // "..."-type strings
    string s = parseDoubleQuotedString();
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
        else if(c == '"')
        {
            s += parseDoubleQuotedString();
            continue;
        }
        else if(c == '\'')
        {
            s += parseSingleQuotedString();
            continue;
        }
        else if(isspace(static_cast<unsigned char>(c)) || c == ';')
        {
            unput(c);
            break;
        }
        s += c;
    }

    yylvalp->clear();
    yylvalp->push_back(s);

    StringTokenMap::const_iterator pos = keywordMap.find(s);
    return pos != keywordMap.end() ? pos->second : ICE_GRID_STRING;
}

%%

namespace IceGrid {

//
// initScanner() fills the keyword map with all keyword-token pairs.
//

void
initScanner()
{
    keywordMap["help"] = ICE_GRID_HELP;
    keywordMap["quit"] = ICE_GRID_EXIT;
    keywordMap["exit"] = ICE_GRID_EXIT;
    keywordMap["application"] = ICE_GRID_APPLICATION;
    keywordMap["server"] = ICE_GRID_SERVER;
    keywordMap["adapter"] = ICE_GRID_ADAPTER;
    keywordMap["add"] = ICE_GRID_ADD;
    keywordMap["remove"] = ICE_GRID_REMOVE;
    keywordMap["list"] = ICE_GRID_LIST;
    keywordMap["shutdown"] = ICE_GRID_SHUTDOWN;
    keywordMap["describe"] = ICE_GRID_DESCRIBE;
    keywordMap["properties"] = ICE_GRID_PROPERTIES;
    keywordMap["property"] = ICE_GRID_PROPERTY;
    keywordMap["state"] = ICE_GRID_STATE;
    keywordMap["pid"] = ICE_GRID_PID;
    keywordMap["endpoints"] = ICE_GRID_ENDPOINTS;
    keywordMap["start"] = ICE_GRID_START;
    keywordMap["patch"] = ICE_GRID_PATCH;
    keywordMap["stop"] = ICE_GRID_STOP;
    keywordMap["signal"] = ICE_GRID_SIGNAL;
    keywordMap["stdout"] = ICE_GRID_STDOUT;
    keywordMap["stderr"] = ICE_GRID_STDERR;
    keywordMap["node"] = ICE_GRID_NODE;
    keywordMap["registry"] = ICE_GRID_REGISTRY;
    keywordMap["ping"] = ICE_GRID_PING;
    keywordMap["load"] = ICE_GRID_LOAD;
    keywordMap["processors"] = ICE_GRID_SOCKETS;
    keywordMap["sockets"] = ICE_GRID_SOCKETS;
    keywordMap["activation"] = ICE_GRID_ACTIVATION;
    keywordMap["object"] = ICE_GRID_OBJECT;
    keywordMap["find"] = ICE_GRID_FIND;
    keywordMap["show"] = ICE_GRID_SHOW;
    keywordMap["copying"] = ICE_GRID_COPYING;
    keywordMap["warranty"] = ICE_GRID_WARRANTY;
    keywordMap["diff"] = ICE_GRID_DIFF;
    keywordMap["update"] = ICE_GRID_UPDATE;
    keywordMap["instantiate"] = ICE_GRID_INSTANTIATE;
    keywordMap["template"] = ICE_GRID_TEMPLATE;
    keywordMap["service"] = ICE_GRID_SERVICE;
    keywordMap["enable"] = ICE_GRID_ENABLE;
    keywordMap["disable"] = ICE_GRID_DISABLE;
}

std::string 
parseDoubleQuotedString()
{
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
    return s;
}

std::string
parseSingleQuotedString()
{
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
	else
	{
	    s += c;
	}
    }
    return s;
}

}
