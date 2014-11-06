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
    connect(final Ice.ObjectPrxHelperBase proxy)
    {
        _executor.executeNoThrow(new Callable<Void>()
        {
            @Override
            public Void call()
            {
                _delegate.connect(proxy);
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
        _executor.execute(new Callable<Void>()
        {
            @Override
            public Void call() throws RetryException
            {
                _delegate.prepareBatchRequest(out);
                return null;
            }
        });
    }

    @Override
    public void
    finishBatchRequest(final BasicStream out)
    {
        _executor.executeNoThrow(new Callable<Void>()
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
        _executor.executeNoThrow(new Callable<Void>()
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
        return _executor.execute(new Callable<Integer>()
        {
            @Override
            public Integer call() throws RetryException
            {
                return _delegate.sendAsyncRequest(out);
            }
        });
    }

    @Override
    public void
    asyncRequestCanceled(final OutgoingAsyncBase outAsync, final Ice.LocalException ex)
    {
        _executor.executeNoThrow(new Callable<Void>()
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
    
    private final RequestHandler _delegate;
    private final QueueExecutorService _executor;
}
