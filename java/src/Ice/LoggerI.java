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

    public void
    trace(String category, String message)
    {
	synchronized(_globalMutex)
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
    }

    public void
    warning(String message)
    {
	synchronized(_globalMutex)
	{
	    System.err.println(_prefix + "warning: " + message);
	}
    }

    public void
    error(String message)
    {
	synchronized(_globalMutex)
	{
	    System.err.println(_prefix + "error: " + message);
	}
    }

    String _prefix = "";
    String _emptyPrefix = "";
    static Object _globalMutex = new Object();
}
