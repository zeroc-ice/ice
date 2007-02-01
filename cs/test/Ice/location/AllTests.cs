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
        if(!b)
        {
            throw new Exception();
        }
    }
    
    public static void allTests(Ice.Communicator communicator)
    {
        ServerManagerPrx manager = ServerManagerPrxHelper.checkedCast(
                                        communicator.stringToProxy("ServerManager :default -t 10000 -p 12010"));
        test(manager != null);
        TestLocatorPrx locator = TestLocatorPrxHelper.uncheckedCast(communicator.getDefaultLocator());
        test(locator != null);
        
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        Ice.ObjectPrx @base = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base3 = communicator.stringToProxy("test");
        Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
        Ice.ObjectPrx base5 = communicator.stringToProxy("test2");
        Ice.ObjectPrx base6 = communicator.stringToProxy("test @ ReplicatedAdapter");
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing ice_locator and ice_getLocator... ");
        test(Ice.Util.proxyIdentityCompare(@base.ice_getLocator(), communicator.getDefaultLocator()) == 0);
        Ice.LocatorPrx anotherLocator = 
            Ice.LocatorPrxHelper.uncheckedCast(communicator.stringToProxy("anotherLocator"));
        @base = @base.ice_locator(anotherLocator);
        test(Ice.Util.proxyIdentityCompare(@base.ice_getLocator(), anotherLocator) == 0);
        communicator.setDefaultLocator(null);
        @base = communicator.stringToProxy("test @ TestAdapter");
        test(@base.ice_getLocator() == null);
        @base = @base.ice_locator(anotherLocator);
        test(Ice.Util.proxyIdentityCompare(@base.ice_getLocator(), anotherLocator) == 0);
        communicator.setDefaultLocator(locator);
        @base = communicator.stringToProxy("test @ TestAdapter");
        test(Ice.Util.proxyIdentityCompare(@base.ice_getLocator(), communicator.getDefaultLocator()) == 0); 
        
        //
        // We also test ice_router/ice_getRouter (perhaps we should add a
        // test/Ice/router test?)
        //
        test(@base.ice_getRouter() == null);
        Ice.RouterPrx anotherRouter = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("anotherRouter"));
        @base = @base.ice_router(anotherRouter);
        test(Ice.Util.proxyIdentityCompare(@base.ice_getRouter(), anotherRouter) == 0);
        Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("dummyrouter"));
        communicator.setDefaultRouter(router);
        @base = communicator.stringToProxy("test @ TestAdapter");
        test(Ice.Util.proxyIdentityCompare(@base.ice_getRouter(), communicator.getDefaultRouter()) == 0);
        communicator.setDefaultRouter(null);
        @base = communicator.stringToProxy("test @ TestAdapter");
        test(@base.ice_getRouter() == null);
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

        Console.Out.Write("testing proxy with unknown identity... ");
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
        
        Console.Out.Write("testing proxy with unknown adapter... ");
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
        
        Console.Out.Write("testing locator cache timeout... ");
        Console.Out.Flush();
        
        int count = locator.getRequestCount();
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(count == locator.getRequestCount());
        System.Threading.Thread.Sleep(new System.TimeSpan(10 * 1200 * 1000)); // 1200ms
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(++count == locator.getRequestCount());
        
        communicator.stringToProxy("test").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        count += 2;
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        test(count == locator.getRequestCount());
        System.Threading.Thread.Sleep(new System.TimeSpan(10 * 1200 * 1000)); // 1200ms
        communicator.stringToProxy("test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        count += 2;
        test(count == locator.getRequestCount());
        
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(-1).ice_ping(); 
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test").ice_locatorCacheTimeout(-1).ice_ping();
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter").ice_ping(); 
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test").ice_ping();
        test(count == locator.getRequestCount());

        test(communicator.stringToProxy("test").ice_locatorCacheTimeout(99).ice_getLocatorCacheTimeout() == 99);

        Console.Out.WriteLine("ok");

        Console.Out.Write("testing proxy from server... ");
        Console.Out.Flush();
        HelloPrx hello = obj.getHello();
        hello.sayHello();
        test(hello.ice_getAdapterId().Equals("TestAdapter"));
        hello = obj.getReplicatedHello();
        hello.sayHello();
        test(hello.ice_getAdapterId().Equals("ReplicatedAdapter"));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing proxy from server after shutdown... ");
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
        
        Console.Out.Write("testing indirect proxies to collocated objects... ");
        Console.Out.Flush();

        Ice.Properties properties = communicator.getProperties();
        properties.setProperty("Ice.PrintAdapterReady", "0");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Hello", "default");
        adapter.setLocator(locator);
        TestLocatorRegistryPrx registry = TestLocatorRegistryPrxHelper.checkedCast(locator.getRegistry());
        test(registry != null);

        Ice.Identity id = new Ice.Identity();
        id.name = Ice.Util.generateUUID();
        registry.addObject(adapter.add(new HelloI(), id));
        adapter.activate();

        try
        {
            HelloPrx helloPrx = HelloPrxHelper.checkedCast(
                communicator.stringToProxy("\"" + communicator.identityToString(id) + "\""));
            helloPrx.ice_getConnection();
            test(false);
        }
        catch(Ice.CollocationOptimizationException)
        {
            Console.Out.WriteLine("ok");
        }
        adapter.deactivate();

        Console.Out.Write("shutdown server manager... ");
        Console.Out.Flush();
        manager.shutdown();
        Console.Out.WriteLine("ok");
    }
}
