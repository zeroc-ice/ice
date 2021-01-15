// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.AdapterDeactivation
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            bool ice1 = TestHelper.GetTestProtocol(communicator.GetProperties()) == Protocol.Ice1;
            TextWriter output = helper.Output;
            output.Write("testing stringToProxy... ");
            output.Flush();
            var obj = ITestIntfPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            output.WriteLine("ok");

            {
                output.Write("creating/destroying/recreating object adapter... ");
                output.Flush();
                {
                    await using var adapter = communicator.CreateObjectAdapterWithEndpoints(
                        "TransientTestAdapter",
                        helper.GetTestEndpoint(1));
                    try
                    {
                        communicator.CreateObjectAdapterWithEndpoints("TransientTestAdapter",
                                                                      helper.GetTestEndpoint(2));
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                }

                // Use a different port than the first adapter to avoid an "address already in use" error.
                {
                    var adapter = communicator.CreateObjectAdapterWithEndpoints(
                        "TransientTestAdapter",
                        helper.GetTestEndpoint(2));

                    TestHelper.Assert(!adapter.ShutdownComplete.IsCompleted);
                    await adapter.DisposeAsync();
                    TestHelper.Assert(adapter.ShutdownComplete.IsCompletedSuccessfully);
                }
                output.WriteLine("ok");
            }

            output.Write("creating/activating/disposing object adapter in one operation... ");
            output.Flush();
            obj.Transient();
            obj.TransientAsync().Wait();
            output.WriteLine("ok");

            {
                output.Write("testing connection closure... ");
                output.Flush();
                for (int i = 0; i < 10; ++i)
                {
                    await using var comm = new Communicator(communicator.GetProperties());
                    _ = IObjectPrx.Parse(helper.GetTestProxy("test", 0), communicator).IcePingAsync();
                }
                output.WriteLine("ok");
            }

            {
                output.Write("testing invalid object adapter endpoints... ");
                output.Flush();
                try
                {
                    communicator.CreateObjectAdapterWithEndpoints(
                        "BadAdapter1",
                        ice1 ? "tcp -h localhost -p 0" : "ice+tcp://localhost:0");
                    TestHelper.Assert(false);
                }
                catch (InvalidConfigurationException)
                {
                    // expected
                }

                try
                {
                    communicator.CreateObjectAdapterWithEndpoints(
                        "BadAdapter2",
                        ice1 ? "tcp -h 127.0.0.1 -p 0:tcp -h \"::1\" -p 10000" :
                            "ice+tcp://127.0.0.1:0?alt-endpoint=[::1]:10000");
                    TestHelper.Assert(false);
                }
                catch (InvalidConfigurationException)
                {
                    // expected
                }
                output.WriteLine("ok");
            }

            {
                output.Write("testing object adapter default published endpoints... ");
                string testHost = "testhost";
                communicator.SetProperty("DAdapter.ServerName", testHost);
                if (ice1)
                {
                    communicator.SetProperty("DAdapter.AcceptNonSecure", "Always");
                }
                {
                    communicator.SetProperty(
                        "DAdapter.Endpoints",
                        ice1 ? "tcp -h \"::0\" -p 0" : "ice+tcp://[::0]:0");

                    await using var adapter = communicator.CreateObjectAdapter("DAdapter");
                    TestHelper.Assert(adapter.PublishedEndpoints.Count == 1);
                    Endpoint publishedEndpoint = adapter.PublishedEndpoints[0];
                    TestHelper.Assert(publishedEndpoint.Host == testHost);
                }
                {
                    communicator.SetProperty(
                        "DAdapter.Endpoints",
                        ice1 ? $"{helper.GetTestEndpoint(1)}:{helper.GetTestEndpoint(2)}" :
                            $"{helper.GetTestEndpoint(1)}?alt-endpoint={helper.GetTestEndpoint(2)}");

                    await using var adapter = communicator.CreateObjectAdapter("DAdapter");
                    TestHelper.Assert(adapter.PublishedEndpoints.Count == 2);
                    Endpoint publishedEndpoint0 = adapter.PublishedEndpoints[0];
                    TestHelper.Assert(publishedEndpoint0.Host == testHost);
                    TestHelper.Assert(publishedEndpoint0.Port == helper.BasePort + 1);
                    Endpoint publishedEndpoint1 = adapter.PublishedEndpoints[1];
                    TestHelper.Assert(publishedEndpoint1.Host == testHost);
                    TestHelper.Assert(publishedEndpoint1.Port == helper.BasePort + 2);
                }
                output.WriteLine("ok");
            }

            output.Write("testing object adapter published endpoints... ");
            output.Flush();
            {
                communicator.SetProperty("PAdapter.PublishedEndpoints",
                    ice1 ? "tcp -h localhost -p 12345 -t 30000" : "ice+tcp://localhost:12345");
                await using var adapter = communicator.CreateObjectAdapter("PAdapter");
                TestHelper.Assert(adapter.PublishedEndpoints.Count == 1);
                Endpoint? endpt = adapter.PublishedEndpoints[0];
                TestHelper.Assert(endpt != null);
                if (ice1)
                {
                    TestHelper.Assert(endpt.ToString() == "tcp -h localhost -p 12345 -t 30000");
                }
                else
                {
                    TestHelper.Assert(endpt.ToString() == "ice+tcp://localhost:12345");
                }
            }
            output.WriteLine("ok");

            Connection connection = await obj.GetConnectionAsync();
            {
                output.Write("testing object adapter with bi-dir connection... ");
                output.Flush();
                ObjectAdapter adapter = communicator.CreateObjectAdapter();
                connection.Adapter = adapter;
                connection.Adapter = null;
                await adapter.DisposeAsync();
                // Setting a deactivated adapter on a connection no longer raise ObjectAdapterDeactivatedException
                connection.Adapter = adapter;
                output.WriteLine("ok");
            }

            output.Write("testing object adapter with router... ");
            output.Flush();
            if (ice1)
            {
                var routerId = new Identity("router", "");
                IRouterPrx router = obj.Clone(IRouterPrx.Factory, label: "rc", identity: routerId);
                {
                    await using var adapter = communicator.CreateObjectAdapterWithRouter(router);
                    await adapter.ActivateAsync();
                    TestHelper.Assert(adapter.PublishedEndpoints.Count == 1);
                    string endpointsStr = adapter.PublishedEndpoints[0].ToString();
                    TestHelper.Assert(endpointsStr == "tcp -h localhost -p 23456 -t 60000");
                }

                try
                {
                    routerId = new Identity("test", "");
                    router = obj.Clone(IRouterPrx.Factory, identity: routerId);
                    await using var adapter = communicator.CreateObjectAdapterWithRouter(router);
                    await adapter.ActivateAsync();
                    TestHelper.Assert(false);
                }
                catch (OperationNotExistException)
                {
                    // Expected: the "test" object doesn't implement Ice::Router!
                }

                try
                {
                    router = IRouterPrx.Parse(helper.GetTestProxy("test", 1), communicator);
                    await using var adapter = communicator.CreateObjectAdapterWithRouter(router);
                    await adapter.ActivateAsync();
                    TestHelper.Assert(false);
                }
                catch (ConnectFailedException)
                {
                }
            }
            else
            {
                try
                {
                    await using var adapter = communicator.CreateObjectAdapterWithRouter(
                        obj.Clone(IRouterPrx.Factory, label: "rc", identity: new Identity("router", "")));

                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                    // expected.
                }
            }
            output.WriteLine("ok");

            output.Write("testing object adapter creation with port in use... ");
            output.Flush();
            {
                await using var adapter1 = communicator.CreateObjectAdapterWithEndpoints("Adpt1",
                                                                                         helper.GetTestEndpoint(10));
                try
                {
                    communicator.CreateObjectAdapterWithEndpoints("Adpt2", helper.GetTestEndpoint(10));
                    TestHelper.Assert(false);
                }
                catch
                {
                    // Expected can't re-use the same endpoint.
                }
            }
            output.WriteLine("ok");

            output.Write("deactivating object adapter in the server... ");
            output.Flush();
            obj.Deactivate();
            output.WriteLine("ok");

            output.Write("testing whether server is gone... ");
            output.Flush();
            try
            {
                obj.IcePing();
                TestHelper.Assert(false);
            }
            catch
            {
                output.WriteLine("ok");
            }
        }
    }
}
