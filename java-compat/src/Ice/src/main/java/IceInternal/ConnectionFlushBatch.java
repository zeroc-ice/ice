// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.util.concurrent.Callable;

public class ConnectionFlushBatch extends OutgoingAsyncBase
{
    public static ConnectionFlushBatch check(Ice.AsyncResult r, Ice.Connection con, String operation)
    {
        check(r, operation);
        if(!(r instanceof ConnectionFlushBatch))
        {
            throw new IllegalArgumentException("Incorrect AsyncResult object for end_" + operation + " method");
        }
        if(r.getConnection() != con)
        {
            throw new IllegalArgumentException("Connection for call to end_" + operation +
                                               " does not match connection that was used to call corresponding " +
                                               "begin_" + operation + " method");
        }
        return (ConnectionFlushBatch)r;
    }

    public ConnectionFlushBatch(Ice.ConnectionI con, Ice.Communicator communicator, Instance instance,
                                String operation, CallbackBase callback)
    {
        super(communicator, instance, operation, callback);
        _connection = con;
    }

    @Override
    public Ice.Connection getConnection()
    {
        return _connection;
    }

    public void invoke()
    {
        try
        {
            final int batchRequestNum = _connection.getBatchRequestQueue().swap(_os);

            int status;
            if(batchRequestNum == 0)
            {
                status = IceInternal.AsyncStatus.Sent;
                if(sent())
                {
                    status |= IceInternal.AsyncStatus.InvokeSentCallback;
                }
            }
            else if(_instance.queueRequests())
            {
                status = _instance.getQueueExecutor().execute(new Callable<Integer>()
                {
                    @Override
                    public Integer call() throws RetryException
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
        catch(Ice.Exception ex)
        {
            if(completed(ex))
            {
                invokeCompletedAsync();
            }
        }
    }

    private Ice.ConnectionI _connection;
}
