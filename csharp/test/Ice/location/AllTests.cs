// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;
using System.Collections.Generic;

public class AllTests : TestCommon.AllTests
{
    public static void allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        ServerManagerPrx manager = ServerManagerPrxHelper.checkedCast(
                                        communicator.stringToProxy("ServerManager :" + app.getTestEndpoint(0)));
        test(manager != null);
        TestLocatorPrx locator = TestLocatorPrxHelper.uncheckedCast(communicator.getDefaultLocator());
        test(locator != null);
        TestLocatorRegistryPrx registry = TestLocatorRegistryPrxHelper.checkedCast(locator.getRegistry());
        test(registry != null);

        Write("testing stringToProxy... ");
        Flush();
        Ice.ObjectPrx @base = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base3 = communicator.stringToProxy("test");
        Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
        Ice.ObjectPrx base5 = communicator.stringToProxy("test2");
        Ice.ObjectPrx base6 = communicator.stringToProxy("test @ ReplicatedAdapter");
        WriteLine("ok");

        Write("testing ice_locator and ice_getLocator... ");
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
        WriteLine("ok");

        Write("starting server... ");
        Flush();
        manager.startServer();
        WriteLine("ok");

        Write("testing checked cast... ");
        Flush();
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
        WriteLine("ok");

        Write("testing id@AdapterId indirect proxy... ");
        Flush();
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
        WriteLine("ok");

        Write("testing id@ReplicaGroupId indirect proxy... ");
        Flush();
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
        WriteLine("ok");

        Write("testing identity indirect proxy... ");
        Flush();
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
        WriteLine("ok");

        Write("testing proxy with unknown identity... ");
        Flush();
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
        WriteLine("ok");

        Write("testing proxy with unknown adapter... ");
        Flush();
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
        WriteLine("ok");

        Write("testing locator cache timeout... ");
        Flush();

        int count = locator.getRequestCount();
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(count == locator.getRequestCount());
        System.Threading.Thread.Sleep(1200); // 1200ms
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(++count == locator.getRequestCount());

        communicator.stringToProxy("test").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        count += 2;
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        test(count == locator.getRequestCount());
        System.Threading.Thread.Sleep(1200); // 1200ms
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

        WriteLine("ok");

        Write("testing proxy from server... ");
        Flush();
        obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("test@TestAdapter"));
        HelloPrx hello = obj.getHello();
        test(hello.ice_getAdapterId().Equals("TestAdapter"));
        hello.sayHello();
        hello = obj.getReplicatedHello();
        test(hello.ice_getAdapterId().Equals("ReplicatedAdapter"));
        hello.sayHello();
        WriteLine("ok");

        Write("testing locator request queuing... ");
        Flush();
        hello = (HelloPrx)obj.getReplicatedHello().ice_locatorCacheTimeout(0).ice_connectionCached(false);
        count = locator.getRequestCount();
        hello.ice_ping();
        test(++count == locator.getRequestCount());
        List<Ice.AsyncResult<Test.Callback_Hello_sayHello> > results =
            new List<Ice.AsyncResult<Test.Callback_Hello_sayHello> >();
        for(int i = 0; i < 1000; i++)
        {
            Ice.AsyncResult<Test.Callback_Hello_sayHello> result = hello.begin_sayHello().
                whenCompleted(
                    () =>
                    {
                    },
                    (Ice.Exception ex) =>
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
            Write("queuing = " + (locator.getRequestCount() - count));
        }
        count = locator.getRequestCount();
        hello = (HelloPrx)hello.ice_adapterId("unknown");
        for(int i = 0; i < 1000; i++)
        {
            Ice.AsyncResult<Test.Callback_Hello_sayHello> result = hello.begin_sayHello().
                whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception ex) =>
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
            Write("queuing = " + (locator.getRequestCount() - count));
        }
        WriteLine("ok");

        Write("testing adapter locator cache... ");
        Flush();
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
        WriteLine("ok");

        Write("testing well-known object locator cache... ");
        Flush();
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
        WriteLine("ok");

        Write("testing locator cache background updates... ");
        Flush();
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
            System.Threading.Thread.Sleep(1200);

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
                    System.Threading.Thread.Sleep(10);
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
                    System.Threading.Thread.Sleep(10);
                }
            }
            catch(Ice.LocalException)
            {
                // Expected to fail once they endpoints have been updated in the background.
            }
            ic.destroy();
        }
        WriteLine("ok");

        Write("testing proxy from server after shutdown... ");
        Flush();
        hello = obj.getReplicatedHello();
        obj.shutdown();
        manager.startServer();
        hello.sayHello();
        WriteLine("ok");

        Write("testing object migration... ");
        Flush();
        hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello"));
        obj.migrateHello();
        hello.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);
        hello.sayHello();
        obj.migrateHello();
        hello.sayHello();
        obj.migrateHello();
        hello.sayHello();
        WriteLine("ok");

        Write("testing locator encoding resolution... ");
        Flush();
        hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello"));
        count = locator.getRequestCount();
        communicator.stringToProxy("test@TestAdapter").ice_encodingVersion(Ice.Util.Encoding_1_1).ice_ping();
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter10").ice_encodingVersion(Ice.Util.Encoding_1_0).ice_ping();
        test(++count == locator.getRequestCount());
        communicator.stringToProxy("test -e 1.0@TestAdapter10-2").ice_ping();
        test(++count == locator.getRequestCount());
        WriteLine("ok");

        Write("shutdown server... ");
        Flush();
        obj.shutdown();
        WriteLine("ok");

        Write("testing whether server is gone... ");
        Flush();
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
        WriteLine("ok");

        Write("testing indirect proxies to collocated objects... ");
        Flush();

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

        HelloPrx helloPrx = HelloPrxHelper.checkedCast(
            communicator.stringToProxy("\"" + communicator.identityToString(id) + "\""));
        test(helloPrx.ice_getConnection() == null);

        adapter.deactivate();
        WriteLine("ok");

        Write("shutdown server manager... ");
        Flush();
        manager.shutdown();
        WriteLine("ok");
    }
}
