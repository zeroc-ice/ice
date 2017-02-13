// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.evictor;
import test.Freeze.evictor.Test.*;

public final class RemoteEvictorFactoryI extends _RemoteEvictorFactoryDisp
{
    RemoteEvictorFactoryI(String envName)
    {
        _envName = envName;
    }

    public RemoteEvictorPrx
    createEvictor(String name, boolean transactional, Ice.Current current)
    {
        RemoteEvictorI remoteEvictor =
            new RemoteEvictorI(current.adapter.getCommunicator(), _envName, name, transactional);
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
