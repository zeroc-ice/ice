// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public class ServerManagerI extends _ServerManagerDisp
{
    ServerManagerI(Ice.ObjectAdapter adapter)
    {
	_adapter = adapter;
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
	_serverCommunicator = Ice.Util.initialize(argv);
	_serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints", "default");
	_serverCommunicator.getProperties().setProperty("TestAdapter.AdapterId", "TestAdapter");
	Ice.ObjectAdapter adapter = _serverCommunicator.createObjectAdapter("TestAdapter");

	Ice.ObjectPrx locator = _serverCommunicator.stringToProxy("locator:default -p 12345 -t 30000");
	adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));

	Ice.Object object = new TestI(adapter);
	Ice.ObjectPrx proxy = adapter.add(object, Ice.Util.stringToIdentity("test"));
	adapter.activate();
    }

    public void
    shutdown(Ice.Current current)
    {
	_serverCommunicator.shutdown();
	_adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private Ice.Communicator _serverCommunicator;
}
