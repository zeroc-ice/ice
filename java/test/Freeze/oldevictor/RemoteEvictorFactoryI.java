// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
