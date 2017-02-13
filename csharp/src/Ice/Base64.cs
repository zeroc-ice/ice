// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceUtilInternal
{

public class Base64
{

public static string
encode(byte[] plainSeq)
{
    if(plainSeq == null || plainSeq.Length == 0) 
    {
        return "";
    }

    System.Text.StringBuilder retval = new System.Text.StringBuilder();
    int base64Bytes = (((plainSeq.Length * 4) / 3) + 1);
    int newlineBytes = (((base64Bytes * 2) / 76) + 1);
    int totalBytes = base64Bytes + newlineBytes;

    retval.Capacity = totalBytes;
 
    byte by1;
    byte by2;
    byte by3;
    byte by4;
    byte by5;
    byte by6;
    byte by7;

    for(int i = 0; i < plainSeq.Length; i += 3)
    {
        by1 = plainSeq[i];
        by2 = 0;
        by3 = 0;

        if((i + 1) < plainSeq.Length)
        {
            by2 = plainSeq[i+1];
        }

        if((i + 2) < plainSeq.Length)
        {
            by3 = plainSeq[i+2];
        }
 
        by4 = (byte)(by1 >> 2);
        by5 = (byte)(((by1 & 0x3) << 4) | (by2 >> 4));
        by6 = (byte)(((by2 & 0xf) << 2) | (by3 >> 6));
        by7 = (byte)(by3 & 0x3f);

        retval.Append(encode(by4));
        retval.Append(encode(by5));
 
        if((i + 1) < plainSeq.Length)
        {
            retval.Append(encode(by6));
        }
        else
        {
            retval.Append('=');
        }
 
        if((i + 2) < plainSeq.Length)
        {
            retval.Append(encode(by7));
        }
        else
        {
            retval.Append('=');
        }
    }

    System.Text.StringBuilder outString = new System.Text.StringBuilder();
    outString.Capacity = totalBytes;
    int iter = 0;

    while((retval.Length - iter) > 76)
    {
        outString.Append(retval.ToString().Substring(iter, 76));
        outString.Append("\r\n");
        iter += 76;
    }

    outString.Append(retval.ToString().Substring(iter));

    return outString.ToString();
}

public static byte[]
decode(string str)
{
    System.Text.StringBuilder newStr = new System.Text.StringBuilder();

    newStr.Capacity = str.Length;

    for(int j = 0; j < str.Length; j++)
    {
        char c = str[j];
        if(isBase64(c))
        {
            newStr.Append(c);
        }
    }

    if(newStr.Length == 0)
    {
        return null;
    }

    // Note: This is how we were previously computing the size of the return
    //       sequence.  The method below is more efficient (and correct).
    // size_t lines = str.size() / 78;
    // size_t totalBytes = (lines * 76) + (((str.size() - (lines * 78)) * 3) / 4);

    // Figure out how long the final sequence is going to be.
    int totalBytes = (newStr.Length * 3 / 4) + 1;

    IceInternal.ByteBuffer retval = IceInternal.ByteBuffer.allocate(totalBytes);

    byte by1;
    byte by2;
    byte by3;
    byte by4;

    char c1, c2, c3, c4;

    int pos = 0;
    for(int i = 0; i < newStr.Length; i += 4)
    {
        c1 = 'A';
        c2 = 'A';
        c3 = 'A';
        c4 = 'A';

        c1 = newStr[i];

        if((i + 1) < newStr.Length)
        {
            c2 = newStr[i + 1];
        }

        if((i + 2) < newStr.Length)
        {
            c3 = newStr[i + 2];
        }

        if((i + 3) < newStr.Length)
        {
            c4 = newStr[i + 3];
        }

        by1 = decode(c1);
        by2 = decode(c2);
        by3 = decode(c3);
        by4 = decode(c4);

        retval.put((byte)((by1 << 2) | (by2 >> 4)));
        ++pos;

        if(c3 != '=')
        {
            retval.put((byte)(((by2 & 0xf) << 4) | (by3 >> 2)));
            ++pos;
        }

        if(c4 != '=')
        {
            retval.put((byte)(((by3 & 0x3) << 6) | by4));
            ++pos;
        }
    }

    return retval.toArray(0, pos);
}

public static bool
isBase64(char c)
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

private static char
encode(byte uc)
{
    if(uc < 26)
    {
        return (char)('A' + uc);
    }
    
    if(uc < 52)
    {
        return (char)('a' + (uc - 26));
    }
    
    if(uc < 62)
    {
        return (char)('0' + (uc - 52));
    }
    
    if(uc == 62)
    {
        return '+';
    }

    return '/';
}

private static byte
decode(char c)
{
    if(c >= 'A' && c <= 'Z')
    {
        return (byte)(c - 'A');
    }

    if(c >= 'a' && c <= 'z')
    {
        return (byte)(c - 'a' + 26);
    }

    if(c >= '0' && c <= '9')
    {
        return (byte)(c - '0' + 52);
    }

    if(c == '+')
    {
        return 62;
    }
 
    return 63;
}

}

}

