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
    LoggerI(String prefix)
    {
	if(prefix.length() > 0)
	{
	    _prefix = prefix + ": ";
	}
    }

    public void
    trace(String category, String message)
    {
	synchronized(_globalMutex)
	{
            StringBuffer s = new StringBuffer("[ " + _prefix + category + ": " + message + " ]");
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
    static java.lang.Object _globalMutex = new java.lang.Object();
}
