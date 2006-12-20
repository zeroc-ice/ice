// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

	_date = java.text.DateFormat.getDateInstance(java.text.DateFormat.SHORT);
	_time = new java.text.SimpleDateFormat(" HH:mm:ss:SSS");
    }

    public void
    print(String message)
    {
	synchronized(_globalMutex)
	{
	    System.err.println(message);
	}
    }

    public void
    trace(String category, String message)
    {
	synchronized(_globalMutex)
	{
            StringBuffer s = new StringBuffer("[ ");
	    s.append(_date.format(new java.util.Date()));
	    s.append(_time.format(new java.util.Date()));
	    s.append(' ');
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
	    s.append(_date.format(new java.util.Date()));
	    s.append(_time.format(new java.util.Date()));
	    s.append(' ');
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
	    s.append(_date.format(new java.util.Date()));
	    s.append(_time.format(new java.util.Date()));
	    s.append(' ');
	    s.append(_prefix);
	    s.append("error: ");
	    s.append(message);
	    System.err.println(s.toString());
	}
    }

    String _prefix = "";
    static java.lang.Object _globalMutex = new java.lang.Object();
    java.text.DateFormat _date = null;
    java.text.SimpleDateFormat _time = null;
}
