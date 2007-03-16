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
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
	String serverManagerRef = communicator.getProperties().getPropertyWithDefault("Test.ServerManager", 
	    "ServerManager :default -t 10000 -p 12010");
	ServerManagerPrx manager = ServerManagerPrxHelper.checkedCast(communicator.stringToProxy(serverManagerRef));
	test(manager != null);
	Ice.LocatorPrx locator = Ice.LocatorPrxHelper.uncheckedCast(communicator.getDefaultLocator());
	test(locator != null);

	out.print("testing stringToProxy... ");
        out.flush();
	Ice.ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
	Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
	Ice.ObjectPrx base3 = communicator.stringToProxy("test");
	Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
	Ice.ObjectPrx base5 = communicator.stringToProxy("test2");
	Ice.ObjectPrx base6 = communicator.stringToProxy("test @ ReplicatedAdapter");
	out.println("ok");

	System.out.print("testing ice_locator and ice_getLocator... ");
	test(Ice.Util.proxyIdentityCompare(base.ice_getLocator(), communicator.getDefaultLocator()) == 0);
	Ice.LocatorPrx anotherLocator = 
	    Ice.LocatorPrxHelper.uncheckedCast(communicator.stringToProxy("anotherLocator"));
	base = base.ice_locator(anotherLocator);
	test(Ice.Util.proxyIdentityCompare(base.ice_getLocator(), anotherLocator) == 0);
	communicator.setDefaultLocator(null);
	base = communicator.stringToProxy("test @ TestAdapter");
	test(base.ice_getLocator() == null);
	base = base.ice_locator(anotherLocator);
	test(Ice.Util.proxyIdentityCompare(base.ice_getLocator(), anotherLocator) == 0);
	communicator.setDefaultLocator(locator);
	base = communicator.stringToProxy("test @ TestAdapter");
	test(Ice.Util.proxyIdentityCompare(base.ice_getLocator(), communicator.getDefaultLocator()) == 0); 
	
	//
	// We also test ice_router/ice_getRouter (perhaps we should add a
	// test/Ice/router test?)
	//
	test(base.ice_getRouter() == null);
	Ice.RouterPrx anotherRouter = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("anotherRouter"));
	base = base.ice_router(anotherRouter);
	test(Ice.Util.proxyIdentityCompare(base.ice_getRouter(), anotherRouter) == 0);
	Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("dummyrouter"));
	communicator.setDefaultRouter(router);
	base = communicator.stringToProxy("test @ TestAdapter");
	test(Ice.Util.proxyIdentityCompare(base.ice_getRouter(), communicator.getDefaultRouter()) == 0);
	communicator.setDefaultRouter(null);
	base = communicator.stringToProxy("test @ TestAdapter");
	test(base.ice_getRouter() == null);
	System.out.println("ok");

	//
	// Start a server, get the port of the adapter it's listening on,
	// and add it to the configuration so that the client can locate
	// the TestAdapter adapter.
	//
	out.print("starting server... ");
        out.flush();
	manager.startServer();
	out.println("ok");

	out.print("testing checked cast... ");
        out.flush();
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
	TestIntfPrx obj6 = TestIntfPrxHelper.checkedCast(base6);
	test(obj6 != null);
	out.println("ok");
 
	out.print("testing id@AdapterId indirect proxy... ");
        out.flush();
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
	out.println("ok");    
    
	out.print("testing id@ReplicaGroupId indirect proxy... ");
        out.flush();
	obj.shutdown();
	manager.startServer();
	try
	{
	    obj6.ice_ping();
	}
	catch(Ice.LocalException ex)
	{
	    test(false);
	}
	out.println("ok");    
    
	out.print("testing identity indirect proxy... ");
        out.flush();
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

	out.println("ok");    

	out.print("testing reference with unknown identity... ");
	out.flush();
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
	out.println("ok");	

	out.print("testing reference with unknown adapter... ");
	out.flush();
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
	out.println("ok");	

	out.print("testing object reference from server... ");
        out.flush();
	HelloPrx hello = obj.getHello();
	hello.sayHello();
	test(communicator.proxyToString(hello).indexOf("TestAdapter") != -1);
	hello = obj.getReplicatedHello();
	hello.sayHello();
	test(communicator.proxyToString(hello).indexOf("ReplicatedAdapter") != -1);
	out.println("ok");

	out.print("testing object reference from server after shutdown... ");
        out.flush();
	obj.shutdown();
	manager.startServer();
	hello.sayHello();
	out.println("ok");

	out.print("testing object migration...");
	out.flush();
	hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello"));
	obj.migrateHello();
	hello.sayHello();
	obj.migrateHello();
	hello.sayHello();
	obj.migrateHello();
	hello.sayHello();
	out.println("ok");

	out.print("testing whether server is gone... ");
        out.flush();
	obj.shutdown();
	try
	{
	    obj2.ice_ping();
	    test(false);
	}
        catch(Ice.LocalException ex)
        {
            out.println("ok");
        }

	out.print("shutdown server manager... ");
        out.flush();
	manager.shutdown();
	out.println("ok");
    }
}
