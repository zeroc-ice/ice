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

        Request(OutgoingAsyncMessageCallback out)
        {
            this.outAsync = out;
        }

        OutgoingAsyncMessageCallback outAsync = null;
        BasicStream os = null;
    }

    public RequestHandler
    connect()
    {
        _reference.getConnection(this);

        synchronized(this)
        {
            if(initialized())
            {
                assert(_connection != null);
                return new ConnectionRequestHandler(_reference, _connection, _compress);
            }
            else
            {
                _updateRequestHandler = true; // The proxy request handler will be updated when the connection is set.
                return this;
            }
        }
    }

    @Override
    public void
    prepareBatchRequest(BasicStream os)
        throws RetryException
    {
        synchronized(this)
        {
            waitBatchRequestInProgress();
            try
            {
                if(!initialized())
                {
                    _batchRequestInProgress = true;
                    _batchStream.swap(os);
                    return;
                }
            }
            catch(Ice.LocalException ex)
            {
                throw new RetryException(ex);
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

                if(!_batchAutoFlush &&
                   _batchStream.size() + _batchRequestsSize > _reference.getInstance().messageSizeMax())
                {
                    Ex.throwMemoryLimitException(_batchStream.size() + _batchRequestsSize,
                                                 _reference.getInstance().messageSizeMax());
                }

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

                BasicStream dummy = new BasicStream(_reference.getInstance(), Protocol.currentProtocolEncoding,
                                                    _batchAutoFlush);
                _batchStream.swap(dummy);
                _batchRequestsSize = Protocol.requestBatchHdr.length;

                return;
            }
        }
        _connection.abortBatchRequest();
    }

    @Override
    public int
    sendAsyncRequest(OutgoingAsyncMessageCallback out)
        throws RetryException
    {
        synchronized(this)
        {
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
    public boolean
    asyncRequestCanceled(OutgoingAsyncMessageCallback outAsync, Ice.LocalException ex)
    {
        synchronized(this)
        {
            if(_exception != null)
            {
                return false; // The request has been notified of a failure already.
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
                        outAsync.dispatchInvocationCancel(ex, _reference.getInstance().clientThreadPool(), null);
                        return true; // We're done
                    }
                }
                assert(false); // The request has to be queued if it timed out and we're not initialized yet.
            }
        }
        return _connection.asyncRequestCanceled(outAsync, ex);
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
            assert(_updateRequestHandler || _requests.isEmpty());

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
        assert(_updateRequestHandler || _requests.isEmpty());

        _exception = ex;
        _proxy = null; // Break cyclic reference count.

        //
        // If some requests were queued, we notify them of the failure. This is done from a thread
        // from the client thread pool since this will result in ice_exception callbacks to be
        // called.
        //
        if(!_requests.isEmpty())
        {
            _reference.getInstance().clientThreadPool().dispatch(new DispatchWorkItem(_connection)
                {
                    @Override
                    public void
                    run()
                    {
                        flushRequestsWithException();
                    };
                });
        }

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
    ConnectRequestHandler(Reference ref, Ice.ObjectPrx proxy)
    {
        _reference = ref;
        _response = _reference.getMode() == Reference.ModeTwoway;
        _proxy = (Ice.ObjectPrxHelperBase)proxy;
        _batchAutoFlush = ref.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
            "Ice.BatchAutoFlush", 1) > 0 ? true : false;
        _initialized = false;
        _flushing = false;
        _batchRequestInProgress = false;
        _batchRequestsSize = Protocol.requestBatchHdr.length;
        _batchStream = new BasicStream(ref.getInstance(), Protocol.currentProtocolEncoding, _batchAutoFlush);
        _updateRequestHandler = false;
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

        final java.util.List<OutgoingAsyncMessageCallback> sentCallbacks =
            new java.util.ArrayList<OutgoingAsyncMessageCallback>();
        try
        {
            java.util.Iterator<Request> p = _requests.iterator(); // _requests is immutable when _flushing = true
            while(p.hasNext())
            {
                Request request = p.next();
                if(request.outAsync != null)
                {
                    if((request.outAsync.send(_connection, _compress, _response) &
                        AsyncStatus.InvokeSentCallback) > 0)
                    {
                        sentCallbacks.add(request.outAsync);
                    }
                }
                else
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
                p.remove();
            }
        }
        catch(final RetryException ex)
        {
            //
            // If the connection dies shortly after connection
            // establishment, we don't systematically retry on
            // RetryException. We handle the exception like it
            // was an exception that occured while sending the
            // request.
            //
            synchronized(this)
            {
                assert(_exception == null && !_requests.isEmpty());
                _exception = ex.get();
                _reference.getInstance().clientThreadPool().dispatch(new DispatchWorkItem(_connection)
                {
                    @Override
                    public void run()
                    {
                        flushRequestsWithException();
                    };
                });
            }
        }
        catch(final Ice.LocalException ex)
        {
            synchronized(this)
            {
                assert(_exception == null && !_requests.isEmpty());
                _exception = ex;
                _reference.getInstance().clientThreadPool().dispatch(new DispatchWorkItem(_connection)
                {
                    @Override
                    public void run()
                    {
                        flushRequestsWithException();
                    };
                });
            }
        }

        if(!sentCallbacks.isEmpty())
        {
            _reference.getInstance().clientThreadPool().dispatch(new DispatchWorkItem(_connection)
            {
                @Override
                public void run()
                {
                    for(OutgoingAsyncMessageCallback callback : sentCallbacks)
                    {
                        callback.invokeSent();
                    }
                };
            });
        }

        //
        // We've finished sending the queued requests and the request handler now send
        // the requests over the connection directly. It's time to substitute the
        // request handler of the proxy with the more efficient connection request
        // handler which does not have any synchronization. This also breaks the cyclic
        // reference count with the proxy.
        //
        // NOTE: _updateRequestHandler is immutable once _flushing = true
        //
        if(_updateRequestHandler && _exception == null)
        {
            _proxy.__setRequestHandler(this, new ConnectionRequestHandler(_reference, _connection, _compress));
        }

        synchronized(this)
        {
            assert(!_initialized);
            if(_exception == null)
            {
                _initialized = true;
                _flushing = false;
            }
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

    private void
    flushRequestsWithException()
    {
        for(Request request : _requests)
        {
            if(request.outAsync != null)
            {
                request.outAsync.finished(_exception);
            }
        }
        _requests.clear();
    }

    private final Reference _reference;
    private boolean _response;

    private Ice.ObjectPrxHelperBase _proxy;

    private final boolean _batchAutoFlush;

    private Ice.ConnectionI _connection;
    private boolean _compress;
    private Ice.LocalException _exception;
    private boolean _initialized;
    private boolean _flushing;

    private java.util.List<Request> _requests = new java.util.LinkedList<Request>();
    private boolean _batchRequestInProgress;
    private int _batchRequestsSize;
    private BasicStream _batchStream;
    private boolean _updateRequestHandler;
}
