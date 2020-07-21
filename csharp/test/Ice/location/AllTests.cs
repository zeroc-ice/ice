//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Location
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var manager = IServerManagerPrx.Parse($"ServerManager :{helper.GetTestEndpoint(0)}", communicator);
            var locator = ITestLocatorPrx.UncheckedCast(communicator.DefaultLocator!);
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
            TestHelper.Assert(ProxyComparer.Identity.Equals(base1.Locator!, communicator.DefaultLocator!));
            var anotherLocator = ILocatorPrx.Parse("anotherLocator", communicator);
            base1 = base1.Clone(locator: anotherLocator);
            TestHelper.Assert(ProxyComparer.Identity.Equals(base1.Locator!, anotherLocator));
            communicator.DefaultLocator = null;
            base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            TestHelper.Assert(base1.Locator == null);
            base1 = base1.Clone(locator: anotherLocator);
            TestHelper.Assert(ProxyComparer.Identity.Equals(base1.Locator!, anotherLocator));
            communicator.DefaultLocator = locator;
            base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            TestHelper.Assert(ProxyComparer.Identity.Equals(base1.Locator!, communicator.DefaultLocator!));

            //
            // We also test ice_router/ice_getRouter(perhaps we should add a
            // test/Ice/router test?)
            //
            TestHelper.Assert(base1.Router == null);
            var anotherRouter = IRouterPrx.Parse("anotherRouter", communicator);
            base1 = base1.Clone(router: anotherRouter);
            TestHelper.Assert(ProxyComparer.Identity.Equals(base1.Router!, anotherRouter));
            var router = IRouterPrx.Parse("dummyrouter", communicator);
            communicator.DefaultRouter = router;
            base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            TestHelper.Assert(ProxyComparer.Identity.Equals(base1.Router!, communicator.DefaultRouter!));
            communicator.DefaultRouter = null;
            base1 = IObjectPrx.Parse("test @ TestAdapter", communicator);
            TestHelper.Assert(base1.Router == null);
            output.WriteLine("ok");

            output.Write("starting server... ");
            output.Flush();
            manager.StartServer();
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
            obj1.Shutdown();
            manager.StartServer();
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
            obj1.Shutdown();
            manager.StartServer();
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
            obj1.Shutdown();
            manager.StartServer();
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
            obj1.Shutdown();
            manager.StartServer();
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
            obj1.Shutdown();
            manager.StartServer();
            try
            {
                obj2.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            obj1.Shutdown();
            manager.StartServer();
            try
            {
                obj3.IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            obj1.Shutdown();
            manager.StartServer();
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
            int count = locator.GetRequestCount();
            basencc.Clone(locatorCacheTimeout: TimeSpan.Zero).IcePing(); // No locator cache.
            TestHelper.Assert(++count == locator.GetRequestCount());
            basencc.Clone(locatorCacheTimeout: TimeSpan.Zero).IcePing(); // No locator cache.
            TestHelper.Assert(++count == locator.GetRequestCount());
            basencc.Clone(locatorCacheTimeout: TimeSpan.FromSeconds(2)).IcePing(); // 2s timeout.
            TestHelper.Assert(count == locator.GetRequestCount());
            Thread.Sleep(1300); // 1300ms
            basencc.Clone(locatorCacheTimeout: TimeSpan.FromSeconds(1)).IcePing(); // 1s timeout.
            TestHelper.Assert(++count == locator.GetRequestCount());

            IObjectPrx.Parse("test", communicator)
                .Clone(locatorCacheTimeout: TimeSpan.Zero).IcePing(); // No locator cache.
            count += 2;
            TestHelper.Assert(count == locator.GetRequestCount());
            IObjectPrx.Parse("test", communicator)
                .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(2)).IcePing(); // 2s timeout
            TestHelper.Assert(count == locator.GetRequestCount());
            System.Threading.Thread.Sleep(1300); // 1300ms
            IObjectPrx.Parse("test", communicator)
                .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(1)).IcePing(); // 1s timeout
            count += 2;
            TestHelper.Assert(count == locator.GetRequestCount());

            IObjectPrx.Parse("test@TestAdapter", communicator)
                .Clone(locatorCacheTimeout: Timeout.InfiniteTimeSpan).IcePing();
            TestHelper.Assert(count == locator.GetRequestCount());
            IObjectPrx.Parse("test", communicator).Clone(locatorCacheTimeout: Timeout.InfiniteTimeSpan).IcePing();
            TestHelper.Assert(count == locator.GetRequestCount());
            IObjectPrx.Parse("test@TestAdapter", communicator).IcePing();
            TestHelper.Assert(count == locator.GetRequestCount());
            IObjectPrx.Parse("test", communicator).IcePing();
            TestHelper.Assert(count == locator.GetRequestCount());

            TestHelper.Assert(IObjectPrx.Parse("test", communicator)
                .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(99)).LocatorCacheTimeout == TimeSpan.FromSeconds(99));

            output.WriteLine("ok");

            output.Write("testing proxy from server... ");
            output.Flush();
            obj1 = ITestIntfPrx.Parse("test@TestAdapter", communicator);
            IHelloPrx? hello = obj1.GetHello();
            TestHelper.Assert(hello != null);
            TestHelper.Assert(hello.AdapterId.Equals("TestAdapter"));
            hello.SayHello();
            hello = obj1.GetReplicatedHello();
            TestHelper.Assert(hello != null);
            TestHelper.Assert(hello.AdapterId.Equals("ReplicatedAdapter"));
            hello.SayHello();
            output.WriteLine("ok");

            output.Write("testing locator request queuing... ");
            output.Flush();
            hello = obj1.GetReplicatedHello()!.Clone(locatorCacheTimeout: TimeSpan.Zero, cacheConnection: false);
            TestHelper.Assert(hello != null);
            count = locator.GetRequestCount();
            hello.IcePing();
            TestHelper.Assert(++count == locator.GetRequestCount());
            var results = new List<Task>();
            for (int i = 0; i < 1000; i++)
            {
                results.Add(hello.SayHelloAsync());
            }
            Task.WaitAll(results.ToArray());
            results.Clear();
            if (locator.GetRequestCount() > count + 800)
            {
                output.Write("queuing = " + (locator.GetRequestCount() - count));
            }
            TestHelper.Assert(locator.GetRequestCount() > count && locator.GetRequestCount() < count + 999);
            count = locator.GetRequestCount();
            hello = hello.Clone(adapterId: "unknown");
            for (int i = 0; i < 1000; i++)
            {
                results.Add(hello.SayHelloAsync().ContinueWith(
                    t =>
                    {
                        try
                        {
                            t.Wait();
                        }
                        catch (AggregateException ex) when (ex.InnerException is AdapterNotFoundException)
                        {
                        }
                    },
                    TaskScheduler.Default));
            }
            Task.WaitAll(results.ToArray());
            results.Clear();
            // XXX:
            // Take into account the retries.
            TestHelper.Assert(locator.GetRequestCount() > count && locator.GetRequestCount() < count + 1999);
            if (locator.GetRequestCount() > count + 800)
            {
                output.Write("queuing = " + (locator.GetRequestCount() - count));
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
                IObjectPrx.Parse("test@TestAdapter3", communicator).Clone(locatorCacheTimeout: TimeSpan.Zero).IcePing();
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
            registry.AddObject(IObjectPrx.Parse("test3@TestUnknown", communicator));
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
                TestHelper.Assert(false);
            }
            catch (AdapterNotFoundException)
            {
            }
            registry.AddObject(IObjectPrx.Parse("test3@TestAdapter4", communicator)); // Update
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
            catch
            {
                TestHelper.Assert(false);
            }

            registry.SetAdapterDirectProxy("TestAdapter4",
                                            IObjectPrx.Parse($"dummy:{helper.GetTestEndpoint(99)}", communicator));
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
            }
            catch
            {
                TestHelper.Assert(false);
            }

            try
            {
                IObjectPrx.Parse("test@TestAdapter4", communicator).Clone(locatorCacheTimeout: TimeSpan.Zero).IcePing();
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
            registry.AddObject(IObjectPrx.Parse("test3@TestAdapter", communicator));
            try
            {
                IObjectPrx.Parse("test3", communicator).IcePing();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            registry.AddObject(IObjectPrx.Parse("test4", communicator));
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
                using Communicator ic = helper.Initialize(properties);

                registry.SetAdapterDirectProxy("TestAdapter5", locator.FindAdapterById("TestAdapter"));
                registry.AddObject(IObjectPrx.Parse("test3@TestAdapter", communicator));

                count = locator.GetRequestCount();
                IObjectPrx.Parse("test@TestAdapter5", ic)
                    .Clone(locatorCacheTimeout: TimeSpan.Zero).IcePing(); // No locator cache.
                IObjectPrx.Parse("test3", ic).Clone(locatorCacheTimeout: TimeSpan.Zero).IcePing(); // No locator cache.
                count += 3;
                TestHelper.Assert(count == locator.GetRequestCount());
                registry.SetAdapterDirectProxy("TestAdapter5", null);
                registry.AddObject(IObjectPrx.Parse($"test3:{helper.GetTestEndpoint(99)}", communicator));
                IObjectPrx.Parse("test@TestAdapter5", ic)
                    .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(10)).IcePing(); // 10s timeout.
                IObjectPrx.Parse("test3", ic)
                    .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(10)).IcePing(); // 10s timeout.
                TestHelper.Assert(count == locator.GetRequestCount());
                Thread.Sleep(1200);

                // The following request should trigger the background
                // updates but still use the cached endpoints and
                // therefore succeed.
                IObjectPrx.Parse("test@TestAdapter5", ic)
                    .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(1)).IcePing(); // 1s timeout.
                IObjectPrx.Parse("test3", ic)
                    .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(1)).IcePing(); // 1s timeout.

                try
                {
                    while (true)
                    {
                        IObjectPrx.Parse("test@TestAdapter5", ic)
                            .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(1)).IcePing(); // 1s timeout.
                        Thread.Sleep(10);
                    }
                }
                catch
                {
                    // Expected to fail once they endpoints have been updated in the background.
                }
                try
                {
                    while (true)
                    {
                        IObjectPrx.Parse("test3", ic)
                            .Clone(locatorCacheTimeout: TimeSpan.FromSeconds(1)).IcePing(); // 1s timeout.
                        Thread.Sleep(10);
                    }
                }
                catch
                {
                    // Expected to fail once they endpoints have been updated in the background.
                }
            }
            output.WriteLine("ok");

            output.Write("testing proxy from server after shutdown... ");
            output.Flush();
            hello = obj1.GetReplicatedHello();
            TestHelper.Assert(hello != null);
            obj1.Shutdown();
            manager.StartServer();
            hello.SayHello();
            output.WriteLine("ok");

            output.Write("testing object migration... ");
            output.Flush();
            hello = IHelloPrx.Parse("hello", communicator);
            obj1.MigrateHello();
            hello.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
            hello.SayHello();
            obj1.MigrateHello();
            hello.SayHello();
            obj1.MigrateHello();
            hello.SayHello();
            output.WriteLine("ok");

            output.Write("testing locator encoding resolution... ");
            output.Flush();
            hello = IHelloPrx.Parse("hello", communicator);
            count = locator.GetRequestCount();
            IObjectPrx.Parse("test@TestAdapter", communicator).Clone(encoding: Encoding.V1_1).IcePing();
            TestHelper.Assert(count == locator.GetRequestCount());
            output.WriteLine("ok");

            output.Write("shutdown server... ");
            output.Flush();
            obj1.Shutdown();
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
            manager.Shutdown();
            output.WriteLine("ok");
        }
    }
}
