// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
	    if(_date != null)
	    {
		s.append(_date.format(new java.util.Date()));
		s.append(' ');
	    }
	    s.append(_prefix);
	    s.append(category);
	    s.append(": ");
            int beg = 0, end;
            while((end = message.indexOf('\n', beg)) != -1)
            {
		s.append(message.substring(beg, end + 1));
		s.append("  ");
		beg = end + 1;
            }
	    if(beg < message.length())
	    {
		s.append(message.substring(beg));
	    }
	    s.append(" ]");
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
