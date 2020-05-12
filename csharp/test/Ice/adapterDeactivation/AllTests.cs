//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;

using Test;

namespace Ice
{
    namespace adapterDeactivation
    {
        public class AllTests
        {
            public static Test.ITestIntfPrx allTests(TestHelper helper)
            {
                Communicator? communicator = helper.Communicator();
                TestHelper.Assert(communicator != null);
                var output = helper.GetWriter();
                output.Write("testing stringToProxy... ");
                output.Flush();
                var baseprx = IObjectPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);
                TestHelper.Assert(baseprx != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var obj = Test.ITestIntfPrx.CheckedCast(baseprx);
                TestHelper.Assert(obj != null);
                TestHelper.Assert(obj.Equals(baseprx));
                output.WriteLine("ok");

                {
                    output.Write("creating/destroying/recreating object adapter... ");
                    output.Flush();
                    ObjectAdapter adapter =
                        communicator.CreateObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                    try
                    {
                        communicator.CreateObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                    adapter.Destroy();

                    //
                    // Use a different port than the first adapter to avoid an "address already in use" error.
                    //
                    adapter = communicator.CreateObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                    adapter.Destroy();
                    output.WriteLine("ok");
                }

                output.Write("creating/activating/deactivating object adapter in one operation... ");
                output.Flush();
                obj.transient();
                obj.transientAsync().Wait();
                output.WriteLine("ok");

                {
                    output.Write("testing connection closure... ");
                    output.Flush();
                    for (int i = 0; i < 10; ++i)
                    {
                        var comm = new Communicator(communicator.GetProperties());
                        IObjectPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator).IcePingAsync();
                        comm.Destroy();
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing object adapter published endpoints... ");
                output.Flush();
                {
                    communicator.SetProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
                    var adapter = communicator.CreateObjectAdapter("PAdapter");
                    TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 1);
                    var endpt = adapter.GetPublishedEndpoints()[0];
                    TestHelper.Assert(endpt != null);
                    TestHelper.Assert(endpt.ToString()!.Equals("tcp -h localhost -p 12345 -t 30000"));
                    var prx = IObjectPrx.Parse("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000", communicator);
                    adapter.SetPublishedEndpoints(prx.Endpoints);
                    TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 2);
                    TestHelper.Assert(adapter.CreateProxy(new Identity("dummy", ""), IObjectPrx.Factory).Endpoints.
                        SequenceEqual(prx.Endpoints));
                    TestHelper.Assert(adapter.GetPublishedEndpoints().SequenceEqual(prx.Endpoints));
                    adapter.RefreshPublishedEndpoints();
                    TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 1);
                    TestHelper.Assert(adapter.GetPublishedEndpoints()[0].Equals(endpt));
                    communicator.SetProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
                    adapter.RefreshPublishedEndpoints();
                    TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 1);
                    TestHelper.Assert(adapter.GetPublishedEndpoints()[0].ToString()!.Equals("tcp -h localhost -p 12345 -t 20000"));
                    adapter.Destroy();
                    TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 0);
                }
                output.WriteLine("ok");

                if (obj.GetConnection() != null)
                {
                    output.Write("testing object adapter with bi-dir connection... ");
                    output.Flush();
                    var adapter = communicator.CreateObjectAdapter();
                    obj.GetConnection().Adapter = adapter;
                    obj.GetConnection().Adapter = null;
                    adapter.Deactivate();
                    // Setting a deactivated adapter on a connection no longer raise ObjectAdapterDeactivatedException
                    obj.GetConnection().Adapter = adapter;
                    output.WriteLine("ok");
                }

                output.Write("testing object adapter with router... ");
                output.Flush();
                {
                    var routerId = new Identity("router", "");
                    var router = baseprx.Clone(routerId, IRouterPrx.Factory, connectionId: "rc");
                    var adapter = communicator.CreateObjectAdapterWithRouter(router);
                    TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 1);
                    var endpointsStr = adapter.GetPublishedEndpoints()[0].ToString();
                    TestHelper.Assert(endpointsStr!.Equals("tcp -h localhost -p 23456 -t 30000"));
                    adapter.RefreshPublishedEndpoints();
                    TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 1);
                    TestHelper.Assert(adapter.GetPublishedEndpoints()[0].ToString()!.Equals("tcp -h localhost -p 23457 -t 30000"));
                    try
                    {
                        adapter.SetPublishedEndpoints(router.Endpoints);
                        TestHelper.Assert(false);
                    }
                    catch (InvalidOperationException)
                    {
                        // Expected.
                    }
                    adapter.Destroy();

                    try
                    {
                        routerId = new Identity("test", "");
                        router = baseprx.Clone(routerId, IRouterPrx.Factory);
                        communicator.CreateObjectAdapterWithRouter(router);
                        TestHelper.Assert(false);
                    }
                    catch (OperationNotExistException)
                    {
                        // Expected: the "test" object doesn't implement Ice::Router!
                    }

                    try
                    {
                        router = IRouterPrx.Parse($"test:{helper.GetTestEndpoint(1)}", communicator);
                        communicator.CreateObjectAdapterWithRouter(router);
                        TestHelper.Assert(false);
                    }
                    catch (ConnectFailedException)
                    {
                    }
                }
                output.WriteLine("ok");

                output.Write("testing object adapter creation with port in use... ");
                output.Flush();
                {
                    var adapter1 = communicator.CreateObjectAdapterWithEndpoints("Adpt1", helper.GetTestEndpoint(10));
                    try
                    {
                        communicator.CreateObjectAdapterWithEndpoints("Adpt2", helper.GetTestEndpoint(10));
                        TestHelper.Assert(false);
                    }
                    catch (System.Exception)
                    {
                        // Expected can't re-use the same endpoint.
                    }
                    adapter1.Destroy();
                }
                output.WriteLine("ok");

                output.Write("deactivating object adapter in the server... ");
                output.Flush();
                obj.deactivate();
                output.WriteLine("ok");

                output.Write("testing whether server is gone... ");
                output.Flush();
                try
                {
                    obj.Clone(connectionTimeout: 100).IcePing(); // Use timeout to speed up testing on Windows
                    TestHelper.Assert(false);
                }
                catch (System.Exception)
                {
                    output.WriteLine("ok");
                }

                return obj;
            }
        }
    }
}
