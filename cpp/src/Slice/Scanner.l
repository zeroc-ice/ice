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

#include <Slice/GrammarUtil.h> // Before Grammer.h, so that YYSTYPE is defined
#include <Slice/Grammar.h>

using namespace std;
using namespace Slice;

namespace Slice
{

//
// Definitions for the case-insensitive keyword-token map.
//
typedef std::map<std::string, int, Slice::CICompare> StringTokenMap;
static StringTokenMap keywordMap;

void Slice::initScanner();

}

#define	YY_USER_INIT initScanner();

%}

%option noyywrap
%option never-interactive

%%

^"#"[[:blank:]]*[[:digit:]]+[[:blank:]]*$ {
    unit->scanPosition(yytext);
}

^"#"[[:blank:]]*[[:digit:]]+[[:blank:]]+"\""[^\"]*"\"".*$ {
    unit->scanPosition(yytext);
}

^"#"[[:blank:]]*"line"[[:blank:]]+[[:digit:]]+[[:blank:]]*$ {
    unit->scanPosition(yytext);
}

^"#"[[:blank:]]*"line"[[:blank:]]+[[:digit:]]+[[:blank:]]+"\""[^\"]*"\"".*$ {
    unit->scanPosition(yytext);
}

"//" {
    // C++-style comment
    int c;
    do
    {
	c = yyinput();
	if(c == '\n')
	{
	    unit->nextLine();
	}
    }
    while(c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    string comment = yytext + 2;
    while(true)
    {
	int c = yyinput();
	if(c == '\n')
	{
	    comment += static_cast<char>(c);
	    unit->nextLine();
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
		comment += static_cast<char>(c);
		unput(next);
	    }
	}
	else if(c == EOF)
	{
	    unit->warning("EOF in comment");
	    break;
	}
	else
	{
	    comment += static_cast<char>(c);
	}
    }
    if(comment[0] == '*')
    {
	unit->setComment(comment);
    }
}

"::" {
    return ICE_SCOPE_DELIMITOR;
}

\\?[[:alpha:]_][[:alnum:]_]* {
    StringTokPtr ident = new StringTok;
    if (*yytext == '\\')
    {
	ident->v = yytext + 1;
    }
    else
    {
	ident->v = yytext;
    }
    *yylvalp = ident;

    StringTokenMap::const_iterator pos = keywordMap.find(ident->v);
    if(pos != keywordMap.end())
    {
    	if(pos->first != ident->v)
	{
	    string msg;
	    msg = "illegal identifier: `" + ident->v + "' differs from keyword `";
	    msg += pos->first + "' only in capitalization";
	    unit->error(msg);
	    ident->v = pos->first;
	}
	return pos->second;
    }
    return ICE_IDENTIFIER;
}

\" {
    // "..."-type strings
    StringTokPtr str = new StringTok;
    while(true)
    {
	char c = static_cast<char>(yyinput());
	if(c == '"')
	{
	    break;
	}
	else if(c == EOF)
	{
	    unit->warning("EOF in string");
	    break;
	}
	else if(c == '\n')
	{
	    str->v += c;
	    unit->nextLine();
	}
	else if(c == '\\')
	{
	    char next = static_cast<char>(yyinput());
	    switch(next)
	    {
		case '\\':
		case '"':
		{
		    str->v += next;
		    break;
		}
	    
		case 'n':
		{
		    str->v += '\n';
		    break;
		}
	    
		case 'r':
		{
		    str->v += '\r';
		    break;
		}

		case 't':
		{
		    str->v += '\t';
		    break;
		}
	    
		case 'v':
		{
		    str->v += '\v';
		    break;
		}
	    
		case 'f':
		{
		    str->v += '\f';
		    break;
		}
	    
		default:
		{
		    str->v += c;
		    unput(next);
		}
	    }
	}
	else
	{
	    str->v += c;
	}
    }
    *yylvalp = str;
    return ICE_STRING_LITERAL;
}

\' {
    // '...'-type strings
    StringTokPtr str = new StringTok;
    while(true)
    {
	char c = static_cast<char>(yyinput());
	if(c == '\'')
	{
	    break;
	}
	else if(c == EOF)
	{
	    unit->warning("EOF in string");
	    break;
	}
	else if(c == '\n')
	{
	    str->v += c;
	    unit->nextLine();
	}
	else
	{
	    str->v += c;
	}
    }
    *yylvalp = str;
    return ICE_STRING_LITERAL;
}

[[:space:]] {
    // Igore white-space
    
    if(yytext[0] == '\n')
    {
	unit->nextLine();
    }
}

. {
    return yytext[0];
}

%%

namespace Slice {

//
// initScanner() fills the keyword map with all keyword-token pairs.
//

void
initScanner()
{
    keywordMap["module"] = ICE_MODULE;
    keywordMap["class"] = ICE_CLASS;
    keywordMap["interface"] = ICE_INTERFACE;
    keywordMap["exception"] = ICE_EXCEPTION;
    keywordMap["struct"] = ICE_STRUCT;
    keywordMap["sequence"] = ICE_SEQUENCE;
    keywordMap["dictionary"] = ICE_DICTIONARY;
    keywordMap["enum"] = ICE_ENUM;
    keywordMap["out"] = ICE_OUT;
    keywordMap["extends"] = ICE_EXTENDS;
    keywordMap["implements"] = ICE_IMPLEMENTS;
    keywordMap["throws"] = ICE_THROWS;
    keywordMap["void"] = ICE_VOID;
    keywordMap["byte"] = ICE_BYTE;
    keywordMap["bool"] = ICE_BOOL;
    keywordMap["short"] = ICE_SHORT;
    keywordMap["int"] = ICE_INT;
    keywordMap["long"] = ICE_LONG;
    keywordMap["float"] = ICE_FLOAT;
    keywordMap["double"] = ICE_DOUBLE;
    keywordMap["string"] = ICE_STRING;
    keywordMap["Object"] = ICE_OBJECT;
    keywordMap["LocalObject"] = ICE_LOCAL_OBJECT;
    keywordMap["local"] = ICE_LOCAL;
}

}
