// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import Ice.ConnectionI;

public class ConnectRequestHandler
    implements RequestHandler, Reference.GetConnectionCallback, RouterInfo.AddProxyCallback
{
    static private class Request
    {
        Request(BasicStream os)
        {
            this.os = new BasicStream(os.instance(), Protocol.currentProtocolEncoding);
            this.os.swap(os);
        }

        Request(OutgoingAsyncBase out)
        {
            this.outAsync = out;
        }

        OutgoingAsyncBase outAsync = null;
        BasicStream os = null;
    }

    @Override
    public RequestHandler 
    connect(Ice.ObjectPrxHelperBase proxy)
    {
        //
        // Initiate the connection if connect() is called by the proxy that
        // created the handler.
        //
        if(proxy == _proxy && _connect)
        {
            _connect = false; // Call getConnection only once
            _reference.getConnection(this);
        }

        try
        {
            synchronized(this)
            {
                if(!initialized())
                {
                    _proxies.add(proxy);
                    return this;
                }
            }
        }
        catch(Ice.LocalException ex)
        {
            throw ex;
        }
        
        if(_connectionRequestHandler != null)
        {
            proxy.__setRequestHandler(this, _connectionRequestHandler);
            return _connectionRequestHandler;
        }
        else
        {
            return this;
        }
    }

    @Override
    public RequestHandler 
    update(RequestHandler previousHandler, RequestHandler newHandler)
    {
        return previousHandler == this ? newHandler : this;
    }

    @Override
    public void
    prepareBatchRequest(BasicStream os)
        throws RetryException
    {
        synchronized(this)
        {
            waitBatchRequestInProgress();

            if(!initialized())
            {
                _batchRequestInProgress = true;
                _batchStream.swap(os);
                return;
            }
        }

        _connection.prepareBatchRequest(os);
    }

    @Override
    public void
    finishBatchRequest(BasicStream os)
    {
        synchronized(this)
        {
            if(!initialized()) // This can't throw until _batchRequestInProgress = false
            {
                assert(_batchRequestInProgress);
                _batchRequestInProgress = false;
                notifyAll();

                _batchStream.swap(os);

                _requests.add(new Request(_batchStream));
                return;
            }
        }
        _connection.finishBatchRequest(os, _compress);
    }

    @Override
    public void
    abortBatchRequest()
    {
        synchronized(this)
        {
            if(!initialized()) // This can't throw until _batchRequestInProgress = false
            {
                assert(_batchRequestInProgress);
                _batchRequestInProgress = false;
                notifyAll();

                BasicStream dummy = new BasicStream(_reference.getInstance(), Protocol.currentProtocolEncoding);
                _batchStream.swap(dummy);

                return;
            }
        }
        _connection.abortBatchRequest();
    }

    @Override
    public int
    sendAsyncRequest(OutgoingAsyncBase out)
        throws RetryException
    {
        synchronized(this)
        {
            if(!_initialized)
            {
                out.cancelable(this); // This will throw if the request is canceled
            }

            try
            {
                if(!initialized())
                {
                    _requests.add(new Request(out));
                    return AsyncStatus.Queued;
                }
            }
            catch(Ice.LocalException ex)
            {
                throw new RetryException(ex);
            }
        }
        return out.send(_connection, _compress, _response);
    }

    @Override
    public void
    asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
    {
        synchronized(this)
        {
            if(_exception != null)
            {
                return; // The request has been notified of a failure already.
            }

            if(!initialized())
            {
                java.util.Iterator<Request> it = _requests.iterator();
                while(it.hasNext())
                {
                    Request request = it.next();
                    if(request.outAsync == outAsync)
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
    synchronized public ConnectionI
    getConnection()
    {
        if(_exception != null)
        {
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }
        else
        {
            return _connection;
        }
    }

    @Override
    synchronized public
    ConnectionI waitForConnection()
        throws InterruptedException, RetryException
    {
        if(_exception != null)
        {
            throw new RetryException(_exception);
        }

        //
        // Wait for the connection establishment to complete or fail.
        //
        while(!_initialized && _exception == null)
        {
            wait();
        }
        return getConnection();
    }

    //
    // Implementation of Reference.GetConnectionCallback
    //

    @Override
    public void
    setConnection(Ice.ConnectionI connection, boolean compress)
    {
        synchronized(this)
        {
            assert(_exception == null && _connection == null);
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
    public synchronized void
    setException(final Ice.LocalException ex)
    {
        assert(!_initialized && _exception == null);
        _exception = ex;
        _proxies.clear();
        _proxy = null; // Break cyclic reference count.

        //
        // NOTE: remove the request handler *before* notifying the
        // requests that the connection failed. It's important to ensure
        // that future invocations will obtain a new connect request
        // handler once invocations are notified.
        //
        try
        {
            _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);
        }
        catch(Ice.CommunicatorDestroyedException exc)
        {
            // Ignore
        }

        for(Request request : _requests)
        {
            if(request.outAsync != null)
            {
                if(request.outAsync.completed(_exception))
                {
                    request.outAsync.invokeCompletedAsync();
                }
            }
        }
        _requests.clear();
        notifyAll();
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
    ConnectRequestHandler(Reference ref, Ice.ObjectPrxHelperBase proxy)
    {
        _reference = ref;
        _connect = true;
        _response = _reference.getMode() == Reference.ModeTwoway;
        _proxy = (Ice.ObjectPrxHelperBase)proxy;
        _initialized = false;
        _flushing = false;
        _batchRequestInProgress = false;
        _batchStream = new BasicStream(ref.getInstance(), Protocol.currentProtocolEncoding);
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
            while(_flushing && _exception == null)
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
                throw (Ice.LocalException)_exception.fillInStackTrace();
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
        synchronized(this)
        {
            assert(_connection != null && !_initialized);
            waitBatchRequestInProgress();

            //
            // We set the _flushing flag to true to prevent any additional queuing. Callers
            // might block for a little while as the queued requests are being sent but this
            // shouldn't be an issue as the request sends are non-blocking.
            //
            _flushing = true;
        }

        final java.util.List<OutgoingAsyncBase> sentCallbacks = new java.util.ArrayList<OutgoingAsyncBase>();
        java.util.Iterator<Request> p = _requests.iterator(); // _requests is immutable when _flushing = true
        Ice.LocalException exception = null;
        while(p.hasNext())
        {
            Request request = p.next();
            try
            {
                if(request.os != null)
                {
                    BasicStream os = new BasicStream(request.os.instance(), Protocol.currentProtocolEncoding);
                    _connection.prepareBatchRequest(os);
                    try
                    {
                        request.os.pos(0);
                        os.writeBlob(request.os.readBlob(request.os.size()));
                    }
                    catch(Ice.LocalException ex)
                    {
                        _connection.abortBatchRequest();
                        throw ex;
                    }
                    _connection.finishBatchRequest(os, _compress);
                }
                else if((request.outAsync.send(_connection, _compress, _response) & AsyncStatus.InvokeSentCallback) > 0)
                {
                    request.outAsync.invokeSentAsync();
                }
            }
            catch(RetryException ex)
            {
                exception = ex.get();
                try
                {
                    // Remove the request handler before retrying.
                    _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);
                }
                catch(Ice.CommunicatorDestroyedException exc)
                {
                    // Ignore
                }
                if(request.outAsync != null)
                {
                    request.outAsync.retryException(ex.get());
                }
            }
            catch(Ice.LocalException ex)
            {
                exception = ex;
                if(request.outAsync != null && request.outAsync.completed(ex))
                {
                    request.outAsync.invokeCompletedAsync();
                }
            }
            p.remove();
        }

        //
        // If we aren't caching the connection, don't bother creating a
        // connection request handler. Otherwise, update the proxies
        // request handler to use the more efficient connection request
        // handler.
        //
        if(_reference.getCacheConnection() && exception == null)
        {
            _connectionRequestHandler = new ConnectionRequestHandler(_reference, _connection, _compress);
            for(Ice.ObjectPrxHelperBase proxy : _proxies)
            {
                proxy.__setRequestHandler(this, _connectionRequestHandler);
            }
        }

        synchronized(this)
        {
            assert(!_initialized);
            _exception = exception;
            _initialized = _exception == null;
            _flushing = false;
            try
            {
                //
                // Only remove once all the requests are flushed to
                // guarantee serialization.
                //
                _reference.getInstance().requestHandlerFactory().removeRequestHandler(_reference, this);
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
                // Ignore
            }
            _proxies.clear();
            _proxy = null; // Break cyclic reference count.
            notifyAll();
        }
    }

    private void
    waitBatchRequestInProgress()
    {
        //
        // This is similar to a mutex lock in that the stream is
        // only "locked" while the request is in progress.
        //
        boolean interrupted = false;
        while(_batchRequestInProgress)
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
        // Restore the interrupted flag if we were interrupted.
        //
        if(interrupted)
        {
            Thread.currentThread().interrupt();
        }
    }

    private final Reference _reference;
    private boolean _connect;
    private boolean _response;
    
    private Ice.ObjectPrxHelperBase _proxy;
    private java.util.Set<Ice.ObjectPrxHelperBase> _proxies = new java.util.HashSet<Ice.ObjectPrxHelperBase>();

    private Ice.ConnectionI _connection;
    private boolean _compress;
    private Ice.LocalException _exception;
    private boolean _initialized;
    private boolean _flushing;

    private java.util.List<Request> _requests = new java.util.LinkedList<Request>();
    private boolean _batchRequestInProgress;
    private BasicStream _batchStream;
    
    private RequestHandler _connectionRequestHandler;
}
