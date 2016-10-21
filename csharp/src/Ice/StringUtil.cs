// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Text;
using System.Diagnostics;
using System.Collections.Generic;

namespace IceUtilInternal
{

    public sealed class StringUtil
    {
        //
        // Return the index of the first character in str to
        // appear in match, starting from 0. Returns -1 if none is
        // found.
        //
        public static int findFirstOf(string str, string match)
        {
            return findFirstOf(str, match, 0);
        }

        //
        // Return the index of the first character in str to
        // appear in match, starting from start. Returns -1 if none is
        // found.
        //
        public static int findFirstOf(string str, string match, int start)
        {
            int len = str.Length;
            for(int i = start; i < len; i++)
            {
                char ch = str[i];
                if(match.IndexOf((char) ch) != -1)
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
        public static int findFirstNotOf(string str, string match)
        {
            return findFirstNotOf(str, match, 0);
        }

        //
        // Return the index of the first character in str which does
        // not appear in match, starting from start. Returns -1 if none is
        // found.
        //
        public static int findFirstNotOf(string str, string match, int start)
        {
            int len = str.Length;
            for(int i = start; i < len; i++)
            {
                char ch = str[i];
                if(match.IndexOf((char) ch) == -1)
                {
                    return i;
                }
            }

            return -1;
        }

        private static void
        encodeChar(char c, StringBuilder sb, string special, Ice.ToStringMode toStringMode)
        {
            switch(c)
            {
                case '\\':
                {
                    sb.Append("\\\\");
                    break;
                }

                case '\'':
                {
                    sb.Append("\\'");
                    break;
                }

                case '"':
                {
                    sb.Append("\\\"");
                    break;
                }

                case '\b':
                {
                    sb.Append("\\b");
                    break;
                }

                case '\f':
                {
                    sb.Append("\\f");
                    break;
                }

                case '\n':
                {
                    sb.Append("\\n");
                    break;
                }

                case '\r':
                {
                    sb.Append("\\r");
                    break;
                }

                case '\t':
                {
                    sb.Append("\\t");
                    break;
                }
                default:
                {
                    if(special != null && special.IndexOf(c) != -1)
                    {
                        sb.Append('\\');
                        sb.Append(c);
                    }
                    else
                    {
                        int i = (int)c;
                        if(i < 32 || i > 126)
                        {
                            if(toStringMode == Ice.ToStringMode.Compat)
                            {
                                //
                                // When ToStringMode=Compat, c is a UTF-8 byte
                                //
                                Debug.Assert(i < 256);

                                sb.Append('\\');
                                string octal = System.Convert.ToString(i, 8);
                                //
                                // Add leading zeroes so that we avoid problems during
                                // decoding. For example, consider the encoded string
                                // \0013 (i.e., a character with value 1 followed by
                                // the character '3'). If the leading zeroes were omitted,
                                // the result would be incorrectly interpreted by the
                                // decoder as a single character with value 11.
                                //
                                for(int j = octal.Length; j < 3; j++)
                                {
                                    sb.Append('0');
                                }
                                sb.Append(octal);
                            }
                            else if(i < 32 || i == 127 || toStringMode == Ice.ToStringMode.ASCII)
                            {
                                // append \\unnnn
                                sb.Append("\\u");
                                string hex = System.Convert.ToString(i, 16);
                                for(int j = hex.Length; j < 4; j++)
                                {
                                    sb.Append('0');
                                }
                                sb.Append(hex);
                            }
                            else
                            {
                                // keep as is
                                sb.Append(c);
                            }
                        }
                        else
                        {
                            // printable ASCII character
                            sb.Append(c);
                        }
                    }
                break;
                }
            }
        }

        //
        // Add escape sequences (such as "\n", or "\007") to the input string
        //
        public static string escapeString(string s, string special, Ice.ToStringMode toStringMode)
        {
            if(special != null)
            {
                for(int i = 0; i < special.Length; ++i)
                {
                    if(special[i] < 32 || special[i] > 126)
                    {
                        throw new System.ArgumentException("special characters must be in ASCII range 32-126",
                                                           "special");
                    }
                }
            }

            if(toStringMode == Ice.ToStringMode.Compat)
            {
                // Encode UTF-8 bytes

                UTF8Encoding utf8 = new UTF8Encoding();
                byte[] bytes = utf8.GetBytes(s);

                StringBuilder result = new StringBuilder(bytes.Length);
                for(int i = 0; i < bytes.Length; i++)
                {
                    encodeChar((char)bytes[i], result, special, toStringMode);
                }

                return result.ToString();
            }
            else
            {
                StringBuilder result = new StringBuilder(s.Length);

                for(int i = 0; i < s.Length; i++)
                {
                    char c = s[i];
                    if(toStringMode == Ice.ToStringMode.Unicode || !System.Char.IsSurrogate(c))
                    {
                        encodeChar(c, result, special, toStringMode);
                    }
                    else
                    {
                        Debug.Assert(toStringMode == Ice.ToStringMode.ASCII && System.Char.IsSurrogate(c));
                        if(i + 1 == s.Length)
                        {
                            throw new System.ArgumentException("High surrogate without low surrogate");
                        }
                        else
                        {
                            i++;
                            int codePoint = System.Char.ConvertToUtf32(c, s[i]);
                            // append \Unnnnnnnn
                            result.Append("\\U");
                            string hex = System.Convert.ToString(codePoint, 16);
                            for(int j = hex.Length; j < 8; j++)
                            {
                                result.Append('0');
                            }
                            result.Append(hex);
                        }
                    }
                }

                return result.ToString();
            }
        }

        private static char
        checkChar(string s, int pos)
        {
            char c = s[pos];
            if(c < 32 || c == 127)
            {
                string msg;
                if(pos > 0)
                {
                    msg = "character after `" + s.Substring(0, pos) + "'";
                }
                else
                {
                    msg = "first character";
                }
                msg += " is not a printable ASCII character (ordinal " + (int)c + ")";
                throw new System.ArgumentException(msg);
            }
            return c;
        }


        //
        // Decode the character or escape sequence starting at start and appends it to result;
        // returns the index of the first character following the decoded character
        // or escape sequence.
        //
        private static int
        decodeChar(string s, int start, int end, StringBuilder result, UTF8Encoding utf8Encoding)
        {
            Debug.Assert(start >= 0);
            Debug.Assert(start < end);
            Debug.Assert(end <= s.Length);

            if(s[start] != '\\')
            {
                result.Append(checkChar(s, start++));
            }
            else
            {
                if(start + 1 == end)
                {
                    throw new System.ArgumentException("trailing backslash");
                }

                char c = s[++start];

                switch(c)
                {
                    case '\\':
                    case '\'':
                    case '"':
                    {
                        ++start;
                        result.Append(c);
                        break;
                    }
                    case 'b':
                    {
                        ++start;
                        result.Append('\b');
                        break;
                    }
                    case 'f':
                    {
                        ++start;
                        result.Append('\f');
                        break;
                    }
                    case 'n':
                    {
                        ++start;
                        result.Append('\n');
                        break;
                    }
                    case 'r':
                    {
                        ++start;
                        result.Append('\r');
                        break;
                    }
                    case 't':
                    {
                        ++start;
                        result.Append('\t');
                        break;
                    }
                    case 'u':
                    case 'U':
                    {
                        int codePoint = 0;
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
                            codePoint = codePoint * 16 + charVal;
                            --size;
                        }
                        if(size > 0)
                        {
                            throw new System.ArgumentException("Invalid universal character name: too few hex digits");
                        }
                        if(inBMP && System.Char.IsSurrogate((char)codePoint))
                        {
                            throw new System.ArgumentException("A non-BMP character cannot be encoded with \\unnnn, use \\Unnnnnnnn instead");
                        }
                        if(inBMP || codePoint <= 0xFFFF)
                        {
                            result.Append((char)codePoint);
                        }
                        else
                        {
                            result.Append(System.Char.ConvertFromUtf32(codePoint));
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
                        bool done = false;
                        while(!done)
                        {
                            int val = 0;
                            for(int j = 0; j < 3 && start < end; ++j)
                            {
                                int charVal = s[start++] - '0';
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
                                    string msg = "octal value \\" + System.Convert.ToString(val, 8) + " (" + val + ") is out of range";
                                    throw new System.ArgumentException(msg);
                                }
                                arr[i++] = (byte)val;

                                if((start + 1 < end) && s[start] == '\\')
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

                        result.Append(utf8Encoding.GetString(arr, 0, i)); // May raise ArgumentException.
                        break;
                    }
                    default:
                    {
                        result.Append(checkChar(s, start++));
                        break;
                    }
                }
            }
            return start;
        }

        //
        // Remove escape sequences added by escapeString. Throws System.ArgumentException
        // for an invalid input string.
        //
        public static string unescapeString(string s, int start, int end)
        {
            Debug.Assert(start >= 0 && start <= end && end <= s.Length);

            // Optimization for strings without escapes
            if(start == end || s.IndexOf('\\', start, end - start) == -1)
            {
                int p = start;
                while(p < end)
                {
                    checkChar(s, p++);
                }
                return s.Substring(start, end - start);
            }
            else
            {
                StringBuilder sb = new StringBuilder(end - start);
                UTF8Encoding utf8Encoding = new UTF8Encoding(false, true);
                while(start < end)
                {
                    start = decodeChar(s, start, end, sb, utf8Encoding);
                }
                return sb.ToString();
            }
        }

        //
        // Split string helper; returns null for unmatched quotes
        //
        static public string[] splitString(string str, string delim)
        {
            List<string> l = new List<string>();
            char[] arr = new char[str.Length];
            int pos = 0;

            int n = 0;
            char quoteChar = '\0';
            while(pos < str.Length)
            {
                if(quoteChar == '\0' && (str[pos] == '"' || str[pos] == '\''))
                {
                    quoteChar = str[pos++];
                    continue; // Skip the quote.
                }
                else if(quoteChar == '\0' && str[pos] == '\\' && pos + 1 < str.Length &&
                        (str[pos + 1] == '\'' || str[pos + 1] == '"'))
                {
                    ++pos; // Skip the backslash
                }
                else if(quoteChar != '\0' && str[pos] == '\\' && pos + 1 < str.Length && str[pos + 1] == quoteChar)
                {
                    ++pos; // Skip the backslash
                }
                else if(quoteChar != '\0' && str[pos] == quoteChar)
                {
                    ++pos;
                    quoteChar = '\0';
                    continue; // Skip the quote.
                }
                else if(delim.IndexOf(str[pos]) != -1)
                {
                    if(quoteChar == '\0')
                    {
                        ++pos;
                        if(n > 0)
                        {
                            l.Add(new string(arr, 0, n));
                            n = 0;
                        }
                        continue;
                    }
                }

                if(pos < str.Length)
                {
                    arr[n++] = str[pos++];
                }

            }

            if(n > 0)
            {
                l.Add(new string(arr, 0, n));
            }
            if(quoteChar != '\0')
            {
                return null; // Unmatched quote.
            }
            return l.ToArray();
        }

        public static int checkQuote(string s)
        {
            return checkQuote(s, 0);
        }

        //
        // If a single or double quotation mark is found at the start position,
        // then the position of the matching closing quote is returned. If no
        // quotation mark is found at the start position, then 0 is returned.
        // If no matching closing quote is found, then -1 is returned.
        //
        public static int checkQuote(string s, int start)
        {
            char quoteChar = s[start];
            if(quoteChar == '"' || quoteChar == '\'')
            {
                start++;
                int len = s.Length;
                int pos;
                while(start < len && (pos = s.IndexOf(quoteChar, start)) != -1)
                {
                    if(s[pos - 1] != '\\')
                    {
                        return pos;
                    }
                    start = pos + 1;
                }
                return -1; // Unmatched quote
            }
            return 0; // Not quoted
        }

        public static bool match(string s, string pat, bool emptyMatch)
        {
            Debug.Assert(s.Length > 0);
            Debug.Assert(pat.Length > 0);

            //
            // If pattern does not contain a wildcard just compare strings.
            //
            int beginIndex = pat.IndexOf('*');
            if(beginIndex < 0)
            {
                return s.Equals(pat);
            }

            //
            // Make sure start of the strings match
            //
            if(beginIndex > s.Length || !s.Substring(0, beginIndex).Equals(pat.Substring(0, beginIndex)))
            {
                return false;
            }

            //
            // Make sure there is something present in the middle to match the
            // wildcard. If emptyMatch is true, allow a match of "".
            //
            int endLength = pat.Length - beginIndex - 1;
            if(endLength > s.Length)
            {
                return false;
            }
            int endIndex = s.Length - endLength;
            if(endIndex < beginIndex || (!emptyMatch && endIndex == beginIndex))
            {
                return false;
            }

            //
            // Make sure end of the strings match
            //
            if(!s.Substring(endIndex, s.Length - endIndex).Equals(
                   pat.Substring(beginIndex + 1, pat.Length - beginIndex - 1)))
            {
                return false;
            }

            return true;
        }

        private class OrdinalStringComparerImpl : System.Collections.Generic.IComparer<string>
        {
            public int Compare(string l, string r)
            {
                return string.CompareOrdinal(l, r);
            }
        }
        public static System.Collections.Generic.IComparer<string> OrdinalStringComparer =
            new OrdinalStringComparerImpl();
    }
}
