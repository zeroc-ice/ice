// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class ConnectionI extends IceInternal.EventHandler implements Connection
{
    public java.lang.Object
    clone()
        throws java.lang.CloneNotSupportedException
    {
        return super.clone();
    }

    public int
    ice_hash()
    {
        return hashCode();
    }

    public void
    validate()
    {
	if(!_endpoint.datagram()) // Datagram connections are always implicitly validated.
	{
	    boolean active;
	    
	    synchronized(this)
	    {
		if(_instance.threadPerConnection() && _threadPerConnection != Thread.currentThread())
		{
		    //
		    // In thread per connection mode, this connection's thread
		    // will take care of connection validation. Therefore all we
		    // have to do here is to wait until this thread has completed
		    // validation.
		    //
		    while(_state == StateNotValidated)
		    {
			try
			{
			    wait();
			}
			catch(InterruptedException ex)
			{
			}
		    }
		    
		    if(_state >= StateClosing)
		    {
			assert(_exception != null);
			throw _exception;
		    }
		    
		    return;
		}
		
		assert(_state == StateNotValidated);
		
		if(_adapter != null)
		{
		    active = true; // The server side has the active role for connection validation.
		}
		else
		{
		    active = false; // The client side has the passive role for connection validation.
		}	    
	    }

	    try
	    {
		int timeout;
		IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
		if(defaultsAndOverrides.overrideConnectTimeout)
		{
		    timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
		}
		else
		{
		    timeout = _endpoint.timeout();
		}

		if(active)
		{
		    IceInternal.BasicStream os = new IceInternal.BasicStream(_instance);
		    os.writeBlob(IceInternal.Protocol.magic);
		    os.writeByte(IceInternal.Protocol.protocolMajor);
		    os.writeByte(IceInternal.Protocol.protocolMinor);
		    os.writeByte(IceInternal.Protocol.encodingMajor);
		    os.writeByte(IceInternal.Protocol.encodingMinor);
		    os.writeByte(IceInternal.Protocol.validateConnectionMsg);
		    os.writeByte((byte)0); // Compression status (always zero for validate connection).
		    os.writeInt(IceInternal.Protocol.headerSize); // Message size.
		    IceInternal.TraceUtil.traceHeader("sending validate connection", os, _logger, _traceLevels);
		    try
		    {
			_transceiver.write(os, timeout);
		    }
		    catch(Ice.TimeoutException ex)
		    {
			throw new Ice.ConnectTimeoutException();
		    }
		}
		else
		{
		    IceInternal.BasicStream is = new IceInternal.BasicStream(_instance);
		    is.resize(IceInternal.Protocol.headerSize, true);
		    is.pos(0);
		    try
		    {
			_transceiver.read(is, timeout);
		    }
		    catch(Ice.TimeoutException ex)
		    {
			throw new Ice.ConnectTimeoutException();
		    }
		    assert(is.pos() == IceInternal.Protocol.headerSize);
		    is.pos(0);
		    byte[] m = is.readBlob(4);
		    if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
		       m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
		    {
		        BadMagicException ex = new BadMagicException();
			ex.badMagic = m;
			throw ex;
		    }
		    byte pMajor = is.readByte();
		    byte pMinor = is.readByte();
		    if(pMajor != IceInternal.Protocol.protocolMajor)
		    {
			UnsupportedProtocolException e = new UnsupportedProtocolException();
			e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
			e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
			e.major = IceInternal.Protocol.protocolMajor;
			e.minor = IceInternal.Protocol.protocolMinor;
			throw e;
		    }
		    byte eMajor = is.readByte();
		    byte eMinor = is.readByte();
		    if(eMajor != IceInternal.Protocol.encodingMajor)
		    {
			UnsupportedEncodingException e = new UnsupportedEncodingException();
			e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
			e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
			e.major = IceInternal.Protocol.encodingMajor;
			e.minor = IceInternal.Protocol.encodingMinor;
			throw e;
		    }
		    byte messageType = is.readByte();
		    if(messageType != IceInternal.Protocol.validateConnectionMsg)
		    {
			throw new ConnectionNotValidatedException();
		    }
		    byte compress = is.readByte(); // Ignore compression status for validate connection.
		    int size = is.readInt();
		    if(size != IceInternal.Protocol.headerSize)
		    {
			throw new IllegalMessageSizeException();
		    }
		    IceInternal.TraceUtil.traceHeader("received validate connection", is, _logger, _traceLevels);
		}
	    }
	    catch(LocalException ex)
	    {
		synchronized(this)
		{
		    setState(StateClosed, ex);
		    assert(_exception != null);
		    throw _exception;
		}
	    }
	}
	
	synchronized(this)
	{
	    if(_acmTimeout > 0)
	    {
		_acmAbsoluteTimeoutMillis = System.currentTimeMillis() + _acmTimeout * 1000;
	    }
	    
	    //
	    // We start out in holding state.
	    //
	    setState(StateHolding);
	}
    }

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

    // DestructionReason.
    public final static int ObjectAdapterDeactivated = 0;
    public final static int CommunicatorDestroyed = 1;

    public synchronized void
    destroy(int reason)
    {
	switch(reason)
	{
	    case ObjectAdapterDeactivated:
	    {
		setState(StateClosing, new ObjectAdapterDeactivatedException());
		break;
	    }
	    
	    case CommunicatorDestroyed:
	    {
		setState(StateClosing, new CommunicatorDestroyedException());
		break;
	    }
	}
    }

    public synchronized void
    close(boolean force)
    {
	if(force)
	{
	    setState(StateClosed, new ForcedCloseConnectionException());
	}
	else
	{
	    setState(StateClosing, new CloseConnectionException());
	}
    }

    public synchronized boolean
    isDestroyed()
    {
	return _state >= StateClosing;
    }

    public boolean
    isFinished()
    {
	Thread threadPerConnection = null;

	synchronized(this)
	{
	    if(_transceiver != null || _dispatchCount != 0 ||
	       (_threadPerConnection != null &&	_threadPerConnection.isAlive()))
	    {
		return false;
	    }

	    assert(_state == StateClosed);

	    threadPerConnection = _threadPerConnection;
	    _threadPerConnection = null;

	    _incomingCache = null;
	}

	if(threadPerConnection != null)
	{
	    while(true)
	    {
		try
		{
		    threadPerConnection.join();
		    break;
		}
		catch(InterruptedException ex)
		{
		}
	    }
	}

	return true;
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

    public void
    waitUntilFinished()
    {
	Thread threadPerConnection = null;

	synchronized(this)
	{
	    //
	    // We wait indefinitely until connection closing has been
	    // initiated. We also wait indefinitely until all outstanding
	    // requests are completed. Otherwise we couldn't guarantee
	    // that there are no outstanding calls when deactivate() is
	    // called on the servant locators.
	    //
	    while(_state < StateClosing || _dispatchCount > 0)
	    {
		try
		{
		    wait();
		}
		catch(InterruptedException ex)
		{
		}
	    }
	    
	    //
	    // Now we must wait until close() has been called on the
	    // transceiver.
	    //
	    while(_transceiver != null)
	    {
		try
		{
		    if(_state != StateClosed && _endpoint.timeout() >= 0)
		    {
			long absoluteWaitTime = _stateTime + _endpoint.timeout();
			long waitTime = absoluteWaitTime - System.currentTimeMillis();
			
			if(waitTime > 0)
			{
			    //
			    // We must wait a bit longer until we close this
			    // connection.
			    //
			    wait(waitTime);
			    if(System.currentTimeMillis() >= absoluteWaitTime)
			    {
				setState(StateClosed, new CloseTimeoutException());
			    }
			}
			else
			{
			    //
			    // We already waited long enough, so let's close this
			    // connection!
			    //
			    setState(StateClosed, new CloseTimeoutException());
			}

			//
			// No return here, we must still wait until
			// close() is called on the _transceiver.
			//
		    }
		    else
		    {
			wait();
		    }
		}
		catch(InterruptedException ex)
		{
		}
	    }

	    assert(_state == StateClosed);

	    threadPerConnection = _threadPerConnection;
	    _threadPerConnection = null;

	    _incomingCache = null;
	}

	if(threadPerConnection != null)
	{
	    while(true)
	    {
		try
		{
		    threadPerConnection.join();
		    break;
		}
		catch(InterruptedException ex)
		{
		}
	    }
	}
    }

    public synchronized void
    monitor()
    {
	if(_state != StateActive)
	{
	    return;
	}
	
	//
	// Check for timed out async requests.
	//
	java.util.Iterator i = _asyncRequests.entryIterator();
	while(i.hasNext())
	{
	    IceInternal.IntMap.Entry e = (IceInternal.IntMap.Entry)i.next();
	    IceInternal.OutgoingAsync out = (IceInternal.OutgoingAsync)e.getValue();
	    if(out.__timedOut())
	    {
		setState(StateClosed, new TimeoutException());
		return;
	    }
	}

	//
	// Active connection management for idle connections.
	//
	if(_acmTimeout > 0 &&
	   _requests.isEmpty() && _asyncRequests.isEmpty() &&
	   !_batchStreamInUse && _batchStream.isEmpty() &&
	   _dispatchCount == 0)
	{
	    if(System.currentTimeMillis() >= _acmAbsoluteTimeoutMillis)
	    {
		setState(StateClosing, new ConnectionTimeoutException());
		return;
	    }
	}	    
    }

    private final static byte[] _requestHdr =
    {
	IceInternal.Protocol.magic[0],
	IceInternal.Protocol.magic[1],
	IceInternal.Protocol.magic[2],
	IceInternal.Protocol.magic[3],
        IceInternal.Protocol.protocolMajor,
        IceInternal.Protocol.protocolMinor,
        IceInternal.Protocol.encodingMajor,
        IceInternal.Protocol.encodingMinor,
        IceInternal.Protocol.requestMsg,
        (byte)0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
        (byte)0, (byte)0, (byte)0, (byte)0  // Request ID (placeholder).
    };

    //
    // TODO: Should not be a member function of ConnectionI.
    //
    public void
    prepareRequest(IceInternal.BasicStream os)
    {
        os.writeBlob(_requestHdr);
    }

    public void
    sendRequest(IceInternal.BasicStream os, IceInternal.Outgoing out, boolean compress)
    {
	int requestId = 0;
	IceInternal.BasicStream stream = null;

	synchronized(this)
	{
	    assert(!(out != null && _endpoint.datagram())); // Twoway requests cannot be datagrams.

	    if(_exception != null)
	    {
		throw _exception;
	    }

	    assert(_state > StateNotValidated);
	    assert(_state < StateClosing);

	    //
	    // Only add to the request map if this is a twoway call.
	    //
	    if(out != null)
	    {
		//
		// Create a new unique request ID.
		//
		requestId = _nextRequestId++;
		if(requestId <= 0)
		{
		    _nextRequestId = 1;
		    requestId = _nextRequestId++;
		}

		//
		// Fill in the request ID.
		//
		os.pos(IceInternal.Protocol.headerSize);
		os.writeInt(requestId);

		//
		// Add to the requests map.
		//
		_requests.put(requestId, out);
	    }

	    stream = doCompress(os, _overrideCompress ? _overrideCompressValue : compress);

	    if(_acmTimeout > 0)
	    {
		_acmAbsoluteTimeoutMillis = System.currentTimeMillis() + _acmTimeout * 1000;
	    }
	}

	try
	{
	    synchronized(_sendMutex)
	    {
		if(_transceiver == null) // Has the transceiver already been closed?
		{
		    assert(_exception != null);
		    throw _exception; // The exception is immutable at this point.
		}

		//
		// Send the request.
		//
		IceInternal.TraceUtil.traceRequest("sending request", os, _logger, _traceLevels);
		_transceiver.write(stream, _endpoint.timeout());
	    }
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		setState(StateClosed, ex);
		assert(_exception != null);
		
		if(out != null)
		{
		    //
		    // If the request has already been removed from
		    // the request map, we are out of luck. It would
		    // mean that finished() has been called already,
		    // and therefore the exception has been set using
		    // the Outgoing::finished() callback. In this
		    // case, we cannot throw the exception here,
		    // because we must not both raise an exception and
		    // have Outgoing::finished() called with an
		    // exception. This means that in some rare cases,
		    // a request will not be retried even though it
		    // could. But I honestly don't know how I could
		    // avoid this, without a very elaborate and
		    // complex design, which would be bad for
		    // performance.
		    //
		    IceInternal.Outgoing o = (IceInternal.Outgoing)_requests.remove(requestId);
		    if(o != null)
		    {
			assert(o == out);
			throw _exception;
		    }
		}
		else
		{
		    throw _exception;
		}
	    }
	}
    }

    public void
    sendAsyncRequest(IceInternal.BasicStream os, IceInternal.OutgoingAsync out, boolean compress)
    {
	int requestId = 0;
	IceInternal.BasicStream stream = null;

	synchronized(this)
	{
	    assert(!_endpoint.datagram()); // Twoway requests cannot be datagrams, and async implies twoway.

	    if(_exception != null)
	    {
		throw _exception;
	    }

	    assert(_state > StateNotValidated);
	    assert(_state < StateClosing);

	    //
	    // Create a new unique request ID.
	    //
	    requestId = _nextRequestId++;
	    if(requestId <= 0)
	    {
		_nextRequestId = 1;
		requestId = _nextRequestId++;
	    }
	    
	    //
	    // Fill in the request ID.
	    //
	    os.pos(IceInternal.Protocol.headerSize);
	    os.writeInt(requestId);
	    
	    //
	    // Add to the async requests map.
	    //
	    _asyncRequests.put(requestId, out);

	    stream = doCompress(os, _overrideCompress ? _overrideCompressValue : compress);

	    if(_acmTimeout > 0)
	    {
		_acmAbsoluteTimeoutMillis = System.currentTimeMillis() + _acmTimeout * 1000;
	    }
	}

	try
	{
	    synchronized(_sendMutex)
	    {
		if(_transceiver == null) // Has the transceiver already been closed?
		{
		    assert(_exception != null);
		    throw _exception; // The exception is immutable at this point.
		}

		//
		// Send the request.
		//
		IceInternal.TraceUtil.traceRequest("sending asynchronous request", os, _logger, _traceLevels);
		_transceiver.write(stream, _endpoint.timeout());
	    }
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		setState(StateClosed, ex);
		assert(_exception != null);
		
		//
		// If the request has already been removed from the
		// async request map, we are out of luck. It would
		// mean that finished() has been called already, and
		// therefore the exception has been set using the
		// OutgoingAsync::__finished() callback. In this case,
		// we cannot throw the exception here, because we must
		// not both raise an exception and have
		// OutgoingAsync::__finished() called with an
		// exception. This means that in some rare cases, a
		// request will not be retried even though it
		// could. But I honestly don't know how I could avoid
		// this, without a very elaborate and complex design,
		// which would be bad for performance.
		//
		IceInternal.OutgoingAsync o = (IceInternal.OutgoingAsync)_asyncRequests.remove(requestId);
		if(o != null)
		{
		    assert(o == out);
		    throw _exception;
		}
	    }
	}
    }

    private final static byte[] _requestBatchHdr =
    {
	IceInternal.Protocol.magic[0],
	IceInternal.Protocol.magic[1],
	IceInternal.Protocol.magic[2],
	IceInternal.Protocol.magic[3],
        IceInternal.Protocol.protocolMajor,
        IceInternal.Protocol.protocolMinor,
        IceInternal.Protocol.encodingMajor,
        IceInternal.Protocol.encodingMinor,
        IceInternal.Protocol.requestBatchMsg,
        0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
        (byte)0, (byte)0, (byte)0, (byte)0  // Number of requests in batch (placeholder).
    };

    public synchronized void
    prepareBatchRequest(IceInternal.BasicStream os)
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

        assert(_state > StateNotValidated);
	assert(_state < StateClosing);

        if(_batchStream.isEmpty())
        {
	    try
	    {
		_batchStream.writeBlob(_requestBatchHdr);
	    }
	    catch(LocalException ex)
	    {
		setState(StateClosed, ex);
		throw ex;
	    }
        }

        _batchStreamInUse = true;
	_batchStream.swap(os);

	//
	// The batch stream now belongs to the caller, until
	// finishBatchRequest() or abortBatchRequest() is called.
	//
    }

    public synchronized void
    finishBatchRequest(IceInternal.BasicStream os, boolean compress)
    {
	//
	// Get the batch stream back and increment the number of
	// requests in the batch.
        //
	_batchStream.swap(os);
	++_batchRequestNum;

	//
	// We compress the whole batch if there is at least one compressed
	// message.
	//
	if(compress)
	{
	    _batchRequestCompress = true;
	}

	//
	// Notify about the batch stream not being in use anymore.
	//
	assert(_batchStreamInUse);
        _batchStreamInUse = false;
	notifyAll();
    }

    public synchronized void
    abortBatchRequest()
    {
	//
	// Destroy and reset the batch stream and batch count. We
	// cannot save old requests in the batch stream, as they might
	// be corrupted due to incomplete marshaling.
	//
	_batchStream = new IceInternal.BasicStream(_instance);
	_batchRequestNum = 0;
	_batchRequestCompress = false;

	//
	// Notify about the batch stream not being in use anymore.
	//
	assert(_batchStreamInUse);
        _batchStreamInUse = false;
	notifyAll();
    }

    public void
    flushBatchRequests()
    {
	IceInternal.BasicStream stream = null;

	synchronized(this)
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

	    if(_batchStream.isEmpty())
	    {
		return; // Nothing to do.
	    }

	    assert(_state > StateNotValidated);
	    assert(_state < StateClosing);

	    //
	    // Fill in the message size.
	    //
	    _batchStream.pos(10);
	    _batchStream.writeInt(_batchStream.size());

	    //
	    // Fill in the number of requests in the batch.
	    //
	    _batchStream.writeInt(_batchRequestNum);

	    stream = doCompress(_batchStream, _overrideCompress ? _overrideCompressValue : _batchRequestCompress);

	    if(_acmTimeout > 0)
	    {
		_acmAbsoluteTimeoutMillis = System.currentTimeMillis() + _acmTimeout * 1000;
	    }

	    //
	    // Prevent that new batch requests are added while we are
	    // flushing.
	    //
	    _batchStreamInUse = true;
	}

	try
	{
	    synchronized(_sendMutex)
	    {
		if(_transceiver == null) // Has the transceiver already been closed?
		{
		    assert(_exception != null);
		    throw _exception; // The exception is immutable at this point.
		}

		//
		// Send the batch request.
		//
		IceInternal.TraceUtil.traceBatchRequest("sending batch request", _batchStream, _logger, _traceLevels);
		_transceiver.write(stream, _endpoint.timeout());
	    }
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		setState(StateClosed, ex);
		assert(_exception != null);
		
		//
		// Since batch requests are all oneways (or datagrams), we
		// must report the exception to the caller.
		//
		throw _exception;
	    }
	}

	synchronized(this)
	{
	    //
	    // Reset the batch stream, and notify that flushing is over.
	    //
	    _batchStream = new IceInternal.BasicStream(_instance);
	    _batchRequestNum = 0;
	    _batchRequestCompress = false;
	    _batchStreamInUse = false;
	    notifyAll();
	}
    }

    public void
    sendResponse(IceInternal.BasicStream os, byte compressFlag)
    {
	IceInternal.BasicStream stream = null;
	try
	{
	    synchronized(_sendMutex)
	    {
		if(_transceiver == null) // Has the transceiver already been closed?
		{
		    assert(_exception != null);
		    throw _exception; // The exception is immutable at this point.
		}

		stream = doCompress(os, compressFlag != 0);

		//
		// Send the reply.
		//
		IceInternal.TraceUtil.traceReply("sending reply", os, _logger, _traceLevels);
		_transceiver.write(stream, _endpoint.timeout());
	    }
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		setState(StateClosed, ex);
	    }
	}

	synchronized(this)
	{
	    assert(_state > StateNotValidated);

	    try
	    {
		if(--_dispatchCount == 0)
		{
		    notifyAll();
		}
		
		if(_state == StateClosing && _dispatchCount == 0)
		{
		    initiateShutdown();
		}

		if(_acmTimeout > 0)
		{
		    _acmAbsoluteTimeoutMillis = System.currentTimeMillis() + _acmTimeout * 1000;
		}
	    }
	    catch(LocalException ex)
	    {
		setState(StateClosed, ex);
	    }
	}
    }

    public synchronized void
    sendNoResponse()
    {
	assert(_state > StateNotValidated);
	
	try
	{
	    if(--_dispatchCount == 0)
	    {
		notifyAll();
	    }
	    
	    if(_state == StateClosing && _dispatchCount == 0)
	    {
		initiateShutdown();
	    }
	}
	catch(LocalException ex)
	{
	    setState(StateClosed, ex);
	}
    }

    public IceInternal.Endpoint
    endpoint()
    {
        // No mutex protection necessary, _endpoint is immutable.
        return _endpoint;
    }

    public synchronized void
    setAdapter(ObjectAdapter adapter)
    {
	if(_exception != null)
	{
	    throw _exception;
	}
	
	assert(_state < StateClosing);

	//
	// Before we set an adapter (or reset it) we wait until the
	// dispatch count with any old adapter is zero.
        //
        // A deadlock can occur if we wait while an operation is
        // outstanding on this adapter that holds a lock while
        // calling this function (e.g., __getDelegate).
        //
        // In order to avoid such a deadlock, we only wait if the new
        // adapter is different than the current one.
        //
        // TODO: Verify that this fix solves all cases.
	//
        if(_adapter != adapter)
        {
            while(_dispatchCount > 0)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            //
            // We never change the thread pool with which we were
            // initially registered, even if we add or remove an object
            // adapter.
            //
            
            _adapter = adapter;
            if(_adapter != null)
            {
                _servantManager = ((ObjectAdapterI)_adapter).getServantManager();
            }
            else
            {
                _servantManager = null;
            }
        }
    }

    public synchronized ObjectAdapter
    getAdapter()
    {
	return _adapter;
    }

    public synchronized ObjectPrx
    createProxy(Identity ident)
    {
        //
        // Create a reference and return a reverse proxy for this
        // reference.
        //
        ConnectionI[] connections = new ConnectionI[1];
        connections[0] = this;
        IceInternal.Reference ref = _instance.referenceFactory().create(ident, new java.util.HashMap(), "",
                                                                        IceInternal.Reference.ModeTwoway, connections);
        return _instance.proxyFactory().referenceToProxy(ref);
    }

    //
    // Operations from EventHandler
    //

    public boolean
    datagram()
    {
	assert(!_instance.threadPerConnection()); // Only for use with a thread pool.
	return _endpoint.datagram(); // No mutex protection necessary, _endpoint is immutable.
    }

    public boolean
    readable()
    {
	assert(!_instance.threadPerConnection()); // Only for use with a thread pool.
        return true;
    }

    public void
    read(IceInternal.BasicStream stream)
    {
	assert(!_instance.threadPerConnection()); // Only for use with a thread pool.

	_transceiver.read(stream, 0);

	//
	// Updating _acmAbsoluteTimeoutMillis is to expensive here,
	// because we would have to acquire a lock just for this
	// purpose. Instead, we update _acmAbsoluteTimeoutMillis in
	// message().
	//
    }

    private final static byte[] _replyHdr =
    {
	IceInternal.Protocol.magic[0],
	IceInternal.Protocol.magic[1],
	IceInternal.Protocol.magic[2],
	IceInternal.Protocol.magic[3],
        IceInternal.Protocol.protocolMajor,
        IceInternal.Protocol.protocolMinor,
        IceInternal.Protocol.encodingMajor,
        IceInternal.Protocol.encodingMinor,
        IceInternal.Protocol.replyMsg,
        (byte)0, // Compression status.
        (byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
    };

    public void
    message(IceInternal.BasicStream stream, IceInternal.ThreadPool threadPool)
    {
	assert(!_instance.threadPerConnection()); // Only for use with a thread pool.

	MessageInfo info = new MessageInfo(stream);

        synchronized(this)
        {
	    //
	    // We must promote within the synchronization, otherwise
	    // there could be various race conditions with close
	    // connection messages and other messages.
	    //
	    threadPool.promoteFollower();

            if(_state != StateClosed)
            {
		parseMessage(info);
            }

	    //
	    // parseMessage() can close the connection, so we must check
	    // for closed state again.
	    //
	    if(_state == StateClosed)
	    {
		return;
	    }
        }

	//
	// Asynchronous replies must be handled outside the thread
	// synchronization, so that nested calls are possible.
	//
	if(info.outAsync != null)
	{
	    info.outAsync.__finished(info.stream);
	}

	//
	// Method invocation (or multiple invocations for batch messages)
	// must be done outside the thread synchronization, so that nested
	// calls are possible.
	//
	invokeAll(info.stream, info.invokeNum, info.requestId, info.compress, info.servantManager, info.adapter);
    }

    public void
    finished(IceInternal.ThreadPool threadPool)
    {
	assert(!_instance.threadPerConnection()); // Only for use with a thread pool.

	threadPool.promoteFollower();

	LocalException exception = null;

	IceInternal.IntMap requests = null;
	IceInternal.IntMap asyncRequests = null;

	synchronized(this)
	{
	    if(_state == StateActive || _state == StateClosing)
	    {
		registerWithPool();
	    }
	    else if(_state == StateClosed)
	    {
		//
		// We must make sure that nobody is sending when we
		// close the transceiver.
		//
		synchronized(_sendMutex)
		{
		    try
		    {
			_transceiver.close();
		    }
		    catch(LocalException ex)
		    {
			exception = ex;
		    }
		    
		    _transceiver = null;
		    notifyAll();
		}
	    }

	    if(_state == StateClosed || _state == StateClosing)
	    {
		requests = _requests;
		_requests = new IceInternal.IntMap();

		asyncRequests = _asyncRequests;
		_asyncRequests = new IceInternal.IntMap();
	    }
	}

	if(requests != null)
	{
	    java.util.Iterator i = requests.entryIterator();
	    while(i.hasNext())
	    {
		IceInternal.IntMap.Entry e = (IceInternal.IntMap.Entry)i.next();
		IceInternal.Outgoing out = (IceInternal.Outgoing)e.getValue();
		out.finished(_exception); // The exception is immutable at this point.
	    }
	}
	
	if(asyncRequests != null)
	{
	    java.util.Iterator i = asyncRequests.entryIterator();
	    while(i.hasNext())
	    {
		IceInternal.IntMap.Entry e = (IceInternal.IntMap.Entry)i.next();
		IceInternal.OutgoingAsync out = (IceInternal.OutgoingAsync)e.getValue();
		out.__finished(_exception); // The exception is immutable at this point.
	    }
	}

	if(exception != null)
	{
	    throw exception;
	}
    }

    public synchronized void
    exception(LocalException ex)
    {
	setState(StateClosed, ex);
    }

    public String
    type()
    {
	return _type; // No mutex lock, _type is immutable.
    }

    public int
    timeout()
    {
	return _endpoint.timeout(); // No mutex protection necessary, _endpoint is immutable.
    }

    public String
    toString()
    {
	return _toString();
    }

    public String
    _toString()
    {
	return _desc; // No mutex lock, _desc is immutable.
    }

    public ConnectionI(IceInternal.Instance instance, IceInternal.Transceiver transceiver, 
		       IceInternal.Endpoint endpoint, ObjectAdapter adapter)
    {
        super(instance);
        _transceiver = transceiver;
	_desc = transceiver.toString();
        _type = transceiver.type();
        _endpoint = endpoint;
        _adapter = adapter;
        _logger = instance.logger(); // Cached for better performance.
        _traceLevels = instance.traceLevels(); // Cached for better performance.
	_registeredWithPool = false;
	_warn = _instance.properties().getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
	_acmAbsoluteTimeoutMillis = 0;
        _nextRequestId = 1;
        _batchStream = new IceInternal.BasicStream(instance);
	_batchStreamInUse = false;
	_batchRequestNum = 0;
	_batchRequestCompress = false;
        _dispatchCount = 0;
        _state = StateNotValidated;
	_stateTime = System.currentTimeMillis();

	if(_endpoint.datagram())
	{
	    _acmTimeout = 0;
	}
	else
	{
	    if(_adapter != null)
	    {
		_acmTimeout = _instance.serverACM();
	    }
	    else
	    {
		_acmTimeout = _instance.clientACM();
	    }
	}

	if(_adapter != null)
	{
	    _servantManager = ((ObjectAdapterI)_adapter).getServantManager();
	}
	else
	{
	    _servantManager = null;
	}

	try
	{
	    if(!_instance.threadPerConnection())
	    {
		//
		// Only set _threadPool if we really need it, i.e., if we are
		// not in thread per connection mode. Thread pools have lazy
		// initialization in Instance, and we don't want them to be
		// created if they are not needed.
		//
		if(_adapter != null)
		{
		    _threadPool = ((ObjectAdapterI)_adapter).getThreadPool();
		}
		else
		{
		    _threadPool = _instance.clientThreadPool();
		}
	    }
	    else
	    {
		_threadPool = null;
		
		//
		// If we are in thread per connection mode, create the thread
		// for this connection.
		//
		_threadPerConnection = new ThreadPerConnection();
		_threadPerConnection.start();
	    }
	}
	catch(java.lang.Exception ex)
	{
	    java.io.StringWriter sw = new java.io.StringWriter();
	    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	    ex.printStackTrace(pw);
	    pw.flush();
	    String s;
	    if(_instance.threadPerConnection())
	    {
		s = "cannot create thread for connection:\n";
	    }
	    else
	    {
		s = "cannot create thread pool for connection:\n";
	    }
	    s += sw.toString();
	    _instance.logger().error(s);
	    
	    try
	    {
		_transceiver.close();
	    }
	    catch(LocalException e)
	    {
		// Here we ignore any exceptions in close().
	    }
	    
	    Ice.SyscallException e = new Ice.SyscallException();
	    e.initCause(ex);
	    throw e;
	}

	_overrideCompress = _instance.defaultsAndOverrides().overrideCompress;
	_overrideCompressValue = _instance.defaultsAndOverrides().overrideCompressValue;
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
	IceUtil.Assert.FinalizerAssert(_state == StateClosed);
	IceUtil.Assert.FinalizerAssert(_transceiver == null);
	IceUtil.Assert.FinalizerAssert(_dispatchCount == 0);
	IceUtil.Assert.FinalizerAssert(_threadPerConnection == null);
	IceUtil.Assert.FinalizerAssert(_incomingCache == null);

        super.finalize();
    }

    private static final int StateNotValidated = 0;
    private static final int StateActive = 1;
    private static final int StateHolding = 2;
    private static final int StateClosing = 3;
    private static final int StateClosed = 4;

    private void
    setState(int state, LocalException ex)
    {
	//
	// If setState() is called with an exception, then only closed
	// and closing states are permissible.
	//
	assert(state == StateClosing || state == StateClosed);

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
		// We don't warn if we are not validated.
		//
		if(_state > StateNotValidated)
		{
		    //
		    // Don't warn about certain expected exceptions.
		    //
		    if(!(_exception instanceof CloseConnectionException ||
			 _exception instanceof ConnectionTimeoutException ||
			 _exception instanceof CommunicatorDestroyedException ||
			 _exception instanceof ObjectAdapterDeactivatedException ||
			 (_exception instanceof ConnectionLostException && _state == StateClosing)))
		    {
			warning("connection exception", _exception);
		    }
		}
            }
        }

	//
	// We must set the new state before we notify requests of any
	// exceptions. Otherwise new requests may retry on a
	// connection that is not yet marked as closed or closing.
	//
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
	    case StateNotValidated:
	    {
		assert(false);
		break;
	    }

            case StateActive:
            {
		//
		// Can only switch from holding or not validated to
		// active.
		//
                if(_state != StateHolding && _state != StateNotValidated)
                {
                    return;
                }
		if(!_instance.threadPerConnection())
		{
		    registerWithPool();
		}
                break;
            }

            case StateHolding:
            {
		//
		// Can only switch from active or not validated to
		// holding.
		//
		if(_state != StateActive && _state != StateNotValidated)
		{
                    return;
                }
		if(!_instance.threadPerConnection())
		{
		    unregisterWithPool();
		}
                break;
            }

            case StateClosing:
            {
		//
		// Can't change back from closed.
		//
                if(_state == StateClosed)
                {
                    return;
                }
		if(!_instance.threadPerConnection())
		{
		    registerWithPool(); // We need to continue to read in closing state.
		}
                break;
            }
	    
            case StateClosed:
            {
		if(_instance.threadPerConnection())
		{
		    //
		    // If we are in thread per connection mode, we
		    // shutdown both for reading and writing. This will
		    // unblock and read call with an exception. The thread
		    // per connection then closes the transceiver.
		    //
		    _transceiver.shutdownReadWrite();
		}
		else if(_state == StateNotValidated)
		{
		    //
		    // If we change from not validated, we can close right
		    // away.
		    //
		    assert(!_registeredWithPool);

		    //
		    // We must make sure that nobody is sending when we
		    // close the transceiver.
		    //
		    synchronized(_sendMutex)
		    {
			try
			{
			    _transceiver.close();
			}
			catch(LocalException ex)
			{
			    // Here we ignore any exceptions in close().
			}

			_transceiver = null;
			//notifyAll(); // We notify already below.
		    }
		}
		else
		{
		    //
		    // Otherwise we first must make sure that we are
		    // registered, then we unregister, and let finished()
		    // do the close.
		    //
		    registerWithPool();
		    unregisterWithPool();

		    //
		    // We must prevent any further writes when _state == StateClosed.
		    // However, functions such as sendResponse cannot acquire the main
		    // mutex in order to check _state. Therefore we shut down the write
		    // end of the transceiver, which causes subsequent write attempts
		    // to fail with an exception.
		    //
		    _transceiver.shutdownWrite();
		}
		break;
            }
        }

	//  
	// We only register with the connection monitor if our new state
	// is StateActive. Otherwise we unregister with the connection
	// monitor, but only if we were registered before, i.e., if our
	// old state was StateActive.
	//
	IceInternal.ConnectionMonitor connectionMonitor = _instance.connectionMonitor();
	if(connectionMonitor != null)
	{
	    if(state == StateActive)
	    {
		connectionMonitor.add(this);
	    }
	    else if(_state == StateActive)
	    {
		connectionMonitor.remove(this);
	    }
	}

        _state = state;
	_stateTime = System.currentTimeMillis();

	notifyAll();

        if(_state == StateClosing && _dispatchCount == 0)
        {
            try
            {
                initiateShutdown();
            }
            catch(LocalException ex)
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
	    synchronized(_sendMutex)
	    {
		//
		// Before we shut down, we send a close connection
		// message.
		//
		IceInternal.BasicStream os = new IceInternal.BasicStream(_instance);
		os.writeBlob(IceInternal.Protocol.magic);
		os.writeByte(IceInternal.Protocol.protocolMajor);
		os.writeByte(IceInternal.Protocol.protocolMinor);
		os.writeByte(IceInternal.Protocol.encodingMajor);
		os.writeByte(IceInternal.Protocol.encodingMinor);
		os.writeByte(IceInternal.Protocol.closeConnectionMsg);
		os.writeByte(_compressionSupported ? (byte)1 : (byte)0);
		os.writeInt(IceInternal.Protocol.headerSize); // Message size.
		
		//
		// Send the message.
		//
		IceInternal.TraceUtil.traceHeader("sending close connection", os, _logger, _traceLevels);
		_transceiver.write(os, _endpoint.timeout());
		//
		// The CloseConnection message should be sufficient. Closing the write
		// end of the socket is probably an artifact of how things were done
		// in IIOP. In fact, shutting down the write end of the socket causes
		// problems on Windows by preventing the peer from using the socket.
		// For example, the peer is no longer able to continue writing a large
		// message after the socket is shutdown.
		//
		//_transceiver.shutdownWrite();
	    }
	}
    }

    private void
    registerWithPool()
    {
	assert(!_instance.threadPerConnection()); // Only for use with a thread pool.

	if(!_registeredWithPool)
	{
	    _threadPool._register(_transceiver.fd(), this);
	    _registeredWithPool = true;
	}
    }

    private void
    unregisterWithPool()
    {
	assert(!_instance.threadPerConnection()); // Only for use with a thread pool.

	if(_registeredWithPool)
	{
	    _threadPool.unregister(_transceiver.fd());
	    _registeredWithPool = false;	
	}
    }

    private IceInternal.BasicStream
    doCompress(IceInternal.BasicStream uncompressed, boolean compress)
    {
	if(_compressionSupported)
	{
	    if(compress && uncompressed.size() >= 100)
	    {
		//
		// Do compression.
		//
		IceInternal.BasicStream cstream = uncompressed.compress(IceInternal.Protocol.headerSize);
		if(cstream != null)
		{
		    //
		    // Set compression status.
		    //
		    cstream.pos(9);
		    cstream.writeByte((byte)2);

		    //
		    // Write the size of the compressed stream into the header.
		    //
		    cstream.pos(10);
		    cstream.writeInt(cstream.size());

		    //
		    // Write the compression status and size of the compressed stream into the header of the
		    // uncompressed stream -- we need this to trace requests correctly.
		    //
		    uncompressed.pos(9);
		    uncompressed.writeByte((byte)2);
		    uncompressed.writeInt(cstream.size());

		    return cstream;
		}
	    }
	}

	uncompressed.pos(9);
	uncompressed.writeByte((byte)((_compressionSupported && compress) ? 1 : 0));

	//
	// Not compressed, fill in the message size.
	//
	uncompressed.pos(10);
	uncompressed.writeInt(uncompressed.size());

	return uncompressed;
    }

    private static class MessageInfo
    {
	MessageInfo(IceInternal.BasicStream stream)
	{
	    this.stream = stream;
	}

	IceInternal.BasicStream stream;
	int invokeNum;
	int requestId;
	byte compress;
	IceInternal.ServantManager servantManager;
	ObjectAdapter adapter;
	IceInternal.OutgoingAsync outAsync;
    }

    private void
    parseMessage(MessageInfo info)
    {
	assert(_state > StateNotValidated && _state < StateClosed);

	if(_acmTimeout > 0)
	{
	    _acmAbsoluteTimeoutMillis = System.currentTimeMillis() + _acmTimeout * 1000;
	}

	try
	{
	    //
	    // We don't need to check magic and version here. This has
	    // already been done by the ThreadPool or ThreadPerConnection,
	    // which provides us with the stream.
	    //
	    assert(info.stream.pos() == info.stream.size());
	    info.stream.pos(8);
	    byte messageType = info.stream.readByte();
	    info.compress = info.stream.readByte();
	    if(info.compress == (byte)2)
	    {
		if(_compressionSupported)
		{
		    IceInternal.BasicStream ustream = info.stream.uncompress(IceInternal.Protocol.headerSize);
		    if(ustream != info.stream)
		    {
			info.stream = ustream;
		    }
		}
		else
		{
		    throw new CompressionNotSupportedException();
		}
	    }
	    info.stream.pos(IceInternal.Protocol.headerSize);

	    switch(messageType)
	    {
		case IceInternal.Protocol.closeConnectionMsg:
		{
		    IceInternal.TraceUtil.traceHeader("received close connection", info.stream, _logger, _traceLevels);
		    if(_endpoint.datagram() && _warn)
		    {
			_logger.warning("ignoring close connection message for datagram connection:\n" + _desc);
		    }
		    else
		    {
			setState(StateClosed, new CloseConnectionException());
		    }
		    break;
		}

		case IceInternal.Protocol.requestMsg:
		{
		    if(_state == StateClosing)
		    {
			IceInternal.TraceUtil.traceRequest("received request during closing\n" +
							   "(ignored by server, client will retry)",
							   info.stream, _logger, _traceLevels);
		    }
		    else
		    {
			IceInternal.TraceUtil.traceRequest("received request", info.stream, _logger, _traceLevels);
			info.requestId = info.stream.readInt();
			info.invokeNum = 1;
			info.servantManager = _servantManager;
			info.adapter = _adapter;
			++_dispatchCount;
		    }
		    break;
		}

		case IceInternal.Protocol.requestBatchMsg:
		{
		    if(_state == StateClosing)
		    {
			IceInternal.TraceUtil.traceBatchRequest("received batch request during closing\n" +
								"(ignored by server, client will retry)",
								info.stream, _logger, _traceLevels);
		    }
		    else
		    {
			IceInternal.TraceUtil.traceBatchRequest("received batch request", info.stream, _logger,
								_traceLevels);
			info.invokeNum = info.stream.readInt();
			if(info.invokeNum < 0)
			{
			    info.invokeNum = 0;
			    throw new NegativeSizeException();
			}
			info.servantManager = _servantManager;
			info.adapter = _adapter;
			_dispatchCount += info.invokeNum;
		    }
		    break;
		}

		case IceInternal.Protocol.replyMsg:
		{
		    IceInternal.TraceUtil.traceReply("received reply", info.stream, _logger, _traceLevels);
		    info.requestId = info.stream.readInt();
		    IceInternal.Outgoing out = (IceInternal.Outgoing)_requests.remove(info.requestId);
		    if(out != null)
		    {
			out.finished(info.stream);
		    }
		    else
		    {
			info.outAsync = (IceInternal.OutgoingAsync)_asyncRequests.remove(info.requestId);
			if(info.outAsync == null)
			{
			    throw new UnknownRequestIdException();
			}
		    }
		    break;
		}

		case IceInternal.Protocol.validateConnectionMsg:
		{
		    IceInternal.TraceUtil.traceHeader("received validate connection", info.stream, _logger,
						      _traceLevels);
		    if(_warn)
		    {
			_logger.warning("ignoring unexpected validate connection message:\n" + _desc);
		    }
		    break;
		}

		default:
		{
		    IceInternal.TraceUtil.traceHeader("received unknown message\n" +
						      "(invalid, closing connection)", info.stream, _logger,
						      _traceLevels);
		    throw new UnknownMessageException();
		}
	    }
	}
	catch(LocalException ex)
	{
	    setState(StateClosed, ex);
	}
    }

    private void
    invokeAll(IceInternal.BasicStream stream, int invokeNum, int requestId, byte compress,
	      IceInternal.ServantManager servantManager, ObjectAdapter adapter)
    {
	//
	// Note: In contrast to other private or protected methods, this
	// operation must be called *without* the mutex locked.
	//

	IceInternal.Incoming in = null;
	try
	{
	    while(invokeNum > 0)
	    {
		
		//
		// Prepare the invocation.
		//
		boolean response = !_endpoint.datagram() && requestId != 0;
		in = getIncoming(adapter, response, compress);
		IceInternal.BasicStream is = in.is();
		stream.swap(is);
		IceInternal.BasicStream os = in.os();
		
		//
		// Prepare the response if necessary.
		//
		if(response)
		{
		    assert(invokeNum == 1); // No further invocations if a response is expected.
		    os.writeBlob(_replyHdr);
		    
		    //
		    // Add the request ID.
		    //
		    os.writeInt(requestId);
		}
		
		in.invoke(servantManager);
		
		//
		// If there are more invocations, we need the stream back.
		//
		if(--invokeNum > 0)
		{
		    stream.swap(is);
                }

		reclaimIncoming(in);
		in = null;
	    }
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		setState(StateClosed, ex);
	    }
	}
	catch(java.lang.AssertionError ex) // Upon assertion, we print the stack trace.
	{
	    synchronized(this)
	    {
		UnknownException uex = new UnknownException();
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		uex.unknown = sw.toString();
		_logger.error(uex.unknown);
		setState(StateClosed, uex);
	    }
	}
	catch(java.lang.Exception ex)
	{
	    synchronized(this)
	    {
		UnknownException uex = new UnknownException();
		java.io.StringWriter sw = new java.io.StringWriter();
		java.io.PrintWriter pw = new java.io.PrintWriter(sw);
		ex.printStackTrace(pw);
		pw.flush();
		uex.unknown = sw.toString();
		setState(StateClosed, uex);
	    }
	}
	finally
	{
	    if(in != null)
	    {
		reclaimIncoming(in);
	    }
	}
		
	//
	// If invoke() above raised an exception, and therefore
	// neither sendResponse() nor sendNoResponse() has been
	// called, then we must decrement _dispatchCount here.
	//
	if(invokeNum > 0)
	{
	    synchronized(this)
	    {
		assert(_dispatchCount > 0);
		_dispatchCount -= invokeNum;
		assert(_dispatchCount >= 0);
		if(_dispatchCount == 0)
		{
		    notifyAll();
		}
	    }
	}
    }

    private void
    run()
    {
	//
	// For non-datagram connections, the thread-per-connection
	// must validate and activate this connection, and not in the
	// connection factory. Please see the comments in the
	// connection factory for details.
	//
	if(!_endpoint.datagram())
	{
	    try
	    {
		validate();
	    }
	    catch(LocalException ex)
	    {
		synchronized(this)
		{
		    assert(_state == StateClosed);
		    
		    //
		    // We must make sure that nobody is sending when
		    // we close the transceiver.
		    //
		    synchronized(_sendMutex)
		    {
			try
			{
			    _transceiver.close();
			}
			catch(LocalException e)
			{
			    // Here we ignore any exceptions in close().
			}
			
			_transceiver = null;
			notifyAll();
		    }
		}
		return;
	    }
	    
	    activate();
	}

	boolean warnUdp = _instance.properties().getPropertyAsInt("Ice.Warn.Datagrams") > 0;

	boolean closed = false;

	IceInternal.BasicStream stream = new IceInternal.BasicStream(_instance);

	while(!closed)
	{
	    //
	    // We must accept new connections outside the thread
	    // synchronization, because we use blocking accept.
	    //

	    try
	    {
		try
		{
		    stream.resize(IceInternal.Protocol.headerSize, true);
		    stream.pos(0);
		    _transceiver.read(stream, -1);

		    int pos = stream.pos();
		    assert(pos >= IceInternal.Protocol.headerSize);
		    stream.pos(0);
		    byte[] m = stream.readBlob(4);
		    if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
		       m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
		    {
			BadMagicException ex = new BadMagicException();
			ex.badMagic = m;
			throw ex;
		    }
		    byte pMajor = stream.readByte();
		    byte pMinor = stream.readByte();
		    if(pMajor != IceInternal.Protocol.protocolMajor)
		    {
			UnsupportedProtocolException e = new UnsupportedProtocolException();
			e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
			e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
			e.major = IceInternal.Protocol.protocolMajor;
			e.minor = IceInternal.Protocol.protocolMinor;
			throw e;
		    }
		    byte eMajor = stream.readByte();
		    byte eMinor = stream.readByte();
		    if(eMajor != IceInternal.Protocol.encodingMajor)
		    {
			UnsupportedEncodingException e = new UnsupportedEncodingException();
			e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
			e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
			e.major = IceInternal.Protocol.encodingMajor;
			e.minor = IceInternal.Protocol.encodingMinor;
			throw e;
		    }
		    byte messageType = stream.readByte();
		    byte compress = stream.readByte();
		    int size = stream.readInt();
		    if(size < IceInternal.Protocol.headerSize)
		    {
			throw new IllegalMessageSizeException();
		    }
		    if(size > _instance.messageSizeMax())
		    {
			throw new MemoryLimitException();
		    }
		    if(size > stream.size())
		    {
			stream.resize(size, true);
		    }
		    stream.pos(pos);

		    if(pos != stream.size())
		    {
			if(_endpoint.datagram())
			{
			    if(warnUdp)
			    {
				_logger.warning("DatagramLimitException: maximum size of " + pos + " exceeded");
			    }
			    throw new DatagramLimitException();
			}
			else
			{
			    _transceiver.read(stream, -1);
			    assert(stream.pos() == stream.size());
			}
		    }
		}
		catch(DatagramLimitException ex) // Expected.
		{
		    continue;
		}
		catch(LocalException ex)
		{
		    exception(ex);
		}

		MessageInfo info = new MessageInfo(stream);

		LocalException exception = null;

		IceInternal.IntMap requests = null;
		IceInternal.IntMap asyncRequests = null;

		synchronized(this)
		{
		    while(_state == StateHolding)
		    {
			try
			{
			    wait();
			}
			catch(InterruptedException ex)
			{
			}
		    }
		
		    if(_state != StateClosed)
		    {
			parseMessage(info);
		    }

		    //
		    // parseMessage() can close the connection, so we must
		    // check for closed state again.
		    //
		    if(_state == StateClosed)
		    {
			//
			// We must make sure that nobody is sending when we close
			// the transceiver.
			//
			synchronized(_sendMutex)
			{
			    try
			    {
				_transceiver.close();
			    }
			    catch(LocalException ex)
			    {
				exception = ex;
			    }
			    
			    _transceiver = null;
			    notifyAll();
			}

			//
			// We cannot simply return here. We have to make sure
			// that all requests (regular and async) are notified
			// about the closed connection below.
			//
			closed = true;
		    }

		    if(_state == StateClosed || _state == StateClosing)
		    {
			requests = _requests;
			_requests = new IceInternal.IntMap();

			asyncRequests = _asyncRequests;
			_asyncRequests = new IceInternal.IntMap();
		    }
		}

		//
		// Asynchronous replies must be handled outside the thread
		// synchronization, so that nested calls are possible.
		//
		if(info.outAsync != null)
		{
		    info.outAsync.__finished(info.stream);
		}
		
		//
		// Method invocation (or multiple invocations for batch messages)
		// must be done outside the thread synchronization, so that nested
		// calls are possible.
		//
		invokeAll(info.stream, info.invokeNum, info.requestId, info.compress, info.servantManager,
			  info.adapter);

		if(requests != null)
		{
		    java.util.Iterator i = requests.entryIterator();
		    while(i.hasNext())
		    {
			IceInternal.IntMap.Entry e = (IceInternal.IntMap.Entry)i.next();
			IceInternal.Outgoing out = (IceInternal.Outgoing)e.getValue();
			out.finished(_exception); // The exception is immutable at this point.
		    }
		}

		if(asyncRequests != null)
		{
		    java.util.Iterator i = asyncRequests.entryIterator();
		    while(i.hasNext())
		    {
			IceInternal.IntMap.Entry e = (IceInternal.IntMap.Entry)i.next();
			IceInternal.OutgoingAsync out = (IceInternal.OutgoingAsync)e.getValue();
			out.__finished(_exception); // The exception is immutable at this point.
		    }
		}

		if(exception != null)
		{
		    assert(closed);
		    throw exception;
		}    
	    }
	    finally
	    {
		stream.reset();
	    }
	}
    }

    private void
    warning(String msg, Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	ex.printStackTrace(pw);
        pw.flush();
        String s = msg + ":\n" + sw.toString() + _desc;
        _logger.warning(s);
    }

    private void
    error(String msg, Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	ex.printStackTrace(pw);
        pw.flush();
        String s = msg + ":\n" + _desc + sw.toString();
        _logger.error(s);
    }

    private IceInternal.Incoming
    getIncoming(ObjectAdapter adapter, boolean response, byte compress)
    {
        IceInternal.Incoming in = null;

        synchronized(_incomingCacheMutex)
        {
            if(_incomingCache == null)
            {
                in = new IceInternal.Incoming(_instance, this, adapter, response, compress);
            }
            else
            {
                in = _incomingCache;
                _incomingCache = _incomingCache.next;
		in.reset(_instance, this, adapter, response, compress);
                in.next = null;
            }
        }

        return in;
    }

    private void
    reclaimIncoming(IceInternal.Incoming in)
    {
        synchronized(_incomingCacheMutex)
        {
	    in.next = _incomingCache;
	    _incomingCache = in;
	    //
	    // Clear references to Ice objects as soon as possible.
	    //
	    _incomingCache.reclaim();
        }
    }

    private class ThreadPerConnection extends Thread
    {
	public void
	run()
	{
	    try
	    {
		ConnectionI.this.run();
	    }
	    catch(Exception ex)
	    {
		ConnectionI.this.error("exception in thread per connection", ex);
	    }
	}
    }
    private Thread _threadPerConnection;

    private IceInternal.Transceiver _transceiver;
    private final String _desc;
    private final String _type;
    private final IceInternal.Endpoint _endpoint;

    private ObjectAdapter _adapter;
    private IceInternal.ServantManager _servantManager;

    private final Logger _logger;
    private final IceInternal.TraceLevels _traceLevels;

    private boolean _registeredWithPool;
    private final IceInternal.ThreadPool _threadPool;

    private final boolean _warn;

    private final int _acmTimeout;
    private long _acmAbsoluteTimeoutMillis;

    private int _nextRequestId;

    private IceInternal.IntMap _requests = new IceInternal.IntMap();
    private IceInternal.IntMap _asyncRequests = new IceInternal.IntMap();

    private LocalException _exception;

    private IceInternal.BasicStream _batchStream;
    private boolean _batchStreamInUse;
    private int _batchRequestNum;
    private boolean _batchRequestCompress;

    private int _dispatchCount;

    private int _state; // The current state.
    private long _stateTime; // The last time when the state was changed.

    //
    // We have a separate mutex for sending, so that we don't block
    // the whole connection when we do a blocking send.
    //
    private java.lang.Object _sendMutex = new java.lang.Object();

    private IceInternal.Incoming _incomingCache;
    private java.lang.Object _incomingCacheMutex = new java.lang.Object();

    private static boolean _compressionSupported = IceInternal.BasicStream.compressible();

    private boolean _overrideCompress;
    private boolean _overrideCompressValue;
}
