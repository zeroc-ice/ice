// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
        }

        _lineSeparator = System.getProperties().getProperty("line.separator");
        _date = java.text.DateFormat.getDateInstance(java.text.DateFormat.SHORT);
        _time = new java.text.SimpleDateFormat(" HH:mm:ss:SSS");
    }

    public void
    print(String message)
    {
        System.err.print(message + _lineSeparator);
    }

    public void
    trace(String category, String message)
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
        System.err.print(s.toString() + _lineSeparator);
    }

    public void
    warning(String message)
    {
        StringBuffer s = new StringBuffer();
        s.append(_date.format(new java.util.Date()));
        s.append(_time.format(new java.util.Date()));
        s.append(' ');
        s.append(_prefix);
        s.append("warning: ");
        s.append(Thread.currentThread().getName() + ": ");
        s.append(message);
        System.err.print(s.toString() + _lineSeparator);
    }

    public void
    error(String message)
    {
        StringBuffer s = new StringBuffer();
        s.append(_date.format(new java.util.Date()));
        s.append(_time.format(new java.util.Date()));
        s.append(' ');
        s.append(_prefix);
        s.append("error: ");
        s.append(Thread.currentThread().getName() + ": ");
        s.append(message);
        System.err.print(s.toString() + _lineSeparator);
    }

    String _prefix = "";
    String _lineSeparator;
    java.text.DateFormat _date;
    java.text.SimpleDateFormat _time;
}
