// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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

    //
    // Write the byte b as an escape sequence if it isn't a printable ASCII
    // character and append the escape sequence to sb. Additional characters
    // that should be escaped can be passed in special. If b is any of these
    // characters, b is preceded by a backslash in sb.
    //
    private static void
    encodeChar(byte b, StringBuffer sb, String special)
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

	//
	// Normally a simple call to the getBytes() specifying the UTF8
	// encoding is all that is needed here. It appears that the Nokia
	// emulators and possibly some of the Nokia phones don't accept
	// encoding arguments to string operations.  This allows us to
	// get a create a byte representation of a UTF encoded string in
	// a roundabout way.
	//
	try
	{
	    java.io.ByteArrayOutputStream bs = new java.io.ByteArrayOutputStream();
	    java.io.DataOutputStream os = new java.io.DataOutputStream(bs);
	    os.writeUTF(s);
	    bytes = bs.toByteArray();
	}
	catch(java.io.IOException ex)
	{
	    Debug.Assert(false);
	    return null; // This should never happen 
	}

        StringBuffer result = new StringBuffer(bytes.length);

	//
	// The characters in the byte array created by the
	// DataOutput.writeUTF() call start on the third byte.
	//
        for(int i = 2; i < bytes.length; i++)
        {
            encodeChar(bytes[i], result, special);
        }
        return result.toString();
    }

    private static char
    checkChar(char c)
    {
	if(!(c >= 32 && c <= 126))
	{
	    throw new IllegalArgumentException("illegal input character");
	}
	return c;
    }

    //
    // Decode the character or escape sequence starting at start and return it.
    // newStart is set to the index of the first character following the decoded character
    // or escape sequence.
    //
    private static char decodeChar(String s, int start, int end, Ice.IntHolder nextStart)
    {
	Debug.Assert(start >= 0);
	Debug.Assert(start < end);
	Debug.Assert(end <= s.length());

	char c;

	if(s.charAt(start) != '\\')
	{
	    c = checkChar(s.charAt(start++));
	}
	else
	{
	    if(start + 1 == end)
	    {
		throw new IllegalArgumentException("trailing backslash in argument");
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
		    int oct = 0;
		    for(int j = 0; j < 3 && start < end; ++j)
		    {
		        int charVal = s.charAt(start++) - '0';
			if(charVal < 0 || charVal > 7)
			{
			    --start;
			    break;
			}
			oct = oct * 8 + charVal;
		    }
		    if(oct > 255)
		    {
		        throw new IllegalArgumentException("octal value out of range");
		    }
		    c = (char)oct;
		    break;
		}
		default:
		{
		    c = checkChar(s.charAt(start++));
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
    decodeString(String s, int start, int end, StringBuffer sb)
    {
	Ice.IntHolder nextStart = new Ice.IntHolder();
        while(start < end)
	{
	    sb.append(decodeChar(s, start, end, nextStart));
	    start = nextStart.value;
	}
    }

    //
    // Remove escape sequences added by escapeString.
    //
    public static boolean
    unescapeString(String s, int start, int end, Ice.StringHolder result)
    {
        if(start < 0)
	{
	    throw new IllegalArgumentException("start offset must be >= 0");
	}
	if(end > s.length())
	{
	    throw new IllegalArgumentException("end offset must <= s.length()");
	}
	if(start > end)
	{
	    throw new IllegalArgumentException("start offset must <= end offset");
	}

	try
	{
	    StringBuffer sb = new StringBuffer();
	    decodeString(s, start, end, sb);
	    String decodedString = sb.toString();

	    byte[] arr = new byte[decodedString.length() + 2];
	    for(int i = 2; i < arr.length; ++i)
	    {
	        arr[i] = (byte)decodedString.charAt(i-2);
	    }

	    //
	    // Normally a simple call to the String constructor with the
	    // byte array and the UTF8 encoding is all that is needed
	    // here. It appears that the Nokia emulators and possibly
	    // some of the Nokia phones don't accept encoding arguments
	    // to string operations. This allows us to get a UTF encoded
	    // string from our byte array in a somewhat roundabout way.
	    //
	    short i = new Integer(decodedString.length()).shortValue();
	    arr[0] = (byte)(i & 0xff00);
	    arr[1] = (byte)(i & 0x00ff);
	    java.io.ByteArrayInputStream bs = new java.io.ByteArrayInputStream(arr);
	    java.io.DataInputStream is = new java.io.DataInputStream(bs);
	    result.value = is.readUTF();
	    return true;
	}
	catch(java.lang.Exception ex)
	{
	    return false;
	}
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

    public static String[]
    split(String s, String delim)
    {
	java.util.Vector arr = new java.util.Vector();
	int beg = findFirstNotOf(s, delim);
	int end = s.length();
	while(beg != -1 && (end = findFirstOf(s, delim, beg)) != -1)
	{
	    arr.addElement(s.substring(beg, end));
	    beg = findFirstNotOf(s, delim, end);
	}
	if(beg != -1)
	{
	    if(end == -1)
	    {
		arr.addElement(s.substring(beg));
	    }
	    else
	    {
		arr.addElement(s.substring(beg, end));
	    }
	}
	String[] result = new String[arr.size()];
	arr.copyInto(result);
	return result;
    }
}
