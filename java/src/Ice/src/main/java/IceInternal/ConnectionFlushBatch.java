// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
            int status;
            if(_instance.queueRequests())
            {
                status = _instance.getQueueExecutor().executeNoThrow(new Callable<Integer>()
                {
                    @Override
                    public Integer call()
                    {
                        return _connection.flushAsyncBatchRequests(ConnectionFlushBatch.this);
                    }
                });
            }
            else
            {
                status = _connection.flushAsyncBatchRequests(this);
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
