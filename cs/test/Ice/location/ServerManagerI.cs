// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public class ServerManagerI : ServerManager_Disp
{
    internal ServerManagerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
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
        _serverCommunicator = Ice.Util.initialize(ref argv);
        _serverCommunicator.getProperties().setProperty("TestAdapter.Endpoints", "default");
        _serverCommunicator.getProperties().setProperty("TestAdapter.AdapterId", "TestAdapter");
        Ice.ObjectAdapter adapter = _serverCommunicator.createObjectAdapter("TestAdapter");
        
        Ice.ObjectPrx locator = _serverCommunicator.stringToProxy("locator:default -p 12345");
        adapter.setLocator(Ice.LocatorPrxHelper.uncheckedCast(locator));
        
        Ice.Object object_Renamed = new TestI(adapter);
        Ice.ObjectPrx proxy = adapter.add(object_Renamed, Ice.Util.stringToIdentity("test"));
        adapter.activate();
    }
    
    public override void shutdown(Ice.Current current)
    {
        _serverCommunicator.shutdown();
        _adapter.getCommunicator().shutdown();
    }
    
    private Ice.ObjectAdapter _adapter;
    private Ice.Communicator _serverCommunicator;
}
