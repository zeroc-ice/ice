// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


using System;
using System.Diagnostics;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }
    
    public static void allTests(Ice.Communicator communicator, System.Collections.ArrayList ports)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string refString = "test";
        for(int i = 0; i < ports.Count; i++)
        {
            refString += ":default -t 60000 -p " + ports[i];
        }
        Ice.ObjectPrx basePrx = communicator.stringToProxy(refString);
        test(basePrx != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestPrx obj = TestPrxHelper.checkedCast(basePrx);
        test(obj != null);
        test(obj.Equals(basePrx));
        Console.Out.WriteLine("ok");
        
        int oldPid = 0;
	bool ami = false;
        for(int i = 1, j = 0; i <= ports.Count; ++i, ++j)
        {
	    if(j > 3)
	    {
	        j = 0;
		ami = !ami;
	    }

	    if(!ami)
	    {
		Console.Out.Write("testing server #" + i + "... ");
		Console.Out.Flush();
		int pid = obj.pid();
		test(pid != oldPid);
		Console.Out.WriteLine("ok");
		oldPid = pid;
	    }
	    else
	    {
                /*
		Console.Out.Write("testing server #" + i + " with AMI... ");
		Console.Out.Flush();
		AMI_Test_pidI cb = new AMI_Test_pidI();
		obj.pid_async(cb);
		test(cb.check());
		int pid = cb.pid();
		test(pid != oldPid);
		Console.Out.WriteLine("ok");
		oldPid = pid;
                */
	    }
            
            if(j == 0)
            {
	    	if(!ami)
		{
		    Console.Out.Write("shutting down server #" + i + "... ");
		    Console.Out.Flush();
		    obj.shutdown();
		    Console.Out.WriteLine("ok");
		}
		else
		{
                    /*
		    Console.Out.Write("shutting down server #" + i + " with AMI... ");
		    AMI_Test_shutdownI cb = new AMI_Test_shtudownI();
		    obj.shtudown_async(cb);
		    test(cb.check());
		    Console.Out.WriteLine("ok");
                    */
		}
            }
            else if(j == 1 || i + 1 > ports.Count)
            {
	        if(!ami)
		{
		    Console.Out.Write("aborting server #" + i + "... ");
		    Console.Out.Flush();
		    try
		    {
			obj.abort();
			test(false);
		    }
		    catch(Ice.ConnectionLostException)
		    {
			Console.Out.WriteLine("ok");
		    }
		    catch(Ice.ConnectFailedException)
		    {
			Console.Out.WriteLine("ok");
		    }
		    catch(Ice.SocketException)
		    {
			Console.Out.WriteLine("ok");
		    }
		}
		else
		{
                    /*
		    Console.Out.Write("aborting server #" + i + " with AMI... ");
		    Console.Out.Flush();
		    AMI_Test_abortI cb = new AMI_Test_abortI();
		    obj.abort_async(cb);
		    test(cb.check());
		    Console.Out.WriteLine("ok");
                    */
		}
            }
            else if(j == 2)
            {
	        if(!ami)
		{
		    Console.Out.Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent call... ");
		    Console.Out.Flush();
		    try
		    {
			obj.idempotentAbort();
			test(false);
		    }
		    catch(Ice.ConnectionLostException)
		    {
			Console.Out.WriteLine("ok");
		    }
		    catch(Ice.ConnectFailedException)
		    {
			Console.Out.WriteLine("ok");
		    }
		    catch(Ice.SocketException)
		    {
			Console.Out.WriteLine("ok");
		    }
		}
		else
		{
                    /*
		    Console.Out.Write("aborting server #" + i + " and #" + (i + 1) + " with idempotent AMI call... ");
		    Console.Out.Flush();
		    AMI_Test_idempotentAbortI cb = new AMI_Test_idempotentAbortI();
		    obj.idempotentAbort_async(cb);
		    test(cb.check());
		    Console.Out.WriteLine("ok");
                    */
		}
                ++i;
            }
            else if(j == 3)
            {
	        if(!ami)
		{
		    Console.Out.Write("aborting server #" + i + " and #" + (i + 1) + " with nonmutating call... ");
		    Console.Out.Flush();
		    try
		    {
			obj.nonmutatingAbort();
			test(false);
		    }
		    catch(Ice.ConnectionLostException)
		    {
			Console.Out.WriteLine("ok");
		    }
		    catch(Ice.ConnectFailedException)
		    {
			Console.Out.WriteLine("ok");
		    }
		    catch(Ice.SocketException)
		    {
			Console.Out.WriteLine("ok");
		    }
		}
		else
		{
                    /*
		    Console.Out.Write("aborting server #" + i + " and #" + (i + 1) + " with nonmutating AMI call... ");
		    Console.Out.Flush();
		    AMI_Test_nonmutatingAbortI cb = new AMI_Test_nonmutatingAbortI();
		    obj.nonmutatingAbort_async(cb);
		    test(cb.check());
		    Console.Out.WriteLine("ok");
                    */
		}
                ++i;
            }
            else
            {
                Debug.Assert(false);
            }
        }
        
        Console.Out.Write("testing whether all servers are gone... ");
        Console.Out.Flush();
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }
    }
}
