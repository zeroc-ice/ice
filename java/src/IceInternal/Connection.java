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
    public boolean
    destroyed()
    {
        _mutex.lock();
        try
        {
            return _state >= StateClosing;
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    validate()
    {
        _mutex.lock();
        try
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
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    hold()
    {
        _mutex.lock();
        try
        {
            setState(StateHolding);
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    activate()
    {
        _mutex.lock();
        try
        {
            setState(StateActive);
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    incUsageCount()
    {
	_mutex.lock();
        try
        {
	    assert(_usageCount >= 0);
	    ++_usageCount;
	}
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    decUsageCount()
    {
	_mutex.lock();
        try
        {
            assert(_usageCount > 0);
	    --_usageCount;
	    if(_usageCount == 0 && _adapter == null)
	    {
		assert(_requests.isEmpty());
		setState(StateClosing, new Ice.CloseConnectionException());
	    }
	}
        finally
        {
            _mutex.unlock();
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

    public void
    sendRequest(Outgoing out, boolean oneway)
    {
        _mutex.lock();
        try
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
        finally
        {
            _mutex.unlock();
        }
    }

    private final static byte[] _requestBatchHdr =
    {
        Protocol.protocolVersion,
        Protocol.encodingVersion,
        Protocol.requestBatchMsg,
        (byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
    };

    public void
    prepareBatchRequest(BasicStream os)
    {
        _mutex.lock();

        if(_exception != null)
        {
            _mutex.unlock();
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
        // is called.
        //
        _batchStream.swap(os);
    }

    public void
    finishBatchRequest(BasicStream os)
    {
        if(_exception != null)
        {
            _mutex.unlock();
            throw _exception;
        }
        assert(_state < StateClosing);

        _batchStream.swap(os); // Get the batch stream back.
        _mutex.unlock(); // Give the Connection back.
    }

    public void
    abortBatchRequest()
    {
        setState(StateClosed, new Ice.AbortBatchRequestException());
        _mutex.unlock(); // Give the Connection back.
    }

    public void
    flushBatchRequest()
    {
        _mutex.lock();
        try
        {
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
                // Fill in the message size.
                //
                _batchStream.writeInt(_batchStream.size());
                TraceUtil.traceBatchRequest("sending batch request", _batchStream, _logger, _traceLevels);
                _transceiver.write(_batchStream, _endpoint.timeout());

                //
                // Reset _batchStream so that new batch messages can be sent.
                //
                _batchStream.destroy();
                _batchStream = new BasicStream(_instance);
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
		assert(_exception != null);
		throw _exception;
            }
        }
        finally
        {
            _mutex.unlock();
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

    public void
    setAdapter(Ice.ObjectAdapter adapter)
    {
        _mutex.lock();
        try
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
        finally
        {
            _mutex.unlock();
        }
    }

    public Ice.ObjectAdapter
    getAdapter()
    {
        _mutex.lock();
        try
        {
            return _adapter;
        }
        finally
        {
            _mutex.unlock();
        }
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
        Incoming in = null;
        boolean batch = false;

        _mutex.lock();
        try
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
                            in = getIncoming();
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
                            in = getIncoming();
                            batch = true;
                        }
                        break;
                    }

                    case Protocol.replyMsg:
                    {
                        TraceUtil.traceReply("received reply", stream, _logger, _traceLevels);
                        int requestId = stream.readInt();
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
        finally
        {
            _mutex.unlock();
        }

        //
        // Method invocation must be done outside the thread
        // synchronization, so that nested calls are possible.
        //
        if(in != null)
        {
            try
            {
		//
		// Prepare the invocation.
		//
                BasicStream is = in.is();
                stream.swap(is);
                BasicStream os = null;

                try
                {
		    //
		    // Prepare the response if necessary.
		    //
                    if(!batch)
                    {
                        int requestId = is.readInt();
                        if(!_endpoint.datagram() && requestId != 0) // 0 means oneway.
                        {
                            ++_responseCount;
			    os = in.os();
                            os.writeBlob(_replyHdr);
                            os.writeInt(requestId);
                        }
                    }
		    
		    //
		    // Do the invocation, or multiple invocations for
		    // batch messages.
		    //
                    do
                    {
			in.invoke(os != null);
                    }
                    while(batch && is.pos() < is.size());
                }
                catch(Ice.LocalException ex)
                {
                    _mutex.lock();
                    reclaimIncoming(in);
                    in = null;
                    try
                    {
                        setState(StateClosed, ex);
                        return;
                    }
                    finally
                    {
                        _mutex.unlock();
                    }
                }

		//
		// Send a response if necessary.
		//
                if(os != null)
                {
                    _mutex.lock();

                    try
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

                            TraceUtil.traceReply("sending reply", os, _logger, _traceLevels);
                            _transceiver.write(os, _endpoint.timeout());

                            --_responseCount;

                            if(_state == StateClosing && _responseCount == 0 && !_endpoint.datagram())
                            {
                                closeConnection();
                            }
                        }
                        catch(Ice.LocalException ex)
                        {
                            setState(StateClosed, ex);
                            return;
                        }
                    }
                    finally
                    {
                        if(in != null)
                        {
                            reclaimIncoming(in);
                            in = null;
                        }
                        _mutex.unlock();
                    }
                }
            }
            finally
            {
                if(in != null)
                {
                    _mutex.lock();
                    reclaimIncoming(in);
                    _mutex.unlock();
                }
            }
        }
    }

    public void
    finished(ThreadPool threadPool)
    {
        _mutex.lock();
        try
        {
            threadPool.promoteFollower();

            if(_state == StateActive || _state == StateClosing)
            {
                registerWithPool();
            }
            else if(_state == StateClosed)
            {
                _transceiver.close();
            }
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public void
    exception(Ice.LocalException ex)
    {
        _mutex.lock();
        try
        {
            setState(StateClosed, ex);
        }
        finally
        {
            _mutex.unlock();
        }
    }

    public String toString()
    {
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
	_warn = _instance.properties().getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
        _nextRequestId = 1;
        _batchStream = new BasicStream(instance);
        _responseCount = 0;
	_usageCount = 0;
        _state = StateHolding;
	_registeredWithPool = false;
    }

    protected void
    finalize()
        throws Throwable
    {
	assert(_usageCount == 0);
        assert(_state == StateClosed);

        //
        // Destroy the EventHandler's stream, so that its buffer
        // can be reclaimed.
        //
        super._stream.destroy();

        super.finalize();
    }

    // DestructionReason
    public final static int ObjectAdapterDeactivated = 0;
    public final static int CommunicatorDestroyed = 1;

    public void
    destroy(int reason)
    {
        _mutex.lock();
        try
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
        finally
        {
            _mutex.unlock();
        }
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
                destroyIncoming();
                break;
            }
        }

        _state = state;

        if(_state == StateClosing && _responseCount == 0 && !_endpoint.datagram())
        {
            try
            {
                closeConnection();
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
            }
        }
    }

    private void
    closeConnection()
    {
        BasicStream os = new BasicStream(_instance);
        os.writeByte(Protocol.protocolVersion);
        os.writeByte(Protocol.encodingVersion);
        os.writeByte(Protocol.closeConnectionMsg);
        os.writeInt(Protocol.headerSize); // Message size.
        _transceiver.write(os, _endpoint.timeout());

	//
	// A close connection is always followed by a connection
	// shutdown.
	//
        _transceiver.shutdown();
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
    getIncoming()
    {
        Incoming in;
        if(_incomingCache == null)
        {
            in = new Incoming(_instance, _adapter);
        }
        else
        {
            in = _incomingCache;
            _incomingCache = _incomingCache.next;
            in.next = null;
            in.reset(_adapter);
        }
        return in;
    }

    private void
    reclaimIncoming(Incoming in)
    {
        in.next = _incomingCache;
        _incomingCache = in;
    }

    private void
    destroyIncoming()
    {
        Incoming in = _incomingCache;
        while(in != null)
        {
            in.destroy();
            in = in.next;
        }
    }

    private final Transceiver _transceiver;
    private final Endpoint _endpoint;
    private Ice.ObjectAdapter _adapter;
    private final Ice.Logger _logger;
    private final TraceLevels _traceLevels;
    private ThreadPool _clientThreadPool;
    private ThreadPool _serverThreadPool;
    private final boolean _warn;
    private int _nextRequestId;
    private IntMap _requests = new IntMap();
    private Ice.LocalException _exception;
    private BasicStream _batchStream;
    private int _responseCount;
    private int _usageCount;
    private int _state;
    private boolean _registeredWithPool;
    private RecursiveMutex _mutex = new RecursiveMutex();
    private Incoming _incomingCache;
}
