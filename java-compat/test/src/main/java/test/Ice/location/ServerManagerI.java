
// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import test.Ice.location.Test._ServerManagerDisp;


public class ServerManagerI extends _ServerManagerDisp
{
    ServerManagerI(ServerLocatorRegistry registry, Ice.InitializationData initData, test.Util.Application app)
    {
        _registry = registry;
        _communicators = new java.util.ArrayList<Ice.Communicator>();
        
        _app = app;
        _initData = initData;
        
        _initData.properties.setProperty("TestAdapter.AdapterId", "TestAdapter");
        _initData.properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
        _initData.properties.setProperty("TestAdapter2.AdapterId", "TestAdapter2");
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
        Ice.Communicator serverCommunicator = _app.initialize(_initData);
        _communicators.add(serverCommunicator);

        //
        // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
        // another OA (e.g.: TestAdapter2 is re-activated using port of TestAdapter).
        //
        serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p " + _nextPort++);
        serverCommunicator.getProperties().setProperty("TestAdapter2.Endpoints", "default -p " + _nextPort++);

        Ice.ObjectAdapter adapter = serverCommunicator.createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2");

        Ice.ObjectPrx locator = serverCommunicator.stringToProxy("locator:default -p 12010");
        adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));
        adapter2.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));

        Ice.Object object = new TestI(adapter, adapter2, _registry);
        _registry.addObject(adapter.add(object, Ice.Util.stringToIdentity("test")));
        _registry.addObject(adapter.add(object, Ice.Util.stringToIdentity("test2")));
        adapter.add(object, Ice.Util.stringToIdentity("test3"));
        
        adapter.activate();
        adapter2.activate();
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
    private Ice.InitializationData _initData;
    private test.Util.Application _app;
    private int _nextPort = 12011;
}
