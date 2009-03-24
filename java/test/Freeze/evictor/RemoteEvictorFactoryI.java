// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.evictor;
import test.Freeze.evictor.Test.*;

public final class RemoteEvictorFactoryI extends _RemoteEvictorFactoryDisp
{
    RemoteEvictorFactoryI(Ice.ObjectAdapter adapter, String envName)
    {
        _adapter = adapter;
        _envName = envName;
    }

    
    public RemoteEvictorPrx
    createEvictor(String name, boolean transactional, Ice.Current current)
    {
        RemoteEvictorI remoteEvictor = new RemoteEvictorI(_adapter, _envName, name, transactional);
        return RemoteEvictorPrxHelper.
            uncheckedCast(_adapter.add(remoteEvictor, _adapter.getCommunicator().stringToIdentity(name)));
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private String _envName;
}
