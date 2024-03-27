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

#ifdef _MSC_VER
#    ifdef yywrap
#        undef yywrap
#        define yywrap() 1
#    endif
#    define YY_NO_UNISTD_H
#endif

using namespace std;
using namespace Ice;
using namespace IceStorm;

namespace IceStorm
{
    typedef map<string, int> StringTokenMap;
    StringTokenMap keywordMap;

    void initScanner();
}

// Override some of the functions flex auto-generates with our own implementations.
#define YY_USER_INIT initScanner();
#define YY_INPUT(buf, result, maxSize) parser->getInput(buf, result, maxSize)

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
    return ICE_STORM_STRING;
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
    return ICE_STORM_STRING;
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

    StringTokenMap::const_iterator pos = keywordMap.find(s);
    return pos != keywordMap.end() ? pos->second : ICE_STORM_STRING;
}

%%

namespace IceStorm
{
    // This function is always called once, right before scanning begins.
    // It fills the keyword map with all keyword-token pairs.
    void initScanner()
    {
        keywordMap["help"] = ICE_STORM_HELP;
        keywordMap["quit"] = ICE_STORM_EXIT;
        keywordMap["exit"] = ICE_STORM_EXIT;
        keywordMap["current"] = ICE_STORM_CURRENT;
        keywordMap["create"] = ICE_STORM_CREATE;
        keywordMap["destroy"] = ICE_STORM_DESTROY;
        keywordMap["link"] = ICE_STORM_LINK;
        keywordMap["unlink"] = ICE_STORM_UNLINK;
        keywordMap["links"] = ICE_STORM_LINKS;
        keywordMap["topics"] = ICE_STORM_TOPICS;
        keywordMap["replica"] = ICE_STORM_REPLICA;
        keywordMap["subscribers"] = ICE_STORM_SUBSCRIBERS;
    }
}
