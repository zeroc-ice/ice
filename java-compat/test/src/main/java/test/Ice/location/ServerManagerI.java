
// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import test.Ice.location.Test._ServerManagerDisp;

public class ServerManagerI extends _ServerManagerDisp
{
    ServerManagerI(ServerLocatorRegistry registry, test.TestHelper helper)
    {
        _registry = registry;
        _communicators = new java.util.ArrayList<Ice.Communicator>();

        _helper = helper;
        _properties = helper.communicator().getProperties()._clone();

        _properties.setProperty("TestAdapter.AdapterId", "TestAdapter");
        _properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
        _properties.setProperty("TestAdapter2.AdapterId", "TestAdapter2");
    }

    @Override
    public void
    startServer(Ice.Current current)
    {
        for(Ice.Communicator c : _communicators)
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
        Ice.Communicator serverCommunicator = _helper.initialize(_properties);
        _communicators.add(serverCommunicator);

        int nRetry = 10;
        while(--nRetry > 0)
        {
            Ice.ObjectAdapter adapter = null;
            Ice.ObjectAdapter adapter2 = null;
            try
            {
                //
                // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
                // another OA (e.g.: TestAdapter2 is re-activated using port of TestAdapter).
                //
                serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints",
                                                               _helper.getTestEndpoint(_nextPort++));
                serverCommunicator.getProperties().setProperty("TestAdapter2.Endpoints",
                                                               _helper.getTestEndpoint(_nextPort++));

                adapter = serverCommunicator.createObjectAdapter("TestAdapter");
                adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2");

                Ice.ObjectPrx locator = serverCommunicator.stringToProxy("locator:" + _helper.getTestEndpoint(0));
                adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));
                adapter2.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));

                Ice.Object object = new TestI(adapter, adapter2, _registry);
                _registry.addObject(adapter.add(object, Ice.Util.stringToIdentity("test")));
                _registry.addObject(adapter.add(object, Ice.Util.stringToIdentity("test2")));
                adapter.add(object, Ice.Util.stringToIdentity("test3"));

                adapter.activate();
                adapter2.activate();
                break;
            }
            catch(Ice.SocketException ex)
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
    public void
    shutdown(Ice.Current current)
    {
        for(Ice.Communicator c : _communicators)
        {
            c.destroy();
        }
        current.adapter.getCommunicator().shutdown();
    }

    private ServerLocatorRegistry _registry;
    private java.util.List<Ice.Communicator> _communicators;
    private Ice.Properties _properties;
    private test.TestHelper _helper;
    private int _nextPort = 1;
}
