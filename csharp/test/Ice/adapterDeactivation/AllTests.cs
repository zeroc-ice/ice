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
                Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
                test(@base != null);
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                var obj = Test.TestIntfPrxHelper.checkedCast(@base);
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
                    adapter.destroy();

                    //
                    // Use a different port than the first adapter to avoid an "address already in use" error.
                    //
                    adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                    adapter.destroy();
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
                        var initData = new InitializationData();
                        initData.properties = communicator.getProperties().ice_clone_();
                        var comm = Util.initialize(initData);
                        comm.stringToProxy("test:" + helper.getTestEndpoint(0)).ice_pingAsync();
                        comm.destroy();
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing object adapter published endpoints... ");
                output.Flush();
                {
                    communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
                    var adapter = communicator.createObjectAdapter("PAdapter");
                    test(adapter.getPublishedEndpoints().Length == 1);
                    var endpt = adapter.getPublishedEndpoints()[0];
                    test(endpt.ToString().Equals("tcp -h localhost -p 12345 -t 30000"));
                    ObjectPrx prx =
                        communicator.stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
                    adapter.setPublishedEndpoints(prx.ice_getEndpoints());
                    test(adapter.getPublishedEndpoints().Length == 2);
                    var id = new Identity();
                    id.name = "dummy";
                    test(IceUtilInternal.Arrays.Equals(adapter.createProxy(id).ice_getEndpoints(), prx.ice_getEndpoints()));
                    test(IceUtilInternal.Arrays.Equals(adapter.getPublishedEndpoints(), prx.ice_getEndpoints()));
                    adapter.refreshPublishedEndpoints();
                    test(adapter.getPublishedEndpoints().Length == 1);
                    test(adapter.getPublishedEndpoints()[0].Equals(endpt));
                    communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 20000");
                    adapter.refreshPublishedEndpoints();
                    test(adapter.getPublishedEndpoints().Length == 1);
                    test(adapter.getPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 12345 -t 20000"));
                    adapter.destroy();
                    test(adapter.getPublishedEndpoints().Length == 0);
                }
                output.WriteLine("ok");

                if (obj.ice_getConnection() != null)
                {
                    output.Write("testing object adapter with bi-dir connection... ");
                    output.Flush();
                    var adapter = communicator.createObjectAdapter("");
                    obj.ice_getConnection().setAdapter(adapter);
                    obj.ice_getConnection().setAdapter(null);
                    adapter.deactivate();
                    try
                    {
                        obj.ice_getConnection().setAdapter(adapter);
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
                    var router =
                       RouterPrxHelper.uncheckedCast(@base.ice_identity(routerId).ice_connectionId("rc"));
                    var adapter = communicator.createObjectAdapterWithRouter("", router);
                    test(adapter.getPublishedEndpoints().Length == 1);
                    test(adapter.getPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 23456 -t 30000"));
                    adapter.refreshPublishedEndpoints();
                    test(adapter.getPublishedEndpoints().Length == 1);
                    test(adapter.getPublishedEndpoints()[0].ToString().Equals("tcp -h localhost -p 23457 -t 30000"));
                    try
                    {
                        adapter.setPublishedEndpoints(router.ice_getEndpoints());
                        test(false);
                    }
                    catch (ArgumentException)
                    {
                        // Expected.
                    }
                    adapter.destroy();

                    try
                    {
                        routerId.name = "test";
                        router = RouterPrxHelper.uncheckedCast(@base.ice_identity(routerId));
                        communicator.createObjectAdapterWithRouter("", router);
                        test(false);
                    }
                    catch (OperationNotExistException)
                    {
                        // Expected: the "test" object doesn't implement Ice::Router!
                    }

                    try
                    {
                        router = RouterPrxHelper.uncheckedCast(communicator.stringToProxy("test:" +
                                                                                              helper.getTestEndpoint(1)));
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
                    adapter1.destroy();
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
                    obj.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
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
