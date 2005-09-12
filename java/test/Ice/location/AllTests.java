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
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(Ice.Communicator communicator)
    {
	ServerManagerPrx manager = ServerManagerPrxHelper.checkedCast(
	    communicator.stringToProxy("ServerManager :default -t 10000 -p 12345"));
	test(manager != null);

	System.out.print("testing stringToProxy... ");
        System.out.flush();
	Ice.ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
	Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
	Ice.ObjectPrx base3 = communicator.stringToProxy("test");
	Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
	Ice.ObjectPrx base5 = communicator.stringToProxy("test2");
	System.out.println("ok");

	//
	// Start a server, get the port of the adapter it's listening on,
	// and add it to the configuration so that the client can locate
	// the TestAdapter adapter.
	//
	System.out.print("starting server... ");
        System.out.flush();
	manager.startServer();
	System.out.println("ok");

	System.out.print("testing checked cast... ");
        System.out.flush();
	TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
	test(obj != null);
	TestIntfPrx obj2 = TestIntfPrxHelper.checkedCast(base2);
	test(obj2 != null);
	TestIntfPrx obj3 = TestIntfPrxHelper.checkedCast(base3);
	test(obj3 != null);
	ServerManagerPrx obj4 = ServerManagerPrxHelper.checkedCast(base4);
	test(obj4 != null);
	TestIntfPrx obj5 = TestIntfPrxHelper.checkedCast(base5);
	test(obj5 != null);
	System.out.println("ok");
 
	System.out.print("testing id@AdapterId indirect proxy... ");
        System.out.flush();
	obj.shutdown();
	manager.startServer();
	try
	{
	    obj2.ice_ping();
	}
	catch(Ice.LocalException ex)
	{
	    test(false);
	}
	System.out.println("ok");    
    
	System.out.print("testing identity indirect proxy... ");
        System.out.flush();
	obj.shutdown();
	manager.startServer();
	try
	{
	    obj3 = TestIntfPrxHelper.checkedCast(base3);
	    obj3.ice_ping();
	}
	catch(Ice.LocalException ex)
	{
	    test(false);
	}
	try
	{
	    obj2 = TestIntfPrxHelper.checkedCast(base2);
	    obj2.ice_ping();
	}
	catch(Ice.LocalException ex)
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
	catch(Ice.LocalException ex)
	{
	    test(false);
	}
	try
	{
	    obj3 = TestIntfPrxHelper.checkedCast(base3);
	    obj3.ice_ping();
	}
	catch(Ice.LocalException ex)
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
	catch(Ice.LocalException ex)
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
	catch(Ice.LocalException ex)
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
	catch(Ice.LocalException ex)
	{
	    test(false);
	}

	System.out.println("ok");    

	System.out.print("testing reference with unknown identity... ");
	System.out.flush();
	try
	{
	    base = communicator.stringToProxy("unknown/unknown");
	    base.ice_ping();
	    test(false);
	}
	catch(Ice.NotRegisteredException ex)
	{
	    test(ex.kindOfObject.equals("object"));
	    test(ex.id.equals("unknown/unknown"));
	}
	System.out.println("ok");	

	System.out.print("testing reference with unknown adapter... ");
	System.out.flush();
	try
	{
	    base = communicator.stringToProxy("test @ TestAdapterUnknown");
	    base.ice_ping();
	    test(false);
	}
	catch(Ice.NotRegisteredException ex)
	{
	    test(ex.kindOfObject.equals("object adapter"));
	    test(ex.id.equals("TestAdapterUnknown"));
	}
	System.out.println("ok");	

	System.out.print("testing object reference from server... ");
        System.out.flush();
	HelloPrx hello = obj.getHello();
	hello.sayHello();
	System.out.println("ok");

	System.out.print("testing object reference from server after shutdown... ");
        System.out.flush();
	obj.shutdown();
	manager.startServer();
	hello.sayHello();
	System.out.println("ok");

	System.out.print("testing object migration...");
	System.out.flush();
	hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello"));
	obj.migrateHello();
	hello.sayHello();
	obj.migrateHello();
	hello.sayHello();
	obj.migrateHello();
	hello.sayHello();
	System.out.println("ok");

	System.out.print("testing whether server is gone... ");
        System.out.flush();
	obj.shutdown();
	try
	{
	    obj2.ice_ping();
	    test(false);
	}
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
        }

	System.out.print("testing indirect references to collocated objects... ");
	Ice.Properties properties = communicator.getProperties();
	properties.setProperty("Ice.PrintAdapterReady", "0");
	properties.setProperty("Hello.Endpoints",
			       properties.getPropertyWithDefault("Hello.Endpoints",
								 "default -p 10001"));
	Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Hello");
	Ice.LocatorPrx locator =
	    Ice.LocatorPrxHelper.uncheckedCast(communicator.stringToProxy(
						   properties.getProperty("Ice.Default.Locator")));
	adapter.setLocator(locator);
	TestLocatorRegistryPrx registry = TestLocatorRegistryPrxHelper.checkedCast(locator.getRegistry());
	test(registry != null);

	Ice.Identity id = new Ice.Identity();
	id.name = Ice.Util.generateUUID().replace(':', '-');
	registry.addObject(adapter.add(new HelloI(), id));
	adapter.activate();

	try
	{
	    HelloPrx helloPrx = HelloPrxHelper.checkedCast(communicator.stringToProxy(id.name));
	    Ice.Connection connection = helloPrx.ice_connection();
	    test(false);
	}
	catch(Ice.CollocationOptimizationException ex)
	{
	    System.out.println("ok");
	}
	adapter.deactivate();

	System.out.print("shutdown server manager... ");
        System.out.flush();
	manager.shutdown();
	System.out.println("ok");
    }
}
