// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceUtilInternal;

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

    //
    // Write the byte b as an escape sequence if it isn't a printable ASCII
    // character and append the escape sequence to sb. Additional characters
    // that should be escaped can be passed in special. If b is any of these
    // characters, b is preceded by a backslash in sb.
    //
    private static void
    encodeChar(byte b, StringBuilder sb, String special)
    {
        switch(b)
        {
            case (byte)'\\':
            {
                sb.append("\\\\");
                break;
            }
            case (byte)'\'':
            {
                sb.append("\\'");
                break;
            }
            case (byte)'"':
            {
                sb.append("\\\"");
                break;
            }
            case (byte)'\b':
            {
                sb.append("\\b");
                break;
            }
            case (byte)'\f':
            {
                sb.append("\\f");
                break;
            }
            case (byte)'\n':
            {
                sb.append("\\n");
                break;
            }
            case (byte)'\r':
            {
                sb.append("\\r");
                break;
            }
            case (byte)'\t':
            {
                sb.append("\\t");
                break;
            }
            default:
            {
                if(!(b >= 32 && b <= 126))
                {
                    sb.append('\\');
                    String octal = Integer.toOctalString(b < 0 ? b + 256 : b);
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
                        sb.append('0');
                    }
                    sb.append(octal);
                }
                else if(special != null && special.indexOf((char)b) != -1)
                {
                    sb.append('\\');
                    sb.append((char)b);
                }
                else
                {
                    sb.append((char)b);
                }
            }
        }
    }

    //
    // Add escape sequences (such as "\n", or "\007") to make a string
    // readable in ASCII. Any characters that appear in special are
    // prefixed with a backlash in the returned string.
    //
    public static String
    escapeString(String s, String special)
    {
        if(special != null)
        {
            for(int i = 0; i < special.length(); ++i)
            {
                if(special.charAt(i) < 32 || special.charAt(i) > 126)
                {
                    throw new IllegalArgumentException("special characters must be in ASCII range 32-126");
                }
            }
        }

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

        StringBuilder result = new StringBuilder(bytes.length);
        for(int i = 0; i < bytes.length; i++)
        {
            encodeChar(bytes[i], result, special);
        }

        return result.toString();
    }

    private static char
    checkChar(String s, int pos)
    {
        char c = s.charAt(pos);
        if(!(c >= 32 && c <= 126))
        {
            String msg;
            if(pos > 0)
            {
                msg = "character after `" + s.substring(0, pos) + "'";
            }
            else
            {
                msg = "first character";
            }
            msg += " is not a printable ASCII character (ordinal " + (int)c + ")";
            throw new IllegalArgumentException(msg);
        }
        return c;
    }

    static class Holder<T>
    {
        public Holder()
        {
        }

        public Holder(T value)
        {
            this.value = value;
        }

        public T value;
    }

    //
    // Decode the character or escape sequence starting at start and return it.
    // newStart is set to the index of the first character following the decoded character
    // or escape sequence.
    //
    private static char decodeChar(String s, int start, int end, Holder<Integer> nextStart)
    {
        assert(start >= 0);
        assert(start < end);
        assert(end <= s.length());

        char c;

        if(s.charAt(start) != '\\')
        {
            c = checkChar(s, start++);
        }
        else
        {
            if(start + 1 == end)
            {
                throw new IllegalArgumentException("trailing backslash");
            }
            switch(s.charAt(++start))
            {
                case '\\':
                case '\'':
                case '"':
                {
                    c = s.charAt(start++);
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
                        int charVal = s.charAt(start++) - '0';
                        if(charVal < 0 || charVal > 7)
                        {
                            --start;
                            break;
                        }
                        val = val * 8 + charVal;
                    }
                    if(val > 255)
                    {
                        String msg = "octal value \\" + Integer.toOctalString(val) + " (" + val + ") is out of range";
                        throw new IllegalArgumentException(msg);
                    }
                    c = (char)val;
                    break;
                }
                default:
                {
                    c = checkChar(s, start++);
                    break;
                }
            }
        }
        nextStart.value = start;
        return c;
    }

    //
    // Remove escape sequences from s and append the result to sb.
    // Return true if successful, false otherwise.
    //
    private static void
    decodeString(String s, int start, int end, StringBuilder sb)
    {
        Holder<Integer> nextStart = new Holder<>();
        while(start < end)
        {
            sb.append(decodeChar(s, start, end, nextStart));
            start = nextStart.value;
        }
    }

    //
    // Remove escape sequences added by escapeString. Throws IllegalArgumentException
    // for an invalid input string.
    //
    public static String
    unescapeString(String s, int start, int end)
    {
        assert(start >= 0 && start <= end && end <= s.length());

        StringBuilder sb = new StringBuilder(end - start);
        decodeString(s, start, end, sb);
        String decodedString = sb.toString();

        byte[] arr = new byte[decodedString.length()];
        for(int i = 0; i < arr.length; ++i)
        {
            arr[i] = (byte)decodedString.charAt(i);
        }

        try
        {
            return new String(arr, 0, arr.length, "UTF8");
        }
        catch(java.io.UnsupportedEncodingException ex)
        {
            throw new IllegalArgumentException("unsupported encoding", ex);
        }
    }

    //
    // Join a list of strings using the given delimiter.
    //
    public static String
    joinString(java.util.List<String> values, String delimiter)
    {
        StringBuffer s = new StringBuffer();
        boolean first = true;
        for(String v : values)
        {
            if(!first)
            {
                s.append(delimiter);
            }
            s.append(v);
            first = false;
        }
        return s.toString();
    }

    //
    // Split string helper; returns null for unmatched quotes
    //
    static public String[]
    splitString(String str, String delim)
    {
        java.util.List<String> l = new java.util.ArrayList<>();
        char[] arr = new char[str.length()];
        int pos = 0;

        int n = 0;
        char quoteChar = '\0';
        while(pos < str.length())
        {
            if(quoteChar == '\0' && (str.charAt(pos) == '"' || str.charAt(pos) == '\''))
            {
                quoteChar = str.charAt(pos++);
                continue; // Skip the quote.
            }
            else if(quoteChar == '\0' && str.charAt(pos) == '\\' && pos + 1 < str.length() &&
                    (str.charAt(pos + 1) == '"' || str.charAt(pos + 1) == '\''))
            {
                ++pos; // Skip the backslash
            }
            else if(quoteChar != '\0' && str.charAt(pos) == '\\' && pos + 1 < str.length() &&
                    str.charAt(pos + 1) == quoteChar)
            {
                ++pos; // Skip the backslash
            }
            else if(quoteChar != '\0' && str.charAt(pos) == quoteChar)
            {
                ++pos;
                quoteChar = '\0';
                continue; // Skip the quote.
            }
            else if(delim.indexOf(str.charAt(pos)) != -1)
            {
                if(quoteChar == '\0')
                {
                    ++pos;
                    if(n > 0)
                    {
                        l.add(new String(arr, 0, n));
                        n = 0;
                    }
                    continue;
                }
            }

            if(pos < str.length())
            {
                arr[n++] = str.charAt(pos++);
            }
        }

        if(n > 0)
        {
            l.add(new String(arr, 0, n));
        }
        if(quoteChar != '\0')
        {
            return null; // Unmatched quote.
        }
        return l.toArray(new String[0]);
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

    public static boolean
    match(String s, String pat, boolean emptyMatch)
    {
        assert(s.length() > 0);
        assert(pat.length() > 0);

        //
        // If pattern does not contain a wildcard just compare strings.
        //
        int beginIndex = pat.indexOf('*');
        if(beginIndex < 0)
        {
            return s.equals(pat);
        }

        //
        // Make sure start of the strings match
        //
        if(beginIndex > s.length() || !s.substring(0, beginIndex).equals(pat.substring(0, beginIndex)))
        {
            return false;
        }

        //
        // Make sure there is something present in the middle to match the
        // wildcard. If emptyMatch is true, allow a match of "".
        //
        int endLength = pat.length() - beginIndex - 1;
        if(endLength == 0)
        {
            return true;
        }
        if(endLength > s.length())
        {
            return false;
        }
        int endIndex = s.length() - endLength;
        if(endIndex < beginIndex || (!emptyMatch && endIndex == beginIndex))
        {
            return false;
        }

        //
        // Make sure end of the strings match
        //
        if(!s.substring(endIndex, s.length() - endIndex).equals(
               pat.substring(beginIndex + 1, pat.length() - beginIndex - 1)))
        {
            return false;
        }

        return true;
    }
}
