// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package Ice;

public final class LoggerI extends LocalObjectImpl implements Logger
{
    public 
    LoggerI(String prefix, boolean timestamp)
    {
	if(prefix.length() > 0)
	{
	    _prefix = prefix + ": ";
	}

	if(timestamp)
	{
	    _date = new java.text.SimpleDateFormat("MM/dd/yy HH:mm:ss:SSS");
	}
    }

    public void
    trace(String category, String message)
    {
	synchronized(_globalMutex)
	{
            StringBuffer s = new StringBuffer("[ ");
	    if(_date != null)
	    {
		s.append(_date.format(new java.util.Date()));
		s.append(' ');
	    }
	    s.append(_prefix);
	    s.append(category);
	    s.append(": ");
	    s.append(message);
	    s.append(" ]");
            int idx = 0;
            while((idx = s.indexOf("\n", idx)) != -1)
            {
                s.insert(idx + 1, "  ");
                ++idx;
            }
            System.err.println(s.toString());
	}
    }

    public void
    warning(String message)
    {
	synchronized(_globalMutex)
	{
	    StringBuffer s = new StringBuffer();
	    if(_date != null)
	    {
		s.append(_date.format(new java.util.Date()));
		s.append(' ');
	    }
	    s.append(_prefix);
	    s.append("warning: ");
	    s.append(message);
	    System.err.println(s.toString());
	}
    }

    public void
    error(String message)
    {
	synchronized(_globalMutex)
	{
	    StringBuffer s = new StringBuffer();
	    if(_date != null)
	    {
		s.append(_date.format(new java.util.Date()));
		s.append(' ');
	    }
	    s.append(_prefix);
	    s.append("error: ");
	    s.append(message);
	    System.err.println(s.toString());
	}
    }

    String _prefix = "";
    static java.lang.Object _globalMutex = new java.lang.Object();
    java.text.SimpleDateFormat _date = null;
}
