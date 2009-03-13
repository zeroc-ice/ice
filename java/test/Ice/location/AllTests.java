// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
            communicator.stringToProxy("ServerManager :default -t 10000 -p 12010"));
        test(manager != null);

        TestLocatorPrx locator = TestLocatorPrxHelper.uncheckedCast(communicator.getDefaultLocator());
        test(locator != null);

        TestLocatorRegistryPrx registry = TestLocatorRegistryPrxHelper.checkedCast(locator.getRegistry());
        test(registry != null);

        System.out.print("testing stringToProxy... ");
        System.out.flush();
        Ice.ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
        Ice.ObjectPrx base3 = communicator.stringToProxy("test");
        Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
        Ice.ObjectPrx base5 = communicator.stringToProxy("test2");
        Ice.ObjectPrx base6 = communicator.stringToProxy("test @ ReplicatedAdapter");
        System.out.println("ok");

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
        TestIntfPrx obj6 = TestIntfPrxHelper.checkedCast(base6);
        test(obj6 != null);
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
            ex.printStackTrace();
            test(false);
        }
        System.out.println("ok");    
    
        System.out.print("testing id@ReplicaGroupId indirect proxy... ");
        System.out.flush();
        obj.shutdown();
        manager.startServer();
        try
        {
            obj6 = TestIntfPrxHelper.checkedCast(base6);
            obj6.ice_ping();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
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
            ex.printStackTrace();
            test(false);
        }
        try
        {
            obj2 = TestIntfPrxHelper.checkedCast(base2);
            obj2.ice_ping();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
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
            ex.printStackTrace();
            test(false);
        }
        try
        {
            obj3 = TestIntfPrxHelper.checkedCast(base3);
            obj3.ice_ping();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
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
            ex.printStackTrace();
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
            ex.printStackTrace();
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
            ex.printStackTrace();
            test(false);
        }

        System.out.println("ok");    

        System.out.print("testing proxy with unknown identity... ");
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

        System.out.print("testing proxy with unknown adapter... ");
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

        System.out.print("testing locator cache timeout... ");
        System.out.flush();
        
        int count = locator.getRequestCount();
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        test(++count == locator.getRequestCount());
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(count == locator.getRequestCount());
        try
        {
            Thread.sleep(1200);
        }
        catch(InterruptedException ex)
        {
        }
        communicator.stringToProxy("test@TestAdapter").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
        test(++count == locator.getRequestCount());
        
        communicator.stringToProxy("test").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
        count += 2;
        test(count == locator.getRequestCount());
        communicator.stringToProxy("test").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout
        test(count == locator.getRequestCount());
        try
        {
            Thread.sleep(1200);
        }
        catch(InterruptedException ex)
        {
        }
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
        
        System.out.println("ok");

        System.out.print("testing proxy from server... ");
        System.out.flush();
        obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("test@TestAdapter"));
        HelloPrx hello = obj.getHello();
        test(hello.ice_getAdapterId().equals("TestAdapter"));
        hello = obj.getReplicatedHello();
        test(hello.ice_getAdapterId().equals("ReplicatedAdapter"));
        hello.sayHello();
        System.out.println("ok");

        System.out.print("testing locator request queuing... ");
        System.out.flush();
        hello = (HelloPrx)obj.getReplicatedHello().ice_locatorCacheTimeout(0).ice_connectionCached(false);
        count = locator.getRequestCount();
        hello.ice_ping();
        test(++count == locator.getRequestCount());
        for(int i = 0; i < 1000; i++)
        {
            class AMICallback extends Test.AMI_Hello_sayHello
            {
                public void
                ice_exception(Ice.LocalException ex)
                {
                    ex.printStackTrace();
                    test(false);
                }

                public void
                ice_response()
                {
                }
            };
            hello.sayHello_async(new AMICallback());
        }
        hello.ice_ping();
        test(locator.getRequestCount() > count && locator.getRequestCount() < count + 999);
        if(locator.getRequestCount() > count + 800)
        {
            System.out.print("queuing = " + (locator.getRequestCount() - count));
        }
        count = locator.getRequestCount();
        hello = (HelloPrx)hello.ice_adapterId("unknown");
        for(int i = 0; i < 1000; i++)
        {
            class AMICallback extends Test.AMI_Hello_sayHello
            {
                public void
                ice_exception(Ice.LocalException ex)
                {
                    test(ex instanceof Ice.NotRegisteredException);
                }

                public void
                ice_response()
                {
                    test(false);
                }
            };
            hello.sayHello_async(new AMICallback());
        }
        try
        {
            hello.ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
        }
        // We need to take into account the retries.
        test(locator.getRequestCount() > count && locator.getRequestCount() < count + 1999);
        if(locator.getRequestCount() > count + 800)
        {
            System.out.print("queuing = " + (locator.getRequestCount() - count));
        }
        System.out.println("ok");

        System.out.print("testing adapter locator cache... ");
        System.out.flush();
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject == "object adapter");
            test(ex.id.equals("TestAdapter3"));
        }
        try
        {
            registry.setAdapterDirectProxy("TestAdapter3", locator.findAdapterById("TestAdapter"));
        }
        catch(Ice.AdapterAlreadyActiveException ex)
        {
        }
        catch(Ice.AdapterNotFoundException ex)
        {
        }
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            try
            {
                registry.setAdapterDirectProxy("TestAdapter3", communicator.stringToProxy("dummy:tcp"));
            }
            catch(Ice.AdapterAlreadyActiveException ex)
            {
            }
            catch(Ice.AdapterNotFoundException ex)
            {
            }
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
    
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
        }
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {   
        }
        try
        {
            registry.setAdapterDirectProxy("TestAdapter3", locator.findAdapterById("TestAdapter"));
        }
        catch(Ice.AdapterAlreadyActiveException ex)
        {
        }
        catch(Ice.AdapterNotFoundException ex)
        {
        }
        try
        {
            communicator.stringToProxy("test@TestAdapter3").ice_ping();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
        System.out.println("ok");

        System.out.print("testing well-known object locator cache... ");
        System.out.flush();
        registry.addObject(communicator.stringToProxy("test3@TestUnknown"));
        try
        {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject == "object adapter");
            test(ex.id.equals("TestUnknown"));
        }
        registry.addObject(communicator.stringToProxy("test3@TestAdapter4")); // Update
        try
        {
            registry.setAdapterDirectProxy("TestAdapter4", communicator.stringToProxy("dummy:tcp"));
        }
        catch(Ice.AdapterAlreadyActiveException ex)
        {
        }
        catch(Ice.AdapterNotFoundException ex)
        {
        }

        try
        {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
        }
        try
        {
            registry.setAdapterDirectProxy("TestAdapter4", locator.findAdapterById("TestAdapter"));
        }
        catch(Ice.AdapterAlreadyActiveException ex)
        {
        }
        catch(Ice.AdapterNotFoundException ex)
        {
        }
        try
        {
            communicator.stringToProxy("test3").ice_ping();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            registry.setAdapterDirectProxy("TestAdapter4", communicator.stringToProxy("dummy:tcp"));
        }
        catch(Ice.AdapterAlreadyActiveException ex)
        {
        }
        catch(Ice.AdapterNotFoundException ex)
        {
        }
        try
        {
            communicator.stringToProxy("test3").ice_ping();
        }
        catch(Ice.LocalException ex)
        {
        ex.printStackTrace();
            test(false);
        }

        try
        {
            communicator.stringToProxy("test@TestAdapter4").ice_locatorCacheTimeout(0).ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
        }
        try
        {
            communicator.stringToProxy("test@TestAdapter4").ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {   
        }
        try
        {
            communicator.stringToProxy("test3").ice_ping();
            test(false);
        }
        catch(Ice.LocalException ex)
        {
        }
        registry.addObject(communicator.stringToProxy("test3@TestAdapter"));
        try
        {
            communicator.stringToProxy("test3").ice_ping();
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        
        registry.addObject(communicator.stringToProxy("test4"));
        try
        {
            communicator.stringToProxy("test4").ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException ex)
        {
        }
        System.out.println("ok");
        
        System.out.print("testing locator cache background updates... ");
        System.out.flush();
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.BackgroundLocatorCacheUpdates", "1");
            Ice.Communicator ic = Ice.Util.initialize(initData);

            try
            {
                registry.setAdapterDirectProxy("TestAdapter5", locator.findAdapterById("TestAdapter"));
                registry.addObject(communicator.stringToProxy("test3@TestAdapter"));
            }
            catch(Ice.AdapterAlreadyActiveException ex)
            {
            }
            catch(Ice.AdapterNotFoundException ex)
            {
            }

            count = locator.getRequestCount();
            ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
            ic.stringToProxy("test3").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
            count += 3;
            test(count == locator.getRequestCount());
            try
            {
                registry.setAdapterDirectProxy("TestAdapter5", null);
            }
            catch(Ice.AdapterAlreadyActiveException ex)
            {
            }
            catch(Ice.AdapterNotFoundException ex)
            {
            }
            registry.addObject(communicator.stringToProxy("test3:tcp"));
            ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
            ic.stringToProxy("test3").ice_locatorCacheTimeout(10).ice_ping(); // 10s timeout.
            test(count == locator.getRequestCount());
            try
            {
                Thread.sleep(1200);
            }
            catch(InterruptedException ex)
            {
            }

            // The following requets should trigger the background updates but still use the cached endpoints
            // and therefore succeed.
            ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
            ic.stringToProxy("test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.

            try
            {
                while(true)
                {
                    ic.stringToProxy("test@TestAdapter5").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                // Expected to fail once they endpoints have been updated in the background.
            }
            try
            {
                while(true)
                {
                    ic.stringToProxy("test3").ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                // Expected to fail once they endpoints have been updated in the background.
            }
            ic.destroy();
        }
        System.out.println("ok");

        System.out.print("testing proxy from server after shutdown... ");
        System.out.flush();
        hello = obj.getReplicatedHello();
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

        System.out.print("testing indirect proxies to collocated objects... ");
        Ice.Properties properties = communicator.getProperties();
        properties.setProperty("Ice.PrintAdapterReady", "0");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Hello", "default");
        adapter.setLocator(locator);

        Ice.Identity id = new Ice.Identity();
        id.name = Ice.Util.generateUUID();
        registry.addObject(adapter.add(new HelloI(), id));
        adapter.activate();

        try
        {
            HelloPrx helloPrx = HelloPrxHelper.checkedCast(
                communicator.stringToProxy("\"" + communicator.identityToString(id) + "\""));
            Ice.Connection connection = helloPrx.ice_getConnection();
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
