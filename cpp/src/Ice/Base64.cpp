// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Base64.h>
#include <iterator>

using namespace std;

string
IceInternal::Base64::encode(const vector<unsigned char>& plainSeq)
{
    string retval;

    if(plainSeq.size() == 0) 
    {
        return retval;
    }
 
    // Reserve enough space for the returned base64 string
    size_t base64Bytes = (((plainSeq.size() * 4) / 3) + 1);
    size_t newlineBytes = (((base64Bytes * 2) / 76) + 1);
    size_t totalBytes = base64Bytes + newlineBytes;
 
    retval.reserve(totalBytes);

    unsigned char by1 = 0;
    unsigned char by2 = 0;
    unsigned char by3 = 0;
    unsigned char by4 = 0;
    unsigned char by5 = 0;
    unsigned char by6 = 0;
    unsigned char by7 = 0;

    for(size_t i = 0; i < plainSeq.size(); i += 3)
    {
        by1 = plainSeq[i];
        by2 = 0;
        by3 = 0;

        if((i + 1) < plainSeq.size())
        {
            by2 = plainSeq[i+1];
        }

        if((i + 2) < plainSeq.size())
        {
            by3 = plainSeq[i+2];
        }
 
        by4 = by1 >> 2;
        by5 = ((by1 & 0x3) << 4) | (by2 >> 4);
        by6 = ((by2 & 0xf) << 2) | (by3 >> 6);
        by7 = by3 & 0x3f;

        retval += encode(by4);
        retval += encode(by5);
 
        if((i + 1) < plainSeq.size())
        {
            retval += encode(by6);
        }
        else
        {
            retval += "=";
        }
 
        if((i + 2) < plainSeq.size())
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

    while((retval.end() - iter) > 76)
    {
        copy(iter, iter+76, back_inserter(outString));
        outString += "\r\n";
        iter += 76;
    }

    copy(iter, retval.end(), back_inserter(outString));

    return outString;
}

vector<unsigned char>
IceInternal::Base64::decode(const string& str)
{
    string newStr;

    newStr.reserve(str.length());

    for(size_t j = 0; j < str.length(); j++)
    {
        if(isBase64(str[j]))
        {
            newStr += str[j];
        }
    }

    vector<unsigned char> retval;

    if(newStr.length() == 0)
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

    unsigned char by1 = 0;
    unsigned char by2 = 0;
    unsigned char by3 = 0;
    unsigned char by4 = 0;

    char c1, c2, c3, c4;

    for(size_t i = 0; i < newStr.length(); i += 4)
    {
        c1 = 'A';
        c2 = 'A';
        c3 = 'A';
        c4 = 'A';

        c1 = newStr[i];

        if((i + 1) < newStr.length())
        {
            c2 = newStr[i + 1];
        }

        if((i + 2) < newStr.length())
        {
            c3 = newStr[i + 2];
        }

        if((i + 3) < newStr.length())
        {
            c4 = newStr[i + 3];
        }

        by1 = decode(c1);
        by2 = decode(c2);
        by3 = decode(c3);
        by4 = decode(c4);

        retval.push_back((by1 << 2) | (by2 >> 4));

        if(c3 != '=')
        {
            retval.push_back(((by2 & 0xf) << 4) | (by3 >> 2));
        }

        if(c4 != '=')
        {
            retval.push_back(((by3 & 0x3) << 6) | by4);
        }
    }

    return retval;
}

bool
IceInternal::Base64::isBase64(char c)
{
    if(c >= 'A' && c <= 'Z')
    {
        return true;
    }

    if(c >= 'a' && c <= 'z')
    {
        return true;
    }

    if(c >= '0' && c <= '9')
    {
        return true;
    }

    if(c == '+')
    {
        return true;
    }

    if(c == '/')
    {
        return true;
    }

    if(c == '=')
    {
        return true;
    }

    return false;
}

char
IceInternal::Base64::encode(unsigned char uc)
{
    if(uc < 26)
    {
        return 'A' + uc;
    }
    
    if(uc < 52)
    {
        return 'a' + (uc - 26);
    }
    
    if(uc < 62)
    {
        return '0' + (uc - 52);
    }
    
    if(uc == 62)
    {
        return '+';
    }

    return '/';
}

unsigned char
IceInternal::Base64::decode(char c)
{
    if(c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }

    if(c >= 'a' && c <= 'z')
    {
        return c - 'a' + 26;
    }

    if(c >= '0' && c <= '9')
    {
        return c - '0' + 52;
    }

    if(c == '+')
    {
        return 62;
    }
 
    return 63;
}
