// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package IceInternal;

public final class Connection extends EventHandler
{
    public synchronized void
    activate()
    {
	setState(StateActive);
    }

    public synchronized void
    hold()
    {
	setState(StateHolding);
    }

    // DestructionReason
    public final static int ObjectAdapterDeactivated = 0;
    public final static int CommunicatorDestroyed = 1;

    public synchronized void
    destroy(int reason)
    {
	_batchStream.destroy();
	
	switch(reason)
	{
	    case ObjectAdapterDeactivated:
	    {
		setState(StateClosing, new Ice.ObjectAdapterDeactivatedException());
		break;
	    }
	    
	    case CommunicatorDestroyed:
	    {
		setState(StateClosing, new Ice.CommunicatorDestroyedException());
		break;
	    }
	}
    }

    public synchronized boolean
    isDestroyed()
    {
	return _state >= StateClosing;
    }

    public synchronized boolean
    isFinished()
    {
	return _transceiver == null;
    }

    public synchronized void
    waitUntilHolding()
    {
	while(_state < StateHolding || _dispatchCount > 0)
	{
	    try
	    {
		wait();
	    }
	    catch(InterruptedException ex)
	    {
	    }
        }
    }

    public synchronized void
    waitUntilFinished()
    {
	while(_transceiver != null)
	{
	    try
	    {
		wait();
	    }
	    catch(InterruptedException ex)
	    {
	    }
	}
    }

    public synchronized void
    validate()
    {
	if(_endpoint.datagram())
	{
	    //
	    // Datagram connections are always implicitly validated.
	    //
	    return;
	}
	
	try
	{
	    if(_adapter != null)
	    {
		//
		// Incoming connections play the active role with
		// respect to connection validation.
		//
		BasicStream os = new BasicStream(_instance);
		os.writeByte(Protocol.protocolVersion);
		os.writeByte(Protocol.encodingVersion);
		os.writeByte(Protocol.validateConnectionMsg);
		os.writeInt(Protocol.headerSize); // Message size.
		TraceUtil.traceHeader("sending validate connection", os, _logger, _traceLevels);
		_transceiver.write(os, _endpoint.timeout());
	    }
	    else
	    {
		//
		// Outgoing connection play the passive role with
		// respect to connection validation.
		//
		BasicStream is = new BasicStream(_instance);
		is.resize(Protocol.headerSize, true);
		is.pos(0);
		_transceiver.read(is, _endpoint.timeout());
		int pos = is.pos();
		assert(pos >= Protocol.headerSize);
		is.pos(0);
		byte protVer = is.readByte();
		if(protVer != Protocol.protocolVersion)
		{
		    throw new Ice.UnsupportedProtocolException();
		}
		byte encVer = is.readByte();
		if(encVer != Protocol.encodingVersion)
		{
		    throw new Ice.UnsupportedEncodingException();
		}
		byte messageType = is.readByte();
		if(messageType != Protocol.validateConnectionMsg)
		{
		    throw new Ice.ConnectionNotValidatedException();
		}
		int size = is.readInt();
		if(size != Protocol.headerSize)
		{
		    throw new Ice.IllegalMessageSizeException();
		}
		TraceUtil.traceHeader("received validate connection", is, _logger, _traceLevels);
	    }
	}
	catch(Ice.LocalException ex)
	{
	    setState(StateClosed, ex);
	    assert(_exception != null);
	    throw _exception;
	}
    }

    public synchronized void
    incProxyCount()
    {
	assert(_proxyCount >= 0);
	++_proxyCount;
    }

    public synchronized void
    decProxyCount()
    {
	assert(_proxyCount > 0);
	--_proxyCount;
	if(_proxyCount == 0 && _adapter == null)
	{
	    assert(_requests.isEmpty());
	    setState(StateClosing, new Ice.CloseConnectionException());
	}
    }

    private final static byte[] _requestHdr =
    {
        Protocol.protocolVersion,
        Protocol.encodingVersion,
        Protocol.requestMsg,
        (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
        (byte)0, (byte)0, (byte)0, (byte)0  // Request ID (placeholder).
    };

    public void
    prepareRequest(BasicStream os)
    {
        os.writeBlob(_requestHdr);
    }

    public synchronized void
    sendRequest(Outgoing out, boolean oneway)
    {
	if(_exception != null)
	{
	    throw _exception;
	}
	assert(_state < StateClosing);
	
	int requestId = 0;
	
	try
	{
	    BasicStream os = out.os();
	    os.pos(3);
	    
	    //
	    // Fill in the message size and request ID.
	    //
	    os.writeInt(os.size());
	    if(!_endpoint.datagram() && !oneway)
	    {
		requestId = _nextRequestId++;
		if(requestId <= 0)
		{
		    _nextRequestId = 1;
		    requestId = _nextRequestId++;
		}
		os.writeInt(requestId);
	    }
	    
	    //
	    // Send the request.
	    //
	    TraceUtil.traceRequest("sending request", os, _logger, _traceLevels);
	    _transceiver.write(os, _endpoint.timeout());
	}
	catch(Ice.LocalException ex)
	{
	    setState(StateClosed, ex);
	    assert(_exception != null);
	    throw _exception;
	}
	
	//
	// Only add to the request map if there was no exception, and if
	// the operation is not oneway.
	//
	if(!_endpoint.datagram() && !oneway)
	{
	    _requests.put(requestId, out);
	}
    }

    private final static byte[] _requestBatchHdr =
    {
        Protocol.protocolVersion,
        Protocol.encodingVersion,
        Protocol.requestBatchMsg,
        (byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
    };

    public synchronized void
    prepareBatchRequest(BasicStream os)
    {
	while(_batchStreamInUse && _exception == null)
	{
	    try
	    {
		wait();
	    }
	    catch(InterruptedException ex)
	    {
	    }
	}

        if(_exception != null)
        {
            throw _exception;
        }
        assert(_state < StateClosing);

        //
        // The Connection now belongs to the caller, until
        // finishBatchRequest() is called.
        //

        if(_batchStream.size() == 0)
        {
            _batchStream.writeBlob(_requestBatchHdr);
        }

        //
        // Give the batch stream to caller, until finishBatchRequest()
        // or abortBatchRequest() is called.
        //
        _batchStreamInUse = true;
    }

    public synchronized void
    finishBatchRequest(BasicStream os)
    {
	assert(_batchStreamInUse);

        if(_exception != null)
        {
            throw _exception;
        }
        assert(_state < StateClosing);

        _batchStream.swap(os); // Get the batch stream back.
	++_batchRequestNum; // Increment the number of requests in the batch.

	//
	// Give the Connection back.
	//
        _batchStreamInUse = false;
	notifyAll();
    }

    public synchronized void
    abortBatchRequest()
    {
	assert(_batchStreamInUse);

        setState(StateClosed, new Ice.AbortBatchRequestException());

	//
	// Give the Connection back.
	//
        _batchStreamInUse = false;
	notifyAll();
    }

    public synchronized void
    flushBatchRequest()
    {
	while(_batchStreamInUse && _exception == null)
	{
	    try
	    {
		wait();
	    }
	    catch(InterruptedException ex)
	    {
	    }
	}

	if(_exception != null)
	{
	    throw _exception;
	}
	assert(_state < StateClosing);
	
	try
	{
	    if(_batchStream.size() == 0)
	    {
		return; // Nothing to send.
	    }
	    
	    _batchStream.pos(3);
	    
	    //
	    // Fill in the message size the number of requests in the batch.
	    //
	    _batchStream.writeInt(_batchStream.size());
	    _batchStream.writeInt(_batchRequestNum);
	    
	    //
	    // Send the batch request.
	    //
	    TraceUtil.traceBatchRequest("sending batch request", _batchStream, _logger, _traceLevels);
	    _transceiver.write(_batchStream, _endpoint.timeout());
	    
	    //
	    // Reset _batchStream so that new batch messages can be sent.
	    //
	    _batchStream.destroy();
	    _batchStream = new BasicStream(_instance);
	    _batchRequestNum = 0;
	}
	catch(Ice.LocalException ex)
	{
	    setState(StateClosed, ex);
	    assert(_exception != null);
	    throw _exception;
	}
    }

    public synchronized void
    sendResponse(BasicStream os)
    {
	try
	{
	    if(_state == StateClosed)
	    {
		return;
	    }
	    
	    //
	    // Fill in the message size.
	    //
	    os.pos(3);
	    final int sz = os.size();
	    os.writeInt(sz);
	    
	    //
	    // Send the reply.
	    //
	    TraceUtil.traceReply("sending reply", os, _logger, _traceLevels);
	    _transceiver.write(os, _endpoint.timeout());
	    
	    if(--_dispatchCount == 0)
	    {
		notifyAll();
	    }
	    
	    if(_state == StateClosing && _dispatchCount == 0)
	    {
		initiateShutdown();
	    }
	}
	catch(Ice.LocalException ex)
	{
	    setState(StateClosed, ex);
	}
    }

    public synchronized void
    sendNoResponse()
    {
	try
	{
	    if(_state == StateClosed)
	    {
		return;
	    }
	    
	    if(--_dispatchCount == 0)
	    {
		notifyAll();
	    }
	    
	    if(_state == StateClosing && _dispatchCount == 0)
	    {
		initiateShutdown();
	    }
	}
	catch(Ice.LocalException ex)
	{
	    setState(StateClosed, ex);
	}
    }

    public int
    timeout()
    {
        // No mutex protection necessary, _endpoint is immutable.
        return _endpoint.timeout();
    }

    public Endpoint
    endpoint()
    {
        // No mutex protection necessary, _endpoint is immutable.
        return _endpoint;
    }

    public synchronized void
    setAdapter(Ice.ObjectAdapter adapter)
    {
	//
	// We are registered with a thread pool in active and closing
	// mode. However, we only change subscription if we're in active
	// mode, and thus ignore closing mode here.
	//
	if(_state == StateActive)
	{
	    if(adapter != null && _adapter == null)
	    {
		//
		// Client is now server.
		//
		unregisterWithPool();
	    }
	    
	    if(adapter == null && _adapter != null)
	    {
		//
		// Server is now client.
		//
		unregisterWithPool();
	    }
	}
	
	_adapter = adapter;
    }

    public synchronized Ice.ObjectAdapter
    getAdapter()
    {
	return _adapter;
    }

    //
    // Operations from EventHandler
    //
    public boolean
    readable()
    {
        return true;
    }

    public void
    read(BasicStream stream)
    {
        _transceiver.read(stream, 0);
    }

    private final static byte[] _replyHdr =
    {
        Protocol.protocolVersion,
        Protocol.encodingVersion,
        Protocol.replyMsg,
        (byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
    };

    public void
    message(BasicStream stream, ThreadPool threadPool)
    {
	int invoke = 0;
	int requestId = 0;

        synchronized(this)
        {
            threadPool.promoteFollower();

            if(_state == StateClosed)
            {
                Thread.yield();
                return;
            }

            try
            {
                assert(stream.pos() == stream.size());
                stream.pos(2);
                byte messageType = stream.readByte();
                stream.pos(Protocol.headerSize);

                switch(messageType)
                {
		    case Protocol.compressedRequestMsg:
		    case Protocol.compressedRequestBatchMsg:
		    case Protocol.compressedReplyMsg:
		    {
			throw new Ice.CompressionNotSupportedException();
		    }
			
                    case Protocol.requestMsg:
                    {
                        if(_state == StateClosing)
                        {
                            TraceUtil.traceRequest("received request during closing\n" +
                                                   "(ignored by server, client will retry)",
                                                   stream, _logger, _traceLevels);
                        }
                        else
                        {
                            TraceUtil.traceRequest("received request", stream, _logger, _traceLevels);
			    requestId = stream.readInt();
			    invoke = 1;
			    ++_dispatchCount;
                        }
                        break;
                    }

                    case Protocol.requestBatchMsg:
                    {
                        if(_state == StateClosing)
                        {
                            TraceUtil.traceBatchRequest("received batch request during closing\n" +
                                                        "(ignored by server, client will retry)",
                                                        stream, _logger, _traceLevels);
                        }
                        else
                        {
                            TraceUtil.traceBatchRequest("received batch request", stream, _logger, _traceLevels);
			    invoke = stream.readInt();
			    if(invoke < 0)
			    {
				throw new Ice.NegativeSizeException();
			    }
			    _dispatchCount += invoke;
                        }
                        break;
                    }

                    case Protocol.replyMsg:
                    {
                        TraceUtil.traceReply("received reply", stream, _logger, _traceLevels);
                        requestId = stream.readInt();
                        Outgoing out = (Outgoing)_requests.remove(requestId);
                        if(out == null)
                        {
                            throw new Ice.UnknownRequestIdException();
                        }
                        out.finished(stream);
                        break;
                    }

                    case Protocol.validateConnectionMsg:
                    {
                        TraceUtil.traceHeader("received validate connection", stream, _logger, _traceLevels);
			if(_warn)
			{
			    _logger.warning("ignoring unexpected validate connection message:\n" +
					    _transceiver.toString());
			}
                        break;
                    }

                    case Protocol.closeConnectionMsg:
                    {
                        TraceUtil.traceHeader("received close connection", stream, _logger, _traceLevels);
                        if(_endpoint.datagram())
                        {
                            if(_warn)
                            {
                                _logger.warning("ignoring close connection message for datagram connection:\n" +
                                                _transceiver.toString());
                            }
                        }
                        else
                        {
                            throw new Ice.CloseConnectionException();
                        }
                        break;
                    }

                    default:
                    {
                        TraceUtil.traceHeader("received unknown message\n" +
                                              "(invalid, closing connection)",
                                              stream, _logger, _traceLevels);
                        throw new Ice.UnknownMessageException();
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
                return;
            }
        }

        //
        // Method invocation must be done outside the thread
        // synchronization, so that nested calls are possible.
        //
        if(invoke > 0)
        {
	    Incoming in = null;

            try
            {
		//
		// Prepare the invocation.
		//
		boolean response = !_endpoint.datagram() && requestId != 0;
		in = getIncoming(response);
                BasicStream is = in.is();
                stream.swap(is);
		BasicStream os = in.os();

                try
                {
		    //
		    // Prepare the response if necessary.
		    //
                    if(response)
                    {
			assert(invoke == 1);
			os.writeBlob(_replyHdr);

			//
			// Fill in the request ID.
			//
			os.writeInt(requestId);
                    }
		    
		    //
		    // Do the invocation, or multiple invocations for batch
		    // messages.
		    //
		    while(invoke-- > 0)
                    {
			in.invoke();
                    }
                }
                catch(Ice.LocalException ex)
                {
                    reclaimIncoming(in); // Must be called outside the synchronization.
                    in = null;
                    synchronized(this)
                    {
                        setState(StateClosed, ex);
                    }
                }
            }
            finally
            {
                if(in != null)
                {
                    reclaimIncoming(in); // Must be called outside the synchronization.
                }
            }
        }
    }

    public synchronized void
    finished(ThreadPool threadPool)
    {
	threadPool.promoteFollower();
	
	if(_state == StateActive || _state == StateClosing)
	{
	    registerWithPool();
	}
	else if(_state == StateClosed)
	{
	    _transceiver.close();
	    _transceiver = null;
	    notifyAll();
	}
    }

    public synchronized void
    exception(Ice.LocalException ex)
    {
	setState(StateClosed, ex);
    }

    public synchronized String
    toString()
    {
	assert(_transceiver != null);
	return _transceiver.toString();
    }

    Connection(Instance instance, Transceiver transceiver, Endpoint endpoint, Ice.ObjectAdapter adapter)
    {
        super(instance);
        _transceiver = transceiver;
        _endpoint = endpoint;
        _adapter = adapter;
        _logger = instance.logger();
        _traceLevels = instance.traceLevels();
	_registeredWithPool = false;
	_warn = _instance.properties().getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
        _nextRequestId = 1;
        _batchStream = new BasicStream(instance);
	_batchStreamInUse = false;
	_batchRequestNum = 0;
        _dispatchCount = 0;
	_proxyCount = 0;
        _state = StateHolding;
    }

    protected void
    finalize()
        throws Throwable
    {
        assert(_state == StateClosed);
	assert(_transceiver == null);
	assert(_dispatchCount == 0);
	assert(_proxyCount == 0);
        assert(_incomingCache == null);

        //
        // Destroy the EventHandler's stream, so that its buffer
        // can be reclaimed.
        //
        super._stream.destroy();

        super.finalize();
    }

    private static final int StateActive = 0;
    private static final int StateHolding = 1;
    private static final int StateClosing = 2;
    private static final int StateClosed = 3;

    private void
    setState(int state, Ice.LocalException ex)
    {
        if(_state == state) // Don't switch twice.
        {
            return;
        }

        if(_exception == null)
        {
	    _exception = ex;

            if(_warn)
            {
                //
                // Don't warn about certain expected exceptions.
                //
                if(!(_exception instanceof Ice.CloseConnectionException ||
		     _exception instanceof Ice.CommunicatorDestroyedException ||
		     _exception instanceof Ice.ObjectAdapterDeactivatedException ||
		     (_exception instanceof Ice.ConnectionLostException && _state == StateClosing)))
                {
                    warning("connection exception", _exception);
                }
            }
        }

        java.util.Iterator i = _requests.entryIterator();
        while(i.hasNext())
        {
            IntMap.Entry e = (IntMap.Entry)i.next();
            Outgoing out = (Outgoing)e.getValue();
            out.finished(_exception);
        }
        _requests.clear();

        setState(state);
    }

    private void
    setState(int state)
    {
        //
        // We don't want to send close connection messages if the endpoint
        // only supports oneway transmission from client to server.
        //
        if(_endpoint.datagram() && state == StateClosing)
        {
            state = StateClosed;
        }

        if(_state == state) // Don't switch twice.
        {
            return;
        }

        switch(state)
        {
            case StateActive:
            {
                if(_state != StateHolding) // Can only switch from holding to active.
                {
                    return;
                }
                registerWithPool();
                break;
            }

            case StateHolding:
            {
                if(_state != StateActive) // Can only switch from active to holding.
                {
                    return;
                }
                unregisterWithPool();
                break;
            }

            case StateClosing:
            {
                if(_state == StateClosed) // Can't change back from closed.
                {
                    return;
                }
                if(_state == StateHolding)
                {
                    //
                    // We need to continue to read data in closing state.
                    //
                    registerWithPool();
                }
                break;
            }

            case StateClosed:
            {
                if(_state == StateHolding)
                {
                    //
                    // If we come from holding state, we first need to
                    // register again before we unregister, so that
                    // finished() is called correctly.
                    //
                    registerWithPool();
                }
                unregisterWithPool();
                destroyIncomingCache();
		_dispatchCount = 0;
                break;
            }
        }

        _state = state;
	notifyAll();

        if(_state == StateClosing && _dispatchCount == 0)
        {
            try
            {
                initiateShutdown();
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
            }
        }
    }

    private void
    initiateShutdown()
    {
	assert(_state == StateClosing);
	assert(_dispatchCount == 0);

	if(!_endpoint.datagram())
	{
	    //
	    // Before we shut down, we send a close connection
	    // message.
	    //
	    BasicStream os = new BasicStream(_instance);
	    os.writeByte(Protocol.protocolVersion);
	    os.writeByte(Protocol.encodingVersion);
	    os.writeByte(Protocol.closeConnectionMsg);
	    os.writeInt(Protocol.headerSize); // Message size.
	    _transceiver.write(os, _endpoint.timeout());
	    _transceiver.shutdown();
	}
    }

    private void
    registerWithPool()
    {
	if(!_registeredWithPool)
	{
	    if(_adapter != null)
	    {
		if(_serverThreadPool == null) // Lazy initialization.
		{
		    _serverThreadPool = _instance.serverThreadPool();
		    assert(_serverThreadPool != null);
		}
		_serverThreadPool._register(_transceiver.fd(), this);
	    }
	    else
	    {
		if(_clientThreadPool == null) // Lazy initialization.
		{
		    _clientThreadPool = _instance.clientThreadPool();
		    assert(_clientThreadPool != null);
		}
		_clientThreadPool._register(_transceiver.fd(), this);
	    }

	    _registeredWithPool = true;
	}
    }

    private void
    unregisterWithPool()
    {
	if(_registeredWithPool)
	{
	    if(_adapter != null)
	    {
		assert(_serverThreadPool != null);
		_serverThreadPool.unregister(_transceiver.fd());
	    }
	    else
	    {
		assert(_clientThreadPool != null);
		_clientThreadPool.unregister(_transceiver.fd());
	    }

	    _registeredWithPool = false;	
	}
    }

    private void
    warning(String msg, Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        Throwable t = ex;
        do
        {
            t.printStackTrace(pw);
            t = t.getCause();
            if(t != null)
            {
                pw.println("Caused by:\n");
            }
        }
        while(t != null);
        pw.flush();
        String s = msg + ":\n" + sw.toString() + _transceiver.toString();
        _logger.warning(s);
    }

    private Incoming
    getIncoming(boolean response)
    {
        Incoming in = null;

        synchronized(_incomingCacheMutex)
        {
            if(_incomingCache == null)
            {
                in = new Incoming(_instance, _adapter, this, response);
            }
            else
            {
                in = _incomingCache;
                _incomingCache = _incomingCache.next;
                in.next = null;
                in.reset(_adapter, this, response);
            }
        }

        return in;
    }

    private void
    reclaimIncoming(Incoming in)
    {
        synchronized(_incomingCacheMutex)
        {
            in.next = _incomingCache;
            _incomingCache = in;
        }
    }

    private void
    destroyIncomingCache()
    {
        Incoming in = null;

        synchronized(_incomingCacheMutex)
        {
            in = _incomingCache;
            _incomingCache = null;
        }

        while(in != null)
        {
            in.destroy();
            in = in.next;
        }
    }

    private Transceiver _transceiver;
    private final Endpoint _endpoint;

    private Ice.ObjectAdapter _adapter;

    private final Ice.Logger _logger;
    private final TraceLevels _traceLevels;

    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private boolean _registeredWithPool;

    private final boolean _warn;

    private int _nextRequestId;
    private IntMap _requests = new IntMap();

    private Ice.LocalException _exception;

    private BasicStream _batchStream;
    private boolean _batchStreamInUse;
    private int _batchRequestNum;

    private int _dispatchCount; // The number of requests currently being dispatched.
    private int _proxyCount; // The number of proxies using this connection.

    private int _state;

    private Incoming _incomingCache;
    private java.lang.Object _incomingCacheMutex = new java.lang.Object();
}
