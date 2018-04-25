// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ProxyFlushBatch extends ProxyOutgoingAsyncBase
{
    public static ProxyFlushBatch check(Ice.AsyncResult r, Ice.ObjectPrx prx, String operation)
    {
        ProxyOutgoingAsyncBase.checkImpl(r, prx, operation);
        try
        {
            return (ProxyFlushBatch)r;
        }
        catch(ClassCastException ex)
        {
            throw new IllegalArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
        }
    }

    public ProxyFlushBatch(Ice.ObjectPrxHelperBase prx, String operation, CallbackBase callback)
    {
        super(prx, operation, callback);
        _observer = ObserverHelper.get(prx, operation);
        _batchRequestNum = prx._getBatchRequestQueue().swap(_os, null);
    }

    @Override
    public int invokeRemote(Ice.ConnectionI connection, boolean compress, boolean response) throws RetryException
    {
        if(_batchRequestNum == 0)
        {
            return sent() ? AsyncStatus.Sent | AsyncStatus.InvokeSentCallback : AsyncStatus.Sent;
        }
        _cachedConnection = connection;
        return connection.sendAsyncRequest(this, compress, false, _batchRequestNum);
    }

    @Override
    public int invokeCollocated(CollocatedRequestHandler handler)
    {
        if(_batchRequestNum == 0)
        {
            return sent() ? AsyncStatus.Sent | AsyncStatus.InvokeSentCallback : AsyncStatus.Sent;
        }
        return handler.invokeAsyncRequest(this, _batchRequestNum, false);
    }

    public void invoke()
    {
        Protocol.checkSupportedProtocol(Protocol.getCompatibleProtocol(_proxy._getReference().getProtocol()));
        invokeImpl(true); // userThread = true
    }

    protected int _batchRequestNum;
}
