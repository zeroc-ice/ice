// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;

namespace Ice
{
    namespace location
    {
        public class AllTests : global::Test.AllTests
        {
            public static void allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var manager = Test.ServerManagerPrxHelper.checkedCast(
                                                communicator.stringToProxy("ServerManager :" + helper.getTestEndpoint(0)));
                test(manager != null);
                var locator = Test.TestLocatorPrxHelper.uncheckedCast(communicator.getDefaultLocator());
                test(locator != null);
                var registry = Test.TestLocatorRegistryPrxHelper.checkedCast(locator.getRegistry());
                test(registry != null);

                var output = helper.getWriter();
                output.Write("testing stringToProxy... ");
                output.Flush();
                Ice.ObjectPrx @base = communicator.stringToProxy("test @ TestAdapter");
                Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
                Ice.ObjectPrx base3 = communicator.stringToProxy("test");
                Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
                Ice.ObjectPrx base5 = communicator.stringToProxy("test2");
                Ice.ObjectPrx base6 = communicator.stringToProxy("test @ ReplicatedAdapter");
                output.WriteLine("ok");

                output.Write("testing ice_locator and ice_getLocator... ");
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
                // We also test ice_router/ice_getRouter(perhaps we should add a
                // test/Ice/router test?)
                //
                test(@base.ice_getRouter() == null);
                Ice.RouterPrx anotherRouter =
                    Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("anotherRouter"));
                @base = @base.ice_router(anotherRouter);
                test(Ice.Util.proxyIdentityCompare(@base.ice_getRouter(), anotherRouter) == 0);
                Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("dummyrouter"));
                communicator.setDefaultRouter(router);
                @base = communicator.stringToProxy("test @ TestAdapter");
                test(Ice.Util.proxyIdentityCompare(@base.ice_getRouter(), communicator.getDefaultRouter()) == 0);
                communicator.setDefaultRouter(null);
                @base = communicator.stringToProxy("test @ TestAdapter");
                test(@base.ice_getRouter() == null);
                output.WriteLine("ok");

                output.Write("starting server... ");
                output.Flush();
                manager.startServer();
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var obj = Test.TestIntfPrxHelper.checkedCast(@base);
                test(obj != null);
                var obj2 = Test.TestIntfPrxHelper.checkedCast(base2);
                test(obj2 != null);
                var obj3 = Test.TestIntfPrxHelper.checkedCast(base3);
                test(obj3 != null);
                var obj4 = Test.ServerManagerPrxHelper.checkedCast(base4);
                test(obj4 != null);
                var obj5 = Test.TestIntfPrxHelper.checkedCast(base5);
                test(obj5 != null);
                var obj6 = Test.TestIntfPrxHelper.checkedCast(base6);
                test(obj6 != null);
                output.WriteLine("ok");

                output.Write("testing id@AdapterId indirect proxy... ");
                output.Flush();
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
                output.WriteLine("ok");

                output.Write("testing id@ReplicaGroupId indirect proxy... ");
                output.Flush();
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
                output.WriteLine("ok");

                output.Write("testing identity indirect proxy... ");
                output.Flush();
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
                    obj5 = Test.TestIntfPrxHelper.checkedCast(base5);
                    obj5.ice_ping();
                }
                catch(Ice.LocalException)
                {
                    test(false);
                }
                output.WriteLine("ok");

                output.Write("testing proxy with unknown identity... ");
                output.Flush();
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
                output.WriteLine("ok");

                output.Write("testing proxy with unknown adapter... ");
                output.Flush();
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
                output.WriteLine("ok");

                output.Write("testing locator cache timeout... ");
                output.Flush();

                Ice.ObjectPrx basencc = communicator.stringToProxy("test@TestAdapter").ice_connectionCached(false);
                int count = locator.getRequestCount();
                basencc.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
                test(++count == locator.getRequestCount());
                basencc.ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
                test(++count == locator.getRequestCount());
                basencc.ice_locatorCacheTimeout(2).ice_ping(); // 2s timeout.
                test(count == locator.getRequestCount());
                System.Threading.Thread.Sleep(1300); // 1300ms
                basencc.ice_locatorCacheTimeout(1).ice_ping(); // 1s timeout.
                test(++count == locator.getRequestCount());

                communicator.stringToProxy("test").ice_locatorCacheTimeout(0).ice_ping(); // No locator cache.
                count += 2;
                test(count == locator.getRequestCount());
                communicator.stringToProxy("test").ice_locatorCacheTimeout(2).ice_ping(); // 2s timeout
                test(count == locator.getRequestCount());
                System.Threading.Thread.Sleep(1300); // 1300ms
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

                output.WriteLine("ok");

                output.Write("testing proxy from server... ");
                output.Flush();
                obj = Test.TestIntfPrxHelper.checkedCast(communicator.stringToProxy("test@TestAdapter"));
                var hello = obj.getHello();
                test(hello.ice_getAdapterId().Equals("TestAdapter"));
                hello.sayHello();
                hello = obj.getReplicatedHello();
                test(hello.ice_getAdapterId().Equals("ReplicatedAdapter"));
                hello.sayHello();
                output.WriteLine("ok");

                output.Write("testing locator request queuing... ");
                output.Flush();
                hello =(Test.HelloPrx)obj.getReplicatedHello().ice_locatorCacheTimeout(0).ice_connectionCached(false);
                count = locator.getRequestCount();
                hello.ice_ping();
                test(++count == locator.getRequestCount());
                List<Ice.AsyncResult<Test.Callback_Hello_sayHello>> results =
                    new List<Ice.AsyncResult<Test.Callback_Hello_sayHello>>();
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
                    output.Write("queuing = " +(locator.getRequestCount() - count));
                }
                count = locator.getRequestCount();
                hello =(Test.HelloPrx)hello.ice_adapterId("unknown");
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
                    output.Write("queuing = " +(locator.getRequestCount() - count));
                }
                output.WriteLine("ok");

                output.Write("testing adapter locator cache... ");
                output.Flush();
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
                output.WriteLine("ok");

                output.Write("testing well-known object locator cache... ");
                output.Flush();
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
                output.WriteLine("ok");

                output.Write("testing locator cache background updates... ");
                output.Flush();
                {
                    Ice.InitializationData initData = new Ice.InitializationData();
                    initData.properties = communicator.getProperties().ice_clone_();
                    initData.properties.setProperty("Ice.BackgroundLocatorCacheUpdates", "1");
                    Ice.Communicator ic = helper.initialize(initData);

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
                output.WriteLine("ok");

                output.Write("testing proxy from server after shutdown... ");
                output.Flush();
                hello = obj.getReplicatedHello();
                obj.shutdown();
                manager.startServer();
                hello.sayHello();
                output.WriteLine("ok");

                output.Write("testing object migration... ");
                output.Flush();
                hello = Test.HelloPrxHelper.checkedCast(communicator.stringToProxy("hello"));
                obj.migrateHello();
                hello.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                hello.sayHello();
                obj.migrateHello();
                hello.sayHello();
                obj.migrateHello();
                hello.sayHello();
                output.WriteLine("ok");

                output.Write("testing locator encoding resolution... ");
                output.Flush();
                hello = Test.HelloPrxHelper.checkedCast(communicator.stringToProxy("hello"));
                count = locator.getRequestCount();
                communicator.stringToProxy("test@TestAdapter").ice_encodingVersion(Ice.Util.Encoding_1_1).ice_ping();
                test(count == locator.getRequestCount());
                communicator.stringToProxy("test@TestAdapter10").ice_encodingVersion(Ice.Util.Encoding_1_0).ice_ping();
                test(++count == locator.getRequestCount());
                communicator.stringToProxy("test -e 1.0@TestAdapter10-2").ice_ping();
                test(++count == locator.getRequestCount());
                output.WriteLine("ok");

                output.Write("shutdown server... ");
                output.Flush();
                obj.shutdown();
                output.WriteLine("ok");

                output.Write("testing whether server is gone... ");
                output.Flush();
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
                output.WriteLine("ok");

                output.Write("testing indirect proxies to collocated objects... ");
                output.Flush();

                //
                // Set up test for calling a collocated object through an
                // indirect, adapterless reference.
                //
                Ice.Properties properties = communicator.getProperties();
                properties.setProperty("Ice.PrintAdapterReady", "0");
                Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Hello", "tcp -h *");
                adapter.setLocator(locator);

                Ice.Identity id = new Ice.Identity();
                id.name = Guid.NewGuid().ToString();
                registry.addObject(adapter.add(new HelloI(), id));
                adapter.activate();

                var helloPrx = Test.HelloPrxHelper.checkedCast(
                    communicator.stringToProxy("\"" + communicator.identityToString(id) + "\""));
                test(helloPrx.ice_getConnection() == null);

                adapter.deactivate();
                output.WriteLine("ok");

                output.Write("shutdown server manager... ");
                output.Flush();
                manager.shutdown();
                output.WriteLine("ok");
            }
        }
    }
}
