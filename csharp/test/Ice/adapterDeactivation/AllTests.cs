//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.IO;
using System.Linq;
using Test;

namespace ZeroC.Ice.Test.AdapterDeactivation
{
    public class AllTests
    {
        public static ITestIntfPrx Run(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            bool oldProtocol = communicator.DefaultProtocol == Protocol.Ice1;
            TextWriter output = helper.GetWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();
            var baseprx = IObjectPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            TestHelper.Assert(baseprx != null);
            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            var obj = ITestIntfPrx.CheckedCast(baseprx);
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
                adapter.Dispose();

                //
                // Use a different port than the first adapter to avoid an "address already in use" error.
                //
                adapter = communicator.CreateObjectAdapterWithEndpoints("TransientTestAdapter", "default");
                adapter.Dispose();
                output.WriteLine("ok");
            }

            output.Write("creating/activating/deactivating object adapter in one operation... ");
            output.Flush();
            obj.Transient();
            obj.TransientAsync().Wait();
            output.WriteLine("ok");

            {
                output.Write("testing connection closure... ");
                output.Flush();
                for (int i = 0; i < 10; ++i)
                {
                    using var comm = new Communicator(communicator.GetProperties());
                    IObjectPrx.Parse(helper.GetTestProxy("test", 0), communicator).IcePingAsync();
                }
                output.WriteLine("ok");
            }

            output.Write("testing object adapter published endpoints... ");
            output.Flush();
            {
                communicator.SetProperty("PAdapter.PublishedEndpoints", "tcp -h localhost -p 12345 -t 30000");
                ObjectAdapter adapter = communicator.CreateObjectAdapter("PAdapter");
                TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 1);
                Endpoint? endpt = adapter.GetPublishedEndpoints()[0];
                TestHelper.Assert(endpt != null);
                if (oldProtocol)
                {
                    TestHelper.Assert(endpt.ToString()!.Equals("tcp -h localhost -p 12345 -t 30000"));
                }
                else
                {
                    TestHelper.Assert(endpt.ToString()!.Equals("ice+tcp://localhost:12345"));
                }

                var prx = IObjectPrx.Parse(oldProtocol ?
                    "dummy:tcp -h localhost -p 12346 -t 20000:tcp -h localhost -p 12347 -t 10000" :
                    "ice+tcp://localhost:12346/dummy?alt-endpoint=localhost:12347", communicator);
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

                if (oldProtocol)
                {
                    TestHelper.Assert(adapter.GetPublishedEndpoints()[0].ToString()!.Equals("tcp -h localhost -p 12345 -t 20000"));
                }
                else
                {
                    TestHelper.Assert(adapter.GetPublishedEndpoints()[0].ToString()!.Equals("ice+tcp://localhost:12345"));
                }
                adapter.Dispose();
            }
            output.WriteLine("ok");

            Connection? connection = obj.GetConnection();
            if (connection != null)
            {
                output.Write("testing object adapter with bi-dir connection... ");
                output.Flush();
                ObjectAdapter adapter = communicator.CreateObjectAdapter();
                connection.Adapter = adapter;
                connection.Adapter = null;
                adapter.Dispose();
                // Setting a deactivated adapter on a connection no longer raise ObjectAdapterDeactivatedException
                connection.Adapter = adapter;
                output.WriteLine("ok");
            }

            output.Write("testing object adapter with router... ");
            output.Flush();
            {
                var routerId = new Identity("router", "");
                IRouterPrx router = baseprx.Clone(routerId, IRouterPrx.Factory, connectionId: "rc");
                ObjectAdapter adapter = communicator.CreateObjectAdapterWithRouter(router);
                TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 1);
                string endpointsStr = adapter.GetPublishedEndpoints()[0].ToString();
                if (oldProtocol)
                {
                    TestHelper.Assert(endpointsStr == "tcp -h localhost -p 23456 -t 60000");
                }
                else
                {
                    TestHelper.Assert(endpointsStr == "ice+tcp://localhost:23456");
                }
                adapter.RefreshPublishedEndpoints();
                TestHelper.Assert(adapter.GetPublishedEndpoints().Count == 1);

                if (oldProtocol)
                {
                    TestHelper.Assert(
                        adapter.GetPublishedEndpoints()[0].ToString() == "tcp -h localhost -p 23457 -t 60000");
                }
                else
                {
                    TestHelper.Assert(adapter.GetPublishedEndpoints()[0].ToString() == "ice+tcp://localhost:23457");
                }
                try
                {
                    adapter.SetPublishedEndpoints(router.Endpoints);
                    TestHelper.Assert(false);
                }
                catch (InvalidOperationException)
                {
                    // Expected.
                }
                adapter.Dispose();

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
                    router = IRouterPrx.Parse(helper.GetTestProxy("test", 1), communicator);
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
                ObjectAdapter adapter1 = communicator.CreateObjectAdapterWithEndpoints("Adpt1", helper.GetTestEndpoint(10));
                try
                {
                    communicator.CreateObjectAdapterWithEndpoints("Adpt2", helper.GetTestEndpoint(10));
                    TestHelper.Assert(false);
                }
                catch
                {
                    // Expected can't re-use the same endpoint.
                }
                adapter1.Dispose();
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
            catch (Exception)
            {
                output.WriteLine("ok");
            }
            return obj;
        }
    }
}
