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

#include <Parser.h>
#include <Grammer.h>

using namespace std;
using namespace Slice;

%}

%option noyywrap
%option never-interactive

WS			[ \t\v\n\r\f]
S			[ \t]
D			[0-9]
L			[a-zA-Z_]

%%

^"#"{S}*{D}+{S}*$ {
    parser -> scanPosition(yytext);
}

^"#"{S}*{D}+{S}+"\""[^\"]*"\"".*$ {
    parser -> scanPosition(yytext);
}

^"#"{S}*"line"{S}+{D}+{S}*$ {
    parser -> scanPosition(yytext);
}

^"#"{S}*"line"{S}+{D}+{S}+"\""[^\"]*"\"".*$ {
    parser -> scanPosition(yytext);
}

"//" {
    // C++-style comment
    
    int c;
    
    do
    {
	c = yyinput();
	if(c == '\n')
	    parser -> nextLine();
    }
    while(c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
   
    string comment = yytext + 1;
    while(true)
    {
	int c = yyinput();
	comment += static_cast<char>(c);
	
	if(c == '\n')
	{
	    parser -> nextLine();
	}
	else if(c == '*')
	{
	    int next = yyinput();
	    
	    if(next == '/')
		break;
	    else
		unput(next);
	}
	else if(c == EOF)
	{
	    parser -> warning("EOF in comment");
	    break;
	}
    }

    if(comment[0] == '*')
	parser -> setComment(comment);
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

"local" {
    return ICE_LOCAL;
}

"extends" {
    return ICE_EXTENDS;
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

"vector" {
    return ICE_VECTOR;
}

"\\"?{L}({L}|{D})* {
    char* s = yytext;
    
    if(s[0] == '\\') // Strip leading backslash
	++s;
    
    String_ptr ident = new String;
    ident -> v = s;
    yylval = ident;
    return ICE_IDENTIFIER;
}

"\\"?{L}({L}|{D})*{WS}*"(" {
    char* s = yytext;
    
    if(s[0] == '\\') // Strip leading backslash
	++s;
    
    String_ptr ident = new String;
    ident -> v = s;
    ident -> v.erase(ident -> v.find_first_of(" \t\v\n\r\f("));
    yylval = ident;
    return ICE_OP_IDENTIFIER;
}

{WS} {
    // Igore white-space
    
    if(yytext[0] == '\n')
	parser -> nextLine();
}

. {
    return yytext[0];
}

%%
