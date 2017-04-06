// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceUtilInternal;

//
// We would prefer to use java.util.Base64 but unfortunately that class isn't supported in Android until
// Android O, so we are using our own implementation.
//
public class Base64
{
    public static String encode(byte[] plainSeq)
    {
        if(plainSeq == null || plainSeq.length == 0)
        {
            return "";
        }

        int base64Bytes = (((plainSeq.length * 4) / 3) + 1);
        int newlineBytes = (((base64Bytes * 2) / 76) + 1);
        int totalBytes = base64Bytes + newlineBytes;

        StringBuilder retval = new StringBuilder(totalBytes);

        int by1;
        int by2;
        int by3;
        int by4;
        int by5;
        int by6;
        int by7;

        for(int i = 0; i < plainSeq.length; i += 3)
        {
            by1 = plainSeq[i] & 0xff;
            by2 = 0;
            by3 = 0;

            if((i + 1) < plainSeq.length)
            {
                by2 = plainSeq[i+1] & 0xff;
            }

            if((i + 2) < plainSeq.length)
            {
                by3 = plainSeq[i+2] & 0xff;
            }

            by4 = (by1 >> 2) & 0xff;
            by5 = (((by1 & 0x3) << 4) | (by2 >> 4)) & 0xff;
            by6 = (((by2 & 0xf) << 2) | (by3 >> 6)) & 0xff;
            by7 = by3 & 0x3f;

            retval.append(encode((byte)by4));
            retval.append(encode((byte)by5));

            if((i + 1) < plainSeq.length)
            {
                retval.append(encode((byte)by6));
            }
            else
            {
                retval.append('=');
            }

            if((i + 2) < plainSeq.length)
            {
                retval.append(encode((byte)by7));
            }
            else
            {
                retval.append('=');
            }
        }

        StringBuilder outString = new StringBuilder(totalBytes);
        int iter = 0;

        while((retval.length() - iter) > 76)
        {
            outString.append(retval.substring(iter, iter + 76));
            outString.append("\r\n");
            iter += 76;
        }

        outString.append(retval.substring(iter));

        return outString.toString();
    }

    public static byte[] decode(String str)
    {
        StringBuilder newStr = new StringBuilder(str.length());

        for(int j = 0; j < str.length(); j++)
        {
            char c = str.charAt(j);
            if(isBase64(c))
            {
                newStr.append(c);
            }
            else
            {
                throw new IllegalArgumentException("invalid base64 character `" + str.charAt(j) + "' (ordinal " +
                                                   ((int)str.charAt(j)) + ")");
            }
        }

        if(newStr.length() == 0)
        {
            return null;
        }

        // Note: This is how we were previously computing the size of the return
        //       sequence. The method below is more efficient (and correct).
        // size_t lines = str.size() / 78;
        // size_t totalBytes = (lines * 76) + (((str.size() - (lines * 78)) * 3) / 4);

        // Figure out how long the final sequence is going to be.
        int totalBytes = (newStr.length() * 3 / 4) + 1;

        java.nio.ByteBuffer retval = java.nio.ByteBuffer.allocate(totalBytes);

        int by1;
        int by2;
        int by3;
        int by4;

        char c1, c2, c3, c4;

        int pos = 0;
        for(int i = 0; i < newStr.length(); i += 4)
        {
            c1 = 'A';
            c2 = 'A';
            c3 = 'A';
            c4 = 'A';

            c1 = newStr.charAt(i);

            if((i + 1) < newStr.length())
            {
                c2 = newStr.charAt(i + 1);
            }

            if((i + 2) < newStr.length())
            {
                c3 = newStr.charAt(i + 2);
            }

            if((i + 3) < newStr.length())
            {
                c4 = newStr.charAt(i + 3);
            }

            by1 = decode(c1) & 0xff;
            by2 = decode(c2) & 0xff;
            by3 = decode(c3) & 0xff;
            by4 = decode(c4) & 0xff;

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

        byte[] arr = new byte[pos];
        System.arraycopy(retval.array(), 0, arr, 0, pos);
        return arr;
    }

    private static boolean isBase64(char c)
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

    private static char encode(byte uc)
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

    private static byte decode(char c)
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
