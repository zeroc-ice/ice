// Copyright (c) ZeroC, Inc.

#include "Base64.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iterator>
#include <sstream>

using namespace std;

string
IceInternal::Base64::encode(const vector<byte>& plainSeq)
{
    // Reserve the exact amount of space needed for the returned base64 string.
    string retval;
    retval.reserve(((plainSeq.size() + 2) / 3) * 4);

    for (size_t i = 0; i < plainSeq.size(); i += 3)
    {
        byte by1 = plainSeq[i];
        byte by2 = byte{0};
        byte by3 = byte{0};
        if ((i + 1) < plainSeq.size())
        {
            by2 = plainSeq[i + 1];
        }
        if ((i + 2) < plainSeq.size())
        {
            by3 = plainSeq[i + 2];
        }

        retval += encode(by1 >> 2);
        retval += encode((by1 & byte{0x3}) << 4 | (by2 >> 4));
        if ((i + 1) < plainSeq.size())
        {
            retval += encode((by2 & byte{0xf}) << 2 | (by3 >> 6));
        }
        else
        {
            retval += "=";
        }
        if ((i + 2) < plainSeq.size())
        {
            retval += encode(by3 & byte{0x3f});
        }
        else
        {
            retval += "=";
        }
    }

    return retval;
}

vector<byte>
IceInternal::Base64::decode(string str)
{
    // First, remove any whitespace from the string.
    auto it = std::remove_if(str.begin(), str.end(), [](unsigned char c) { return std::isspace(c); });
    str.erase(it, str.end());

    // Reject any non-base64 characters.
    auto paddingStart = str.begin() + static_cast<ptrdiff_t>(str.find_last_not_of('=') + 1);
    it = std::find_if_not(str.begin(), paddingStart, isBase64);
    if (it != paddingStart)
    {
        ostringstream os;
        os << "invalid base64 character '" << *it << "' (ordinal " << +static_cast<unsigned char>(*it) << ")";
        throw std::invalid_argument(os.str());
    }
    // Reject bad padding. There can be at most 2 padding characters, and it must make the total length a multiple of 4.
    if (paddingStart != str.end() && (str.end() - paddingStart > 2 || (str.size() % 4) != 0))
    {
        throw std::invalid_argument("invalid base64 padding");
    }
    // Drop any padding characters at this point.
    str.erase(paddingStart, str.end());

    // Reject any base64 strings with only 1 out of 4 characters in the final sequence.
    // The final sequence may have 2, 3, or 4 characters, but 1 can't encode a full byte.
    if (str.length() % 4 == 1)
    {
        throw std::invalid_argument("invalid base64 string length");
    }

    vector<byte> retval;
    retval.reserve((str.length() * 3) / 4);

    for (size_t i = 0; i < str.length(); i += 4)
    {
        const char c1 = str[i];
        const char c2 = str[i + 1]; // Guaranteed okay because of the 'str.length() % 4 == 1' check above.
        const char c3 = ((i + 2) < str.length()) ? str[i + 2] : '=';
        const char c4 = ((i + 3) < str.length()) ? str[i + 3] : '=';

        const byte by1 = decode(c1);
        const byte by2 = decode(c2);
        const byte by3 = decode(c3);
        const byte by4 = decode(c4);

        retval.push_back((by1 << 2) | by2 >> 4);
        if (c3 != '=')
        {
            retval.push_back(((by2 & byte{0xf}) << 4) | (by3 >> 2));
        }
        if (c4 != '=')
        {
            retval.push_back(((by3 & byte{0x3}) << 6) | by4);
        }
    }

    return retval;
}

bool
IceInternal::Base64::isBase64(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '/';
}

char
IceInternal::Base64::encode(byte uc)
{
    if (uc < byte{26})
    {
        return 'A' + static_cast<char>(uc);
    }

    if (uc < byte{52})
    {
        return 'a' + static_cast<char>(uc) - 26;
    }

    if (uc < byte{62})
    {
        return '0' + static_cast<char>(uc) - 52;
    }

    if (uc == byte{62})
    {
        return '+';
    }

    return '/';
}

byte
IceInternal::Base64::decode(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return static_cast<byte>(c - 'A');
    }

    if (c >= 'a' && c <= 'z')
    {
        return static_cast<byte>(c - 'a' + 26);
    }

    if (c >= '0' && c <= '9')
    {
        return static_cast<byte>(c - '0' + 52);
    }

    if (c == '+')
    {
        return byte{62};
    }

    return byte{63};
}
