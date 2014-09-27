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
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionException;

import Ice.CommunicatorDestroyedException;

public class ConnectionBatchOutgoingAsync extends BatchOutgoingAsync
{
    public ConnectionBatchOutgoingAsync(Ice.ConnectionI con, Ice.Communicator communicator, Instance instance,
                                        String operation, CallbackBase callback)
    {
        super(communicator, instance, operation, callback);
        _connection = con;
    }

    public void __invoke()
    {
        int status;
        if(_instance.queueRequests())
        {
            Future<Integer> future = _instance.getQueueExecutor().submit(new Callable<Integer>()
            {
                @Override
                public Integer call() throws RetryException
                {
                    return _connection.flushAsyncBatchRequests(ConnectionBatchOutgoingAsync.this);
                }
            });

            boolean interrupted = false;
            while(true)
            {
                try 
                {
                    status = future.get();
                    if(interrupted)
                    {
                        Thread.currentThread().interrupt();
                    }
                    break;
                }
                catch(InterruptedException ex)
                {
                    interrupted = true;
                }
                catch(RejectedExecutionException e)
                {
                    throw new CommunicatorDestroyedException();
                }
                catch(ExecutionException e)
                {
                    try
                    {
                        throw e.getCause();
                    }
                    catch(RuntimeException ex)
                    {
                        throw ex;
                    }
                    catch(Throwable ex)
                    {
                        assert(false);
                    }
                }
            }
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

    @Override
    public Ice.Connection getConnection()
    {
        return _connection;
    }
    
    @Override
    protected void cancelRequest()
    {
        _connection.asyncRequestCanceled(this, new Ice.OperationInterruptedException());
    }

    private Ice.ConnectionI _connection;
}
