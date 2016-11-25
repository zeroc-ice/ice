// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using Test;

public class ServerManagerI : ServerManagerDisp_
{
    internal ServerManagerI(ServerLocatorRegistry registry, Ice.InitializationData initData, TestCommon.Application app)
    {
        _registry = registry;
        _communicators = new ArrayList();
        _initData = initData;
        _app = app;
        _initData.properties.setProperty("TestAdapter.AdapterId", "TestAdapter");
        _initData.properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
        _initData.properties.setProperty("TestAdapter2.AdapterId", "TestAdapter2");
    }

    public override void startServer(Ice.Current current)
    {
        foreach(Ice.Communicator c in _communicators)
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
        Ice.Communicator serverCommunicator = Ice.Util.initialize(_initData);
        _communicators.Add(serverCommunicator);

        //
        // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
        // another OA (e.g.: TestAdapter2 is re-activated using port of TestAdapter).
        //
        serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints", _app.getTestEndpoint(_nextPort++));
        serverCommunicator.getProperties().setProperty("TestAdapter2.Endpoints", _app.getTestEndpoint(_nextPort++));

        Ice.ObjectAdapter adapter = serverCommunicator.createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2");

        Ice.ObjectPrx locator = serverCommunicator.stringToProxy("locator:" + _app.getTestEndpoint(0));
        adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));
        adapter2.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));

        Ice.Object @object = new TestI(adapter, adapter2, _registry);
        _registry.addObject(adapter.add(@object, Ice.Util.stringToIdentity("test")));
        _registry.addObject(adapter.add(@object, Ice.Util.stringToIdentity("test2")));
        adapter.add(@object, Ice.Util.stringToIdentity("test3"));

        adapter.activate();
        adapter2.activate();
    }

    public override void shutdown(Ice.Current current)
    {
        foreach(Ice.Communicator c in _communicators)
        {
            c.destroy();
        }
        _communicators.Clear();
        current.adapter.getCommunicator().shutdown();
    }

    private ServerLocatorRegistry _registry;
    private ArrayList _communicators;
    private Ice.InitializationData _initData;
    private TestCommon.Application _app;
    private int _nextPort = 1;
}
