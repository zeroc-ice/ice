// **********************************************************************
//
// Copyright (c) 2003
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

namespace IceInternal
{

    using System.Collections;
    using System.Diagnostics;

    public sealed class Connection : EventHandler
    {
	public void validate()
	{
	    lock(this)
	    {
		if(_exception != null)
		{
		    throw _exception;
		}
		
		if(_state != StateNotValidated)
		{
		    return;
		}
		
		if(!_endpoint.datagram()) // Datagram connections are always implicitly validated.
		{
		    try
		    {
			if(_adapter != null)
			{
			    //
			    // Incoming connections play the active role with
			    // respect to connection validation.
			    //
			    BasicStream os = new BasicStream(_instance);
			    os.writeByte(Protocol.magic[0]);
			    os.writeByte(Protocol.magic[1]);
			    os.writeByte(Protocol.magic[2]);
			    os.writeByte(Protocol.magic[3]);
			    os.writeByte(Protocol.protocolMajor);
			    os.writeByte(Protocol.protocolMinor);
			    os.writeByte(Protocol.encodingMajor);
			    os.writeByte(Protocol.encodingMinor);
			    os.writeByte(Protocol.validateConnectionMsg);
			    os.writeByte((byte)0); // Compression status.
			    os.writeInt(Protocol.headerSize); // Message size.
			    TraceUtil.traceHeader("sending validate connection", os, _logger, _traceLevels);
			    _transceiver.write(os, _endpoint.timeout());
			}
			else
			{
			    //
			    // Outgoing connections play the passive role with
			    // respect to connection validation.
			    //
			    BasicStream ins = new BasicStream(_instance);
			    ins.resize(Protocol.headerSize, true);
			    ins.pos(0);
			    _transceiver.read(ins, _endpoint.timeout());
			    int pos = ins.pos();
			    Debug.Assert(pos >= Protocol.headerSize);
			    ins.pos(0);
			    byte[] m = new byte[4];
			    m[0] = ins.readByte();
			    m[1] = ins.readByte();
			    m[2] = ins.readByte();
			    m[3] = ins.readByte();
			    if(m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
			       m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
			    {
				Ice.BadMagicException ex = new Ice.BadMagicException();
				ex.badMagic = new Ice.ByteSeq(m);
				throw ex;
			    }
			    byte pMajor = ins.readByte();
			    byte pMinor = ins.readByte();
			    
			    //
			    // We only check the major version number
			    // here. The minor version number is irrelevant --
			    // no matter what minor version number is offered
			    // by the server, we can be certain that the
			    // server supports at least minor version 0.  As
			    // the client, we are obliged to never produce a
			    // message with a minor version number that is
			    // larger than what the server can understand, but
			    // we don't care if the server understands more
			    // than we do.
			    //
			    // Note: Once we add minor versions, we need to
			    // modify the client side to never produce a
			    // message with a minor number that is greater
			    // than what the server can handle. Similarly, the
			    // server side will have to be modified so it
			    // never replies with a minor version that is
			    // greater than what the client can handle.
			    //
			    if(pMajor != Protocol.protocolMajor)
			    {
				Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
				e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
				e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
				e.major = Protocol.protocolMajor;
				e.minor = Protocol.protocolMinor;
				throw e;
			    }
			    
			    byte eMajor = ins.readByte();
			    byte eMinor = ins.readByte();
			    
			    //
			    // The same applies here as above -- only the
			    // major version number of the encoding is
			    // relevant.
			    //
			    if(eMajor != Protocol.encodingMajor)
			    {
				Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
				e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
				e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
				e.major = Protocol.encodingMajor;
				e.minor = Protocol.encodingMinor;
				throw e;
			    }
			    
			    byte messageType = ins.readByte();
			    if(messageType != Protocol.validateConnectionMsg)
			    {
				throw new Ice.ConnectionNotValidatedException();
			    }
			    
			    byte compress = ins.readByte();
			    if(compress == (byte)2)
			    {
				throw new Ice.CompressionNotSupportedException();
			    }
			    
			    int size = ins.readInt();
			    if(size != Protocol.headerSize)
			    {
				throw new Ice.IllegalMessageSizeException();
			    }
			    TraceUtil.traceHeader("received validate connection", ins, _logger, _traceLevels);
			}
		    }
		    catch(Ice.LocalException ex)
		    {
			setState(StateClosed, ex);
			Debug.Assert(_exception != null);
			throw _exception;
		    }
		}
		
		//
		// We only print warnings after successful connection validation.
		//
		_warn = _instance.properties().getPropertyAsInt("Ice.Warn.Connections") > 0;
		
		//
		// We only use active connection management after successful
		// connection validation. We don't use active connection
		// management for datagram connections at all, because such
		// "virtual connections" cannot be reestablished.
		//
		if(!_endpoint.datagram())
		{
		    _acmTimeout = _instance.properties().getPropertyAsInt("Ice.ConnectionIdleTime");
		    if(_acmTimeout > 0)
		    {
			_acmAbsoluteTimeoutMillis = (System.DateTime.Now.Ticks - 621355968000000000) / 10000 +
						    _acmTimeout * 1000;
		    }
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
	}
	
	public bool isValidated()
	{
	    //
	    // No synchronization necessary, _state is declared
	    // volatile. Synchronization is not possible here anyway,
	    // because this function must not block.
	    //
	    return _state > StateNotValidated;
	}

	public bool isDestroyed()
	{
	    //
	    // No synchronization necessary, _state is declared
	    // volatile. Synchronization is not possible here anyway,
	    // because this function must not block.
	    //
	    return _state >= StateClosing;
	}

	public bool isFinished()
	{
	    //
	    // No synchronization necessary, _transceiver and
	    // _dispatchCount are declared volatile. Synchronization is
	    // not possible here anyway, because this function must not
	    // block.
	    //
	    return _transceiver == null && _dispatchCount == 0;
	}

	public void waitUntilHolding()
	{
	    lock(this)
	    {
		while(_state < StateHolding || _dispatchCount > 0)
		{
		    try
		    {
			System.Threading.Monitor.Wait(this);
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
		}
	    }
	}
	
	public void waitUntilFinished()
	{
	    lock(this)
	    {
		//
		// We wait indefinitely until all outstanding requests are
		// completed. Otherwise we couldn't guarantee that there are
		// no outstanding calls when deactivate() is called on the
		// servant locators.
		//
		while(_dispatchCount > 0)
		{
		    try
		    {
			System.Threading.Monitor.Wait(this);
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
		}
		
		//
		// Now we must wait for connection closure. If there is a
		// timeout, we force the connection closure.
		//
		while(_transceiver != null)
		{
		    try
		    {
			if(_endpoint.timeout() >= 0)
			{
			    long absoluteTimeoutMillis = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
							 + _endpoint.timeout();
			    
			    System.Threading.Monitor.Wait(this, System.TimeSpan.FromMilliseconds(_endpoint.timeout()));
			    
			    if((System.DateTime.Now.Ticks - 621355968000000000) / 10000 >= absoluteTimeoutMillis)
			    {
				setState(StateClosed, new Ice.CloseTimeoutException());
				// No return here, we must still wait until _transceiver becomes null.
			    }
			}
			else
			{
			    System.Threading.Monitor.Wait(this);
			}
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
		}
		
		Debug.Assert(_state == StateClosed);
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
		foreach(OutgoingAsync og in _asyncRequests.Values)
		{
		    if(og.__timedOut())
		    {
			setState(StateClosed, new Ice.TimeoutException());
			return;
		    }
		}
		
		//
		// Active connection management for idle connections.
		//
		// TODO: Hack: ACM for incoming connections doesn't work right
		// with AMI.
		//
		if(_acmTimeout > 0 && closingOK() && _adapter == null)
		{
		    if((System.DateTime.Now.Ticks - 621355968000000000) / 10000 >= _acmAbsoluteTimeoutMillis)
		    {
			setState(StateClosing, new Ice.ConnectionTimeoutException());
			return ;
		    }
		}
	    }
	}
	
	public void incProxyCount()
	{
	    lock(this)
	    {
		Debug.Assert(_proxyCount >= 0);
		++_proxyCount;
	    }
	}
	
	public void decProxyCount()
	{
	    lock(this)
	    {
		Debug.Assert(_proxyCount > 0);
		--_proxyCount;
		
		if(_proxyCount == 0 && _adapter == null && closingOK())
		{
		    setState(StateClosing, new Ice.CloseConnectionException());
		}
	    }
	}
	
	private static readonly byte[] _requestHdr = new byte[]
	{
	    Protocol.magic[0], Protocol.magic[1], Protocol.magic[2], Protocol.magic[3],
	    Protocol.protocolMajor, Protocol.protocolMinor,
	    Protocol.encodingMajor, Protocol.encodingMinor,
	    Protocol.requestMsg,
	    (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0
	};
	
	public void prepareRequest(BasicStream os)
	{
	    os.writeBlob(_requestHdr);
	}
	
	public void sendRequest(Outgoing og, bool oneway)
	{
	    lock(this)
	    {
		if(_exception != null)
		{
		    throw _exception;
		}
		Debug.Assert(_state > StateNotValidated);
		Debug.Assert(_state < StateClosing);
		
		int requestId = 0;
		
		try
		{
		    BasicStream os = og.ostr();
		    os.pos(10);
		    
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
		    Debug.Assert(_exception != null);
		    throw _exception;
		}
		
		//
		// Only add to the request map if there was no exception, and if
		// the operation is not oneway.
		//
		if(!_endpoint.datagram() && !oneway)
		{
		    _requests[requestId] = og;
		}
		
		if(_acmTimeout > 0)
		{
		    _acmAbsoluteTimeoutMillis = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
						+ _acmTimeout * 1000;
		}
	    }
	}
	
	public void sendAsyncRequest(OutgoingAsync og)
	{
	    lock(this)
	    {
		if(_exception != null)
		{
		    throw _exception;
		}
		Debug.Assert(_state > StateNotValidated);
		Debug.Assert(_state < StateClosing);
		
		int requestId = 0;
		
		try
		{
		    BasicStream os = og.__os();
		    os.pos(10);
		    
		    //
		    // Fill in the message size and request ID.
		    //
		    os.writeInt(os.size());
		    requestId = _nextRequestId++;
		    if(requestId <= 0)
		    {
			_nextRequestId = 1;
			requestId = _nextRequestId++;
		    }
		    os.writeInt(requestId);
		    
		    //
		    // Send the request.
		    //
		    TraceUtil.traceRequest("sending asynchronous request", os, _logger, _traceLevels);
		    _transceiver.write(os, _endpoint.timeout());
		}
		catch(Ice.LocalException ex)
		{
		    setState(StateClosed, ex);
		    Debug.Assert(_exception != null);
		    throw _exception;
		}
		
		//
		// Only add to the request map if there was no exception.
		//
		_asyncRequests[requestId] = og;
		
		if(_acmTimeout > 0)
		{
		    _acmAbsoluteTimeoutMillis = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
						+ _acmTimeout * 1000;
		}
	    }
	}
	
	private static readonly byte[] _requestBatchHdr = new byte[]
	{
	    Protocol.magic[0], Protocol.magic[1], Protocol.magic[2], Protocol.magic[3],
	    Protocol.protocolMajor, Protocol.protocolMinor,
	    Protocol.encodingMajor, Protocol.encodingMinor,
	    Protocol.requestBatchMsg,
	    0,
	    (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0, (byte)0
	};
	
	public void prepareBatchRequest(BasicStream os)
	{
	    lock(this)
	    {
		while(_batchStreamInUse && _exception == null)
		{
		    try
		    {
			System.Threading.Monitor.Wait(this);
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
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
		    catch(Ice.LocalException ex)
		    {
			setState(StateClosed, ex);
			throw ex;
		    }
		}
		
		_batchStreamInUse = true;
		_batchStream.swap(os);
		
		//
		// _batchStream now belongs to the caller, until
		// finishBatchRequest() or abortBatchRequest() is called.
		//
	    }
	}
	
	public void finishBatchRequest(BasicStream os)
	{
	    lock(this)
	    {
		if(_exception != null)
		{
		    throw _exception;
		}
		Debug.Assert(_state > StateNotValidated);
		Debug.Assert(_state < StateClosing);
		
		_batchStream.swap(os); // Get the batch stream back.
		++_batchRequestNum; // Increment the number of requests in the batch.
		
		//
		// Give the Connection back.
		//
		Debug.Assert(_batchStreamInUse);
		_batchStreamInUse = false;
		System.Threading.Monitor.PulseAll(this);
	    }
	}
	
	public void abortBatchRequest()
	{
	    lock(this)
	    {
		setState(StateClosed, new Ice.AbortBatchRequestException());
		
		//
		// Give the Connection back.
		//
		Debug.Assert(_batchStreamInUse);
		_batchStreamInUse = false;
		System.Threading.Monitor.PulseAll(this);
	    }
	}
	
	public void flushBatchRequest()
	{
	    lock(this)
	    {
		while(_batchStreamInUse && _exception == null)
		{
		    try
		    {
			System.Threading.Monitor.Wait(this);
		    }
		    catch(System.Threading.ThreadInterruptedException)
		    {
		    }
		}
		
		if(_exception != null)
		{
		    throw _exception;
		}
		Debug.Assert(_state > StateNotValidated);
		Debug.Assert(_state < StateClosing);
		
		if(!_batchStream.isEmpty())
		{
		    try
		    {
			_batchStream.pos(10);
			
			//
			// Fill in the message size.
			//
			_batchStream.writeInt(_batchStream.size());
			
			//
			// Fill in the number of requests in the batch.
			//
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
			Debug.Assert(_exception != null);
			throw _exception;
		    }
		    
		    if(_acmTimeout > 0)
		    {
			_acmAbsoluteTimeoutMillis = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
						    + _acmTimeout * 1000;
		    }
		}
		
		if(_proxyCount == 0 && _adapter == null && closingOK())
		{
		    setState(StateClosing, new Ice.CloseConnectionException());
		}
	    }
	}
	
	public void sendResponse(BasicStream os, byte compress)
	{
	    lock(this)
	    {
		try
		{
		    if(--_dispatchCount == 0)
		    {
			System.Threading.Monitor.PulseAll(this);
		    }
		    
		    if(_state == StateClosed)
		    {
			return;
		    }
		    
		    //
		    // Fill in the message size.
		    //
		    os.pos(10);
		    int sz = os.size();
		    os.writeInt(sz);
		    
		    //
		    // Send the reply.
		    //
		    TraceUtil.traceReply("sending reply", os, _logger, _traceLevels);
		    _transceiver.write(os, _endpoint.timeout());
		    
		    if(_state == StateClosing && _dispatchCount == 0)
		    {
			initiateShutdown();
		    }
		}
		catch(Ice.LocalException ex)
		{
		    setState(StateClosed, ex);
		}
		
		if(_acmTimeout > 0)
		{
		    _acmAbsoluteTimeoutMillis = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
						+ _acmTimeout * 1000;
		}
	    }
	}
	
	public void sendNoResponse()
	{
	    lock(this)
	    {
		try
		{
		    if(--_dispatchCount == 0)
		    {
			System.Threading.Monitor.PulseAll(this);
		    }
		    
		    if(_state == StateClosed)
		    {
			return;
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
	}
	
	public int timeout()
	{
	    // No mutex protection necessary, _endpoint is immutable.
	    return _endpoint.timeout();
	}
	
	public Endpoint endpoint()
	{
	    // No mutex protection necessary, _endpoint is immutable.
	    return _endpoint;
	}

	public void setAdapter(Ice.ObjectAdapter adapter)
	{
	    lock(this)
	    {
		//
		// We never change the thread pool with which we were
		// initially registered, even if we add or remove an object
		// adapter.
		//
		
		_adapter = adapter;
		if(_adapter != null)
		{
		    _servantManager = ((Ice.ObjectAdapterI) _adapter).getServantManager();
		}
		else
		{
		    _servantManager = null;
		}
	    }
	}

	public Ice.ObjectAdapter getAdapter()
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
	
	public override void read(BasicStream stream)
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
	    Protocol.magic[0], Protocol.magic[1], Protocol.magic[2], Protocol.magic[3],
	    Protocol.protocolMajor, Protocol.protocolMinor,
	    Protocol.encodingMajor, Protocol.encodingMinor,
	    Protocol.replyMsg,
	    (byte)0, (byte)0, (byte)0, (byte)0, (byte)0
	};
	
	public override void message(BasicStream stream, ThreadPool threadPool)
	{
	    OutgoingAsync outAsync = null;
	    
	    int invoke = 0;
	    int requestId = 0;
	    byte compress = 0;
	    
	    lock(this)
	    {
		threadPool.promoteFollower();
		
		if(_state == StateClosed)
		{
		    System.Threading.Thread.Sleep(0);
		    return ;
		}
		
		if(_acmTimeout > 0)
		{
		    _acmAbsoluteTimeoutMillis = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
						+ _acmTimeout * 1000;
		}
		
		try
		{
		    Debug.Assert(stream.pos() == stream.size());
		    stream.pos(0);
		    
		    byte[] m = new byte[4];
		    m[0] = stream.readByte();
		    m[1] = stream.readByte();
		    m[2] = stream.readByte();
		    m[3] = stream.readByte();
		    if(m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
		       m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
		    {
			Ice.BadMagicException ex = new Ice.BadMagicException();
			ex.badMagic = new Ice.ByteSeq(m);
			throw ex;
		    }
		    
		    byte pMajor = stream.readByte();
		    byte pMinor = stream.readByte();
		    if(pMajor != Protocol.protocolMajor || pMinor > Protocol.protocolMinor)
		    {
			Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
			e.badMajor = pMajor < 0 ? pMajor + 255 : pMajor;
			e.badMinor = pMinor < 0 ? pMinor + 255 : pMinor;
			e.major = Protocol.protocolMajor;
			e.minor = Protocol.protocolMinor;
			throw e;
		    }
		    
		    byte eMajor = stream.readByte();
		    byte eMinor = stream.readByte();
		    if(eMajor != Protocol.encodingMajor || eMinor > Protocol.encodingMinor)
		    {
			Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
			e.badMajor = eMajor < 0 ? eMajor + 255 : eMajor;
			e.badMinor = eMinor < 0 ? eMinor + 255 : eMinor;
			e.major = Protocol.encodingMajor;
			e.minor = Protocol.encodingMinor;
			throw e;
		    }
		    
		    byte messageType = stream.readByte();
		    compress = stream.readByte();
		    
		    if(compress == (byte)2)
		    {
			throw new Ice.CompressionNotSupportedException();
		    }
		    
		    stream.pos(Protocol.headerSize);
		    
		    switch(messageType)
		    {
			case Protocol.requestMsg: 
			{
			    if(_state == StateClosing)
			    {
				TraceUtil.traceRequest("received request during closing\n"
						       + "(ignored by server, client will retry)",
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
				TraceUtil.traceBatchRequest("received batch request during closing\n"
							    + "(ignored by server, client will retry)",
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
			    Outgoing og = (Outgoing)_requests[requestId];
			    _requests.Remove(requestId);
			    if(og != null)
			    {
				og.finished(stream);
			    }
			    else
			    {
				outAsync = (OutgoingAsync)_asyncRequests[requestId];
				_asyncRequests.Remove(requestId);
				if(outAsync == null)
				{
				    throw new Ice.UnknownRequestIdException();
				}
				
				if(_proxyCount == 0 && _adapter == null && closingOK())
				{
				    setState(StateClosing, new Ice.CloseConnectionException());
				}
			    }
			    break;
			}
			
			case Protocol.validateConnectionMsg: 
			{
			    TraceUtil.traceHeader("received validate connection", stream, _logger, _traceLevels);
			    if(_warn)
			    {
				_logger.warning("ignoring unexpected validate connection message:\n"
						+ _transceiver.ToString());
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
				    _logger.warning("ignoring close connection message for datagram connection:\n"
						    + _transceiver.ToString());
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
			    TraceUtil.traceHeader("received unknown message\n" + "(invalid, closing connection)",
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
	    Incoming inc = null;
	    try
	    {
		while(invoke-- > 0)
		{
		    //
		    // Prepare the invocation.
		    //
		    bool response = !_endpoint.datagram() && requestId != 0;
		    inc = getIncoming(response, compress);
		    BasicStream ins = inc.istr();
		    stream.swap(ins);
		    BasicStream os = inc.ostr();
		    
		    //
		    // Prepare the response if necessary.
		    //
		    if(response)
		    {
			Debug.Assert(invoke == 0); // No further invocations if a response is expected.
			os.writeBlob(_replyHdr);
			
			//
			// Fill in the request ID.
			//
			os.writeInt(requestId);
		    }
		    
		    inc.invoke(_servantManager);
		    
		    //
		    // If there are more invocations, we need the stream back.
		    //
		    if(invoke > 0)
		    {
			stream.swap(ins);
		    }
		    
		    reclaimIncoming(inc);
		    inc = null;
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		lock(this)
		{
		    setState(StateClosed, ex);
		}
	    }
	    catch(System.Exception ex)
	    {
		//
		// For other errors, we don't kill the whole
		// process, but just print the stack trace and close the
		// connection.
		//
		warning("closing connection", ex);
		lock(this)
		{
		    setState(StateClosed, new Ice.UnknownException());
		}
	    }
	    finally
	    {
		if(inc != null)
		{
		    reclaimIncoming(inc);
		}
	    }
	}
	
	public override void finished(ThreadPool threadPool)
	{
	    lock(this)
	    {
		threadPool.promoteFollower();
		
		if(_state == StateActive || _state == StateClosing)
		{
		    registerWithPool();
		}
		else if(_state == StateClosed && _transceiver != null)
		{
		    _transceiver.close();
		    _transceiver = null;
		    _threadPool = null; // We don't need the thread pool anymore.
		    System.Threading.Monitor.PulseAll(this);
		}
	    }
	}
	
	public override void exception(Ice.LocalException ex)
	{
	    lock(this)
	    {
		setState(StateClosed, ex);
	    }
	}
	
	public override string ToString()
	{
	    Debug.Assert(_transceiver != null);
	    return _transceiver.ToString();
	}
	
	internal Connection(Instance instance, Transceiver transceiver,
			    Endpoint endpoint, Ice.ObjectAdapter adapter)
	    : base(instance)
	{
	    _transceiver = transceiver;
	    _endpoint = endpoint;
	    _adapter = adapter;
	    _logger = instance.logger(); // Cached for better performance.
	    _traceLevels = instance.traceLevels(); // Cached for better performance.
	    _registeredWithPool = false;
	    _warn = false;
	    _acmTimeout = 0;
	    _acmAbsoluteTimeoutMillis = 0;
	    _requests = new Hashtable();
	    _asyncRequests = new Hashtable();
	    _nextRequestId = 1;
	    _batchStream = new BasicStream(instance);
	    _batchStreamInUse = false;
	    _batchRequestNum = 0;
	    _dispatchCount = 0;
	    _proxyCount = 0;
	    _state = StateNotValidated;
	    _incomingCacheMutex = new System.Threading.Mutex();
	    
	    if(_adapter != null)
	    {
		_threadPool = ((Ice.ObjectAdapterI) _adapter).getThreadPool();
		_servantManager = ((Ice.ObjectAdapterI) _adapter).getServantManager();
	    }
	    else
	    {
		_threadPool = _instance.clientThreadPool();
		_servantManager = null;
	    }
	}
	
	~Connection()
	{
	    Debug.Assert(_state == StateClosed, "~Connection(): _state != StateClosed");
	    Debug.Assert(_transceiver == null, "~Connection(): _tranceiver != null");
	    Debug.Assert(_dispatchCount == 0, "~Connection(): _dispatchCount != 0");
	    Debug.Assert(_proxyCount == 0, "~Connection(): _proxyCount != 0");
	    //Debug.Assert(_incomingCache == null, "~Connection(): _incomingCache != null"); // TODO: this assertion appears wrong -- _incoming cache is never null once an Incoming has been reclaimed.

	    _batchStream.destroy();
	}
	
	private const int StateNotValidated = 0;
	private const int StateActive = 1;
	private const int StateHolding = 2;
	private const int StateClosing = 3;
	private const int StateClosed = 4;
	
	private void setState(int state, Ice.LocalException ex)
	{
	    if(_state == state)
	    // Don't switch twice.
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
		    if(!(_exception is Ice.CloseConnectionException ||
			  _exception is Ice.ConnectionTimeoutException ||
			  _exception is Ice.CommunicatorDestroyedException ||
			  _exception is Ice.ObjectAdapterDeactivatedException ||
			  (_exception is Ice.ConnectionLostException && _state == StateClosing)))
		    {
			warning("connection exception", _exception);
		    }
		}
	    }
	    
	    //
	    // We must set the new state before we notify requests of any
	    // exceptions. Otherwise new requests may retry on a
	    // connection that is not yet marked as closed or closing.
	    //
	    setState(state);
	    
	    foreach(Outgoing og in _requests.Values)
	    {
		 og.finished(_exception);
	    }
	    _requests.Clear();
	    
	    foreach(OutgoingAsync og in _asyncRequests.Values)
	    {
		og.__finished(_exception);
	    }
	    _asyncRequests.Clear();
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
	    
	    if(_state == state)
	    // Don't switch twice.
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
			_transceiver.close();
			_transceiver = null;
			_threadPool = null; // We don't need the thread pool anymore.
		    }
		    else
		    {
			registerWithPool();
			unregisterWithPool();
		    }
		    break;
		}
	    }
	    
	    _state = state;
	    System.Threading.Monitor.PulseAll(this);
	    
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
	
	private void initiateShutdown()
	{
	    Debug.Assert(_state == StateClosing);
	    Debug.Assert(_dispatchCount == 0);
	    
	    if(!_endpoint.datagram())
	    {
		//
		// Before we shut down, we send a close connection
		// message.
		//
		BasicStream os = new BasicStream(_instance);
		os.writeByte(Protocol.magic[0]);
		os.writeByte(Protocol.magic[1]);
		os.writeByte(Protocol.magic[2]);
		os.writeByte(Protocol.magic[3]);
		os.writeByte(Protocol.protocolMajor);
		os.writeByte(Protocol.protocolMinor);
		os.writeByte(Protocol.encodingMajor);
		os.writeByte(Protocol.encodingMinor);
		os.writeByte(Protocol.closeConnectionMsg);
		os.writeByte((byte)0); // Compression status.
		os.writeInt(Protocol.headerSize); // Message size.
		_transceiver.write(os, _endpoint.timeout());
		_transceiver.shutdown();
	    }
	}
	
	private void registerWithPool()
	{
	    if(!_registeredWithPool)
	    {
		Debug.Assert(_threadPool != null);
		_threadPool._register(_transceiver.fd(), this);
		_registeredWithPool = true;
		
		ConnectionMonitor connectionMonitor = _instance.connectionMonitor();
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
		Debug.Assert(_threadPool != null);
		_threadPool.unregister(_transceiver.fd());
		_registeredWithPool = false;
		
		ConnectionMonitor connectionMonitor = _instance.connectionMonitor();
		if(connectionMonitor != null)
		{
		    connectionMonitor.remove(this);
		}
	    }
	}
	
	private void warning(string msg, System.Exception ex)
	{
	    _logger.warning(msg + ":\n" + ex.StackTrace.ToString() + _transceiver.ToString());
	}
	
	private Incoming getIncoming(bool response, byte compress)
	{
	    Incoming inc = null;
	    
	    _incomingCacheMutex.WaitOne();
	    try
	    {
		if(_incomingCache == null)
		{
		    inc = new Incoming(_instance, this, _adapter, response, compress);
		}
		else
		{
		    inc = _incomingCache;
		    _incomingCache = _incomingCache.next;
		    inc.next = null;
		    inc.reset(_instance, this, _adapter, response, compress);
		}
	    }
	    finally
	    {
		_incomingCacheMutex.ReleaseMutex();
	    }
	    
	    return inc;
	}
	
	private void reclaimIncoming(Incoming inc)
	{
	    _incomingCacheMutex.WaitOne();
	    inc.next = _incomingCache;
	    _incomingCache = inc;
	    _incomingCacheMutex.ReleaseMutex();
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
	
	private volatile Transceiver _transceiver; // Must be volatile, see comment in isFinished().
	private volatile Endpoint _endpoint;
	
	private Ice.ObjectAdapter _adapter;
	private ServantManager _servantManager;
	
	private volatile Ice.Logger _logger;
	private volatile TraceLevels _traceLevels;
	
	private bool _registeredWithPool;
	private ThreadPool _threadPool;
	
	private bool _warn;
	
	private int _acmTimeout;
	private long _acmAbsoluteTimeoutMillis;
	
	private int _nextRequestId;
	private Hashtable _requests;
	private Hashtable _asyncRequests;
	
	private Ice.LocalException _exception;
	
	private BasicStream _batchStream;
	private bool _batchStreamInUse;
	private int _batchRequestNum;
	
	private volatile int _dispatchCount; // Must be volatile, see comment in isDestroyed().
	
	private int _proxyCount;
	
	private volatile int _state; // Must be volatile, see comment in isDestroyed().
	
	private Incoming _incomingCache;
	private System.Threading.Mutex _incomingCacheMutex;
    }

}
