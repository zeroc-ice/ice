// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using Test;

public class ServerManagerI : _ServerManagerDisp
{
    internal ServerManagerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
	_communicators = new ArrayList();
    }
    
    public override void startServer(Ice.Current current)
    {
        string[] argv = new string[0];
        
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
        Ice.ObjectAdapter adapter = serverCommunicator.createObjectAdapter("TestAdapter");
        
        Ice.ObjectPrx locator = serverCommunicator.stringToProxy("locator:default -p 12345 -t 30000");
        adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));
        
        Ice.Object @object = new TestI(adapter);
        Ice.ObjectPrx proxy = adapter.add(@object, Ice.Util.stringToIdentity("test"));
        adapter.activate();
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
    private ArrayList _communicators;
}
