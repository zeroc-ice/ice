%{

// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/GrammarUtil.h>        // Before Grammar.h, so that YYSTYPE is defined
#include <FreezeScript/Grammar.h>
#include <IceUtil/InputUtil.h>

#include <stdlib.h>
#include <math.h>

#include <map>

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

#ifdef _MSC_VER
#   ifdef freeze_script_wrap
#      undef freeze_script_wrap
#      define freeze_script_wrap() 1
#   endif
#   define YY_NO_UNISTD_H
#endif

#ifdef __SUNPRO_CC
#   ifdef freeze_script_wrap
#      undef freeze_script_wrap
#      define freeze_script_wrap() 1
#   endif
#   ifdef ICE_64
#       pragma error_messages(off,truncwarn)
#   endif
#endif

using namespace std;
using namespace FreezeScript;

namespace FreezeScript
{

typedef map<string, int> KeywordMap;
static KeywordMap keywordMap;

void initScanner();
int checkKeyword(const string&);
StringTokPtr parseString(char);

}

#define        YY_USER_INIT initScanner();

#define        YY_INPUT(buf, result, max_size) { result = getInput(buf, max_size); }
%}

%option noyywrap
%option never-interactive
%option prefix="freeze_script_"
%option outfile="lex.yy.c"

identifier		[[:alpha:]_][[:alnum:]_]*
integer_constant	(\+|-)?((0[0-7]+)|(0x[[:xdigit:]]+)|([[:digit:]]+))
fractional_constant	(\+|-)?(([[:digit:]]*\.[[:digit:]]+)|([[:digit:]]+\.))
exponent_part		(e|E)(\+|-)?[[:digit:]]+
floating_literal	(({fractional_constant}{exponent_part}?)|((\+|-)?[[:digit:]]+{exponent_part}))[fF]?

%%

"//" {
    // C++-style comment
    int c;
    do
    {
        c = yyinput();
        if(c == '\n')
        {
            parseLine++;
        }
    }
    while(c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    while(true)
    {
        int c = yyinput();
        if(c == '\n')
        {
            parseLine++;
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
                unput(next);
            }
        }
        else if(c == EOF)
        {
            parseErrorReporter->expressionSyntaxError("EOF in comment");
            break;
        }
    }
}

{identifier} {
    StringTokPtr ident = new StringTok;
    ident->v = yytext;
    *yylvalp = ident;
    return checkKeyword(ident->v);
}

\" {
    StringTokPtr str = parseString('"');
    *yylvalp = str;
    return TOK_STRING_LITERAL;
}

\' {
    StringTokPtr str = parseString('\'');
    *yylvalp = str;
    return TOK_STRING_LITERAL;
}

{integer_constant} {
    IntegerTokPtr itp = new IntegerTok;
    *yylvalp = itp;
    if(!IceUtilInternal::stringToInt64(string(yytext), itp->v))
    {
	assert(itp->v != 0);
        string msg = "integer constant `";
        msg += yytext;
        msg += "' out of range";
        parseErrorReporter->expressionSyntaxError(msg);
    }
    return TOK_INTEGER_LITERAL;
}

{floating_literal} {
    errno = 0;
    FloatingTokPtr ftp = new FloatingTok;
    *yylvalp = ftp;
    string literal(yytext);
    char lastChar = literal[literal.size() - 1];
    if(lastChar == 'f' || lastChar == 'F')
    {
        literal = literal.substr(0, literal.size() - 1);        // Clobber trailing 'f' or 'F' suffix
    }
    ftp->v = strtod(literal.c_str(), 0);
    if((ftp->v == HUGE_VAL || ftp->v == -HUGE_VAL) && errno == ERANGE)
    {
        string msg = "floating-point constant `";
        msg += yytext;
        msg += "' too large (overflow)";
        parseErrorReporter->expressionSyntaxError(msg);
    }
    else if(ftp->v == 0 && errno == ERANGE)
    {
        string msg = "floating-point constant `";
        msg += yytext;
        msg += "' too small (underflow)";
        parseErrorReporter->expressionSyntaxError(msg);
    }
    return TOK_FLOATING_POINT_LITERAL;
}

[[:space:]] {
    // Igore white-space
    
    if(yytext[0] == '\n')
    {
        parseLine++;
    }
}

"<"     return TOK_LESS_THAN;
">"     return TOK_GREATER_THAN;
"<="    return TOK_LESS_EQUAL;
">="    return TOK_GREATER_EQUAL;
"=="    return TOK_EQUAL;
"!="    return TOK_NEQ;
"+"     return TOK_ADD;
"-"     return TOK_SUB;
"*"     return TOK_MUL;
"/"     return TOK_DIV;
"%"     return TOK_MOD;
"("     return TOK_LPAREN;
")"     return TOK_RPAREN;
"["     return TOK_LBRACKET;
"]"     return TOK_RBRACKET;
"::"    return TOK_SCOPE_DELIMITER;

. {
    return yytext[0];
}

%%

namespace FreezeScript
{

void
initScanner()
{
    keywordMap["true"] = TOK_TRUE;
    keywordMap["false"] = TOK_FALSE;
    keywordMap["and"] = TOK_AND;
    keywordMap["or"] = TOK_OR;
    keywordMap["not"] = TOK_NOT;
    keywordMap["nil"] = TOK_NIL;
}

int
checkKeyword(const string& id)
{
    KeywordMap::const_iterator pos = keywordMap.find(id);
    if(pos != keywordMap.end())
    {
        return pos->second;
    }
    return TOK_IDENTIFIER;
}

StringTokPtr
parseString(char start)
{
    StringTokPtr str = new StringTok;
    while(true)
    {
        char c = static_cast<char>(yyinput());
        if(c == start)
        {
            break;
        }
        else if(c == EOF)
        {
            parseErrorReporter->expressionSyntaxError("EOF in string");
            break;
        }
        else if(c == '\n')
        {
            parseErrorReporter->expressionSyntaxError("newline in string");
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
                    str->v += static_cast<char>(us);
                    break;
                }
                case 'x':
                {
                    IceUtil::Int64 ull = 0;
                    while(isxdigit(static_cast<unsigned char>(next = static_cast<char>(yyinput()))))
                    {
                        ull *= 16;
                        if(isdigit(static_cast<unsigned char>(next)))
                        {
                            ull += next - '0';
                        }
                        else if(islower(static_cast<unsigned char>(next)))
                        {
                            ull += next - 'a' + 10;
                        }
                        else
                        {
                            ull += next - 'A' + 10;
                        }
                    }
                    unput(next);
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

    return str;
}

} // End of namespace FreezeScript
