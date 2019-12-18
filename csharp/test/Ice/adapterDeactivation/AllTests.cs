//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    namespace adapterDeactivation
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.TestIntfPrx allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing stringToProxy... ");
                output.Flush();
                string @ref = "test:" + helper.getTestEndpoint(0);
                Ice.IObjectPrx @base = IObjectPrx.Parse(@ref, communicator);
                test(@base != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var obj = Test.TestIntfPrx.CheckedCast(@base);
                test(obj != null);
                test(obj.Equals(@base));
                output.WriteLine("ok");

                {
                    output.Write("creating/destroying/recreating object adapter... ");
                    output.Flush();
                    ObjectAdapter adapter =
                        communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                    try
                    {
                        communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                        test(false);
                    }
                    catch (AlreadyRegisteredException)
                    {
                    }
                    adapter.Destroy();

                    //
                    // Use a different port than the first adapter to avoid an "address already in use" error.
                    //
                    adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
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
                        comm.destroy();
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing object adapter published endpoints... ");
                output.Flush();
                {
                    communicator.SetProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
                    var adapter = communicator.createObjectAdapter("PAdapter");
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    var endpt = adapter.GetPublishedEndpoints()[0];
                    test(endpt.ToString().Equals("tcp -h localhost -p 12345 -t 30000"));
                    var prx = IObjectPrx.Parse("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000", communicator);
                    adapter.SetPublishedEndpoints(prx.Endpoints);
                    test(adapter.GetPublishedEndpoints().Length == 2);
                    test(Collections.Equals(adapter.CreateProxy(new Identity("dummy", "")).Endpoints, prx.Endpoints));
                    test(Collections.Equals(adapter.GetPublishedEndpoints(), prx.Endpoints));
                    adapter.RefreshPublishedEndpoints();
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    test(adapter.GetPublishedEndpoints()[0].Equals(endpt));
                    communicator.SetProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
                    adapter.RefreshPublishedEndpoints();
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    test(adapter.GetPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 12345 -t 20000"));
                    adapter.Destroy();
                    test(adapter.GetPublishedEndpoints().Length == 0);
                }
                output.WriteLine("ok");

                if (obj.GetConnection() != null)
                {
                    output.Write("testing object adapter with bi-dir connection... ");
                    output.Flush();
                    var adapter = communicator.createObjectAdapter("");
                    obj.GetConnection().setAdapter(adapter);
                    obj.GetConnection().setAdapter(null);
                    adapter.Deactivate();
                    try
                    {
                        obj.GetConnection().setAdapter(adapter);
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
                    var routerId = new Identity();
                    routerId.name = "router";
                    var router = RouterPrx.UncheckedCast(@base.Clone(routerId, connectionId: "rc"));
                    var adapter = communicator.createObjectAdapterWithRouter("", router);
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    test(adapter.GetPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 23456 -t 30000"));
                    adapter.RefreshPublishedEndpoints();
                    test(adapter.GetPublishedEndpoints().Length == 1);
                    test(adapter.GetPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 23457 -t 30000"));
                    try
                    {
                        adapter.SetPublishedEndpoints(router.Endpoints);
                        test(false);
                    }
                    catch (ArgumentException)
                    {
                        // Expected.
                    }
                    adapter.Destroy();

                    try
                    {
                        routerId.name = "test";
                        router = RouterPrx.UncheckedCast(@base.Clone(routerId));
                        communicator.createObjectAdapterWithRouter("", router);
                        test(false);
                    }
                    catch (OperationNotExistException)
                    {
                        // Expected: the "test" object doesn't implement Ice::Router!
                    }

                    try
                    {
                        router = RouterPrx.Parse($"test:{helper.getTestEndpoint(1)}", communicator);
                        communicator.createObjectAdapterWithRouter("", router);
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
                    var adapter1 = communicator.createObjectAdapterWithEndpoints("Adpt1", helper.getTestEndpoint(10));
                    try
                    {
                        communicator.createObjectAdapterWithEndpoints("Adpt2", helper.getTestEndpoint(10));
                        test(false);
                    }
                    catch (LocalException)
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
                catch (LocalException)
                {
                    output.WriteLine("ok");
                }

                return obj;
            }
        }
    }
}
