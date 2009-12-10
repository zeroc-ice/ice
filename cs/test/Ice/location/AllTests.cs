// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;
using System.Collections.Generic;

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
                                        communicator.stringToProxy("ServerManager :default -p 12010"));
        test(manager != null);
        TestLocatorPrx locator = TestLocatorPrxHelper.uncheckedCast(communicator.getDefaultLocator());
        test(locator != null);
        TestLocatorRegistryPrx registry = TestLocatorRegistryPrxHelper.checkedCast(locator.getRegistry());
        test(registry != null);
        
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
            obj3.ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        try
        {
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
            obj2.ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        try
        {
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
        obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("test@TestAdapter"));
        HelloPrx hello = obj.getHello();
        test(hello.ice_getAdapterId().Equals("TestAdapter"));
        hello.sayHello();
        hello = obj.getReplicatedHello();
        test(hello.ice_getAdapterId().Equals("ReplicatedAdapter"));
        hello.sayHello();
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing locator request queuing... ");
        Console.Out.Flush();
        hello = (HelloPrx)obj.getReplicatedHello().ice_locatorCacheTimeout(0).ice_connectionCached(false);
        count = locator.getRequestCount();
        hello.ice_ping();
        test(++count == locator.getRequestCount());
        List<Ice.AsyncResult<Test.Callback_Hello_sayHello> > results =
            new List<Ice.AsyncResult<Test.Callback_Hello_sayHello> >();
        for(int i = 0; i < 1000; i++)
        {
            Ice.AsyncResult<Test.Callback_Hello_sayHello> result = hello.begin_sayHello().
                whenCompleted(delegate()
                              {
                              },
                              delegate(Ice.Exception ex)
                              {
                                  test(false);
                              });
            results.Add(result);
        }
        foreach(Ice.AsyncResult<Test.Callback_Hello_sayHello> result in results)
        {
            result.waitForCompleted();
        }
        results.Clear();
        test(locator.getRequestCount() > count && locator.getRequestCount() < count + 999);
        if(locator.getRequestCount() > count + 800)
        {
            Console.Out.Write("queuing = " + (locator.getRequestCount() - count));
        }
        count = locator.getRequestCount();
        hello = (HelloPrx)hello.ice_adapterId("unknown");
        for(int i = 0; i < 1000; i++)
        {
            Ice.AsyncResult<Test.Callback_Hello_sayHello> result = hello.begin_sayHello().
                whenCompleted(delegate()
                              {
                                  test(false);
                              },
                              delegate(Ice.Exception ex)
                              {
                                  test(ex is Ice.NotRegisteredException);
                              });
            results.Add(result);
        }
        foreach(Ice.AsyncResult<Test.Callback_Hello_sayHello> result in results)
        {
            result.waitForCompleted();
        }
        results.Clear();
        // XXX:
        // Take into account the retries.
        test(locator.getRequestCount() > count && locator.getRequestCount() < count + 1999);
        if(locator.getRequestCount() > count + 800)
        {
            Console.Out.Write("queuing = " + (locator.getRequestCount() - count));
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing adapter locator cache... ");
        Console.Out.Flush();
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject == "object adapter");
            test(ex.id.Equals("TestAdapter3"));
        }
        registry.setAdapterDirectProxy("TestAdapter3", locator.findAdapterById("TestAdapter"));
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            registry.setAdapterDirectProxy("TestAdapter3", communicator.stringToProxy("dummy:tcp"));
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
    
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {   
        }
        registry.setAdapterDirectProxy("TestAdapter3", locator.findAdapterById("TestAdapter"));
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing well-known object locator cache... ");
        Console.Out.Flush();
        registry.addObject(communicator.stringToProxy("test3@TestUnknown"));
        try
        {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject == "object adapter");
            test(ex.id.Equals("TestUnknown"));
        }
        registry.addObject(communicator.stringToProxy("test3@TestAdapter4")); // Update
        registry.setAdapterDirectProxy("TestAdapter4", communicator.stringToProxy("dummy:tcp"));
        try
        {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        registry.setAdapterDirectProxy("TestAdapter4", locator.findAdapterById("TestAdapter"));
        try
        {
            communicator.stringToProxy("test3").ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }

        registry.setAdapterDirectProxy("TestAdapter4", communicator.stringToProxy("dummy:tcp"));
        try
        {
            communicator.stringToProxy("test3").ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }

        try
        {
            communicator.stringToProxy("test@TestAdapter4").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        try
        {
            communicator.stringToProxy("test@TestAdapter4").ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {   
        }
        try
        {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        registry.addObject(communicator.stringToProxy("test3@TestAdapter"));
        try
        {
            communicator.stringToProxy("test3").ice_ping();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        
        registry.addObject(communicator.stringToProxy("test4"));
        try
        {
            communicator.stringToProxy("test4").ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException)
        {
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing locator cache background updates... ");
        Console.Out.Flush();
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.BackgroundLocatorCacheUpdates", "1");
            Ice.Communicator ic = Ice.Util.initialize(initData);

            registry.setAdapterDirectProxy("TestAdapter5", locator.findAdapterById("TestAdapter"));
            registry.addObject(communicator.stringToProxy("test3@TestAdapter"));

            count = locator.getRequestCount();
            ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
            ic.stringToProxy("test3").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
            count += 3;
            test(count == locator.getRequestCount());
            registry.setAdapterDirectProxy("TestAdapter5", null);
            registry.addObject(communicator.stringToProxy("test3:tcp"));
            ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
            ic.stringToProxy("test3").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
            test(count == locator.getRequestCount());
            System.Threading.Thread.Sleep(new System.TimeSpan(10 * 1200 * 1000));

            // The following request should trigger the background
            // updates but still use the cached endpoints and
            // therefore succeed.
            ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
            ic.stringToProxy("test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.

            try
            {
                while(true)
                {
                    ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                    System.Threading.Thread.Sleep(new System.TimeSpan(10 * 10 * 1000));
                }
            }
            catch(Ice.LocalException)
            {
                // Expected to fail once they endpoints have been updated in the background.
            }
            try
            {
                while(true)
                {
                    ic.stringToProxy("test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                    System.Threading.Thread.Sleep(new System.TimeSpan(10 * 10 * 1000));
                }
            }
            catch(Ice.LocalException)
            {
                // Expected to fail once they endpoints have been updated in the background.
            }
            ic.destroy();
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing proxy from server after shutdown... ");
        Console.Out.Flush();
        hello = obj.getReplicatedHello();
        obj.shutdown();
        manager.startServer();
        hello.sayHello();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing object migration... ");
        Console.Out.Flush();
        hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello"));
        obj.migrateHello();
        hello.sayHello();
        obj.migrateHello();
        hello.sayHello();
        obj.migrateHello();
        hello.sayHello();
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
        }
        try
        {
            obj3.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        try
        {
            obj5.ice_ping();
            test(false);
        }
        catch(Ice.LocalException)
        {
        }
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("testing indirect proxies to collocated objects... ");
        Console.Out.Flush();

        //
        // Set up test for calling a collocated object through an
        // indirect, adapterless reference.
        //
        Ice.Properties properties = communicator.getProperties();
        properties.setProperty("Ice.PrintAdapterReady", "0");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Hello", "default");
        adapter.setLocator(locator);

        Ice.Identity id = new Ice.Identity();
        id.name = Guid.NewGuid().ToString();
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
        }
        adapter.deactivate();
        Console.Out.WriteLine("ok");

        Console.Out.Write("shutdown server manager... ");
        Console.Out.Flush();
        manager.shutdown();
        Console.Out.WriteLine("ok");
    }
}
