// Copyright (c) ZeroC, Inc.

#include "Base64.h"

#include <cstddef>
#include <iterator>

using namespace std;

string
IceInternal::Base64::encode(const vector<byte>& plainSeq)
{
    string retval;

    if (plainSeq.size() == 0)
    {
        return retval;
    }

    // Reserve enough space for the returned base64 string
    size_t base64Bytes = (((plainSeq.size() * 4) / 3) + 1);
    size_t newlineBytes = (((base64Bytes * 2) / 76) + 1);
    size_t totalBytes = base64Bytes + newlineBytes;

    retval.reserve(totalBytes);

    byte by1 = byte{0};
    byte by2 = byte{0};
    byte by3 = byte{0};
    byte by4 = byte{0};
    byte by5 = byte{0};
    byte by6 = byte{0};
    byte by7 = byte{0};

    for (size_t i = 0; i < plainSeq.size(); i += 3)
    {
        by1 = plainSeq[i];
        by2 = byte{0};
        by3 = byte{0};

        if ((i + 1) < plainSeq.size())
        {
            by2 = plainSeq[i + 1];
        }

        if ((i + 2) < plainSeq.size())
        {
            by3 = plainSeq[i + 2];
        }

        by4 = by1 >> 2;
        by5 = (by1 & byte{0x3}) << 4 | (by2 >> 4);
        by6 = (by2 & byte{0xf}) << 2 | (by3 >> 6);
        by7 = by3 & byte{0x3f};

        retval += encode(by4);
        retval += encode(by5);

        if ((i + 1) < plainSeq.size())
        {
            retval += encode(by6);
        }
        else
        {
            retval += "=";
        }

        if ((i + 2) < plainSeq.size())
        {
            retval += encode(by7);
        }
        else
        {
            retval += "=";
        }
    }

    string outString;
    outString.reserve(totalBytes);
    string::iterator iter = retval.begin();

    while ((retval.end() - iter) > 76)
    {
        copy(iter, iter + 76, back_inserter(outString));
        outString += "\r\n";
        iter += 76;
    }

    copy(iter, retval.end(), back_inserter(outString));

    return outString;
}

vector<byte>
IceInternal::Base64::decode(const string& str)
{
    string newStr;

    newStr.reserve(str.length());

    for (size_t j = 0; j < str.length(); j++)
    {
        if (isBase64(str[j]))
        {
            newStr += str[j];
        }
    }

    vector<byte> retval;

    if (newStr.length() == 0)
    {
        return retval;
    }

    // Note: This is how we were previously computing the size of the return
    //       sequence.  The method below is more efficient (and correct).
    // size_t lines = str.size() / 78;
    // size_t totalBytes = (lines * 76) + (((str.size() - (lines * 78)) * 3) / 4);

    // Figure out how long the final sequence is going to be.
    size_t totalBytes = (newStr.size() * 3 / 4) + 1;

    retval.reserve(totalBytes);

    byte by1{0};
    byte by2{0};
    byte by3{0};
    byte by4{0};

    char c1, c2, c3, c4;

    for (size_t i = 0; i < newStr.length(); i += 4)
    {
        c2 = 'A';
        c3 = 'A';
        c4 = 'A';

        c1 = newStr[i];

        if ((i + 1) < newStr.length())
        {
            c2 = newStr[i + 1];
        }

        if ((i + 2) < newStr.length())
        {
            c3 = newStr[i + 2];
        }

        if ((i + 3) < newStr.length())
        {
            c4 = newStr[i + 3];
        }

        by1 = decode(c1);
        by2 = decode(c2);
        by3 = decode(c3);
        by4 = decode(c4);

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
    if (c >= 'A' && c <= 'Z')
    {
        return true;
    }

    if (c >= 'a' && c <= 'z')
    {
        return true;
    }

    if (c >= '0' && c <= '9')
    {
        return true;
    }

    if (c == '+')
    {
        return true;
    }

    if (c == '/')
    {
        return true;
    }

    if (c == '=')
    {
        return true;
    }

    return false;
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
