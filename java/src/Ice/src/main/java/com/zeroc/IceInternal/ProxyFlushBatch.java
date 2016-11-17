// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class ProxyFlushBatch extends ProxyOutgoingAsyncBaseI<Void>
{
    public ProxyFlushBatch(com.zeroc.Ice._ObjectPrxI prx)
    {
        super(prx, "ice_flushBatchRequests");
        _observer = ObserverHelper.get(prx, "ice_flushBatchRequests");
        _batchRequestNum = prx._getBatchRequestQueue().swap(_os);
    }

    @Override
    public boolean completed(com.zeroc.Ice.InputStream is)
    {
        assert(false);
        return false;
    }

    @Override
    protected synchronized void markSent()
    {
        super.markSent();

        assert((_state & StateOK) != 0);
        complete(null);
    }

    @Override
    protected boolean needCallback()
    {
        return true;
    }

    @Override
    protected void markCompleted()
    {
        super.markCompleted();
        if(_exception != null)
        {
            completeExceptionally(_exception);
        }
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

    public void waitForResponse()
    {
        if(Thread.interrupted())
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }

        try
        {
            get();
        }
        catch(InterruptedException ex)
        {
            throw new com.zeroc.Ice.OperationInterruptedException();
        }
        catch(java.util.concurrent.ExecutionException ee)
        {
            try
            {
                throw ee.getCause();
            }
            catch(RuntimeException ex) // Includes LocalException
            {
                throw ex;
            }
            catch(Throwable ex)
            {
                throw new com.zeroc.Ice.UnknownException(ex);
            }
        }
    }

    protected int _batchRequestNum;
}
