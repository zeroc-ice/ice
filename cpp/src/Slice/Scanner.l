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

#include <Slice/GrammerUtil.h> // Before Grammer.h, so that YYSTYPE is defined
#include <Slice/Grammer.h>

using namespace std;
using namespace Slice;

%}

%option noyywrap
%option never-interactive

WS	[ \t\v\n\r\f]
S	[ \t]
D	[0-9]
L	[a-zA-Z_]

%%

^"#"{S}*{D}+{S}*$ {
    unit->scanPosition(yytext);
}

^"#"{S}*{D}+{S}+"\""[^\"]*"\"".*$ {
    unit->scanPosition(yytext);
}

^"#"{S}*"line"{S}+{D}+{S}*$ {
    unit->scanPosition(yytext);
}

^"#"{S}*"line"{S}+{D}+{S}+"\""[^\"]*"\"".*$ {
    unit->scanPosition(yytext);
}

"//" {
    // C++-style comment
    int c;
    do
    {
	c = yyinput();
	if (c == '\n')
	{
	    unit->nextLine();
	}
    }
    while (c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    string comment = yytext + 2;
    while (true)
    {
	int c = yyinput();
	comment += static_cast<char>(c);
	if (c == '\n')
	{
	    unit->nextLine();
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
	    unit->warning("EOF in comment");
	    break;
	}
    }
    if (comment[0] == '*')
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

"class" {
    return ICE_CLASS;
}

"interface" {
    return ICE_INTERFACE;
}

"struct" {
    return ICE_STRUCT;
}

"local" {
    return ICE_LOCAL;
}

"extends" {
    return ICE_EXTENDS;
}

"implements" {
    return ICE_IMPLEMENTS;
}

"throws" {
    return ICE_THROWS;
}

"void" {
    return ICE_VOID;
}

"byte" {
    return ICE_BYTE;
}

"bool" {
    return ICE_BOOL;
}

"short" {
    return ICE_SHORT;
}

"int" {
    return ICE_INT;
}

"long" {
    return ICE_LONG;
}

"float" {
    return ICE_FLOAT;
}

"double" {
    return ICE_DOUBLE;
}

"string" {
    return ICE_STRING;
}

"wstring" {
    return ICE_WSTRING;
}

"Object" {
    return ICE_OBJECT;
}

"LocalObject" {
    return ICE_LOCAL_OBJECT;
}

"native" {
    return ICE_NATIVE;
}

"sequence" {
    return ICE_SEQUENCE;
}

"dictionary" {
    return ICE_DICTIONARY;
}

"enum" {
    return ICE_ENUM;
}

"nonmutating" {
    return ICE_NONMUTATING;
}

"\\"?{L}({L}|{D})* {
    char* s = yytext;
    
    if (s[0] == '\\') // Strip leading backslash
    {
	++s;
    }
    
    StringTokPtr ident = new StringTok;
    ident->v = s;
    *yylvalp = ident;
    return ICE_IDENTIFIER;
}

"\\"?{L}({L}|{D})*{WS}*"(" {
    char* s = yytext;
    
    if (s[0] == '\\') // Strip leading backslash
    {
	++s;
    }
    
    StringTokPtr ident = new StringTok;
    ident->v = s;
    ident->v.erase(ident->v.find_first_of(" \t\v\n\r\f("));
    *yylvalp = ident;
    return ICE_OP_IDENTIFIER;
}

{WS} {
    // Igore white-space
    
    if (yytext[0] == '\n')
    {
	unit->nextLine();
    }
}

. {
    return yytext[0];
}

%%
