// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import java.util.concurrent.Callable;

public class ConnectionFlushBatch extends OutgoingAsyncBaseI<Void>
{
    public ConnectionFlushBatch(com.zeroc.Ice.ConnectionI con, com.zeroc.Ice.Communicator communicator,
                                Instance instance)
    {
        super(communicator, instance, "flushBatchRequests");
        _connection = con;
    }

    @Override
    public com.zeroc.Ice.Connection getConnection()
    {
        return _connection;
    }

    @Override
    protected void markSent()
    {
        super.markSent();

        assert((_state & StateOK) != 0);
        complete(null);
    }

    @Override
    protected void markCompleted()
    {
        if(_exception != null)
        {
            completeExceptionally(_exception);
        }
        super.markCompleted();
    }

    public void invoke()
    {
        try
        {
            final int batchRequestNum = _connection.getBatchRequestQueue().swap(_os);

            int status;
            if(batchRequestNum == 0)
            {
                status = AsyncStatus.Sent;
                if(sent())
                {
                    status |= AsyncStatus.InvokeSentCallback;
                }
            }
            else if(_instance.queueRequests())
            {
                status = _instance.getQueueExecutor().execute(new Callable<Integer>()
                {
                    @Override
                    public Integer call()
                        throws RetryException
                    {
                        return _connection.sendAsyncRequest(ConnectionFlushBatch.this, false, false, batchRequestNum);
                    }
                });
            }
            else
            {
                status = _connection.sendAsyncRequest(this, false, false, batchRequestNum);
            }

            if((status & AsyncStatus.Sent) > 0)
            {
                _sentSynchronously = true;
                if((status & AsyncStatus.InvokeSentCallback) > 0)
                {
                    invokeSent();
                }
            }
        }
        catch(RetryException ex)
        {
            if(completed(ex.get()))
            {
                invokeCompletedAsync();
            }
        }
        catch(com.zeroc.Ice.Exception ex)
        {
            if(completed(ex))
            {
                invokeCompletedAsync();
            }
        }
    }

    private com.zeroc.Ice.ConnectionI _connection;
}
