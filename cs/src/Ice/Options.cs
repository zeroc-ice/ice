// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using System.Diagnostics;

namespace IceUtil
{
    public sealed class Options
    {
	public sealed class BadQuote : System.ApplicationException
	{
	    public BadQuote(string message) : base(message)
	    {
	    }
	}

	enum State { Normal, DoubleQuote, SingleQuote };
 
	static public string[]
	split(string line)
        {
	    string IFS = " \t\n";
	    
	    line = line.Trim();

	    State state = State.Normal;

	    string arg = "";
	    ArrayList vec = new ArrayList();
	    
	    for(int i = 0; i < line.Length; ++i)
	    {
		char c = line[i];
		switch(state)
		{
		    case State.Normal:
		    {
			switch(c)
			{
			    case '\\':
			    {
				//
				// Ignore a backslash at the end of the string,
				// and strip backslash-newline pairs. If a
				// backslash is followed by a space, we just
				// write the space. Anything else following a
				// backslash is preserved, including the backslash.
				// (This deviates from bash quoting rules, but is
				// necessary so we don't drop backslashes from Windows
				// path names.)
				//
				if(i < line.Length - 1 && line[++i] != '\n')
				{
				    if(line[i] == ' ')
				    {
					arg += ' ';
				    }
				    else
				    {
					arg += '\\';
					arg += line[i];
				    }
				}
				break;
			    }
			    case '\'':
			    {
				state = State.SingleQuote;
				break;
			    }
			    case '"':
			    {
				state = State.DoubleQuote;
				break;
			    }
			    default:
			    {
				if(IFS.IndexOf(line[i]) != -1)
				{
				    vec.Add(arg);
				    arg = "";
				    
				    //
				    // Move to start of next argument.
				    //
				    while(++i < line.Length && IFS.IndexOf(line[i]) != -1)
				    {
					;
				    }
				    --i;
				}
				else
				{
				    arg += line[i];
				}
				break;
			    }
			}
			break;
		    }
		    case State.DoubleQuote:
		    {
			//
			// Within double quotes, only backslash retains its special
			// meaning, and only if followed by double quote, backslash,
			// or newline. If not followed by one of these characters,
			// both the backslash and the character are preserved.
			//
			if(c == '\\' && i < line.Length - 1)
			{
			    switch(c = line[++i])
			    {
				case '"':
				case '\\':
				case '\n':
				{
				    arg += c;
				    break;
				}
				default:
				{
				    arg += '\\';
				    arg += c;
				    break;
				}
			    }
			}
			else if(c == '"') // End of double-quote mode.
			{
			    state = State.Normal;
			}
			else
			{
			    arg += c; // Everything else is taken literally.
			}
			break;
		    }
		    case State.SingleQuote:
		    {
			if(c == '\'') // End of single-quote mode.
			{
			    state = State.Normal;
			}
			else
			{
			    arg += c; // Everything else is taken literally.
			}
			break;
		    }
		    default:
		    {
			Debug.Assert(false);
			break;
		    }
		}
	    }
	    
	    switch(state)
	    {
		case State.Normal:
		{
		    vec.Add(arg);
		    break;
		}
		case State.SingleQuote:
		{
		    throw new BadQuote("missing closing single quote");
		}
		case State.DoubleQuote:
		{
		    throw new BadQuote("missing closing double quote");
		}
		default:
		{
		    Debug.Assert(false);
		    break;
		}
	    }
	    
	    return (string[])vec.ToArray(typeof(string));
	}
    }
}
