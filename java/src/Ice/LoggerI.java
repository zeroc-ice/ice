// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class LoggerI implements Logger
{
    public synchronized void
    trace(String category, String message)
    {
        String s = "[ " + category + ": ";
        int start = 0;
        int next;
        while((next = message.indexOf('\n', start)) != -1)
        {
            s += message.substring(start, next + 1);
            s += "  ";
            start = next + 1;
        }
        s += message.substring(start);
        s += " ]";
        System.err.println(s);
    }

    public synchronized void
    warning(String message)
    {
        System.out.println("warning: " + message);
    }

    public synchronized void
    error(String message)
    {
        System.err.println("error: " + message);
    }
}
