%{

// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/GrammarUtil.h>	// Before Grammer.h, so that YYSTYPE is defined
#include <Slice/Grammar.h>
#include <IceUtil/InputUtil.h>

#include <stdlib.h>
#include <math.h>

#if defined(_MSC_VER) && defined(ICE_64)
//
// '=' : conversion from 'size_t' to 'int', possible loss of data
// The result of fread() is a size_t and gets inserted into an int
//
#   pragma warning( 4 : 4267 )
//
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
// Puts a pointer-difference into an int
//
#   pragma warning( 4 : 4244 )
#endif

using namespace std;
using namespace Slice;

namespace Slice
{

//
// Definitions for the case-insensitive keyword-token map.
//
typedef std::map<std::string, int, Slice::CICompare> StringTokenMap;
static StringTokenMap keywordMap;

void initScanner();
void checkIdentifier(const string&);
int checkKeyword(string&);

}

#define	YY_USER_INIT initScanner();

%}

%option noyywrap
%option never-interactive
%option prefix="slice_"
%option outfile="lex.yy.c"

identifier		\\?[[:alpha:]_][[:alnum:]_]*
integer_constant	(\+|-)?((0[0-7]+)|(0x[[:xdigit:]]+)|([[:digit:]]+))
fractional_constant	(\+|-)?(([[:digit:]]*\.[[:digit:]]+)|([[:digit:]]+\.))
exponent_part		(e|E)(\+|-)?[[:digit:]]+
floating_literal	(({fractional_constant}{exponent_part}?)|((\+|-)?[[:digit:]]+{exponent_part}))[fF]?

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
    return ICE_SCOPE_DELIMITER;
}

"[" {
    return ICE_METADATA_OPEN;
}

"]" {
    return ICE_METADATA_CLOSE;
}

"[[" {
    return ICE_GLOBAL_METADATA_OPEN;
}

"]]" {
    return ICE_GLOBAL_METADATA_CLOSE;
}

{identifier}[[:space:]]*"(" {
    StringTokPtr ident = new StringTok;
    ident->v = *yytext == '\\' ? yytext + 1 : yytext;
    ident->v.erase(ident->v.find_first_of(" \t\v\n\r\f("));
    *yylvalp = ident;
    checkIdentifier(ident->v);
    if(*yytext == '\\')
    {
        return ICE_IDENT_OP;
    }
    return checkKeyword(ident->v) == ICE_IDENTIFIER ? ICE_IDENT_OP : ICE_KEYWORD_OP;
}

{identifier} {
    StringTokPtr ident = new StringTok;
    ident->v = *yytext == '\\' ? yytext + 1 : yytext;
    *yylvalp = ident;
    checkIdentifier(ident->v);
    return *yytext == '\\' ? ICE_IDENTIFIER : checkKeyword(ident->v);
}

\" {
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
	    unit->error("EOF in string");
	    break;
	}
	else if(c == '\n')
	{
	    unit->error("newline in string");
	}
	else if(c == '\\')
	{
	    char next = static_cast<char>(yyinput());
	    switch(next)
	    {
		case '\\':
		case '"':
		case '\'':
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

		case 'a':
		{
		    str->v += '\a';
		    break;
		}

		case 'b':
		{
		    str->v += '\b';
		    break;
		}

		case '?':
		{
		    str->v += '\?';
		    break;
		}

		case '0':
		case '1':
		case '2':
		case '3':
		{
		    static string octalDigits = "01234567";
		    unsigned short us = next - '0';
		    if(octalDigits.find_first_of(next = static_cast<char>(yyinput())) != string::npos)
		    {
		    	us = us * 8 + next - '0';
			if(octalDigits.find_first_of(next = static_cast<char>(yyinput())) != string::npos)
			{
			    us = us * 8 + next - '0';
			}
			else
			{
			    unput(next);
			}
		    }
		    else
		    {
		    	unput(next);
		    }
		    if(us == 0)
		    {
			unit->error("illegal NUL character in string constant");
		    }
		    str->v += static_cast<char>(us);
		    break;
		}
		case 'x':
		{
		    IceUtil::Int64 ull = 0;
		    while(isxdigit(next = static_cast<char>(yyinput())))
		    {
			ull *= 16;
			if(isdigit(next))
			{
			    ull += next - '0';
			}
			else if(islower(next))
			{
			    ull += next - 'a' + 10;
			}
			else
			{
			    ull += next - 'A' + 10;
			}
		    }
		    unput(next);
		    if(ull == 0)
		    {
			unit->error("illegal NUL character in string constant");
		    }
		    str->v += static_cast<char>(ull);
		    break;
		}
	
		// TODO: add universal character names
	    
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

{integer_constant} {
    IntegerTokPtr itp = new IntegerTok;
    *yylvalp = itp;
    if(!IceUtil::stringToInt64(string(yytext), itp->v))
    {
	assert(itp->v != 0);
	string msg = "integer constant `";
	msg += yytext;
	msg += "' out of range";
	unit->error(msg);
    }
    return ICE_INTEGER_LITERAL;
}

{floating_literal} {
    errno = 0;
    FloatingTokPtr ftp = new FloatingTok;
    *yylvalp = ftp;
    string literal(yytext);
    char lastChar = literal[literal.size() - 1];
    if(lastChar == 'f' || lastChar == 'F')
    {
    	literal = literal.substr(0, literal.size() - 1);	// Clobber trailing 'f' or 'F' suffix
    }
    ftp->v = strtod(literal.c_str(), 0);
    if((ftp->v == HUGE_VAL || ftp->v == -HUGE_VAL) && errno == ERANGE)
    {
	string msg = "floating-point constant `";
	msg += yytext;
	msg += "' too large (overflow)";
	unit->error(msg);
    }
    else if(ftp->v == 0 && errno == ERANGE)
    {
	string msg = "floating-point constant `";
	msg += yytext;
	msg += "' too small (underflow)";
	unit->error(msg);
    }
    return ICE_FLOATING_POINT_LITERAL;
}

[[:space:]] {
    // Igore white-space
    
    if(yytext[0] == '\n')
    {
	unit->nextLine();
    }
}

. {
    if(yytext[0] < 32 || yytext[0] > 126)
    {
	stringstream s;
	s << "illegal input character: '\\";
	s.width(3);
	s.fill('0');
	s << oct << static_cast<int>(static_cast<unsigned char>(yytext[0]));
	s << "'";
	unit->error(s.str());
	return BAD_CHAR;
    }
    return yytext[0];
}

%%

namespace Slice {

//
// initScanner() fills the keyword map with all keyword-token pairs.
//

void
initScanner()
{
    keywordMap["module"] = ICE_MODULE;
    keywordMap["class"] = ICE_CLASS;
    keywordMap["interface"] = ICE_INTERFACE;
    keywordMap["exception"] = ICE_EXCEPTION;
    keywordMap["struct"] = ICE_STRUCT;
    keywordMap["sequence"] = ICE_SEQUENCE;
    keywordMap["dictionary"] = ICE_DICTIONARY;
    keywordMap["enum"] = ICE_ENUM;
    keywordMap["out"] = ICE_OUT;
    keywordMap["extends"] = ICE_EXTENDS;
    keywordMap["implements"] = ICE_IMPLEMENTS;
    keywordMap["throws"] = ICE_THROWS;
    keywordMap["void"] = ICE_VOID;
    keywordMap["byte"] = ICE_BYTE;
    keywordMap["bool"] = ICE_BOOL;
    keywordMap["short"] = ICE_SHORT;
    keywordMap["int"] = ICE_INT;
    keywordMap["long"] = ICE_LONG;
    keywordMap["float"] = ICE_FLOAT;
    keywordMap["double"] = ICE_DOUBLE;
    keywordMap["string"] = ICE_STRING;
    keywordMap["Object"] = ICE_OBJECT;
    keywordMap["LocalObject"] = ICE_LOCAL_OBJECT;
    keywordMap["local"] = ICE_LOCAL;
    keywordMap["const"] = ICE_CONST;
    keywordMap["false"] = ICE_FALSE;
    keywordMap["true"] = ICE_TRUE;
    keywordMap["nonmutating"] = ICE_NONMUTATING;
    keywordMap["idempotent"] = ICE_IDEMPOTENT;
}

//
// Check if an identifier is well-formed.
//

void
checkIdentifier(const string& id)
{
    if(id.find('_') != string::npos)
    {
	unit->error("illegal underscore in identifier `" + id + "'");
    }

    //
    // Weed out identifiers with reserved suffixes.
    //
    static const string suffixBlacklist[] = { "Helper", "Holder", "Prx", "Ptr" };
    for(size_t i = 0; i < sizeof(suffixBlacklist) / sizeof(*suffixBlacklist); ++i)
    {
	if(id.find(suffixBlacklist[i], id.size() - suffixBlacklist[i].size()) != string::npos)
	{
	    unit->error("illegal identifier `" + id + "': `" + suffixBlacklist[i] + "' suffix is reserved");
	}
    }
}

//
// Check if an identifier looks like a keyword.
// If the identifier is a keyword, return the
// corresponding keyword token; otherwise, return
// an identifier token.
//

int
checkKeyword(string& id)
{
    StringTokenMap::const_iterator pos = keywordMap.find(id);
    if(pos != keywordMap.end())
    {
    	if(pos->first != id)
	{
	    string msg;
	    msg = "illegal identifier: `" + id + "' differs from keyword `";
	    msg += pos->first + "' only in capitalization";
	    unit->error(msg);
	    id = pos->first;
	}
	return pos->second;
    }
    return ICE_IDENTIFIER;
}

}
