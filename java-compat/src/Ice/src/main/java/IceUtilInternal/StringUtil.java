// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtilInternal;

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
    encodeChar(char c, StringBuilder sb, String special, Ice.ToStringMode toStringMode)
    {
        switch(c)
        {
            case '\\':
            {
                sb.append("\\\\");
                break;
            }
            case '\'':
            {
                sb.append("\\'");
                break;
            }
            case '"':
            {
                sb.append("\\\"");
                break;
            }
            case '\b':
            {
                sb.append("\\b");
                break;
            }
            case '\f':
            {
                sb.append("\\f");
                break;
            }
            case '\n':
            {
                sb.append("\\n");
                break;
            }
            case '\r':
            {
                sb.append("\\r");
                break;
            }
            case '\t':
            {
                sb.append("\\t");
                break;
            }
            default:
            {
                if(special != null && special.indexOf(c) != -1)
                {
                    sb.append('\\');
                    sb.append(c);
                }
                else
                {
                    if(c < 32 || c > 126)
                    {
                        if(toStringMode == Ice.ToStringMode.Compat)
                        {
                            //
                            // When ToStringMode=Compat, c is a UTF-8 byte
                            //
                            assert(c < 256);

                            sb.append('\\');
                            String octal = Integer.toOctalString(c);
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
                        else if(c < 32 || c == 127 || toStringMode == Ice.ToStringMode.ASCII)
                        {
                            // append \\unnnn
                            sb.append("\\u");
                            String hex = Integer.toHexString(c);
                            for(int j = hex.length(); j < 4; j++)
                            {
                                sb.append('0');
                            }
                            sb.append(hex);
                        }
                        else
                        {
                            // keep as is
                            sb.append(c);
                        }
                    }
                    else
                    {
                        // printable ASCII character
                        sb.append(c);
                    }
                }
                break;
            }
        }
    }

    //
    // Add escape sequences (like "\n" to the input string)
    // The second parameter adds characters to escape, and can be empty.
    //
    public static String
    escapeString(String s, String special, Ice.ToStringMode toStringMode)
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

        if(toStringMode == Ice.ToStringMode.Compat)
        {
            // Encode UTF-8 bytes

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
                encodeChar((char)(bytes[i] & 0xFF), result, special, toStringMode);
            }

            return result.toString();
        }
        else
        {
            StringBuilder result = new StringBuilder(s.length());

            for(int i = 0; i < s.length(); i++)
            {
                char c = s.charAt(i);
                if(toStringMode == Ice.ToStringMode.Unicode || !Character.isSurrogate(c))
                {
                    encodeChar(c, result, special, toStringMode);
                }
                else
                {
                    assert(toStringMode == Ice.ToStringMode.ASCII && Character.isSurrogate(c));
                    if(i + 1 == s.length())
                    {
                        throw new IllegalArgumentException("High surrogate without low surrogate");
                    }
                    else
                    {
                        i++;
                        int codePoint = Character.toCodePoint(c, s.charAt(i));
                        // append \Unnnnnnnn
                        result.append("\\U");
                        String hex = Integer.toHexString(codePoint);
                        for(int j = hex.length(); j < 8; j++)
                        {
                            result.append('0');
                        }
                        result.append(hex);
                    }
                }
            }

            return result.toString();
        }
    }

    private static char
    checkChar(String s, int pos)
    {
        char c = s.charAt(pos);
        if(c < 32 || c == 127)
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
            msg += " has invalid ordinal value " + (int)c;
            throw new IllegalArgumentException(msg);
        }
        return c;
    }

    //
    // Decode the character or escape sequence starting at start and appends it to result;
    // returns the index of the first character following the decoded character
    // or escape sequence.
    //
    private static int
    decodeChar(String s, int start, int end, StringBuilder result)
    {
        assert(start >= 0);
        assert(start < end);
        assert(end <= s.length());

        if(s.charAt(start) != '\\')
        {
            result.append(checkChar(s, start++));
        }
        else
        {
            if(start + 1 == end)
            {
                throw new IllegalArgumentException("trailing backslash");
            }

            char c = s.charAt(++start);

            switch(c)
            {
                case '\\':
                case '\'':
                case '"':
                {
                    ++start;
                    result.append(c);
                    break;
                }
                case 'b':
                {
                    ++start;
                    result.append('\b');
                    break;
                }
                case 'f':
                {
                    ++start;
                    result.append('\f');
                    break;
                }
                case 'n':
                {
                    ++start;
                    result.append('\n');
                    break;
                }
                case 'r':
                {
                    ++start;
                    result.append('\r');
                    break;
                }
                case 't':
                {
                    ++start;
                    result.append('\t');
                    break;
                }
                case 'u':
                case 'U':
                {
                    int codePoint = 0;
                    boolean inBMP = (c == 'u');
                    int size = inBMP ? 4 : 8;
                    ++start;
                    while(size > 0 && start < end)
                    {
                        c = s.charAt(start++);
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
                        codePoint = codePoint * 16 + charVal;
                        --size;
                    }
                    if(size > 0)
                    {
                        throw new IllegalArgumentException("Invalid universal character name: too few hex digits");
                    }
                    if(inBMP && Character.isSurrogate((char)codePoint))
                    {
                        throw new IllegalArgumentException("A non-BMP character cannot be encoded with \\unnnn, use \\Unnnnnnnn instead");
                    }
                    if(inBMP || Character.isBmpCodePoint(codePoint))
                    {
                        result.append((char)codePoint);
                    }
                    else
                    {
                        result.append(Character.toChars(codePoint));
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
                    // UTF-8 byte sequence encoded with octal escapes

                    byte[] arr = new byte[end - start];
                    int i = 0;
                    boolean done = false;
                    while(!done)
                    {
                        int val = 0;
                        for(int j = 0; j < 3 && start < end; ++j)
                        {
                            int charVal = s.charAt(start++) - '0';
                            if(charVal < 0 || charVal > 7)
                            {
                                --start;
                                if(j == 0)
                                {
                                    // first character after escape is not 0-7:
                                    done = true;
                                    --start; // go back to the previous backslash
                                }
                                break; // for
                            }
                            val = val * 8 + charVal;
                        }

                        if(!done)
                        {
                            if(val > 255)
                            {
                                String msg = "octal value \\" + Integer.toOctalString(val) + " (" + val + ") is out of range";
                                throw new IllegalArgumentException(msg);
                            }
                            arr[i++] = (byte)val;

                            if((start + 1 < end) && s.charAt(start) == '\\')
                            {
                                start++;
                                // loop, read next octal escape sequence
                            }
                            else
                            {
                                done = true;
                            }
                        }
                    }

                    try
                    {
                        result.append(new String(arr, 0, i, "UTF8"));
                    }
                    catch(java.io.UnsupportedEncodingException ex)
                    {
                        throw new IllegalArgumentException("unsupported encoding", ex);
                    }
                    break;
                }
                default:
                {
                    result.append(checkChar(s, start++));
                    break;
                }
            }
        }

        return start;
    }

    //
    // Remove escape sequences added by escapeString. Throws IllegalArgumentException
    // for an invalid input string.
    //
    public static String
    unescapeString(String s, int start, int end)
    {
        assert(start >= 0 && start <= end && end <= s.length());

        // Optimization for strings without escapes
        int p = s.indexOf('\\', start);
        if(p == -1 || p >= end)
        {
            p = start;
            while(p < end)
            {
                checkChar(s, p++);
            }
            return s.substring(start, end);
        }
        else
        {
            StringBuilder sb = new StringBuilder(end - start);
            while(start < end)
            {
                start = decodeChar(s, start, end, sb);
            }
            return sb.toString();
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
        java.util.List<String> l = new java.util.ArrayList<String>();
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
