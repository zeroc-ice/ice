// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Threading;

    public sealed class ConnectionI : IceInternal.EventHandler, Connection
    {
        public int ice_hash() // From LocalObject.
        {
            return GetHashCode();
        }

        public object Clone() // From LocalObject.
        {
            return MemberwiseClone();
        }

	public void validate()
	{
	    lock(this)
	    {
		Debug.Assert(_state == StateNotValidated);

		if(!endpoint().datagram()) // Datagram connections are always implicitly validated.
		{
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

			if(_adapter != null)
			{
			    lock(_sendMutex)
			    {
				//
				// Incoming connections play the active role with
				// respect to connection validation.
				//
				IceInternal.BasicStream os = new IceInternal.BasicStream(_instance);
				os.writeByte(IceInternal.Protocol.magic[0]);
				os.writeByte(IceInternal.Protocol.magic[1]);
				os.writeByte(IceInternal.Protocol.magic[2]);
				os.writeByte(IceInternal.Protocol.magic[3]);
				os.writeByte(IceInternal.Protocol.protocolMajor);
				os.writeByte(IceInternal.Protocol.protocolMinor);
				os.writeByte(IceInternal.Protocol.encodingMajor);
				os.writeByte(IceInternal.Protocol.encodingMinor);
				os.writeByte(IceInternal.Protocol.validateConnectionMsg);
				os.writeByte((byte)0); // Compression status (always zero for validate connection).
				os.writeInt(IceInternal.Protocol.headerSize); // Message size.
				IceInternal.TraceUtil.traceHeader("sending validate connection", os, _logger, 
								  _traceLevels);
				try
				{
				    _transceiver.write(os, timeout);
				}
				catch(TimeoutException)
				{
				    throw new ConnectTimeoutException("Connect timed out after " + timeout + " msec");
				}
			    }
			}
			else
			{
			    //
			    // Outgoing connections play the passive role with
			    // respect to connection validation.
			    //
			    IceInternal.BasicStream ins = new IceInternal.BasicStream(_instance);
			    ins.resize(IceInternal.Protocol.headerSize, true);
			    ins.pos(0);
			    try
			    {
			        _transceiver.read(ins, timeout);
			    }
			    catch(TimeoutException)
			    {
			        throw new ConnectTimeoutException("Connect timed out after " + timeout + " msec");
			    }
			    Debug.Assert(ins.pos() == IceInternal.Protocol.headerSize);
			    ins.pos(0);
			    byte[] m = new byte[4];
			    m[0] = ins.readByte();
			    m[1] = ins.readByte();
			    m[2] = ins.readByte();
			    m[3] = ins.readByte();
			    if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
			       m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
			    {
				BadMagicException ex = new BadMagicException();
				ex.badMagic = m;
				throw ex;
			    }
			    byte pMajor = ins.readByte();
			    byte pMinor = ins.readByte();
			    if(pMajor != IceInternal.Protocol.protocolMajor)
			    {
				UnsupportedProtocolException e = new UnsupportedProtocolException();
				e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
				e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
				e.major = IceInternal.Protocol.protocolMajor;
				e.minor = IceInternal.Protocol.protocolMinor;
				throw e;
			    }
			    byte eMajor = ins.readByte();
			    byte eMinor = ins.readByte();
			    if(eMajor != IceInternal.Protocol.encodingMajor)
			    {
				UnsupportedEncodingException e = new UnsupportedEncodingException();
				e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
				e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
				e.major = IceInternal.Protocol.encodingMajor;
				e.minor = IceInternal.Protocol.encodingMinor;
				throw e;
			    }
			    byte messageType = ins.readByte();
			    if(messageType != IceInternal.Protocol.validateConnectionMsg)
			    {
				throw new ConnectionNotValidatedException();
			    }
			    byte compress = ins.readByte(); // Ignore compression status for validate connection.
			    int size = ins.readInt();
			    if(size != IceInternal.Protocol.headerSize)
			    {
				throw new IllegalMessageSizeException();
			    }
			    IceInternal.TraceUtil.traceHeader("received validate connection", ins, _logger, 
							      _traceLevels);
			}
		    }
		    catch(LocalException ex)
		    {
			setState(StateClosed, ex);
			Debug.Assert(_exception != null);
			throw _exception;
		    }
		}
		
		if(_acmTimeout > 0)
		{
		    long _acmAbsolutetimoutMillis = System.DateTime.Now.Ticks / 10 + _acmTimeout * 1000;
		}

		//
		// We start out in holding state.
		//
		setState(StateHolding);
	    }
	}
	
	public void activate()
	{
	    lock(this)
	    {
		setState(StateActive);
	    }
	}
	
	public void hold()
	{
	    lock(this)
	    {
		setState(StateHolding);
	    }
	}
	
	// DestructionReason.
	public const int ObjectAdapterDeactivated = 0;
	public const int CommunicatorDestroyed = 1;
	
	public void destroy(int reason)
	{
	    lock(this)
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
	}
	
	public void close(bool force)
	{
	    lock(this)
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
	}

	public bool isValidated()
	{
	    lock(this)
	    {
		return _state > StateNotValidated;
	    }
	}

	public bool isDestroyed()
	{
	    lock(this)
	    {
		return _state >= StateClosing;
	    }
	}

	public bool isFinished()
	{
	    lock(this)
	    {
		if(_transceiver == null && _dispatchCount == 0)
		{
		    //
		    // We must destroy the incoming cache. It is now not
		    // needed anymore.
		    //
		    lock(_incomingCacheMutex)
		    {
			while(_incomingCache != null)
			{
			    _incomingCache.__destroy();
			    _incomingCache = _incomingCache.next;
			}
		    }

		    return true;
		}
		else
		{
		    return false;
		}
	    }
	}

	public void waitUntilHolding()
	{
	    lock(this)
	    {
		while(_state < StateHolding || _dispatchCount > 0)
		{
		    Monitor.Wait(this);
		}
	    }
	}
	
	public void waitUntilFinished()
	{
	    lock(this)
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
		    Monitor.Wait(this);
		}
		
		//
		// Now we must wait until close() has been called on the
		// transceiver.
		//
		while(_transceiver != null)
		{
		    if(_state != StateClosed && _endpoint.timeout() >= 0)
		    {
			long absoluteWaitTime = _stateTime + _endpoint.timeout();
			int waitTime = (int)(absoluteWaitTime - System.DateTime.Now.Ticks / 10);
			
			if(waitTime > 0)
			{
			    //
			    // We must wait a bit longer until we close
			    // this connection.
			    //
			    Monitor.Wait(this, waitTime);
			    if(System.DateTime.Now.Ticks / 10 >= absoluteWaitTime)
			    {
				setState(StateClosed, new CloseTimeoutException());
			    }
			}
			else
			{
			    //
			    // We already waited long enough, so let's
			    // close this connection!
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
			Monitor.Wait(this);
		    }
		}
	    }

	    Debug.Assert(_state == StateClosed);

	    //
	    // We must destroy the incoming cache. It is now not
	    // needed anymore.
	    //
	    lock(_incomingCacheMutex)
	    {
		while(_incomingCache != null)
		{
		    _incomingCache.__destroy();
		    _incomingCache = _incomingCache.next;
		}
	    }
	}
	
	public void monitor()
	{
	    lock(this)
	    {
		if(_state != StateActive)
		{
		    return;
		}
		
		//
		// Check for timed out async requests.
		//
		foreach(IceInternal.OutgoingAsync og in _asyncRequests.Values)
		{
		    if(og.__timedOut())
		    {
			setState(StateClosed, new TimeoutException());
			return;
		    }
		}
		
		//
		// Active connection management for idle connections.
		//
		//
		if(_acmTimeout > 0 &&
		    _requests.Count == 0 && _asyncRequests.Count == 0 &&
		    !_batchStreamInUse && _batchStream.isEmpty() &&
		    _dispatchCount == 0)
		{
		    if(System.DateTime.Now.Ticks / 10 >= _acmAbsoluteTimeoutMillis)
		    {
			setState(StateClosing, new ConnectionTimeoutException());
			return;
		    }
		}
	    }
	}
	
	private static readonly byte[] _requestHdr = new byte[]
	{
	    IceInternal.Protocol.magic[0], IceInternal.Protocol.magic[1], IceInternal.Protocol.magic[2], 
	    IceInternal.Protocol.magic[3],
	    IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor,
	    IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor,
	    IceInternal.Protocol.requestMsg,
	    (byte)0, // Compression status.
	    (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
	    (byte)0, (byte)0, (byte)0, (byte)0	// Request ID (placeholder).
	};
	
	//
	// TODO: Should not be a member function of ConnectionI.
	//
	public void prepareRequest(IceInternal.BasicStream os)
	{
	    os.writeBlob(_requestHdr);
	}

	private IceInternal.BasicStream doCompress(IceInternal.BasicStream uncompressed, bool compress)
	{
	    if(_compressionSupported)
	    {
		if(compress && uncompressed.size() >= 100)
		{
		    //
		    // Do compression.
		    //
		    IceInternal.BasicStream cstream = null;
		    if(uncompressed.compress(ref cstream, IceInternal.Protocol.headerSize))
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
	
	public void sendRequest(IceInternal.BasicStream os, IceInternal.Outgoing og, bool compress)
	{
	    int requestId = 0;
	    IceInternal.BasicStream stream;

	    lock(this)
	    {
		Debug.Assert(!(og != null && _endpoint.datagram())); // Twoway requests cannot be datagrams.

		if(_exception != null)
		{
		    throw _exception;
		}

		Debug.Assert(_state > StateNotValidated);
		Debug.Assert(_state < StateClosing);

		//
		// Only add to the request map if this is a twoway call.
		//
		if(og != null)
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
		    // Add ot the requests map.
		    //
		    _requests[requestId] = og;
		}

		stream = doCompress(os, _overrideCompress ? _overrideCompressValue : compress);
		
		if(_acmTimeout > 0)
		{
		    _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10 + _acmTimeout * 1000;
		}
	    }
	    
	    try
	    {
		lock(_sendMutex)
		{
		    if(_transceiver == null) // Has the transceiver already been closed?
		    {
			Debug.Assert(_exception != null);
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
		lock(this)
		{
		    setState(StateClosed, ex);
		    Debug.Assert(_exception != null);

		    if(og != null)
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
			IceInternal.Outgoing o = (IceInternal.Outgoing)_requests[requestId];
			_requests.Remove(requestId);
			if(o != null)
			{
			    Debug.Assert(o == og);
			    throw _exception;
			}				
		    }
		    else
		    {
			throw _exception;
		    }
		}
	    }
	    finally
	    {
	        if(!Object.ReferenceEquals(os, stream))
		{
		    stream.destroy();
		}
	    }
	}
	
	public void sendAsyncRequest(IceInternal.BasicStream os, IceInternal.OutgoingAsync og, bool compress)
	{
	    int requestId = 0;
	    IceInternal.BasicStream stream;

	    lock(this)
	    {
		Debug.Assert(!_endpoint.datagram()); // Twoway requests cannot be datagrams, and async implies twoway.

		if(_exception != null)
		{
		    throw _exception;
		}

		Debug.Assert(_state > StateNotValidated);
		Debug.Assert(_state < StateClosing);

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
		_asyncRequests[requestId] = og;
		    
		stream = doCompress(os, _overrideCompress ? _overrideCompressValue : compress);

		if(_acmTimeout > 0)
		{
		    _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10	+ _acmTimeout * 1000;
		}
	    }

	    try
	    {
		lock(_sendMutex)
		{
		    if(_transceiver == null) // Has the transceiver already been closed?
		    {
			Debug.Assert(_exception != null);
			throw _exception; // The exception is imuutable at this point.
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
		lock(this)
		{
		    setState(StateClosed, ex);
		    Debug.Assert(_exception != null);
		    
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
		    IceInternal.OutgoingAsync o = (IceInternal.OutgoingAsync)_asyncRequests[requestId];
		    _asyncRequests.Remove(requestId);
		    if(o != null)
		    {
			Debug.Assert(o == og);
			throw _exception;
		    }
		}
	    }
	    finally
	    {
	        if(!Object.ReferenceEquals(os, stream))
		{
		    stream.destroy();
		}
	    }
	}
	
	private static readonly byte[] _requestBatchHdr = new byte[]
	{
	    IceInternal.Protocol.magic[0], IceInternal.Protocol.magic[1], IceInternal.Protocol.magic[2],
	    IceInternal.Protocol.magic[3],
	    IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor,
	    IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor,
	    IceInternal.Protocol.requestBatchMsg,
	    (byte)0, // Compression status.
	    (byte)0, (byte)0, (byte)0, (byte)0, // Message size (placeholder).
	    (byte)0, (byte)0, (byte)0, (byte)0	// Number of requests in batch (placeholder).
	};
	
	public void prepareBatchRequest(IceInternal.BasicStream os)
	{
	    lock(this)
	    {
		while(_batchStreamInUse && _exception == null)
		{
		    Monitor.Wait(this);
		}
		
		if(_exception != null)
		{
		    throw _exception;
		}

		Debug.Assert(_state > StateNotValidated);
		Debug.Assert(_state < StateClosing);
		
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
		// finishBatchRequest() or abortBatchRequest() is
		// called.
		//
	    }
	}
	
	public void finishBatchRequest(IceInternal.BasicStream os, bool compress)
	{
	    lock(this)
	    {
		//
		// Get the batch stream back and increment the number of
		// requests in the batch.
		//
		_batchStream.swap(os);
		++_batchRequestNum;

		//
		// We compress the whole batch if there is at least
		// one compressed message.
		//
		if(compress)
		{
		    _batchRequestCompress = true;
		}

		//
		// Notify about the batch stream not being in use anymore.
		//
		Debug.Assert(_batchStreamInUse);
		_batchStreamInUse = false;
		Monitor.PulseAll(this);
	    }
	}
	
	public void abortBatchRequest()
	{
	    lock(this)
	    {
		//
		// Destroy and reset the batch stream and batch
		// count. We cannot safe old requests in the batch
		// stream, as they might be corrupted due to
		// incomplete marshaling.
		//
		_batchStream.destroy();
		_batchStream = new IceInternal.BasicStream(_instance);
		_batchRequestNum = 0;
		_batchRequestCompress = false;
		
		//
		// Notify about the batch stream not being in use
		// anymore.
		//
		Debug.Assert(_batchStreamInUse);
		_batchStreamInUse = false;
		Monitor.PulseAll(this);
	    }
	}

	public void flushBatchRequests()
	{
	    IceInternal.BasicStream stream;

	    lock(this)
	    {
		while(_batchStreamInUse && _exception == null)
		{
		    Monitor.Wait(this);
		}
		
		if(_exception != null)
		{
		    throw _exception;
		}

		if(_batchStream.isEmpty())
		{
		    return; // Nothing to do.
		}

		Debug.Assert(_state > StateNotValidated);
		Debug.Assert(_state < StateClosing);
		
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
		    _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10 + _acmTimeout * 1000;
		}

		//
		// Prevent that new batch requests are added while we are
		// flushing.
		//
		_batchStreamInUse = true;
	    }

	    try
	    {
		lock(_sendMutex)
		{
		    if(_transceiver == null) // Has the transceiver already been closed?
		    {
			Debug.Assert(_exception != null);
			throw _exception; // The exception is immutable at this point.
		    }

		    //
		    // Send the batch request.
		    //
		    IceInternal.TraceUtil.traceBatchRequest("sending batch request", _batchStream, _logger,
							    _traceLevels);
		    _transceiver.write(stream, _endpoint.timeout());
		}
	    }
	    catch(LocalException ex)
	    {
		lock(this)
		{
		    setState(StateClosed, ex);
		    Debug.Assert(_exception != null);

		    //
		    // Since batch requests area all oneways (or datarams), we
		    // must report the exception to the caller.
		    //
		    throw _exception;
		}
	    }
	    finally
	    {
	        if(!Object.ReferenceEquals(_batchStream, stream))
		{
		    stream.destroy();
		}
	    }

	    lock(this)
	    {
		//
		// Reset the batch stream, and notify that flushing is over.
		//
		_batchStream.destroy();
		_batchStream = new IceInternal.BasicStream(_instance);
		_batchRequestNum = 0;
		_batchRequestCompress = false;
		_batchStreamInUse = false;
		Monitor.PulseAll(this);
	    }
	}

	public ObjectPrx createProxy(Identity ident)
	{
	    lock(this)
	    {
		//
		// Create a reference and return a reverse proxy for this
		// reference.
		//
		IceInternal.Endpoint[] endpoints = new IceInternal.Endpoint[0];
		ConnectionI[] connections = new ConnectionI[1];
		connections[0] = this;
		IceInternal.Reference @ref = _instance.referenceFactory().create(ident, new Context(), "",
										 IceInternal.Reference.Mode.ModeTwoway,
										 connections);
		return _instance.proxyFactory().referenceToProxy(@ref);
	    }
	}
	
	public void sendResponse(IceInternal.BasicStream os, byte compress)
	{
	    IceInternal.BasicStream stream = null;
	    try
	    {
		lock(_sendMutex)
		{
		    if(_transceiver == null) // Has the transceiver already been closed?
		    {
			Debug.Assert(_exception != null);
			throw _exception; // The exception is immutable at this point.
		    }

		    stream = doCompress(os, compress != 0);

		    //
		    // Send the reply.
		    //
		    IceInternal.TraceUtil.traceReply("sending reply", os, _logger, _traceLevels);
		    _transceiver.write(stream, _endpoint.timeout());
		}
	    }
	    catch(LocalException ex)
	    {
		lock(this)
		{
		    setState(StateClosed, ex);
		}
	    }
	    finally
	    {
	        if(stream != null && !Object.ReferenceEquals(os, stream))
		{
		    stream.destroy();
		}
	    }

	    lock(this)
	    {
		Debug.Assert(_state > StateNotValidated);

		try
		{
		    if(--_dispatchCount == 0)
		    {
			Monitor.PulseAll(this);
		    }

		    if(_state == StateClosing && _dispatchCount == 0)
		    {
			initiateShutdown();
		    }
		
		    if(_acmTimeout > 0)
		    {
			_acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10 + _acmTimeout * 1000;
		    }
		}
		catch(LocalException ex)
		{
		    setState(StateClosed, ex);
		}
	    }
	}
	
	public void sendNoResponse()
	{
	    Debug.Assert(_state > StateNotValidated);

	    lock(this)
	    {
		try
		{
		    if(--_dispatchCount == 0)
		    {
			Monitor.PulseAll(this);
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
	
	public IceInternal.Endpoint endpoint()
	{
	    // No mutex protection necessary, _endpoint is immutable.
	    return _endpoint;
	}

	public void setAdapter(ObjectAdapter adapter)
	{
	    lock(this)
	    {
		if(_exception != null)
		{
		    throw _exception;
		}
		
		Debug.Assert(_state < StateClosing);

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
			Monitor.Wait(this);
		    }

		    //
		    // We never change the thread pool with which we were
		    // initially registered, even if we add or remove an object
		    // adapter.
		    //
		    
		    _adapter = adapter;
		    if(_adapter != null)
		    {
			_servantManager = ((ObjectAdapterI) _adapter).getServantManager();
		    }
		    else
		    {
			_servantManager = null;
		    }
		}
	    }
	}

	public ObjectAdapter getAdapter()
	{
	    lock(this)
	    {
		return _adapter;
	    }
	}
	
	//
	// Operations from EventHandler
	//
	
	public override bool datagram()
	{
	    return _endpoint.datagram();
	}
	
	public override bool readable()
	{
	    return true;
	}
	
	public override void read(IceInternal.BasicStream stream)
	{
	    if(_transceiver != null)
	    {
		_transceiver.read(stream, 0);
	    }
	    
	    //
	    // Updating _acmAbsoluteTimeoutMillis is to expensive here,
	    // because we would have to acquire a lock just for this
	    // purpose. Instead, we update _acmAbsoluteTimeoutMillis in
	    // message().
	    //
	}
	
	private static readonly byte[] _replyHdr = new byte[]
	{
	    IceInternal.Protocol.magic[0], IceInternal.Protocol.magic[1], IceInternal.Protocol.magic[2], 
	    IceInternal.Protocol.magic[3],
	    IceInternal.Protocol.protocolMajor, IceInternal.Protocol.protocolMinor,
	    IceInternal.Protocol.encodingMajor, IceInternal.Protocol.encodingMinor,
	    IceInternal.Protocol.replyMsg,
	    (byte)0, // Compression status.
	    (byte)0, (byte)0, (byte)0, (byte)0 // Message size (placeholder).
	};
	
	public override void message(IceInternal.BasicStream stream, IceInternal.ThreadPool threadPool)
	{
	    byte compress = 0;
	    int requestId = 0;
	    int invokeNum = 0;
	    IceInternal.ServantManager servantManager = null;
	    ObjectAdapter adapter = null;
	    IceInternal.OutgoingAsync outAsync = null;

	    lock(this)
	    {
		//
		// We must promote with the synchronization, otherwise
		// there could be various race conditions with close
		// connection messages and other messages.
		//
		threadPool.promoteFollower();
		
		Debug.Assert(_state > StateNotValidated);

		if(_state == StateClosed)
		{
		    return;
		}
		
		if(_acmTimeout > 0)
		{
		    _acmAbsoluteTimeoutMillis = System.DateTime.Now.Ticks / 10 + _acmTimeout * 1000;
		}
		
		try
		{
		    //
		    // We don't need to check magic and version here. This
		    // has already been done by the ThreadPool, which
		    // provides us the stream.
		    //
		    Debug.Assert(stream.pos() == stream.size());
		    stream.pos(8);
		    byte messageType = stream.readByte();
		    compress = stream.readByte();
		    if(compress == (byte)2)
		    {
			if(_compressionSupported)
			{
			    IceInternal.BasicStream uncompressedStream
				= stream.uncompress(IceInternal.Protocol.headerSize);
			    stream.destroy();
			    stream = uncompressedStream;
			}
			else
			{
			    throw new CompressionNotSupportedException();
			}
		    }
		    stream.pos(IceInternal.Protocol.headerSize);
		    
		    switch(messageType)
		    {
			case IceInternal.Protocol.closeConnectionMsg:
			{
			    IceInternal.TraceUtil.traceHeader("received close connection", stream, _logger, 
							      _traceLevels);
			    if(_endpoint.datagram() && _warn)
			    {
				_logger.warning("ignoring close connection message for datagram connection:\n"
						+ _desc);
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
				IceInternal.TraceUtil.traceRequest("received request during closing\n"
								   + "(ignored by server, client will retry)",
								   stream, _logger, _traceLevels);
			    }
			    else
			    {
				IceInternal.TraceUtil.traceRequest("received request", stream, _logger, _traceLevels);
				requestId = stream.readInt();
				invokeNum = 1;
				servantManager = _servantManager;
				adapter = _adapter;
				++_dispatchCount;
			    }
			    break;
			}
			
			case IceInternal.Protocol.requestBatchMsg: 
			{
			    if(_state == StateClosing)
			    {
				IceInternal.TraceUtil.traceBatchRequest("received batch request during closing\n"
									+ "(ignored by server, client will retry)",
									stream, _logger, _traceLevels);
			    }
			    else
			    {
				IceInternal.TraceUtil.traceBatchRequest("received batch request", stream, _logger, 
									_traceLevels);
				invokeNum = stream.readInt();
				if(invokeNum < 0)
				{
				    throw new NegativeSizeException();
				}
				servantManager = _servantManager;
				adapter = _adapter;
				_dispatchCount += invokeNum;
			    }
			    break;
			}
			
			case IceInternal.Protocol.replyMsg: 
			{
			    IceInternal.TraceUtil.traceReply("received reply", stream, _logger, _traceLevels);
			    requestId = stream.readInt();
			    IceInternal.Outgoing og = (IceInternal.Outgoing)_requests[requestId];
			    _requests.Remove(requestId);
			    if(og != null)
			    {
				og.finished(stream);
			    }
			    else
			    {
				outAsync = (IceInternal.OutgoingAsync)_asyncRequests[requestId];
				_asyncRequests.Remove(requestId);
				if(outAsync == null)
				{
				    throw new UnknownRequestIdException();
				}
			    }
			    break;
			}
			
			case IceInternal.Protocol.validateConnectionMsg: 
			{
			    IceInternal.TraceUtil.traceHeader("received validate connection", stream, _logger, 
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
							      "(invalid, closing connection)",
							      stream, _logger, _traceLevels);
			    throw new UnknownMessageException();
			}
		    }
		}
		catch(LocalException ex)
		{
		    setState(StateClosed, ex);
		    return;
		}
	    }
	    
	    //
	    // Asynchronous replies must be handled outside the thread
	    // synchronization, so that nested calls are possible.
	    //
	    if(outAsync != null)
	    {
		outAsync.__finished(stream);
	    }
	    
	    //
	    // Method invocation (or multiple invocations for batch messages)
	    // must be done outside the thread synchronization, so that nested
	    // calls are possible.
	    //
	    IceInternal.Incoming inc = null;
	    try
	    {
		while(invokeNum > 0)
		{
		    //
		    // Prepare the invocation.
		    //
		    bool response = !_endpoint.datagram() && requestId != 0;
		    inc = getIncoming(adapter, response, compress);
		    IceInternal.BasicStream ins = inc.istr();
		    stream.swap(ins);
		    IceInternal.BasicStream os = inc.ostr();
		    
		    //
		    // Prepare the response if necessary.
		    //
		    if(response)
		    {
			Debug.Assert(invokeNum == 1); // No further invocations if a response is expected.
			os.writeBlob(_replyHdr);
			
			//
			// Fill in the request ID.
			//
			os.writeInt(requestId);
		    }
		    
		    inc.invoke(servantManager);
		    
		    //
		    // If there are more invocations, we need the stream back.
		    //
		    if(--invokeNum > 0)
		    {
			stream.swap(ins);
		    }
		    
		    reclaimIncoming(inc);
		    inc = null;
		}
	    }
	    catch(LocalException ex)
	    {
		lock(this)
		{
		    setState(StateClosed, ex);
		}
	    }
	    catch(System.Exception ex)
	    {
		lock(this)
		{
		    UnknownException uex = new UnknownException();
		    uex.unknown = ex.ToString();
		    setState(StateClosed, uex);
		}
	    }
	    finally
	    {
		if(inc != null)
		{
		    reclaimIncoming(inc);
		}
	    }
	    
	    //
	    // If invoke() above raised an exception, and therefore
	    // neither sendResponse() nor sendNoResponse() has been
	    // called, then we must decrement _dispatchCount here.
	    //
	    if(invokeNum > 0)
	    {
		lock(this)
		{
		    Debug.Assert(_dispatchCount > 0);
		    _dispatchCount -= invokeNum;
		    Debug.Assert(_dispatchCount >= 0);
		    if(_dispatchCount == 0)
		    {
			Monitor.PulseAll(this);
		    }
		}
	    }
	}

	public override void finished(IceInternal.ThreadPool threadPool)
	{
	    threadPool.promoteFollower();
	    
	    LocalException exception = null;

	    Hashtable requests = null;
	    Hashtable asyncRequests = null;
	    IceInternal.Incoming inc = null;

	    lock(this)
	    {
		if(_state == StateActive || _state == StateClosing)
		{
		    registerWithPool();
		}
		else if(_state == StateClosed)
		{
		    //
		    // We must make sure that nobody is sending when we
		    // close the transeiver.
		    //
		    lock(_sendMutex)
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
			Monitor.PulseAll(this);
		    }
		}

		if(_state == StateClosed || _state == StateClosing)
		{
		    requests = _requests;
		    _requests = new Hashtable();

		    asyncRequests = _asyncRequests;
		    _asyncRequests = new Hashtable();
		}
	    }

	    while(inc != null)
	    {
		inc.__destroy();
		inc = inc.next;
	    }

	    if(requests != null)
	    {
		foreach(IceInternal.Outgoing og in requests.Values)
		{
		    og.finished(_exception); // The exception is immutable at this point.
		}
	    }

	    if(asyncRequests != null)
	    {
		foreach(IceInternal.OutgoingAsync og in asyncRequests.Values)
		{
		    og.__finished(_exception); // The exception is immutable at this point.
		}
	    }

	    if(exception != null)
	    {
		throw exception;
	    }
	}
	
	public override void exception(LocalException ex)
	{
	    lock(this)
	    {
		setState(StateClosed, ex);
	    }
	}

	public string type()
	{
	    return _type; // No mutex lock, _type is immutable.
	}

	public int timeout()
	{
	    return _endpoint.timeout(); // No mutex protection necessary, _endpoint is immutable.
	}
	
	public string _Ice_toString()
	{
	    return ToString();
	}

	public override string ToString()
	{
	    return _desc; // No mutex lock, _desc is immutable.
	}
	
	static ConnectionI()
	{
	    _compressionSupported = IceInternal.BasicStream.compressible();
	}

	internal ConnectionI(IceInternal.Instance instance, IceInternal.Transceiver transceiver,
			    IceInternal.Endpoint endpoint, ObjectAdapter adapter)
	    : base(instance)
	{
	    _transceiver = transceiver;
	    _desc = transceiver.ToString();
	    _type = transceiver.type();
	    _endpoint = endpoint;
	    _adapter = adapter;
	    _logger = instance.logger(); // Cached for better performance.
	    _traceLevels = instance.traceLevels(); // Cached for better performance.
	    _registeredWithPool = false;
	    _warn = _instance.properties().getPropertyAsInt("Ice.Warn.Connections") > 0;
	    _acmTimeout = _endpoint.datagram() ? 0 : _instance.connectionIdleTime();
	    _acmAbsoluteTimeoutMillis = 0;
	    _nextRequestId = 1;
	    _batchStream = new IceInternal.BasicStream(instance);
	    _batchStreamInUse = false;
	    _batchRequestNum = 0;
	    _batchRequestCompress = false;
	    _dispatchCount = 0;
	    _state = StateNotValidated;
	    _stateTime = System.DateTime.Now.Ticks / 10;
	    
	    try
	    {
	        if(_adapter != null)
                {
		    _threadPool = ((ObjectAdapterI) _adapter).getThreadPool();
		    _servantManager = ((ObjectAdapterI) _adapter).getServantManager();
		}
		else
                {
		    _threadPool = _instance.clientThreadPool();
		    _servantManager = null;
		}
	    }
	    catch(System.Exception ex)
	    {
		string s = "cannot create thread pool for connection:\n" + ex;
		_instance.logger().error(s);

		_state = StateClosed;
		try
		{
		    _transceiver.close();
		}
		catch(LocalException)
		{
		    // Here we ignore any exceptions in close().
		}
		_transceiver = null;

		throw new Ice.SyscallException(ex);
	    }

	    _overrideCompress = _instance.defaultsAndOverrides().overrideCompress;
	    _overrideCompressValue = _instance.defaultsAndOverrides().overrideCompressValue;

	}
	
	~ConnectionI()
	{
	    Debug.Assert(_state == StateClosed);
	    Debug.Assert(_transceiver == null);
	    Debug.Assert(_dispatchCount == 0);

	    _batchStream.destroy();
	}
	
	private const int StateNotValidated = 0;
	private const int StateActive = 1;
	private const int StateHolding = 2;
	private const int StateClosing = 3;
	private const int StateClosed = 4;
	
	private void setState(int state, LocalException ex)
	{
	    //
	    // If setState() is called with an exception, then only closed
	    // and closing states are permissible.
	    //
	    Debug.Assert(state == StateClosing || state == StateClosed);

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
			if(!(_exception is CloseConnectionException ||
			      _exception is ConnectionTimeoutException ||
			      _exception is CommunicatorDestroyedException ||
			      _exception is ObjectAdapterDeactivatedException ||
			      (_exception is ConnectionLostException && _state == StateClosing)))
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

	private void setState(int state)
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
		    Debug.Assert(false);
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
		    registerWithPool();
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
		    unregisterWithPool();
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
		    registerWithPool(); // We need to continue to read in closing state.
		    break;
		}
		
		case StateClosed: 
		{
		    //
		    // If we change from not validated, we can close right
		    // away. Otherwise we first must make sure that we are
		    // registered, then we unregister, and let finished()
		    // do the close.
		    //
		    if(_state == StateNotValidated)
		    {
			Debug.Assert(!_registeredWithPool);

			//
			// We must make sure that nobidy is sending when
			// we close the transceiver.
			//
			lock(_sendMutex)
			{
			    try
			    {
				_transceiver.close();
			    }
			    catch(LocalException)
			    {
				// Here we ignore any exceptions in close().
			    }

			    _transceiver = null;
			    //Monitor.PulseAll(); // We notify already below.
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
	    
	    _state = state;
	    _stateTime = System.DateTime.Now.Ticks / 10;
	    Monitor.PulseAll(this);
	    
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
	
	private void initiateShutdown()
	{
	    Debug.Assert(_state == StateClosing);
	    Debug.Assert(_dispatchCount == 0);
	    
	    if(!_endpoint.datagram())
	    {
		lock(_sendMutex)
		{
		    //
		    // Before we shut down, we send a close connection
		    // message.
		    //
		    IceInternal.BasicStream os = new IceInternal.BasicStream(_instance);
		    os.writeByte(IceInternal.Protocol.magic[0]);
		    os.writeByte(IceInternal.Protocol.magic[1]);
		    os.writeByte(IceInternal.Protocol.magic[2]);
		    os.writeByte(IceInternal.Protocol.magic[3]);
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
		    //_transceiver.shutdown();
		}
	    }
	}
	
	private void registerWithPool()
	{
	    if(!_registeredWithPool)
	    {
		_threadPool.register(_transceiver.fd(), this);
		_registeredWithPool = true;
		
		IceInternal.ConnectionMonitor connectionMonitor = _instance.connectionMonitor();
		if(connectionMonitor != null)
		{
		    connectionMonitor.add(this);
		}
	    }
	}
	
	private void unregisterWithPool()
	{
	    if(_registeredWithPool)
	    {
		_threadPool.unregister(_transceiver.fd());
		_registeredWithPool = false;
		
		IceInternal.ConnectionMonitor connectionMonitor = _instance.connectionMonitor();
		if(connectionMonitor != null)
		{
		    connectionMonitor.remove(this);
		}
	    }
	}
	
	private void warning(string msg, System.Exception ex)
	{
	    _logger.warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());
	}
	
	private IceInternal.Incoming getIncoming(ObjectAdapter adapter, bool response, byte compress)
	{
	    IceInternal.Incoming inc = null;
	    
	    lock(_incomingCacheMutex)
	    {
		if(_incomingCache == null)
		{
		    inc = new IceInternal.Incoming(_instance, this, adapter, response, compress);
		}
		else
		{
		    inc = _incomingCache;
		    _incomingCache = _incomingCache.next;
		    inc.next = null;
		    inc.reset(_instance, this, adapter, response, compress);
		}
	    }
	    
	    return inc;
	}
	
	private void reclaimIncoming(IceInternal.Incoming inc)
	{
	    lock(_incomingCacheMutex)
	    {
		inc.next = _incomingCache;
		_incomingCache = inc;
	    }
	}
	
	private bool closingOK()
	{
	    return
		_requests.Count == 0 &&
		_asyncRequests.Count == 0 &&
		!_batchStreamInUse &&
		_batchStream.isEmpty() &&
		_dispatchCount == 0;
	}
	
	private volatile IceInternal.Transceiver _transceiver;
	private volatile string _desc;
	private volatile string _type;
	private volatile IceInternal.Endpoint _endpoint;
	
	private ObjectAdapter _adapter;
	private IceInternal.ServantManager _servantManager;
	
	private volatile Logger _logger;
	private volatile IceInternal.TraceLevels _traceLevels;
	
	private bool _registeredWithPool;
	private IceInternal.ThreadPool _threadPool;
	
	private bool _warn;
	
	private int _acmTimeout;
	private long _acmAbsoluteTimeoutMillis;
	
	private int _nextRequestId;
	private Hashtable _requests = new Hashtable();
	private Hashtable _asyncRequests = new Hashtable();
	
	private LocalException _exception;

	private IceInternal.BasicStream _batchStream;
	private bool _batchStreamInUse;
	private int _batchRequestNum;
	private bool _batchRequestCompress;
	
	private volatile int _dispatchCount;
	
	private volatile int _state; // The current state.
	private long _stateTime; // The last time when the state was changed.
	
	//
	// We have a separate mutex for sending, so that we don't block
	// the whole connection when we do a blocking send.
	//
	private object _sendMutex = new object();

	private IceInternal.Incoming _incomingCache;
	private object _incomingCacheMutex = new object();

	private static volatile bool _compressionSupported;

	private bool _overrideCompress;
	private bool _overrideCompressValue;
    }
}
