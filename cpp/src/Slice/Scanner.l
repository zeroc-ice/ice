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

"module" {
    return ICE_MODULE;
}

"module"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"class" {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    *yylvalp = local;
    return ICE_CLASS;
}

"local"[[:space:]]*"class" {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    *yylvalp = local;
    return ICE_CLASS;
}

"class"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"interface" {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    *yylvalp = local;
    return ICE_INTERFACE;
}

"local"[[:space:]]*"interface" {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    *yylvalp = local;
    return ICE_INTERFACE;
}

"interface"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"exception" {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    *yylvalp = local;
    return ICE_EXCEPTION;
}

"local"[[:space:]]*"exception" {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    *yylvalp = local;
    return ICE_EXCEPTION;
}

"exception"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"struct" {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    *yylvalp = local;
    return ICE_STRUCT;
}

"local"[[:space:]]*"struct" {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    *yylvalp = local;
    return ICE_STRUCT;
}

"struct"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"sequence" {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    *yylvalp = local;
    return ICE_SEQUENCE;
}

"local"[[:space:]]*"sequence" {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    *yylvalp = local;
    return ICE_SEQUENCE;
}

"sequence"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"local"[[:space:]]*"dictionary" {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    *yylvalp = local;
    return ICE_DICTIONARY;
}

"dictionary" {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    *yylvalp = local;
    return ICE_DICTIONARY;
}

"dictionary"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"enum" {
    BoolTokPtr local = new BoolTok;
    local->v = false;
    *yylvalp = local;
    return ICE_ENUM;
}

"local"[[:space:]]*"enum" {
    BoolTokPtr local = new BoolTok;
    local->v = true;
    *yylvalp = local;
    return ICE_ENUM;
}

"enum"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"out" {
    return ICE_OUT;
}

"out"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"extends" {
    return ICE_EXTENDS;
}

"extends"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"implements" {
    return ICE_IMPLEMENTS;
}

"implements"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"throws" {
    return ICE_THROWS;
}

"throws"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"void" {
    return ICE_VOID;
}

"void"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"byte" {
    return ICE_BYTE;
}

"byte"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"bool" {
    return ICE_BOOL;
}

"bool"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"short" {
    return ICE_SHORT;
}

"short"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"int" {
    return ICE_INT;
}

"int"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"long" {
    return ICE_LONG;
}

"long"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"float" {
    return ICE_FLOAT;
}

"float"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"double" {
    return ICE_DOUBLE;
}

"double"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"string" {
    return ICE_STRING;
}

"string"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"Object" {
    return ICE_OBJECT;
}

"Object"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
}

"LocalObject" {
    return ICE_LOCAL_OBJECT;
}

"LocalObject"[[:space:]]*"(" {
    return ICE_OP_KEYWORD;
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
    return ICE_IDENTIFIER;
}

\\?[[:alpha:]_][[:alnum:]_]*[[:space:]]*"(" {
    StringTokPtr ident = new StringTok;
    if (*yytext == '\\')
    {
	ident->v = yytext + 1;
    }
    else
    {
	ident->v = yytext;
    }
    ident->v.erase(ident->v.find_first_of(" \t\v\n\r\f("));
    *yylvalp = ident;
    return ICE_OP_IDENTIFIER;
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
