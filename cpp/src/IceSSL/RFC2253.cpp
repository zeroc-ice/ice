// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringUtil.h>

#include <IceSSL/Plugin.h>
#include <IceSSL/RFC2253.h>
#include <Ice/Object.h>

using namespace std;
using namespace IceSSL;

namespace
{

//
// See RFC 2253 and RFC 1779.
//

string special = ",=+<>#;";
string hexvalid = "0123456789abcdefABCDEF";

}

static char unescapeHex(const string&, size_t);
static pair<string,string> parseNameComponent(const string&, size_t&);
static pair<string,string> parseAttributeTypeAndValue(const string&, size_t&);
static string parseAttributeType(const string&, size_t&);
static string parseAttributeValue(const string&, size_t&);
static string parsePair(const string&, size_t&);
static string parseHexPair(const string&, size_t&, bool);
static void eatWhite(const string&, size_t&);

IceSSL::RFC2253::RDNEntrySeq
IceSSL::RFC2253::parse(const string& data)
{
    RDNEntrySeq results;
    RDNEntry current;
    current.negate = false;
    size_t pos = 0;
    while(pos < data.size())
    {
        eatWhite(data, pos);
        if(pos < data.size() && data[pos] == '!')
        {
            if(!current.rdn.empty())
            {
                throw ParseException(__FILE__, __LINE__, "negation symbol '!' must appear at start of list");
            }
            ++pos;
            current.negate = true;
        }
        current.rdn.push_back(parseNameComponent(data, pos));
        eatWhite(data, pos);
        if(pos < data.size() && data[pos] == ',')
        {
            ++pos;
        }
        else if(pos < data.size() && data[pos] == ';')
        {
            ++pos;
            results.push_back(current);
            current.rdn.clear();
            current.negate = false;
        }
        else if(pos < data.size())
        {
            throw ParseException(__FILE__, __LINE__, "expected ',' or ';' at `" + data.substr(pos) + "'");
        }
    }
    if(!current.rdn.empty())
    {
        results.push_back(current);
    }

    return results;
}

IceSSL::RFC2253::RDNSeq
IceSSL::RFC2253::parseStrict(const string& data)
{
    RDNSeq results;
    size_t pos = 0;
    while(pos < data.size())
    {
        results.push_back(parseNameComponent(data, pos));
        eatWhite(data, pos);
        if(pos < data.size() && (data[pos] == ',' || data[pos] == ';'))
        {
            ++pos;
        }
        else if(pos < data.size())
        {
            throw ParseException(__FILE__, __LINE__, "expected ',' or ';' at `" + data.substr(pos) + "'");
        }
    }
    return results;
}

string
IceSSL::RFC2253::unescape(const string& data)
{
    if(data.size() == 0)
    {
        return data;
    }

    if(data[0] == '"')
    {
        if(data[data.size() - 1] != '"')
        {
            throw ParseException(__FILE__, __LINE__, "unescape: missing \"");
        }

        //
        // Return the string without quotes.
        //
        return data.substr(1, data.size() - 2);
    }

    //
    // Unescape the entire string.
    //
    string result;
    if(data[0] == '#')
    {
        size_t pos = 1;
        while(pos < data.size())
        {
            result += unescapeHex(data, pos);
            pos += 2;
        }
    }
    else
    {
        size_t pos = 0;
        while(pos < data.size())
        {
            if(data[pos] != '\\')
            {
                result += data[pos];
                ++pos;
            }
            else
            {
                ++pos;
                if(pos >= data.size())
                {
                    throw ParseException(__FILE__, __LINE__, "unescape: invalid escape sequence");
                }
                if(special.find(data[pos]) != string::npos || data[pos] != '\\' || data[pos] != '"')
                {
                    result += data[pos];
                    ++pos;
                }
                else
                {
                    result += unescapeHex(data, pos);
                    pos += 2;
                }
            }
        }
    }

    return result;
}

static int
hexToInt(char v)
{
    if(v >= '0' && v <= '9')
    {
        return v - '0';
    }
    if(v >= 'a' && v <= 'f')
    {
        return 10 + (v - 'a');
    }
    if(v >= 'A' && v <= 'F')
    {
        return 10 + (v - 'A');
    }
    throw ParseException(__FILE__, __LINE__, "unescape: invalid hex pair");
    return 0; // To satisfy the compiler.
}

static char
unescapeHex(const string& data, size_t pos)
{
    assert(pos < data.size());
    if(pos + 2 >= data.size())
    {
        throw ParseException(__FILE__, __LINE__, "unescape: invalid hex pair");
    }
    return static_cast<char>(hexToInt(data[pos]) * 16 + hexToInt(data[pos + 1]));
}

static pair<string,string>
parseNameComponent(const string& data, size_t& pos)
{
    pair<string, string> final = parseAttributeTypeAndValue(data, pos);
    while(pos < data.size())
    {
        eatWhite(data, pos);
        if(pos < data.size() && data[pos] == '+')
        {
            ++pos;
        }
        else
        {
            break;
        }
        pair<string, string> p = parseAttributeTypeAndValue(data, pos);
        final.second += "+";
        final.second += p.first;
        final.second += '=';
        final.second += p.second;
    }
    return final;
}

static pair<string,string>
parseAttributeTypeAndValue(const string& data, size_t& pos)
{
    pair<string, string> p;
    p.first = parseAttributeType(data, pos);
    eatWhite(data, pos);

    if(pos >= data.size())
    {
        throw ParseException(__FILE__, __LINE__, "invalid attribute type/value pair (unexpected end of data)");
    }
    if(data[pos] != '=')
    {
        throw ParseException(__FILE__, __LINE__, "invalid attribute type/value pair (missing =)");
    }
    ++pos;
    p.second = parseAttributeValue(data, pos);
    return p;
}

static string
parseAttributeType(const string& data, size_t& pos)
{
    eatWhite(data, pos);
    if(pos >= data.size())
    {
        throw ParseException(__FILE__, __LINE__, "invalid attribute type (expected end of data)");
    }

    string result;

    //
    // RFC 1779.
    // <key> ::= 1*( <keychar> ) | "OID." <oid> | "oid." <oid>
    // <oid> ::= <digitstring> | <digitstring> "." <oid>
    // RFC 2253:
    // attributeType = (ALPHA 1*keychar) | oid
    // keychar    = ALPHA | DIGIT | "-"
    // oid        = 1*DIGIT *("." 1*DIGIT)
    //
    // In section 4 of RFC 2253 the document says:
    // Implementations MUST allow an oid in the attribute type to be
    // prefixed by one of the character strings "oid." or "OID.".
    //
    // Here we must also check for "oid." and "OID." before parsing
    // according to the ALPHA KEYCHAR* rule.
    // 
    // First the OID case.
    //
    if(IceUtilInternal::isDigit(data[pos]) ||
       (data.size() - pos >= 4 && (data.substr(pos, 4) == "oid." || data.substr(pos, 4) == "OID.")))
    {
        if(!IceUtilInternal::isDigit(data[pos]))
        {
            result += data.substr(pos, 4);
            pos += 4;
        }

        while(true)
        {
            // 1*DIGIT
            while(pos < data.size() && IceUtilInternal::isDigit(data[pos]))
            {
                result += data[pos];
                ++pos;
            }
            // "." 1*DIGIT
            if(pos < data.size() && data[pos] == '.')
            {
                result += data[pos];
                ++pos;
                // 1*DIGIT must follow "."
                if(pos < data.size() && !IceUtilInternal::isDigit(data[pos]))
                {
                    throw ParseException(__FILE__, __LINE__, "invalid attribute type (expected end of data)");
                }
            }
            else
            {
                break;
            }
        }
    }
    else if(IceUtilInternal::isAlpha(data[pos]))
    {
        //
        // The grammar is wrong in this case. It should be ALPHA
        // KEYCHAR* otherwise it will not accept "O" as a valid
        // attribute type.
        //
        result += data[pos];
        ++pos;
        // 1* KEYCHAR
        while(pos < data.size() && 
              (IceUtilInternal::isAlpha(data[pos]) || IceUtilInternal::isDigit(data[pos]) || data[pos] == '-'))
        {
            result += data[pos];
            ++pos;
        }
    }
    else
    {
        throw ParseException(__FILE__, __LINE__, "invalid attribute type");
    }
    return result;
}

static string
parseAttributeValue(const string& data, size_t& pos)
{
    eatWhite(data, pos);
    string result;
    if(pos >= data.size())
    {
        return result;
    }

    //
    // RFC 2253
    // # hexstring
    //
    if(data[pos] == '#')
    {
        result += data[pos];
        ++pos;
        while(true)
        {
            string h = parseHexPair(data, pos, true);
            if(h.size() == 0)
            {
                break;
            }
            result += h;
        }
    }
    //
    // RFC 2253
    // QUOTATION *( quotechar | pair ) QUOTATION ; only from v2
    // quotechar     = <any character except "\" or QUOTATION >
    //
    else if(data[pos] == '"')
    {
        result += data[pos];
        ++pos;
        while(true)
        {
            if(pos >= data.size())
            {
                throw ParseException(__FILE__, __LINE__, "invalid attribute value (unexpected end of data)");
            }
            // final terminating "
            if(data[pos] == '"')
            {
                result += data[pos];
                ++pos;
                break;
            }
            // any character except '\'
            else if(data[pos] != '\\')
            {
                result += data[pos];
                ++pos;
            }
            // pair '\'
            else
            {
                result += parsePair(data, pos);
            }
        }
    }
    //
    // RFC 2253
    // * (stringchar | pair)
    // stringchar = <any character except one of special, "\" or QUOTATION >
    //
    else
    {
        while(pos < data.size())
        {
            if(data[pos] == '\\')
            {
                result += parsePair(data, pos);
            }
            else if(special.find(data[pos]) == string::npos && data[pos] != '"')
            {
                result += data[pos];
                ++pos;
            }
            else
            {
                break;
            }
        }
    }
    return result;
}

//
// RFC2253:
// pair       = "\" ( special | "\" | QUOTATION | hexpair )
//
static string
parsePair(const string& data, size_t& pos)
{
    string result;

    assert(data[pos] == '\\');
    result += data[pos];
    ++pos;

    if(pos >= data.size())
    {
        throw ParseException(__FILE__, __LINE__, "invalid escape format (unexpected end of data)");
    }

    if(special.find(data[pos]) != string::npos || data[pos] != '\\' || data[pos] != '"')
    {
        result += data[pos];
        ++pos;
        return result;
    }
    return parseHexPair(data, pos, false);
}
    
//
// RFC 2253
// hexpair    = hexchar hexchar
//
static string
parseHexPair(const string& data, size_t& pos, bool allowEmpty)
{
    string result;
    if(pos < data.size() && hexvalid.find(data[pos]) != string::npos)
    {
        result += data[pos];
        ++pos;
    }
    if(pos < data.size() && hexvalid.find(data[pos]) != string::npos)
    {
        result += data[pos];
        ++pos;
    }
    if(result.size() != 2)
    {
        if(allowEmpty && result.size() == 0)
        {
            return result;
        }
        throw ParseException(__FILE__, __LINE__, "invalid hex format");
    }
    return result;
}

//
// RFC 2253:
//
// Implementations MUST allow for space (' ' ASCII 32) characters to be
// present between name-component and ',', between attributeTypeAndValue
// and '+', between attributeType and '=', and between '=' and
// attributeValue.  These space characters are ignored when parsing.
//
static void
eatWhite(const string& data, size_t& pos)
{
    while(pos < data.size() && data[pos] == ' ')
    {
        ++pos;
    }
}

#ifdef never
void
print(const list< list<pair<string, string> > >& r)
{
    if(r.size() > 1)
    {
        cout << "result: " << r.size() << " DNs" << endl;
    }
    for(list< list<pair<string, string> > >::const_iterator q = r.begin(); q != r.end(); ++q)
    {
        list<pair<string, string> > l = *q;
        cout << "result: " << l.size() << " RDNs" << endl;
        for(list<pair<string, string> >::const_iterator p = l.begin(); p != l.end(); ++p)
        {
            cout << "\t\"" << p->first << "\"=\"" << p->second << "\"" << endl;
        }
    }
}

int
main()
{
    string examples[] = {
        "CN=Steve Kille,O=Isode Limited,C=GB",
        "OU=Sales+CN=J. Smith,O=Widget Inc.,C=US",
        "CN=L. Eagle,O=Sue\\, Grabbit and Runn,C=GB",
        "CN=Before\\0DAfter,O=Test,C=GB",
        "1.3.6.1.4.1.1466.0=#04024869,O=Test,C=GB",
        "SN=Lu\\C4\\8Di\\C4\\87",
    };
    try
    {
        for(int i = 0; i < sizeof(examples)/sizeof(examples[0]); ++i)
        {
            cout << "string: " << examples[i] << endl;
            print(RFC2253::parse(examples[i]));
        }
    }
    catch(const RFC2253::ParseException& e)
    {
        cout << "error: " << e.reason << endl;
    }
}
#endif
