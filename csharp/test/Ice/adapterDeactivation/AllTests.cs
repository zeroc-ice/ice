//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;

namespace Ice
{
    namespace adapterDeactivation
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.ITestIntfPrx allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing stringToProxy... ");
                output.Flush();
                var baseprx = IObjectPrx.Parse($"test:{helper.getTestEndpoint(0)}", communicator);
                Debug.Assert(baseprx != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var obj = Test.ITestIntfPrx.CheckedCast(baseprx);
                Debug.Assert(obj != null);
                test(obj.Equals(baseprx));
                output.WriteLine("ok");

                {
                    output.Write("creating/destroying/recreating object adapter... ");
                    output.Flush();
                    ObjectAdapter adapter =
                        communicator.CreateObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                    try
                    {
                        communicator.CreateObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                        test(false);
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
                        IObjectPrx.Parse($"test:{helper.getTestEndpoint(0)}", communicator).IcePingAsync();
                        comm.Destroy();
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing object adapter published endpoints... ");
                output.Flush();
                {
                    communicator.SetProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
                    var adapter = communicator.CreateObjectAdapter("PAdapter");
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    var endpt = adapter.GetPublishedEndpoints()[0];
                    Debug.Assert(endpt != null);
                    test(endpt.ToString()!.Equals("tcp -h localhost -p 12345 -t 30000"));
                    var prx = IObjectPrx.Parse("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000", communicator);
                    adapter.SetPublishedEndpoints(prx.Endpoints);
                    test(adapter.GetPublishedEndpoints().Length == 2);
                    test(global::Test.Collections.Equals(adapter.CreateProxy(new Identity("dummy", ""), IObjectPrx.Factory).Endpoints,
                        prx.Endpoints));
                    test(global::Test.Collections.Equals(adapter.GetPublishedEndpoints(), prx.Endpoints));
                    adapter.RefreshPublishedEndpoints();
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    test(adapter.GetPublishedEndpoints()[0].Equals(endpt));
                    communicator.SetProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
                    adapter.RefreshPublishedEndpoints();
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    test(adapter.GetPublishedEndpoints()[0].ToString()!.Equals("tcp -h localhost -p 12345 -t 20000"));
                    adapter.Destroy();
                    test(adapter.GetPublishedEndpoints().Length == 0);
                }
                output.WriteLine("ok");

                if (obj.GetConnection() != null)
                {
                    output.Write("testing object adapter with bi-dir connection... ");
                    output.Flush();
                    var adapter = communicator.CreateObjectAdapter();
                    obj.GetConnection().SetAdapter(adapter);
                    obj.GetConnection().SetAdapter(null);
                    adapter.Deactivate();
                    try
                    {
                        obj.GetConnection().SetAdapter(adapter);
                        test(false);
                    }
                    catch (ObjectAdapterDeactivatedException)
                    {
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing object adapter with router... ");
                output.Flush();
                {
                    var routerId = new Identity("router", "");
                    var router = baseprx.Clone(routerId, IRouterPrx.Factory, connectionId: "rc");
                    var adapter = communicator.CreateObjectAdapterWithRouter(router);
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    var endpointsStr = adapter.GetPublishedEndpoints()[0].ToString();
                    test(endpointsStr!.Equals("tcp -h localhost -p 23456 -t 30000"));
                    adapter.RefreshPublishedEndpoints();
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    test(adapter.GetPublishedEndpoints()[0].ToString()!.Equals("tcp -h localhost -p 23457 -t 30000"));
                    try
                    {
                        adapter.SetPublishedEndpoints(router.Endpoints);
                        test(false);
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
                        test(false);
                    }
                    catch (OperationNotExistException)
                    {
                        // Expected: the "test" object doesn't implement Ice::Router!
                    }

                    try
                    {
                        router = IRouterPrx.Parse($"test:{helper.getTestEndpoint(1)}", communicator);
                        communicator.CreateObjectAdapterWithRouter(router);
                        test(false);
                    }
                    catch (ConnectFailedException)
                    {
                    }
                }
                output.WriteLine("ok");

                output.Write("testing object adapter creation with port in use... ");
                output.Flush();
                {
                    var adapter1 = communicator.CreateObjectAdapterWithEndpoints("Adpt1", helper.getTestEndpoint(10));
                    try
                    {
                        communicator.CreateObjectAdapterWithEndpoints("Adpt2", helper.getTestEndpoint(10));
                        test(false);
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
                    test(false);
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
