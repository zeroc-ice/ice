// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public class ServerManagerI extends _ServerManagerDisp
{
    ServerManagerI(Ice.ObjectAdapter adapter)
    {
	_adapter = adapter;
	_communicators = new java.util.ArrayList();
    }

    public void
    startServer(Ice.Current current)
    {
	String[] argv = new String[0];

	//
	// Simulate a server: create a new communicator and object
	// adapter. The object adapter is started on a system allocated
	// port. The configuration used here contains the Ice.Locator
	// configuration variable. The new object adapter will register
	// its endpoints with the locator and create references containing
	// the adapter id instead of the endpoints.
	//
	Ice.Communicator serverCommunicator = Ice.Util.initialize(argv);
	_communicators.add(serverCommunicator);
	serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints", "default");
	serverCommunicator.getProperties().setProperty("TestAdapter.AdapterId", "TestAdapter");
	Ice.ObjectAdapter adapter = serverCommunicator.createObjectAdapter("TestAdapter");

	Ice.ObjectPrx locator = serverCommunicator.stringToProxy("locator:default -p 12345 -t 30000");
	adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));

	Ice.Object object = new TestI(adapter);
	Ice.ObjectPrx proxy = adapter.add(object, Ice.Util.stringToIdentity("test"));
	adapter.activate();
    }

    public void
    shutdown(Ice.Current current)
    {
        java.util.Iterator i = _communicators.iterator();
	while(i.hasNext())
	{
	    ((Ice.Communicator)i.next()).destroy();
	}
	_adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private java.util.ArrayList _communicators;
}
