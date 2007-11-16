// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Text;
using System.Diagnostics;

namespace IceUtil
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
        
        //
        // Write the byte b as an escape sequence if it isn't a printable ASCII
        // character and append the escape sequence to sb. Additional characters
        // that should be escaped can be passed in special. If b is any of these
        // characters, b is preceded by a backslash in sb.
        //
        private static void encodeChar(byte b, StringBuilder sb, string special)
        {
            switch((char)b)
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
                    if(!(b >= 32 && b <= 126))
                    {
                        sb.Append('\\');
                        string octal = System.Convert.ToString(b, 8);
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
                    else if(special != null && special.IndexOf((char)b) != -1)
                    {
                        sb.Append('\\');
                        sb.Append((char)b);
                    }
                    else
                    {
                        sb.Append((char)b);
                    }
                }
                break;
            }
        }
        
        //
        // Add escape sequences (such as "\n", or "\007") to make a string
        // readable in ASCII. Any characters that appear in special are
        // prefixed with a backslash in the returned string.
        //
        public static string escapeString(string s, string special)
        {
            if(special != null)
            {
                for(int i = 0; i < special.Length; ++i)
                {
                    if((int)special[i] < 32 || (int)special[i] > 126)
                    {
                        throw new System.ArgumentException("special characters must be in ASCII range 32-126", "special");
                    }
                }
            }

            UTF8Encoding utf8 = new UTF8Encoding();
            byte[] bytes = utf8.GetBytes(s);

            StringBuilder result = new StringBuilder(bytes.Length);
            for(int i = 0; i < bytes.Length; i++)
            {
                encodeChar(bytes[i], result, special);
            }
            
            return result.ToString();
        }
        
        private static char checkChar(char c)
        {
            if(!(c >= 32 && c <= 126))
            {
                throw new System.ArgumentException("illegal input character");
            }
            return c;
        }

        //
        // Decode the character or escape sequence starting at start and return it.
        // end marks the one-past-the-end position of the substring to be scanned.
        // nextStart is set to the index of the first character following the decoded
        // character or escape sequence.
        //
        private static char decodeChar(string s, int start, int end, out int nextStart)
        {
            Debug.Assert(start >= 0);
            Debug.Assert(start < end);
            Debug.Assert(end <= s.Length);

            char c;

            if(s[start] != '\\')
            {
                c = checkChar(s[start++]);
            }
            else
            {
                if(start + 1 == end)
                {
                    throw new System.ArgumentException("trailing backslash in argument");
                }
                switch(s[++start])
                {
                    case '\\': 
                    case '\'': 
                    case '"': 
                    {
                        c = s[start++];
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
                        int oct = 0;
                        for(int j = 0; j < 3 && start < end; ++j)
                        {
                            int charVal = s[start++] - '0';
                            if(charVal < 0 || charVal > 7)
                            {
                                --start;
                                break;
                            }
                            oct = oct * 8 + charVal;
                        }
                        if(oct > 255)
                        {
                            throw new System.ArgumentException("octal value out of range", "s");
                        }
                        c = System.Convert.ToChar(oct);
                        break;
                    }
                    default:
                    {
                        c = checkChar(s[start++]);
                        break;
                    }
                }
            }
            nextStart = start;
            return c;
        }

        //
        // Remove escape sequences from s and append the result to sb.
        // Return true if successful, false otherwise.
        //
        private static void decodeString(string s, int start, int end, StringBuilder sb)
        {
            while(start < end)
            {
                sb.Append(decodeChar(s, start, end, out start));
            }
        }

        //
        // Remove escape sequences added by escapeString.
        //
        public static bool unescapeString(string s, int start, int end, out string result)
        {
            if(start < 0)
            {
                throw new System.ArgumentException("start offset must be >= 0", "start");
            }
            if(end > s.Length)
            {
                throw new System.ArgumentException("end offset must be <= s.Length", "end");
            }
            if(start > end)
            {
                throw new System.ArgumentException("start offset must be <= end offset");
            }
            
            result = null;
            try
            {
                StringBuilder sb = new StringBuilder();
                decodeString(s, start, end, sb);
                string decodedString = sb.ToString();

                byte[] arr = new byte[decodedString.Length];
                for(int i = 0; i < arr.Length; ++i)
                {
                    arr[i] = (byte)decodedString[i];
                }

                UTF8Encoding utf8 = new UTF8Encoding(false, true);
                result = utf8.GetString(arr);
                return true;
            }
            catch(System.Exception)
            {
                return false;
            }
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

        private class OrdinalStringComparerImpl : System.Collections.IComparer
        {
            public int Compare(object l, object r)
            {
                return string.CompareOrdinal((string)l, (string)r);
            }        
        }
        public static readonly System.Collections.IComparer OrdinalStringComparer = new OrdinalStringComparerImpl();
    }
}
