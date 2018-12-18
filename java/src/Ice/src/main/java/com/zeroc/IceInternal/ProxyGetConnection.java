// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class ProxyGetConnection extends ProxyOutgoingAsyncBaseI<com.zeroc.Ice.Connection>
{
    public ProxyGetConnection(com.zeroc.Ice._ObjectPrxI prx)
    {
        super(prx, "ice_getConnection");
        _observer = ObserverHelper.get(prx, "ice_getConnection");
    }

    @Override
    protected void markCompleted()
    {
        complete(_cachedConnection);
    }

    @Override
    public boolean completed(com.zeroc.Ice.InputStream is)
    {
        assert(false);
        return false;
    }

    @Override
    public int invokeRemote(com.zeroc.Ice.ConnectionI connection, boolean compress, boolean response)
        throws RetryException
    {
        _cachedConnection = connection;
        if(finished(true, true))
        {
            invokeCompletedAsync();
        }
        return AsyncStatus.Sent;
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler)
    {
        if(finished(true, true))
        {
            invokeCompletedAsync();
        }
        return AsyncStatus.Sent;
    }

    public void invoke()
    {
        invokeImpl(true); // userThread = true
    }
}
