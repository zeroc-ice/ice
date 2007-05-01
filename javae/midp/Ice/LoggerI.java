// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

//
// The default logger for MIDP application simply stubs out the calls to
// the logger interface. MIDP apps generally don't have a stderr or
// stdout to write to.
//

public final class LoggerI implements Logger
{
    public 
    LoggerI(String prefix)
    {
	if(prefix.length() > 0)
	{
	    _prefix = prefix + ": ";
	}

        _date = java.util.Calendar.getInstance();
    }

    public void
    print(String message)
    {
	synchronized(_globalMutex)
	{
	    _out.println(message);
	    _out.flush();
	}
    }

    public void
    trace(String category, String message)
    {
        StringBuffer s = new StringBuffer("[ ");
        s = timeStamp(s);
        s.append(" ");
	s.append(_prefix);
	s.append(category);
	s.append(": ");
        s.append(message);
        s.append(" ]");
        int start = 0;
        int end = 0;
        String temp = s.toString();
        s.setLength(0);
        while((end = temp.indexOf("\n", start)) != -1)
        {
            s.append(temp.substring(start, end-1));
            s.append(" ");
            start = end + 1;
        }
        s.append(temp.substring(start));

	synchronized(_globalMutex)
        {
            _out.println(s.toString());
	    _out.flush();
	}
    }

    //
    // MIDP does not have a direct way to produce a date & time string
    // consistent with the JDK version of the logger. timeStamp is
    // intended to emulate it through java.util.Calendar fields.
    //
    private StringBuffer
    timeStamp(StringBuffer b)
    {
        _date.setTime(new java.util.Date());
        /* M.D.Y */
        b.append(_date.get(java.util.Calendar.MONTH));
        b.append(".");
        b.append(_date.get(java.util.Calendar.DATE));
        b.append(".");
        b.append(_date.get(java.util.Calendar.YEAR));
        b.append(" ");

        /* HH:mm:ss:SSS */
        b.append(_date.get(java.util.Calendar.HOUR));
        b.append(":");
        int t = _date.get(java.util.Calendar.MINUTE);
        if(t < 10)
        {
            b.append("0");
        }
        b.append(t);
        b.append(":");
        t = _date.get(java.util.Calendar.SECOND);
        if(t < 10)
        {
            b.append("0");
        }
        b.append(t);
        b.append(":");
        b.append(_date.get(java.util.Calendar.MILLISECOND));
        return b;
    }

    public void
    warning(String message)
    {
	StringBuffer s = new StringBuffer();
        s = timeStamp(s);
        s.append(" ");
	s.append(_prefix);
	s.append("warning: ");
	s.append(message);

	synchronized(_globalMutex)
	{
	    _out.println(s.toString());
	    _out.flush();
	}
    }

    public void
    error(String message)
    {
	StringBuffer s = new StringBuffer();
        s = timeStamp(s);
        s.append(" ");
	s.append(_prefix);
	s.append("error: ");
	s.append(message);

	synchronized(_globalMutex)
	{
	    _out.println(s.toString());
	    _out.flush();
	}
    }

    String _prefix = "";
    static java.lang.Object _globalMutex = new java.lang.Object();
    java.util.Calendar _date;
    java.io.PrintStream _out = System.err;
}
