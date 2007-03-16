// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/StringUtil.h>
#include <IceE/ExceptionBase.h>

using namespace std;
using namespace IceUtil;

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

//
// Write the byte b as an escape sequence if it isn't a printable ASCII
// character and append the escape sequence to s. Additional characters
// that should be escaped can be passed in special. If b is any of these
// characters, b is preceded by a backslash in s.
//
static void
encodeChar(string::value_type b, string& s, const string& special)
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
	    unsigned char i = static_cast<unsigned char>(b);
	    if(!(i >= 32 && i <= 126))
	    {
		s.push_back('\\');
		string octal = toOctalString(i);
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
		s.push_back(b);
	    }
	    else
	    {
		s.push_back(b);
	    }
	    break;
	}
    }
}

//
// Add escape sequences (such as "\n", or "\007") to make a string
// readable in ASCII. Any characters that appear in special are
// prefixed with a backslash in the returned string.
//
string
IceUtil::escapeString(const string& s, const string& special)
{
    string::size_type i;
    for(i = 0; i < special.size(); ++i)
    {
	if(static_cast<unsigned char>(special[i]) < 32 || static_cast<unsigned char>(special[i]) > 126)
	{
	    throw IllegalArgumentException(__FILE__, __LINE__, "special characters must be in ASCII range 32-126");
	}
    }
    
    string result;
    for(i = 0; i < s.size(); ++i)
    {
	encodeChar(s[i], result, special);
    }
    
    return result;
}

static char
checkChar(char c)
{
    if(!(static_cast<unsigned char>(c) >= 32 && static_cast<unsigned char>(c) <= 126))
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "illegal input character");
    }
    return c;
}

//
// Decode the character or escape sequence starting at start and return it.
// end marks the one-past-the-end position of the substring to be scanned.
// nextStart is set to the index of the first character following the decoded
// character or escape sequence.
//
static char
decodeChar(const string& s, string::size_type start, string::size_type end, string::size_type& nextStart)
{
    assert(start >= 0);
    assert(start < end);
    assert(end <= s.size());

    char c;

    if(s[start] != '\\')
    {
	c = checkChar(s[start++]);
    }
    else
    {
	if(start + 1 == end)
	{
	    throw IllegalArgumentException(__FILE__, __LINE__, "trailing backslash in argument");
	}
	switch(s[++start])
	{
	    case '\\': 
	    case '\'': 
	    case '"': 
	    {
		c = s[start++];
		break;
	    }
	    case 'b': 
	    {
		++start;
		c = '\b';
		break;
	    }
	    case 'f': 
	    {
		++start;
		c = '\f';
		break;
	    }
	    case 'n': 
	    {
		++start;
		c = '\n';
		break;
	    }
	    case 'r': 
	    {
		++start;
		c = '\r';
		break;
	    }
	    case 't': 
	    {
		++start;
		c = '\t';
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
	    {
		int oct = 0;
		for(int j = 0; j < 3 && start < end; ++j)
		{
		    int charVal = s[start++] - '0';
		    if(charVal < 0 || charVal > 7)
		    {
		        --start;
			break;
		    }
		    oct = oct * 8 + charVal;
		}
		if(oct > 255)
		{
		    throw IllegalArgumentException(__FILE__, __LINE__, "octal value out of range");
		}
		c = (char)oct;
		break;
	    }
	    default:
	    {
		c = checkChar(s[start++]);
		break;
	    }
	}
    }
    nextStart = start;
    return c;
}

//
// Remove escape sequences from s and append the result to sb.
// Return true if successful, false otherwise.
//
static void decodeString(const string& s, string::size_type start, string::size_type end, string& sb)
{
    while(start < end)
    {
	sb.push_back(decodeChar(s, start, end, start));
    }
}

//
// Remove escape sequences added by escapeString.
//
bool
IceUtil::unescapeString(const string& s, string::size_type start, string::size_type end, string& result)
{
    if(start < 0)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "start offset must be >= 0");
    }
    if(end > s.size())
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "end offset must be <= s.size()");
    }
    if(start > end)
    {
	throw IllegalArgumentException(__FILE__, __LINE__, "start offset must <= end offset");
    }

    result.reserve(end - start);

    try
    {
	result.clear();
	decodeString(s, start, end, result);
	return true;
    }
    catch(...)
    {
	return false;
    }
}

//
// If a single or double quotation mark is found at the start position,
// then the position of the matching closing quote is returned. If no
// quotation mark is found at the start position, then 0 is returned.
// If no matching closing quote is found, then -1 is returned.
//
string::size_type
IceUtil::checkQuote(const string& s, string::size_type start)
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
IceUtil::match(const string& s, const string& pat, bool matchPeriod)
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
