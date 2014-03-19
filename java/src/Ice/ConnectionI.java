// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

import Ice.Instrumentation.InvocationObserver;

public final class ConnectionI extends IceInternal.EventHandler implements Connection
{
    public interface StartCallback
    {
        void connectionStartCompleted(ConnectionI connection);
        void connectionStartFailed(ConnectionI connection, Ice.LocalException ex);
    }

    private class TimeoutCallback implements IceInternal.TimerTask
    {
        public void
        runTimerTask()
        {
            timedOut();
        }
    }

    public void
    start(StartCallback callback)
    {
        try
        {
            synchronized(this)
            {
                if(_state >= StateClosed) // The connection might already be closed if the communicator was destroyed.
                {
                    assert(_exception != null);
                    throw (Ice.LocalException)_exception.fillInStackTrace();
                }

                if(!initialize(IceInternal.SocketOperation.None) || !validate(IceInternal.SocketOperation.None))
                {
                    if(callback != null)
                    {
                        _startCallback = callback;
                        return;
                    }

                    //
                    // Wait for the connection to be validated.
                    //
                    while(_state <= StateNotValidated)
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
                        throw (Ice.LocalException)_exception.fillInStackTrace();
                    }
                }

                //
                // We start out in holding state.
                //
                setState(StateHolding);
            }
        }
        catch(Ice.LocalException ex)
        {
            exception(ex);
            if(callback != null)
            {
                callback.connectionStartFailed(this, _exception);
                return;
            }
            else
            {
                waitUntilFinished();
                throw ex;
            }
        }

        if(callback != null)
        {
            callback.connectionStartCompleted(this);
        }
    }

    public synchronized void
    activate()
    {
        if(_state <= StateNotValidated)
        {
            return;
        }

        if(_acmTimeout > 0)
        {
            _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
        }

        setState(StateActive);
    }

    public synchronized void
    hold()
    {
        if(_state <= StateNotValidated)
        {
            return;
        }

        setState(StateHolding);
    }

    // DestructionReason.
    public final static int ObjectAdapterDeactivated = 0;
    public final static int CommunicatorDestroyed = 1;

    synchronized public void
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

    synchronized public void
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
            while(!_requests.isEmpty() || !_asyncRequests.isEmpty())
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
    isActiveOrHolding()
    {
        return _state > StateNotValidated && _state < StateClosing;
    }

    public synchronized boolean
    isFinished()
    {
        if(_state != StateFinished || _dispatchCount != 0)
        {
            return false;
        }

        assert(_state == StateFinished);
        return true;
    }

    public synchronized void
    throwException()
    {
        if(_exception != null)
        {
            assert(_state >= StateClosing);
            throw (Ice.LocalException)_exception.fillInStackTrace();
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

    public synchronized void
    waitUntilFinished()
    {
        //
        // We wait indefinitely until the connection is finished and all
        // outstanding requests are completed. Otherwise we couldn't
        // guarantee that there are no outstanding calls when deactivate()
        // is called on the servant locators.
        //
        while(_state < StateFinished || _dispatchCount > 0)
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
            }
        }

        assert(_state == StateFinished);

        //
        // Clear the OA. See bug 1673 for the details of why this is necessary.
        //
        _adapter = null;
    }

    synchronized public void
    updateObserver()
    {
        if(_state < StateNotValidated || _state > StateClosed)
        {
            return;
        }

        assert(_instance.getObserver() != null);
        _observer = _instance.getObserver().getConnectionObserver(initConnectionInfo(),
                                                                  _endpoint,
                                                                  toConnectionState(_state),
                                                                  _observer);
        if(_observer != null)
        {
            _observer.attach();
        }
        else
        {
            _writeStreamPos = -1;
            _readStreamPos = -1;
        }
    }

    synchronized public void
    monitor(long now)
    {
        if(_state != StateActive)
        {
            return;
        }

        //
        // Active connection management for idle connections.
        //
        if(_acmTimeout <= 0 ||
           !_requests.isEmpty() || !_asyncRequests.isEmpty() || _dispatchCount > 0 ||
           _readStream.size() > IceInternal.Protocol.headerSize || !_writeStream.isEmpty() || !_batchStream.isEmpty())
        {
            return;
        }

        if(now >= _acmAbsoluteTimeoutMillis)
        {
            setState(StateClosing, new ConnectionTimeoutException());
        }
    }

    synchronized public boolean
    sendRequest(IceInternal.Outgoing out, boolean compress, boolean response)
        throws IceInternal.LocalExceptionWrapper
    {
        int requestId = 0;
        final IceInternal.BasicStream os = out.os();

        if(_exception != null)
        {
            //
            // If the connection is closed before we even have a chance
            // to send our request, we always try to send the request
            // again.
            //
            throw new IceInternal.LocalExceptionWrapper((Ice.LocalException)_exception.fillInStackTrace(), true);
        }

        assert(_state > StateNotValidated);
        assert(_state < StateClosing);

        //
        // Ensure the message isn't bigger than what we can send with the
        // transport.
        //
        _transceiver.checkSendSize(os.getBuffer(), _instance.messageSizeMax());

        if(response)
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
        }

        out.attachRemoteObserver(initConnectionInfo(), _endpoint, requestId, 
                                 os.size() - IceInternal.Protocol.headerSize - 4);

        //
        // Send the message. If it can't be sent without blocking the message is added
        // to _sendStreams and it will be sent by the selector thread or by this thread
        // if flush is true.
        //
        boolean sent = false;
        try
        {
            OutgoingMessage message = new OutgoingMessage(out, out.os(), compress, requestId);
            sent = (sendMessage(message) & IceInternal.AsyncStatus.Sent) > 0;
        }
        catch(Ice.LocalException ex)
        {
            setState(StateClosed, ex);
            assert(_exception != null);
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }

        if(response)
        {
            //
            // Add to the requests map.
            //
            _requests.put(requestId, out);
        }

        return sent; // The request was sent.
    }

    synchronized public int
    sendAsyncRequest(IceInternal.OutgoingAsync out, boolean compress, boolean response)
        throws IceInternal.LocalExceptionWrapper
    {
        int requestId = 0;
        final IceInternal.BasicStream os = out.__getOs();

        if(_exception != null)
        {
            //
            // If the connection is closed before we even have a chance
            // to send our request, we always try to send the request
            // again.
            //
            throw new IceInternal.LocalExceptionWrapper((Ice.LocalException)_exception.fillInStackTrace(), true);
        }

        assert(_state > StateNotValidated);
        assert(_state < StateClosing);

        //
        // Ensure the message isn't bigger than what we can send with the
        // transport.
        //
        _transceiver.checkSendSize(os.getBuffer(), _instance.messageSizeMax());

        if(response)
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
        }

        out.__attachRemoteObserver(initConnectionInfo(), _endpoint, requestId, 
                                   os.size() - IceInternal.Protocol.headerSize - 4);

        int status;
        try
        {
            status = sendMessage(new OutgoingMessage(out, out.__getOs(), compress, requestId));
        }
        catch(Ice.LocalException ex)
        {
            setState(StateClosed, ex);
            assert(_exception != null);
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }

        if(response)
        {
            //
            // Add to the async requests map.
            //
            _asyncRequests.put(requestId, out);
        }
        return status;
    }

    public synchronized void
    prepareBatchRequest(IceInternal.BasicStream os)
        throws IceInternal.LocalExceptionWrapper
    {
        //
        // Wait if flushing is currently in progress.
        //
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
            //
            // If there were no batch requests queued when the connection failed, we can safely 
            // retry with a new connection. Otherwise, we must throw to notify the caller that 
            // some previous batch requests were not sent.
            //
            if(_batchStream.isEmpty())
            {
                throw new IceInternal.LocalExceptionWrapper((Ice.LocalException)_exception.fillInStackTrace(), true);
            }
            else
            {
                throw (Ice.LocalException)_exception.fillInStackTrace();
            }
        }

        assert(_state > StateNotValidated);
        assert(_state < StateClosing);

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
    finishBatchRequest(IceInternal.BasicStream os, boolean compress)
    {
        try
        {
            synchronized(this)
            {
                //
                // Get the batch stream back.
                //
                _batchStream.swap(os);

                if(_exception != null)
                {
                    throw (Ice.LocalException)_exception.fillInStackTrace();
                }

                boolean flush = false;
                if(_batchAutoFlush)
                {
                    //
                    // Throw memory limit exception if the first message added causes us to go over
                    // limit. Otherwise put aside the marshalled message that caused limit to be
                    // exceeded and rollback stream to the marker.
                    try
                    {
                        _transceiver.checkSendSize(_batchStream.getBuffer(), _instance.messageSizeMax());
                    }
                    catch(Ice.LocalException ex)
                    {
                        if(_batchRequestNum > 0)
                        {
                            flush = true;
                        }
                        else
                        {
                            throw ex;
                        }
                    }
                }

                if(flush)
                {
                    //
                    // Temporarily save the last request.
                    //
                    byte[] lastRequest = new byte[_batchStream.size() - _batchMarker];
                    IceInternal.Buffer buffer = _batchStream.getBuffer();
                    buffer.b.position(_batchMarker);
                    buffer.b.get(lastRequest);
                    _batchStream.resize(_batchMarker, false);

                    try
                    {
                        //
                        // Fill in the number of requests in the batch.
                        //
                        _batchStream.pos(IceInternal.Protocol.headerSize);
                        _batchStream.writeInt(_batchRequestNum);

                        OutgoingMessage message = new OutgoingMessage(_batchStream, _batchRequestCompress, true);
                        sendMessage(message);
                    }
                    catch(Ice.LocalException ex)
                    {
                        setState(StateClosed, ex);
                        assert(_exception != null);
                        throw (Ice.LocalException)_exception.fillInStackTrace();
                    }

                    //
                    // Reset the batch stream.
                    //
                    _batchStream = new IceInternal.BasicStream(_instance, IceInternal.Protocol.currentProtocolEncoding,
                                                               _batchAutoFlush);
                    _batchRequestNum = 0;
                    _batchRequestCompress = false;
                    _batchMarker = 0;

                    //
                    // Check again if the last request doesn't exceed the maximum message size.
                    //
                    if(IceInternal.Protocol.requestBatchHdr.length + lastRequest.length >  _instance.messageSizeMax())
                    {
                        IceInternal.Ex.throwMemoryLimitException(
                            IceInternal.Protocol.requestBatchHdr.length + lastRequest.length,
                            _instance.messageSizeMax());
                    }

                    //
                    // Start a new batch with the last message that caused us to go over the limit.
                    //
                    _batchStream.writeBlob(IceInternal.Protocol.requestBatchHdr);
                    _batchStream.writeBlob(lastRequest);
                }

                //
                // Increment the number of requests in the batch.
                //
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
        }
        catch(Ice.LocalException ex)
        {
            abortBatchRequest();
            throw ex;
        }
    }

    public synchronized void
    abortBatchRequest()
    {
        _batchStream = new IceInternal.BasicStream(_instance, IceInternal.Protocol.currentProtocolEncoding, 
                                                   _batchAutoFlush);
        _batchRequestNum = 0;
        _batchRequestCompress = false;
        _batchMarker = 0;

        assert(_batchStreamInUse);
        _batchStreamInUse = false;
        notifyAll();
    }

    public void
    flushBatchRequests()
    {
        final InvocationObserver observer = IceInternal.ObserverHelper.get(_instance, __flushBatchRequests_name);
        IceInternal.BatchOutgoing out = new IceInternal.BatchOutgoing(this, _instance, observer);
        out.invoke();
    }

    private static final String __flushBatchRequests_name = "flushBatchRequests";

    public Ice.AsyncResult
    begin_flushBatchRequests()
    {
        return begin_flushBatchRequestsInternal(null);
    }

    public Ice.AsyncResult
    begin_flushBatchRequests(Callback cb)
    {
        return begin_flushBatchRequestsInternal(cb);
    }

    public Ice.AsyncResult
    begin_flushBatchRequests(Callback_Connection_flushBatchRequests cb)
    {
        return begin_flushBatchRequestsInternal(cb);
    }

    private Ice.AsyncResult
    begin_flushBatchRequestsInternal(IceInternal.CallbackBase cb)
    {
        IceInternal.ConnectionBatchOutgoingAsync result =
            new IceInternal.ConnectionBatchOutgoingAsync(this, _communicator, _instance, __flushBatchRequests_name, cb);
        try
        {
            result.__send();
        }
        catch(LocalException __ex)
        {
            result.__exceptionAsync(__ex);
        }
        return result;
    }

    public void
    end_flushBatchRequests(AsyncResult r)
    {
        AsyncResult.__check(r, this, __flushBatchRequests_name);
        r.__wait();
    }

    synchronized public boolean
    flushBatchRequests(IceInternal.BatchOutgoing out)
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
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }

        if(_batchRequestNum == 0)
        {
            out.sent(false);
            return true;
        }

        //
        // Fill in the number of requests in the batch.
        //
        _batchStream.pos(IceInternal.Protocol.headerSize);
        _batchStream.writeInt(_batchRequestNum);

        out.attachRemoteObserver(initConnectionInfo(), _endpoint,
                                 _batchStream.size() - IceInternal.Protocol.headerSize - 4);

        _batchStream.swap(out.os());

        boolean sent = false;
        try
        {
            OutgoingMessage message = new OutgoingMessage(out, out.os(), _batchRequestCompress, 0);
            sent = (sendMessage(message) & IceInternal.AsyncStatus.Sent) > 0;
        }
        catch(Ice.LocalException ex)
        {
            setState(StateClosed, ex);
            assert(_exception != null);
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }

        //
        // Reset the batch stream.
        //
        _batchStream = new IceInternal.BasicStream(_instance, IceInternal.Protocol.currentProtocolEncoding, 
                                                   _batchAutoFlush);
        _batchRequestNum = 0;
        _batchRequestCompress = false;
        _batchMarker = 0;
        return sent;
    }

    synchronized public int
    flushAsyncBatchRequests(IceInternal.BatchOutgoingAsync outAsync)
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
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }

        if(_batchRequestNum == 0)
        {
            int status = IceInternal.AsyncStatus.Sent;
            if(outAsync.__sent(this))
            {
                status |= IceInternal.AsyncStatus.InvokeSentCallback;
            }
            return status;
        }

        //
        // Fill in the number of requests in the batch.
        //
        _batchStream.pos(IceInternal.Protocol.headerSize);
        _batchStream.writeInt(_batchRequestNum);

        outAsync.__attachRemoteObserver(initConnectionInfo(), _endpoint, 0,
                                        _batchStream.size() - IceInternal.Protocol.headerSize - 4);

        _batchStream.swap(outAsync.__getOs());

        int status;
        try
        {
            OutgoingMessage message = new OutgoingMessage(outAsync, outAsync.__getOs(), _batchRequestCompress, 0);
            status = sendMessage(message);
        }
        catch(Ice.LocalException ex)
        {
            setState(StateClosed, ex);
            assert(_exception != null);
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }

        //
        // Reset the batch stream.
        //
        _batchStream = new IceInternal.BasicStream(_instance, IceInternal.Protocol.currentProtocolEncoding, 
                                                   _batchAutoFlush);
        _batchRequestNum = 0;
        _batchRequestCompress = false;
        _batchMarker = 0;
        return status;
    }

    synchronized public void
    sendResponse(IceInternal.BasicStream os, byte compressFlag)
    {
        assert(_state > StateNotValidated);

        try
        {
            if(--_dispatchCount == 0)
            {
                if(_state == StateFinished)
                {
                    _reaper.add(this, _observer);
                }
                notifyAll();
            }

            if(_state >= StateClosed)
            {
                assert(_exception != null);
                throw (Ice.LocalException)_exception.fillInStackTrace();
            }

            sendMessage(new OutgoingMessage(os, compressFlag != 0, true));

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

    synchronized public void
    sendNoResponse()
    {
        assert(_state > StateNotValidated);
        try
        {
            if(--_dispatchCount == 0)
            {
                if(_state == StateFinished)
                {
                    _reaper.add(this, _observer);
                }
                notifyAll();
            }

            if(_state >= StateClosed)
            {
                assert(_exception != null);
                throw (Ice.LocalException)_exception.fillInStackTrace();
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

    public IceInternal.EndpointI
    endpoint()
    {
        return _endpoint; // No mutex protection necessary, _endpoint is immutable.
    }

    public IceInternal.Connector
    connector()
    {
        return _connector; // No mutex protection necessary, _connector is immutable.
    }

    public synchronized void
    setAdapter(ObjectAdapter adapter)
    {
        if(_state <= StateNotValidated || _state >= StateClosing)
        {
            return;
        }
        assert(_state < StateClosing);

        _adapter = adapter;

        if(_adapter != null)
        {
            _servantManager = ((ObjectAdapterI)_adapter).getServantManager();
            if(_servantManager == null)
            {
                _adapter = null;
            }
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

    public Endpoint
    getEndpoint()
    {
        return _endpoint; // No mutex protection necessary, _endpoint is immutable.
    }

    public ObjectPrx
    createProxy(Identity ident)
    {
        //
        // Create a reference and return a reverse proxy for this
        // reference.
        //
        return _instance.proxyFactory().referenceToProxy(_instance.referenceFactory().create(ident, this));
    }

    //
    // Operations from EventHandler
    //
    public void
    message(IceInternal.ThreadPoolCurrent current)
    {
        StartCallback startCB = null;
        java.util.List<OutgoingMessage> sentCBs = null;
        MessageInfo info = null;

        synchronized(this)
        {
            if(_state >= StateClosed)
            {
                return;
            }

            try
            {
                unscheduleTimeout(current.operation);
                if((current.operation & IceInternal.SocketOperation.Write) != 0 && !_writeStream.isEmpty())
                {
                    if(_observer != null)
                    {
                        observerStartWrite(_writeStream.pos());
                    }
                    if(!_transceiver.write(_writeStream.getBuffer()))
                    {
                        assert(!_writeStream.isEmpty());
                        scheduleTimeout(IceInternal.SocketOperation.Write, _endpoint.timeout());
                        return;
                    }
                    if(_observer != null)
                    {
                        observerFinishWrite(_writeStream.pos());
                    }
                    assert(!_writeStream.getBuffer().b.hasRemaining());
                }
                if((current.operation & IceInternal.SocketOperation.Read) != 0 && !_readStream.isEmpty())
                {
                    if(_readHeader) // Read header if necessary.
                    {
                        if(!_transceiver.read(_readStream.getBuffer(), _hasMoreData))
                        {
                            return;
                        }
                        assert(!_readStream.getBuffer().b.hasRemaining());
                        _readHeader = false;

                        if(_observer != null)
                        {
                            _observer.receivedBytes(IceInternal.Protocol.headerSize);
                        }

                        int pos = _readStream.pos();
                        if(pos < IceInternal.Protocol.headerSize)
                        {
                            //
                            // This situation is possible for small UDP packets.
                            //
                            throw new Ice.IllegalMessageSizeException();
                        }

                        _readStream.pos(0);
                        byte[] m = new byte[4];
                        m[0] = _readStream.readByte();
                        m[1] = _readStream.readByte();
                        m[2] = _readStream.readByte();
                        m[3] = _readStream.readByte();
                        if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
                           m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                        {
                            Ice.BadMagicException ex = new Ice.BadMagicException();
                            ex.badMagic = m;
                            throw ex;
                        }

                        _readProtocol.__read(_readStream);
                        IceInternal.Protocol.checkSupportedProtocol(_readProtocol);

                        _readProtocolEncoding.__read(_readStream);
                        IceInternal.Protocol.checkSupportedProtocolEncoding(_readProtocolEncoding);

                        _readStream.readByte(); // messageType
                        _readStream.readByte(); // compress
                        int size = _readStream.readInt();
                        if(size < IceInternal.Protocol.headerSize)
                        {
                            throw new Ice.IllegalMessageSizeException();
                        }
                        if(size > _instance.messageSizeMax())
                        {
                            IceInternal.Ex.throwMemoryLimitException(size, _instance.messageSizeMax());
                        }
                        if(size > _readStream.size())
                        {
                            _readStream.resize(size, true);
                        }
                        _readStream.pos(pos);
                    }

                    if(_readStream.pos() != _readStream.size())
                    {
                        if(_endpoint.datagram())
                        {
                            throw new Ice.DatagramLimitException(); // The message was truncated.
                        }
                        else
                        {
                            if(_observer != null)
                            {
                                observerStartRead(_readStream.pos());
                            }
                            if(!_transceiver.read(_readStream.getBuffer(), _hasMoreData))
                            {
                                assert(!_readStream.isEmpty());
                                scheduleTimeout(IceInternal.SocketOperation.Read, _endpoint.timeout());
                                return;
                            }
                            if(_observer != null)
                            {
                                observerFinishRead(_readStream.pos());
                            }
                            assert(!_readStream.getBuffer().b.hasRemaining());
                        }
                    }
                }
                
                if(_state <= StateNotValidated)
                {
                    if(_state == StateNotInitialized && !initialize(current.operation))
                    {
                        return;
                    }
                    
                    if(_state <= StateNotValidated && !validate(current.operation))
                    {
                        return;
                    }

                    _threadPool.unregister(this, current.operation);

                    //
                    // We start out in holding state.
                    //
                    setState(StateHolding);
                    startCB = _startCallback;
                    _startCallback = null;
                }
                else
                {
                    assert(_state <= StateClosing);
                    
                    //
                    // We parse messages first, if we receive a close
                    // connection message we won't send more messages.
                    // 
                    if((current.operation & IceInternal.SocketOperation.Read) != 0)
                    {
                        info = parseMessage(current.stream); // Optimization: use the thread's stream.
                    }

                    if((current.operation & IceInternal.SocketOperation.Write) != 0)
                    {
                        sentCBs = sendNextMessage();
                    }
                    
                    //
                    // We increment the dispatch count to prevent the
                    // communicator destruction during the callback.
                    //
                    if(sentCBs != null || info != null && info.outAsync != null)
                    {
                        ++_dispatchCount;
                    }
                }
            }
            catch(DatagramLimitException ex) // Expected.
            {
                if(_warnUdp)
                {
                    _logger.warning("maximum datagram size of " + _readStream.pos() + " exceeded");
                }
                _readStream.resize(IceInternal.Protocol.headerSize, true);
                _readStream.pos(0);
                _readHeader = true;
                return;
            }
            catch(SocketException ex)
            {
                setState(StateClosed, ex);
                return;
            }
            catch(LocalException ex)
            {
                if(_endpoint.datagram())
                {
                    if(_warn)
                    {
                        String s = "datagram connection exception:\n" + ex + '\n' + _desc;
                        _logger.warning(s);
                    }
                    _readStream.resize(IceInternal.Protocol.headerSize, true);
                    _readStream.pos(0);
                    _readHeader = true;
                }
                else
                {
                    setState(StateClosed, ex);
                }
                return;
            }

            if(_acmTimeout > 0)
            {
                _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
            }
            
            current.ioCompleted();
        }
        
        if(_dispatcher != null)
        {
            if(info != null)
            {
                //
                // Create a new stream for the dispatch instead of using the thread
                // pool's thread stream.
                //
                assert(info.stream == current.stream);
                IceInternal.BasicStream stream = info.stream;
                info.stream = new IceInternal.BasicStream(_instance, IceInternal.Protocol.currentProtocolEncoding);
                info.stream.swap(stream);
            }

            final StartCallback finalStartCB = startCB;
            final java.util.List<OutgoingMessage> finalSentCBs = sentCBs;
            final MessageInfo finalInfo = info;
            try
            {
                _dispatcher.dispatch(new Runnable()
                    {
                        public void
                        run()
                            {
                                dispatch(finalStartCB, finalSentCBs, finalInfo);
                            }
                    }, this);
            }
            catch(java.lang.Exception ex)
            {
                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                {
                    warning("dispatch exception", ex);
                }
            }
        }
        else
        {
            dispatch(startCB, sentCBs, info);
        }
    }

    protected void
    dispatch(StartCallback startCB, java.util.List<OutgoingMessage> sentCBs, MessageInfo info)
    {
        //
        // Notify the factory that the connection establishment and
        // validation has completed.
        //
        if(startCB != null)
        {
            startCB.connectionStartCompleted(this);
        }

        //
        // Notify AMI calls that the message was sent.
        //
        if(sentCBs != null)
        {
            for(OutgoingMessage msg : sentCBs)
            {
                msg.outAsync.__sent();
            }
        }

        if(info != null)
        {
            //
            // Asynchronous replies must be handled outside the thread
            // synchronization, so that nested calls are possible.
            //
            if(info.outAsync != null)
            {
                info.outAsync.__finished(info.stream);
            }

            if(info.invokeNum > 0)
            {
                //
                // Method invocation (or multiple invocations for batch messages)
                // must be done outside the thread synchronization, so that nested
                // calls are possible.
                //
                invokeAll(info.stream, info.invokeNum, info.requestId, info.compress, info.servantManager,
                          info.adapter);
            }
        }
        
        //
        // Decrease dispatch count.
        //
        if(sentCBs != null || info != null && info.outAsync != null)
        {
            synchronized(this)
            {
                if(--_dispatchCount == 0)
                {
                    //
                    // Only initiate shutdown if not already done. It
                    // might have already been done if the sent
                    // callback or AMI callback was dispatched when
                    // the connection was already in the closing
                    // state.
                    //
                    if(_state == StateClosing && !_shutdownInitiated)
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
                    else if(_state == StateFinished)
                    {
                        _reaper.add(this, _observer);
                    }
                    notifyAll();
                }
            }
        }
    }

    public void
    finished(IceInternal.ThreadPoolCurrent current)
    {
        //
        // Check if the connection needs to call user callbacks. If it doesn't, we
        // can safely run finish() from this "IO thread". Otherwise, we either run
        // finish() with the dispatcher if one is set, or we promote another IO 
        // thread first before calling finish().
        //
        if(_startCallback == null && _sendStreams.isEmpty() && _asyncRequests.isEmpty())
        {
            finish();
            return;
        }

        if(_dispatcher == null)
        {
            current.ioCompleted();
            finish();
        }
        else
        {
            try
            {
                _dispatcher.dispatch(new Runnable()
                    {
                        public void
                        run()
                        {
                            finish();
                        }
                    }, 
                    this);
            }
            catch(java.lang.Exception ex)
            {
                if(_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                {
                    warning("dispatch exception", ex);
                }
            }
        }
    }

    public void
    finish()
    {
        synchronized(this)
        {
            assert(_state == StateClosed);
            unscheduleTimeout(IceInternal.SocketOperation.Read | 
                              IceInternal.SocketOperation.Write |
                              IceInternal.SocketOperation.Connect);
        }

        if(_startCallback != null)
        {
            _startCallback.connectionStartFailed(this, _exception);
            _startCallback = null;
        }
        
        if(!_sendStreams.isEmpty())
        {
            if(!_writeStream.isEmpty())
            {
                //
                // Return the stream to the outgoing call. This is important for 
                // retriable AMI calls which are not marshalled again.
                //
                OutgoingMessage message = _sendStreams.getFirst();
                _writeStream.swap(message.stream);
            }
        
            //
            // NOTE: for twoway requests which are not sent, finished can be called twice: the 
            // first time because the outgoing is in the _sendStreams set and the second time 
            // because it's either in the _requests/_asyncRequests set. This is fine, only the 
            // first call should be taken into account by the implementation of finished.
            //
            for(OutgoingMessage p : _sendStreams)
            {
                if(p.requestId > 0)
                {
                    if(p.out != null) // Make sure finished isn't called twice.
                    {
                        _requests.remove(p.requestId);
                    }
                    else
                    {
                        _asyncRequests.remove(p.requestId);
                    }
                }
                p.finished(_exception);
            }
            _sendStreams.clear();
        }
        
        for(IceInternal.Outgoing p : _requests.values())
        {
            p.finished(_exception, true);
        }
        _requests.clear();

        for(IceInternal.OutgoingAsync p : _asyncRequests.values())
        {
            p.__finished(_exception, true);
        }
        _asyncRequests.clear();
        
        //
        // This must be done last as this will cause waitUntilFinished() to return (and communicator
        // objects such as the timer might be destroyed too).
        //
        synchronized(this)
        {
            setState(StateFinished);
            if(_dispatchCount == 0)
            {
                _reaper.add(this, _observer);
            }
        }
    }

    public String
    toString()
    {
        return _toString();
    }

    public java.nio.channels.SelectableChannel 
    fd()
    {
        return _transceiver.fd();
    }

    public boolean 
    hasMoreData()
    {
        return _hasMoreData.value;
    }

    public synchronized void
    timedOut()
    {
        if(_state <= StateNotValidated)
        {
            setState(StateClosed, new ConnectTimeoutException());
        }
        else if(_state < StateClosing)
        {
            setState(StateClosed, new TimeoutException());
        }
        else if(_state == StateClosing)
        {
            setState(StateClosed, new CloseTimeoutException());
        }
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

    public synchronized ConnectionInfo
    getInfo()
    {
        if(_state >= StateClosed)
        {
            throw (Ice.LocalException)_exception.fillInStackTrace();
        }
        return initConnectionInfo();
    }

    public String
    _toString()
    {
        return _desc; // No mutex lock, _desc is immutable.
    }

    public synchronized void
    exception(LocalException ex)
    {
        setState(StateClosed, ex);
    }

    public synchronized void
    invokeException(LocalException ex, int invokeNum)
    {
        //
        // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
        // called in case of a fatal exception we decrement _dispatchCount here.
        //

        setState(StateClosed, ex);

        if(invokeNum > 0)
        {
            assert(_dispatchCount > 0);
            _dispatchCount -= invokeNum;
            assert(_dispatchCount >= 0);
            if(_dispatchCount == 0)
            {
                if(_state == StateFinished)
                {
                    _reaper.add(this, _observer);
                }
                notifyAll();
            }
        }
    }

    public ConnectionI(Communicator communicator, IceInternal.Instance instance, IceInternal.ConnectionReaper reaper,
                       IceInternal.Transceiver transceiver, IceInternal.Connector connector,
                       IceInternal.EndpointI endpoint, ObjectAdapter adapter)
    {
        _communicator = communicator;
        _instance = instance;
        _reaper = reaper;
        _transceiver = transceiver;
        _desc = transceiver.toString();
        _type = transceiver.type();
        _connector = connector;
        _endpoint = endpoint;
        _adapter = adapter;
        final Ice.InitializationData initData = instance.initializationData();
        _dispatcher = initData.dispatcher; // Cached for better performance.
        _logger = initData.logger; // Cached for better performance.
        _traceLevels = instance.traceLevels(); // Cached for better performance.
        _timer = instance.timer();
        _writeTimeout = new TimeoutCallback();
        _writeTimeoutScheduled = false;
        _readTimeout = new TimeoutCallback();
        _readTimeoutScheduled = false;
        _warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
        _warnUdp = instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
        _cacheBuffers = instance.cacheMessageBuffers();
        _acmAbsoluteTimeoutMillis = 0;
        _nextRequestId = 1;
        _batchAutoFlush = initData.properties.getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0 ? true : false;
        _batchStream = new IceInternal.BasicStream(instance, IceInternal.Protocol.currentProtocolEncoding, 
                                                   _batchAutoFlush);
        _batchStreamInUse = false;
        _batchRequestNum = 0;
        _batchRequestCompress = false;
        _batchMarker = 0;
        _readStream = new IceInternal.BasicStream(instance, IceInternal.Protocol.currentProtocolEncoding);
        _readHeader = false;
        _readStreamPos = -1;
        _writeStream = new IceInternal.BasicStream(instance, IceInternal.Protocol.currentProtocolEncoding);
        _writeStreamPos = -1;
        _dispatchCount = 0;
        _state = StateNotInitialized;

        int compressionLevel = initData.properties.getPropertyAsIntWithDefault("Ice.Compression.Level", 1);
        if(compressionLevel < 1)
        {
            compressionLevel = 1;
        }
        else if(compressionLevel > 9)
        {
            compressionLevel = 9;
        }
        _compressionLevel = compressionLevel;

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
            if(_endpoint.datagram())
            {
                _acmTimeout = 0;
            }
            else
            {
                if(_adapter != null)
                {
                    _acmTimeout = ((ObjectAdapterI)_adapter).getACM();
                }
                else
                {
                    _acmTimeout = _instance.clientACM();
                }
            }

            if(_adapter != null)
            {
                _threadPool = ((ObjectAdapterI)_adapter).getThreadPool();
            }
            else
            {
                _threadPool = _instance.clientThreadPool();
            }
            _threadPool.initialize(this);
        }
        catch(Ice.LocalException ex)
        {
            throw ex;
        }
        catch(java.lang.Exception ex)
        {
            throw new Ice.SyscallException(ex);
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_startCallback == null);
            IceUtilInternal.Assert.FinalizerAssert(_state == StateFinished);
            IceUtilInternal.Assert.FinalizerAssert(_dispatchCount == 0);
            IceUtilInternal.Assert.FinalizerAssert(_sendStreams.isEmpty());
            IceUtilInternal.Assert.FinalizerAssert(_requests.isEmpty());
            IceUtilInternal.Assert.FinalizerAssert(_asyncRequests.isEmpty());
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private static final int StateNotInitialized = 0;
    private static final int StateNotValidated = 1;
    private static final int StateActive = 2;
    private static final int StateHolding = 3;
    private static final int StateClosing = 4;
    private static final int StateClosed = 5;
    private static final int StateFinished = 6;

    private void
    setState(int state, LocalException ex)
    {
        //
        // If setState() is called with an exception, then only closed
        // and closing states are permissible.
        //
        assert(state >= StateClosing);

        if(_state == state) // Don't switch twice.
        {
            return;
        }

        if(_exception == null)
        {
            _exception = ex;

            //
            // We don't warn if we are not validated.
            //
            if(_warn && _validated)
            {
                //
                // Don't warn about certain expected exceptions.
                //
                if(!(_exception instanceof CloseConnectionException ||
                     _exception instanceof ForcedCloseConnectionException ||
                     _exception instanceof ConnectionTimeoutException ||
                     _exception instanceof CommunicatorDestroyedException ||
                     _exception instanceof ObjectAdapterDeactivatedException ||
                     (_exception instanceof ConnectionLostException && _state == StateClosing)))
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

        //
        // Skip graceful shutdown if we are destroyed before validation.
        //
        if(_state <= StateNotValidated && state == StateClosing)
        {
            state = StateClosed;
        }

        if(_state == state) // Don't switch twice.
        {
            return;
        }

        try
        {
            switch(state)
            {
            case StateNotInitialized:
            {
                assert(false);
                break;
            }

            case StateNotValidated:
            {
                if(_state != StateNotInitialized)
                {
                    assert(_state == StateClosed);
                    return;
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
                _threadPool.register(this, IceInternal.SocketOperation.Read);
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
                if(_state == StateActive)
                {
                    _threadPool.unregister(this, IceInternal.SocketOperation.Read);
                }
                break;
            }

            case StateClosing:
            {
                //
                // Can't change back from closed.
                //
                if(_state >= StateClosed)
                {
                    return;
                }
                if(_state == StateHolding)
                {
                    // We need to continue to read in closing state.
                    _threadPool.register(this, IceInternal.SocketOperation.Read); 
                }
                break;
            }

            case StateClosed:
            {
                if(_state == StateFinished)
                {
                    return;
                }
                _threadPool.finish(this);
                break;
            }
            
            case StateFinished:
            {
                assert(_state == StateClosed);
                _transceiver.close();
                _communicator = null;
                break;
            }
            }
        }
        catch(Ice.LocalException ex)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            String s = "unexpected connection exception:\n " + _desc + "\n" + sw.toString();
            _instance.initializationData().logger.error(s);
        }

        //
        // We only register with the connection monitor if our new state
        // is StateActive. Otherwise we unregister with the connection
        // monitor, but only if we were registered before, i.e., if our
        // old state was StateActive.
        //
        if(_acmTimeout > 0)
        {
            if(state == StateActive)
            {
                _instance.connectionMonitor().add(this);
            }
            else if(_state == StateActive)
            {
                _instance.connectionMonitor().remove(this);
            }
        }

        if(_instance.getObserver() != null)
        {
            Ice.Instrumentation.ConnectionState oldState = toConnectionState(_state);
            Ice.Instrumentation.ConnectionState newState = toConnectionState(state);
            if(oldState != newState)
            {
                _observer = _instance.getObserver().getConnectionObserver(initConnectionInfo(),
                                                                          _endpoint, 
                                                                          newState,
                                                                          _observer);
                if(_observer != null)
                {
                    _observer.attach();
                }
                else
                {
                    _writeStreamPos = -1;
                    _readStreamPos = -1;
                }
            }
            if(_observer != null && state == StateClosed && _exception != null)
            {
                if(!(_exception instanceof CloseConnectionException ||
                     _exception instanceof ForcedCloseConnectionException ||
                     _exception instanceof ConnectionTimeoutException ||
                     _exception instanceof CommunicatorDestroyedException ||
                     _exception instanceof ObjectAdapterDeactivatedException ||
                     (_exception instanceof ConnectionLostException && _state == StateClosing)))
                {
                    _observer.failed(_exception.ice_name());
                }
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
        assert(!_shutdownInitiated);

        _shutdownInitiated = true;

        if(!_endpoint.datagram())
        {
            //
            // Before we shut down, we send a close connection
            // message.
            //
            IceInternal.BasicStream os = new IceInternal.BasicStream(_instance, 
                                                                     IceInternal.Protocol.currentProtocolEncoding);
            os.writeBlob(IceInternal.Protocol.magic);
            IceInternal.Protocol.currentProtocol.__write(os);
            IceInternal.Protocol.currentProtocolEncoding.__write(os);
            os.writeByte(IceInternal.Protocol.closeConnectionMsg);
            os.writeByte((byte)0); // compression status: always report 0 for CloseConnection in Java.
            os.writeInt(IceInternal.Protocol.headerSize); // Message size.

            if((sendMessage(new OutgoingMessage(os, false, false)) & IceInternal.AsyncStatus.Sent) > 0)
            {
                //
                // Schedule the close timeout to wait for the peer to close the connection. If
                // the message was queued for sending, sendNextMessage will schedule the timeout
                // once all messages were sent.
                //
                scheduleTimeout(IceInternal.SocketOperation.Write, closeTimeout());
            }

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

    private boolean
    initialize(int operation)
    {
        int s = _transceiver.initialize(_readStream.getBuffer(), _writeStream.getBuffer());
        if(s != IceInternal.SocketOperation.None)
        {
            scheduleTimeout(s, connectTimeout());
            _threadPool.update(this, operation, s);
            return false;
        }

        //
        // Update the connection description once the transceiver is initialized.
        //
        _desc = _transceiver.toString();
        setState(StateNotValidated);
        return true;
    }

    private boolean
    validate(int operation)
    {
        if(!_endpoint.datagram()) // Datagram connections are always implicitly validated.
        {
            if(_adapter != null) // The server side has the active role for connection validation.
            {
                if(_writeStream.size() == 0)
                {
                    _writeStream.writeBlob(IceInternal.Protocol.magic);
                    IceInternal.Protocol.currentProtocol.__write(_writeStream);
                    IceInternal.Protocol.currentProtocolEncoding.__write(_writeStream);
                    _writeStream.writeByte(IceInternal.Protocol.validateConnectionMsg);
                    _writeStream.writeByte((byte)0); // Compression status (always zero for validate connection).
                    _writeStream.writeInt(IceInternal.Protocol.headerSize); // Message size.
                    IceInternal.TraceUtil.traceSend(_writeStream, _logger, _traceLevels);
                    _writeStream.prepareWrite();
                }

                if(_observer != null)
                {
                    observerStartWrite(_writeStream.pos());
                }
                if(_writeStream.pos() != _writeStream.size() && !_transceiver.write(_writeStream.getBuffer()))
                {
                    scheduleTimeout(IceInternal.SocketOperation.Write, connectTimeout());
                    _threadPool.update(this, operation, IceInternal.SocketOperation.Write);
                    return false;
                }
                if(_observer != null)
                {
                    observerFinishWrite(_writeStream.pos());
                }
            }
            else // The client side has the passive role for connection validation.
            {
                if(_readStream.size() == 0)
                {
                    _readStream.resize(IceInternal.Protocol.headerSize, true);
                    _readStream.pos(0);
                }

                if(_observer != null)
                {
                    observerStartRead(_readStream.pos());
                }
                if(_readStream.pos() != _readStream.size() && !_transceiver.read(_readStream.getBuffer(), _hasMoreData))
                {
                    scheduleTimeout(IceInternal.SocketOperation.Read, connectTimeout());
                    _threadPool.update(this, operation, IceInternal.SocketOperation.Read);
                    return false;
                }
                if(_observer != null)
                {
                    observerFinishRead(_readStream.pos());
                }

                assert(_readStream.pos() == IceInternal.Protocol.headerSize);
                _readStream.pos(0);
                byte[] m = _readStream.readBlob(4);
                if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
                   m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                {
                    BadMagicException ex = new BadMagicException();
                    ex.badMagic = m;
                    throw ex;
                }

                _readProtocol.__read(_readStream);
                IceInternal.Protocol.checkSupportedProtocol(_readProtocol);
                
                _readProtocolEncoding.__read(_readStream);
                IceInternal.Protocol.checkSupportedProtocolEncoding(_readProtocolEncoding);
                
                byte messageType = _readStream.readByte();
                if(messageType != IceInternal.Protocol.validateConnectionMsg)
                {
                    throw new ConnectionNotValidatedException();
                }
                _readStream.readByte(); // Ignore compression status for validate connection.
                int size = _readStream.readInt();
                if(size != IceInternal.Protocol.headerSize)
                {
                    throw new IllegalMessageSizeException();
                }
                IceInternal.TraceUtil.traceRecv(_readStream, _logger, _traceLevels);

                _validated = true;
            }
        }

        _writeStream.resize(0, false);
        _writeStream.pos(0);

        _readStream.resize(IceInternal.Protocol.headerSize, true);
        _readHeader = true;
        _readStream.pos(0);
        
        return true;
    }

    private java.util.List<OutgoingMessage>
    sendNextMessage()
    {
        assert(!_sendStreams.isEmpty());
        assert(!_writeStream.isEmpty() && _writeStream.pos() == _writeStream.size());

        java.util.List<OutgoingMessage> callbacks = new java.util.LinkedList<OutgoingMessage>();
        try
        {
            while(true)
            {
                //
                // Notify the message that it was sent.
                //
                OutgoingMessage message = _sendStreams.getFirst();
                _writeStream.swap(message.stream);
                if(message.sent(this, true))
                {
                    callbacks.add(message);
                }
                _sendStreams.removeFirst();

                //
                // If there's nothing left to send, we're done.
                //
                if(_sendStreams.isEmpty())
                {
                    break;
                }

                //
                // If we are in the closed state, don't continue sending.
                //
                // The connection can be in the closed state if parseMessage
                // (called before sendNextMessage by message()) closes the
                // connection.
                // 
                if(_state >= StateClosed)
                {
                    return callbacks;
                }
        
                //
                // Otherwise, prepare the next message stream for writing.
                //
                message = _sendStreams.getFirst();
                assert(!message.prepared);
                IceInternal.BasicStream stream = message.stream;

                message.stream = doCompress(stream, message.compress);
                message.stream.prepareWrite();
                message.prepared = true;

                if(message.outAsync != null)
                {
                    IceInternal.TraceUtil.trace("sending asynchronous request", stream, _logger, _traceLevels);
                }
                else
                {
                    IceInternal.TraceUtil.traceSend(stream, _logger, _traceLevels);
                }
                _writeStream.swap(message.stream);

                //
                // Send the message.
                //
                if(_observer != null)
                {
                    observerStartWrite(_writeStream.pos());
                }
                if(_writeStream.pos() != _writeStream.size() && !_transceiver.write(_writeStream.getBuffer()))
                {
                    assert(!_writeStream.isEmpty());
                    scheduleTimeout(IceInternal.SocketOperation.Write, _endpoint.timeout());
                    return callbacks;
                }
                if(_observer != null)
                {
                    observerFinishWrite(_writeStream.pos());
                }
            }
        }
        catch(Ice.LocalException ex)
        {
            setState(StateClosed, ex);
            return callbacks;
        }

        assert(_writeStream.isEmpty());
        _threadPool.unregister(this, IceInternal.SocketOperation.Write);

        //
        // If all the messages were sent and we are in the closing state, we schedule 
        // the close timeout to wait for the peer to close the connection.
        //
        if(_state == StateClosing)
        {
            scheduleTimeout(IceInternal.SocketOperation.Write, closeTimeout());
        }

        return callbacks;
    }

    private int
    sendMessage(OutgoingMessage message)
    {
        assert(_state < StateClosed);
        if(!_sendStreams.isEmpty())
        {
            message.adopt();
            _sendStreams.addLast(message);
            return IceInternal.AsyncStatus.Queued;
        }

        //
        // Attempt to send the message without blocking. If the send blocks, we register
        // the connection with the selector thread or we request the caller to call
        // finishSendMessage() outside the synchronization.
        //

        assert(!message.prepared);

        IceInternal.BasicStream stream = message.stream;

        message.stream = doCompress(stream, message.compress);
        message.stream.prepareWrite();
        message.prepared = true;

        if(message.outAsync != null)
        {
            IceInternal.TraceUtil.trace("sending asynchronous request", stream, _logger, _traceLevels);
        }
        else
        {
            IceInternal.TraceUtil.traceSend(stream, _logger, _traceLevels);
        }

        if(_observer != null)
        {
            observerStartWrite(message.stream.pos());
        }
        if(_transceiver.write(message.stream.getBuffer()))
        {
            if(_observer != null)
            {
                observerFinishWrite(message.stream.pos());
            }
            int status = IceInternal.AsyncStatus.Sent;
            if(message.sent(this, false))
            {
                status |= IceInternal.AsyncStatus.InvokeSentCallback;
            }
            if(_acmTimeout > 0)
            {
                _acmAbsoluteTimeoutMillis = IceInternal.Time.currentMonotonicTimeMillis() + _acmTimeout * 1000;
            }
            return status;
        }
        message.adopt();

        _writeStream.swap(message.stream);
        _sendStreams.addLast(message);
        scheduleTimeout(IceInternal.SocketOperation.Write, _endpoint.timeout());
        _threadPool.register(this, IceInternal.SocketOperation.Write);
        return IceInternal.AsyncStatus.Queued;
    }

    private IceInternal.BasicStream
    doCompress(IceInternal.BasicStream uncompressed, boolean compress)
    {
        boolean compressionSupported = false;
        if(compress)
        {
            //
            // Don't check whether compression support is available unless the proxy
            // is configured for compression.
            //
            compressionSupported = IceInternal.BasicStream.compressible();
        }

        if(compressionSupported && uncompressed.size() >= 100)
        {
            //
            // Do compression.
            //
            IceInternal.BasicStream cstream = uncompressed.compress(IceInternal.Protocol.headerSize, _compressionLevel);
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

        uncompressed.pos(9);
        uncompressed.writeByte((byte)(compressionSupported ? 1 : 0));

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

    private MessageInfo 
    parseMessage(IceInternal.BasicStream stream)
    {
        assert(_state > StateNotValidated && _state < StateClosed);

        MessageInfo info = new MessageInfo(stream);

        _readStream.swap(info.stream);
        _readStream.resize(IceInternal.Protocol.headerSize, true);
        _readStream.pos(0);
        _readHeader = true;

        //
        // Connection is validated on first message. This is only used by
        // setState() to check wether or not we can print a connection
        // warning (a client might close the connection forcefully if the
        // connection isn't validated).
        //
        _validated = true;

        assert(info.stream.pos() == info.stream.size());

        try
        {
            //
            // We don't need to check magic and version here. This has already
            // been done by the ThreadPool which provides us with the stream.
            //
            assert(info.stream.pos() == info.stream.size());
            info.stream.pos(8);
            byte messageType = info.stream.readByte();
            info.compress = info.stream.readByte();
            if(info.compress == (byte)2)
            {
                if(IceInternal.BasicStream.compressible())
                {
                    info.stream = info.stream.uncompress(IceInternal.Protocol.headerSize);
                }
                else
                {
                    FeatureNotSupportedException ex = new FeatureNotSupportedException();
                    ex.unsupportedFeature = "Cannot uncompress compressed message: "
                        + "org.apache.tools.bzip2.CBZip2OutputStream was not found";
                    throw ex;
                }
            }
            info.stream.pos(IceInternal.Protocol.headerSize);

            switch(messageType)
            {
                case IceInternal.Protocol.closeConnectionMsg:
                {
                    IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                    if(_endpoint.datagram())
                    {
                        if(_warn)
                        {
                            _logger.warning("ignoring close connection message for datagram connection:\n" + _desc);
                        }
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
                        IceInternal.TraceUtil.trace("received request during closing\n" +
                                                    "(ignored by server, client will retry)",
                                                    info.stream, _logger, _traceLevels);
                    }
                    else
                    {
                        IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
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
                        IceInternal.TraceUtil.trace("received batch request during closing\n" +
                                                    "(ignored by server, client will retry)",
                                                    info.stream, _logger, _traceLevels);
                    }
                    else
                    {
                        IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        info.invokeNum = info.stream.readInt();
                        if(info.invokeNum < 0)
                        {
                            info.invokeNum = 0;
                            throw new UnmarshalOutOfBoundsException();
                        }
                        info.servantManager = _servantManager;
                        info.adapter = _adapter;
                        _dispatchCount += info.invokeNum;
                    }
                    break;
                }

                case IceInternal.Protocol.replyMsg:
                {
                    IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                    info.requestId = info.stream.readInt();
                    IceInternal.Outgoing out = _requests.remove(info.requestId);
                    if(out != null)
                    {
                        out.finished(info.stream);
                    }
                    else
                    {
                        info.outAsync = _asyncRequests.remove(info.requestId);
                        if(info.outAsync == null)
                        {
                            throw new UnknownRequestIdException();
                        }
                    }
                    notifyAll(); // Notify threads blocked in close(false)
                    break;
                }

                case IceInternal.Protocol.validateConnectionMsg:
                {
                    IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                    if(_warn)
                    {
                        _logger.warning("ignoring unexpected validate connection message:\n" + _desc);
                    }
                    break;
                }

                default:
                {
                    IceInternal.TraceUtil.trace("received unknown message\n(invalid, closing connection)",
                                                info.stream, _logger, _traceLevels);
                    throw new UnknownMessageException();
                }
            }
        }
        catch(LocalException ex)
        {
            if(_endpoint.datagram())
            {
                if(_warn)
                {
                    _logger.warning("datagram connection exception:\n" + ex + '\n' + _desc);
                }
            }
            else
            {
                setState(StateClosed, ex);
            }
        }

        return info;
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
                in = getIncoming(adapter, response, compress, requestId);

                //
                // Dispatch the invocation.
                //
                in.invoke(servantManager, stream);

                --invokeNum;

                reclaimIncoming(in);
                in = null;
            }

            stream.clear();
        }
        catch(LocalException ex)
        {
            invokeException(ex, invokeNum);
        }
        catch(java.lang.AssertionError ex) // Upon assertion, we print the stack trace.
        {
            UnknownException uex = new UnknownException(ex);
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            uex.unknown = sw.toString();
            _logger.error(uex.unknown);
            invokeException(uex, invokeNum);
        }
        catch(java.lang.OutOfMemoryError ex)
        {
            UnknownException uex = new UnknownException(ex);
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            uex.unknown = sw.toString();
            _logger.error(uex.unknown);
            invokeException(uex, invokeNum);
        }
        finally
        {
            if(in != null)
            {
                reclaimIncoming(in);
            }
        }
    }

    private void 
    scheduleTimeout(int status, int timeout)
    {
        if(timeout < 0)
        {
            return;
        }

        if((status & IceInternal.SocketOperation.Read) != 0)
        {
            _timer.schedule(_readTimeout, timeout);
            _readTimeoutScheduled = true;
        }
        if((status & (IceInternal.SocketOperation.Write | IceInternal.SocketOperation.Connect)) != 0)
        {
            _timer.schedule(_writeTimeout, timeout);
            _writeTimeoutScheduled = true;
        }
    }

    private void
    unscheduleTimeout(int status)
    {
        if((status & IceInternal.SocketOperation.Read) != 0 && _readTimeoutScheduled)
        {
            _timer.cancel(_readTimeout);
            _readTimeoutScheduled = false;
        }
        if((status & (IceInternal.SocketOperation.Write | IceInternal.SocketOperation.Connect)) != 0 &&
           _writeTimeoutScheduled)
        {
            _timer.cancel(_writeTimeout);
            _writeTimeoutScheduled = false;
        }
    }

    private int
    connectTimeout()
    {
        IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        if(defaultsAndOverrides.overrideConnectTimeout)
        {
            return defaultsAndOverrides.overrideConnectTimeoutValue;
        }
        else
        {
            return _endpoint.timeout();
        }
    }

    private int
    closeTimeout()
    {
        IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        if(defaultsAndOverrides.overrideCloseTimeout)
        {
            return defaultsAndOverrides.overrideCloseTimeoutValue;
        }
        else
        {
            return _endpoint.timeout();
        }
    }

    private ConnectionInfo
    initConnectionInfo()
    {
        if(_info != null)
        {
            return _info;
        }

        ConnectionInfo info = _transceiver.getInfo();
        info.connectionId = _endpoint.connectionId();
        info.adapterName = _adapter != null ? _adapter.getName() : "";
        info.incoming = _connector == null;
        if(_state > StateNotInitialized)
        {
            _info = info; // Cache the connection information only if initialized.
        }
        return info;
    }

    private Ice.Instrumentation.ConnectionState
    toConnectionState(int state)
    {
        return connectionStateMap[state];
    }
    
    private void
    warning(String msg, Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        String s = msg + ":\n" + _desc + "\n" + sw.toString();
        _logger.warning(s);
    }

    private void
    observerStartRead(int pos)
    {
        if(_readStreamPos >= 0)
        {
            _observer.receivedBytes(pos - _readStreamPos);
        }
        _readStreamPos = pos;
    }

    private void
    observerFinishRead(int pos)
    {
        if(_readStreamPos == -1)
        {
            return;
        }
        assert(pos >= _readStreamPos);
        _observer.receivedBytes(pos - _readStreamPos);
        _readStreamPos = -1;
    }

    private void
    observerStartWrite(int pos)
    {
        if(_writeStreamPos >= 0)
        {
            _observer.sentBytes(pos - _writeStreamPos);
        }
        _writeStreamPos = pos;
    }

    private void
    observerFinishWrite(int pos)
    {
        if(_writeStreamPos == -1)
        {
            return;
        }
        assert(pos >= _writeStreamPos);
        _observer.sentBytes(pos - _writeStreamPos);
        _writeStreamPos = -1;
    }

    private IceInternal.Incoming
    getIncoming(ObjectAdapter adapter, boolean response, byte compress, int requestId)
    {
        IceInternal.Incoming in = null;

        if(_cacheBuffers > 0)
        {
            synchronized(_incomingCacheMutex)
            {
                if(_incomingCache == null)
                {
                    in = new IceInternal.Incoming(_instance, this, adapter, response, compress, requestId);
                }
                else
                {
                    in = _incomingCache;
                    _incomingCache = _incomingCache.next;
                    in.reset(_instance, this, adapter, response, compress, requestId);
                    in.next = null;
                }
            }
        }
        else
        {
            in = new IceInternal.Incoming(_instance, this, adapter, response, compress, requestId);
        }

        return in;
    }

    private void
    reclaimIncoming(IceInternal.Incoming in)
    {
        if(_cacheBuffers > 0)
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
    }

    public IceInternal.Outgoing
    getOutgoing(IceInternal.RequestHandler handler, String operation, OperationMode mode,
                java.util.Map<String, String> context, InvocationObserver observer)
        throws IceInternal.LocalExceptionWrapper
    {
        IceInternal.Outgoing out = null;

        if(_cacheBuffers > 0)
        {
            synchronized(_outgoingCacheMutex)
            {
                if(_outgoingCache == null)
                {
                    out = new IceInternal.Outgoing(handler, operation, mode, context, observer);
                }
                else
                {
                    out = _outgoingCache;
                    _outgoingCache = _outgoingCache.next;
                    out.reset(handler, operation, mode, context, observer);
                    out.next = null;
                }
            }
        }
        else
        {
            out = new IceInternal.Outgoing(handler, operation, mode, context, observer);
        }

        return out;
    }

    public void
    reclaimOutgoing(IceInternal.Outgoing out)
    {
        if(_cacheBuffers > 0)
        {
            //
            // Clear references to Ice objects as soon as possible.
            //
            out.reclaim();

            synchronized(_outgoingCacheMutex)
            {
                out.next = _outgoingCache;
                _outgoingCache = out;
            }
        }
    }

    private static class OutgoingMessage
    {
        OutgoingMessage(IceInternal.BasicStream stream, boolean compress, boolean adopt)
        {
            this.stream = stream;
            this.compress = compress;
            this.adopt = adopt;
            this.isSent = false;
            this.requestId = 0;
        }

        OutgoingMessage(IceInternal.OutgoingMessageCallback out, IceInternal.BasicStream stream, boolean compress,
                        int requestId)
        {
            this.stream = stream;
            this.compress = compress;
            this.out = out;
            this.requestId = requestId;
            this.isSent = false;
        }

        OutgoingMessage(IceInternal.OutgoingAsyncMessageCallback out, IceInternal.BasicStream stream, boolean compress,
                        int requestId)
        {
            this.stream = stream;
            this.compress = compress;
            this.outAsync = out;
            this.requestId = requestId;
            this.isSent = false;
        }

        public void
        adopt()
        {
            if(adopt)
            {
                IceInternal.BasicStream stream =
                    new IceInternal.BasicStream(this.stream.instance(), IceInternal.Protocol.currentProtocolEncoding);
                stream.swap(this.stream);
                this.stream = stream;
                adopt = false;
            }
        }

        public boolean
        sent(ConnectionI connection, boolean notify)
        {
            isSent = true; // The message is sent.

            if(out != null)
            {
                out.sent(notify); // true = notify the waiting thread that the request was sent.
                return false;
            }
            else if(outAsync != null)
            {
                return outAsync.__sent(connection);
            }
            else
            {
                return false;
            }
        }

        public void
        finished(Ice.LocalException ex)
        {
            if(out != null)
            {
                out.finished(ex, isSent);
            }
            else if(outAsync != null)
            {
                outAsync.__finished(ex, isSent);
            }
        }

        public IceInternal.BasicStream stream;
        public IceInternal.OutgoingMessageCallback out;
        public IceInternal.OutgoingAsyncMessageCallback outAsync;
        public boolean compress;
        public int requestId;
        boolean adopt;
        boolean prepared;
        boolean isSent;
    }

    private Communicator _communicator;
    private final IceInternal.Instance _instance;
    private final IceInternal.ConnectionReaper _reaper;
    private final IceInternal.Transceiver _transceiver;
    private String _desc;
    private final String _type;
    private final IceInternal.Connector _connector;
    private final IceInternal.EndpointI _endpoint;

    private ObjectAdapter _adapter;
    private IceInternal.ServantManager _servantManager;

    private final Dispatcher _dispatcher;
    private final Logger _logger;
    private final IceInternal.TraceLevels _traceLevels;
    private final IceInternal.ThreadPool _threadPool;

    private final IceInternal.Timer _timer;
    private final IceInternal.TimerTask _writeTimeout;
    private boolean _writeTimeoutScheduled;
    private final IceInternal.TimerTask _readTimeout;
    private boolean _readTimeoutScheduled;

    private StartCallback _startCallback = null;
    private Ice.BooleanHolder _hasMoreData = new Ice.BooleanHolder(false);

    private final boolean _warn;
    private final boolean _warnUdp;
    private final long _acmTimeout;
    private long _acmAbsoluteTimeoutMillis;

    private final int _compressionLevel;

    private int _nextRequestId;

    private java.util.Map<Integer, IceInternal.Outgoing> _requests =
        new java.util.HashMap<Integer, IceInternal.Outgoing>();
    private java.util.Map<Integer, IceInternal.OutgoingAsync> _asyncRequests =
        new java.util.HashMap<Integer, IceInternal.OutgoingAsync>();

    private LocalException _exception;

    private boolean _batchAutoFlush;
    private IceInternal.BasicStream _batchStream;
    private boolean _batchStreamInUse;
    private int _batchRequestNum;
    private boolean _batchRequestCompress;
    private int _batchMarker;

    private java.util.LinkedList<OutgoingMessage> _sendStreams = new java.util.LinkedList<OutgoingMessage>();

    private IceInternal.BasicStream _readStream;
    private boolean _readHeader;
    private IceInternal.BasicStream _writeStream;

    private Ice.Instrumentation.ConnectionObserver _observer;
    private int _readStreamPos;
    private int _writeStreamPos;

    private int _dispatchCount;

    private int _state; // The current state.
    private boolean _shutdownInitiated = false;
    private boolean _validated = false;

    private IceInternal.Incoming _incomingCache;
    private java.lang.Object _incomingCacheMutex = new java.lang.Object();

    private IceInternal.Outgoing _outgoingCache;
    private java.lang.Object _outgoingCacheMutex = new java.lang.Object();

    private Ice.ProtocolVersion _readProtocol = new Ice.ProtocolVersion();
    private Ice.EncodingVersion _readProtocolEncoding = new Ice.EncodingVersion();

    private int _cacheBuffers;

    private Ice.ConnectionInfo _info;

    private static Ice.Instrumentation.ConnectionState connectionStateMap[] = {
        Ice.Instrumentation.ConnectionState.ConnectionStateValidating,   // StateNotInitialized
        Ice.Instrumentation.ConnectionState.ConnectionStateValidating,   // StateNotValidated
        Ice.Instrumentation.ConnectionState.ConnectionStateActive,       // StateActive
        Ice.Instrumentation.ConnectionState.ConnectionStateHolding,      // StateHolding
        Ice.Instrumentation.ConnectionState.ConnectionStateClosing,      // StateClosing
        Ice.Instrumentation.ConnectionState.ConnectionStateClosed,       // StateClosed
        Ice.Instrumentation.ConnectionState.ConnectionStateClosed,       // StateFinished
    };
}
