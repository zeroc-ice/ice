// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public final class Options
{
    static public final class BadQuote extends Exception
    {
	BadQuote(String message)
        {
	    super(message);
	}
    }

    static public String[]
    split(String line)
	throws BadQuote
    {
	final String IFS = " \t\n";
	final int NormalState = 1;
	final int DoubleQuoteState = 2;
	final int SingleQuoteState = 3;

	line = line.trim();

	int state = NormalState;

	StringBuffer arg = new StringBuffer();
	java.util.List vec = new java.util.ArrayList();

	for(int i = 0; i < line.length(); ++i)
	{
	    char c = line.charAt(i);
	    switch(state)
	    {
	    case NormalState:
	    {
		switch(c)
		{
		case '\\':
		{
		    //
		    // Ignore a backslash at the end of the string,
		    // and strip backslash-newline pairs. Anything
		    // else following a backslash is taken literally.
		    //
		    if(i < line.length() - 1 && line.charAt(++i) != '\n')
		    {
			arg.append(line.charAt(i));
		    }
		    break;
		}
		case '\'':
		{
		    state = SingleQuoteState;
		    break;
		}
		case '"':
		{
		    state = DoubleQuoteState;
		    break;
		}
		default:
		{
		    if(IFS.indexOf(line.charAt(i)) != -1)
		    {
			vec.add(arg.toString());
			arg = new StringBuffer();
			
			//
			// Move to start of next argument.
			//
			while(++i < line.length() && IFS.indexOf(line.charAt(i)) != -1);
			--i;
		    }
		    else
		    {
			arg.append(line.charAt(i));
		    }
		    break;
		}
		}
		break;
	    }
	    case DoubleQuoteState:
	    {
	        //
		// Within double quotes, only backslash retains its special
		// meaning, and only if followed by double quote, backslash,
		// or newline. If not followed by one of these characters,
		// both the backslash and the character are preserved.
		//
		if(c == '\\' && i < line.length() - 1)
		{
		    switch(c = line.charAt(++i))
		    {
			case '"':
			case '\\':
			case '\n':
			{
			    arg.append(c);
			    break;
			}
			default:
			{
			    arg.append('\\');
			    arg.append(c);
			    break;
			}
		    }
		}
		else if(c == '"') // End of double-quote mode.
		{
		    state = NormalState;
		}
		else
		{
		    arg.append(c); // Everything else is taken literally.
		}
		break;
	    }
	    case SingleQuoteState:
	    {
		if(c == '\'') // End of single-quote mode.
		{
		    state = NormalState;
		}
		else
		{
		    arg.append(c); // Everything else is taken literally.
		}
		break;
	    }
	    default:
		assert(false);
		break;
	    }
	}

	switch(state)
	{
	case NormalState:
	{
	    vec.add(arg.toString());
	    break;
	}
	case SingleQuoteState:
	{
	    throw new BadQuote("missing closing single quote");
	}
	case DoubleQuoteState:
	{
	    throw new BadQuote("missing closing double quote");
	}
	default:
	{
	    assert(false);
	    break;
	}
	}
	
	return (String[])vec.toArray(new String[0]);
    }


}
