%{

// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/GrammarUtil.h>  // Before Grammer.h, so that YYSTYPE is defined
#include <Slice/Grammar.h>
#include <IceUtil/InputUtil.h>

#include <iomanip>

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

#if defined(_MSC_VER) && defined(ICE_32)
//
// '<' : signed/unsigned mismatch
//
#   pragma warning( 4 : 4018 )
#endif

#ifdef _MSC_VER
#   ifdef slice_wrap
#      undef slice_wrap
#      define slice_wrap() 1
#   endif
#   define YY_NO_UNISTD_H
#endif

#ifdef __SUNPRO_CC
#   ifdef slice_wrap
#      undef slice_wrap
#      define slice_wrap() 1
#   endif
#   ifdef ICE_64
#       pragma error_messages(off,truncwarn)
#   endif
#endif

using namespace std;
using namespace Slice;

namespace Slice
{

//
// Definitions for the case-insensitive keyword-token map.
//
typedef std::map<std::string, int> StringTokenMap;
static StringTokenMap keywordMap;

void initScanner();
int checkKeyword(string&);

}

#define YY_USER_INIT initScanner();

%}

%option noyywrap
%option never-interactive
%option prefix="slice_"
%option outfile="lex.yy.c"

identifier      \\?[[:alpha:]_][[:alnum:]_]*
integer_constant    (\+|-)?((0[0-7]+)|(0x[[:xdigit:]]+)|([[:digit:]]+))
fractional_constant (\+|-)?(([[:digit:]]*\.[[:digit:]]+)|([[:digit:]]+\.))
exponent_part       (e|E)(\+|-)?[[:digit:]]+
floating_literal    (({fractional_constant}{exponent_part}?)|((\+|-)?[[:digit:]]+{exponent_part}))[fF]?

%s BOMSCAN
%s MAINSCAN

%%

^"#"[[:blank:]]*[[:digit:]]+[[:blank:]]*$ {
    if(unit->scanPosition(yytext))
    {
        BEGIN(BOMSCAN);
    }
}

^"#"[[:blank:]]*[[:digit:]]+[[:blank:]]+"\""[^\"]*"\"".*$ {
    if(unit->scanPosition(yytext))
    {
        BEGIN(BOMSCAN);
    }
}

^"#"[[:blank:]]*"line"[[:blank:]]+[[:digit:]]+[[:blank:]]*$ {
    if(unit->scanPosition(yytext))
    {
        BEGIN(BOMSCAN);
    }
}

^"#"[[:blank:]]*"line"[[:blank:]]+[[:digit:]]+[[:blank:]]+"\""[^\"]*"\"".*$ {
    if(unit->scanPosition(yytext))
    {
        BEGIN(BOMSCAN);
    }
}

"//" {
    // C++-style comment
    BEGIN(MAINSCAN);
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
    BEGIN(MAINSCAN);
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
    if(!comment.empty() && comment[0] == '*')
    {
        unit->setComment(comment);
    }
}

"::" {
    BEGIN(MAINSCAN);
    return ICE_SCOPE_DELIMITER;
}

"[" {
    BEGIN(MAINSCAN);
    return ICE_METADATA_OPEN;
}

"]" {
    BEGIN(MAINSCAN);
    return ICE_METADATA_CLOSE;
}

"[[" {
    BEGIN(MAINSCAN);
    return ICE_GLOBAL_METADATA_OPEN;
}

"]]" {
    BEGIN(MAINSCAN);
    return ICE_GLOBAL_METADATA_CLOSE;
}

{identifier}[[:space:]]*"(" {
    BEGIN(MAINSCAN);
    StringTokPtr ident = new StringTok;
    ident->v = *yytext == '\\' ? yytext + 1 : yytext;
    ident->v.erase(ident->v.find_first_of(" \t\v\n\r\f("));
    *yylvalp = ident;
    if(*yytext == '\\')
    {
        return ICE_IDENT_OP;
    }
    int st = checkKeyword(ident->v);
    if(st == ICE_IDENTIFIER)
    {
        return ICE_IDENT_OP;
    }
    else if(st == ICE_OPTIONAL)
    {
        return ICE_OPTIONAL_OP;
    }
    else
    {
        return ICE_KEYWORD_OP;
    }
}

{identifier} {
    BEGIN(MAINSCAN);
    StringTokPtr ident = new StringTok;
    ident->v = *yytext == '\\' ? yytext + 1 : yytext;
    *yylvalp = ident;
    return *yytext == '\\' ? ICE_IDENTIFIER : checkKeyword(ident->v);
}

\" {
    BEGIN(MAINSCAN);
    StringTokPtr str = new StringTok;
    str->literal = "\"";
    while(true)
    {
        char c = static_cast<char>(yyinput());
        str->literal += c;
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
            str->literal += next;
            switch(next)
            {
                case '\\':
                {
                    //
                    // add extra escape to our internal string
                    //
                    str->v += '\\';
                    str->v += '\\';
                    break;
                }
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

                //
                // Octal value \nnn limited to three octal digits but terminate at the first character
                // that is not a valid octal digit if encountered sooner.
                //
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '7':
                {
                    static string octalDigits = "01234567";
                    IceUtil::Int64 value = 0;
                    string escape;
                    escape += next;
                    for(int i = 0; i < 2; ++i)
                    {
                        next = static_cast<char>(yyinput());
                        if(octalDigits.find_first_of(next) == string::npos)
                        {
                            unput(next);
                            break;
                        }
                        escape += next;
                    }
                    str->literal += escape;
                    value = IceUtilInternal::strToInt64(escape.c_str(), 0, 8);

                    if(value == 0)
                    {
                        unit->error("illegal NUL character in string constant");
                    }
                    else if(value > 255)
                    {
                        ostringstream os;
                        os << "octal escape sequence out of range: '\\" << oct << value << "'";
                        unit->warning(os.str());
                    }
                    str->v += static_cast<char>(value);
                    break;
                }

                case 'x':
                {
                    IceUtil::Int64 value = 0;
                    string escape = "";

                    //
                    // Unlike C++, we limit hex escape sequences to 2 hex digits
                    //
                    while(isxdigit(static_cast<unsigned char>(next = static_cast<char>(yyinput()))) && escape.length() < 2)
                    {
                        escape += next;
                    }
                    unput(next);

                    str->literal += escape;
                    value = IceUtilInternal::strToInt64(escape.c_str(), 0, 16);

                    if(value == 0)
                    {
                        unit->error("illegal NUL character in string constant");
                    }
                    assert(value >= 0 && value <= 255);
                    str->v += static_cast<char>(value);
                    break;
                }

                //
                // Universal character name \unnnn code point U+nnnn
                //
                case 'u':
                {
                    IceUtil::Int64 value = 0;
                    string escape = "";

                    for(int i = 0; i < 4; ++i)
                    {
                        next = static_cast<char>(yyinput());
                        str->literal += next;
                        if(!isxdigit(static_cast<unsigned char>(next)))
                        {
                            unit->error("unknown escape sequence in string constant: " + str->literal);
                            break;
                        }
                        escape += next;
                    }

                    value = escape.size() == 4 ? IceUtilInternal::strToInt64(escape.c_str(), 0, 16) : -1;

                    ostringstream os;
                    os << '\\' << 'u';
                    os.fill('0');
                    os.width(4);
                    os << hex << value;

                    if(value == 0)
                    {
                        unit->error("illegal NUL character in string constant");
                    }


                    //
                    // Determine if a character is a surrogate:
                    //
                    // * High surrogate code point, ranging from 0xd800 to 0xdbff, inclusive
                    // * Low surrogate code point, ranging from 0xdc00 to 0xdfff, inclusive.
                    //
                    else if((value >= 0xd800 && value <= 0xdbff) || (value >= 0xdc00 && value <= 0xdfff))
                    {
                        unit->error("unknown escape sequence in string constant: '" + os.str() + "'");
                    }

                    str->v += os.str();

                    break;
                }

                case 'U':
                {
                    IceUtil::Int64 value = 0;
                    string escape = "";

                    for(int i = 0; i < 8; ++i)
                    {
                        next = static_cast<char>(yyinput());
                        str->literal += next;
                        if(!isxdigit(static_cast<unsigned char>(next)))
                        {

                            unit->error("unknown escape sequence in string constant: " + str->literal);
                            break;
                        }
                        escape += next;
                    }

                    value = escape.size() == 8 ? IceUtilInternal::strToInt64(escape.c_str(), 0, 16) : -1;

                    ostringstream os;
                    os << '\\' << 'U';
                    os.fill('0');
                    os.width(8);
                    os << hex << value;

                    if(value == 0)
                    {
                        unit->error("illegal NUL character in string constant");
                    }

                    //
                    // Determine if a character is a surrogate:
                    //
                    // * High surrogate code point, ranging from 0xd800 to 0xdbff, inclusive
                    // * Low surrogate code point, ranging from 0xdc00 to 0xdfff, inclusive.
                    //
                    else if((value >= 0xd800 && value <= 0xdbff) || (value >= 0xdc00 && value <= 0xdfff))
                    {
                        unit->error("unknown escape sequence in string constant: '" + os.str() + "'");
                    }

                    str->v += os.str();
                    break;
                }

                default:
                {
                    ostringstream os;
                    os << "unknown escape sequence '\\" << next << "'";
                    unit->warning(os.str());

                    // Escape the \ in this unknown escape sequence
                    str->v += '\\';
                    str->v += '\\';
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
    BEGIN(MAINSCAN);
    IntegerTokPtr itp = new IntegerTok;
    itp->literal = string(yytext);
    *yylvalp = itp;
    if(!IceUtilInternal::stringToInt64(string(yytext), itp->v))
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
    BEGIN(MAINSCAN);
    errno = 0;
    FloatingTokPtr ftp = new FloatingTok;
    *yylvalp = ftp;
    string literal(yytext);
    ftp->literal = literal;
    char lastChar = literal[literal.size() - 1];
    if(lastChar == 'f' || lastChar == 'F')
    {
        literal = literal.substr(0, literal.size() - 1);    // Clobber trailing 'f' or 'F' suffix
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
    // Ignore white-space

    if(unit->currentLine() != 0)
    {
        BEGIN(MAINSCAN);
    }
    if(yytext[0] == '\n')
    {
        unit->nextLine();
    }
}

<BOMSCAN>^"\357\273\277" {
    // Ignore UTF-8 BOM, rule only active when parsing start of file.

    BEGIN(MAINSCAN);
}

. {
    BEGIN(MAINSCAN);
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
    keywordMap["idempotent"] = ICE_IDEMPOTENT;
    keywordMap["optional"] = ICE_OPTIONAL;
    keywordMap["Value"] = ICE_VALUE;
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
