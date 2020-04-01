%top{
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/ScannerConfig.h>

}

%{

#include <Ice/Ice.h>
#include <IceGrid/Parser.h>
#include <IceGrid/Grammar.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

#ifdef _MSC_VER
#   ifdef yywrap
#      undef yywrap
#      define yywrap() 1
#   endif
#endif

#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

namespace IceGrid
{

static std::map<std::string, int> keywordMap;

void initScanner();
std::string parseDoubleQuotedString();
std::string parseSingleQuotedString();

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
            s += static_cast<char>(c);
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
        int c = yyinput();
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
    keywordMap = {
        {"help", ICE_GRID_HELP},
        {"quit", ICE_GRID_EXIT},
        {"exit", ICE_GRID_EXIT},
        {"application", ICE_GRID_APPLICATION},
        {"server", ICE_GRID_SERVER},
        {"adapter", ICE_GRID_ADAPTER},
        {"add", ICE_GRID_ADD},
        {"remove", ICE_GRID_REMOVE},
        {"list", ICE_GRID_LIST},
        {"shutdown", ICE_GRID_SHUTDOWN},
        {"describe", ICE_GRID_DESCRIBE},
        {"properties", ICE_GRID_PROPERTIES},
        {"property", ICE_GRID_PROPERTY},
        {"state", ICE_GRID_STATE},
        {"pid", ICE_GRID_PID},
        {"endpoints", ICE_GRID_ENDPOINTS},
        {"start", ICE_GRID_START},
        {"stop", ICE_GRID_STOP},
        {"signal", ICE_GRID_SIGNAL},
        {"stdout", ICE_GRID_STDOUT},
        {"stderr", ICE_GRID_STDERR},
        {"node", ICE_GRID_NODE},
        {"registry", ICE_GRID_REGISTRY},
        {"ping", ICE_GRID_PING},
        {"load", ICE_GRID_LOAD},
        {"processors", ICE_GRID_SOCKETS},
        {"sockets", ICE_GRID_SOCKETS},
        {"activation", ICE_GRID_ACTIVATION},
        {"object", ICE_GRID_OBJECT},
        {"find", ICE_GRID_FIND},
        {"show", ICE_GRID_SHOW},
        {"copying", ICE_GRID_COPYING},
        {"warranty", ICE_GRID_WARRANTY},
        {"diff", ICE_GRID_DIFF},
        {"update", ICE_GRID_UPDATE},
        {"instantiate", ICE_GRID_INSTANTIATE},
        {"template", ICE_GRID_TEMPLATE},
        {"service", ICE_GRID_SERVICE},
        {"enable", ICE_GRID_ENABLE},
        {"disable", ICE_GRID_DISABLE}
    };
}

std::string
parseDoubleQuotedString()
{
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
    return s;
}

std::string
parseSingleQuotedString()
{
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
            s += static_cast<char>(c);
        }
    }
    return s;
}

}
