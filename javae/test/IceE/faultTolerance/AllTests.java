// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public class AllTests
{
    public static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
	Callback()
	{
	    _called = false;
	}

	public synchronized boolean
	check()
	{
	    while(!_called)
	    {
		try
		{
		    wait(30000);
		}
		catch(InterruptedException ex)
		{
		    continue;
		}

		if(!_called)
		{
		    return false; // Must be timeout.
		}
	    }
	    
	    _called = false;
	    return true;
	}
	
	public synchronized void
	called()
	{
	    IceUtil.Debug.Assert(!_called);
	    _called = true;
	    notify();
	}

	private boolean _called;
    }

    public static void
    allTests(Ice.Communicator communicator, int[] ports, java.io.PrintStream out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test";
	String host = communicator.getProperties().getProperty("Test.Host");
	if(host.length() > 0)
	{
	    host = " -h " + host;
	}
        for(int i = 0; i < ports.length; i++)
        {
            ref += ":default -t 60000 -p " + ports[i] + host;
        }
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");

        int oldPid = 0;
        for(int i = 1, j = 0; i <= ports.length; ++i, ++j)
        {
	    if(j > 3)
	    {
		j = 0;
	    }

	    out.print("testing server #" + i + "... ");
	    out.flush();
	    int pid = obj.pid();
	    test(pid != oldPid);
	    out.println("ok");
	    oldPid = pid;

            if(j == 0)
            {
		out.print("shutting down server #" + i + "... ");
		out.flush();
		obj.shutdown();
		out.println("ok");
            }
            else if(j == 1 || i + 1 > ports.length)
            {
		out.print("aborting server #" + i + "... ");
		out.flush();
		try
		{
		    obj.abort();
		    test(false);
		}
		catch(Ice.ConnectionLostException ex)
		{
		    out.println("ok");
		}
		catch(Ice.ConnectFailedException exc)
		{
		    out.println("ok");
		}
		catch(Ice.SocketException ex)
		{
		    out.println("ok");
		}
            }
            else if(j == 2 || j == 3)
            {
		out.print("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
		out.flush();
		try
		{
		    obj.idempotentAbort();
		    test(false);
		}
		catch(Ice.ConnectionLostException ex)
		{
		    out.println("ok");
		}
		catch(Ice.ConnectFailedException exc)
		{
		    out.println("ok");
		}
		catch(Ice.SocketException ex)
		{
		    out.println("ok");
		}

                ++i;
            }
            else
            {
                IceUtil.Debug.Assert(false);
            }
        }

        out.print("testing whether all servers are gone... ");
        out.flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
            out.println("ok");
        }
    }
}
