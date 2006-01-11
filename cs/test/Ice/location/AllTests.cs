// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
        Ice.ObjectPrx @base = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base3 = communicator.stringToProxy("test");
        Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
	Ice.ObjectPrx base5 = communicator.stringToProxy("test2");
	Ice.ObjectPrx base6 = communicator.stringToProxy("test @ ReplicatedAdapter");
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
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        TestIntfPrx obj2 = TestIntfPrxHelper.checkedCast(base2);
        test(obj2 != null);
        TestIntfPrx obj3 = TestIntfPrxHelper.checkedCast(base3);
        test(obj3 != null);
        ServerManagerPrx obj4 = ServerManagerPrxHelper.checkedCast(base4);
        test(obj4 != null);
	TestIntfPrx obj5 = TestIntfPrxHelper.checkedCast(base5);
	test(obj5 != null);
	TestIntfPrx obj6 = TestIntfPrxHelper.checkedCast(base6);
	test(obj6 != null);
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing id@AdapterId indirect proxy... ");
        Console.Out.Flush();
        obj.shutdown();
        manager.startServer();
        try
        {
            obj2.ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing id@ReplicaGroupId indirect proxy... ");
        Console.Out.Flush();
        obj.shutdown();
        manager.startServer();
        try
        {	
            obj6.ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing identity indirect proxy... ");
        Console.Out.Flush();
        obj.shutdown();
        manager.startServer();
	try
	{
	    obj3 = TestIntfPrxHelper.checkedCast(base3);
	    obj3.ice_ping();
	}
	catch(Ice.LocalException)
	{
	    test(false);
	}
	try
	{
	    obj2 = TestIntfPrxHelper.checkedCast(base2);
	    obj2.ice_ping();
	}
	catch(Ice.LocalException)
	{
	    test(false);
	}
	obj.shutdown();
	manager.startServer();
	try
	{
	    obj2 = TestIntfPrxHelper.checkedCast(base2);
	    obj2.ice_ping();
	}
	catch(Ice.LocalException)
	{
	    test(false);
	}
	try
	{
	    obj3 = TestIntfPrxHelper.checkedCast(base3);
	    obj3.ice_ping();
	}
	catch(Ice.LocalException)
	{
	    test(false);
	}
	obj.shutdown();
	manager.startServer();
	try
	{
	    obj2 = TestIntfPrxHelper.checkedCast(base2);
	    obj2.ice_ping();
	}
	catch(Ice.LocalException)
	{
	    test(false);
	}
	obj.shutdown();
	manager.startServer();
	try
	{
	    obj3 = TestIntfPrxHelper.checkedCast(base2);
	    obj3.ice_ping();
	}
	catch(Ice.LocalException)
	{
	    test(false);
	}
	obj.shutdown();
	manager.startServer();
	try
	{
	    obj5 = TestIntfPrxHelper.checkedCast(base5);
	    obj5.ice_ping();
	}
	catch(Ice.LocalException)
	{
	    test(false);
	}
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing reference with unknown identity... ");
        Console.Out.Flush();
        try
        {
            @base = communicator.stringToProxy("unknown/unknown");
            @base.ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object"));
            test(ex.id.Equals("unknown/unknown"));
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing reference with unknown adapter... ");
        Console.Out.Flush();
        try
        {
            @base = communicator.stringToProxy("test @ TestAdapterUnknown");
            @base.ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object adapter"));
            test(ex.id.Equals("TestAdapterUnknown"));
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing object reference from server... ");
        Console.Out.Flush();
        HelloPrx hello = obj.getHello();
        hello.sayHello();
	test(hello.ice_getAdapterId().Equals("TestAdapter"));
	hello = obj.getReplicatedHello();
        hello.sayHello();
	test(hello.ice_getAdapterId().Equals("ReplicatedAdapter"));
        Console.Out.WriteLine("ok");
        
	Console.Out.Write("testing object reference from server after shutdown... ");
        Console.Out.Flush();
	obj.shutdown();
	manager.startServer();
	hello.sayHello();
	Console.Out.WriteLine("ok");

	Console.Out.Write("testing object migration...");
	Console.Out.Flush();
	hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello"));
 	obj.migrateHello();
	hello.sayHello();
 	obj.migrateHello();
 	hello.sayHello();
 	obj.migrateHello();
 	hello.sayHello();
	Console.Out.WriteLine("ok");

        Console.Out.Write("testing whether server is gone... ");
        Console.Out.Flush();
        obj.shutdown();
        try
        {
            obj2.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }
        
        Console.Out.Write("shutdown server manager... ");
        Console.Out.Flush();
        manager.shutdown();
        Console.Out.WriteLine("ok");
    }
}
