// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

    public void invoke(final Ice.CompressBatch compressBatch)
    {
        _observer = ObserverHelper.get(_instance, "flushBatchRequests");
        try
        {
            final Ice.BooleanHolder compress = new Ice.BooleanHolder();
            final int batchRequestNum = _connection.getBatchRequestQueue().swap(_os, compress);
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
                        boolean comp;
                        if(compressBatch == Ice.CompressBatch.Yes)
                        {
                            comp = true;
                        }
                        else if(compressBatch == Ice.CompressBatch.No)
                        {
                            comp = false;
                        }
                        else
                        {
                            comp = compress.value;
                        }
                        return _connection.sendAsyncRequest(ConnectionFlushBatch.this, comp, false, batchRequestNum);
                    }
                });
            }
            else
            {
                boolean comp;
                if(compressBatch == Ice.CompressBatch.Yes)
                {
                    comp = true;
                }
                else if(compressBatch == Ice.CompressBatch.No)
                {
                    comp = false;
                }
                else
                {
                    comp = compress.value;
                }
                status = _connection.sendAsyncRequest(this, comp, false, batchRequestNum);
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
