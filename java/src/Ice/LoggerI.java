// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

        _lineSeparator = System.getProperty("line.separator");
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
        StringBuilder s = new StringBuilder(256);
        s.append("[ ");
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
        s.append(_lineSeparator);
        System.err.print(s.toString());
    }

    public void
    warning(String message)
    {
        StringBuilder s = new StringBuilder(256);
        s.append(_date.format(new java.util.Date()));
        s.append(_time.format(new java.util.Date()));
        s.append(' ');
        s.append(_prefix);
        s.append("warning: ");
        s.append(Thread.currentThread().getName());
        s.append(": ");
        s.append(message);
        s.append(_lineSeparator);
        System.err.print(s.toString());
    }

    public void
    error(String message)
    {
        StringBuilder s = new StringBuilder(256);
        s.append(_date.format(new java.util.Date()));
        s.append(_time.format(new java.util.Date()));
        s.append(' ');
        s.append(_prefix);
        s.append("error: ");
        s.append(Thread.currentThread().getName());
        s.append(": ");
        s.append(message);
        s.append(_lineSeparator);
        System.err.print(s.toString());
    }

    String _prefix = "";
    String _lineSeparator;
    java.text.DateFormat _date;
    java.text.SimpleDateFormat _time;
}
