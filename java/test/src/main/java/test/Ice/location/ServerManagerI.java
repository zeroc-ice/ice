
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import test.Ice.location.Test.ServerManager;

public class ServerManagerI implements ServerManager
{
    ServerManagerI(ServerLocatorRegistry registry, com.zeroc.Ice.InitializationData initData, test.Util.Application app)
    {
        _registry = registry;
        _communicators = new java.util.ArrayList<com.zeroc.Ice.Communicator>();

        _app = app;
        _initData = initData;

        _initData.properties.setProperty("TestAdapter.AdapterId", "TestAdapter");
        _initData.properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
        _initData.properties.setProperty("TestAdapter2.AdapterId", "TestAdapter2");
    }

    @Override
    public void startServer(com.zeroc.Ice.Current current)
    {
        for(com.zeroc.Ice.Communicator c : _communicators)
        {
            c.waitForShutdown();
            c.destroy();
        }
        _communicators.clear();

        //
        // Simulate a server: create a new communicator and object
        // adapter. The object adapter is started on a system allocated
        // port. The configuration used here contains the Ice.Locator
        // configuration variable. The new object adapter will register
        // its endpoints with the locator and create references containing
        // the adapter id instead of the endpoints.
        //
        com.zeroc.Ice.Communicator serverCommunicator = _app.initialize(_initData);
        _communicators.add(serverCommunicator);

        //
        // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
        // another OA (e.g.: TestAdapter2 is re-activated using port of TestAdapter).
        //
        int nRetry = 10;
        while(--nRetry > 0)
        {
            com.zeroc.Ice.ObjectAdapter adapter = null;
            com.zeroc.Ice.ObjectAdapter adapter2 = null;
            try
            {
                serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints",
                                                               _app.getTestEndpoint(_nextPort++));
                serverCommunicator.getProperties().setProperty("TestAdapter2.Endpoints",
                                                               _app.getTestEndpoint(_nextPort++));

                adapter = serverCommunicator.createObjectAdapter("TestAdapter");
                adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2");

                com.zeroc.Ice.ObjectPrx locator = serverCommunicator.stringToProxy("locator:"  +
                                                                                   _app.getTestEndpoint(0));
                adapter.setLocator(com.zeroc.Ice.LocatorPrx.uncheckedCast(locator));
                adapter2.setLocator(com.zeroc.Ice.LocatorPrx.uncheckedCast(locator));

                com.zeroc.Ice.Object object = new TestI(adapter, adapter2, _registry);
                _registry.addObject(adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("test")), null);
                _registry.addObject(adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("test2")), null);
                adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("test3"));

                adapter.activate();
                adapter2.activate();
                break;
            }
            catch(com.zeroc.Ice.SocketException ex)
            {
                if(nRetry == 0)
                {
                    throw ex;
                }

                // Retry, if OA creation fails with EADDRINUSE (this can occur when running with JS web
                // browser clients if the driver uses ports in the same range as this test, ICE-8148)
                if(adapter != null)
                {
                    adapter.destroy();
                }
                if(adapter2 != null)
                {
                    adapter2.destroy();
                }
            }
        }
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        for(com.zeroc.Ice.Communicator c : _communicators)
        {
            c.destroy();
        }
        current.adapter.getCommunicator().shutdown();
    }

    private ServerLocatorRegistry _registry;
    private java.util.List<com.zeroc.Ice.Communicator> _communicators;
    private com.zeroc.Ice.InitializationData _initData;
    private test.Util.Application _app;
    private int _nextPort = 1;
}
