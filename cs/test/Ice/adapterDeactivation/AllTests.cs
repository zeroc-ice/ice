// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new System.Exception();
        }
    }
    
    public static TestIntfPrx allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string @ref = "test:default -p 12010 -t 2000";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        test(obj.Equals(@base));
        Console.Out.WriteLine("ok");

        {
            Console.Out.Write("creating/destroying/recreating object adapter... ");
            Console.Out.Flush();
            Ice.ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999");
            try
            {
                communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9998");
                test(false);
            }
            catch(Ice.AlreadyRegisteredException)
            {
            }
            adapter.destroy();

            //
            // Use a different port than the first adapter to avoid an "address already in use" error.
            //
            adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9998");
            adapter.destroy();
            Console.Out.WriteLine("ok");
        }
        
        Console.Out.Write("creating/activating/deactivating object adapter in one operation... ");
        Console.Out.Flush();
        obj.transient();
        Console.Out.WriteLine("ok");

        Console.Out.Write("deactivating object adapter in the server... ");
        Console.Out.Flush();
        obj.deactivate();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing whether server is gone... ");
        Console.Out.Flush();
        try
        {
            obj.ice_ping();
            throw new System.ApplicationException();
        }
        catch(Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }
        
        return obj;
    }
}
