// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Base64.h>
#include <iostream>

using namespace std;

string
IceUtil::Base64::encode(const ByteSeq& plainSeq)
{
    string retval;

    if (plainSeq.size() == 0) 
    {
        return retval;
    }
 
    // Reserve enough space for the returned base64 string
    long base64Bytes = (((plainSeq.size() * 4L) / 3L) + 1L);
    long newlineBytes = (((base64Bytes * 2L) / 76L) + 1L);
    long totalBytes = base64Bytes + newlineBytes;
 
    retval.reserve(totalBytes);

    unsigned char by1 = 0;
    unsigned char by2 = 0;
    unsigned char by3 = 0;
    unsigned char by4 = 0;
    unsigned char by5 = 0;
    unsigned char by6 = 0;
    unsigned char by7 = 0;

    for (unsigned int i = 0; i < plainSeq.size(); i += 3)
    {
        by1 = plainSeq[i];
        by2 = 0;
        by3 = 0;

        if ((i + 1) < plainSeq.size())
        {
            by2 = plainSeq[i+1];
        }

        if ((i + 2) < plainSeq.size())
        {
            by3 = plainSeq[i+2];
        }
 
        by4 = by1 >> 2;
        by5 = ((by1 & 0x3) << 4) | (by2 >> 4);
        by6 = ((by2 & 0xf) << 2) | (by3 >> 6);
        by7 = by3 & 0x3f;

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
        copy(iter, iter+76, back_inserter(outString));
        outString += "\r\n";
        iter += 76;
    }

    copy(iter, retval.end(), back_inserter(outString));

    return outString;
}

IceUtil::ByteSeq
IceUtil::Base64::decode(const string& _str)
{
    string str;

    for (unsigned int j = 0; j < _str.length(); j++)
    {
        if (isBase64(_str[j]))
        {
            str += _str[j];
        }
    }

    ByteSeq retval;

    if (str.length() == 0)
    {
        return retval;
    }

    // Figure out how long the final sequence is going to be.
    long lines = _str.size() / 78;
    long totalBytes = (lines * 76) + (((_str.size() - (lines * 78)) * 3) / 4);

    retval.reserve(totalBytes);

    unsigned char by1 = 0;
    unsigned char by2 = 0;
    unsigned char by3 = 0;
    unsigned char by4 = 0;

    char c1, c2, c3, c4;

    for (unsigned int i = 0; i < str.length(); i += 4)
    {
        c1='A';
        c2='A';
        c3='A';
        c4='A';

        c1 = str[i];

        if ((i + 1) < str.length())
        {
            c2 = str[i+1];
        }

        if ((i + 2) < str.length())
        {
            c3 = str[i+2];
        }

        if ((i + 3) < str.length())
        {
            c4 = str[i+3];
        }

        by1 = decode(c1);
        by2 = decode(c2);
        by3 = decode(c3);
        by4 = decode(c4);

        retval.push_back((by1 << 2) | (by2 >> 4));

        if (c3 != '=')
        {
            retval.push_back(((by2 & 0xf) << 4) | (by3 >> 2));
        }

        if (c4 != '=')
        {
            retval.push_back(((by3 & 0x3) << 6) | by4);
        }
    }

    return retval;
}

char
IceUtil::Base64::encode(unsigned char uc)
{
    if (uc < 26)
    {
        return 'A'+uc;
    }
    
    if (uc < 52)
    {
        return 'a'+(uc-26);
    }
    
    if (uc < 62)
    {
        return '0'+(uc-52);
    }
    
    if (uc == 62)
    {
        return '+';
    }

    return '/';
}

unsigned char
IceUtil::Base64::decode(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }

    if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 26;
    }

    if (c >= '0' && c <= '9')
    {
        return c - '0' + 52;
    }

    if (c == '+')
    {
        return 62;
    }
 
    return 63;
}


bool
IceUtil::Base64::isBase64(char c)
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
