//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.location.Test;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace Ice.location
{
    public class AllTests
    {
        public static void allTests(global::Test.TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var manager = IServerManagerPrx.Parse($"ServerManager :{helper.GetTestEndpoint(0)}", communicator);
            var locator = ITestLocatorPrx.UncheckedCast(communicator.GetDefaultLocator()!);
            Console.WriteLine("registry checkedcast");
            var registry = ITestLocatorRegistryPrx.CheckedCast(locator.GetRegistry()!);
            TestHelper.Assert(registry != null);

            System.IO.TextWriter output = helper.GetWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();
            var base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            var base2 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            var base3 = IObjectPrx.Parse("test", communicator);
            var base4 = IObjectPrx.Parse("ServerManager", communicator);
            var base5 = IObjectPrx.Parse("test2", communicator);
            var base6 = IObjectPrx.Parse("test @ ReplicatedAdapter", communicator);
            output.WriteLine("ok");

            output.Write("testing ice_locator and ice_getLocator... ");
            TestHelper.Assert(default(ProxyIdentityComparer).Equals(base1.Locator!, communicator.GetDefaultLocator()!));
            var anotherLocator = ILocatorPrx.Parse("anotherLocator", communicator);
            base1 = base1.Clone(locator: anotherLocator);
            TestHelper.Assert(default(ProxyIdentityComparer).Equals(base1.Locator!, anotherLocator));
            communicator.SetDefaultLocator(null);
            base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            TestHelper.Assert(base1.Locator == null);
            base1 = base1.Clone(locator: anotherLocator);
            TestHelper.Assert(default(ProxyIdentityComparer).Equals(base1.Locator!, anotherLocator));
            communicator.SetDefaultLocator(locator);
            base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            TestHelper.Assert(default(ProxyIdentityComparer).Equals(base1.Locator!, communicator.GetDefaultLocator()!));

            //
            // We also test ice_router/ice_getRouter(perhaps we should add a
            // test/Ice/router test?)
            //
            TestHelper.Assert(base1.Router == null);
            var anotherRouter = IRouterPrx.Parse("anotherRouter", communicator);
            base1 = base1.Clone(router: anotherRouter);
            TestHelper.Assert(default(ProxyIdentityComparer).Equals(base1.Router!, anotherRouter));
            var router = IRouterPrx.Parse("dummyrouter", communicator);
            communicator.SetDefaultRouter(router);
            base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            TestHelper.Assert(default(ProxyIdentityComparer).Equals(base1.Router!, communicator.GetDefaultRouter()!));
            communicator.SetDefaultRouter(null);
            base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            TestHelper.Assert(base1.Router == null);
            output.WriteLine("ok");

            output.Write("starting server... ");
            output.Flush();
            manager.startServer();
            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            var obj1 = ITestIntfPrx.CheckedCast(base1);
            TestHelper.Assert(obj1 != null);
            var obj2 = ITestIntfPrx.CheckedCast(base2);
            TestHelper.Assert(obj2 != null);
            var obj3 = ITestIntfPrx.CheckedCast(base3);
            TestHelper.Assert(obj3 != null);
            var obj4 = IServerManagerPrx.CheckedCast(base4);
            TestHelper.Assert(obj4 != null);
            var obj5 = ITestIntfPrx.CheckedCast(base5);
            TestHelper.Assert(obj5 != null);
            var obj6 = ITestIntfPrx.CheckedCast(base6);
            TestHelper.Assert(obj6 != null);
            output.WriteLine("ok");

            output.Write("testing id@AdapterId indirect proxy... ");
            output.Flush();
            obj1.shutdown();
            manager.startServer();
            try
            {
                obj2.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            output.WriteLine("ok");

            output.Write("testing id@ReplicaGroupId indirect proxy... ");
            output.Flush();
            obj1.shutdown();
            manager.startServer();
            try
            {
                obj6.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            output.WriteLine("ok");

            output.Write("testing identity indirect proxy... ");
            output.Flush();
            obj1.shutdown();
            manager.startServer();
            try
            {
                obj3.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            try
            {
                obj2.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            obj1.shutdown();
            manager.startServer();
            try
            {
                obj2.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            try
            {
                obj3.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            obj1.shutdown();
            manager.startServer();
            try
            {
                obj2.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            obj1.shutdown();
            manager.startServer();
            try
            {
                obj3.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            obj1.shutdown();
            manager.startServer();
            try
            {
                obj5 = ITestIntfPrx.CheckedCast(base5);
                TestHelper.Assert(obj5 != null);
                obj5.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            output.WriteLine("ok");

            output.Write("testing proxy with unknown identity... ");
            output.Flush();
            try
            {
                base1 = IObjectPrx.Parse("unknown/unknown", communicator);
                base1.IcePing();
                TestHelper.Assert(false);
            }
            catch (ObjectNotFoundException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing proxy with unknown adapter... ");
            output.Flush();
            try
            {
                base1 = IObjectPrx.Parse("test @ TestAdapterUnknown", communicator);
                base1.IcePing();
                TestHelper.Assert(false);
            }
            catch (AdapterNotFoundException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing locator cache timeout... ");
            output.Flush();

            IObjectPrx basencc = IObjectPrx.Parse("test@TestAdapter", communicator).Clone(cacheConnection: false);
            int count = locator.getRequestCount();
            basencc.Clone(locatorCacheTimeout: 0).IcePing(); // No locator cache.
            TestHelper.Assert(++count == locator.getRequestCount());
            basencc.Clone(locatorCacheTimeout: 0).IcePing(); // No locator cache.
            TestHelper.Assert(++count == locator.getRequestCount());
            basencc.Clone(locatorCacheTimeout: 2).IcePing(); // 2s timeout.
            TestHelper.Assert(count == locator.getRequestCount());
            System.Threading.Thread.Sleep(1300); // 1300ms
            basencc.Clone(locatorCacheTimeout: 1).IcePing(); // 1s timeout.
            TestHelper.Assert(++count == locator.getRequestCount());

            IObjectPrx.Parse("test", communicator).Clone(locatorCacheTimeout: 0).IcePing(); // No locator cache.
            count += 2;
            TestHelper.Assert(count == locator.getRequestCount());
            IObjectPrx.Parse("test", communicator).Clone(locatorCacheTimeout: 2).IcePing(); // 2s timeout
            TestHelper.Assert(count == locator.getRequestCount());
            System.Threading.Thread.Sleep(1300); // 1300ms
            IObjectPrx.Parse("test", communicator).Clone(locatorCacheTimeout: 1).IcePing(); // 1s timeout
            count += 2;
            TestHelper.Assert(count == locator.getRequestCount());

            IObjectPrx.Parse("test@TestAdapter", communicator).Clone(locatorCacheTimeout: -1).IcePing();
            TestHelper.Assert(count == locator.getRequestCount());
            IObjectPrx.Parse("test", communicator).Clone(locatorCacheTimeout: -1).IcePing();
            TestHelper.Assert(count == locator.getRequestCount());
            IObjectPrx.Parse("test@TestAdapter", communicator).IcePing();
            TestHelper.Assert(count == locator.getRequestCount());
            IObjectPrx.Parse("test", communicator).IcePing();
            TestHelper.Assert(count == locator.getRequestCount());

            TestHelper.Assert(IObjectPrx.Parse("test", communicator).Clone(locatorCacheTimeout: 99).LocatorCacheTimeout == 99);

            output.WriteLine("ok");

            output.Write("testing proxy from server... ");
            output.Flush();
            obj1 = ITestIntfPrx.Parse("test@TestAdapter", communicator);
            IHelloPrx? hello = obj1.getHello();
            TestHelper.Assert(hello != null);
            TestHelper.Assert(hello.AdapterId.Equals("TestAdapter"));
            hello.sayHello();
            hello = obj1.getReplicatedHello();
            TestHelper.Assert(hello != null);
            TestHelper.Assert(hello.AdapterId.Equals("ReplicatedAdapter"));
            hello.sayHello();
            output.WriteLine("ok");

            output.Write("testing locator request queuing... ");
            output.Flush();
            hello = obj1.getReplicatedHello()!.Clone(locatorCacheTimeout: 0, cacheConnection: false);
            TestHelper.Assert(hello != null);
            count = locator.getRequestCount();
            hello.IcePing();
            TestHelper.Assert(++count == locator.getRequestCount());
            var results = new List<Task>();
            for (int i = 0; i < 1000; i++)
            {
                results.Add(hello.sayHelloAsync());
            }
            Task.WaitAll(results.ToArray());
            results.Clear();
            TestHelper.Assert(locator.getRequestCount() > count && locator.getRequestCount() < count + 999);
            if (locator.getRequestCount() > count + 800)
            {
                output.Write("queuing = " + (locator.getRequestCount() - count));
            }
            count = locator.getRequestCount();
            hello = hello.Clone(adapterId: "unknown");
            for (int i = 0; i < 1000; i++)
            {
                results.Add(hello.sayHelloAsync().ContinueWith((Task t) =>
                {
                    try
                    {
                        t.Wait();
                    }
                    catch (AggregateException ex) when (ex.InnerException is AdapterNotFoundException)
                    {
                    }
                }));
            }
            Task.WaitAll(results.ToArray());
            results.Clear();
            // XXX:
            // Take into account the retries.
            TestHelper.Assert(locator.getRequestCount() > count && locator.getRequestCount() < count + 1999);
            if (locator.getRequestCount() > count + 800)
            {
                output.Write("queuing = " + (locator.getRequestCount() - count));
            }
            output.WriteLine("ok");

            output.Write("testing adapter locator cache... ");
            output.Flush();
            try
            {
                IObjectPrx.Parse("test@TestAdapter3", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (AdapterNotFoundException)
            {
            }
            registry.SetAdapterDirectProxy("TestAdapter3", locator.FindAdapterById("TestAdapter"));
            try
            {
                IObjectPrx.Parse("test@TestAdapter3", communicator).IcePing();
                registry.SetAdapterDirectProxy("TestAdapter3",
                                                IObjectPrx.Parse($"dummy:{helper.GetTestEndpoint(99)}", communicator));
                IObjectPrx.Parse("test@TestAdapter3", communicator).IcePing();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                IObjectPrx.Parse("test@TestAdapter3", communicator).Clone(locatorCacheTimeout: 0).IcePing();
                TestHelper.Assert(false);
            }
            catch (ConnectionRefusedException)
            {
            }

            try
            {
                IObjectPrx.Parse("test@TestAdapter3", communicator).IcePing();
            }
            catch (ConnectionRefusedException)
            {
            }

            registry.SetAdapterDirectProxy("TestAdapter3", locator.FindAdapterById("TestAdapter"));
            try
            {
                IObjectPrx.Parse("test@TestAdapter3", communicator).IcePing();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }
            output.WriteLine("ok");

            output.Write("testing well-known object locator cache... ");
            output.Flush();
            registry.addObject(IObjectPrx.Parse("test3@TestUnknown", communicator));
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (AdapterNotFoundException)
            {
            }
            registry.addObject(IObjectPrx.Parse("test3@TestAdapter4", communicator)); // Update
            registry.SetAdapterDirectProxy("TestAdapter4",
                                            IObjectPrx.Parse($"dummy:{helper.GetTestEndpoint(99)}", communicator));
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (ConnectionRefusedException)
            {
            }
            registry.SetAdapterDirectProxy("TestAdapter4", locator.FindAdapterById("TestAdapter"));
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }

            registry.SetAdapterDirectProxy("TestAdapter4",
                                            IObjectPrx.Parse($"dummy:{helper.GetTestEndpoint(99)}", communicator));
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                IObjectPrx.Parse("test@TestAdapter4", communicator).Clone(locatorCacheTimeout: 0).IcePing();
                TestHelper.Assert(false);
            }
            catch (ConnectionRefusedException)
            {
            }
            try
            {
                IObjectPrx.Parse("test@TestAdapter4", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (ConnectionRefusedException)
            {
            }
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (ConnectionRefusedException)
            {
            }
            registry.addObject(IObjectPrx.Parse("test3@TestAdapter", communicator));
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }

            registry.addObject(IObjectPrx.Parse("test4", communicator));
            try
            {
                IObjectPrx.Parse("test4", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (NoEndpointException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing locator cache background updates... ");
            output.Flush();
            {
                Dictionary<string, string> properties = communicator.GetProperties();
                properties["Ice.BackgroundLocatorCacheUpdates"] = "1";
                Communicator ic = helper.Initialize(properties);

                registry.SetAdapterDirectProxy("TestAdapter5", locator.FindAdapterById("TestAdapter"));
                registry.addObject(IObjectPrx.Parse("test3@TestAdapter", communicator));

                count = locator.getRequestCount();
                IObjectPrx.Parse("test@TestAdapter5", ic).Clone(locatorCacheTimeout: 0).IcePing(); // No locator cache.
                IObjectPrx.Parse("test3", ic).Clone(locatorCacheTimeout: 0).IcePing(); // No locator cache.
                count += 3;
                TestHelper.Assert(count == locator.getRequestCount());
                registry.SetAdapterDirectProxy("TestAdapter5", null);
                registry.addObject(IObjectPrx.Parse($"test3:{helper.GetTestEndpoint(99)}", communicator));
                IObjectPrx.Parse("test@TestAdapter5", ic).Clone(locatorCacheTimeout: 10).IcePing(); // 10s timeout.
                IObjectPrx.Parse("test3", ic).Clone(locatorCacheTimeout: 10).IcePing(); // 10s timeout.
                TestHelper.Assert(count == locator.getRequestCount());
                System.Threading.Thread.Sleep(1200);

                // The following request should trigger the background
                // updates but still use the cached endpoints and
                // therefore succeed.
                IObjectPrx.Parse("test@TestAdapter5", ic).Clone(locatorCacheTimeout: 1).IcePing(); // 1s timeout.
                IObjectPrx.Parse("test3", ic).Clone(locatorCacheTimeout: 1).IcePing(); // 1s timeout.

                try
                {
                    while (true)
                    {
                        IObjectPrx.Parse("test@TestAdapter5", ic).Clone(locatorCacheTimeout: 1).IcePing(); // 1s timeout.
                        System.Threading.Thread.Sleep(10);
                    }
                }
                catch (System.Exception)
                {
                    // Expected to fail once they endpoints have been updated in the background.
                }
                try
                {
                    while (true)
                    {
                        IObjectPrx.Parse("test3", ic).Clone(locatorCacheTimeout: 1).IcePing(); // 1s timeout.
                        System.Threading.Thread.Sleep(10);
                    }
                }
                catch (System.Exception)
                {
                    // Expected to fail once they endpoints have been updated in the background.
                }
                ic.Destroy();
            }
            output.WriteLine("ok");

            output.Write("testing proxy from server after shutdown... ");
            output.Flush();
            hello = obj1.getReplicatedHello();
            TestHelper.Assert(hello != null);
            obj1.shutdown();
            manager.startServer();
            hello.sayHello();
            output.WriteLine("ok");

            output.Write("testing object migration... ");
            output.Flush();
            hello = IHelloPrx.Parse("hello", communicator);
            obj1.migrateHello();
            hello.GetConnection().Close(ConnectionClose.GracefullyWithWait);
            hello.sayHello();
            obj1.migrateHello();
            hello.sayHello();
            obj1.migrateHello();
            hello.sayHello();
            output.WriteLine("ok");

            output.Write("testing locator encoding resolution... ");
            output.Flush();
            hello = IHelloPrx.Parse("hello", communicator);
            count = locator.getRequestCount();
            IObjectPrx.Parse("test@TestAdapter", communicator).Clone(encoding: Encoding.V1_1).IcePing();
            TestHelper.Assert(count == locator.getRequestCount());
            output.WriteLine("ok");

            output.Write("shutdown server... ");
            output.Flush();
            obj1.shutdown();
            output.WriteLine("ok");

            output.Write("testing whether server is gone... ");
            output.Flush();
            try
            {
                obj2.IcePing();
                TestHelper.Assert(false);
            }
            catch (AdapterNotFoundException)
            {
            }
            try
            {
                obj3.IcePing();
                TestHelper.Assert(false);
            }
            catch (AdapterNotFoundException)
            {
            }
            try
            {
                TestHelper.Assert(obj5 != null);
                obj5.IcePing();
                TestHelper.Assert(false);
            }
            catch (AdapterNotFoundException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing indirect proxies to collocated objects... ");
            output.Flush();

            communicator.SetProperty("Hello.AdapterId", Guid.NewGuid().ToString());
            ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints("Hello", "default");

            var id = new Identity(Guid.NewGuid().ToString(), "");
            adapter.Add(id, new Hello());
            adapter.Activate();

            // Ensure that calls on the well-known proxy is collocated.
            IHelloPrx? helloPrx;
            helloPrx = IHelloPrx.Parse($"\"{id.ToString(communicator.ToStringMode)}\"", communicator);
            TestHelper.Assert(helloPrx.GetConnection() == null);

            // Ensure that calls on the indirect proxy (with adapter ID) is collocated
            helloPrx = IHelloPrx.CheckedCast(adapter.CreateIndirectProxy(id, IObjectPrx.Factory));
            TestHelper.Assert(helloPrx != null && helloPrx.GetConnection() == null);

            // Ensure that calls on the direct proxy is collocated
            helloPrx = IHelloPrx.CheckedCast(adapter.CreateDirectProxy(id, IObjectPrx.Factory));
            TestHelper.Assert(helloPrx != null && helloPrx.GetConnection() == null);

            output.WriteLine("ok");

            output.Write("shutdown server manager... ");
            output.Flush();
            manager.shutdown();
            output.WriteLine("ok");
        }
    }
}
