// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringUtil.h>
#include <IceUtil/StringConverter.h>
#include <cstring>

#ifdef ICE_OS_WINRT
#  include <IceUtil/ScopedArray.h>
#endif

using namespace std;
using namespace IceUtil;

namespace
{

string
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
void
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

}

//
// Add escape sequences (such as "\n", or "\007") to make a string
// readable in ASCII. Any characters that appear in special are
// prefixed with a backslash in the returned string.
//
string
IceUtilInternal::escapeString(const string& s, const string& special)
{
    for(string::size_type i = 0; i < special.size(); ++i)
    {
        if(static_cast<unsigned char>(special[i]) < 32 || static_cast<unsigned char>(special[i]) > 126)
        {
            throw IllegalArgumentException(__FILE__, __LINE__, "special characters must be in ASCII range 32-126");
        }
    }
    
    string result;
    for(string::size_type i = 0; i < s.size(); ++i)
    {
        encodeChar(s[i], result, special);
    }
    
    return result;
}

namespace
{

char
checkChar(const string& s, string::size_type pos)
{
    unsigned char c = static_cast<unsigned char>(s[pos]);
    if(!(c >= 32 && c <= 126))
    {
        ostringstream ostr;
        if(pos > 0)
        {
            ostr << "character after `" << s.substr(0, pos) << "'";
        }
        else
        {
            ostr << "first character";
        }
        ostr << " is not a printable ASCII character (ordinal " << static_cast<int>(c) << ")";
        throw IllegalArgumentException(__FILE__, __LINE__, ostr.str());
    }
    return c;
}

//
// Decode the character or escape sequence starting at start and return it.
// end marks the one-past-the-end position of the substring to be scanned.
// nextStart is set to the index of the first character following the decoded
// character or escape sequence.
//
char
decodeChar(const string& s, string::size_type start, string::size_type end, string::size_type& nextStart)
{
    assert(start < end);
    assert(end <= s.size());

    char c;

    if(s[start] != '\\')
    {
        c = checkChar(s, start++);
    }
    else
    {
        if(start + 1 == end)
        {
            throw IllegalArgumentException(__FILE__, __LINE__, "trailing backslash");
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
                int val = 0;
                for(int j = 0; j < 3 && start < end; ++j)
                {
                    int charVal = s[start++] - '0';
                    if(charVal < 0 || charVal > 7)
                    {
                        --start;
                        break;
                    }
                    val = val * 8 + charVal;
                }
                if(val > 255)
                {
                    ostringstream ostr;
                    ostr << "octal value \\" << oct << val << dec << " (" << val << ") is out of range";
                    throw IllegalArgumentException(__FILE__, __LINE__, ostr.str());
                }
                c = static_cast<char>(val);
                break;
            }
            default:
            {
                c = checkChar(s, start++);
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
void
decodeString(const string& s, string::size_type start, string::size_type end, string& sb)
{
    while(start < end)
    {
        sb.push_back(decodeChar(s, start, end, start));
    }
}

}

//
// Remove escape sequences added by escapeString.
//
string
IceUtilInternal::unescapeString(const string& s, string::size_type start, string::size_type end)
{
    assert(start <= end && end <= s.size());

    string result;
    result.reserve(end - start);
    result.clear();
    decodeString(s, start, end, result);
    return result;
}

bool
IceUtilInternal::splitString(const string& str, const string& delim, vector<string>& result)
{
    string::size_type pos = 0;
    string::size_type length = str.length();
    string elt;

    char quoteChar = '\0';
    while(pos < length)
    {
        if(quoteChar == '\0' && (str[pos] == '"' || str[pos] == '\''))
        {
            quoteChar = str[pos++];
            continue; // Skip the quote
        }
        else if(quoteChar == '\0' && str[pos] == '\\' && pos + 1 < length && 
                (str[pos + 1] == '\'' || str[pos + 1] == '"'))
        {
            ++pos;
        }
        else if(quoteChar != '\0' && str[pos] == '\\' && pos + 1 < length && str[pos + 1] == quoteChar)
        {
            ++pos;
        }
        else if(quoteChar != '\0' && str[pos] == quoteChar)
        {
            ++pos;
            quoteChar = '\0';
            continue; // Skip the end quote
        }
        else if(delim.find(str[pos]) != string::npos)
        {
            if(quoteChar == '\0')
            {
                ++pos;
                if(elt.length() > 0)
                {
                    result.push_back(elt);
                    elt = "";
                }
                continue;
            }
        }
            
        if(pos < length)
        {
            elt += str[pos++];
        }
    }

    if(elt.length() > 0)
    {
        result.push_back(elt);
    }
    if(quoteChar != '\0')
    {
        return false; // Unmatched quote.
    }
    return true;
}

string
IceUtilInternal::joinString(const std::vector<std::string>& values, const std::string& delimiter)
{
    ostringstream out;
    for(unsigned int i = 0; i < values.size(); i++)
    {
        if(i != 0)
        {
            out << delimiter;
        }
        out << values[i];
    }
    return out.str();
}

//
// Trim white space (" \t\r\n")
//
string
IceUtilInternal::trim(const string& s)
{
    static const string delim = " \t\r\n";
    string::size_type beg = s.find_first_not_of(delim);
    if(beg == string::npos)
    {
        return "";
    }
    else
    {
        return s.substr(beg, s.find_last_not_of(delim) - beg + 1);
    }
}

//
// If a single or double quotation mark is found at the start position,
// then the position of the matching closing quote is returned. If no
// quotation mark is found at the start position, then 0 is returned.
// If no matching closing quote is found, then -1 is returned.
//
string::size_type
IceUtilInternal::checkQuote(const string& s, string::size_type start)
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
// as a wildcard: it matches any non-empty sequence of characters.
// We match by hand here because it's portable across platforms 
// (whereas regex() isn't). Only one * per pattern is supported.
//
bool
IceUtilInternal::match(const string& s, const string& pat, bool emptyMatch)
{
    assert(!s.empty());
    assert(!pat.empty());

    //
    // If pattern does not contain a wildcard just compare strings.
    //
    string::size_type beginIndex = pat.find('*');
    if(beginIndex == string::npos)
    {
        return s == pat;
    }

    //
    // Make sure start of the strings match
    //
    if(beginIndex > s.length() || s.substr(0, beginIndex) != pat.substr(0, beginIndex))
    {
        return false;
    }

    //
    // Make sure there is something present in the middle to match the
    // wildcard. If emptyMatch is true, allow a match of "".
    //
    string::size_type endLength = pat.length() - beginIndex - 1;
    if(endLength > s.length())
    {
        return false;
    }
    string::size_type endIndex = s.length() - endLength;
    if(endIndex < beginIndex || (!emptyMatch && endIndex == beginIndex))
    {
        return false;
    }

    //
    // Make sure end of the strings match
    //
    if(s.substr(endIndex, s.length()) != pat.substr(beginIndex + 1, pat.length()))
    {
        return false;
    }

    return true;
}

#ifdef _WIN32

string
IceUtilInternal::errorToString(int error, LPCVOID source)
{
    if(error < WSABASEERR)
    {
#ifdef ICE_OS_WINRT

        int size = 256;
        IceUtil::ScopedArray<wchar_t> lpMsgBuf(new wchar_t[size]);

        DWORD stored = 0;
        
        while(stored == 0)
        {
            stored = FormatMessageW(
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS |
                (source != NULL ? FORMAT_MESSAGE_FROM_HMODULE : 0),
                source,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                lpMsgBuf.get(),
                size,
                NULL);

            if(stored == 0)
            {
                DWORD err = GetLastError();
                if(err == ERROR_INSUFFICIENT_BUFFER)
                {
                    if(size == 65536)
                    {
                        break; // already at the max size
                    }
                    else
                    {
                        size *= 4;
                        size = max(size, 65536);
                        lpMsgBuf.reset(new wchar_t[size]);
                    }
                }
                else
                {
                    break;
                }   
            }
        }

        LPWSTR msg = lpMsgBuf.get();

#else
        LPWSTR msg = 0;
 
        DWORD stored = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            (source != NULL ? FORMAT_MESSAGE_FROM_HMODULE : 0),
            source,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            reinterpret_cast<LPWSTR>(&msg),
            0,
            NULL);
#endif
        
        if(stored > 0)
        {
            assert(msg && wcslen(msg) > 0);
            wstring result = msg;
            if(result[result.length() - 1] == L'\n')
            {
                result = result.substr(0, result.length() - 2);
            }
#ifndef ICE_OS_WINRT
            if(msg)
            {
                LocalFree(msg);
            }
#endif
            return wstringToString(result, getProcessStringConverter(), getProcessWstringConverter());
        }
        else
        {
#ifndef ICE_OS_WINRT
            if(msg)
            {
                LocalFree(msg);
            }
#endif
            ostringstream os;
            os << "unknown error: " << error;
            return os.str();
        }
    }
    
    switch(error)
    {
    case WSAEINTR:
        return "WSAEINTR";
        
    case WSAEBADF:
        return "WSAEBADF";
        
    case WSAEACCES:
        return "WSAEACCES";
        
    case WSAEFAULT:
        return "WSAEFAULT";
        
    case WSAEINVAL:
        return "WSAEINVAL";
        
    case WSAEMFILE:
        return "WSAEMFILE";
        
    case WSAEWOULDBLOCK:
        return "WSAEWOULDBLOCK";
        
    case WSAEINPROGRESS:
        return "WSAEINPROGRESS";
        
    case WSAEALREADY:
        return "WSAEALREADY";
        
    case WSAENOTSOCK:
        return "WSAENOTSOCK";
        
    case WSAEDESTADDRREQ:
        return "WSAEDESTADDRREQ";
        
    case WSAEMSGSIZE:
        return "WSAEMSGSIZE";
        
    case WSAEPROTOTYPE:
        return "WSAEPROTOTYPE";
        
    case WSAENOPROTOOPT:
        return "WSAENOPROTOOPT";
        
    case WSAEPROTONOSUPPORT:
        return "WSAEPROTONOSUPPORT";
        
    case WSAESOCKTNOSUPPORT:
        return "WSAESOCKTNOSUPPORT";
        
    case WSAEOPNOTSUPP:
        return "WSAEOPNOTSUPP";
        
    case WSAEPFNOSUPPORT:
        return "WSAEPFNOSUPPORT";
        
    case WSAEAFNOSUPPORT:
        return "WSAEAFNOSUPPORT";
        
    case WSAEADDRINUSE:
        return "WSAEADDRINUSE";
        
    case WSAEADDRNOTAVAIL:
        return "WSAEADDRNOTAVAIL";
        
    case WSAENETDOWN:
        return "WSAENETDOWN";
        
    case WSAENETUNREACH:
        return "WSAENETUNREACH";
        
    case WSAENETRESET:
        return "WSAENETRESET";
        
    case WSAECONNABORTED:
        return "WSAECONNABORTED";
        
    case WSAECONNRESET:
        return "WSAECONNRESET";
        
    case WSAENOBUFS:
        return "WSAENOBUFS";
        
    case WSAEISCONN:
        return "WSAEISCONN";
        
    case WSAENOTCONN:
        return "WSAENOTCONN";
        
    case WSAESHUTDOWN:
        return "WSAESHUTDOWN";
        
    case WSAETOOMANYREFS:
        return "WSAETOOMANYREFS";
        
    case WSAETIMEDOUT:
        return "WSAETIMEDOUT";
        
    case WSAECONNREFUSED:
        return "WSAECONNREFUSED";
        
    case WSAELOOP:
        return "WSAELOOP";
        
    case WSAENAMETOOLONG:
        return "WSAENAMETOOLONG";
        
    case WSAEHOSTDOWN:
        return "WSAEHOSTDOWN";
        
    case WSAEHOSTUNREACH:
        return "WSAEHOSTUNREACH";
        
    case WSAENOTEMPTY:
        return "WSAENOTEMPTY";
        
    case WSAEPROCLIM:
        return "WSAEPROCLIM";
        
    case WSAEUSERS:
        return "WSAEUSERS";
        
    case WSAEDQUOT:
        return "WSAEDQUOT";
        
    case WSAESTALE:
        return "WSAESTALE";
        
    case WSAEREMOTE:
        return "WSAEREMOTE";
        
    case WSAEDISCON:
        return "WSAEDISCON";
        
    case WSASYSNOTREADY:
        return "WSASYSNOTREADY";
        
    case WSAVERNOTSUPPORTED:
        return "WSAVERNOTSUPPORTED";
        
    case WSANOTINITIALISED:
        return "WSANOTINITIALISED";
        
    case WSAHOST_NOT_FOUND:
        return "WSAHOST_NOT_FOUND";
        
    case WSATRY_AGAIN:
        return "WSATRY_AGAIN";
        
    case WSANO_RECOVERY:
        return "WSANO_RECOVERY";
        
    case WSANO_DATA:
        return "WSANO_DATA";

    default:
    {
        ostringstream os;
        os << "unknown socket error: " << error;
        return os.str();
    }
    }
}

string
IceUtilInternal::lastErrorToString()
{
    return errorToString(GetLastError());
}

#else

string
IceUtilInternal::errorToString(int error)
{
    return strerror(error);
}

string
IceUtilInternal::lastErrorToString()
{
    return errorToString(errno);
}

#endif

string
IceUtilInternal::toLower(const std::string& s)
{
    string result;
    result.reserve(s.size());
    for(unsigned int i = 0; i < s.length(); ++i)
    {
        if(isascii(s[i]))
        {
            result += tolower(static_cast<unsigned char>(s[i]));
        }
        else
        {
            result += s[i];
        }
    }
    return result;
}

string
IceUtilInternal::toUpper(const std::string& s)
{
    string result;
    result.reserve(s.size());
    for(unsigned int i = 0; i < s.length(); ++i)
    {
        if(isascii(s[i]))
        {
            result += toupper(static_cast<unsigned char>(s[i]));
        }
        else
        {
            result += s[i];
        }
    }
    return result;
}

bool
IceUtilInternal::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool
IceUtilInternal::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

string
IceUtilInternal::removeWhitespace(const std::string& s)
{
    string result;
    for(unsigned int i = 0; i < s.length(); ++ i)
    {
         if(!isspace(static_cast<unsigned char>(s[i])))
         {
             result += s[i];
         }
    }
    return result;
}
