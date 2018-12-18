// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class ProxyFlushBatch extends ProxyOutgoingAsyncBaseI<Void>
{
    public ProxyFlushBatch(com.zeroc.Ice._ObjectPrxI prx)
    {
        super(prx, "ice_flushBatchRequests");
        _observer = ObserverHelper.get(prx, "ice_flushBatchRequests");
        BatchRequestQueue.SwapResult r = prx._getBatchRequestQueue().swap(_os);
        _batchRequestNum = r != null ? r.batchRequestNum : 0;
    }

    @Override
    public boolean completed(com.zeroc.Ice.InputStream is)
    {
        assert(false);
        return false;
    }

    @Override
    protected void markCompleted()
    {
        complete(null);
    }

    @Override
    public int invokeRemote(com.zeroc.Ice.ConnectionI connection, boolean compress, boolean response)
        throws RetryException
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
