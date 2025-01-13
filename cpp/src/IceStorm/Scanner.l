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
using namespace IceStorm;

namespace IceStorm
{
    using StringTokenMap = map<string, int>;
    StringTokenMap keywordMap;

    void initScanner();
    void yynoreturn fatalError(const char* msg);
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
    yylval->clear();
    yylval->push_back(s);
    return ICESTORM_STRING;
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
            s += c;
        }
    }
    yylval->clear();
    yylval->push_back(s);
    return ICESTORM_STRING;
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
    return pos != keywordMap.end() ? pos->second : ICESTORM_STRING;
}

%%

// NOLINTEND

namespace IceStorm
{
    // This function is always called once, right before scanning begins.
    // It fills the keyword map with all keyword-token pairs.
    void initScanner()
    {
        keywordMap["help"] = ICESTORM_HELP;
        keywordMap["quit"] = ICESTORM_EXIT;
        keywordMap["exit"] = ICESTORM_EXIT;
        keywordMap["current"] = ICESTORM_CURRENT;
        keywordMap["create"] = ICESTORM_CREATE;
        keywordMap["destroy"] = ICESTORM_DESTROY;
        keywordMap["link"] = ICESTORM_LINK;
        keywordMap["unlink"] = ICESTORM_UNLINK;
        keywordMap["links"] = ICESTORM_LINKS;
        keywordMap["topics"] = ICESTORM_TOPICS;
        keywordMap["replica"] = ICESTORM_REPLICA;
        keywordMap["subscribers"] = ICESTORM_SUBSCRIBERS;
    }

    // This function is called whenever the scanner encounters an unrecoverable error.
    void yynoreturn fatalError(const char* msg)
    {
        cerr << "fatal error: " << msg << endl
             << "\tlast matched text: '" << yytext << "'" << endl
             << "\tlast scanner state: '" << YY_START << "'" << endl;
        exit(YY_EXIT_FAILURE);
    }
}
