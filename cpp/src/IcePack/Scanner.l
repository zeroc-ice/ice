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

#ifdef WIN32
#   include <io.h>
#   define isatty _isatty
#   define fileno _fileno
//  '_isatty' : inconsistent dll linkage.  dllexport assumed.
#   pragma warning( disable : 4273 )
#endif

using namespace std;
using namespace Ice;
using namespace IcePack;

const char* yycommands;

#ifdef HAVE_READLINE

#   include <readline/readline.h>
#   include <readline/history.h>

#   define YY_INPUT(buf, result, maxSize) \
\
if (yycommands) \
{ \
    if (strcmp(yycommands, ";") == 0) \
    { \
        buf[0] = EOF; \
        result = 1; \
    } \
    else \
    { \
	result = strlen(yycommands); \
	if (result >= maxSize) \
	{ \
	    YY_FATAL_ERROR("fatal error in flex scanner: command line too long"); \
	} \
	else \
	{ \
	    strcpy(buf, yycommands); \
	    yycommands = ";"; \
	} \
    }\
} \
else if (yy_current_buffer->yy_is_interactive) \
{ \
    char* line = readline(parser->getPrompt()); \
    if (line && *line) \
    { \
         add_history(line); \
    } \
    if (!line) \
    { \
	buf[0] = EOF; \
	result = 1; \
    } \
    else \
    { \
	result = strlen(line) + 1; \
	if (result >= maxSize) \
	{ \
	    free(line); \
	    YY_FATAL_ERROR("fatal error in flex scanner: input line too long"); \
	} \
	else \
	{ \
	    strcpy(buf, line); \
	    strcat(buf, "\n"); \
	    free(line); \
	} \
    } \
} \
else \
{ \
    if (((result = fread(buf, 1, maxSize, yyin)) == 0) && ferror(yyin)) \
    { \
	YY_FATAL_ERROR("input in flex scanner failed"); \
    } \
}

#else

#   define YY_INPUT(buf, result, maxSize) \
\
if (yycommands) \
{ \
    if (strcmp(yycommands, ";") == 0) \
    { \
        buf[0] = EOF; \
        result = 1; \
    } \
    else \
    { \
	result = strlen(yycommands); \
	if (result >= maxSize) \
	{ \
	    YY_FATAL_ERROR("fatal error in flex scanner: command line too long"); \
	} \
	else \
	{ \
	    strcpy(buf, yycommands); \
	    yycommands = ";"; \
	} \
    }\
} \
else if (yy_current_buffer->yy_is_interactive) \
{ \
    if (isatty(fileno(yyin))) \
    { \
	cout << parser->getPrompt() << flush; \
    } \
    int c = '*', n; \
    for (n = 0; n < maxSize && (c = getc(yyin)) != EOF && c != '\n'; ++n ) \
    { \
        buf[n] = (char)c; \
    } \
    if (c == '\n') \
    { \
        buf[n++] = (char)c; \
    } \
    if (c == EOF && ferror(yyin)) \
    { \
        YY_FATAL_ERROR("input in flex scanner failed"); \
    } \
    result = n; \
} \
else \
{ \
    if (((result = fread(buf, 1, maxSize, yyin)) == 0) && ferror(yyin)) \
    { \
	YY_FATAL_ERROR("input in flex scanner failed"); \
    } \
}

#endif

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
    yylval.clear();
    yylval.push_back(s);
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
    yylval.clear();
    yylval.push_back(s);
    return ICE_PACK_STRING;
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
