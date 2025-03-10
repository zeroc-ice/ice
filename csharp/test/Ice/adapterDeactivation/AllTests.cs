// Copyright (c) ZeroC, Inc.

namespace Ice.adapterDeactivation
{
        public class AllTests : global::Test.AllTests
        {
            public static async Task<Test.TestIntfPrx> allTests(global::Test.TestHelper helper)
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
                    Ice.ObjectAdapter adapter =
                        communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                    try
                    {
                        communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                        test(false);
                    }
                    catch (Ice.AlreadyRegisteredException)
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
                await obj.transientAsync();
                output.WriteLine("ok");

                {
                    output.Write("testing connection closure... ");
                    output.Flush();
                    for (int i = 0; i < 10; ++i)
                    {
                        Ice.InitializationData initData = new Ice.InitializationData();
                        initData.properties = communicator.getProperties().Clone();
                        Ice.Communicator comm = Ice.Util.initialize(initData);
                        _ = comm.stringToProxy("test:" + helper.getTestEndpoint(0)).ice_pingAsync();
                        comm.destroy();
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing object adapter published endpoints... ");
                output.Flush();
                {
                    communicator.getProperties().setProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("PAdapter");
                    test(adapter.getPublishedEndpoints().Length == 1);
                    Ice.Endpoint endpt = adapter.getPublishedEndpoints()[0];
                    test(endpt.ToString() == "tcp -h localhost -p 12345 -t 30000");
                    Ice.ObjectPrx prx =
                        communicator.stringToProxy("dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000");
                    adapter.setPublishedEndpoints(prx.ice_getEndpoints());
                    test(adapter.getPublishedEndpoints().Length == 2);
                    Ice.Identity id = new Ice.Identity("dummy", "");
                    test(Enumerable.SequenceEqual(adapter.createProxy(id).ice_getEndpoints(), prx.ice_getEndpoints()));
                    test(Enumerable.SequenceEqual(adapter.getPublishedEndpoints(), prx.ice_getEndpoints()));
                    adapter.destroy();
                    test(adapter.getPublishedEndpoints().Length == 0);
                }
                output.WriteLine("ok");

                output.Write("testing object adapter published host... ");
                output.Flush();
                {
                    communicator.getProperties().setProperty("PHAdapter.Endpoints", "default -h *");

                    // PublishedHost not set
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 1);
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo &&
                            ipEndpointInfo.host.Length > 0);
                        adapter.destroy();
                    }

                    communicator.getProperties().setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 1);
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo &&
                            ipEndpointInfo.host == "test.zeroc.com");
                        adapter.destroy();
                    }
                    // Listening on loopback
                    communicator.getProperties().setProperty("PHAdapter.Endpoints", "default -h 127.0.0.1");

                    communicator.getProperties().setProperty("PHAdapter.PublishedHost", "");
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 1);
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo &&
                            ipEndpointInfo.host == "127.0.0.1");
                        adapter.destroy();
                    }

                    communicator.getProperties().setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 1);
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo &&
                            ipEndpointInfo.host == "test.zeroc.com");
                        adapter.destroy();
                    }

                    // Two loopback endpoints with different ports
                    communicator.getProperties().setProperty(
                        "PHAdapter.Endpoints",
                        "default -h 127.0.0.1 -p 12345:default -h 127.0.0.1");

                    communicator.getProperties().setProperty("PHAdapter.PublishedHost", "");
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 2);
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo0 &&
                            ipEndpointInfo0.host == "127.0.0.1" &&
                            ipEndpointInfo0.port == 12345);
                        test(getUnderlying(publishedEndpoints[1].getInfo()) is IPEndpointInfo ipEndpointInfo1 &&
                            ipEndpointInfo1.host == "127.0.0.1");
                        adapter.destroy();
                    }

                    // Two endpoints - one loopback, one not loopback
                    communicator.getProperties().setProperty(
                        "PHAdapter.Endpoints",
                        "default -h 127.0.0.1 -p 12345:default -h *");

                    communicator.getProperties().setProperty("PHAdapter.PublishedHost", "");
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 1); // loopback filtered out
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo &&
                            ipEndpointInfo.host.Length > 0 &&
                            ipEndpointInfo.port != 12345);
                        adapter.destroy();
                    }

                    communicator.getProperties().setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 1); // loopback filtered out
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo &&
                            ipEndpointInfo.host == "test.zeroc.com" &&
                            ipEndpointInfo.port != 12345);
                        adapter.destroy();
                    }

                    // Two non-loopback endpoints
                    communicator.getProperties().setProperty("PHAdapter.Endpoints", "tcp -h * -p 12345:default -h *");

                    communicator.getProperties().setProperty("PHAdapter.PublishedHost", "");
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 2);
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo0 &&
                            ipEndpointInfo0.host.Length > 0 &&
                            ipEndpointInfo0.port == 12345);
                        test(getUnderlying(publishedEndpoints[1].getInfo()) is IPEndpointInfo ipEndpointInfo1 &&
                            ipEndpointInfo1.host.Length > 0 &&
                            ipEndpointInfo1.port != 12345);
                        adapter.destroy();
                    }

                    communicator.getProperties().setProperty("PHAdapter.PublishedHost", "test.zeroc.com");
                    {
                        ObjectAdapter adapter = communicator.createObjectAdapter("PHAdapter");
                        var publishedEndpoints = adapter.getPublishedEndpoints();
                        test(publishedEndpoints.Length == 2);
                        test(getUnderlying(publishedEndpoints[0].getInfo()) is IPEndpointInfo ipEndpointInfo0 &&
                            ipEndpointInfo0.host == "test.zeroc.com" &&
                            ipEndpointInfo0.port == 12345);
                        test(getUnderlying(publishedEndpoints[1].getInfo()) is IPEndpointInfo ipEndpointInfo1 &&
                            ipEndpointInfo1.host == "test.zeroc.com" &&
                            ipEndpointInfo1.port != 12345);
                        adapter.destroy();
                    }

                    static EndpointInfo getUnderlying(EndpointInfo endpointInfo) =>
                        endpointInfo.underlying is null ? endpointInfo : getUnderlying(endpointInfo.underlying);
                }
                output.WriteLine("ok");

                if (obj.ice_getConnection() != null)
                {
                    output.Write("testing object adapter with bi-dir connection... ");
                    output.Flush();
                    test(communicator.getDefaultObjectAdapter() is null);
                    test(obj.ice_getCachedConnection().getAdapter() is null);

                    ObjectAdapter adapter = communicator.createObjectAdapter("");

                    communicator.setDefaultObjectAdapter(adapter);
                    test(communicator.getDefaultObjectAdapter() == adapter);

                    // create new connection
                    await obj.ice_getCachedConnection().closeAsync();
                    await obj.ice_pingAsync();

                    test(obj.ice_getCachedConnection().getAdapter() == adapter);

                    // Ensure destroying the OA doesn't affect the ability to send outgoing requests.
                    adapter.destroy();
                    await obj.ice_getCachedConnection().closeAsync();
                    obj.ice_ping();

                    communicator.setDefaultObjectAdapter(null);

                    // create new connection
                    await obj.ice_getCachedConnection().closeAsync();
                    await obj.ice_pingAsync();

                    adapter = communicator.createObjectAdapter("");
                    test(obj.ice_getCachedConnection().getAdapter() is null);
                    obj.ice_getCachedConnection().setAdapter(adapter);
                    test(obj.ice_getCachedConnection().getAdapter() == adapter);
                    obj.ice_getCachedConnection().setAdapter(null);

                    adapter.destroy();
                    try
                    {
                        obj.ice_getCachedConnection().setAdapter(adapter);
                        test(false);
                    }
                    catch (Ice.ObjectAdapterDestroyedException)
                    {
                    }
                    output.WriteLine("ok");
                }

                output.Write("testing object adapter with router... ");
                output.Flush();
                {
                    Ice.Identity routerId = new Ice.Identity("router", "");
                    Ice.RouterPrx router =
                        Ice.RouterPrxHelper.uncheckedCast(@base.ice_identity(routerId).ice_connectionId("rc"));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithRouter("", router);
                    test(adapter.getPublishedEndpoints().Length == 1);
                    test(adapter.getPublishedEndpoints()[0].ToString() == "tcp -h localhost -p 23456 -t 30000");
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
                        router = Ice.RouterPrxHelper.uncheckedCast(@base.ice_identity(routerId));
                        communicator.createObjectAdapterWithRouter("", router);
                        test(false);
                    }
                    catch (Ice.OperationNotExistException)
                    {
                        // Expected: the "test" object doesn't implement Ice::Router!
                    }

                    try
                    {
                        router = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("test:" +
                                                                                              helper.getTestEndpoint(1)));
                        communicator.createObjectAdapterWithRouter("", router);
                        test(false);
                    }
                    catch (Ice.ConnectFailedException)
                    {
                    }

                    try
                    {
                        router = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("test:" +
                                                                                              helper.getTestEndpoint(1)));

                        communicator.getProperties().setProperty("AdapterWithRouter.Endpoints", "tcp -h 127.0.0.1");
                        communicator.createObjectAdapterWithRouter("AdapterWithRouter", router);
                        test(false);
                    }
                    catch (Ice.InitializationException)
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
                    catch (Ice.LocalException)
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
                if (obj.ice_getConnection() is null) // collocated
                {
                    obj.ice_ping();
                    output.WriteLine("ok");
                }
                else
                {
                    try
                    {
                        obj.ice_invocationTimeout(100).ice_ping(); // Use timeout to speed up testing on Windows
                        test(false);
                    }
                    catch (Ice.LocalException)
                    {
                        output.WriteLine("ok");
                    }
                }

                output.Write("testing server idle time...");
                output.Flush();
                var task1 = Task.Run(() =>
                    {
                        var initData = new Ice.InitializationData()
                        {
                            properties = new Ice.Properties(),
                        };
                        initData.properties.setProperty("Ice.ServerIdleTime", "1");
                        // The thread pool threads have to be idle first before server idle time is checked.
                        initData.properties.setProperty("Ice.ThreadPool.Server.ThreadIdleTime", "1");
                        using var idleCommunicator = Ice.Util.initialize(initData);
                        var idleOA = idleCommunicator.createObjectAdapterWithEndpoints("IdleAdapter", "tcp -h 127.0.0.1");
                        idleOA.activate();
                        idleCommunicator.waitForShutdown();
                    });
                var task2 = Task.Run(async () =>
                    {
                        var initData = new Ice.InitializationData()
                        {
                            properties = new Ice.Properties(),
                        };
                        initData.properties.setProperty("Ice.ServerIdleTime", "0");
                        // The thread pool threads have to be idle first before server idle time is checked.
                        initData.properties.setProperty("Ice.ThreadPool.Server.ThreadIdleTime", "1");
                        using var idleCommunicator = Ice.Util.initialize(initData);
                        {
                            var idleOA = idleCommunicator.createObjectAdapterWithEndpoints("IdleAdapter", "tcp -h 127.0.0.1");
                            idleOA.activate();
                            await Task.Delay(1100);
                            test(!idleCommunicator.isShutdown());
                        }
                    });
                await task1;
                await task2;
                output.WriteLine("ok");

                return obj;
            }
        }
    }

