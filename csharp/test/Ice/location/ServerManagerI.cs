//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.location.Test;
using System.Collections.Generic;
using Test;

namespace Ice.location
{
    public class ServerManager : IServerManager
    {
        internal ServerManager(ServerLocatorRegistry registry, TestHelper helper)
        {
            _registry = registry;
            _helper = helper;
        }

        public void startServer(Current current)
        {
            foreach (Communicator? c in _communicators)
            {
                c.WaitForShutdown();
                c.Destroy();
            }
            _communicators.Clear();

            //
            // Simulate a server: create a new communicator and object
            // adapter. The object adapter is started on a system allocated
            // port. The configuration used here contains the Ice.Locator
            // configuration variable. The new object adapter will register
            // its endpoints with the locator and create references containing
            // the adapter id instead of the endpoints.
            //
            Dictionary<string, string> properties = _helper.Communicator()!.GetProperties();
            properties["TestAdapter.AdapterId"] = "TestAdapter";
            properties["TestAdapter.ReplicaGroupId"] = "ReplicatedAdapter";
            properties["TestAdapter2.AdapterId"] = "TestAdapter2";

            Communicator serverCommunicator = _helper.Initialize(properties);
            _communicators.Add(serverCommunicator);

            //
            // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
            // another OA(e.g.: TestAdapter2 is re-activated using port of TestAdapter).
            //
            int nRetry = 10;
            while (--nRetry > 0)
            {
                ObjectAdapter? adapter = null;
                ObjectAdapter? adapter2 = null;
                try
                {
                    serverCommunicator.SetProperty("TestAdapter.Endpoints",
                                                                _helper.GetTestEndpoint(_nextPort++));
                    serverCommunicator.SetProperty("TestAdapter2.Endpoints",
                                                                _helper.GetTestEndpoint(_nextPort++));

                    adapter = serverCommunicator.CreateObjectAdapter("TestAdapter");
                    adapter2 = serverCommunicator.CreateObjectAdapter("TestAdapter2");

                    var locator = ILocatorPrx.Parse($"locator:{_helper.GetTestEndpoint(0)}", serverCommunicator);
                    adapter.Locator = locator;
                    adapter2.Locator = locator;

                    var testI = new TestIntf(adapter, adapter2, _registry);
                    _registry.AddObject(adapter.Add("test", testI, Ice.IObjectPrx.Factory));
                    _registry.AddObject(adapter.Add("test2", testI, Ice.IObjectPrx.Factory));
                    adapter.Add("test3", testI);

                    adapter.Activate();
                    adapter2.Activate();
                    break;
                }
                catch (TransportException ex)
                {
                    if (nRetry == 0)
                    {
                        throw ex;
                    }

                    // Retry, if OA creation fails with EADDRINUSE(this can occur when running with JS web
                    // browser clients if the driver uses ports in the same range as this test, ICE-8148)
                    if (adapter != null)
                    {
                        adapter.Destroy();
                    }
                    if (adapter2 != null)
                    {
                        adapter2.Destroy();
                    }
                }
            }
        }

        public void shutdown(Current current)
        {
            foreach (Communicator c in _communicators)
            {
                c.Destroy();
            }
            _communicators.Clear();
            current.Adapter.Communicator.Shutdown();
        }

        private readonly ServerLocatorRegistry _registry;
        private readonly List<Communicator> _communicators = new List<Communicator>();
        private readonly TestHelper _helper;
        private int _nextPort = 1;
    }
}
