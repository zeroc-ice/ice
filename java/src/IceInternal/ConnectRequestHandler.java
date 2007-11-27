// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    public void
    prepareBatchRequest(BasicStream os)
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
                    throw new Ice.MemoryLimitException();
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
        return (!getConnection(true).sendRequest(out, _compress, _response) || _response) ? _connection : null;
    }

    public void
    sendAsyncRequest(OutgoingAsync out)
    {
        try
        { 
            synchronized(this)
            {
                if(!initialized())
                {
                    _requests.add(new Request(out));
                    return;
                }
            }

            _connection.sendAsyncRequest(out, _compress, _response);
        }
        catch(LocalExceptionWrapper ex)
        {
            out.__finished(ex);
        }
        catch(Ice.LocalException ex)
        {
            out.__finished(ex);
        }
    }

    public boolean
    flushBatchRequests(BatchOutgoing out)
    {
        return getConnection(true).flushBatchRequests(out);
    }

    public void
    flushAsyncBatchRequests(BatchOutgoingAsync out)
    {
        try
        { 
            synchronized(this)
            {
                if(!initialized())
                {
                    _requests.add(new Request(out));
                    return;
                }
            }

            _connection.flushAsyncBatchRequests(out);
        }
        catch(Ice.LocalException ex)
        {
            out.__finished(ex);
        }
    }

    public Outgoing
    getOutgoing(String operation, Ice.OperationMode mode, java.util.Map context)
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
            _connection = connection;
            _compress = compress;
        }

        //
        // If this proxy is for a non-local object, and we are using a router, then
        // add this proxy to the router info object.
        //
        RouterInfo ri = _reference.getRouterInfo();
        if(ri != null)
        {
            if(!ri.addProxy(_proxy, this))
            {
                return; // The request handler will be initialized once addProxy returns.
            }
        }

        flushRequests();
    }

    public void 
    setException(Ice.LocalException ex)
    {
        synchronized(this)
        {
            _exception = ex;
            _proxy = null; // Break cyclic reference count.
            _delegate = null; // Break cyclic reference count.
            notifyAll();
        }
        
        java.util.Iterator p = _requests.iterator();
        while(p.hasNext())
        {
            Request request = (Request)p.next();
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

    //
    // Implementation of RouterInfo.AddProxyCallback
    //
    public void
    addedProxy()
    {
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

    public RequestHandler
    connect()
    {
        _reference.getConnection(this);

        synchronized(this)
        {
            if(_connection != null)
            {
                return new ConnectionRequestHandler(_reference, _connection, _compress);
            }
            else
            {
                _updateRequestHandler = true;
                return this;
            }
        }
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
            while(_flushing)
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
            assert(_connection != null);
            
            if(_batchRequestInProgress)
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
            
        java.util.Iterator p = _requests.iterator(); // _requests is immutable when _flushing = true
        while(p.hasNext())
        {
            Request request = (Request)p.next();
            if(request.out != null)
            {
                try
                {
                    _connection.sendAsyncRequest(request.out, _compress, _response);
                }
                catch(LocalExceptionWrapper ex)
                {
                    request.out.__finished(ex);
                }
                catch(Ice.LocalException ex)
                {
                    request.out.__finished(ex);
                }
            }
            else if(request.batchOut != null)
            {
                try
                {
                    _connection.flushAsyncBatchRequests(request.batchOut);
                }
                catch(Ice.LocalException ex)
                {
                    request.batchOut.__finished(ex);
                }
            }
            else
            {
                //
                // TODO: Add sendBatchRequest() method to ConnectionI?
                //
                try
                {
                    BasicStream os = new BasicStream(request.os.instance());
                    _connection.prepareBatchRequest(os);
                    request.os.pos(0);
                    os.writeBlob(request.os.readBlob(request.os.size()));
                    _connection.finishBatchRequest(os, _compress);
                }
                catch(Ice.LocalException ex)
                {
                    _connection.abortBatchRequest();
                    _exception = ex;
                }
            }
        }
        _requests.clear();

        synchronized(this)
        {
            _initialized = true;
            _flushing = false;
            notifyAll();
        }

        if(_updateRequestHandler && _exception == null)
        {
            _proxy.__setRequestHandler(_delegate, new ConnectionRequestHandler(_reference, _connection, _compress));
        }
        _proxy = null; // Break cyclic reference count.
        _delegate = null; // Break cyclic reference count.
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

    private java.util.ArrayList _requests = new java.util.ArrayList();
    private boolean _batchRequestInProgress;
    private int _batchRequestsSize;
    private BasicStream _batchStream;
    private boolean _updateRequestHandler;
}
