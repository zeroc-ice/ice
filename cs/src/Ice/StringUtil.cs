// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
	
	private static void encodeChar(byte b, System.Text.StringBuilder s, string special)
	{
	    switch((char)b)
	    {
		case '\\': 
		{
		    s.Append("\\\\");
		    break;
		}
		
		case '\'': 
		{
		    s.Append("\\'");
		    break;
		}
		
		case '"': 
		{
		    s.Append("\\\"");
		    break;
		}
		
		case '\b': 
		{
		    s.Append("\\b");
		    break;
		}
		
		case '\f': 
		{
		    s.Append("\\f");
		    break;
		}
		
		case '\n': 
		{
		    s.Append("\\n");
		    break;
		}
		
		case '\r': 
		{
		    s.Append("\\r");
		    break;
		}
		
		case '\t': 
		{
		    s.Append("\\t");
		    break;
		}
		
		default: 
		{
		    if(b < (byte)32 || b > (byte)126)
		    {
			s.Append('\\');
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
			    s.Append('0');
			}
			s.Append(octal);
		    }
		    else if(special != null && special.IndexOf((char)b) != -1)
		    {
			s.Append('\\');
			encodeChar(b, s, null);
		    }
		    else
		    {
			s.Append((char)b);
		    }
		}
		break;
	    }
	}
	
        //
        // Add escape sequences (like "\n", or "\0xxx") to make a string
        // readable in ASCII.
        //
	public static string escapeString(string s, string special)
	{
	    System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding();
	    byte[] bytes = utf8.GetBytes(s);

	    System.Text.StringBuilder result = new System.Text.StringBuilder(bytes.Length);
	    for(int i = 0; i < bytes.Length; i++)
	    {
		encodeChar(bytes[i], result, special);
	    }
	    
	    return result.ToString();
	}
	
        //
        // Remove escape sequences added by escapeString.
        //
	public static bool unescapeString(string s, int start, int end, out string result)
	{
	    Debug.Assert(start >= 0);
	    Debug.Assert(end <= s.Length);
	    Debug.Assert(start <= end);
	    
	    result = null;
	    try
	    {
		int num = end - start;
		byte[] arr = new byte[num];
		for(int i = 0; i < num; ++i)
		{
		    arr[i] = (byte)s[start + i];
		}
		System.Text.UTF8Encoding utf8 = new System.Text.UTF8Encoding(false, true);
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
    }
}
