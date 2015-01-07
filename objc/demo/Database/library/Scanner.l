%{ 
// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Parser.h>
#include <Grammar.h>
#include <ctype.h>

#import <Foundation/NSString.h>
#import <Foundation/NSData.h>

#define YY_INPUT(buf, result, maxSize) \
    {                                                  \
        result = [parser getInput: buf max: maxSize ]; \
    }

%}

WS	[ \t\v\f\r]
NL	[\n]

%option noyywrap
%option always-interactive

%%

"//" {
    // C++-style comment
    int c;
    do
    {
	c = input();
    }
    while(c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    while(true)
    {
	int c = input();
	if(c == '*')
	{
	    int next = input();
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
	    [parser warning: "EOF in comment"];
	    break;
	}
    }
}

"help" {
    return TOK_HELP;
}

"quit"|"exit" {
    return TOK_EXIT;
}

"add" {
    return TOK_ADD_BOOK;
}

"isbn" {
    return TOK_FIND_ISBN;
}

"authors" {
    return TOK_FIND_AUTHORS;
}

"title" {
    return TOK_FIND_TITLE;
}

"next" {
    return TOK_NEXT_FOUND_BOOK;
}

"current" {
    return TOK_PRINT_CURRENT;
}

"rent" {
    return TOK_RENT_BOOK;
}

"return" {
    return TOK_RETURN_BOOK;
}

"remove" {
    return TOK_REMOVE_CURRENT;
}

{WS}*(\\{WS}*{NL})? {
    size_t len = strlen(yytext);
    size_t i;
    for(i = 0; i < len; ++i)
    {
	if(yytext[i] == '\\')
	{
	    [parser continueLine];
	}
    }
}

{NL}|; {
    return ';';
}

\" {
    // "..."-type strings
    NSMutableData* s = [ [ NSMutableData alloc] init];
    while(true)
    {
	char c = (char)(input());
	if(c == '"')
	{
	    break;
	}
	else if(c == EOF)
	{
	    [parser warning: "EOF in string"];
	    break;
	}
	else if(c == '\\')
	{
	    char next = (char)(input());
	    switch(next)
	    {
		case '\\':
		case '"':
		{
                    c = next;
		    break;
		}
	    
		case 'n':
		{
                    c = '\n';
		    break;
		}
	    
		case 'r':
		{
                    c = '\r';
		    break;
		}

		case 't':
		{
                    c = '\t';
		    break;
		}
	    
		case 'v':
		{
                    c = '\v';
		    break;
		}
	    
		case 'f':
		{
                    c = '\f';
		    break;
		}
	    
		default:
		{
		    unput(next);
		}
	    }
	}
        [s appendBytes: &c length: 1];
    }

    *yylvalp = [NSMutableArray array];
    NSString* ss = [ [NSString alloc] initWithData: s encoding: NSUTF8StringEncoding ];
    [s release];
    [*yylvalp addObject: ss];
    [ss release];
    return TOK_STRING;
}

\' {
    // '...'-type strings
    NSMutableData* s = [ [ NSMutableData alloc] init];
    while(true)
    {
	char c = (char)(input());
	if(c == '\'')
	{
	    break;
	}
	else if(c == EOF)
	{
            [parser warning: "EOF in string"];
	    break;
	}
	else
	{
            [s appendBytes: &c length: 1];
	}
    }
    *yylvalp = [NSMutableArray array];
    NSString* ss = [ [NSString alloc] initWithData: s encoding: NSUTF8StringEncoding ];
    [s release];
    [*yylvalp addObject: ss];
    [ss release];
    return TOK_STRING;
}

. {
    // Simple strings
    NSMutableData* s = [ [ NSMutableData alloc] init];
    [s appendBytes: yytext length: 1];
    while(true)
    {
	char c = (char)(input());
	if(c == EOF)
	{
	    break;
	}
	else if(isspace(c) || c == ';')
	{
	    unput(c);
	    break;
	}
	
        [s appendBytes: &c length: 1];
    }
    *yylvalp = [NSMutableArray array];
    NSString* ss = [ [NSString alloc] initWithData: s encoding: NSUTF8StringEncoding ];
    [s release];
    [*yylvalp addObject: ss];
    [ss release];
    return TOK_STRING;
}

%%
