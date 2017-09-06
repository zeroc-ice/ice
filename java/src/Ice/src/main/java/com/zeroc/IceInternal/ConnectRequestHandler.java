// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import java.util.concurrent.Callable;

public class ConnectRequestHandler
    implements RequestHandler, Reference.GetConnectionCallback, RouterInfo.AddProxyCallback
{
    synchronized public RequestHandler
    connect(com.zeroc.Ice._ObjectPrxI proxy)
    {
        if(!initialized())
        {
            _proxies.add(proxy);
        }
        return _requestHandler;
    }

    @Override
    public RequestHandler
    update(RequestHandler previousHandler, RequestHandler newHandler)
    {
        return previousHandler == this ? newHandler : this;
    }

    @Override
    public int
    sendAsyncRequest(ProxyOutgoingAsyncBase out)
        throws RetryException
    {
        synchronized(this)
        {
            if(!_initialized)
            {
                out.cancelable(this); // This will throw if the request is canceled
            }

            if(!initialized())
            {
                _requests.add(out);
                return AsyncStatus.Queued;
            }
        }
        return out.invokeRemote(_connection, _compress, _response);
    }

    @Override
    public void
    asyncRequestCanceled(OutgoingAsyncBase outAsync, com.zeroc.Ice.LocalException ex)
    {
        synchronized(this)
        {
            if(_exception != null)
            {
                return; // The request has been notified of a failure already.
            }

            if(!initialized())
            {
                java.util.Iterator<ProxyOutgoingAsyncBase> it = _requests.iterator();
                while(it.hasNext())
                {
                    OutgoingAsyncBase request = it.next();
                    if(request == outAsync)
                    {
                        it.remove();
                        if(outAsync.completed(ex))
                        {
                            outAsync.invokeCompletedAsync();
                        }
                        return;
                    }
                }
                assert(false); // The request has to be queued if it timed out and we're not initialized yet.
            }
        }
        _connection.asyncRequestCanceled(outAsync, ex);
    }

    @Override
    public Reference
    getReference()
    {
        return _reference;
    }

    @Override
    synchronized public com.zeroc.Ice.ConnectionI
    getConnection()
    {
        //
        // First check for the connection, it's important otherwise the user could first get a connection
        // and then the exception if he tries to obtain the proxy cached connection mutiple times (the
        // exception can be set after the connection is set if the flush of pending requests fails).
        //
        if(_connection != null)
        {
            return _connection;
        }
        else if(_exception != null)
        {
            throw (com.zeroc.Ice.LocalException)_exception.fillInStackTrace();
        }
        return null;
    }

    //
    // Implementation of Reference.GetConnectionCallback
    //

    @Override
    public void
    setConnection(com.zeroc.Ice.ConnectionI connection, boolean compress)
    {
        synchronized(this)
        {
            assert(!_flushing && _exception == null && _connection == null);
            _connection = connection;
            _compress = compress;
        }

        //
        // If this proxy is for a non-local object, and we are using a router, then
        // add this proxy to the router info object.
        //
        RouterInfo ri = _reference.getRouterInfo();
        if(ri != null && !ri.addProxy(_proxy, this))
        {
            return; // The request handler will be initialized once addProxy returns.
        }

        //
        // We can now send the queued requests.
        //
        flushRequests();
    }

    @Override
    public void
    setException(final com.zeroc.Ice.LocalException ex)
    {
        synchronized(this)
        {
            assert(!_flushing && !_initialized && _exception == null);
            _exception = ex;
            _flushing = true; // Ensures request handler is removed before processing new requests.
        }

        //
        // NOTE: remove the request handler *before* notifying the requests that the connection
        // failed. It's important to ensure that future invocations will obtain a new connect
        // request handler once invocations are notified.
        //
        try
        {
            _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);
        }
        catch(com.zeroc.Ice.CommunicatorDestroyedException exc)
        {
            // Ignore
        }

        for(OutgoingAsyncBase outAsync : _requests)
        {
            if(outAsync.completed(_exception))
            {
                outAsync.invokeCompletedAsync();
            }
        }
        _requests.clear();

        synchronized(this)
        {
            _flushing = false;
            _proxies.clear();
            _proxy = null; // Break cyclic reference count.
            notifyAll();
        }
     }

    //
    // Implementation of RouterInfo.AddProxyCallback
    //
    @Override
    public void
    addedProxy()
    {
        //
        // The proxy was added to the router info, we're now ready to send the
        // queued requests.
        //
        flushRequests();
    }

    public
    ConnectRequestHandler(Reference ref, com.zeroc.Ice._ObjectPrxI proxy)
    {
        _reference = ref;
        _response = _reference.getMode() == Reference.ModeTwoway;
        _proxy = proxy;
        _initialized = false;
        _flushing = false;

        if(_reference.getInstance().queueRequests())
        {
            _requestHandler = new QueueRequestHandler(_reference.getInstance(), this);
        }
        else
        {
            _requestHandler = this;
        }
    }

    private boolean
    initialized()
    {
        // Must be called with the mutex locked.

        if(_initialized)
        {
            assert(_connection != null);
            return true;
        }
        else
        {
            //
            // This is similar to a mutex lock in that the flag is
            // only true for a short period of time.
            //
            boolean interrupted = false;
            while(_flushing)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                    interrupted = true;
                }
            }
            //
            // Restore the interrupted status.
            //
            if(interrupted)
            {
                Thread.currentThread().interrupt();
            }

            if(_exception != null)
            {
                if(_connection != null)
                {
                    //
                    // Only throw if the connection didn't get established. If
                    // it died after being established, we allow the caller to
                    // retry the connection establishment by not throwing here
                    // (the connection will throw RetryException).
                    //
                    return true;
                }
                throw (com.zeroc.Ice.LocalException)_exception.fillInStackTrace();
            }
            else
            {
                return _initialized;
            }
        }
    }

    private void
    flushRequests()
    {
        if(_reference.getInstance().queueRequests())
        {
            _reference.getInstance().getQueueExecutor().executeNoThrow(new Callable<Void>()
            {
                @Override
                public Void call() throws Exception
                {
                    flushRequestsImpl();
                    return null;
                }
            });
        }
        else
        {
            flushRequestsImpl();
        }
    }

    private void
    flushRequestsImpl()
    {
        synchronized(this)
        {
            assert(_connection != null && !_initialized);

            //
            // We set the _flushing flag to true to prevent any additional queuing. Callers
            // might block for a little while as the queued requests are being sent but this
            // shouldn't be an issue as the request sends are non-blocking.
            //
            _flushing = true;
        }

        com.zeroc.Ice.LocalException exception = null;
        for(ProxyOutgoingAsyncBase outAsync : _requests)
        {
            try
            {
                if((outAsync.invokeRemote(_connection, _compress, _response) & AsyncStatus.InvokeSentCallback) > 0)
                {
                    outAsync.invokeSentAsync();
                }
            }
            catch(RetryException ex)
            {
                exception = ex.get();

                // Remove the request handler before retrying.
                _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);
                outAsync.retryException(ex.get());
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                exception = ex;
                if(outAsync.completed(ex))
                {
                    outAsync.invokeCompletedAsync();
                }
            }
        }
        _requests.clear();

        //
        // If we aren't caching the connection, don't bother creating a
        // connection request handler. Otherwise, update the proxies
        // request handler to use the more efficient connection request
        // handler.
        //
        if(_reference.getCacheConnection() && exception == null)
        {
            RequestHandler previous = _requestHandler;
            _requestHandler = new ConnectionRequestHandler(_reference, _connection, _compress);
            if(_reference.getInstance().queueRequests())
            {
                _requestHandler = new QueueRequestHandler(_reference.getInstance(), _requestHandler);
            }
            for(com.zeroc.Ice._ObjectPrxI proxy : _proxies)
            {
                proxy._updateRequestHandler(previous, _requestHandler);
            }
        }

        synchronized(this)
        {
            assert(!_initialized);
            _exception = exception;
            _initialized = _exception == null;
            _flushing = false;

            //
            // Only remove once all the requests are flushed to
            // guarantee serialization.
            //
            _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);

            _proxies.clear();
            _proxy = null; // Break cyclic reference count.
            notifyAll();
        }
    }

    private final Reference _reference;
    private boolean _response;

    private com.zeroc.Ice._ObjectPrxI _proxy;
    private java.util.Set<com.zeroc.Ice._ObjectPrxI> _proxies = new java.util.HashSet<>();

    private com.zeroc.Ice.ConnectionI _connection;
    private boolean _compress;
    private com.zeroc.Ice.LocalException _exception;
    private boolean _initialized;
    private boolean _flushing;

    private java.util.List<ProxyOutgoingAsyncBase> _requests = new java.util.LinkedList<>();
    private RequestHandler _requestHandler;
}
