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
        String s = "[ " + category + ": Thread(" + Thread.currentThread().getName() + ") ";
        // TODO: Better way to do this?
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
        System.err.println("warning: " + message);
    }

    public synchronized void
    error(String message)
    {
        System.err.println("error: " + message);
    }
}
