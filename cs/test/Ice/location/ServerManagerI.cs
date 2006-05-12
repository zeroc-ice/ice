// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using Test;

public class ServerManagerI : ServerManagerDisp_
{
    internal ServerManagerI(Ice.ObjectAdapter adapter, ServerLocatorRegistry registry)
    {
        _adapter = adapter;
	_registry = registry;
	_communicators = new ArrayList();
    }
    
    public override void startServer(Ice.Current current)
    {
        string[] argv = new string[0];
        
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
        Ice.Communicator serverCommunicator = Ice.Util.initialize(ref argv);
	_communicators.Add(serverCommunicator);
        serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints", "default");
        serverCommunicator.getProperties().setProperty("TestAdapter.AdapterId", "TestAdapter");
        serverCommunicator.getProperties().setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
        Ice.ObjectAdapter adapter = serverCommunicator.createObjectAdapter("TestAdapter");

        serverCommunicator.getProperties().setProperty("TestAdapter2.Endpoints", "default");
        serverCommunicator.getProperties().setProperty("TestAdapter2.AdapterId", "TestAdapter2");
        Ice.ObjectAdapter adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2");
        
        Ice.ObjectPrx locator = serverCommunicator.stringToProxy("locator:default -p 12010 -t 30000");
        adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));
        adapter2.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));
        
	Ice.Object @object = new TestI(adapter, adapter2, _registry);
	_registry.addObject(adapter.add(@object, serverCommunicator.stringToIdentity("test")));
	_registry.addObject(adapter.add(@object, serverCommunicator.stringToIdentity("test2")));

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
        _adapter.getCommunicator().shutdown();
    }
    
    private Ice.ObjectAdapter _adapter;
    private ServerLocatorRegistry _registry;
    private ArrayList _communicators;
}
