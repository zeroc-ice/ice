// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    // TODO: Should not be a member function of Connection.
    //
    public void
    prepareRequest(IceInternal.BasicStream os)
    {
        os.writeBlob(_requestHdr);
    }

    private int
    fillRequestId(IceInternal.BasicStream os)
    {
	//
	// Create a new unique request ID.
	//
	int requestId = _nextRequestId++;
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

	return requestId;
    }

    private void
    sendRequest(IceInternal.BasicStream os)
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
	// Send the request.
	//
	IceInternal.TraceUtil.traceRequest("sending request", os, _logger, _traceLevels);
	_transceiver.write(os, _endpoint.timeout());
    }

    public void
    sendBlockingRequest(IceInternal.BasicStream os, IceInternal.BasicStream is, IceInternal.Outgoing out)
    {
	int requestId = 0;

	synchronized(this)
	{
	    if(_exception != null)
	    {
		throw _exception;
	    }

	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_state > StateNotValidated);
		IceUtil.Debug.Assert(_state < StateClosing);
	    }

	    //
	    // Fill in request id if this is a twoway call.
	    //
	    if(out != null)
	    {
	        requestId = fillRequestId(os);
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
	}

	try
	{
	    synchronized(_sendMutex)
	    {
	        sendRequest(os);

		if(out != null)
		{
		    readStream(is);
		}
	    }

	    synchronized(this)
	    {
		if(_state != StateClosed)
		{
		    MessageInfo info = new MessageInfo(is);
		    parseMessage(info, requestId);
		}

		//
		// parseMessage() can close the connection, so we must
		// check for closed state again.
		//
		if(_state == StateClosed)
		{
		    try
		    {
		        _transceiver.close();
		    }
		    catch(LocalException ex)
		    {
		    }

		    _transceiver = null;
		    throw _exception;
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
		throw _exception;
	    }
	}
    }

    public void
    sendRequest(IceInternal.BasicStream os, IceInternal.Outgoing out)
    {
	int requestId = 0;

	synchronized(this)
	{
	    if(_exception != null)
	    {
		throw _exception;
	    }

	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(_state > StateNotValidated);
		IceUtil.Debug.Assert(_state < StateClosing);
	    }

	    //
	    // Only add to the request map if this is a twoway call.
	    //
	    if(out != null)
	    {
	        requestId = fillRequestId(os);
		_requests.put(requestId, out);
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
	}

	try
	{
	    synchronized(_sendMutex)
	    {
	        sendRequest(os);
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
			if(IceUtil.Debug.ASSERT)
			{
			    IceUtil.Debug.Assert(o == out);
			}
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

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state > StateNotValidated);
	    IceUtil.Debug.Assert(_state < StateClosing);
	}

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
    finishBatchRequest(IceInternal.BasicStream os)
    {
	//
	// Get the batch stream back and increment the number of
	// requests in the batch.
        //
	_batchStream.swap(os);
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

    public void
    flushBatchRequests()
    {
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
	    synchronized(_sendMutex)
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
	    _batchStream = new IceInternal.BasicStream(_instance);
	    _batchRequestNum = 0;
	    _batchStreamInUse = false;
	    notifyAll();
	}
    }

    public void
    sendResponse(IceInternal.BasicStream os)
    {
	try
	{
	    synchronized(_sendMutex)
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

    public boolean
    blocking()
    {
        return _blocking;
    }

    public synchronized void
    setAdapter(ObjectAdapter adapter)
    {
	if(_exception != null)
	{
	    throw _exception;
	}
	
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state < StateClosing);
	}

	_adapter = adapter;

	if(_adapter != null)
	{
	    _servantManager = _adapter.getServantManager();
	}
	else
	{
	    _servantManager = null;
	}

	//
	// We never change the thread pool with which we were
	// initially registered, even if we add or remove an object
	// adapter.
	//
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
        Connection[] connections = new Connection[1];
        connections[0] = this;
        IceInternal.Reference ref = _instance.referenceFactory().create(ident, new java.util.Hashtable(), "",
                                                                        IceInternal.Reference.ModeTwoway, connections);
        return _instance.proxyFactory().referenceToProxy(ref);
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
        _adapter = adapter;
        _logger = instance.logger(); // Cached for better performance.
        _traceLevels = instance.traceLevels(); // Cached for better performance.
	_warn = _instance.properties().getPropertyAsInt("Ice.Warn.Connections") > 0 ? true : false;
        _nextRequestId = 1;
        _batchStream = new IceInternal.BasicStream(instance);
	_batchStreamInUse = false;
	_batchRequestNum = 0;
        _dispatchCount = 0;
        _state = StateNotValidated;
	_stateTime = System.currentTimeMillis();
	_blocking = _instance.properties().getPropertyAsInt("Ice.Blocking") > 0 && _adapter == null;

	if(_adapter != null)
	{
	    _servantManager = _adapter.getServantManager();
	}
	else
	{
	    _servantManager = null;
	}

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
		synchronized(_sendMutex)
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
		// If we are in thread per connection mode, we
		// shutdown both for reading and writing. This will
		// unblock any read call with an exception. The thread
		// per connection then closes the transceiver.
		//
		_transceiver.shutdownReadWrite();
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
            }
            catch(LocalException ex)
            {
                setState(StateClosed, ex);
            }

	    if(_state != StateClosed && _blocking)
	    {
	        try
		{
		    _transceiver.close();
		}
		catch(LocalException ex)
		{
		}
		_transceiver = null;
		_state = StateClosed;
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
	MessageInfo(IceInternal.BasicStream stream)
	{
	    this.stream = stream;
	}

	IceInternal.BasicStream stream;
	int invokeNum;
	int requestId;
	IceInternal.ServantManager servantManager;
	ObjectAdapter adapter;
    }

    private void
    parseMessage(MessageInfo info, int requestId)
    {
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(_state > StateNotValidated && _state < StateClosed);
	}

	try
	{
	    //
	    // We don't need to check magic and version here. This has
	    // already been done by the ThreadPool or ThreadPerConnection,
	    // which provides us with the stream.
	    //
	    if(IceUtil.Debug.ASSERT)
	    {
		IceUtil.Debug.Assert(info.stream.pos() == info.stream.size());
	    }
	    info.stream.pos(8);
	    byte messageType = info.stream.readByte();
	    byte compress = info.stream.readByte();
	    if(compress == (byte)2)
	    {
		FeatureNotSupportedException ex = new FeatureNotSupportedException();
		ex.unsupportedFeature = "compression";
		throw ex;
	    }
	    info.stream.pos(IceInternal.Protocol.headerSize);

	    if(_blocking)
	    {
	        switch(messageType)
	        {
		    case IceInternal.Protocol.closeConnectionMsg:
		    {
		        IceInternal.TraceUtil.traceHeader("received close connection", info.stream, _logger,
							  _traceLevels);
		        setState(StateClosed, new CloseConnectionException());
		        break;
		    }

		    case IceInternal.Protocol.replyMsg:
		    {
		        IceInternal.TraceUtil.traceReply("received reply", info.stream, _logger, _traceLevels);
		        info.requestId = info.stream.readInt();
			if(info.requestId != requestId)
		        {
		            throw new UnknownRequestIdException();
		        }
		        break;
		    }

		    default:
		    {
		        IceInternal.TraceUtil.traceHeader("received unexpected message\n" +
						          "(invalid, closing connection)", info.stream, _logger,
						          _traceLevels);
		        throw new UnknownMessageException();
		    }
		}
	    }
	    else
	    {
	        switch(messageType)
	        {
		    case IceInternal.Protocol.closeConnectionMsg:
		    {
		        IceInternal.TraceUtil.traceHeader("received close connection", info.stream, _logger,
							  _traceLevels);
		        setState(StateClosed, new CloseConnectionException());
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
		            throw new UnknownRequestIdException();
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
	}
	catch(LocalException ex)
	{
	    setState(StateClosed, ex);
	}
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

    private void
    invokeAll(IceInternal.BasicStream stream, int invokeNum, int requestId,
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
		boolean response = requestId != 0;
		in = getIncoming(adapter, response);
		IceInternal.BasicStream is = in.is();
		stream.swap(is);
		IceInternal.BasicStream os = in.os();
		
		//
		// Prepare the response if necessary.
		//
		if(response)
		{
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(invokeNum == 1); // No further invocations if a response is expected.
		    }
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
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(_dispatchCount > 0);
		}
		_dispatchCount -= invokeNum;
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
	        readStream(stream);

		MessageInfo info = new MessageInfo(stream);

		LocalException exception = null;

		IceInternal.IntMap requests = null;

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
			parseMessage(info, -1);
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
			// that all requests are notified about the closed
			// connection below.
			//
			closed = true;
		    }

		    if(_state == StateClosed || _state == StateClosing)
		    {
			requests = _requests;
			_requests = new IceInternal.IntMap();
		    }
		}

		//
		// Method invocation (or multiple invocations for batch messages)
		// must be done outside the thread synchronization, so that nested
		// calls are possible.
		//
		invokeAll(info.stream, info.invokeNum, info.requestId, info.servantManager, info.adapter);

		if(requests != null)
		{
		    java.util.Enumeration i = requests.elements();
		    while(i.hasMoreElements())
		    {
			IceInternal.IntMap.Entry e = (IceInternal.IntMap.Entry)i.nextElement();
			IceInternal.Outgoing out = (IceInternal.Outgoing)e.getValue();
			out.finished(_exception); // The exception is immutable at this point.
		    }
		}

		if(exception != null)
		{
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(closed);
		    }
		    throw exception;
		}
	    }
	    finally
	    {
		stream.reset();
	    }
	}
    }

    public void
    warning(String msg, Exception ex)
    {
        String s = msg + ":\n" + ex.toString() + _desc;
        _logger.warning(s);
    }

    public void
    error(String msg, Exception ex)
    {
        String s = msg + ":\n" + _desc + ex.toString();
        _logger.error(s);
    }

    private void
    readStream(IceInternal.BasicStream stream)
    {
	try
	{
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
	        _transceiver.read(stream, _blocking ? _endpoint.timeout() : -1);
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(stream.pos() == stream.size());
		}
	    }
	}
	catch(LocalException ex)
	{
	    exception(ex);
	}
    }

    private IceInternal.Incoming
    getIncoming(ObjectAdapter adapter, boolean response)
    {
        IceInternal.Incoming in = null;

        synchronized(_incomingCacheMutex)
        {
            if(_incomingCache == null)
            {
                in = new IceInternal.Incoming(_instance, this, adapter, response);
            }
            else
            {
                in = _incomingCache;
                _incomingCache = _incomingCache.next;
                in.next = null;
                in.reset(_instance, this, adapter, response);
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
        }
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

    private ObjectAdapter _adapter;
    private IceInternal.ServantManager _servantManager;

    private /*final*/ Logger _logger;
    private /*final*/ IceInternal.TraceLevels _traceLevels;

    private /*final*/ boolean _warn;

    private int _nextRequestId;

    private IceInternal.IntMap _requests = new IceInternal.IntMap();

    private LocalException _exception;

    private IceInternal.BasicStream _batchStream;
    private boolean _batchStreamInUse;
    private int _batchRequestNum;

    private int _dispatchCount;

    private int _state; // The current state.
    private long _stateTime; // The last time when the state was changed.

    private boolean _blocking;

    //
    // We have a separate mutex for sending, so that we don't block
    // the whole connection when we do a blocking send.
    //
    private java.lang.Object _sendMutex = new java.lang.Object();

    private IceInternal.Incoming _incomingCache;
    private java.lang.Object _incomingCacheMutex = new java.lang.Object();

    private IceInternal.Outgoing _outgoingCache;
    private java.lang.Object _outgoingCacheMutex = new java.lang.Object();
}
