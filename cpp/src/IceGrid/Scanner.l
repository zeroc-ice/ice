%top{
// Copyright (c) ZeroC, Inc.

#include "../Ice/ScannerConfig.h"
#include <cstdint>

// NOLINTBEGIN

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
    return ICEGRID_STRING;
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
    return ICEGRID_STRING;
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
    return pos != keywordMap.end() ? pos->second : ICEGRID_STRING;
}

%%

// NOLINTEND

namespace IceGrid
{
    // This function is always called once, right before scanning begins.
    // It fills the keyword map with all keyword-token pairs.
    void initScanner()
    {
        keywordMap["help"] = ICEGRID_HELP;
        keywordMap["quit"] = ICEGRID_EXIT;
        keywordMap["exit"] = ICEGRID_EXIT;
        keywordMap["application"] = ICEGRID_APPLICATION;
        keywordMap["server"] = ICEGRID_SERVER;
        keywordMap["adapter"] = ICEGRID_ADAPTER;
        keywordMap["add"] = ICEGRID_ADD;
        keywordMap["remove"] = ICEGRID_REMOVE;
        keywordMap["list"] = ICEGRID_LIST;
        keywordMap["shutdown"] = ICEGRID_SHUTDOWN;
        keywordMap["describe"] = ICEGRID_DESCRIBE;
        keywordMap["properties"] = ICEGRID_PROPERTIES;
        keywordMap["property"] = ICEGRID_PROPERTY;
        keywordMap["status"] = ICEGRID_STATUS;
        keywordMap["pid"] = ICEGRID_PID;
        keywordMap["endpoints"] = ICEGRID_ENDPOINTS;
        keywordMap["start"] = ICEGRID_START;
        keywordMap["stop"] = ICEGRID_STOP;
        keywordMap["signal"] = ICEGRID_SIGNAL;
        keywordMap["stdout"] = ICEGRID_STDOUT;
        keywordMap["stderr"] = ICEGRID_STDERR;
        keywordMap["node"] = ICEGRID_NODE;
        keywordMap["registry"] = ICEGRID_REGISTRY;
        keywordMap["ping"] = ICEGRID_PING;
        keywordMap["load"] = ICEGRID_LOAD;
        keywordMap["processors"] = ICEGRID_SOCKETS;
        keywordMap["sockets"] = ICEGRID_SOCKETS;
        keywordMap["activation"] = ICEGRID_ACTIVATION;
        keywordMap["object"] = ICEGRID_OBJECT;
        keywordMap["find"] = ICEGRID_FIND;
        keywordMap["show"] = ICEGRID_SHOW;
        keywordMap["copying"] = ICEGRID_COPYING;
        keywordMap["warranty"] = ICEGRID_WARRANTY;
        keywordMap["diff"] = ICEGRID_DIFF;
        keywordMap["update"] = ICEGRID_UPDATE;
        keywordMap["instantiate"] = ICEGRID_INSTANTIATE;
        keywordMap["template"] = ICEGRID_TEMPLATE;
        keywordMap["service"] = ICEGRID_SERVICE;
        keywordMap["enable"] = ICEGRID_ENABLE;
        keywordMap["disable"] = ICEGRID_DISABLE;
    }

    // This function is called whenever the scanner encounters an unrecoverable error.
    void yynoreturn fatalError(const char* msg)
    {
        cerr << "fatal error: " << msg << endl
             << "\tlast matched text: '" << yytext << "'" << endl
             << "\tlast scanner state: '" << YY_START << "'" << endl;
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
