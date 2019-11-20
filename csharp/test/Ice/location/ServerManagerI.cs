//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using Ice.location.Test;

namespace Ice
{
    namespace location
    {
        public class ServerManagerI : Test.ServerManager
        {
            internal ServerManagerI(ServerLocatorRegistry registry, global::Test.TestHelper helper)
            {
                _registry = registry;
                _communicators = new ArrayList();
                _helper = helper;
            }

            public void startServer(Ice.Current current)
            {
                foreach (Ice.Communicator c in _communicators)
                {
                    c.waitForShutdown();
                    c.destroy();
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
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = _helper.communicator().getProperties().ice_clone_();
                initData.properties.setProperty("TestAdapter.AdapterId", "TestAdapter");
                initData.properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
                initData.properties.setProperty("TestAdapter2.AdapterId", "TestAdapter2");

                Ice.Communicator serverCommunicator = _helper.initialize(initData);
                _communicators.Add(serverCommunicator);

                //
                // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
                // another OA(e.g.: TestAdapter2 is re-activated using port of TestAdapter).
                //
                int nRetry = 10;
                while (--nRetry > 0)
                {
                    Ice.ObjectAdapter adapter = null;
                    Ice.ObjectAdapter adapter2 = null;
                    try
                    {
                        serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints",
                                                                       _helper.getTestEndpoint(_nextPort++));
                        serverCommunicator.getProperties().setProperty("TestAdapter2.Endpoints",
                                                                       _helper.getTestEndpoint(_nextPort++));

                        adapter = serverCommunicator.createObjectAdapter("TestAdapter");
                        adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2");

                        Ice.ObjectPrx locator = serverCommunicator.stringToProxy("locator:" + _helper.getTestEndpoint(0));
                        adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));
                        adapter2.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));

                        var testI = new TestI(adapter, adapter2, _registry);
                        _registry.addObject(adapter.Add(testI, Util.stringToIdentity("test")));
                        _registry.addObject(adapter.Add(testI, Util.stringToIdentity("test2")));
                        adapter.Add(testI, Util.stringToIdentity("test3"));

                        adapter.activate();
                        adapter2.activate();
                        break;
                    }
                    catch (Ice.SocketException ex)
                    {
                        if (nRetry == 0)
                        {
                            throw ex;
                        }

                        // Retry, if OA creation fails with EADDRINUSE(this can occur when running with JS web
                        // browser clients if the driver uses ports in the same range as this test, ICE-8148)
                        if (adapter != null)
                        {
                            adapter.destroy();
                        }
                        if (adapter2 != null)
                        {
                            adapter2.destroy();
                        }
                    }
                }
            }

            public void shutdown(Ice.Current current)
            {
                foreach (Ice.Communicator c in _communicators)
                {
                    c.destroy();
                }
                _communicators.Clear();
                current.adapter.getCommunicator().shutdown();
            }

            private ServerLocatorRegistry _registry;
            private ArrayList _communicators;
            private global::Test.TestHelper _helper;
            private int _nextPort = 1;
        }
    }
}
