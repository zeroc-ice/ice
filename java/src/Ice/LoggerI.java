// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class LoggerI implements Logger
{
    public 
    LoggerI(String prefix)
    {
	if(prefix.length() > 0)
	{
	    _prefix = prefix + ": ";

	    //
	    // A prefix filled with spaces and as long as the prefix.
	    //
	    StringBuffer emptyPrefix = new StringBuffer(prefix.length());
	    for(int i = 0; i < prefix.length() + 2; i++)
	    {
		emptyPrefix.append(' ');
	    }
	    _emptyPrefix = emptyPrefix.toString();
	}
    }

    public synchronized void
    trace(String category, String message)
    {
        String s = "[ " + category + ": ";
        int start = 0;
        int next;
        while((next = message.indexOf('\n', start)) != -1)
        {
            s += message.substring(start, next + 1);
            s += _emptyPrefix + "  ";
            start = next + 1;
        }
        s += message.substring(start);
        s += " ]";
	System.err.println(_prefix + s);
    }

    public synchronized void
    warning(String message)
    {
	System.err.println(_prefix + "warning: " + message);
    }

    public synchronized void
    error(String message)
    {
	System.err.println(_prefix + "error: " + message);
    }

    String _prefix = "";
    String _emptyPrefix = "";
}
