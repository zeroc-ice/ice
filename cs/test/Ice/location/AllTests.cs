// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


using System;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }
    
    public static void allTests(Ice.Communicator communicator)
    {
        ServerManagerPrx manager = ServerManagerPrxHelper.checkedCast(
					communicator.stringToProxy("ServerManager :default -t 10000 -p 12345"));
        test(manager != null);
        
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        Ice.ObjectPrx base_Renamed = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base3 = communicator.stringToProxy("test");
        Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
        Console.Out.WriteLine("ok");
        
        //
        // Start a server, get the port of the adapter it's listening on,
        // and add it to the configuration so that the client can locate
        // the TestAdapter adapter.
        //
        Console.Out.Write("starting server... ");
        Console.Out.Flush();
        manager.startServer();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestPrx obj = TestPrxHelper.checkedCast(base_Renamed);
        test(obj != null);
        TestPrx obj2 = TestPrxHelper.checkedCast(base2);
        test(obj2 != null);
        TestPrx obj3 = TestPrxHelper.checkedCast(base3);
        test(obj3 != null);
        ServerManagerPrx obj4 = ServerManagerPrxHelper.checkedCast(base4);
        test(obj4 != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing object reference from server... ");
        Console.Out.Flush();
        HelloPrx hello = obj.getHello();
        hello.sayHello();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("shutdown server... ");
        Console.Out.Flush();
        obj.shutdown();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("restarting server... ");
        Console.Out.Flush();
        manager.startServer();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing whether server is still reachable... ");
        Console.Out.Flush();
        try
        {
            obj2.ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing object reference from server... ");
        Console.Out.Flush();
        hello.sayHello();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing reference with unknown identity...");
        Console.Out.Flush();
        try
        {
            base_Renamed = communicator.stringToProxy("unknown/unknown");
            base_Renamed.ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object"));
            test(ex.id.Equals("unknown/unknown"));
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing reference with unknown adapter...");
        Console.Out.Flush();
        try
        {
            base_Renamed = communicator.stringToProxy("test @ TestAdapterUnknown");
            base_Renamed.ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object adapter"));
            test(ex.id.Equals("TestAdapterUnknown"));
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("shutdown server... ");
        Console.Out.Flush();
        obj.shutdown();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing whether server is gone... ");
        Console.Out.Flush();
        try
        {
            obj2.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }
        
        Console.Out.Write("shutdown server manager...");
        Console.Out.Flush();
        manager.shutdown();
        Console.Out.WriteLine("ok");
    }
}
