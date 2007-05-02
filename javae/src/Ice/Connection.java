// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class Connection
{

    synchronized public void
    waitForValidation()
    {
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
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_exception != null);
	    }
	    throw _exception;
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
	    //
	    // If we do a graceful shutdown, then we wait until all
	    // outstanding requests have been completed. Otherwise,
	    // the CloseConnectionException will cause all outstanding
	    // requests to be retried, regardless of whether the
	    // server has processed them or not.
	    //
	    while(!_requests.isEmpty())
	    {
		try
		{
		    wait();
		}
		catch(InterruptedException ex)
		{
		}
	    }

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
	       (_threadPerConnection != null && _threadPerConnection.isAlive()))
	    {
		return false;
	    }

	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_state == StateClosed);
	    }

	    threadPerConnection = _threadPerConnection;
	    _threadPerConnection = null;
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
    throwException()
    {
        if(_exception != null)
	{
	    IceUtil.Debug.Assert(_state >= StateClosing);
	    throw _exception;
	}
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

	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_state == StateClosed);
	    }

	    threadPerConnection = _threadPerConnection;
	    _threadPerConnection = null;
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

    public void
    sendRequest(IceInternal.BasicStream os, IceInternal.Outgoing out)
        throws IceInternal.LocalExceptionWrapper
    {
	boolean requestSent = false;
	try
	{
	    synchronized(_sendMonitor)
	    {
		if(_transceiver == null)
		{
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(_exception != null);

		    }
		    throw new IceInternal.LocalExceptionWrapper(_exception, true);
		}

		int requestId = 0;
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

		    _requests.put(requestId, out);
		}

		//
		// Fill in the message size.
		//
		os.pos(10);
		os.writeInt(os.size());
		
		//
		// Send the request.
		//
		IceInternal.TraceUtil.traceRequest("sending request", os, _logger, _traceLevels);
		_transceiver.write(os, _endpoint.timeout());
		requestSent = true;
		
		if(out == null)
		{
		    return;
		}
		
		if(_blocking)
		{
		    //
		    // Re-use the stream for reading the reply.
		    //
		    os.reset();
			
		    //
		    // Read the reply.
		    //
		    MessageInfo info = new MessageInfo();
		    readStreamAndParseMessage(os, info);
		    if(info.invokeNum > 0)
		    {
			Ice.Util.throwUnknownMessageException();
		    }
		    else if(info.requestId != requestId)
		    {
			Ice.Util.throwUnknownRequestIdException();
		    }
			
		    out.finished(os);
		}
		else
		{
		    //
		    // Wait until the request has completed, or until the
		    // request times out.
		    //
		    int tout = timeout();
		    long expireTime = 0;
		    if(tout > 0)
		    {
			expireTime = System.currentTimeMillis() + tout;
		    }
			
		    while(out.state() == IceInternal.Outgoing.StateInProgress)
		    {
			try
			{
			    if(tout > 0)
			    {
				long now = System.currentTimeMillis();
				if(now < expireTime)
				{
				    _sendMonitor.wait(expireTime - now);
				}
				    
				//
				// Make sure we woke up because of timeout and not another response.
				//
				if(out.state() == IceInternal.Outgoing.StateInProgress &&  
				   System.currentTimeMillis() > expireTime)
				{
				    throw new TimeoutException();
				}
			    }
			    else
			    {
				_sendMonitor.wait();
			    }
			}
			catch(InterruptedException ex)
			{
			}
		    }
		}
	    }
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		setState(StateClosed, ex);
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_exception != null);
		}
		if(!requestSent)
		{
		    throw _exception;
		}
	    }

	    //
	    // If the request was already sent, we don't throw
	    // directly but instead we set the Outgoing object
	    // exception with finished().  Throwing directly would
	    // break "at-most-once" (see also comment in
	    // Outgoing.invoke())
	    //
	    synchronized(_sendMonitor)
	    {
		if(_blocking)
		{
		    out.finished(ex);
		}
		else
		{
		    // Wait for the connection thread to propagate the exception
		    // to the Outgoing object.
		    while(out.state() == IceInternal.Outgoing.StateInProgress)
		    {
			try
			{
			    _sendMonitor.wait();
			}
			catch(java.lang.InterruptedException e)
			{
			}
		    }
		}
	    }
	}
    }

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

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state > StateNotValidated);
	    IceUtil.Debug.Assert(_state < StateClosing);
	}

        if(_batchStream.isEmpty())
        {
	    try
	    {
		_batchStream.writeBlob(IceInternal.Protocol.requestBatchHdr);
	    }
	    catch(LocalException ex)
	    {
		setState(StateClosed, ex);
		throw ex;
	    }
        }

        _batchStreamInUse = true;
        _batchMarker = _batchStream.size();
	_batchStream.swap(os);

	//
	// The batch stream now belongs to the caller, until
	// finishBatchRequest() or abortBatchRequest() is called.
	//
    }

    public void
    finishBatchRequest(IceInternal.BasicStream os)
    {
        boolean autoflush = false;
        byte[] lastRequest = null;

        synchronized(this)
        {
            //
            // Get the batch stream back.
            //
            _batchStream.swap(os);

            if(_batchAutoFlush)
            {
                synchronized(_sendMonitor)
                {
                    if(_transceiver == null)
                    {
                        if(IceUtil.Debug.ASSERT)
                        {
                            IceUtil.Debug.Assert(_exception != null);
                        }
                        throw _exception; // The exception is immutable at this point.
                    }
                    //
                    // Throw memory limit exception if the first  message added causes us 
                    // to go over limit. Otherwise put aside the marshalled message that
                    // caused limit to be exceeded and  rollback stream to the marker. 
                    //
                    if(_batchStream.size() > _instance.messageSizeMax())
                    {
                        if(_batchRequestNum == 0)
                        {
                            resetBatch(true);
                            throw new Ice.MemoryLimitException();
                        }

                        lastRequest = new byte[_batchStream.size() - _batchMarker];
                        IceInternal.ByteBuffer buffer = _batchStream.prepareRead();
                        buffer.position(_batchMarker);
                        buffer.get(lastRequest);
                        _batchStream.resize(_batchMarker, false);
                        autoflush = true;
                    }
                }
            }

            if(!autoflush)
            {
                //
                // Increment the number of requests in the batch.
                //
                ++_batchRequestNum;

                //
                // Notify about the batch stream not being in use anymore.
                //
	        if(IceUtil.Debug.ASSERT)
	        {
	            IceUtil.Debug.Assert(_batchStreamInUse);
	        }
                _batchStreamInUse = false;
                notifyAll();
            }
        }

        if(autoflush)
        {
            //
            // We have to keep _batchStreamInUse set until after we insert the
            // saved marshalled data into a new stream.
            //
            flushBatchRequestsInternal(true);

            synchronized(this)
            {
                //
                // Throw memory limit exception if the message that caused us to go over
                // limit causes us to exceed the limit by itself.
                //
                if(IceInternal.Protocol.requestBatchHdr.length + lastRequest.length >  _instance.messageSizeMax())
                {
                    resetBatch(true);
                    throw new MemoryLimitException();
                }

                //
                // Start a new batch with the last message that caused us to
                // go over the limit.
                //
                try
                {
                    _batchStream.writeBlob(IceInternal.Protocol.requestBatchHdr);
                    _batchStream.writeBlob(lastRequest);
                }
                catch(LocalException ex)
                {
                    setState(StateClosed, ex);
                    throw ex;
                }

                //
                // Notify that the batch stream not in use anymore.
                //
                ++_batchRequestNum;
                _batchStreamInUse = false;
                notifyAll();
            }
        }
    }

    public synchronized void
    abortBatchRequest()
    {
        //
        // Reset the batch stream. We cannot save old requests
        // in the batch stream, as they might be corrupted due to
        // incomplete marshaling.
        //
        resetBatch(true);
    }

    public void
    flushBatchRequests()
    {
        flushBatchRequestsInternal(false);
    }

    private void
    flushBatchRequestsInternal(boolean ignoreInUse)
    {
	synchronized(this)
	{
            if(!ignoreInUse)
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
	    }
	    
	    if(_exception != null)
	    {
		throw _exception;
	    }

	    if(_batchStream.isEmpty())
	    {
		return; // Nothing to do.
	    }

	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_state > StateNotValidated);
		IceUtil.Debug.Assert(_state < StateClosing);
	    }

	    //
	    // Fill in the message size.
	    //
	    _batchStream.pos(10);
	    _batchStream.writeInt(_batchStream.size());

	    //
	    // Fill in the number of requests in the batch.
	    //
	    _batchStream.writeInt(_batchRequestNum);

	    //
	    // Compression not supported.
	    //
	    _batchStream.pos(9);
	    _batchStream.writeByte((byte)(0));

	    //
	    // Prevent that new batch requests are added while we are
	    // flushing.
	    //
	    _batchStreamInUse = true;
	}

	try
	{
	    synchronized(_sendMonitor)
	    {
		if(_transceiver == null) // Has the transceiver already been closed?
		{
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(_exception != null);
		    }
		    throw _exception; // The exception is immutable at this point.
		}

		//
		// Send the batch request.
		//
		IceInternal.TraceUtil.traceBatchRequest("sending batch request", _batchStream, _logger, _traceLevels);
		_transceiver.write(_batchStream, _endpoint.timeout());
	    }
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		setState(StateClosed, ex);
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_exception != null);
		}
		
		//
		// Since batch requests are all oneways, we
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
            resetBatch(!ignoreInUse);
	}
    }

    private void
    resetBatch(boolean resetInUse)
    {
        _batchStream = new IceInternal.BasicStream(_instance, _batchAutoFlush);
        _batchRequestNum = 0;

        //
        // Notify about the batch stream not being in use anymore.
        //
        if(resetInUse)
        {
            if(IceUtil.Debug.ASSERT)
            {
                IceUtil.Debug.Assert(_batchStreamInUse);
            }
            _batchStreamInUse = false;
            notifyAll();
        }
    }

    public void
    sendResponse(IceInternal.BasicStream os)
    {
	try
	{
	    synchronized(_sendMonitor)
	    {
		if(_transceiver == null) // Has the transceiver already been closed?
		{
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(_exception != null);
		    }
		    throw _exception; // The exception is immutable at this point.
		}

	        //
	        // Compression not supported.
	        //
	        os.pos(9);
	        os.writeByte((byte)(0));

	        //
	        // Fill in the message size.
	        //
	        os.pos(10);
	        os.writeInt(os.size());

		//
		// Send the reply.
		//
		IceInternal.TraceUtil.traceReply("sending reply", os, _logger, _traceLevels);
		_transceiver.write(os, _endpoint.timeout());
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
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_state > StateNotValidated);
	    }

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
    }

    public synchronized void
    sendNoResponse()
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state > StateNotValidated);
	}
	
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
        if(_blocking)
	{
	    FeatureNotSupportedException ex = new FeatureNotSupportedException();
	    ex.unsupportedFeature = "setAdapter with blocking connection";
	    throw ex;
	}

	//
	// Wait for all the incoming to be dispatched (to be consistent
	// with IceE).
	//
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
	
	if(_exception != null)
	{
	    throw _exception;
	}
	
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state < StateClosing);
	}

	_in.setAdapter(adapter);
    }

    public synchronized ObjectAdapter
    getAdapter()
    {
	return _in.getAdapter();
    }

    public synchronized ObjectPrx
    createProxy(Identity ident)
    {
        //
        // Create a reference and return a reverse proxy for this
        // reference.
        //
        Connection[] connections = new Connection[1];
        connections[0] = this;
        IceInternal.Reference ref = 
            _instance.referenceFactory().create(ident, null, "", IceInternal.Reference.ModeTwoway, connections);
        return _instance.proxyFactory().referenceToProxy(ref);
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
	return _desc; // No mutex lock, _desc is immutable.
    }

    public Connection(IceInternal.Instance instance, IceInternal.Transceiver transceiver, 
		      IceInternal.Endpoint endpoint, ObjectAdapter adapter)
    {
        _instance = instance;
        _transceiver = transceiver;
	_desc = transceiver.toString();
        _type = transceiver.type();
        _endpoint = endpoint;
        _logger = instance.initializationData().logger; // Cached for better performance.
        _traceLevels = instance.traceLevels(); // Cached for better performance.
	_warn = _instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
        _nextRequestId = 1;
        _batchAutoFlush = _instance.initializationData().properties.getPropertyAsIntWithDefault(
                "Ice.BatchAutoFlush", 1) > 0 ? true : false;
        _batchStream = new IceInternal.BasicStream(instance, _batchAutoFlush);
	_batchStreamInUse = false;
	_batchRequestNum = 0;
        _dispatchCount = 0;
        _state = StateNotValidated;
	_stateTime = System.currentTimeMillis();
	_blocking = _instance.initializationData().properties.getPropertyAsInt("Ice.Blocking") > 0 && adapter == null;
	_stream = new IceInternal.BasicStream(_instance);
	_in = new IceInternal.Incoming(_instance, this, _stream, adapter);

	if(_blocking)
	{
	    validate();
	}
	else
	{
	    try
	    {
	        //
	        // If we are in thread per connection mode, create the thread
	        // for this connection.
	        //
	        _threadPerConnection = new ThreadPerConnection(this);
	        _threadPerConnection.start();
	    }
	    catch(java.lang.Exception ex)
	    {
	        ex.printStackTrace();
	        String s = "cannot create thread for connection:\n";;
	        s += ex.toString();
	        _logger.error(s);
	    
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
	}
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
	IceUtil.Debug.FinalizerAssert(_state == StateClosed);
	IceUtil.Debug.FinalizerAssert(_transceiver == null);
	IceUtil.Debug.FinalizerAssert(_dispatchCount == 0);
	IceUtil.Debug.FinalizerAssert(_threadPerConnection == null);
    }

    private static final int StateNotValidated = 0;
    private static final int StateActive = 1;
    private static final int StateHolding = 2;
    private static final int StateClosing = 3;
    private static final int StateClosed = 4;

    private void
    validate()
    {
	boolean active;
	    
	synchronized(this)
	{
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_state == StateNotValidated || _state == StateClosed);
	    }
	    if(_state == StateClosed)
	    {
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_exception != null);
		}
		throw _exception;
	    }
		
	    if(_in.getAdapter() != null)
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
		synchronized(_sendMonitor)
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
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(is.pos() == IceInternal.Protocol.headerSize);
		}
	        is.pos(0);
	        byte[] m = is.readBlob(4);
	        if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
	           m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
	        {
                    Ice.Util.throwBadMagicException(m);
	        }
	        byte pMajor = is.readByte();
	        byte pMinor = is.readByte();
	        if(pMajor != IceInternal.Protocol.protocolMajor)
	        {
                    Ice.Util.throwUnsupportedProtocolException(pMajor, pMinor);
	        }
	        byte eMajor = is.readByte();
	        byte eMinor = is.readByte();
	        if(eMajor != IceInternal.Protocol.encodingMajor)
	        {
                    Ice.Util.throwUnsupportedEncodingException(eMajor, eMinor);
	        }
	        byte messageType = is.readByte();
	        if(messageType != IceInternal.Protocol.validateConnectionMsg)
	        {
	    	    Ice.Util.throwConnectionNotValidatedException();
	        }
	        byte compress = is.readByte(); // Ignore compression status for validate connection.
	        int size = is.readInt();
	        if(size != IceInternal.Protocol.headerSize)
	        {
	    	    Ice.Util.throwIllegalMessageSizeException();
	        }
	        IceInternal.TraceUtil.traceHeader("received validate connection", is, _logger, _traceLevels);
	    }
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		setState(StateClosed, ex);
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_exception != null);
		}
		throw _exception;
	    }
	}
	
	synchronized(this)
	{
	    //
	    // We start out in holding state.
	    //
	    setState(StateHolding);
	}
    }

    private void
    setState(int state, LocalException ex)
    {
	//
	// If setState() is called with an exception, then only closed
	// and closing states are permissible.
	//
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(state == StateClosing || state == StateClosed);
	}

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
			 _exception instanceof ForcedCloseConnectionException ||
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
	// Skip graceful shutdown if we are destroyed before validation.
	//
	if(_state == StateNotValidated && state == StateClosing)
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
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(false);
		}
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
                break;
            }
	    
            case StateClosed:
            {
		//
		// We shutdown both for reading and writing. This will
		// unblock and read call with an exception. The thread
		// per connection then closes the transceiver.
		//
		_transceiver.shutdownReadWrite();

		//
		// In blocking mode, we close the transceiver now.
		//
		if(_blocking)
		{
		    synchronized(_sendMonitor)
		    {
			try
			{
			    _transceiver.close();
			}
			catch(Ice.LocalException ex)
			{
			}
			_transceiver = null;
		    }
		}
		break;
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
		if(_blocking)
		{
		    setState(StateClosed);
		}
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
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state == StateClosing);
	    IceUtil.Debug.Assert(_dispatchCount == 0);
	}

	synchronized(_sendMonitor)
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
	    os.writeByte((byte)0); // Compression not supported.
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

    private static class MessageInfo
    {
	int invokeNum;
	int requestId;
    }

    private void
    readStreamAndParseMessage(IceInternal.BasicStream stream, MessageInfo info)
    {
	//
	// Read the header.
	//
	stream.resize(IceInternal.Protocol.headerSize, true);
	stream.pos(0);
	_transceiver.read(stream, _blocking ? _endpoint.timeout() : -1);
	
	int pos = stream.pos();
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(pos >= IceInternal.Protocol.headerSize);
	}
	stream.pos(0);
	byte[] m = stream.readBlob(4);
	if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
	   m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
	{
            Ice.Util.throwBadMagicException(m);
	}
	byte pMajor = stream.readByte();
	byte pMinor = stream.readByte();
	if(pMajor != IceInternal.Protocol.protocolMajor)
	{
            Ice.Util.throwUnsupportedProtocolException(pMajor, pMinor);
	}
	byte eMajor = stream.readByte();
	byte eMinor = stream.readByte();
	if(eMajor != IceInternal.Protocol.encodingMajor)
	{
            Ice.Util.throwUnsupportedEncodingException(eMajor, eMinor);
	}
	byte messageType = stream.readByte();
	byte compress = stream.readByte();
	if(compress == (byte)2)
	{
	    FeatureNotSupportedException ex = new FeatureNotSupportedException();
	    ex.unsupportedFeature = "compression";
	    throw ex;
	}

	int size = stream.readInt();
	if(size < IceInternal.Protocol.headerSize)
	{
	    Ice.Util.throwIllegalMessageSizeException();
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

	//
	// Read the rest of the message.
	//
	if(pos != stream.size())
	{
	    _transceiver.read(stream, _blocking ? _endpoint.timeout() : -1);
	}
	
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(stream.pos() == stream.size());
	}
	stream.pos(IceInternal.Protocol.headerSize);

	switch(messageType)
	{
	case IceInternal.Protocol.closeConnectionMsg:
	{
	    IceInternal.TraceUtil.traceHeader("received close connection", stream, _logger, _traceLevels);
	    throw new CloseConnectionException();
	}

	case IceInternal.Protocol.replyMsg:
	{
	    IceInternal.TraceUtil.traceReply("received reply", stream, _logger, _traceLevels);
	    info.requestId = stream.readInt();
	    break;
	}

	case IceInternal.Protocol.requestMsg:
	{
	    IceInternal.TraceUtil.traceRequest("received request", stream, _logger, _traceLevels);
	    info.requestId = stream.readInt();
	    info.invokeNum = 1;
	    break;
	}

	case IceInternal.Protocol.requestBatchMsg:
	{
	    IceInternal.TraceUtil.traceBatchRequest("received batch request", stream, _logger, _traceLevels);
	    info.invokeNum = stream.readInt();
	    if(info.invokeNum < 0)
	    {
		info.invokeNum = 0;
		Ice.Util.throwNegativeSizeException();
	    }
	    break;
	}
	
	case IceInternal.Protocol.validateConnectionMsg:
	{
	    IceInternal.TraceUtil.traceHeader("received validate connection", stream, _logger, _traceLevels);
	    if(_warn)
	    {
		_logger.warning("ignoring unexpected validate connection message:\n" + _desc);
	    }
	    break;
	}

	default:
	{
	    IceInternal.TraceUtil.traceHeader("received unexpected message\n" +
					      "(invalid, closing connection)", stream, _logger,
					      _traceLevels);
	    Ice.Util.throwUnknownMessageException();
	}
	}
    }

    public void
    run()
    {
	//
	// The thread-per-connection must validate and activate this connection,
	// and not in the connection factory. Please see the comments in the
	// connection factory for details.
	//
	try
	{
	    validate();
	}
	catch(LocalException ex)
	{
	    synchronized(this)
	    {
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_state == StateClosed);
		}
	        
	        //
	        // We must make sure that nobody is sending when
	        // we close the transceiver.
	        //
	        synchronized(_sendMonitor)
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

	boolean closed = false;

	MessageInfo info = new MessageInfo();

	while(!closed)
	{
	    info.requestId = 0;
	    info.invokeNum = 0;
	    _in.os().reset();
	    _in.is().reset();
	    
	    //
	    // Read and parse the next message. We don't need to lock the
	    // send monitor here as we have the guarantee that
	    // _transceiver won't be set to 0 by another thread, the
	    // thread per connection is the only thread that can set
	    // _transceiver to 0.
	    //
	    try
	    {
		readStreamAndParseMessage(_stream, info);
	    }
	    catch(Ice.LocalException ex)
	    {
		synchronized(this)
		{
		    setState(StateClosed, ex);
		}
	    }
	    
	    synchronized(this)
	    {
		if(_state != StateClosed)
		{
		    if(info.invokeNum > 0) // We received a request or a batch request
		    {
			if(_state == StateClosing)
			{
			    IceInternal.TraceUtil.traceRequest(
				"received " + (info.invokeNum > 1 ? "batch request" : "request") + " during closing\n"+
				"(ignored by server, client will retry)", _stream, _logger, _traceLevels);
			    info.invokeNum = 0;
			}
			_dispatchCount += info.invokeNum;
		    }
		    else if(info.requestId > 0)
		    {
			try
			{
			    synchronized(_sendMonitor)
			    {
				IceInternal.Outgoing out = (IceInternal.Outgoing)_requests.remove(info.requestId);
				if(out != null)
				{
				    out.finished(_stream);
				    _sendMonitor.notifyAll(); // Wake up threads waiting in sendRequest()
				}
				else
				{
				    Ice.Util.throwUnknownRequestIdException();
				}
			    }
			}
			catch(Ice.LocalException ex)
			{
			    setState(StateClosed, ex);
			}
		    }
		}

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
		
		if(_state == StateClosed)
		{
		    //
		    // We must make sure that nobody is sending when we close
		    // the transceiver.
		    //
		    synchronized(_sendMonitor)
		    {
			try
			{
			    _transceiver.close();
			}
			catch(LocalException ex)
			{
			}
			
			_transceiver = null;
			notifyAll();
		    }
		    
		    //
		    // We cannot simply return here. We have to make sure
		    // that all requests are notified about the closed
		    // connection below.
		    //
		    closed = true;
		}
		
		if(_state == StateClosed || _state == StateClosing)
		{
		    synchronized(_sendMonitor)
		    {
			java.util.Enumeration i = _requests.elements();
			while(i.hasMoreElements())
			{
			    IceInternal.IntMap.Entry e = (IceInternal.IntMap.Entry)i.nextElement();
			    IceInternal.Outgoing out = (IceInternal.Outgoing)e.getValue();
			    out.finished(_exception); // The exception is immutable at this point.
			}
			_requests.clear();
			_sendMonitor.notifyAll(); // Wake up threads waiting in sendRequest()
		    }
		}
	    }
	    
	    //
	    // Method invocation (or multiple invocations for batch messages)
	    // must be done outside the thread synchronization, so that nested
	    // calls are possible.
	    //
	    try
	    {
		for(; info.invokeNum > 0; --info.invokeNum)
		{
		    //
		    // Prepare the response if necessary.
		    //
		    final boolean response = info.requestId != 0;
		    if(response)
		    {
			if(IceUtil.Debug.ASSERT)
			{
			    // No further invocations if a response is expected.
			    IceUtil.Debug.Assert(info.invokeNum == 1);
			}

			//
			// Add the reply header and request id.
			//
			IceInternal.BasicStream os = _in.os();
			os.writeBlob(IceInternal.Protocol.replyHdr);
			os.writeInt(info.requestId);
		    }
		
		    _in.invoke(response, info.requestId);
		}
	    }
	    catch(LocalException ex)
	    {
		synchronized(this)
		{
		    setState(StateClosed, ex);
		}
	    }
	    catch(IceUtil.AssertionError ex) // Upon assertion, we print the stack trace.
	    {
		synchronized(this)
		{
		    UnknownException uex = new UnknownException();
		    uex.unknown = ex.toString();
		    _logger.error(uex.unknown);
		    setState(StateClosed, uex);
		}
	    }
	    catch(java.lang.Exception ex)
	    {
		synchronized(this)
		{
		    UnknownException uex = new UnknownException();
		    uex.unknown = ex.toString();
		    setState(StateClosed, uex);
		}
	    }
	    
	    //
	    // If invoke() above raised an exception, and therefore
	    // neither sendResponse() nor sendNoResponse() has been
	    // called, then we must decrement _dispatchCount here.
	    //
	    if(info.invokeNum > 0)
	    {
		synchronized(this)
		{
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(_dispatchCount > 0);
		    }
		    _dispatchCount -= info.invokeNum;
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(_dispatchCount >= 0);
		    }
		    if(_dispatchCount == 0)
		    {
			notifyAll();
		    }
		}
	    }
	}
    }

    public void
    warning(String msg, Exception ex)
    {
        String s = msg + ":\n" + ex.toString() + "\n" + _desc;
        _logger.warning(s);
    }

    public void
    error(String msg, Exception ex)
    {
        String s = msg + ":\n" + ex.toString() + "\n" + _desc;
        _logger.error(s);
    }

    public IceInternal.Outgoing
    getOutgoing(IceInternal.Reference reference, String operation, OperationMode mode, java.util.Hashtable context)
    {
	IceInternal.Outgoing out;

	synchronized(_outgoingCacheMutex)
	{
	    if(_outgoingCache == null)
	    {
		out = new IceInternal.Outgoing(this, reference, operation, mode, context);
	    }
	    else
	    {
		out = _outgoingCache;
		_outgoingCache = _outgoingCache.next;
		out.reset(reference, operation, mode, context);
		out.next = null;
	    }
	}

	return out;
    }

    public void
    reclaimOutgoing(IceInternal.Outgoing out)
    {
	synchronized(_outgoingCacheMutex)
	{
	    out.next = _outgoingCache;
	    _outgoingCache = out;
	}
    }

    private class ThreadPerConnection extends Thread
    {
	ThreadPerConnection(Connection connection)
	{
	    _connection = connection;
	}

	public void
	run()
	{
	    try
	    {
		_connection.run();
	    }
	    catch(Exception ex)
	    {
		_connection.error("exception in thread per connection", ex);
	    }
	}

	Connection _connection;
    }
    private Thread _threadPerConnection;

    private IceInternal.Instance _instance;
    private IceInternal.Transceiver _transceiver;
    private /*final*/ String _desc;
    private /*final*/ String _type;
    private /*final*/ IceInternal.Endpoint _endpoint;

    private /*final*/ IceInternal.BasicStream _stream;
    private /*final*/ IceInternal.Incoming _in;

    private /*final*/ Logger _logger;
    private /*final*/ IceInternal.TraceLevels _traceLevels;

    private /*final*/ boolean _warn;

    private LocalException _exception;

    private boolean _batchAutoFlush;
    private IceInternal.BasicStream _batchStream;
    private boolean _batchStreamInUse;
    private int _batchRequestNum;
    private int _batchMarker;

    private int _dispatchCount;

    private int _state; // The current state.
    private long _stateTime; // The last time when the state was changed.

    private boolean _blocking;

    //
    // We have a separate mutex for sending, so that we don't block
    // the whole connection when we do a blocking send.  The monitor
    // is also used by outgoing calls to wait for replies when thread
    // per connection is used. The _nextRequestId, _requests and
    // _requestsHint attributes are also protected by this monitor.
    // Calls on the (non thread-safe) Outgoing objects should also
    // only be made with this monitor locked.
    //
    // Finally, it's safe to lock the _sendMonitor with the connection
    // already locked. The contrary isn't permitted.
    //
    private java.lang.Object _sendMonitor = new java.lang.Object();
    private int _nextRequestId;
    private IceInternal.IntMap _requests = new IceInternal.IntMap();

    private IceInternal.Outgoing _outgoingCache;
    private java.lang.Object _outgoingCacheMutex = new java.lang.Object();
}
