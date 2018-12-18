// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ProxyGetConnection extends ProxyOutgoingAsyncBase
{
    public static ProxyGetConnection check(Ice.AsyncResult r, Ice.ObjectPrx prx, String operation)
    {
        ProxyOutgoingAsyncBase.checkImpl(r, prx, operation);
        try
        {
            return (ProxyGetConnection)r;
        }
        catch(ClassCastException ex)
        {
            throw new IllegalArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
        }
    }

    public ProxyGetConnection(Ice.ObjectPrxHelperBase prx, String operation, CallbackBase cb)
    {
        super(prx, operation, cb);
        _observer = ObserverHelper.get(prx, operation);
    }

    @Override
    public int invokeRemote(Ice.ConnectionI connection, boolean compress, boolean response)
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

    @Override
    public Ice.Connection getConnection()
    {
        return _cachedConnection;
    }

    public void invoke()
    {
        invokeImpl(true); // userThread = true
    }
}
