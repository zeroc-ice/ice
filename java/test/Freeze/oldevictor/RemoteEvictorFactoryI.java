// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.oldevictor;
import test.Freeze.oldevictor.Test.*;

public final class RemoteEvictorFactoryI extends _RemoteEvictorFactoryDisp
{
    RemoteEvictorFactoryI(String envName)
    {
        _envName = envName;
    }

    
    public RemoteEvictorPrx
    createEvictor(String name, Ice.Current current)
    {
        RemoteEvictorI remoteEvictor = new RemoteEvictorI(current.adapter.getCommunicator(), _envName, name);
        return RemoteEvictorPrxHelper.
            uncheckedCast(current.adapter.add(remoteEvictor, current.adapter.getCommunicator().stringToIdentity(name)));
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private String _envName;
}
