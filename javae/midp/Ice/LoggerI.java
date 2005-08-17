// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

//
// The default logger for MIDP application simply stubs out the calls to the logger interface. MIDP apps generally
// don't have a stderr or stdout to write to.
//

public final class LoggerI extends LocalObjectImpl implements Logger
{
    public 
    LoggerI(String prefix, boolean timestamp)
    {
	if(prefix.length() > 0)
	{
	    _prefix = prefix + ": ";
	}

	_timestamp = timestamp;
    }

    public void
    print(String message)
    {
	synchronized(_globalMutex)
	{
	    _out.println(message);
	}
    }

    public void
    trace(String category, String message)
    {
	synchronized(_globalMutex)
	{
            StringBuffer s = new StringBuffer("[ ");
	    if(_timestamp)
	    {
		s.append(new java.util.Date().toString());
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
            _out.println(s.toString());
	}
    }

    public void
    warning(String message)
    {
	synchronized(_globalMutex)
	{
	    StringBuffer s = new StringBuffer();
	    if(_timestamp)
	    {
		s.append(new java.util.Date().toString());
		s.append(' ');
	    }
	    s.append(_prefix);
	    s.append("warning: ");
	    s.append(message);
	    _out.println(s.toString());
	}
    }

    public void
    error(String message)
    {
	synchronized(_globalMutex)
	{
	    StringBuffer s = new StringBuffer();
	    if(_timestamp)
	    {
		s.append(new java.util.Date().toString());
		s.append(' ');
	    }
	    s.append(_prefix);
	    s.append("error: ");
	    s.append(message);
	    _out.println(s.toString());
	}
    }

    protected
    LoggerI(LoggerI source)
    {
	super(source);
	_prefix = source._prefix;
	_timestamp = source._timestamp;
	_out = source._out;
    }
    
    public java.lang.Object
    ice_clone()
    {
	return new LoggerI(this);
    }

    String _prefix = "";
    static java.lang.Object _globalMutex = new java.lang.Object();
    boolean _timestamp = false;
    java.io.PrintStream _out = System.err;
}
