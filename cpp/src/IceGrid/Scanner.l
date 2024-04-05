%top{
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IceUtil/ScannerConfig.h"
#include <cstdint>

}

%{

#include "Ice/Ice.h"
#include "Parser.h"
#include "Grammar.h"

#include <iostream>

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{
    using StringTokenMap = map<string, int>;
    StringTokenMap keywordMap;

    void initScanner();
    void yynoreturn fatalError(const char* msg);
    string parseDoubleQuotedString();
    string parseSingleQuotedString();
}

// Override some of the functions flex auto-generates with our own implementations.
#define YY_USER_INIT initScanner();
#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)
#define YY_FATAL_ERROR(msg) fatalError(msg);

%}

  /* Instructs flex to not suppress any warnings when generating the scanner. */
%option warn
  /* By default flex will 'default match' any text it encounters that doesn't match any specified rules. This
   * option disables default-matching (it throws 'scanner jammed' instead) to make grammar holes more obvious. */
%option nodefault
  /* Directs flex to generate a scanner tailored for use by bison.
   * These options change the signature of the main lexing function, which must match the one declared in Grammar.y */
%option bison-bridge

  /* Ensures flex generates a scanner that supports reading 8-bit characters. */
%option 8bit
  /* Directs flex to generate lookup tables that are better aligned in memory to
   * improve access speeds, even if this means allocating larger tables. */
%option align
  /* Directs flex to store matched text as 'char *' instead of char arrays, for improved performance. */
%option pointer
  /* We always want the scanner to run in interactive mode. */
%option always-interactive

  /* Disables the generation of functions we don't use to reduce clutter, and possibly improve performance. */
%option noyywrap
%option noyy_scan_buffer noyy_scan_bytes noyy_scan_string
%option noyyget_extra noyyset_extra noyyget_leng noyyget_text
%option noyyget_in noyyset_in noyyget_out noyyset_out
%option noyyget_lineno noyyset_lineno noyyget_lloc noyyset_lloc
%option noyyget_lval noyyset_lval noyyget_debug noyyset_debug

WS      [ \t\v\f\r]
NL      [\n]
keyword [[:alpha:]]*

%%

"//" {
    // C++-style comment
    int c;
    do
    {
        c = yyinput();
    }
    while (c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    while (true)
    {
        int c = yyinput();
        if (c == '*')
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

{WS}*(\\{WS}*{NL})? {
    size_t len = strlen(yytext);
    for (size_t i = 0; i < len; ++i)
    {
        if (yytext[i] == '\\')
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
    yylval->clear();
    yylval->push_back(s);
    return ICE_GRID_STRING;
}

\' {
    // '...'-type strings
    string s;
    while (true)
    {
        int c = yyinput();
        if (c == '\'')
        {
            break;
        }
        else if (c == EOF)
        {
            parser->warning("EOF in string");
            break;
        }
        else
        {
            s += static_cast<char>(c);
        }
    }
    yylval->clear();
    yylval->push_back(s);
    return ICE_GRID_STRING;
}

. {
    // Simple strings
    string s;
    s += yytext[0];
    while (true)
    {
        int c = yyinput();
        if (c == EOF)
        {
            break;
        }
        else if (c == '"')
        {
            s += parseDoubleQuotedString();
            continue;
        }
        else if (c == '\'')
        {
            s += parseSingleQuotedString();
            continue;
        }
        else if (isspace(c) || c == ';')
        {
            unput(c);
            break;
        }
        s += static_cast<char>(c);
    }

    yylval->clear();
    yylval->push_back(s);

    const auto pos = keywordMap.find(s);
    return pos != keywordMap.end() ? pos->second : ICE_GRID_STRING;
}

%%

namespace IceGrid
{
    // This function is always called once, right before scanning begins.
    // It fills the keyword map with all keyword-token pairs.
    void initScanner()
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

    // This function is called whenever the scanner encounters an unrecoverable error.
    void yynoreturn fatalError(const char* msg)
    {
        cerr << "fatal error: " << msg << endl
             << "\tlast matched text: `" << yytext << "'" << endl
             << "\tlast scanner state: `" << YY_START << "'" << endl;
        exit(YY_EXIT_FAILURE);
    }

    string
    parseDoubleQuotedString()
    {
        string s;
        while (true)
        {
            int c = yyinput();
            if (c == '"')
            {
                break;
            }
            else if (c == EOF)
            {
                parser->warning("EOF in string");
                break;
            }
            else if (c == '\\')
            {
                int next = yyinput();
                switch (next)
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

    string
    parseSingleQuotedString()
    {
        string s;
        while (true)
        {
            int c = yyinput();
            if (c == '\'')
            {
                break;
            }
            else if (c == EOF)
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
