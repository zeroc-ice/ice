// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

import java.util.concurrent.Callable;

import com.zeroc.IceInternal.AsyncStatus;
import com.zeroc.IceInternal.Buffer;
import com.zeroc.IceInternal.Incoming;
import com.zeroc.IceInternal.OutgoingAsyncBase;
import com.zeroc.IceInternal.Protocol;
import com.zeroc.IceInternal.SocketOperation;
import com.zeroc.IceInternal.Time;
import com.zeroc.IceInternal.TraceUtil;
import com.zeroc.Ice.Instrumentation.ConnectionState;

public final class ConnectionI extends com.zeroc.IceInternal.EventHandler
    implements Connection, com.zeroc.IceInternal.ResponseHandler, com.zeroc.IceInternal.CancellationHandler
{
    public interface StartCallback
    {
        void connectionStartCompleted(ConnectionI connection);

        void connectionStartFailed(ConnectionI connection, LocalException ex);
    }

    private class TimeoutCallback implements Runnable
    {
        @Override
        public void run()
        {
            timedOut();
        }
    }

    public void start(StartCallback callback)
    {
        try
        {
            synchronized(this)
            {
                // The connection might already be closed if the communicator
                // was destroyed.
                if(_state >= StateClosed)
                {
                    assert (_exception != null);
                    throw (LocalException) _exception.fillInStackTrace();
                }

                if(!initialize(SocketOperation.None) || !validate(SocketOperation.None))
                {
                    _startCallback = callback;
                    return;
                }

                //
                // We start out in holding state.
                //
                setState(StateHolding);
            }
        }
        catch(LocalException ex)
        {
            exception(ex);
            callback.connectionStartFailed(this, _exception);
            return;
        }

        callback.connectionStartCompleted(this);
    }

    public void startAndWait() throws InterruptedException
    {
        try
        {
            synchronized(this)
            {
                // The connection might already be closed if the communicator
                // was destroyed.
                if(_state >= StateClosed)
                {
                    assert (_exception != null);
                    throw (LocalException) _exception.fillInStackTrace();
                }

                if(!initialize(SocketOperation.None) || !validate(SocketOperation.None))
                {
                    while(_state <= StateNotValidated)
                    {
                        wait();
                    }

                    if(_state >= StateClosing)
                    {
                        assert (_exception != null);
                        throw (LocalException) _exception.fillInStackTrace();
                    }
                }

                //
                // We start out in holding state.
                //
                setState(StateHolding);
            }
        }
        catch(LocalException ex)
        {
            exception(ex);
            waitUntilFinished();
        }
    }

    public synchronized void activate()
    {
        if(_state <= StateNotValidated)
        {
            return;
        }

        if(_acmLastActivity > 0)
        {
            _acmLastActivity = Time.currentMonotonicTimeMillis();
        }

        setState(StateActive);
    }

    public synchronized void hold()
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

    synchronized public void destroy(int reason)
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

    @Override
    public void close(final ConnectionClose mode)
    {
        if(Thread.interrupted())
        {
            throw new OperationInterruptedException();
        }

        if(_instance.queueRequests())
        {
            _instance.getQueueExecutor().executeNoThrow(new Callable<Void>()
            {
                @Override
                public Void call()
                    throws Exception
                {
                    closeImpl(mode);
                    return null;
                }
            });
        }
        else
        {
            closeImpl(mode);
        }
    }

    synchronized private void closeImpl(ConnectionClose mode)
    {
        if(mode == ConnectionClose.Forcefully)
        {
            setState(StateClosed, new ConnectionManuallyClosedException(false));
        }
        else if(mode == ConnectionClose.Gracefully)
        {
            setState(StateClosing, new ConnectionManuallyClosedException(true));
        }
        else
        {
            assert(mode == ConnectionClose.GracefullyWithWait);

            //
            // Wait until all outstanding requests have been completed.
            //
            while(!_asyncRequests.isEmpty())
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                    throw new OperationInterruptedException();
                }
            }

            setState(StateClosing, new ConnectionManuallyClosedException(true));
        }
    }

    public synchronized boolean isActiveOrHolding()
    {
        return _state > StateNotValidated && _state < StateClosing;
    }

    public synchronized boolean isFinished()
    {
        if(_state != StateFinished || _dispatchCount != 0)
        {
            return false;
        }

        assert (_state == StateFinished);
        return true;
    }

    public synchronized void throwException()
    {
        if(_exception != null)
        {
            assert (_state >= StateClosing);
            throw (LocalException) _exception.fillInStackTrace();
        }
    }

    public synchronized void waitUntilHolding() throws InterruptedException
    {
        while(_state < StateHolding || _dispatchCount > 0)
        {
            wait();
        }
    }

    public synchronized void waitUntilFinished() throws InterruptedException
    {
        //
        // We wait indefinitely until the connection is finished and all
        // outstanding requests are completed. Otherwise we couldn't
        // guarantee that there are no outstanding calls when deactivate()
        // is called on the servant locators.
        //
        while(_state < StateFinished || _dispatchCount > 0)
        {
            wait();
        }

        assert (_state == StateFinished);

        //
        // Clear the OA. See bug 1673 for the details of why this is necessary.
        //
        _adapter = null;
    }

    synchronized public void updateObserver()
    {
        if(_state < StateNotValidated || _state > StateClosed)
        {
            return;
        }

        assert (_instance.initializationData().observer != null);
        _observer = _instance.initializationData().observer.getConnectionObserver(initConnectionInfo(),
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

    synchronized public void monitor(long now, com.zeroc.IceInternal.ACMConfig acm)
    {
        if(_state != StateActive)
        {
            return;
        }

        //
        // We send a heartbeat if there was no activity in the last
        // (timeout / 4) period. Sending a heartbeat sooner than
        // really needed is safer to ensure that the receiver will
        // receive the heartbeat in time. Sending the heartbeat if
        // there was no activity in the last (timeout / 2) period
        // isn't enough since monitor() is called only every (timeout
        // / 2) period.
        //
        // Note that this doesn't imply that we are sending 4
        // heartbeats per timeout period because the monitor() method
        // is still only called every (timeout / 2) period.
        //
        if(acm.heartbeat == ACMHeartbeat.HeartbeatAlways ||
           (acm.heartbeat != ACMHeartbeat.HeartbeatOff && _writeStream.isEmpty() &&
            now >= (_acmLastActivity + acm.timeout / 4)))
        {
            if(acm.heartbeat != ACMHeartbeat.HeartbeatOnInvocation || _dispatchCount > 0)
            {
                sendHeartbeatNow();
            }
        }

        if(_readStream.size() > Protocol.headerSize || !_writeStream.isEmpty())
        {
            //
            // If writing or reading, nothing to do, the connection
            // timeout will kick-in if writes or reads don't progress.
            // This check is necessary because the activity timer is
            // only set when a message is fully read/written.
            //
            return;
        }

        if(acm.close != ACMClose.CloseOff && now >= (_acmLastActivity + acm.timeout))
        {
            if(acm.close == ACMClose.CloseOnIdleForceful ||
               (acm.close != ACMClose.CloseOnIdle && (!_asyncRequests.isEmpty())))
            {
                //
                // Close the connection if we didn't receive a heartbeat in
                // the last period.
                //
                setState(StateClosed, new ConnectionTimeoutException());
            }
            else if(acm.close != ACMClose.CloseOnInvocation && _dispatchCount == 0 && _batchRequestQueue.isEmpty() &&
                    _asyncRequests.isEmpty())
            {
                //
                // The connection is idle, close it.
                //
                setState(StateClosing, new ConnectionTimeoutException());
            }
        }
    }

    synchronized public int
    sendAsyncRequest(OutgoingAsyncBase out, boolean compress, boolean response, int batchRequestNum)
            throws com.zeroc.IceInternal.RetryException
    {
        final OutputStream os = out.getOs();

        if(_exception != null)
        {
            //
            // If the connection is closed before we even have a chance
            // to send our request, we always try to send the request
            // again.
            //
            throw new com.zeroc.IceInternal.RetryException((LocalException) _exception.fillInStackTrace());
        }

        assert (_state > StateNotValidated);
        assert (_state < StateClosing);

        //
        // Ensure the message isn't bigger than what we can send with the
        // transport.
        //
        _transceiver.checkSendSize(os.getBuffer());

        //
        // Notify the request that it's cancelable with this connection.
        // This will throw if the request is canceled.
        //
        out.cancelable(this);

        int requestId = 0;
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
            os.pos(Protocol.headerSize);
            os.writeInt(requestId);
        }
        else if(batchRequestNum > 0)
        {
            os.pos(Protocol.headerSize);
            os.writeInt(batchRequestNum);
        }

        out.attachRemoteObserver(initConnectionInfo(), _endpoint, requestId);

        int status;
        try
        {
            status = sendMessage(new OutgoingMessage(out, os, compress, requestId));
        }
        catch(LocalException ex)
        {
            setState(StateClosed, ex);
            assert (_exception != null);
            throw (LocalException) _exception.fillInStackTrace();
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

    public com.zeroc.IceInternal.BatchRequestQueue
    getBatchRequestQueue()
    {
        return _batchRequestQueue;
    }

    @Override
    public void flushBatchRequests(CompressBatch compressBatch)
    {
        ObjectPrx.waitForResponseForCompletion(flushBatchRequestsAsync(compressBatch));
    }

    @Override
    public java.util.concurrent.CompletableFuture<Void> flushBatchRequestsAsync(CompressBatch compressBatch)
    {
        com.zeroc.IceInternal.ConnectionFlushBatch f =
            new com.zeroc.IceInternal.ConnectionFlushBatch(this, _communicator, _instance);
        f.invoke(compressBatch);
        return f;
    }

    @Override
    synchronized public void setCloseCallback(final CloseCallback callback)
    {
        if(_state >= StateClosed)
        {
            if(callback != null)
            {
                _threadPool.dispatch(new com.zeroc.IceInternal.DispatchWorkItem(this)
                {
                    @Override
                    public void run()
                    {
                        try
                        {
                            callback.closed(ConnectionI.this);
                        }
                        catch(Exception ex)
                        {
                            _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                        }
                    }
                });
            }
        }
        else
        {
            _closeCallback = callback;
        }
    }

    @Override
    synchronized public void setHeartbeatCallback(final HeartbeatCallback callback)
    {
        _heartbeatCallback = callback;
    }

    @Override
    public void heartbeat()
    {
        ObjectPrx.waitForResponseForCompletion(heartbeatAsync());
    }

    private class HeartbeatAsync extends com.zeroc.IceInternal.OutgoingAsyncBaseI<Void>
    {
        public HeartbeatAsync(Communicator communicator, com.zeroc.IceInternal.Instance instance)
        {
            super(communicator, instance, "heartbeat");
        }

        @Override
        public Connection getConnection()
        {
            return ConnectionI.this;
        }

        @Override
        protected void markSent()
        {
            super.markSent();

            assert((_state & StateOK) != 0);
            complete(null);
        }

        @Override
        protected void markCompleted()
        {
            if(_exception != null)
            {
                completeExceptionally(_exception);
            }
            super.markCompleted();
        }

        public void invoke()
        {
            try
            {
                _os.writeBlob(Protocol.magic);
                ProtocolVersion.ice_write(_os, Protocol.currentProtocol);
                EncodingVersion.ice_write(_os, Protocol.currentProtocolEncoding);
                _os.writeByte(Protocol.validateConnectionMsg);
                _os.writeByte((byte) 0);
                _os.writeInt(Protocol.headerSize); // Message size.

                int status;
                if(_instance.queueRequests())
                {
                    status = _instance.getQueueExecutor().execute(new Callable<Integer>()
                    {
                        @Override
                        public Integer call()
                            throws com.zeroc.IceInternal.RetryException
                        {
                            return ConnectionI.this.sendAsyncRequest(HeartbeatAsync.this, false, false, 0);
                        }
                    });
                }
                else
                {
                    status = ConnectionI.this.sendAsyncRequest(this, false, false, 0);
                }

                if((status & AsyncStatus.Sent) > 0)
                {
                    _sentSynchronously = true;
                    if((status & AsyncStatus.InvokeSentCallback) > 0)
                    {
                        invokeSent();
                    }
                }
            }
            catch(com.zeroc.IceInternal.RetryException ex)
            {
                if(completed(ex.get()))
                {
                    invokeCompletedAsync();
                }
            }
            catch(com.zeroc.Ice.Exception ex)
            {
                if(completed(ex))
                {
                    invokeCompletedAsync();
                }
            }
        }
    }

    @Override
    public java.util.concurrent.CompletableFuture<Void> heartbeatAsync()
    {
        HeartbeatAsync __f = new HeartbeatAsync(_communicator, _instance);
        __f.invoke();
        return __f;
    }

    @Override
    synchronized public void setACM(java.util.OptionalInt timeout, java.util.Optional<ACMClose> close,
            java.util.Optional<ACMHeartbeat> heartbeat)
    {
        if(_monitor == null || _state >= StateClosed)
        {
            return;
        }

        if(_state == StateActive)
        {
            _monitor.remove(this);
        }
        _monitor = _monitor.acm(timeout, close, heartbeat);

        if(_monitor.getACM().timeout <= 0)
        {
            _acmLastActivity = -1; // Disable the recording of last activity.
        }
        else if(_state == StateActive && _acmLastActivity == -1)
        {
            _acmLastActivity = Time.currentMonotonicTimeMillis();
        }

        if(_state == StateActive)
        {
            _monitor.add(this);
        }
    }

    @Override
    synchronized public ACM getACM()
    {
        return _monitor != null ? _monitor.getACM() : new ACM(0, ACMClose.CloseOff, ACMHeartbeat.HeartbeatOff);
    }

    @Override
    synchronized public void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex)
    {
        if(_state >= StateClosed)
        {
            return; // The request has already been or will be shortly notified of the failure.
        }

        java.util.Iterator<OutgoingMessage> it = _sendStreams.iterator();
        while(it.hasNext())
        {
            OutgoingMessage o = it.next();
            if(o.outAsync == outAsync)
            {
                if(o.requestId > 0)
                {
                    _asyncRequests.remove(o.requestId);
                }

                if(ex instanceof ConnectionTimeoutException)
                {
                    setState(StateClosed, ex);
                }
                else
                {
                    //
                    // If the request is being sent, don't remove it from the send
                    // streams, it will be removed once the sending is finished.
                    //
                    // Note that since we swapped the message stream to _writeStream
                    // it's fine if the OutgoingAsync output stream is released (and
                    // as long as canceled requests cannot be retried).
                    //
                    o.canceled();
                    if(o != _sendStreams.getFirst())
                    {
                        it.remove();
                    }
                    if(outAsync.completed(ex))
                    {
                        outAsync.invokeCompletedAsync();
                    }
                }
                return;
            }
        }

        if(outAsync instanceof com.zeroc.IceInternal.OutgoingAsync)
        {
            java.util.Iterator<OutgoingAsyncBase> it2 = _asyncRequests.values().iterator();
            while(it2.hasNext())
            {
                if(it2.next() == outAsync)
                {
                    if(ex instanceof ConnectionTimeoutException)
                    {
                        setState(StateClosed, ex);
                    }
                    else
                    {
                        it2.remove();
                        if(outAsync.completed(ex))
                        {
                            outAsync.invokeCompletedAsync();
                        }
                    }
                    return;
                }
            }
        }
    }

    @Override
    public void sendResponse(int requestId, OutputStream os, byte compressFlag, boolean amd)
    {
        //
        // We may be executing on the "main thread" (e.g., in Android together with a custom dispatcher)
        // and therefore we have to defer network calls to a separate thread.
        //
        final boolean queueResponse = _instance.queueRequests();

        synchronized(this)
        {
            assert(_state > StateNotValidated);

            if(!queueResponse)
            {
                sendResponseImpl(os, compressFlag);
            }
        }

        if(queueResponse)
        {
            _instance.getQueueExecutor().executeNoThrow(new Callable<Void>()
            {
                @Override
                public Void call()
                    throws Exception
                {
                    sendResponseImpl(os, compressFlag);
                    return null;
                }
            });
        }
    }

    private synchronized void sendResponseImpl(OutputStream os, byte compressFlag)
    {
        try
        {
            if(--_dispatchCount == 0)
            {
                if(_state == StateFinished)
                {
                    reap();
                }
                notifyAll();
            }

            if(_state < StateClosed)
            {
                sendMessage(new OutgoingMessage(os, compressFlag != 0, true));

                if(_state == StateClosing && _dispatchCount == 0)
                {
                    initiateShutdown();
                }
            }
        }
        catch(LocalException ex)
        {
            setState(StateClosed, ex);
        }
    }

    @Override
    public void sendNoResponse()
    {
        boolean shutdown = false;

        synchronized(this)
        {
            assert (_state > StateNotValidated);
            try
            {
                if(--_dispatchCount == 0)
                {
                    if(_state == StateFinished)
                    {
                        reap();
                    }
                    notifyAll();
                }

                if(_state >= StateClosed)
                {
                    assert (_exception != null);
                    throw (LocalException) _exception.fillInStackTrace();
                }

                if(_state == StateClosing && _dispatchCount == 0)
                {
                    //
                    // We may be executing on the "main thread" (e.g., in Android together with a custom dispatcher)
                    // and therefore we have to defer network calls to a separate thread.
                    //
                    if(_instance.queueRequests())
                    {
                        shutdown = true;
                    }
                    else
                    {
                        initiateShutdown();
                    }
                }
            }
            catch(LocalException ex)
            {
                setState(StateClosed, ex);
            }
        }

        if(shutdown)
        {
            queueShutdown(false);
        }
    }

    @Override
    public boolean systemException(int requestId, SystemException ex, boolean amd)
    {
        return false; // System exceptions aren't marshalled.
    }

    @Override
    public synchronized void invokeException(int requestId, LocalException ex, int invokeNum, boolean amd)
    {
        //
        // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
        // called in case of a fatal exception we decrement _dispatchCount here.
        //

        setState(StateClosed, ex);

        if(invokeNum > 0)
        {
            assert (_dispatchCount > 0);
            _dispatchCount -= invokeNum;
            assert (_dispatchCount >= 0);
            if(_dispatchCount == 0)
            {
                if(_state == StateFinished)
                {
                    reap();
                }
                notifyAll();
            }
        }
    }

    public com.zeroc.IceInternal.EndpointI endpoint()
    {
        return _endpoint; // No mutex protection necessary, _endpoint is
                          // immutable.
    }

    public com.zeroc.IceInternal.Connector connector()
    {
        return _connector; // No mutex protection necessary, _connector is
                           // immutable.
    }

    @Override
    public synchronized void setAdapter(ObjectAdapter adapter)
    {
        if(_state <= StateNotValidated || _state >= StateClosing)
        {
            return;
        }

        _adapter = adapter;

        if(_adapter != null)
        {
            _servantManager = ((ObjectAdapterI) _adapter).getServantManager();
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

    @Override
    public synchronized ObjectAdapter getAdapter()
    {
        return _adapter;
    }

    @Override
    public Endpoint getEndpoint()
    {
        return _endpoint; // No mutex protection necessary, _endpoint is
                          // immutable.
    }

    @Override
    public ObjectPrx createProxy(Identity ident)
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
    @Override
    public void message(com.zeroc.IceInternal.ThreadPoolCurrent current)
    {
        StartCallback startCB = null;
        java.util.List<OutgoingMessage> sentCBs = null;
        MessageInfo info = null;
        int dispatchCount = 0;

        synchronized(this)
        {
            if(_state >= StateClosed)
            {
                return;
            }

            if(!current.ioReady())
            {
                return;
            }

            int readyOp = current.operation;
            try
            {
                unscheduleTimeout(current.operation);

                int writeOp = SocketOperation.None;
                int readOp = SocketOperation.None;

                if((readyOp & SocketOperation.Write) != 0)
                {
                    final Buffer buf = _writeStream.getBuffer();
                    if(_observer != null)
                    {
                        observerStartWrite(buf);
                    }
                    writeOp = write(buf);
                    if(_observer != null && (writeOp & SocketOperation.Write) == 0)
                    {
                        observerFinishWrite(buf);
                    }
                }

                while((readyOp & SocketOperation.Read) != 0)
                {
                    final Buffer buf = _readStream.getBuffer();
                    if(_observer != null && !_readHeader)
                    {
                        observerStartRead(buf);
                    }

                    readOp = read(buf);
                    if((readOp & SocketOperation.Read) != 0)
                    {
                        break;
                    }
                    if(_observer != null && !_readHeader)
                    {
                        assert (!buf.b.hasRemaining());
                        observerFinishRead(buf);
                    }

                    if(_readHeader) // Read header if necessary.
                    {
                        _readHeader = false;

                        if(_observer != null)
                        {
                            _observer.receivedBytes(Protocol.headerSize);
                        }

                        int pos = _readStream.pos();
                        if(pos < Protocol.headerSize)
                        {
                            //
                            // This situation is possible for small UDP packets.
                            //
                            throw new IllegalMessageSizeException();
                        }

                        _readStream.pos(0);
                        byte[] m = new byte[4];
                        m[0] = _readStream.readByte();
                        m[1] = _readStream.readByte();
                        m[2] = _readStream.readByte();
                        m[3] = _readStream.readByte();
                        if(m[0] != Protocol.magic[0] ||
                           m[1] != Protocol.magic[1] ||
                           m[2] != Protocol.magic[2] ||
                           m[3] != Protocol.magic[3])
                        {
                            BadMagicException ex = new BadMagicException();
                            ex.badMagic = m;
                            throw ex;
                        }

                        _readProtocol.ice_readMembers(_readStream);
                        Protocol.checkSupportedProtocol(_readProtocol);

                        _readProtocolEncoding.ice_readMembers(_readStream);
                        Protocol.checkSupportedProtocolEncoding(_readProtocolEncoding);

                        _readStream.readByte(); // messageType
                        _readStream.readByte(); // compress
                        int size = _readStream.readInt();
                        if(size < Protocol.headerSize)
                        {
                            throw new IllegalMessageSizeException();
                        }
                        if(size > _messageSizeMax)
                        {
                            com.zeroc.IceInternal.Ex.throwMemoryLimitException(size, _messageSizeMax);
                        }
                        if(size > _readStream.size())
                        {
                            _readStream.resize(size);
                        }
                        _readStream.pos(pos);
                    }

                    if(_readStream.pos() != _readStream.size())
                    {
                        if(_endpoint.datagram())
                        {
                            // The message was truncated.
                            throw new DatagramLimitException();
                        }
                        continue;
                    }
                    break;
                }

                int newOp = readOp | writeOp;
                readyOp = readyOp & ~newOp;
                assert (readyOp != 0 || newOp != 0);

                if(_state <= StateNotValidated)
                {
                    if(newOp != 0)
                    {
                        //
                        // Wait for all the transceiver conditions to be
                        // satisfied before continuing.
                        //
                        scheduleTimeout(newOp);
                        _threadPool.update(this, current.operation, newOp);
                        return;
                    }

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
                    if(_startCallback != null)
                    {
                        startCB = _startCallback;
                        _startCallback = null;
                        if(startCB != null)
                        {
                            ++dispatchCount;
                        }
                    }
                }
                else
                {
                    assert(_state <= StateClosingPending);

                    //
                    // We parse messages first, if we receive a close
                    // connection message we won't send more messages.
                    //
                    if((readyOp & SocketOperation.Read) != 0)
                    {
                        // Optimization: use the thread's stream.
                        info = new MessageInfo(current.stream);
                        newOp |= parseMessage(info);
                        dispatchCount += info.messageDispatchCount;
                    }

                    if((readyOp & SocketOperation.Write) != 0)
                    {
                        sentCBs = new java.util.LinkedList<>();
                        newOp |= sendNextMessage(sentCBs);
                        if(!sentCBs.isEmpty())
                        {
                            ++dispatchCount;
                        }
                        else
                        {
                            sentCBs = null;
                        }
                    }

                    if(_state < StateClosed)
                    {
                        scheduleTimeout(newOp);
                        _threadPool.update(this, current.operation, newOp);
                    }
                }

                if(_acmLastActivity > 0)
                {
                    _acmLastActivity = Time.currentMonotonicTimeMillis();
                }

                if(dispatchCount == 0)
                {
                    return; // Nothing to dispatch we're done!
                }

                _dispatchCount += dispatchCount;
                current.ioCompleted();
            }
            catch(DatagramLimitException ex) // Expected.
            {
                if(_warnUdp)
                {
                    _logger.warning("maximum datagram size of " + _readStream.pos() + " exceeded");
                }
                _readStream.resize(Protocol.headerSize);
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
                    _readStream.resize(Protocol.headerSize);
                    _readStream.pos(0);
                    _readHeader = true;
                }
                else
                {
                    setState(StateClosed, ex);
                }
                return;
            }
        }

        if(!_dispatcher) // Optimization, call dispatch() directly if there's no dispatcher.
        {
            dispatch(startCB, sentCBs, info);
        }
        else
        {
            // No need for the stream if heartbeat callback
            if(info != null && info.heartbeatCallback == null)
            {
                //
                // Create a new stream for the dispatch instead of using the
                // thread pool's thread stream.
                //
                assert (info.stream == current.stream);
                InputStream stream = info.stream;
                info.stream = new InputStream(_instance, Protocol.currentProtocolEncoding);
                info.stream.swap(stream);
            }

            final StartCallback finalStartCB = startCB;
            final java.util.List<OutgoingMessage> finalSentCBs = sentCBs;
            final MessageInfo finalInfo = info;
            _threadPool.dispatchFromThisThread(new com.zeroc.IceInternal.DispatchWorkItem(this)
            {
                @Override
                public void run()
                {
                    dispatch(finalStartCB, finalSentCBs, finalInfo);
                }
            });
        }
    }

    protected void dispatch(StartCallback startCB, java.util.List<OutgoingMessage> sentCBs, MessageInfo info)
    {
        int dispatchedCount = 0;

        //
        // Notify the factory that the connection establishment and
        // validation has completed.
        //
        if(startCB != null)
        {
            startCB.connectionStartCompleted(this);
            ++dispatchedCount;
        }

        //
        // Notify AMI calls that the message was sent.
        //
        if(sentCBs != null)
        {
            for(OutgoingMessage msg : sentCBs)
            {
                msg.outAsync.invokeSent();
            }
            ++dispatchedCount;
        }

        if(info != null)
        {
            //
            // Asynchronous replies must be handled outside the thread
            // synchronization, so that nested calls are possible.
            //
            if(info.outAsync != null)
            {
                info.outAsync.invokeCompleted();
                ++dispatchedCount;
            }

            if(info.heartbeatCallback != null)
            {
                try
                {
                    info.heartbeatCallback.heartbeat(this);
                }
                catch(Exception ex)
                {
                    _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                }
                ++dispatchedCount;
            }

            //
            // Method invocation (or multiple invocations for batch messages)
            // must be done outside the thread synchronization, so that nested
            // calls are possible.
            //
            if(info.invokeNum > 0)
            {
                invokeAll(info.stream, info.invokeNum, info.requestId, info.compress, info.servantManager,
                          info.adapter);

                //
                // Don't increase dispatchedCount, the dispatch count is
                // decreased when the incoming reply is sent.
                //
            }
        }

        //
        // Decrease dispatch count.
        //
        if(dispatchedCount > 0)
        {
            boolean shutdown = false;

            synchronized(this)
            {
                _dispatchCount -= dispatchedCount;
                if(_dispatchCount == 0)
                {
                    //
                    // Only initiate shutdown if not already done. It might
                    // have already been done if the sent callback or AMI
                    // callback was dispatched when the connection was already
                    // in the closing state.
                    //
                    if(_state == StateClosing)
                    {
                        if(_instance.queueRequests())
                        {
                            //
                            // We can't call initiateShutdown() from this thread in certain
                            // situations (such as in Android).
                            //
                            shutdown = true;
                        }
                        else
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
                    else if(_state == StateFinished)
                    {
                        reap();
                    }
                    if(!shutdown)
                    {
                        notifyAll();
                    }
                }
            }

            if(shutdown)
            {
                queueShutdown(true);
            }
        }
    }

    @Override
    public void finished(com.zeroc.IceInternal.ThreadPoolCurrent current, final boolean close)
    {
        synchronized(this)
        {
            assert (_state == StateClosed);
            unscheduleTimeout(SocketOperation.Read | SocketOperation.Write);
        }

        //
        // If there are no callbacks to call, we don't call ioCompleted() since
        // we're not going to call code that will potentially block (this avoids
        // promoting a new leader and unecessary thread creation, especially if
        // this is called on shutdown).
        //
        if(_startCallback == null && _sendStreams.isEmpty() && _asyncRequests.isEmpty() &&
                _closeCallback == null && _heartbeatCallback == null)
        {
            finish(close);
            return;
        }

        current.ioCompleted();
        if(!_dispatcher) // Optimization, call finish() directly if there's no
                         // dispatcher.
        {
            finish(close);
        }
        else
        {
            _threadPool.dispatchFromThisThread(new com.zeroc.IceInternal.DispatchWorkItem(this)
            {
                @Override
                public void run()
                {
                    finish(close);
                }
            });
        }
    }

    public void finish(boolean close)
    {
        if(!_initialized)
        {
            if(_instance.traceLevels().network >= 2)
            {
                StringBuffer s = new StringBuffer("failed to ");
                s.append(_connector != null ? "establish" : "accept");
                s.append(" ");
                s.append(_endpoint.protocol());
                s.append(" connection\n");
                s.append(toString());
                s.append("\n");
                s.append(_exception);
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
            }
        }
        else
        {
            if(_instance.traceLevels().network >= 1)
            {
                StringBuffer s = new StringBuffer("closed ");
                s.append(_endpoint.protocol());
                s.append(" connection\n");
                s.append(toString());

                //
                // Trace the cause of unexpected connection closures
                //
                if(!(_exception instanceof CloseConnectionException ||
                     _exception instanceof ConnectionManuallyClosedException ||
                     _exception instanceof ConnectionTimeoutException ||
                     _exception instanceof CommunicatorDestroyedException ||
                     _exception instanceof ObjectAdapterDeactivatedException))
                {
                    s.append("\n");
                    s.append(_exception);
                }
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
            }
        }

        if(close)
        {
            try
            {
                _transceiver.close();
            }
            catch(LocalException ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "unexpected connection exception:\n " + _desc + "\n" + sw.toString();
                _instance.initializationData().logger.error(s);
            }
        }

        if(_startCallback != null)
        {
            if(_instance.queueRequests())
            {
                //
                // The connectionStartFailed method might try to connect with another connector.
                //
                _instance.getQueueExecutor().executeNoThrow(new Callable<Void>()
                {
                    @Override
                    public Void call() throws Exception
                    {
                        _startCallback.connectionStartFailed(ConnectionI.this, _exception);
                        return null;
                    }
                });
            }
            else
            {
                _startCallback.connectionStartFailed(this, _exception);
            }
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

            for(OutgoingMessage p : _sendStreams)
            {
                p.completed(_exception);
                if(p.requestId > 0) // Make sure finished isn't called twice.
                {
                    _asyncRequests.remove(p.requestId);
                }
            }
            _sendStreams.clear();
        }

        for(OutgoingAsyncBase p : _asyncRequests.values())
        {
            if(p.completed(_exception))
            {
                p.invokeCompleted();
            }
        }
        _asyncRequests.clear();

        //
        // Don't wait to be reaped to reclaim memory allocated by read/write streams.
        //
        _writeStream.clear();
        _writeStream.getBuffer().clear();
        _readStream.clear();
        _readStream.getBuffer().clear();

        if(_closeCallback != null)
        {
            try
            {
                _closeCallback.closed(this);
            }
            catch(Exception ex)
            {
                _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
            }
            _closeCallback = null;
        }

        _heartbeatCallback = null;

        //
        // This must be done last as this will cause waitUntilFinished() to
        // return (and communicator objects such as the timer might be destroyed
        // too).
        //
        synchronized(this)
        {
            setState(StateFinished);

            if(_dispatchCount == 0)
            {
                reap();
            }
        }
    }

    @Override
    public String toString()
    {
        return _toString();
    }

    @Override
    public java.nio.channels.SelectableChannel fd()
    {
        return _transceiver.fd();
    }

    @Override
    public void setReadyCallback(com.zeroc.IceInternal.ReadyCallback callback)
    {
        _transceiver.setReadyCallback(callback);
    }

    public synchronized void timedOut()
    {
        if(_state <= StateNotValidated)
        {
            setState(StateClosed, new ConnectTimeoutException());
        }
        else if(_state < StateClosing)
        {
            setState(StateClosed, new TimeoutException());
        }
        else if(_state < StateClosed)
        {
            setState(StateClosed, new CloseTimeoutException());
        }
    }

    @Override
    public String type()
    {
        return _type; // No mutex lock, _type is immutable.
    }

    @Override
    public int timeout()
    {
        return _endpoint.timeout(); // No mutex protection necessary, _endpoint
                                    // is immutable.
    }

    @Override
    public synchronized ConnectionInfo getInfo()
    {
        if(_state >= StateClosed)
        {
            throw (LocalException) _exception.fillInStackTrace();
        }
        return initConnectionInfo();
    }

    @Override
    public synchronized void setBufferSize(int rcvSize, int sndSize)
    {
        if(_state >= StateClosed)
        {
            throw (LocalException) _exception.fillInStackTrace();
        }
        _transceiver.setBufferSize(rcvSize, sndSize);
        _info = null; // Invalidate the cached connection info
    }

    @Override
    public String _toString()
    {
        return _desc; // No mutex lock, _desc is immutable.
    }

    public synchronized void exception(LocalException ex)
    {
        setState(StateClosed, ex);
    }

    public ConnectionI(Communicator communicator, com.zeroc.IceInternal.Instance instance,
                       com.zeroc.IceInternal.ACMMonitor monitor, com.zeroc.IceInternal.Transceiver transceiver,
                       com.zeroc.IceInternal.Connector connector, com.zeroc.IceInternal.EndpointI endpoint,
                       ObjectAdapterI adapter)
    {
        _communicator = communicator;
        _instance = instance;
        _monitor = monitor;
        _transceiver = transceiver;
        _desc = transceiver.toString();
        _type = transceiver.protocol();
        _connector = connector;
        _endpoint = endpoint;
        _adapter = adapter;
        final InitializationData initData = instance.initializationData();
        // Cached for better performance.
        _dispatcher = initData.dispatcher != null;
        _logger = initData.logger; // Cached for better performance.
        _traceLevels = instance.traceLevels(); // Cached for better performance.
        _timer = instance.timer();
        _writeTimeout = new TimeoutCallback();
        _writeTimeoutFuture = null;
        _readTimeout = new TimeoutCallback();
        _readTimeoutFuture = null;
        _warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
        _warnUdp = instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
        _cacheBuffers = instance.cacheMessageBuffers();
        if(_monitor != null && _monitor.getACM().timeout > 0)
        {
            _acmLastActivity = Time.currentMonotonicTimeMillis();
        }
        else
        {
            _acmLastActivity = -1;
        }
        _nextRequestId = 1;
        _messageSizeMax = adapter != null ? adapter.messageSizeMax() : instance.messageSizeMax();
        _batchRequestQueue = new com.zeroc.IceInternal.BatchRequestQueue(instance, _endpoint.datagram());
        _readStream = new InputStream(instance, Protocol.currentProtocolEncoding);
        _readHeader = false;
        _readStreamPos = -1;
        _writeStream = new OutputStream(instance, Protocol.currentProtocolEncoding);
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

        if(adapter != null)
        {
            _servantManager = adapter.getServantManager();
        }
        else
        {
            _servantManager = null;
        }

        try
        {
            if(adapter != null)
            {
                _threadPool = adapter.getThreadPool();
            }
            else
            {
                _threadPool = _instance.clientThreadPool();
            }
            _threadPool.initialize(this);
        }
        catch(LocalException ex)
        {
            throw ex;
        }
        catch(java.lang.Exception ex)
        {
            throw new SyscallException(ex);
        }
    }

    @Override
    protected synchronized void finalize() throws Throwable
    {
        try
        {
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_startCallback == null);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_state == StateFinished);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_dispatchCount == 0);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_sendStreams.isEmpty());
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_asyncRequests.isEmpty());
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
    private static final int StateClosingPending = 5;
    private static final int StateClosed = 6;
    private static final int StateFinished = 7;

    private void setState(int state, LocalException ex)
    {
        //
        // If setState() is called with an exception, then only closed
        // and closing states are permissible.
        //
        assert state >= StateClosing;

        if(_state == state) // Don't switch twice.
        {
            return;
        }

        if(_exception == null)
        {
            //
            // If we are in closed state, an exception must be set.
            //
            assert (_state != StateClosed);

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
                     _exception instanceof ConnectionManuallyClosedException ||
                     _exception instanceof ConnectionTimeoutException ||
                     _exception instanceof CommunicatorDestroyedException ||
                     _exception instanceof ObjectAdapterDeactivatedException ||
                     (_exception instanceof ConnectionLostException && _state >= StateClosing)))
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
                    assert (false);
                    break;
                }

                case StateNotValidated:
                {
                    if(_state != StateNotInitialized)
                    {
                        assert (_state == StateClosed);
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
                    _threadPool.register(this, SocketOperation.Read);
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
                        _threadPool.unregister(this, SocketOperation.Read);
                    }
                    break;
                }

                case StateClosing:
                case StateClosingPending:
                {
                    //
                    // Can't change back from closing pending.
                    //
                    if(_state >= StateClosingPending)
                    {
                        return;
                    }
                    break;
                }

                case StateClosed:
                {
                    if(_state == StateFinished)
                    {
                        return;
                    }

                    _batchRequestQueue.destroy(_exception);

                    //
                    // Don't need to close now for connections so only close the transceiver
                    // if the selector request it.
                    //
                    if(_threadPool.finish(this, false))
                    {
                        _transceiver.close();
                    }
                    break;
                }

                case StateFinished:
                {
                    assert (_state == StateClosed);
                    _communicator = null;
                    break;
                }
            }
        }
        catch(LocalException ex)
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
        if(_monitor != null)
        {
            if(state == StateActive)
            {
                if(_acmLastActivity > 0)
                {
                    _acmLastActivity = Time.currentMonotonicTimeMillis();
                }
                _monitor.add(this);
            }
            else if(_state == StateActive)
            {
                _monitor.remove(this);
            }
        }

        if(_instance.initializationData().observer != null)
        {
            ConnectionState oldState = toConnectionState(_state);
            ConnectionState newState = toConnectionState(state);
            if(oldState != newState)
            {
                _observer = _instance.initializationData().observer.getConnectionObserver(initConnectionInfo(),
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
                     _exception instanceof ConnectionManuallyClosedException ||
                     _exception instanceof ConnectionTimeoutException ||
                     _exception instanceof CommunicatorDestroyedException ||
                     _exception instanceof ObjectAdapterDeactivatedException ||
                     (_exception instanceof ConnectionLostException && _state >= StateClosing)))
                {
                    _observer.failed(_exception.ice_id());
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

    private void initiateShutdown()
    {
        assert(_state == StateClosing && _dispatchCount == 0);

        if(_shutdownInitiated)
        {
            return;
        }
        _shutdownInitiated = true;

        if(!_endpoint.datagram())
        {
            //
            // Before we shut down, we send a close connection message.
            //
            OutputStream os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
            os.writeBlob(Protocol.magic);
            Protocol.currentProtocol.ice_writeMembers(os);
            Protocol.currentProtocolEncoding.ice_writeMembers(os);
            os.writeByte(Protocol.closeConnectionMsg);
            os.writeByte((byte) 0); // compression status: always report 0 for
                                    // CloseConnection in Java.
            os.writeInt(Protocol.headerSize); // Message size.

            if((sendMessage(new OutgoingMessage(os, false, false)) & AsyncStatus.Sent) > 0)
            {
                setState(StateClosingPending);

                //
                // Notify the transceiver of the graceful connection closure.
                //
                int op = _transceiver.closing(true, _exception);
                if(op != 0)
                {
                    scheduleTimeout(op);
                    _threadPool.register(this, op);
                }
            }
        }
    }

    private void queueShutdown(boolean notify)
    {
        //
        // Must be called without synchronization!
        //
        _instance.getQueueExecutor().executeNoThrow(new Callable<Void>()
        {
            @Override
            public Void call()
                throws Exception
            {
                synchronized(ConnectionI.this)
                {
                    try
                    {
                        initiateShutdown();
                    }
                    catch(LocalException ex)
                    {
                        setState(StateClosed, ex);
                    }
                    if(notify)
                    {
                        ConnectionI.this.notifyAll();
                    }
                }
                return null;
            }
        });
    }

    private void sendHeartbeatNow()
    {
        assert(_state == StateActive);

        if(!_endpoint.datagram())
        {
            OutputStream os = new OutputStream(_instance, Protocol.currentProtocolEncoding);
            os.writeBlob(Protocol.magic);
            Protocol.currentProtocol.ice_writeMembers(os);
            Protocol.currentProtocolEncoding.ice_writeMembers(os);
            os.writeByte(Protocol.validateConnectionMsg);
            os.writeByte((byte) 0);
            os.writeInt(Protocol.headerSize); // Message size.

            try
            {
                OutgoingMessage message = new OutgoingMessage(os, false, false);
                sendMessage(message);
            }
            catch(LocalException ex)
            {
                setState(StateClosed, ex);
                assert (_exception != null);
            }
        }
    }

    private boolean initialize(int operation)
    {
        int s = _transceiver.initialize(_readStream.getBuffer(), _writeStream.getBuffer());
        if(s != SocketOperation.None)
        {
            scheduleTimeout(s);
            _threadPool.update(this, operation, s);
            return false;
        }

        //
        // Update the connection description once the transceiver is
        // initialized.
        //
        _desc = _transceiver.toString();
        _initialized = true;
        setState(StateNotValidated);

        return true;
    }

    private boolean validate(int operation)
    {
        if(!_endpoint.datagram()) // Datagram connections are always implicitly
                                  // validated.
        {
            if(_adapter != null) // The server side has the active role for
                                 // connection validation.
            {
                if(_writeStream.isEmpty())
                {
                    _writeStream.writeBlob(Protocol.magic);
                    Protocol.currentProtocol.ice_writeMembers(_writeStream);
                    Protocol.currentProtocolEncoding.ice_writeMembers(_writeStream);
                    _writeStream.writeByte(Protocol.validateConnectionMsg);
                    _writeStream.writeByte((byte) 0); // Compression status
                                                      // (always zero for
                                                      // validate connection).
                    _writeStream.writeInt(Protocol.headerSize); // Message size.
                    TraceUtil.traceSend(_writeStream, _logger, _traceLevels);
                    _writeStream.prepareWrite();
                }

                if(_observer != null)
                {
                    observerStartWrite(_writeStream.getBuffer());
                }

                if(_writeStream.pos() != _writeStream.size())
                {
                    int op = write(_writeStream.getBuffer());
                    if(op != 0)
                    {
                        scheduleTimeout(op);
                        _threadPool.update(this, operation, op);
                        return false;
                    }
                }

                if(_observer != null)
                {
                    observerFinishWrite(_writeStream.getBuffer());
                }
            }
            else
            // The client side has the passive role for connection validation.
            {
                if(_readStream.isEmpty())
                {
                    _readStream.resize(Protocol.headerSize);
                    _readStream.pos(0);
                }

                if(_observer != null)
                {
                    observerStartRead(_readStream.getBuffer());
                }

                if(_readStream.pos() != _readStream.size())
                {
                    int op = read(_readStream.getBuffer());
                    if(op != 0)
                    {
                        scheduleTimeout(op);
                        _threadPool.update(this, operation, op);
                        return false;
                    }
                }

                if(_observer != null)
                {
                    observerFinishRead(_readStream.getBuffer());
                }

                assert (_readStream.pos() == Protocol.headerSize);
                _readStream.pos(0);
                byte[] m = _readStream.readBlob(4);
                if(m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
                   m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
                {
                    BadMagicException ex = new BadMagicException();
                    ex.badMagic = m;
                    throw ex;
                }

                _readProtocol.ice_readMembers(_readStream);
                Protocol.checkSupportedProtocol(_readProtocol);

                _readProtocolEncoding.ice_readMembers(_readStream);
                Protocol.checkSupportedProtocolEncoding(_readProtocolEncoding);

                byte messageType = _readStream.readByte();
                if(messageType != Protocol.validateConnectionMsg)
                {
                    throw new ConnectionNotValidatedException();
                }
                _readStream.readByte(); // Ignore compression status for
                                        // validate connection.
                int size = _readStream.readInt();
                if(size != Protocol.headerSize)
                {
                    throw new IllegalMessageSizeException();
                }
                TraceUtil.traceRecv(_readStream, _logger, _traceLevels);

                _validated = true;
            }
        }

        _writeStream.resize(0);
        _writeStream.pos(0);

        _readStream.resize(Protocol.headerSize);
        _readStream.pos(0);
        _readHeader = true;

        if(_instance.traceLevels().network >= 1)
        {
            StringBuffer s = new StringBuffer();
            if(_endpoint.datagram())
            {
                s.append("starting to ");
                s.append(_connector != null ? "send" : "receive");
                s.append(" ");
                s.append(_endpoint.protocol());
                s.append(" messages\n");
                s.append(_transceiver.toDetailedString());
            }
            else
            {
                s.append(_connector != null ? "established" : "accepted");
                s.append(" ");
                s.append(_endpoint.protocol());
                s.append(" connection\n");
                s.append(toString());
            }
            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
        }

        return true;
    }

    private int sendNextMessage(java.util.List<OutgoingMessage> callbacks)
    {
        if(_sendStreams.isEmpty())
        {
            return SocketOperation.None;
        }
        else if(_state == StateClosingPending && _writeStream.pos() == 0)
        {
            // Message wasn't sent, empty the _writeStream, we're not going to send more data.
            OutgoingMessage message = _sendStreams.getFirst();
            _writeStream.swap(message.stream);
            return SocketOperation.None;
        }

        assert (!_writeStream.isEmpty() && _writeStream.pos() == _writeStream.size());
        try
        {
            while(true)
            {
                //
                // Notify the message that it was sent.
                //
                OutgoingMessage message = _sendStreams.getFirst();
                _writeStream.swap(message.stream);
                if(message.sent())
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
                // If we are in the closed state or if the close is
                // pending, don't continue sending.
                //
                // This can occur if parseMessage (called before
                // sendNextMessage by message()) closes the connection.
                //
                if(_state >= StateClosingPending)
                {
                    return SocketOperation.None;
                }

                //
                // Otherwise, prepare the next message stream for writing.
                //
                message = _sendStreams.getFirst();
                assert (!message.prepared);
                OutputStream stream = message.stream;

                message.stream = doCompress(stream, message.compress);
                message.stream.prepareWrite();
                message.prepared = true;
                TraceUtil.traceSend(stream, _logger, _traceLevels);
                _writeStream.swap(message.stream);

                //
                // Send the message.
                //
                if(_observer != null)
                {
                    observerStartWrite(_writeStream.getBuffer());
                }
                if(_writeStream.pos() != _writeStream.size())
                {
                    int op = write(_writeStream.getBuffer());
                    if(op != 0)
                    {
                        return op;
                    }
                }
                if(_observer != null)
                {
                    observerFinishWrite(_writeStream.getBuffer());
                }
            }

            //
            // If all the messages were sent and we are in the closing state, we schedule
            // the close timeout to wait for the peer to close the connection.
            //
            if(_state == StateClosing && _shutdownInitiated)
            {
                setState(StateClosingPending);
                int op = _transceiver.closing(true, _exception);
                if(op != 0)
                {
                    return op;
                }
            }
        }
        catch(LocalException ex)
        {
            setState(StateClosed, ex);
        }
        return SocketOperation.None;
    }

    private int sendMessage(OutgoingMessage message)
    {
        assert (_state < StateClosed);

        if(!_sendStreams.isEmpty())
        {
            message.adopt();
            _sendStreams.addLast(message);
            return AsyncStatus.Queued;
        }

        //
        // Attempt to send the message without blocking. If the send blocks, we
        // register the connection with the selector thread.
        //

        assert (!message.prepared);

        OutputStream stream = message.stream;

        message.stream = doCompress(stream, message.compress);
        message.stream.prepareWrite();
        message.prepared = true;
        int op;
        TraceUtil.traceSend(stream, _logger, _traceLevels);

        //
        // Send the message without blocking.
        //
        if(_observer != null)
        {
            observerStartWrite(message.stream.getBuffer());
        }
        op = write(message.stream.getBuffer());
        if(op == 0)
        {
            if(_observer != null)
            {
                observerFinishWrite(message.stream.getBuffer());
            }

            int status = AsyncStatus.Sent;
            if(message.sent())
            {
                status |= AsyncStatus.InvokeSentCallback;
            }

            if(_acmLastActivity > 0)
            {
                _acmLastActivity = Time.currentMonotonicTimeMillis();
            }
            return status;
        }

        message.adopt();

        _writeStream.swap(message.stream);
        _sendStreams.addLast(message);
        scheduleTimeout(op);
        _threadPool.register(this, op);
        return AsyncStatus.Queued;
    }

    private OutputStream doCompress(OutputStream uncompressed, boolean compress)
    {
        boolean compressionSupported = false;
        if(compress)
        {
            //
            // Don't check whether compression support is available unless the
            // proxy is configured for compression.
            //
            compressionSupported = com.zeroc.IceInternal.BZip2.supported();
        }

        if(compressionSupported && uncompressed.size() >= 100)
        {
            //
            // Do compression.
            //
            Buffer cbuf = com.zeroc.IceInternal.BZip2.compress(uncompressed.getBuffer(),
                                                               Protocol.headerSize, _compressionLevel);
            if(cbuf != null)
            {
                OutputStream cstream =
                    new OutputStream(uncompressed.instance(), uncompressed.getEncoding(), cbuf, true);

                //
                // Set compression status.
                //
                cstream.pos(9);
                cstream.writeByte((byte) 2);

                //
                // Write the size of the compressed stream into the header.
                //
                cstream.pos(10);
                cstream.writeInt(cstream.size());

                //
                // Write the compression status and size of the compressed
                // stream into the header of the uncompressed stream -- we need
                // this to trace requests correctly.
                //
                uncompressed.pos(9);
                uncompressed.writeByte((byte) 2);
                uncompressed.writeInt(cstream.size());

                return cstream;
            }
        }

        uncompressed.pos(9);
        uncompressed.writeByte((byte) (compressionSupported ? 1 : 0));

        //
        // Not compressed, fill in the message size.
        //
        uncompressed.pos(10);
        uncompressed.writeInt(uncompressed.size());

        return uncompressed;
    }

    private static class MessageInfo
    {
        MessageInfo(InputStream stream)
        {
            this.stream = stream;
        }

        InputStream stream;
        int invokeNum;
        int requestId;
        byte compress;
        com.zeroc.IceInternal.ServantManager servantManager;
        ObjectAdapter adapter;
        OutgoingAsyncBase outAsync;
        HeartbeatCallback heartbeatCallback;
        int messageDispatchCount;
    }

    private int parseMessage(MessageInfo info)
    {
        assert (_state > StateNotValidated && _state < StateClosed);

        _readStream.swap(info.stream);
        _readStream.resize(Protocol.headerSize);
        _readStream.pos(0);
        _readHeader = true;

        assert (info.stream.pos() == info.stream.size());

        //
        // Connection is validated on first message. This is only used by
        // setState() to check wether or not we can print a connection
        // warning (a client might close the connection forcefully if the
        // connection isn't validated).
        //
        _validated = true;

        try
        {
            //
            // We don't need to check magic and version here. This has already
            // been done by the ThreadPool which provides us with the stream.
            //
            info.stream.pos(8);
            byte messageType = info.stream.readByte();
            info.compress = info.stream.readByte();
            if(info.compress == (byte)2)
            {
                if(com.zeroc.IceInternal.BZip2.supported())
                {
                    Buffer ubuf = com.zeroc.IceInternal.BZip2.uncompress(info.stream.getBuffer(),
                                                                         Protocol.headerSize,
                                                                         _messageSizeMax);
                    info.stream = new InputStream(info.stream.instance(), info.stream.getEncoding(), ubuf, true);
                }
                else
                {
                    FeatureNotSupportedException ex = new FeatureNotSupportedException();
                    ex.unsupportedFeature = "Cannot uncompress compressed message: "
                                            + "org.apache.tools.bzip2.CBZip2OutputStream was not found";
                    throw ex;
                }
            }
            info.stream.pos(Protocol.headerSize);

            switch(messageType)
            {
                case Protocol.closeConnectionMsg:
                {
                    TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                    if(_endpoint.datagram())
                    {
                        if(_warn)
                        {
                            _logger.warning("ignoring close connection message for datagram connection:\n" + _desc);
                        }
                    }
                    else
                    {
                        setState(StateClosingPending, new CloseConnectionException());

                        //
                        // Notify the transceiver of the graceful connection closure.
                        //
                        int op = _transceiver.closing(false, _exception);
                        if(op != 0)
                        {
                            return op;
                        }
                        setState(StateClosed);
                    }
                    break;
                }

                case Protocol.requestMsg:
                {
                    if(_state >= StateClosing)
                    {
                        TraceUtil.trace("received request during closing\n(ignored by server, client will retry)",
                                        info.stream, _logger, _traceLevels);
                    }
                    else
                    {
                        TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        info.requestId = info.stream.readInt();
                        info.invokeNum = 1;
                        info.servantManager = _servantManager;
                        info.adapter = _adapter;
                        ++info.messageDispatchCount;
                    }
                    break;
                }

                case Protocol.requestBatchMsg:
                {
                    if(_state >= StateClosing)
                    {
                        TraceUtil.trace("received batch request during closing\n(ignored by server, client will retry)",
                                        info.stream, _logger, _traceLevels);
                    }
                    else
                    {
                        TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        info.invokeNum = info.stream.readInt();
                        if(info.invokeNum < 0)
                        {
                            info.invokeNum = 0;
                            throw new UnmarshalOutOfBoundsException();
                        }
                        info.servantManager = _servantManager;
                        info.adapter = _adapter;
                        info.messageDispatchCount += info.invokeNum;
                    }
                    break;
                }

                case Protocol.replyMsg:
                {
                    TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                    info.requestId = info.stream.readInt();

                    OutgoingAsyncBase outAsync = _asyncRequests.remove(info.requestId);
                    if(outAsync != null && outAsync.completed(info.stream))
                    {
                        info.outAsync = outAsync;
                        ++info.messageDispatchCount;
                    }
                    notifyAll(); // Notify threads blocked in close(false)
                    break;
                }

                case Protocol.validateConnectionMsg:
                {
                    TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                    if(_heartbeatCallback != null)
                    {
                        info.heartbeatCallback = _heartbeatCallback;
                        ++info.messageDispatchCount;
                    }
                    break;
                }

                default:
                {
                    TraceUtil.trace("received unknown message\n(invalid, closing connection)", info.stream,
                                    _logger, _traceLevels);
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

        return _state == StateHolding ? SocketOperation.None : SocketOperation.Read;
    }

    private void invokeAll(InputStream stream, int invokeNum, int requestId, byte compress,
                           com.zeroc.IceInternal.ServantManager servantManager, ObjectAdapter adapter)
    {
        //
        // Note: In contrast to other private or protected methods, this
        // operation must be called *without* the mutex locked.
        //

        Incoming in = null;
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
            invokeException(requestId, ex, invokeNum, false);
        }
        catch(com.zeroc.IceInternal.ServantError ex)
        {
            //
            // ServantError is thrown when an Error has been raised by servant (or servant locator)
            // code. We've already attempted to complete the invocation and send a response.
            //
            Throwable t = ex.getCause();
            //
            // Suppress AssertionError and OutOfMemoryError, rethrow everything else.
            //
            if(!(t instanceof java.lang.AssertionError ||
                 t instanceof java.lang.OutOfMemoryError ||
                 t instanceof java.lang.StackOverflowError))
            {
                throw (java.lang.Error)t;
            }
        }
        catch(java.lang.Error ex)
        {
            //
            // An Error was raised outside of servant code (i.e., by Ice code).
            // Attempt to log the error and clean up. This may still fail
            // depending on the severity of the error.
            //
            // Note that this does NOT send a response to the client.
            //
            UnknownException uex = new UnknownException(ex);
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            uex.unknown = sw.toString();
            _logger.error(uex.unknown);
            invokeException(requestId, uex, invokeNum, false);
            //
            // Suppress AssertionError and OutOfMemoryError, rethrow everything else.
            //
            if(!(ex instanceof java.lang.AssertionError ||
                 ex instanceof java.lang.OutOfMemoryError ||
                 ex instanceof java.lang.StackOverflowError))
            {
                throw ex;
            }
        }
        finally
        {
            if(in != null)
            {
                reclaimIncoming(in);
            }
        }
    }

    private void scheduleTimeout(int status)
    {
        int timeout;
        if(_state < StateActive)
        {
            com.zeroc.IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideConnectTimeout)
            {
                timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
            }
            else
            {
                timeout = _endpoint.timeout();
            }
        }
        else if(_state < StateClosingPending)
        {
            if(_readHeader) // No timeout for reading the header.
            {
                status &= ~SocketOperation.Read;
            }
            timeout = _endpoint.timeout();
        }
        else
        {
            com.zeroc.IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            if(defaultsAndOverrides.overrideCloseTimeout)
            {
                timeout = defaultsAndOverrides.overrideCloseTimeoutValue;
            }
            else
            {
                timeout = _endpoint.timeout();
            }
        }

        if(timeout < 0)
        {
            return;
        }

        try
        {
            if((status & SocketOperation.Read) != 0)
            {
                if(_readTimeoutFuture != null)
                {
                    _readTimeoutFuture.cancel(false);
                }
                _readTimeoutFuture = _timer.schedule(_readTimeout, timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
            }
            if((status & (SocketOperation.Write | SocketOperation.Connect)) != 0)
            {
                if(_writeTimeoutFuture != null)
                {
                    _writeTimeoutFuture.cancel(false);
                }
                _writeTimeoutFuture = _timer.schedule(_writeTimeout, timeout,
                        java.util.concurrent.TimeUnit.MILLISECONDS);
            }
        }
        catch(Throwable ex)
        {
            assert (false);
        }
    }

    private void unscheduleTimeout(int status)
    {
        if((status & SocketOperation.Read) != 0 && _readTimeoutFuture != null)
        {
            _readTimeoutFuture.cancel(false);
            _readTimeoutFuture = null;
        }
        if((status & (SocketOperation.Write | SocketOperation.Connect)) != 0 &&
           _writeTimeoutFuture != null)
        {
            _writeTimeoutFuture.cancel(false);
            _writeTimeoutFuture = null;
        }
    }

    private ConnectionInfo initConnectionInfo()
    {
        if(_state > StateNotInitialized && _info != null) // Update the connection information until it's initialized
        {
            return _info;
        }

        try
        {
            _info = _transceiver.getInfo();
        }
        catch(LocalException ex)
        {
            _info = new ConnectionInfo();
        }
        for(ConnectionInfo info = _info; info != null; info = info.underlying)
        {
            info.connectionId = _endpoint.connectionId();
            info.adapterName = _adapter != null ? _adapter.getName() : "";
            info.incoming = _connector == null;
        }
        return _info;
    }

    private ConnectionState toConnectionState(int state)
    {
        return connectionStateMap[state];
    }

    private void warning(String msg, java.lang.Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        String s = msg + ":\n" + _desc + "\n" + sw.toString();
        _logger.warning(s);
    }

    private void observerStartRead(Buffer buf)
    {
        if(_readStreamPos >= 0)
        {
            assert (!buf.empty());
            _observer.receivedBytes(buf.b.position() - _readStreamPos);
        }
        _readStreamPos = buf.empty() ? -1 : buf.b.position();
    }

    private void observerFinishRead(Buffer buf)
    {
        if(_readStreamPos == -1)
        {
            return;
        }
        assert (buf.b.position() >= _readStreamPos);
        _observer.receivedBytes(buf.b.position() - _readStreamPos);
        _readStreamPos = -1;
    }

    private void observerStartWrite(Buffer buf)
    {
        if(_writeStreamPos >= 0)
        {
            assert (!buf.empty());
            _observer.sentBytes(buf.b.position() - _writeStreamPos);
        }
        _writeStreamPos = buf.empty() ? -1 : buf.b.position();
    }

    private void observerFinishWrite(Buffer buf)
    {
        if(_writeStreamPos == -1)
        {
            return;
        }
        if(buf.b.position() > _writeStreamPos)
        {
            _observer.sentBytes(buf.b.position() - _writeStreamPos);
        }
        _writeStreamPos = -1;
    }

    private Incoming getIncoming(ObjectAdapter adapter, boolean response, byte compress,
                                                       int requestId)
    {
        Incoming in = null;

        if(_cacheBuffers > 0)
        {
            synchronized(_incomingCacheMutex)
            {
                if(_incomingCache == null)
                {
                    in = new Incoming(_instance, this, this, adapter, response, compress, requestId);
                }
                else
                {
                    in = _incomingCache;
                    _incomingCache = _incomingCache.next;
                    in.reset(_instance, this, this, adapter, response, compress, requestId);
                    in.next = null;
                }
            }
        }
        else
        {
            in = new Incoming(_instance, this, this, adapter, response, compress, requestId);
        }

        return in;
    }

    private void reclaimIncoming(Incoming in)
    {
        if(_cacheBuffers > 0 && in.reclaim())
        {
            synchronized(_incomingCacheMutex)
            {
                in.next = _incomingCache;
                _incomingCache = in;
            }
        }
    }

    private void reap()
    {
        if(_monitor != null)
        {
            _monitor.reap(this);
        }
        if(_observer != null)
        {
            _observer.detach();
        }
    }

    private int read(Buffer buf)
    {
        int start = buf.b.position();
        int op = _transceiver.read(buf);
        if(_instance.traceLevels().network >= 3 && buf.b.position() != start)
        {
            StringBuffer s = new StringBuffer("received ");
            if(_endpoint.datagram())
            {
                s.append(buf.b.limit());
            }
            else
            {
                s.append(buf.b.position() - start);
                s.append(" of ");
                s.append(buf.b.limit() - start);
            }
            s.append(" bytes via ");
            s.append(_endpoint.protocol());
            s.append("\n");
            s.append(toString());

            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
        }
        return op;
    }

    private int write(Buffer buf)
    {
        int start = buf.b.position();
        int op = _transceiver.write(buf);
        if(_instance.traceLevels().network >= 3 && buf.b.position() != start)
        {
            StringBuffer s = new StringBuffer("sent ");
            s.append(buf.b.position() - start);
            if(!_endpoint.datagram())
            {
                s.append(" of ");
                s.append(buf.b.limit() - start);
            }
            s.append(" bytes via ");
            s.append(_endpoint.protocol());
            s.append("\n");
            s.append(toString());
            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.toString());
        }
        return op;
    }

    private static class OutgoingMessage
    {
        OutgoingMessage(OutputStream stream, boolean compress, boolean adopt)
        {
            this.stream = stream;
            this.compress = compress;
            this.adopt = adopt;
            this.requestId = 0;
        }

        OutgoingMessage(OutgoingAsyncBase out, OutputStream stream, boolean compress,
                int requestId)
        {
            this.stream = stream;
            this.compress = compress;
            this.outAsync = out;
            this.requestId = requestId;
        }

        public void canceled()
        {
            assert (outAsync != null);
            outAsync = null;
        }

        public void adopt()
        {
            if(adopt)
            {
                OutputStream stream = new OutputStream(this.stream.instance(), Protocol.currentProtocolEncoding);
                stream.swap(this.stream);
                this.stream = stream;
                adopt = false;
            }
        }

        public boolean sent()
        {
            if(outAsync != null)
            {
                return outAsync.sent();
            }
            return false;
        }

        public void completed(LocalException ex)
        {
            if(outAsync != null && outAsync.completed(ex))
            {
                outAsync.invokeCompleted();
            }
        }

        public OutputStream stream;
        public OutgoingAsyncBase outAsync;
        public boolean compress;
        public int requestId;
        boolean adopt;
        boolean prepared;
    }

    private Communicator _communicator;
    private final com.zeroc.IceInternal.Instance _instance;
    private com.zeroc.IceInternal.ACMMonitor _monitor;
    private final com.zeroc.IceInternal.Transceiver _transceiver;
    private String _desc;
    private final String _type;
    private final com.zeroc.IceInternal.Connector _connector;
    private final com.zeroc.IceInternal.EndpointI _endpoint;

    private ObjectAdapter _adapter;
    private com.zeroc.IceInternal.ServantManager _servantManager;

    private final boolean _dispatcher;
    private final Logger _logger;
    private final com.zeroc.IceInternal.TraceLevels _traceLevels;
    private final com.zeroc.IceInternal.ThreadPool _threadPool;

    private final java.util.concurrent.ScheduledExecutorService _timer;
    private final Runnable _writeTimeout;
    private java.util.concurrent.Future<?> _writeTimeoutFuture;
    private final Runnable _readTimeout;
    private java.util.concurrent.Future<?> _readTimeoutFuture;

    private StartCallback _startCallback = null;

    private final boolean _warn;
    private final boolean _warnUdp;

    private long _acmLastActivity;

    private final int _compressionLevel;

    private int _nextRequestId;

    private java.util.Map<Integer, OutgoingAsyncBase> _asyncRequests = new java.util.HashMap<>();

    private LocalException _exception;

    private final int _messageSizeMax;
    private com.zeroc.IceInternal.BatchRequestQueue _batchRequestQueue;

    private java.util.LinkedList<OutgoingMessage> _sendStreams = new java.util.LinkedList<>();

    private InputStream _readStream;
    private boolean _readHeader;
    private OutputStream _writeStream;

    private com.zeroc.Ice.Instrumentation.ConnectionObserver _observer;
    private int _readStreamPos;
    private int _writeStreamPos;

    private int _dispatchCount;

    private int _state; // The current state.
    private boolean _shutdownInitiated = false;
    private boolean _initialized = false;
    private boolean _validated = false;

    private Incoming _incomingCache;
    private final java.lang.Object _incomingCacheMutex = new java.lang.Object();

    private ProtocolVersion _readProtocol = new ProtocolVersion();
    private EncodingVersion _readProtocolEncoding = new EncodingVersion();

    private int _cacheBuffers;

    private ConnectionInfo _info;

    private CloseCallback _closeCallback;
    private HeartbeatCallback _heartbeatCallback;

    private static ConnectionState connectionStateMap[] =
    {
        ConnectionState.ConnectionStateValidating, // StateNotInitialized
        ConnectionState.ConnectionStateValidating, // StateNotValidated
        ConnectionState.ConnectionStateActive, // StateActive
        ConnectionState.ConnectionStateHolding, // StateHolding
        ConnectionState.ConnectionStateClosing, // StateClosing
        ConnectionState.ConnectionStateClosing, // StateClosingPending
        ConnectionState.ConnectionStateClosed, // StateClosed
        ConnectionState.ConnectionStateClosed, // StateFinished
    };
}
