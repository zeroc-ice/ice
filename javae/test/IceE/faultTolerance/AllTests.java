// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    allTests(Ice.Communicator communicator, int[] ports)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test";
        for(int i = 0; i < ports.length; i++)
        {
            ref += ":default -t 60000 -p " + ports[i];
        }
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        System.out.println("ok");

        int oldPid = 0;
        for(int i = 1, j = 0; i <= ports.length; ++i, ++j)
        {
	    if(j > 3)
	    {
		j = 0;
	    }

	    System.out.print("testing server #" + i + "... ");
	    System.out.flush();
	    int pid = obj.pid();
	    test(pid != oldPid);
	    System.out.println("ok");
	    oldPid = pid;

            if(j == 0)
            {
		System.out.print("shutting down server #" + i + "... ");
		System.out.flush();
		obj.shutdown();
		System.out.println("ok");
            }
            else if(j == 1 || i + 1 > ports.length)
            {
		System.out.print("aborting server #" + i + "... ");
		System.out.flush();
		try
		{
		    obj.abort();
		    test(false);
		}
		catch(Ice.ConnectionLostException ex)
		{
		    System.out.println("ok");
		}
		catch(Ice.ConnectFailedException exc)
		{
		    System.out.println("ok");
		}
		catch(Ice.SocketException ex)
		{
		    System.out.println("ok");
		}
            }
            else if(j == 2)
            {
		System.out.print("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
		System.out.flush();
		try
		{
		    obj.idempotentAbort();
		    test(false);
		}
		catch(Ice.ConnectionLostException ex)
		{
		    System.out.println("ok");
		}
		catch(Ice.ConnectFailedException exc)
		{
		    System.out.println("ok");
		}
		catch(Ice.SocketException ex)
		{
		    System.out.println("ok");
		}

                ++i;
            }
            else if(j == 3)
            {
		System.out.print("aborting server #" + i + " and #" + (i + 1) + " with nonmutating call... ");
		System.out.flush();
		try
		{
		    obj.nonmutatingAbort();
		    test(false);
		}
		catch(Ice.ConnectionLostException ex)
		{
		    System.out.println("ok");
		}
		catch(Ice.ConnectFailedException exc)
		{
		    System.out.println("ok");
		}
		catch(Ice.SocketException ex)
		{
		    System.out.println("ok");
		}

                ++i;
            }
            else
            {
                IceUtil.Debug.Assert(false);
            }
        }

        System.out.print("testing whether all servers are gone... ");
        System.out.flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
        }
    }
}
