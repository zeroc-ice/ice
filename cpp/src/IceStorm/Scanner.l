%top{
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/ScannerConfig.h>

}

%{

#include <Ice/Ice.h>
#include <IceStorm/Parser.h>
#include <IceStorm/Grammar.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;

#ifdef _MSC_VER
#   ifdef yywrap
#      undef yywrap
#      define yywrap() 1
#   endif
#endif

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

namespace IceStorm
{

static std::map<std::string, int> keywordMap;

void initScanner();

}
#define         YY_USER_INIT initScanner();

%}

WS      [ \t\v\f\r]
NL      [\n]
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
    string s;
    while(true)
    {
        int c = yyinput();
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
            int next = yyinput();
            switch(next)
            {
                case '\\':
                case '"':
                {
                    s += static_cast<char>(next);
                    break;
                }

                default:
                {
                    s += static_cast<char>(c);
                    unput(next);
                }
            }
        }
        else
        {
            s += static_cast<char>(c);
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
        int c = yyinput();
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
    return ICE_STORM_STRING;
}

. {
    // Simple strings
    string s;
    s += yytext[0];
    while(true)
    {
        int c = yyinput();
        if(c == EOF)
        {
            break;
        }
        else if(isspace(c) || c == ';')
        {
            unput(c);
            break;
        }

        s += static_cast<char>(c);
    }

    yylvalp->clear();
    yylvalp->push_back(s);

    const auto pos = keywordMap.find(s);
    return pos != keywordMap.end() ? pos->second : ICE_STORM_STRING;
}

%%

namespace IceStorm {

//
// initScanner() fills the keyword map with all keyword-token pairs.
//

void
initScanner()
{
    keywordMap = {
        {"help", ICE_STORM_HELP},
        {"quit", ICE_STORM_EXIT},
        {"exit", ICE_STORM_EXIT},
        {"current", ICE_STORM_CURRENT},
        {"create", ICE_STORM_CREATE},
        {"destroy", ICE_STORM_DESTROY},
        {"link", ICE_STORM_LINK},
        {"unlink", ICE_STORM_UNLINK},
        {"links", ICE_STORM_LINKS},
        {"topics", ICE_STORM_TOPICS},
        {"replica", ICE_STORM_REPLICA},
        {"subscribers", ICE_STORM_SUBSCRIBERS},
    };
}

}
