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

public final class RemoteEvictorFactoryI extends Test._RemoteEvictorFactoryDisp
{
    RemoteEvictorFactoryI(Ice.ObjectAdapter adapter, String envName)
    {
        _adapter = adapter;
	_envName = envName;
    }

    
    public Test.RemoteEvictorPrx
    createEvictor(String name, Ice.Current current)
    {
	RemoteEvictorI remoteEvictor = new RemoteEvictorI(_adapter, _envName, name);
	return Test.RemoteEvictorPrxHelper.
	    uncheckedCast(_adapter.add(remoteEvictor, Ice.Util.stringToIdentity(name)));
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private String _envName;
}
