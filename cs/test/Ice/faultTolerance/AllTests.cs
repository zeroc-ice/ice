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
            refString += ":default -t 4000 -p " + ports[i];
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
        for(int i = 1, j = 0; i <= ports.Count; ++i, j = j >= 3 ? 0 : j + 1)
        {
            Console.Out.Write("testing server #" + i + "... ");
            Console.Out.Flush();
            int pid = obj.pid();
            test(pid != oldPid);
            Console.Out.WriteLine("ok");
            
            if(j == 0)
            {
                Console.Out.Write("shutting down server #" + i + "... ");
                Console.Out.Flush();
                obj.shutdown();
                Console.Out.WriteLine("ok");
            }
            else if(j == 1 || i + 1 > ports.Count)
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
            }
            else if(j == 2)
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
                ++i;
            }
            else if(j == 3)
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
