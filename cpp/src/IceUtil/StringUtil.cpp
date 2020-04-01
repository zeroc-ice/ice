//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/StringUtil.h>
#include <IceUtil/StringConverter.h>
#include <cstring>
#include <string.h> // for strerror_r

#include <sstream>
#include <iomanip>

using namespace std;
using namespace IceUtil;

namespace
{

char
toHexDigit(Byte b)
{
    assert(b < 16);
    if(b < 10)
    {
        return static_cast<char>('0' + b);
    }
    else
    {
        return static_cast<char>('a' - 10 + b);
    }
}

unsigned int
addContinuationByte(string::iterator& p, string::iterator end, unsigned int codePoint)
{
    if(p == end)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "UTF-8 sequence too short");
    }

    Byte b = static_cast<Byte>(*p++);

    if((b >> 6) != 2)
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "Invalid UTF-8 sequence");
    }
    return (codePoint << 6) + (b & 0x3F);
}

//
// Appends a 2 to 4 bytes UTF-8 sequence as a universal character name
//
void
appendUniversalName(char c, string::iterator& p, string::iterator end, string& result)
{
    unsigned int codePoint;

    Byte b = static_cast<Byte>(c);
    if((b >> 5) == 0x06)
    {
        // 2 bytes
        codePoint = (b & 0x1F);
        codePoint = addContinuationByte(p, end, codePoint);
    }
    else if((b >> 4) == 0x0E)
    {
        // 3 bytes
        codePoint = (b & 0x0F);
        codePoint = addContinuationByte(p, end, codePoint);
        codePoint = addContinuationByte(p, end, codePoint);
    }
    else if((b >> 3) == 0x1E)
    {
        // 4 bytes
        codePoint = (b & 0x07);
        codePoint = addContinuationByte(p, end, codePoint);
        codePoint = addContinuationByte(p, end, codePoint);
        codePoint = addContinuationByte(p, end, codePoint);
    }
    else
    {
        ostringstream ostr;
        ostr <<  "Invalid first byte 0x" << hex << static_cast<unsigned short>(b) << " in UTF-8 sequence" << endl;
        throw IllegalArgumentException(__FILE__, __LINE__,  ostr.str());
    }

    if(codePoint > 0xFFFF)
    {
        result.append("\\U");
        for(int j = 7; j >= 0; j--)
        {
            result.push_back(toHexDigit(static_cast<Byte>((codePoint >> (j * 4)) & 0x0F)));
        }
    }
    else
    {
        result.append("\\u");
        for(int j = 3; j >= 0; j--)
        {
            result.push_back(toHexDigit(static_cast<Byte>((codePoint >> (j * 4)) & 0x0F)));
        }
    }
}

}

//
// Add escape sequences. Any characters that appear in special are prefixed with a backslash in the returned string.
//
string
IceUtilInternal::escapeString(const string& s, const string& special, ToStringMode toStringMode)
{
    for(string::size_type i = 0; i < special.size(); ++i)
    {
        if(static_cast<unsigned char>(special[i]) < 32 || static_cast<unsigned char>(special[i]) > 126)
        {
            throw IllegalArgumentException(__FILE__, __LINE__, "Special characters must be in ASCII range 32-126");
        }
    }

    //
    // First convert to UTF-8
    //
    string u8s = nativeToUTF8(s, getProcessStringConverter());

    string::iterator p = u8s.begin();

    string result;

    while(p != u8s.end())
    {
        char c = *p++;

        switch(c)
        {
            case '\\':
            {
                result.append("\\\\");
                break;
            }
            case '\'':
            {
                result.append("\\'");
                break;
            }
            case '"':
            {
                result.append("\\\"");
                break;
            }
            case '\a':
            {
                if(toStringMode == ToStringMode::Compat)
                {
                    // Octal escape for compatibility with 3.6 and earlier
                    result.append("\\007");
                }
                else
                {
                    result.append("\\a");
                }
                break;
            }
            case '\b':
            {
                result.append("\\b");
                break;
            }
            case '\f':
            {
                result.append("\\f");
                break;
            }
            case '\n':
            {
                result.append("\\n");
                break;
            }
            case '\r':
            {
                result.append("\\r");
                break;
            }
            case '\t':
            {
                result.append("\\t");
                break;
            }
            case '\v':
            {
                if(toStringMode == ToStringMode::Compat)
                {
                    // Octal escape for compatibility with 3.6 and earlier
                    result.append("\\013");
                }
                else
                {
                    result.append("\\v");
                }
                break;
            }
            default:
            {
                if(special.find(c) != string::npos)
                {
                    result.push_back('\\');
                    result.push_back(c);
                }
                else
                {
                    unsigned char i = static_cast<unsigned char>(c);

                    if(i < 32 || i > 126)
                    {
                        if(toStringMode == ToStringMode::Compat)
                        {
                            // append octal string

                            // Add leading zeroes so that we avoid problems during
                            // decoding. For example, consider the escaped string
                            // \0013 (i.e., a character with value 1 followed by the
                            // character '3'). If the leading zeroes were omitted, the
                            // result would be incorrectly interpreted as a single
                            // character with value 11.
                            //
                            ostringstream os;
                            os << '\\' << oct << setfill('0') << setw(3) << static_cast<unsigned int>(i);
                            result.append(os.str());
                        }
                        else if(i < 32 || i == 127)
                        {
                            // append \u00nn
                            result.append("\\u00");
                            result.push_back(toHexDigit(i >> 4));
                            result.push_back(toHexDigit(i & 0x0F));
                        }
                        else if(toStringMode == ToStringMode::ASCII)
                        {
                            // append \unnnn or \Unnnnnnnn after reading more UTF-8 bytes
                            appendUniversalName(c, p, u8s.end(), result);
                        }
                        else
                        {
                            // keep as is
                            result.push_back(c);
                        }
                    }
                    else
                    {
                        // printable ASCII character
                        result.push_back(c);
                    }
                }
                break;
            }
        }
    }

    if(toStringMode == ToStringMode::Unicode)
    {
        //
        // Convert back to Native
        //
        result = UTF8ToNative(result, getProcessStringConverter());
    }
    // else it's a pure ASCII string

    return result;
}

namespace
{

char
checkChar(const string& s, string::size_type pos)
{
    unsigned char c = static_cast<unsigned char>(s[pos]);
    if(c < 32 || c == 127)
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
        ostr << " has invalid ordinal value " << static_cast<int>(c);
        throw IllegalArgumentException(__FILE__, __LINE__, ostr.str());
    }
    return static_cast<char>(c);
}

//
// Append codePoint as a UTF-8 sequence
//
void
appendUTF8(unsigned int codePoint, string& result)
{
    if(codePoint >= 0xD800 && codePoint <= 0xDFFF)
    {
        throw IllegalArgumentException(__FILE__, __LINE__,
                                       "A universal character name cannot designate a surrogate");
    }

    if(codePoint <= 0x7F)
    {
        // ASCII
        result.push_back(static_cast<char>(codePoint));
    }
    else if(codePoint <= 0x7FF)
    {
        // 2 bytes
        result.push_back(static_cast<char>((codePoint >> 6) | 0xC0));
        result.push_back(static_cast<char>((codePoint & 0x3F) | 0x80));
    }
    else if(codePoint <= 0xFFFF)
    {
        // 3 bytes
        result.push_back(static_cast<char>((codePoint >> 12) | 0xE0));
        result.push_back(static_cast<char>(((codePoint >> 6) & 0x3F) | 0x80));
        result.push_back(static_cast<char>((codePoint & 0x3F) | 0x80));
    }
    else if(codePoint <= 0x10FFFF)
    {
        // 4 bytes
        result.push_back(static_cast<char>((codePoint >> 18) | 0xF0));
        result.push_back(static_cast<char>(((codePoint >> 12) & 0x3F) | 0x80));
        result.push_back(static_cast<char>(((codePoint >> 6) & 0x3F) | 0x80));
        result.push_back(static_cast<char>((codePoint & 0x3F) | 0x80));
    }
    else
    {
        throw IllegalArgumentException(__FILE__, __LINE__, "Invalid universal character name");
    }
}

//
// Decode the character or escape sequence starting at start and appends it to result;
// end marks the one-past-the-end position of the substring to be scanned.
// nextStart is set to the index of the first character following the decoded
// character or escape sequence.
//
bool
decodeChar(const string& s, string::size_type start, string::size_type end, string::size_type& nextStart,
           const string& special, string& result)
{
    assert(start < end);
    assert(end <= s.size());

    bool pureASCII = true;

    if(s[start] != '\\')
    {
        result.push_back(checkChar(s, start++));
    }
    else if(start + 1 == end)
    {
        // Keep trailing backslash
        ++start;
        result.push_back('\\');
    }
    else
    {
        char c = s[++start];

        switch(c)
        {
            case '\\':
            case '\'':
            case '"':
            case '?':
            {
                ++start;
                result.push_back(c);
                break;
            }
            case 'a':
            {
                ++start;
                result.push_back('\a');
                break;
            }
            case 'b':
            {
                ++start;
                result.push_back('\b');
                break;
            }
            case 'f':
            {
                ++start;
                result.push_back('\f');
                break;
            }
            case 'n':
            {
                ++start;
                result.push_back('\n');
                break;
            }
            case 'r':
            {
                ++start;
                result.push_back('\r');
                break;
            }
            case 't':
            {
                ++start;
                result.push_back('\t');
                break;
            }
            case 'v':
            {
                ++start;
                result.push_back('\v');
                break;
            }
            case 'u':
            case 'U':
            {
                unsigned int codePoint = 0;
                bool inBMP = (c == 'u');
                int size = inBMP ? 4 : 8;
                ++start;
                while(size > 0 && start < end)
                {
                    c = s[start++];
                    int charVal = 0;
                    if(c >= '0' && c <= '9')
                    {
                        charVal = c - '0';
                    }
                    else if(c >= 'a' && c <= 'f')
                    {
                        charVal = 10 + (c - 'a');
                    }
                    else if(c >= 'A' && c <= 'F')
                    {
                        charVal = 10 + (c - 'A');
                    }
                    else
                    {
                        break; // while
                    }
                    codePoint = codePoint * 16 + static_cast<unsigned int>(charVal);
                    --size;
                }
                if(size > 0)
                {
                    throw IllegalArgumentException(__FILE__, __LINE__,
                                                   "Invalid universal character name: too few hex digits");
                }

                appendUTF8(codePoint, result);
                if(codePoint > 127)
                {
                    pureASCII = false;
                }
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
                result.push_back(static_cast<char>(val));
                if(val > 127)
                {
                    pureASCII = false;
                }
                break;
            }
            case 'x':
            {
                int val = 0;
                int size = 2;
                ++start;
                while(size > 0 && start < end)
                {
                    c = s[start++];
                    int charVal = 0;
                    if(c >= '0' && c <= '9')
                    {
                        charVal = c - '0';
                    }
                    else if(c >= 'a' && c <= 'f')
                    {
                        charVal = 10 + (c - 'a');
                    }
                    else if(c >= 'A' && c <= 'F')
                    {
                        charVal = 10 + (c - 'A');
                    }
                    else
                    {
                        --start; // move back
                        break; // while
                    }
                    val = val * 16 + charVal;
                    --size;
                }
                if(size == 2)
                {
                    throw IllegalArgumentException(__FILE__, __LINE__,
                                                   "Invalid \\x escape sequence: no hex digit");
                }
                result.push_back(static_cast<char>(val));
                if(val > 127)
                {
                    pureASCII = false;
                }
                break;
            }
            default:
            {
                if(static_cast<unsigned char>(c) > 127)
                {
                    pureASCII = false;
                }
                if(special.empty() || special.find(c) == string::npos)
                {
                    result.push_back('\\'); // not in special, so we keep the backslash
                }
                result.push_back(checkChar(s, start++));
                break;
            }
        }
    }
    nextStart = start;
    return pureASCII;
}

}

//
// Remove escape sequences added by escapeString.
//
string
IceUtilInternal::unescapeString(const string& s, string::size_type start, string::size_type end, const string& special)
{
    assert(start <= end && end <= s.size());

    for(string::size_type i = 0; i < special.size(); ++i)
    {
        if(static_cast<unsigned char>(special[i]) < 32 || static_cast<unsigned char>(special[i]) > 126)
        {
            throw IllegalArgumentException(__FILE__, __LINE__, "Special characters must be in ASCII range 32-126");
        }
    }

    // Optimization for strings without escapes
    string::size_type p = s.find('\\', start);
    if(p == string::npos || p >= end)
    {
        p = start;
        while(p < end)
        {
            checkChar(s, p++);
        }
        return s.substr(start, end);
    }
    else
    {
        StringConverterPtr stringConverter = getProcessStringConverter();

        const string* inputStringPtr = &s;
        string u8s;

        if(stringConverter)
        {
            bool inputIsPureASCII = true;
            string::size_type i = start;
            while(i < end && inputIsPureASCII)
            {
                inputIsPureASCII = static_cast<unsigned char>(s[i++]) <= 127;
            }

            if(!inputIsPureASCII)
            {
                u8s = nativeToUTF8(s.substr(start, end), stringConverter);
                inputStringPtr = &u8s;
                start = 0;
                end = u8s.size();
            }
        }

        bool resultIsPureASCII = true;
        string result;
        result.reserve(end - start);
        while(start < end)
        {
            if(decodeChar(*inputStringPtr, start, end, start, special, result))
            {
                resultIsPureASCII = false;
            }
        }

        if(stringConverter && !resultIsPureASCII)
        {
            // Need to convert from UTF-8 to Native
            result = UTF8ToNative(result, stringConverter);
        }

        return result;
    }
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
        LPWSTR msg = 0;

        DWORD stored = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            (source != nullptr ? FORMAT_MESSAGE_FROM_HMODULE : 0),
            source,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            reinterpret_cast<LPWSTR>(&msg),
            0,
            nullptr);

        if(stored > 0)
        {
            assert(msg && wcslen(msg) > 0);
            wstring result = msg;
            if(result[result.length() - 1] == L'\n')
            {
                result = result.substr(0, result.length() - 2);
            }
            return wstringToString(result, getProcessStringConverter(), getProcessWstringConverter());
        }
        else
        {
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
    vector<char> buffer(500);
    while(true)
    {
#if !defined(__GLIBC__) || ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE)
        //
        // Use the XSI-compliant version of strerror_r
        //
        int err = strerror_r(error, &buffer[0], buffer.size());
        if(err == 0)
        {
            return string(&buffer[0]);
        }
#else
        //
        // Use the GNU-specific version of strerror_r
        //
        int oerrno = errno;
        errno = 0;
        const char* msg = strerror_r(error, &buffer[0], buffer.size());
        int err = errno;
        errno = oerrno;
        if(err == 0)
        {
            return msg;
        }
#endif
        if(err == ERANGE && buffer.size() < 1024 * 1024)
        {
            buffer.resize(buffer.size() * 2);
        }
        else
        {
            ostringstream os;
            os << "Unknown error `" << error << "'";
            return os.str();
        }
    }
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
            result += static_cast<char>(tolower(static_cast<unsigned char>(s[i])));
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
            result += static_cast<char>(toupper(static_cast<unsigned char>(s[i])));
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
