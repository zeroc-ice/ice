// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ConnectRequestHandler 
    implements RequestHandler, Reference.GetConnectionCallback, RouterInfo.AddProxyCallback
{
    static class Request
    {
        Request(BasicStream os)
        {
            this.os = new BasicStream(os.instance());
            this.os.swap(os);
        }

        Request(OutgoingAsync out)
        {
            this.out = out;
        }

        Request(BatchOutgoingAsync out)
        {
            this.batchOut = out;
        }

        OutgoingAsync out = null;
        BatchOutgoingAsync batchOut = null;
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

    public void
    prepareBatchRequest(BasicStream os)
        throws LocalExceptionWrapper
    {
        synchronized(this)
        {
            while(_batchRequestInProgress)
            {
                try
                {
                    wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            
            if(!initialized())
            {
                _batchStream.swap(os);
                _batchRequestInProgress = true;
                return;
            }
        }
        
        _connection.prepareBatchRequest(os);
    }

    public void
    finishBatchRequest(BasicStream os)
    {
        synchronized(this)
        {
            if(!initialized())
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

    public void
    abortBatchRequest()
    {
        synchronized(this)
        {
            if(!initialized())
            {
                assert(_batchRequestInProgress);
                _batchRequestInProgress = false;
                notifyAll();

                BasicStream dummy = new BasicStream(_reference.getInstance(), _batchAutoFlush);
                _batchStream.swap(dummy);
                _batchRequestsSize = Protocol.requestBatchHdr.length;
                return;
            }
        }

        _connection.abortBatchRequest();
    }

    public Ice.ConnectionI
    sendRequest(Outgoing out)
        throws LocalExceptionWrapper
    {
        if(!getConnection(true).sendRequest(out, _compress, _response) || _response)
        {
            return _connection; // The request has been sent or we're expecting a response.
        }
        else
        {
            return null; // The request hasn't been sent yet.
        }
    }

    public boolean
    sendAsyncRequest(OutgoingAsync out)
        throws LocalExceptionWrapper
    {
        synchronized(this)
        {
            if(!initialized())
            {
                _requests.add(new Request(out));
                return false;
            }
        }
        return _connection.sendAsyncRequest(out, _compress, _response);
    }

    public boolean
    flushBatchRequests(BatchOutgoing out)
    {
        return getConnection(true).flushBatchRequests(out);
    }

    public boolean
    flushAsyncBatchRequests(BatchOutgoingAsync out)
    {
        synchronized(this)
        {
            if(!initialized())
            {
                _requests.add(new Request(out));
                return false;
            }
        }
        return _connection.flushAsyncBatchRequests(out);
    }

    public Outgoing
    getOutgoing(String operation, Ice.OperationMode mode, java.util.Map<String, String> context)
        throws LocalExceptionWrapper
    {
        synchronized(this)
        {
            if(!initialized())
            {
                return new IceInternal.Outgoing(this, operation, mode, context);
            }
        }

        return _connection.getOutgoing(this, operation, mode, context);
    }

    public void
    reclaimOutgoing(Outgoing out)
    {
        synchronized(this)
        {
            if(_connection == null)
            {
                return;
            }
        }

        _connection.reclaimOutgoing(out);
    }

    public Reference 
    getReference()
    {
        return _reference;
    }

    synchronized public Ice.ConnectionI
    getConnection(boolean wait)
    {
        if(wait)
        {
            //
            // Wait for the connection establishment to complete or fail.
            //
            while(!_initialized && _exception == null)
            {
                try
                {
                    wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
        }

        if(_exception != null)
        {
            throw _exception;
        }
        else
        {
            assert(!wait || _initialized);
            return _connection;
        }
    }

    //
    // Implementation of Reference.GetConnectionCallback
    //

    public void
    setConnection(Ice.ConnectionI connection, boolean compress)
    {
        synchronized(this)
        {
            assert(_connection == null && _exception == null);
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

    public synchronized void 
    setException(final Ice.LocalException ex)
    {
        assert(!_initialized && _exception == null);
        assert(_updateRequestHandler || _requests.isEmpty());
        
        _exception = ex;
        _proxy = null; // Break cyclic reference count.
        _delegate = null; // Break cyclic reference count.
        
        //
        // If some requests were queued, we notify them of the failure. This is done from a thread
        // from the client thread pool since this will result in ice_exception callbacks to be 
        // called.
        //
        if(!_requests.isEmpty())
        {
            _reference.getInstance().clientThreadPool().execute(new ThreadPoolWorkItem() 
                {
                    public void
                    execute(ThreadPoolCurrent current)
                    {
                        current.ioCompleted();
                        flushRequestsWithException(ex);
                    };
                });
        }

        notifyAll();
    }

    //
    // Implementation of RouterInfo.AddProxyCallback
    //
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
    ConnectRequestHandler(Reference ref, Ice.ObjectPrx proxy, Ice._ObjectDelM delegate)
    {
        _reference = ref;
        _response = _reference.getMode() == Reference.ModeTwoway;
        _proxy = (Ice.ObjectPrxHelperBase)proxy; 
        _delegate = delegate;
        _batchAutoFlush = ref.getInstance().initializationData().properties.getPropertyAsIntWithDefault(
            "Ice.BatchAutoFlush", 1) > 0 ? true : false;
        _batchStream = new BasicStream(ref.getInstance(), _batchAutoFlush);
        _batchRequestInProgress = false;
        _batchRequestsSize = Protocol.requestBatchHdr.length;
        _updateRequestHandler = false;
    }

    private boolean
    initialized()
    {
        if(_initialized)
        {
            assert(_connection != null);
            return true;
        }
        else
        {
            while(_flushing && _exception == null)
            {
                try
                {
                    wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

	    if(_exception != null)
	    {
		throw _exception;
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
            
            while(_batchRequestInProgress)
            {
                try
                {
                    wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            
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
                if(request.out != null)
                {
                    if(_connection.sendAsyncRequest(request.out, _compress, _response))
                    {
                        if(request.out instanceof Ice.AMISentCallback)
                        {
                            sentCallbacks.add(request.out);
                        }
                    }
                }
                else if(request.batchOut != null)
                {
                    if(_connection.flushAsyncBatchRequests(request.batchOut))
                    {
                        if(request.batchOut instanceof Ice.AMISentCallback)
                        {
                            sentCallbacks.add(request.batchOut);
                        }
                    }
                }
                else
                {
                    BasicStream os = new BasicStream(request.os.instance());
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
        catch(final LocalExceptionWrapper ex)
        {
            synchronized(this)
            {
                assert(_exception == null && !_requests.isEmpty());
                _exception = ex.get();
                _reference.getInstance().clientThreadPool().execute(new ThreadPoolWorkItem() 
                    {
                        public void
                        execute(ThreadPoolCurrent current)
                        {
                            current.ioCompleted();
                            flushRequestsWithException(ex);
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
                _reference.getInstance().clientThreadPool().execute(new ThreadPoolWorkItem() 
                    {
                        public void
                        execute(ThreadPoolCurrent current)
                        {
                            current.ioCompleted();
                            flushRequestsWithException(ex);
                        };
                    });
            }
        }
        
        if(!sentCallbacks.isEmpty())
        {
            final Instance instance = _reference.getInstance();
            instance.clientThreadPool().execute(new ThreadPoolWorkItem() 
                                                {
                                                    public void
                                                    execute(ThreadPoolCurrent current)
                                                    {
                                                        current.ioCompleted();
                                                        for(OutgoingAsyncMessageCallback callback : sentCallbacks)
                                                        {
                                                            callback.__sent(instance);
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
            _proxy.__setRequestHandler(_delegate, new ConnectionRequestHandler(_reference, _connection, _compress));
        }

        synchronized(this)
        {
            if(_exception == null)
            {
                assert(!_initialized);
                _initialized = true;
                _flushing = false;
            }
            _proxy = null; // Break cyclic reference count.
            _delegate = null; // Break cyclic reference count.
            notifyAll();
        }
    }

    void
    flushRequestsWithException(Ice.LocalException ex)
    {
        for(Request request : _requests)
        {
            if(request.out != null)
            {            
                request.out.__finished(ex);
            }
            else if(request.batchOut != null)
            {
                request.batchOut.__finished(ex);
            }
        }
        _requests.clear();
    }

    void
    flushRequestsWithException(LocalExceptionWrapper ex)
    {
        for(Request request : _requests)
        {
            if(request.out != null)
            {            
                request.out.__finished(ex);
            }
            else if(request.batchOut != null)
            {
                request.batchOut.__finished(ex.get());
            }
        }
        _requests.clear();
    }

    private final Reference _reference;
    private final boolean _batchAutoFlush;
    private Ice.ObjectPrxHelperBase _proxy;
    private Ice._ObjectDelM _delegate;
    private boolean _initialized = false;
    private boolean _flushing = false;
    private Ice.ConnectionI _connection = null;
    private boolean _compress = false;
    private boolean _response;
    private Ice.LocalException _exception = null;

    private java.util.List<Request> _requests = new java.util.LinkedList<Request>();
    private boolean _batchRequestInProgress;
    private int _batchRequestsSize;
    private BasicStream _batchStream;
    private boolean _updateRequestHandler;
}
