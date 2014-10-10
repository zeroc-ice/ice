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
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionException;

import Ice.CommunicatorDestroyedException;
import Ice.ConnectionI;

public class QueueRequestHandler implements RequestHandler
{
    public
    QueueRequestHandler(Instance instance, RequestHandler delegate)
    {
        _executor = instance.getQueueExecutor();
        assert(delegate != null);
        _delegate = delegate;
    }

    @Override
    public RequestHandler 
    connect()
    {
        performCallable(new Callable<Void>()
        {
            @Override
            public Void call()
            {
                _delegate.connect();
                return null;
            }
        });
        return this;
    }
    
    @Override
    public RequestHandler 
    update(RequestHandler previousHandler, RequestHandler newHandler)
    {
        //
        // Only update to new handler if the previous handler matches this one.
        //
        if(previousHandler == this)
        {
            if(newHandler != null)
            {
                return new QueueRequestHandler(_delegate.getReference().getInstance(), newHandler);
            }
            else
            {
                return null;
            }
        }
        return this;
    }
    
    @Override
    public void
    prepareBatchRequest(final BasicStream out) throws RetryException
    {
        try
        {
            performCallable(new Callable<Void>()
            {
                @Override
                public Void call() throws RetryException
                {
                    _delegate.prepareBatchRequest(out);
                    return null;
                }
            });
        }
        catch(RuntimeException ex)
        {
            if(ex.getCause() instanceof RetryException)
            {
                throw (RetryException)ex.getCause();
            }
            throw ex;
        }

    }

    @Override
    public void
    finishBatchRequest(final BasicStream out)
    {
        performCallable(new Callable<Void>()
        {
            @Override
            public Void call() throws RetryException
            {
                _delegate.finishBatchRequest(out);
                return null;
            }
        });
    }

    @Override
    public void
    abortBatchRequest()
    {
        performCallable(new Callable<Void>()
        {
            @Override
            public Void call()
            {
                _delegate.abortBatchRequest();
                return null;
            }
        });
    }

    @Override
    public int
    sendAsyncRequest(final OutgoingAsyncBase out) throws RetryException
    {
        try
        {
            return performCallable(new Callable<Integer>()
            {
                @Override
                public Integer call() throws RetryException
                {
                    return _delegate.sendAsyncRequest(out);
                }
            });
        }
        catch(RuntimeException ex)
        {
            if(ex.getCause() instanceof RetryException)
            {
                throw (RetryException)ex.getCause();
            }
            throw ex;
        }
    }

    @Override
    public void
    asyncRequestCanceled(final OutgoingAsyncBase outAsync, final Ice.LocalException ex)
    {
        performCallable(new Callable<Void>()
        {
            @Override
            public Void call()
            {
                _delegate.asyncRequestCanceled(outAsync, ex);
                return null;
            }
        });
    }

    @Override
    public Reference
    getReference()
    {
        return _delegate.getReference();
    }

    @Override
    public ConnectionI
    getConnection()
    {
        return _delegate.getConnection();
    }

    @Override
    public ConnectionI
    waitForConnection()
        throws InterruptedException, RetryException
    {
        return _delegate.waitForConnection();
    }
    
    private <T> T performCallable(Callable<T> callable) {
        try
        {
            Future<T> future = _executor.submit(callable);
            boolean interrupted = false;
            while(true)
            {
                try 
                {
                    T value = future.get();
                    if(interrupted)
                    {
                        Thread.currentThread().interrupt();
                    }
                    return value;
                }
                catch(InterruptedException ex)
                {
                    interrupted = true;
                }
            }
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
                throw new RuntimeException(ex);
            }
        }
    }
    
    private final RequestHandler _delegate;
    private final ExecutorService _executor;
}
