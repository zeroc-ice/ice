// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public final class StringUtil
{
    //
    // Return the index of the first character in str to
    // appear in match, starting from 0. Returns -1 if none is
    // found.
    //
    public static int
    findFirstOf(String str, String match)
    {
        return findFirstOf(str, match, 0);
    }

    //
    // Return the index of the first character in str to
    // appear in match, starting from start. Returns -1 if none is
    // found.
    //
    public static int
    findFirstOf(String str, String match, int start)
    {
        final int len = str.length();
        for(int i = start; i < len; i++)
        {
            char ch = str.charAt(i);
            if(match.indexOf(ch) != -1)
            {
                return i;
            }
        }

        return -1;
    }

    //
    // Return the index of the first character in str which does
    // not appear in match, starting from 0. Returns -1 if none is
    // found.
    //
    public static int
    findFirstNotOf(String str, String match)
    {
        return findFirstNotOf(str, match, 0);
    }

    //
    // Return the index of the first character in str which does
    // not appear in match, starting from start. Returns -1 if none is
    // found.
    //
    public static int
    findFirstNotOf(String str, String match, int start)
    {
        final int len = str.length();
        for(int i = start; i < len; i++)
        {
            char ch = str.charAt(i);
            if(match.indexOf(ch) == -1)
            {
                return i;
            }
        }

        return -1;
    }

    private static void
    escapeChar(byte b, StringBuffer s, String special)
    {
        switch(b)
        {
        case (byte)'\\':
        {
            s.append("\\\\");
            break;
        }
        case (byte)'\'':
        {
            s.append("\\'");
            break;
        }
        case (byte)'"':
        {
            s.append("\\\"");
            break;
        }
        case (byte)'\b':
        {
            s.append("\\b");
            break;
        }
        case (byte)'\f':
        {
            s.append("\\f");
            break;
        }
        case (byte)'\n':
        {
            s.append("\\n");
            break;
        }
        case (byte)'\r':
        {
            s.append("\\r");
            break;
        }
        case (byte)'\t':
        {
            s.append("\\t");
            break;
        }
        default:
        {
            if(b <= (byte)31 || b == (byte)127) // Bytes are signed in Java (-128 to 127)
            {
                s.append('\\');
                String octal = Integer.toOctalString(b);
                //
                // Add leading zeroes so that we avoid problems during
                // decoding. For example, consider the encoded string
                // \0013 (i.e., a character with value 1 followed by
                // the character '3'). If the leading zeroes were omitted,
                // the result would be incorrectly interpreted by the
                // decoder as a single character with value 11.
                //
                for(int j = octal.length(); j < 3; j++)
                {
                    s.append('0');
                }
                s.append(octal);
            }
            else if(special != null && special.indexOf((char)b) != -1)
            {
                s.append('\\');
                escapeChar(b, s, null);
            }
            else
            {
                s.append((char)b);
            }
        }
        }
    }

    //
    // Add escape sequences (like "\n", or "\0xxx") to make a string
    // readable in ASCII.
    //
    public static String
    escapeString(String s, String special)
    {
        byte[] bytes = null;
        try
        {
            bytes = s.getBytes("UTF8");
        }
        catch(java.io.UnsupportedEncodingException ex)
        {
            assert(false);
            return null;
        }

        StringBuffer result = new StringBuffer(bytes.length);

        for(int i = 0; i < bytes.length; i++)
        {
            escapeChar(bytes[i], result, special);
        }

        return result.toString();
    }

    //
    // Remove escape sequences added by escapeString.
    //
    public static boolean
    unescapeString(String s, int start, int end, Ice.StringHolder result)
    {
        final int len = s.length();
        assert(start >= 0);
        assert(end <= len);
        assert(start <= end);

        byte[] bytes = new byte[len];
        int bc = 0;
        while(start < end)
        {
            char ch = s.charAt(start);
            if(ch == '\\')
            {
                start++;
                if(start == end)
                {
                    return false; // Missing character
                }
                ch = s.charAt(start);
                switch(ch)
                {
                case '\\':
                {
                    bytes[bc++] = (byte)'\\';
                    break;
                }
                case '\'':
                case '"':
                {
                    bytes[bc++] = (byte)ch;
                    break;
                }
                case 'b':
                {
                    bytes[bc++] = (byte)'\b';
                    break;
                }
                case 'f':
                {
                    bytes[bc++] = (byte)'\f';
                    break;
                }
                case 'n':
                {
                    bytes[bc++] = (byte)'\n';
                    break;
                }
                case 'r':
                {
                    bytes[bc++] = (byte)'\r';
                    break;
                }
                case 't':
                {
                    bytes[bc++] = (byte)'\t';
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
                    while(count < 3 && start < end && s.charAt(start) >= '0' && s.charAt(start) <= '9')
                    {
                        val <<= 3;
                        val |= s.charAt(start) - '0';
                        start++;
                        count++;
                    }
                    if(val > 255)
                    {
                        return false; // Octal value out of range
                    }
                    bytes[bc++] = (byte)val;
                    continue; // don't increment start
                }
                default:
                {
                    byte b = (byte)ch;
                    if(b <= (byte)31 || b == (byte)127) // Bytes are signed in Java (-128 to 127)
                    {
                        return false; // Malformed encoding
                    }
                    else
                    {
                        bytes[bc++] = b;
                    }
                }
                }
            }
            else
            {
                bytes[bc++] = (byte)ch;
            }
            start++;
        }

        try
        {
            result.value = new String(bytes, 0, bc, "UTF8");
        }
        catch(java.io.UnsupportedEncodingException ex)
        {
            assert(false);
        }

        return true;
    }

    public static int
    checkQuote(String s)
    {
        return checkQuote(s, 0);
    }

    //
    // If a single or double quotation mark is found at the start position,
    // then the position of the matching closing quote is returned. If no
    // quotation mark is found at the start position, then 0 is returned.
    // If no matching closing quote is found, then -1 is returned.
    //
    public static int
    checkQuote(String s, int start)
    {
        char quoteChar = s.charAt(start);
        if(quoteChar == '"' || quoteChar == '\'')
        {
            start++;
            final int len = s.length();
            int pos;
            while(start < len && (pos = s.indexOf(quoteChar, start)) != -1)
            {
                if(s.charAt(pos - 1) != '\\')
                {
                    return pos;
                }
                start = pos + 1;
            }
            return -1; // Unmatched quote
        }
        return 0; // Not quoted
    }
}
