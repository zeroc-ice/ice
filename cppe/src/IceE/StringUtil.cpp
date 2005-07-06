// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/StringUtil.h>

using namespace std;

static string
toOctalString(unsigned int n)
{
    string s;
    s.resize(32);
    string::size_type charPos = 32;
    const int radix = 1 << 3;
    int mask = radix - 1;
    do
    {
        s[--charPos] = '0' + (n & mask);
        n >>= 3;
    }
    while(n != 0);

    return string(s, charPos, (32 - charPos));
}

static void
escapeChar(string::value_type b, string& s, const string& special)
{
    switch(b)
    {
    case '\\':
    {
        s.append("\\\\");
        break;
    }
    case '\'':
    {
        s.append("\\'");
        break;
    }
    case '"':
    {
        s.append("\\\"");
        break;
    }
    case '\b':
    {
        s.append("\\b");
        break;
    }
    case '\f':
    {
        s.append("\\f");
        break;
    }
    case '\n':
    {
        s.append("\\n");
        break;
    }
    case '\r':
    {
        s.append("\\r");
        break;
    }
    case '\t':
    {
        s.append("\\t");
        break;
    }
    default:
    {
        if(static_cast<signed char>(b) <= 31 || b == 127)
        {
            s.push_back('\\');
            string octal = toOctalString(b);

            //
            // Add leading zeroes so that we avoid problems during
            // decoding. For example, consider the escaped string
            // \0013 (i.e., a character with value 1 followed by the
            // character '3'). If the leading zeroes were omitted, the
            // result would be incorrectly interpreted as a single
            // character with value 11.
            //
            for(string::size_type j = octal.size(); j < 3; j++)
            {
                s.push_back('0');
            }
            s.append(octal);
        }
        else if(special.find(b) != string::npos)
        {
            s.push_back('\\');
            escapeChar(b, s, "");
        }
        else
        {
            s.push_back(b);
        }
    }
    }
}

string
Ice::escapeString(const string& s, const string& special)
{
    string result;
    result.reserve(s.size());

    for(string::size_type i = 0; i < s.size(); i++)
    {
        escapeChar(s[i], result, special);
    }

    return result;
}

bool
Ice::unescapeString(const string& s, string::size_type start, string::size_type end, string& result)
{
    assert(end <= s.size());
    assert(start <= end);

    result.reserve(end - start);

    while(start < end)
    {
        char ch = s[start];
        if(ch == '\\')
        {
            start++;
            if(start == end)
            {
                return false; // Missing character.
            }
            ch = s[start];
            switch(ch)
            {
            case '\\':
            {
                result.push_back('\\');
                break;
            }
            case '\'':
            case '"':
            {
                result.push_back(ch);
                break;
            }
            case 'b':
            {
                result.push_back('\b');
                break;
            }
            case 'f':
            {
                result.push_back('\f');
                break;
            }
            case 'n':
            {
                result.push_back('\n');
                break;
            }
            case 'r':
            {
                result.push_back('\r');
                break;
            }
            case 't':
            {
                result.push_back('\t');
                break;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                int count = 0;
                int val = 0;
                while(count < 3 && start < end && s[start] >= '0' && s[start] <= '9')
                {
                    val <<= 3;
                    val |= s[start] - '0';
                    start++;
                    count++;
                }
                if(val > 255)
                {
                    return false; // Octal value out of range.
                }
                result.push_back((string::value_type)val);
                continue; // Don't increment start.
            }
            default:
            {
                if(static_cast<signed char>(ch) <= 31 || ch == 127)
                {
                    return false; // Malformed encoding.
                }
                else
                {
                    result.push_back(ch);
                }
            }
            }
        }
        else
        {
            result.push_back(ch);
        }
        start++;
    }

    return true;
}

//
// If a single or double quotation mark is found at the start position,
// then the position of the matching closing quote is returned. If no
// quotation mark is found at the start position, then 0 is returned.
// If no matching closing quote is found, then -1 is returned.
//
string::size_type
Ice::checkQuote(const string& s, string::size_type start)
{
    string::value_type quoteChar = s[start];
    if(quoteChar == '"' || quoteChar == '\'')
    {
        start++;
        string::size_type pos;
        while(start < s.size() && (pos = s.find(quoteChar, start)) != string::npos)
        {
            if(s[pos - 1] != '\\')
            {
                return pos;
            }
            start = pos + 1;
        }
        return string::npos; // Unmatched quote.
    }
    return 0; // Not quoted.
}

//
// Match `s' against the pattern `pat'. A * in the pattern acts
// as a wildcard: it matches any non-empty sequence of characters
// other than a period (`.'). We match by hand here because
// it's portable across platforms (whereas regex() isn't).
//
bool
Ice::match(const string& s, const string& pat, bool matchPeriod)
{
    assert(!s.empty());
    assert(!pat.empty());

    if(pat.find('*') == string::npos)
    {
        return s == pat;
    }

    string::size_type sIndex = 0;
    string::size_type patIndex = 0;
    do
    {
        if(pat[patIndex] == '*')
	{
	    //
	    // Don't allow matching x..y against x.*.y if requested -- star matches non-empty sequence only.
	    //
	    if(!matchPeriod && s[sIndex] == '.')
	    {
		return false;
	    }
	    while(sIndex < s.size() && (matchPeriod || s[sIndex] != '.'))
	    {
	        ++sIndex;
	    }
	    patIndex++;
	}
	else
	{
	    if(pat[patIndex] != s[sIndex])
	    {
		return false;
	    }
	    ++sIndex;
	    ++patIndex;
	}
    }
    while(sIndex < s.size() && patIndex < pat.size());

    return sIndex == s.size() && patIndex == pat.size();
}
