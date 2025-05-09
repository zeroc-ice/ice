// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.ConnectionObserver;
import com.zeroc.Ice.Instrumentation.ConnectionState;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.channels.SelectableChannel;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

/**
 * @hidden Public because it's used by the 'Ice/metrics' test.
 */
public final class ConnectionI extends EventHandler implements Connection, CancellationHandler {
    public interface StartCallback {
        void connectionStartCompleted(ConnectionI connection);

        void connectionStartFailed(ConnectionI connection, LocalException ex);
    }

    public void start(StartCallback callback) {
        try {
            synchronized (this) {
                // The connection might already be closed if the communicator was destroyed.
                if (_state >= StateClosed) {
                    assert (_exception != null);
                    throw (LocalException) _exception.fillInStackTrace();
                }

                if (!initialize(SocketOperation.None) || !validate(SocketOperation.None)) {
                    if (_connectTimeout > 0) {
                        // Schedules a one-time check.
                        _timer.schedule(this::connectTimedOut, _connectTimeout, TimeUnit.SECONDS);
                    }
                    _startCallback = callback;
                    return;
                }

                //
                // We start out in holding state.
                //
                setState(StateHolding);
            }
        } catch (LocalException ex) {
            exception(ex);
            callback.connectionStartFailed(this, _exception);
            return;
        }

        callback.connectionStartCompleted(this);
    }

    public void startAndWait() throws InterruptedException {
        try {
            synchronized (this) {
                // The connection might already be closed if the communicator was destroyed.
                if (_state >= StateClosed) {
                    assert (_exception != null);
                    throw (LocalException) _exception.fillInStackTrace();
                }

                if (!initialize(SocketOperation.None) || !validate(SocketOperation.None)) {
                    while (_state <= StateNotValidated) {
                        wait();
                    }

                    if (_state >= StateClosing) {
                        assert (_exception != null);
                        throw (LocalException) _exception.fillInStackTrace();
                    }
                }

                //
                // We start out in holding state.
                //
                setState(StateHolding);
            }
        } catch (LocalException ex) {
            exception(ex);
            waitUntilFinished();
        }
    }

    public synchronized void activate() {
        if (_state <= StateNotValidated) {
            return;
        }

        setState(StateActive);
    }

    public synchronized void hold() {
        if (_state <= StateNotValidated) {
            return;
        }

        setState(StateHolding);
    }

    // DestructionReason.
    public static final int ObjectAdapterDeactivated = 0;
    public static final int CommunicatorDestroyed = 1;

    public synchronized void destroy(int reason) {
        switch (reason) {
            case ObjectAdapterDeactivated:
            {
                setState(
                    StateClosing,
                    new ObjectAdapterDeactivatedException(
                        _adapter != null ? _adapter.getName() : ""));
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
    public synchronized void abort() {
        setState(
            StateClosed,
            new ConnectionAbortedException("connection aborted by the application", true));
    }

    @Override
    public void close() {
        synchronized (this) {
            if (_state < StateClosing) {
                if (_asyncRequests.isEmpty()) {
                    doApplicationClose();
                } else {
                    _closeRequested = true;
                    // we don't wait forever for outstanding invocations to complete
                    scheduleCloseTimer();
                }
            }
            // else nothing else to do, already closing or closed.

            // Wait until the connection has been closed.
            while (_state < StateClosed) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    throw new OperationInterruptedException(ex);
                }
            }

            if (!(_exception instanceof ConnectionClosedException
                || _exception instanceof CloseConnectionException
                || _exception instanceof CommunicatorDestroyedException
                || _exception instanceof ObjectAdapterDeactivatedException
                || _exception instanceof ObjectAdapterDestroyedException)) {
                assert (_exception != null);
                throw _exception;
            }
        }
    }

    public synchronized boolean isActiveOrHolding() {
        return _state > StateNotValidated && _state < StateClosing;
    }

    public synchronized boolean isFinished() {
        if (_state != StateFinished || _upcallCount != 0) {
            return false;
        }

        assert (_state == StateFinished);
        return true;
    }

    public synchronized void throwException() {
        if (_exception != null) {
            assert (_state >= StateClosing);
            throw (LocalException) _exception.fillInStackTrace();
        }
    }

    public synchronized void waitUntilHolding() throws InterruptedException {
        while (_state < StateHolding || _upcallCount > 0) {
            wait();
        }
    }

    public synchronized void waitUntilFinished() throws InterruptedException {
        //
        // We wait indefinitely until the connection is finished and all outstanding requests are
        // completed. Otherwise we couldn't
        // guarantee that there are no outstanding calls when deactivate()
        // is called on the servant locators.
        //
        while (_state < StateFinished || _upcallCount > 0) {
            wait();
        }

        assert (_state == StateFinished);

        //
        // Clear the OA. See bug 1673 for the details of why this is necessary.
        //
        _adapter = null;
    }

    public synchronized void updateObserver() {
        if (_state < StateNotValidated || _state > StateClosed) {
            return;
        }

        assert (_instance.initializationData().observer != null);
        _observer =
            _instance
                .initializationData()
                .observer
                .getConnectionObserver(
                    initConnectionInfo(),
                    _endpoint,
                    toConnectionState(_state),
                    _observer);
        if (_observer != null) {
            _observer.attach();
        } else {
            _writeStreamPos = -1;
            _readStreamPos = -1;
        }
    }

    public synchronized int sendAsyncRequest(
            OutgoingAsyncBase out, boolean compress, boolean response, int batchRequestNum)
        throws RetryException {
        final OutputStream os = out.getOs();

        if (_exception != null) {
            //
            // If the connection is closed before we even have a chance to send our request, we
            // always try to send the request again.
            //
            throw new RetryException((LocalException) _exception.fillInStackTrace());
        }

        assert (_state > StateNotValidated);
        assert (_state < StateClosing);

        //
        // Ensure the message isn't bigger than what we can send with the transport.
        //
        _transceiver.checkSendSize(os.getBuffer());

        //
        // Notify the request that it's cancelable with this connection. This will throw if the
        // request is canceled.
        //
        out.cancelable(this);

        int requestId = 0;
        if (response) {
            //
            // Create a new unique request ID.
            //
            requestId = _nextRequestId++;
            if (requestId <= 0) {
                _nextRequestId = 1;
                requestId = _nextRequestId++;
            }

            //
            // Fill in the request ID.
            //
            os.pos(Protocol.headerSize);
            os.writeInt(requestId);
        } else if (batchRequestNum > 0) {
            os.pos(Protocol.headerSize);
            os.writeInt(batchRequestNum);
        }

        out.attachRemoteObserver(initConnectionInfo(), _endpoint, requestId);

        // We're just about to send a request, so we are not inactive anymore.
        cancelInactivityTimer();

        int status;
        try {
            status = sendMessage(new OutgoingMessage(out, os, compress, requestId));
        } catch (LocalException ex) {
            setState(StateClosed, ex);
            assert (_exception != null);
            throw (LocalException) _exception.fillInStackTrace();
        }

        if (response) {
            //
            // Add to the async requests map.
            //
            _asyncRequests.put(requestId, out);
        }
        return status;
    }

    public BatchRequestQueue getBatchRequestQueue() {
        return _batchRequestQueue;
    }

    @Override
    public void flushBatchRequests(CompressBatch compressBatch) {
        _iceI_flushBatchRequestsAsync(compressBatch).waitForResponse();
    }

    @Override
    public CompletableFuture<Void> flushBatchRequestsAsync(
            CompressBatch compressBatch) {
        return _iceI_flushBatchRequestsAsync(compressBatch);
    }

    private ConnectionFlushBatch _iceI_flushBatchRequestsAsync(CompressBatch compressBatch) {
        var f = new ConnectionFlushBatch(this, _communicator, _instance);
        f.invoke(compressBatch);
        return f;
    }

    @Override
    public synchronized void setCloseCallback(final CloseCallback callback) {
        if (_state >= StateClosed) {
            if (callback != null) {
                _threadPool.dispatch(
                    new RunnableThreadPoolWorkItem(this) {
                        @Override
                        public void run() {
                            try {
                                callback.closed(ConnectionI.this);
                            } catch (LocalException ex) {
                                _logger.error(
                                    "connection callback exception:\n" + ex + '\n' + _desc);
                            }
                        }
                    });
            }
        } else {
            _closeCallback = callback;
        }
    }

    @Override
    public synchronized void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex) {
        if (_state >= StateClosed) {
            return; // The request has already been or will be shortly notified of the failure.
        }

        Iterator<OutgoingMessage> it = _sendStreams.iterator();
        while (it.hasNext()) {
            OutgoingMessage o = it.next();
            if (o.outAsync == outAsync) {
                if (o.requestId > 0) {
                    _asyncRequests.remove(o.requestId);
                }

                if (ex instanceof ConnectionAbortedException) {
                    setState(StateClosed, ex);
                } else {
                    //
                    // If the request is being sent, don't remove it from the send streams, it will
                    // be removed once the sending is finished.
                    //
                    // Note that since we swapped the message stream to _writeStream
                    // it's fine if the OutgoingAsync output stream is released (and
                    // as long as canceled requests cannot be retried).
                    //
                    o.canceled();
                    if (o != _sendStreams.getFirst()) {
                        it.remove();
                    }
                    if (outAsync.completed(ex)) {
                        outAsync.invokeCompletedAsync();
                    }
                }

                if (_closeRequested && _state < StateClosing && _asyncRequests.isEmpty()) {
                    doApplicationClose();
                }
                return;
            }
        }

        if (outAsync instanceof OutgoingAsync) {
            Iterator<OutgoingAsyncBase> it2 = _asyncRequests.values().iterator();
            while (it2.hasNext()) {
                if (it2.next() == outAsync) {
                    if (ex instanceof ConnectionAbortedException) {
                        setState(StateClosed, ex);
                    } else {
                        it2.remove();
                        if (outAsync.completed(ex)) {
                            outAsync.invokeCompletedAsync();
                        }
                    }

                    if (_closeRequested && _state < StateClosing && _asyncRequests.isEmpty()) {
                        doApplicationClose();
                    }
                    return;
                }
            }
        }
    }

    public EndpointI endpoint() {
        return _endpoint; // No mutex protection necessary, _endpoint is
        // immutable.
    }

    public Connector connector() {
        return _connector; // No mutex protection necessary, _connector is
        // immutable.
    }

    @Override
    public void setAdapter(ObjectAdapter adapter) {
        if (_connector == null) { // server connection
            throw new UnsupportedOperationException(
                "setAdapter can only be called on a client connection");
        }

        if (adapter != null) {
            // Go through the adapter to set the adapter on this connection to ensure the object
            // adapter is still active and to ensure proper locking order.
            adapter.setAdapterOnConnection(this);
        } else {
            synchronized (this) {
                if (_state <= StateNotValidated || _state >= StateClosing) {
                    return;
                }
                _adapter = null;
            }
        }

        //
        // We never change the thread pool with which we were initially registered, even if we add
        // or remove an object adapter.
        //
    }

    @Override
    public synchronized ObjectAdapter getAdapter() {
        return _adapter;
    }

    @Override
    public Endpoint getEndpoint() {
        return _endpoint; // No mutex protection necessary, _endpoint is
        // immutable.
    }

    @Override
    public ObjectPrx createProxy(Identity ident) {
        //
        // Create a reference and return a reverse proxy for this reference.
        //
        var ref = _instance.referenceFactory().create(ident, this);
        return ref == null ? null : new _ObjectPrxI(ref);
    }

    synchronized void setAdapterFromAdapter(ObjectAdapter adapter) {
        if (_state <= StateNotValidated || _state >= StateClosing) {
            return;
        }
        assert (adapter != null); // Called by ObjectAdapter::setAdapterOnConnection
        _adapter = adapter;
    }

    //
    // Operations from EventHandler
    //
    @Override
    public void message(ThreadPoolCurrent current) {
        StartCallback startCB = null;
        List<OutgoingMessage> sentCBs = null;
        MessageInfo info = null;
        int upcallCount = 0;

        synchronized (this) {
            if (_state >= StateClosed) {
                return;
            }

            if (!current.ioReady()) {
                return;
            }

            try {
                int writeOp = SocketOperation.None;
                int readOp = SocketOperation.None;

                // If writes are ready, write the data from the connection's write buffer
                // (_writeStream)
                if ((current.operation & SocketOperation.Write) != 0) {
                    final Buffer buf = _writeStream.getBuffer();
                    if (_observer != null) {
                        observerStartWrite(buf);
                    }
                    writeOp = write(buf);
                    if (_observer != null && (writeOp & SocketOperation.Write) == 0) {
                        observerFinishWrite(buf);
                    }
                }

                // If reads are ready, read the data into the connection's read buffer
                // (_readStream). The data is read until:
                // - the full message is read (the transport read returns SocketOperationNone)
                // and the read buffer is fully filled - the read operation on the transport can't
                // continue without blocking
                if ((current.operation & SocketOperation.Read) != 0) {
                    while (true) {
                        final Buffer buf = _readStream.getBuffer();
                        if (_observer != null && !_readHeader) {
                            observerStartRead(buf);
                        }

                        readOp = read(buf);
                        if ((readOp & SocketOperation.Read) != 0) {
                            // Can't continue without blocking, exit out of the loop.
                            break;
                        }
                        if (_observer != null && !_readHeader) {
                            assert (!buf.b.hasRemaining());
                            observerFinishRead(buf);
                        }

                        // If read header is true, we're reading a new Ice protocol message and we
                        // need to read the message header.
                        if (_readHeader) {
                            // The next read will read the remainder of the message.
                            _readHeader = false;

                            if (_observer != null) {
                                _observer.receivedBytes(Protocol.headerSize);
                            }

                            //
                            // Connection is validated on first message. This is only used by
                            // setState() to check whether or not we can print a connection
                            // warning (a client might close the connection forcefully if the
                            // connection isn't validated, we don't want to print a warning
                            // in this case).
                            //
                            _validated = true;

                            // Full header should be read because the size of _readStream is always
                            // headerSize (14) when reading a new message (see the code that sets
                            // _readHeader = true).
                            int pos = _readStream.pos();
                            if (pos < Protocol.headerSize) {
                                //
                                // This situation is possible for small UDP packets.
                                //
                                throw new MarshalException(
                                    "Received Ice message with too few bytes in header.");
                            }

                            // Decode the header.
                            _readStream.pos(0);
                            byte[] m = new byte[4];
                            m[0] = _readStream.readByte();
                            m[1] = _readStream.readByte();
                            m[2] = _readStream.readByte();
                            m[3] = _readStream.readByte();
                            if (m[0] != Protocol.magic[0]
                                || m[1] != Protocol.magic[1]
                                || m[2] != Protocol.magic[2]
                                || m[3] != Protocol.magic[3]) {
                                throw new ProtocolException(
                                    "Bad magic in message header: "
                                        + Integer.toHexString(m[0])
                                        + " "
                                        + Integer.toHexString(m[1])
                                        + " "
                                        + Integer.toHexString(m[2])
                                        + " "
                                        + Integer.toHexString(m[3]));
                            }

                            _readProtocol.ice_readMembers(_readStream);
                            Protocol.checkSupportedProtocol(_readProtocol);

                            _readProtocolEncoding.ice_readMembers(_readStream);
                            Protocol.checkSupportedProtocolEncoding(_readProtocolEncoding);

                            _readStream.readByte(); // messageType
                            _readStream.readByte(); // compress
                            int size = _readStream.readInt();
                            if (size < Protocol.headerSize) {
                                throw new MarshalException(
                                    "Received Ice message with unexpected size " + size + ".");
                            }

                            // Resize the read buffer to the message size.
                            if (size > _messageSizeMax) {
                                Ex.throwMemoryLimitException(size, _messageSizeMax);
                            }
                            if (size > _readStream.size()) {
                                _readStream.resize(size);
                            }
                            _readStream.pos(pos);
                        }

                        if (_readStream.pos() != _readStream.size()) {
                            if (_endpoint.datagram()) {
                                // The message was truncated.
                                throw new DatagramLimitException();
                            }
                            continue;
                        }
                        break;
                    }
                }

                // readOp and writeOp are set to the operations that the transport read or write
                // calls from above returned. They indicate which operations will need to be
                // monitored by the thread pool's selector when this method returns.
                int newOp = readOp | writeOp;

                // Operations that are ready. For example, if message was called with
                // SocketOperationRead and the transport read returned SocketOperationNone,
                // reads are considered done: there's no additional data to read.
                int readyOp = current.operation & ~newOp;

                if (_state <= StateNotValidated) {
                    // If the connection is still not validated and there's still data to read or
                    // write, continue waiting for data to read or write.
                    if (newOp != 0) {
                        _threadPool.update(this, current.operation, newOp);
                        return;
                    }

                    // Initialize the connection if it's not initialized yet.
                    if (_state == StateNotInitialized && !initialize(current.operation)) {
                        return;
                    }

                    // Validate the connection if it's not validate yet.
                    if (_state <= StateNotValidated && !validate(current.operation)) {
                        return;
                    }

                    // The connection is validated and doesn't need additional data to be read or
                    // written. So unregister it from the thread pool's selector.
                    _threadPool.unregister(this, current.operation);

                    // The connection starts in the holding state. It will be activated by the
                    // connection factory.
                    setState(StateHolding);
                    if (_startCallback != null) {
                        startCB = _startCallback;
                        _startCallback = null;
                        if (startCB != null) {
                            ++upcallCount;
                        }
                    }
                } else { // The connection is active or waits for the CloseConnection message.
                    assert (_state <= StateClosingPending);

                    //
                    // We parse messages first, if we receive a close connection message we won't
                    // send more messages.
                    //
                    if ((readyOp & SocketOperation.Read) != 0) {
                        // Optimization: use the thread's stream.
                        info = new MessageInfo(current.stream);

                        // At this point, the protocol message is fully read and can therefore be
                        // decoded by parseMessage. parseMessage returns the operation to wait for
                        // readiness next.
                        newOp |= parseMessage(info);
                        upcallCount += info.upcallCount;
                    }

                    if ((readyOp & SocketOperation.Write) != 0) {
                        // At this point the message from _writeStream is fully written and the next
                        // message can be written.
                        sentCBs = new LinkedList<>();
                        newOp |= sendNextMessage(sentCBs);
                        if (!sentCBs.isEmpty()) {
                            ++upcallCount;
                        } else {
                            sentCBs = null;
                        }
                    }

                    // If the connection is not closed yet, we update the thread pool selector to
                    // wait for readiness of read, write or both operations.
                    if (_state < StateClosed) {
                        _threadPool.update(this, current.operation, newOp);
                    }
                }

                if (upcallCount == 0) {
                    return; // Nothing to dispatch we're done!
                }

                _upcallCount += upcallCount;

                // There's something to dispatch so we mark IO as completed to elect a new leader
                // thread and let IO be performed on this new leader thread while this thread
                // continues with dispatching the up-calls.
                current.ioCompleted();
            } catch (DatagramLimitException ex) // Expected.
                {
                    if (_warnUdp) {
                        _logger.warning("maximum datagram size of " + _readStream.pos() + " exceeded");
                    }
                    _readStream.resize(Protocol.headerSize);
                    _readStream.pos(0);
                    _readHeader = true;
                    return;
                } catch (SocketException ex) {
                setState(StateClosed, ex);
                return;
            } catch (LocalException ex) {
                if (_endpoint.datagram()) {
                    if (_warn) {
                        String s = "datagram connection exception:\n" + ex + '\n' + _desc;
                        _logger.warning(s);
                    }
                    _readStream.resize(Protocol.headerSize);
                    _readStream.pos(0);
                    _readHeader = true;
                } else {
                    setState(StateClosed, ex);
                }
                return;
            }
        }

        if (!_executor) // Optimization, call upcall() directly if there's no executor.
            {
                upcall(startCB, sentCBs, info);
            } else {
            if (info != null) {
                //
                // Create a new stream for the dispatch instead of using the thread pool's thread
                // stream.
                //
                assert (info.stream == current.stream);
                InputStream stream = info.stream;
                info.stream =
                    new InputStream(
                        _instance,
                        Protocol.currentProtocolEncoding,
                        _instance.cacheMessageBuffers() > 1);
                info.stream.swap(stream);
            }
            final StartCallback finalStartCB = startCB;
            final List<OutgoingMessage> finalSentCBs = sentCBs;
            final MessageInfo finalInfo = info;
            _threadPool.executeFromThisThread(
                new RunnableThreadPoolWorkItem(this) {
                    @Override
                    public void run() {
                        upcall(finalStartCB, finalSentCBs, finalInfo);
                    }
                });
        }
    }

    protected void upcall(
            StartCallback startCB, List<OutgoingMessage> sentCBs, MessageInfo info) {
        int dispatchedCount = 0;

        //
        // Notify the factory that the connection establishment and validation has completed.
        //
        if (startCB != null) {
            startCB.connectionStartCompleted(this);
            ++dispatchedCount;
        }

        //
        // Notify AMI calls that the message was sent.
        //
        if (sentCBs != null) {
            for (OutgoingMessage msg : sentCBs) {
                msg.outAsync.invokeSent();
            }
            ++dispatchedCount;
        }

        if (info != null) {
            //
            // Asynchronous replies must be handled outside the thread synchronization, so that
            // nested calls are possible.
            //
            if (info.outAsync != null) {
                info.outAsync.invokeCompleted();
                ++dispatchedCount;
            }

            //
            // Method invocation (or multiple invocations for batch messages) must be done outside
            // the thread synchronization, so that nested calls are possible.
            //
            if (info.requestCount > 0) {
                dispatchAll(
                    info.stream,
                    info.requestCount,
                    info.requestId,
                    info.compress,
                    info.adapter);

                //
                // Don't increase dispatchedCount, the dispatch count is decreased when the incoming
                // reply is sent.
                //
            }
        }

        //
        // Decrease dispatch count.
        //
        if (dispatchedCount > 0) {
            boolean finished = false;

            synchronized (this) {
                _upcallCount -= dispatchedCount;
                if (_upcallCount == 0) {
                    //
                    // Only initiate shutdown if not already done. It might have already been done
                    // if the sent callback or AMI callback was dispatched when the connection was
                    // already in the closing state.
                    //
                    if (_state == StateClosing) {
                        try {
                            initiateShutdown();
                        } catch (LocalException ex) {
                            setState(StateClosed, ex);
                        }
                    } else if (_state == StateFinished) {
                        finished = true;
                        if (_observer != null) {
                            _observer.detach();
                        }
                    }
                    notifyAll();
                }
            }

            if (finished && _removeFromFactory != null) {
                _removeFromFactory.accept(this);
            }
        }
    }

    @Override
    public void finished(ThreadPoolCurrent current, final boolean close) {
        // Lock the connection here to ensure setState() completes before
        // the code below is executed. This method can be called by the
        // thread pool as soon as setState() calls _threadPool->finish(...).
        // There's no need to lock the mutex for the remainder of the code
        // because the data members accessed by finish() are immutable once
        // _state == StateClosed (and we don't want to hold the mutex when
        // calling upcalls).
        synchronized (this) {
            assert _state == StateClosed;
        }

        //
        // If there are no callbacks to call, we don't call ioCompleted() since
        // we're not going to call code that will potentially block (this avoids promoting a new
        // leader and unnecessary thread creation, especially if this is called on shutdown).
        //
        if (_startCallback == null
            && _sendStreams.isEmpty()
            && _asyncRequests.isEmpty()
            && _closeCallback == null) {
            finish(close);
            return;
        }

        current.ioCompleted();
        if (!_executor) // Optimization, call finish() directly if there's no
            // executor.
            {
                finish(close);
            } else {
            _threadPool.executeFromThisThread(
                new RunnableThreadPoolWorkItem(this) {
                    @Override
                    public void run() {
                        finish(close);
                    }
                });
        }
    }

    public void finish(boolean close) {
        if (!_initialized) {
            if (_instance.traceLevels().network >= 2) {
                StringBuffer s = new StringBuffer("failed to ");
                s.append(_connector != null ? "establish" : "accept");
                s.append(" ");
                s.append(_endpoint.protocol());
                s.append(" connection\n");
                s.append(toString());
                s.append("\n");
                s.append(_exception);
                _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().networkCat, s.toString());
            }
        } else {
            if (_instance.traceLevels().network >= 1) {
                StringBuffer s = new StringBuffer("closed ");
                s.append(_endpoint.protocol());
                s.append(" connection\n");
                s.append(toString());

                //
                // Trace the cause of unexpected connection closures
                //
                if (!(_exception instanceof CloseConnectionException
                    || _exception instanceof ConnectionAbortedException
                    || _exception instanceof ConnectionClosedException
                    || _exception instanceof CommunicatorDestroyedException
                    || _exception instanceof ObjectAdapterDeactivatedException
                    || _exception instanceof ObjectAdapterDestroyedException)) {
                    s.append("\n");
                    s.append(_exception);
                }
                _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().networkCat, s.toString());
            }
        }

        if (close) {
            try {
                _transceiver.close();
            } catch (LocalException ex) {
                StringWriter sw = new StringWriter();
                PrintWriter pw = new PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "unexpected connection exception:\n " + _desc + "\n" + sw.toString();
                _instance.initializationData().logger.error(s);
            }
        }

        if (_startCallback != null) {
            _startCallback.connectionStartFailed(this, _exception);
            _startCallback = null;
        }

        if (!_sendStreams.isEmpty()) {
            if (!_writeStream.isEmpty()) {
                //
                // Return the stream to the outgoing call. This is important for retriable AMI calls
                // which are not marshaled again.
                //
                OutgoingMessage message = _sendStreams.getFirst();
                _writeStream.swap(message.stream);
            }

            for (OutgoingMessage p : _sendStreams) {
                p.completed(_exception);
                if (p.requestId > 0) // Make sure finished isn't called twice.
                    {
                        _asyncRequests.remove(p.requestId);
                    }
            }
            _sendStreams.clear();
        }

        for (OutgoingAsyncBase p : _asyncRequests.values()) {
            if (p.completed(_exception)) {
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

        if (_closeCallback != null) {
            try {
                _closeCallback.closed(this);
            } catch (LocalException ex) {
                _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
            }
            _closeCallback = null;
        }

        //
        // This must be done last as this will cause waitUntilFinished() to
        // return (and communicator objects such as the timer might be destroyed too).
        //
        boolean finished = false;
        synchronized (this) {
            setState(StateFinished);

            if (_upcallCount == 0) {
                finished = true;
                if (_observer != null) {
                    _observer.detach();
                }
            }
        }

        if (finished && _removeFromFactory != null) {
            _removeFromFactory.accept(this);
        }
    }

    @Override
    public String toString() {
        return _toString();
    }

    @Override
    public SelectableChannel fd() {
        return _transceiver.fd();
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        _transceiver.setReadyCallback(callback);
    }

    @Override
    public String type() {
        return _type; // No mutex lock, _type is immutable.
    }

    @Override
    public synchronized ConnectionInfo getInfo() {
        if (_state >= StateClosed) {
            throw (LocalException) _exception.fillInStackTrace();
        }
        return initConnectionInfo();
    }

    @Override
    public synchronized void setBufferSize(int rcvSize, int sndSize) {
        if (_state >= StateClosed) {
            throw (LocalException) _exception.fillInStackTrace();
        }
        _transceiver.setBufferSize(rcvSize, sndSize);
        _info = null; // Invalidate the cached connection info
    }

    @Override
    public String _toString() {
        return _desc; // No mutex lock, _desc is immutable.
    }

    public synchronized void exception(LocalException ex) {
        setState(StateClosed, ex);
    }

    public ThreadPool getThreadPool() {
        return _threadPool;
    }

    public synchronized void idleCheck(int idleTimeout) {
        if (_state == StateActive && _idleTimeoutTransceiver.isIdleCheckEnabled()) {
            if (_instance.traceLevels().network >= 1) {
                _instance
                    .initializationData()
                    .logger
                    .trace(
                        _instance.traceLevels().networkCat,
                        "connection aborted by the idle check because it did not receive any bytes for "
                            + idleTimeout
                            + "s\n"
                            + _transceiver.toDetailedString());
            }

            setState(
                StateClosed,
                new ConnectionAbortedException(
                    "Connection aborted by the idle check because it did not receive any bytes for "
                        + idleTimeout
                        + "s.",
                    false));
        }
        // else nothing to do
    }

    public synchronized void sendHeartbeat() {
        assert !_endpoint.datagram();

        if (_state == StateActive || _state == StateHolding || _state == StateClosing) {

            // We check if the connection has become inactive.
            if (_inactivityTimerFuture == null // timer not already scheduled
                && _inactivityTimeout > 0 // inactivity timeout is enabled
                && _state == StateActive // only schedule the timer if the connection is active
                && _dispatchCount == 0 // no pending dispatch
                && _asyncRequests.isEmpty() // no pending invocation
                && _readHeader // we're not waiting for the remainder of an incoming message
                && _sendStreams.size() <= 1 // there is at most one pending outgoing message
            ) {
                // We may become inactive while the peer is back-pressuring us. In this case, we
                // only schedule the inactivity timer if there is no pending outgoing message or the
                // pending outgoing message is a heartbeat.

                // The stream of the first _sendStreams message is in _writeStream.
                if (_sendStreams.isEmpty()
                    || _writeStream.getBuffer().b.get(8) == Protocol.validateConnectionMsg) {
                    scheduleInactivityTimer();
                }
            }

            // We send a heartbeat to the peer to generate a "write" on the connection. This write
            // in turns creates a read on the peer, and resets the peer's idle check timer. When
            // _sendStream is not empty, there is already an outstanding write, so we don't need to
            // send a heartbeat. It's possible the first message of _sendStreams was already sent
            // but not yet removed from _sendStreams: it means the last write occurred very
            // recently, which is good enough with respect to the idle check.
            // As a result of this optimization, the only possible heartbeat in _sendStreams is the
            // first _sendStreams message.
            if (_sendStreams.isEmpty()) {
                OutputStream os = new OutputStream(Protocol.currentProtocolEncoding);
                os.writeBlob(Protocol.magic);
                Protocol.currentProtocol.ice_writeMembers(os);
                Protocol.currentProtocolEncoding.ice_writeMembers(os);
                os.writeByte(Protocol.validateConnectionMsg);
                os.writeByte((byte) 0);
                os.writeInt(Protocol.headerSize); // Message size.

                try {
                    sendMessage(new OutgoingMessage(os, false));
                } catch (LocalException ex) {
                    setState(StateClosed, ex);
                }
            }
        }
        // else, nothing to do.
    }

    ConnectionI(
            Communicator communicator,
            Instance instance,
            Transceiver transceiver,
            Connector connector,
            EndpointI endpoint,
            ObjectAdapter adapter,
            Consumer<ConnectionI> removeFromFactory, // can be null
            ConnectionOptions options) {
        _communicator = communicator;
        _instance = instance;
        _desc = transceiver.toString();
        _type = transceiver.protocol();
        _connector = connector;
        _endpoint = endpoint;
        _adapter = adapter;
        final InitializationData initData = instance.initializationData();
        // Cached for better performance.
        _executor = initData.executor != null;
        _logger = initData.logger; // Cached for better performance.
        _traceLevels = instance.traceLevels(); // Cached for better performance.
        _connectTimeout = options.connectTimeout();
        _closeTimeout = options.closeTimeout(); // not used for datagram connections
        // suppress inactivity timeout for datagram connections
        _inactivityTimeout = endpoint.datagram() ? 0 : options.inactivityTimeout();
        _maxDispatches = options.maxDispatches();
        _timer = instance.timer();
        _removeFromFactory = removeFromFactory;
        _warn = initData.properties.getIcePropertyAsInt("Ice.Warn.Connections") > 0;
        _warnUdp =
            instance.initializationData().properties.getIcePropertyAsInt("Ice.Warn.Datagrams")
                > 0;
        _nextRequestId = 1;
        _messageSizeMax = connector == null ? adapter.messageSizeMax() : instance.messageSizeMax();
        _batchRequestQueue = new BatchRequestQueue(instance, _endpoint.datagram());
        _readStream =
            new InputStream(
                instance,
                Protocol.currentProtocolEncoding,
                instance.cacheMessageBuffers() > 1);
        _readHeader = false;
        _readStreamPos = -1;
        _writeStream = new OutputStream(); // temporary stream
        _writeStreamPos = -1;
        _upcallCount = 0;
        _state = StateNotInitialized;

        int compressionLevel = initData.properties.getIcePropertyAsInt("Ice.Compression.Level");
        if (compressionLevel < 1) {
            compressionLevel = 1;
        } else if (compressionLevel > 9) {
            compressionLevel = 9;
        }
        _compressionLevel = compressionLevel;

        if (options.idleTimeout() > 0 && !endpoint.datagram()) {
            _idleTimeoutTransceiver =
                new IdleTimeoutTransceiverDecorator(
                    transceiver,
                    this,
                    options.idleTimeout(),
                    options.enableIdleCheck(),
                    _instance.timer());
            transceiver = _idleTimeoutTransceiver;
        } else {
            _idleTimeoutTransceiver = null;
        }

        _transceiver = transceiver;

        try {
            if (connector == null) { // server connection
                assert adapter != null;
                _threadPool = adapter.getThreadPool();
            } else { // client connection
                _threadPool = _instance.clientThreadPool();
            }
            _threadPool.initialize(this);
        } catch (LocalException ex) {
            throw ex;
        } catch (Exception ex) {
            throw new SyscallException(ex);
        }
    }

    @SuppressWarnings({"deprecation", "nofinalizer"})
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            Assert.FinalizerAssert(_startCallback == null);
            Assert.FinalizerAssert(_state == StateFinished);
            Assert.FinalizerAssert(_upcallCount == 0);
            Assert.FinalizerAssert(_sendStreams.isEmpty());
            Assert.FinalizerAssert(_asyncRequests.isEmpty());
        } catch (Exception ex) {} finally {
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

    private void setState(int state, LocalException ex) {
        //
        // If setState() is called with an exception, then only closed
        // and closing states are permissible.
        //
        assert state >= StateClosing;

        if (_state == state) // Don't switch twice.
            {
                return;
            }

        if (_exception == null) {
            //
            // If we are in closed state, an exception must be set.
            //
            assert (_state != StateClosed);

            _exception = ex;

            //
            // We don't warn if we are not validated.
            //
            if (_warn && _validated) {
                //
                // Don't warn about certain expected exceptions.
                //
                if (!(_exception instanceof CloseConnectionException
                    || _exception instanceof ConnectionAbortedException
                    || _exception instanceof ConnectionClosedException
                    || _exception instanceof CommunicatorDestroyedException
                    || _exception instanceof ObjectAdapterDeactivatedException
                    || _exception instanceof ObjectAdapterDestroyedException
                    || (_exception instanceof ConnectionLostException
                    && _state >= StateClosing))) {
                    warning("connection exception", _exception);
                }
            }
        }

        //
        // We must set the new state before we notify requests of any
        // exceptions. Otherwise new requests may retry on a connection that is not yet marked as
        // closed or closing.
        //
        setState(state);
    }

    private void setState(int state) {
        //
        // We don't want to send close connection messages if the endpoint
        // only supports oneway transmission from client to server.
        //
        if (_endpoint.datagram() && state == StateClosing) {
            state = StateClosed;
        }

        //
        // Skip graceful shutdown if we are destroyed before validation.
        //
        if (_state <= StateNotValidated && state == StateClosing) {
            state = StateClosed;
        }

        if (_state == state) {
            // Don't switch twice.
            return;
        }

        if (state > StateActive) {
            cancelInactivityTimer();
        }

        try {
            switch (state) {
                case StateNotInitialized:
                {
                    assert false;
                    break;
                }

                case StateNotValidated:
                {
                    if (_state != StateNotInitialized) {
                        assert (_state == StateClosed);
                        return;
                    }
                    break;
                }

                case StateActive:
                {
                    // Can only switch from holding or not validated to active.
                    if (_state != StateHolding && _state != StateNotValidated) {
                        return;
                    }

                    if (_maxDispatches <= 0 || _dispatchCount < _maxDispatches) {
                        _threadPool.register(this, SocketOperation.Read);
                        if (_idleTimeoutTransceiver != null) {
                            _idleTimeoutTransceiver.enableIdleCheck();
                        }
                    }
                    // else don't resume reading since we're at or over the _maxDispatches
                    // limit.
                    break;
                }

                case StateHolding:
                {
                    //
                    // Can only switch from active or not validated to holding.
                    //
                    if (_state != StateActive && _state != StateNotValidated) {
                        return;
                    }
                    if (_state == StateActive
                        && (_maxDispatches <= 0 || _dispatchCount < _maxDispatches)) {
                        _threadPool.unregister(this, SocketOperation.Read);
                        if (_idleTimeoutTransceiver != null) {
                            _idleTimeoutTransceiver.disableIdleCheck();
                        }
                    }
                    // else reads are already disabled because the _maxDispatches limit is
                    // reached or exceeded.
                    break;
                }

                case StateClosing:
                case StateClosingPending:
                {
                    //
                    // Can't change back from closing pending.
                    //
                    if (_state >= StateClosingPending) {
                        return;
                    }
                    break;
                }

                case StateClosed:
                {
                    if (_state == StateFinished) {
                        return;
                    }

                    _batchRequestQueue.destroy(_exception);

                    //
                    // Don't need to close now for connections so only close the transceiver if
                    // the selector request it.
                    //
                    if (_threadPool.finish(this, false)) {
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
        } catch (LocalException ex) {
            StringWriter sw = new StringWriter();
            PrintWriter pw = new PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();
            String s = "unexpected connection exception:\n " + _desc + "\n" + sw.toString();
            _instance.initializationData().logger.error(s);
        }

        if (_instance.initializationData().observer != null) {
            ConnectionState oldState = toConnectionState(_state);
            ConnectionState newState = toConnectionState(state);
            if (oldState != newState) {
                _observer =
                    _instance
                        .initializationData()
                        .observer
                        .getConnectionObserver(
                            initConnectionInfo(), _endpoint, newState, _observer);
                if (_observer != null) {
                    _observer.attach();
                } else {
                    _writeStreamPos = -1;
                    _readStreamPos = -1;
                }
            }
            if (_observer != null && state == StateClosed && _exception != null) {
                if (!(_exception instanceof CloseConnectionException
                    || _exception instanceof ConnectionAbortedException
                    || _exception instanceof ConnectionClosedException
                    || _exception instanceof CommunicatorDestroyedException
                    || _exception instanceof ObjectAdapterDeactivatedException
                    || _exception instanceof ObjectAdapterDestroyedException
                    || (_exception instanceof ConnectionLostException
                    && _state >= StateClosing))) {
                    _observer.failed(_exception.ice_id());
                }
            }
        }
        _state = state;

        notifyAll();

        if (_state == StateClosing && _upcallCount == 0) {
            try {
                initiateShutdown();
            } catch (LocalException ex) {
                setState(StateClosed, ex);
            }
        }
    }

    private void initiateShutdown() {
        assert (_state == StateClosing && _upcallCount == 0);

        if (_shutdownInitiated) {
            return;
        }
        _shutdownInitiated = true;

        if (!_endpoint.datagram()) {
            //
            // Before we shut down, we send a close connection message.
            //
            OutputStream os = new OutputStream(Protocol.currentProtocolEncoding);
            os.writeBlob(Protocol.magic);
            Protocol.currentProtocol.ice_writeMembers(os);
            Protocol.currentProtocolEncoding.ice_writeMembers(os);
            os.writeByte(Protocol.closeConnectionMsg);
            os.writeByte((byte) 0); // compression status
            os.writeInt(Protocol.headerSize); // Message size.

            scheduleCloseTimer();

            if ((sendMessage(new OutgoingMessage(os, false)) & AsyncStatus.Sent) > 0) {
                setState(StateClosingPending);

                //
                // Notify the transceiver of the graceful connection closure.
                //
                int op = _transceiver.closing(true, _exception);
                if (op != 0) {
                    _threadPool.register(this, op);
                }
            }
        }
    }

    private boolean initialize(int operation) {
        int s = _transceiver.initialize(_readStream.getBuffer(), _writeStream.getBuffer());
        if (s != SocketOperation.None) {
            _threadPool.update(this, operation, s);
            return false;
        }

        //
        // Update the connection description once the transceiver is initialized.
        //
        _desc = _transceiver.toString();
        _initialized = true;
        setState(StateNotValidated);

        return true;
    }

    private boolean validate(int operation) {
        if (!_endpoint.datagram()) // Datagram connections are always implicitly
            // validated.
            {
                if (_connector == null) // The server side has the active role for
                    // connection validation.
                    {
                        if (_writeStream.isEmpty()) {
                            _writeStream.writeBlob(Protocol.magic);
                            Protocol.currentProtocol.ice_writeMembers(_writeStream);
                            Protocol.currentProtocolEncoding.ice_writeMembers(_writeStream);
                            _writeStream.writeByte(Protocol.validateConnectionMsg);
                            _writeStream.writeByte((byte) 0); // Compression status
                            // (always zero for validate connection).
                            _writeStream.writeInt(Protocol.headerSize); // Message size.
                            TraceUtil.traceSend(_writeStream, _instance, this, _logger, _traceLevels);
                            _writeStream.prepareWrite();
                        }

                        if (_observer != null) {
                            observerStartWrite(_writeStream.getBuffer());
                        }

                        if (_writeStream.pos() != _writeStream.size()) {
                            int op = write(_writeStream.getBuffer());
                            if (op != 0) {
                                _threadPool.update(this, operation, op);
                                return false;
                            }
                        }

                        if (_observer != null) {
                            observerFinishWrite(_writeStream.getBuffer());
                        }
                    } else
                // The client side has the passive role for connection validation.
                {
                    if (_readStream.isEmpty()) {
                        _readStream.resize(Protocol.headerSize);
                        _readStream.pos(0);
                    }

                    if (_observer != null) {
                        observerStartRead(_readStream.getBuffer());
                    }

                    if (_readStream.pos() != _readStream.size()) {
                        int op = read(_readStream.getBuffer());
                        if (op != 0) {
                            _threadPool.update(this, operation, op);
                            return false;
                        }
                    }

                    if (_observer != null) {
                        observerFinishRead(_readStream.getBuffer());
                    }

                    _validated = true;

                    assert (_readStream.pos() == Protocol.headerSize);
                    _readStream.pos(0);
                    byte[] m = _readStream.readBlob(4);
                    if (m[0] != Protocol.magic[0]
                        || m[1] != Protocol.magic[1]
                        || m[2] != Protocol.magic[2]
                        || m[3] != Protocol.magic[3]) {
                        throw new ProtocolException(
                            "Bad magic in message header: "
                                + Integer.toHexString(m[0])
                                + " "
                                + Integer.toHexString(m[1])
                                + " "
                                + Integer.toHexString(m[2])
                                + " "
                                + Integer.toHexString(m[3]));
                    }

                    _readProtocol.ice_readMembers(_readStream);
                    Protocol.checkSupportedProtocol(_readProtocol);

                    _readProtocolEncoding.ice_readMembers(_readStream);
                    Protocol.checkSupportedProtocolEncoding(_readProtocolEncoding);

                    byte messageType = _readStream.readByte();
                    if (messageType != Protocol.validateConnectionMsg) {
                        throw new ProtocolException(
                            "Received message of type "
                                + messageType
                                + " over a connection that is not yet validated.");
                    }
                    _readStream.readByte(); // Ignore compression status for validate connection.
                    int size = _readStream.readInt();
                    if (size != Protocol.headerSize) {
                        throw new MarshalException(
                            "Received ValidateConnection message with unexpected size "
                                + size
                                + ".");
                    }
                    TraceUtil.traceRecv(_readStream, this, _logger, _traceLevels);

                    // Client connection starts sending heartbeats once it has received the
                    // ValidateConnection message.
                    if (_idleTimeoutTransceiver != null) {
                        _idleTimeoutTransceiver.scheduleHeartbeat();
                    }
                }
            }

        _writeStream.resize(0);
        _writeStream.pos(0);

        _readStream.resize(Protocol.headerSize);
        _readStream.pos(0);
        _readHeader = true;

        if (_instance.traceLevels().network >= 1) {
            StringBuffer s = new StringBuffer();
            if (_endpoint.datagram()) {
                s.append("starting to ");
                s.append(_connector != null ? "send" : "receive");
                s.append(" ");
                s.append(_endpoint.protocol());
                s.append(" messages\n");
                s.append(_transceiver.toDetailedString());
            } else {
                s.append(_connector != null ? "established" : "accepted");
                s.append(" ");
                s.append(_endpoint.protocol());
                s.append(" connection\n");
                s.append(toString());
            }
            _instance
                .initializationData()
                .logger
                .trace(_instance.traceLevels().networkCat, s.toString());
        }

        return true;
    }

    /**
     * Sends the next queued messages. This method is called by message() once the message which is
     * being sent (_sendStreams.First) is fully sent. Before sending the next message, this message
     * is removed from _sendsStream. If any, its sent callback is also queued in given callback
     * queue.
     *
     * @param callbacks The sent callbacks to call for the messages that were sent.
     * @return The socket operation to register with the thread pool's selector to send the
     *     remainder of the pending message being sent (_sendStreams.First).
     */
    private int sendNextMessage(List<OutgoingMessage> callbacks) {
        if (_sendStreams.isEmpty()) {
            // This can occur if no message was being written and the socket write operation was
            // registered with the thread pool (a transceiver read method can request writing data).
            return SocketOperation.None;
        } else if (_state == StateClosingPending && _writeStream.pos() == 0) {
            // Message wasn't sent, empty the _writeStream, we're not going to send more data
            // because the connection is being closed.
            OutgoingMessage message = _sendStreams.getFirst();
            _writeStream.swap(message.stream);
            return SocketOperation.None;
        }

        // Assert that the message was fully written.
        assert (!_writeStream.isEmpty() && _writeStream.pos() == _writeStream.size());

        try {
            while (true) {
                //
                // The message that was being sent is sent. We can swap back the write stream buffer
                // to the outgoing message (required for retry) and queue its sent callback (if
                // any).
                //
                OutgoingMessage message = _sendStreams.getFirst();
                _writeStream.swap(message.stream);
                if (message.sent()) {
                    callbacks.add(message);
                }
                _sendStreams.removeFirst();

                //
                // If there's nothing left to send, we're done.
                //
                if (_sendStreams.isEmpty()) {
                    break;
                }

                //
                // If we are in the closed state or if the close is pending, don't continue sending.
                //
                // This can occur if parseMessage (called before
                // sendNextMessage by message()) closes the connection.
                //
                if (_state >= StateClosingPending) {
                    return SocketOperation.None;
                }

                //
                // Otherwise, prepare the next message.
                //
                message = _sendStreams.getFirst();
                assert (!message.prepared);
                OutputStream stream = message.stream;

                message.stream = doCompress(stream, message.compress);
                message.stream.prepareWrite();
                message.prepared = true;

                TraceUtil.traceSend(stream, _instance, this, _logger, _traceLevels);

                //
                // Send the message.
                //
                _writeStream.swap(message.stream);
                if (_observer != null) {
                    observerStartWrite(_writeStream.getBuffer());
                }
                if (_writeStream.pos() != _writeStream.size()) {
                    int op = write(_writeStream.getBuffer());
                    if (op != 0) {
                        return op;
                    }
                }
                if (_observer != null) {
                    observerFinishWrite(_writeStream.getBuffer());
                }

                // If the message was sent right away, loop to send the next queued message.
            }

            // Once the CloseConnection message is sent, we transition to the StateClosingPending
            // state.
            if (_state == StateClosing && _shutdownInitiated) {
                setState(StateClosingPending);
                int op = _transceiver.closing(true, _exception);
                if (op != 0) {
                    return op;
                }
            }
        } catch (LocalException ex) {
            setState(StateClosed, ex);
        }
        return SocketOperation.None;
    }

    /**
     * Sends or queues the given message.
     *
     * @param message The message to send.
     * @return The send status.
     */
    private int sendMessage(OutgoingMessage message) {
        assert _state >= StateActive;
        assert _state < StateClosed;

        // Some messages are queued for sending. Just adds the message to the send queue and tell
        // the caller that the message was queued.
        if (!_sendStreams.isEmpty()) {
            _sendStreams.addLast(message);
            return AsyncStatus.Queued;
        }

        assert (!message.prepared);

        // Prepare the message for sending.
        OutputStream stream = message.stream;

        message.stream = doCompress(stream, message.compress);
        message.stream.prepareWrite();
        message.prepared = true;
        int op;
        TraceUtil.traceSend(stream, _instance, this, _logger, _traceLevels);

        // Send the message without blocking.
        if (_observer != null) {
            observerStartWrite(message.stream.getBuffer());
        }
        op = write(message.stream.getBuffer());
        if (op == 0) {
            // The message was sent so we're done.
            if (_observer != null) {
                observerFinishWrite(message.stream.getBuffer());
            }

            int status = AsyncStatus.Sent;
            if (message.sent()) {
                status |= AsyncStatus.InvokeSentCallback;
            }

            return status;
        }

        // The message couldn't be sent right away so we add it to the send stream queue (which is
        // empty) and swap its stream with `_writeStream`. The socket operation returned by the
        // transceiver write is registered with the thread
        // pool. At this point the message() method will take care of sending the whole
        // message (held by _writeStream) when the transceiver is ready to write more of the message
        // buffer.

        _writeStream.swap(message.stream);
        _sendStreams.addLast(message);
        _threadPool.register(this, op);
        return AsyncStatus.Queued;
    }

    private OutputStream doCompress(OutputStream uncompressed, boolean compress) {
        boolean compressionSupported = false;
        if (compress) {
            //
            // Don't check whether compression support is available unless the proxy is configured
            // for compression.
            //
            compressionSupported = BZip2.supported();
        }

        if (compressionSupported && uncompressed.size() >= 100) {
            //
            // Do compression.
            //
            Buffer cbuf =
                BZip2.compress(
                    uncompressed.getBuffer(), Protocol.headerSize, _compressionLevel);
            if (cbuf != null) {
                var cstream = new OutputStream(new Buffer(cbuf, true), uncompressed.getEncoding());

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

    private static class MessageInfo {
        MessageInfo(InputStream stream) {
            this.stream = stream;
        }

        InputStream stream;
        int requestCount;
        int requestId;
        byte compress;
        ObjectAdapter adapter;
        OutgoingAsyncBase outAsync;
        int upcallCount;
    }

    private int parseMessage(MessageInfo info) {
        assert (_state > StateNotValidated && _state < StateClosed);

        _readStream.swap(info.stream);
        _readStream.resize(Protocol.headerSize);
        _readStream.pos(0);
        _readHeader = true;

        assert (info.stream.pos() == info.stream.size());

        try {
            //
            // We don't need to check magic and version here. This has already
            // been done by the ThreadPool which provides us with the stream.
            //
            info.stream.pos(8);
            byte messageType = info.stream.readByte();
            info.compress = info.stream.readByte();
            if (info.compress == (byte) 2) {
                if (BZip2.supported()) {
                    Buffer ubuf =
                        BZip2.uncompress(
                            info.stream.getBuffer(), Protocol.headerSize, _messageSizeMax);
                    info.stream =
                        new InputStream(
                            info.stream.instance(), info.stream.getEncoding(), ubuf, true);
                } else {
                    throw new FeatureNotSupportedException(
                        "Cannot uncompress compressed message: "
                            + "org.apache.tools.bzip2.CBZip2OutputStream was not found");
                }
            }
            info.stream.pos(Protocol.headerSize);

            switch (messageType) {
                case Protocol.closeConnectionMsg:
                {
                    TraceUtil.traceRecv(info.stream, this, _logger, _traceLevels);
                    if (_endpoint.datagram()) {
                        if (_warn) {
                            _logger.warning(
                                "ignoring close connection message for datagram connection:\n"
                                    + _desc);
                        }
                    } else {
                        setState(StateClosingPending, new CloseConnectionException());

                        //
                        // Notify the transceiver of the graceful connection closure.
                        //
                        int op = _transceiver.closing(false, _exception);
                        if (op != 0) {
                            scheduleCloseTimer();
                            return op;
                        }
                        setState(StateClosed);
                    }
                    break;
                }

                case Protocol.requestMsg:
                {
                    if (_state >= StateClosing) {
                        TraceUtil.trace(
                            "received request during closing\n(ignored by server, client will retry)",
                            info.stream,
                            this,
                            _logger,
                            _traceLevels);
                    } else {
                        TraceUtil.traceRecv(info.stream, this, _logger, _traceLevels);
                        info.requestId = info.stream.readInt();
                        info.requestCount = 1;
                        info.adapter = _adapter;
                        ++info.upcallCount;

                        cancelInactivityTimer();
                        ++_dispatchCount;
                    }
                    break;
                }

                case Protocol.requestBatchMsg:
                {
                    if (_state >= StateClosing) {
                        TraceUtil.trace(
                            "received batch request during closing\n(ignored by server, client will retry)",
                            info.stream,
                            this,
                            _logger,
                            _traceLevels);
                    } else {
                        TraceUtil.traceRecv(info.stream, this, _logger, _traceLevels);
                        info.requestCount = info.stream.readInt();
                        if (info.requestCount < 0) {
                            info.requestCount = 0;
                            throw new MarshalException(
                                "Received batch request with "
                                    + info.requestCount
                                    + "batches.");
                        }
                        info.adapter = _adapter;
                        info.upcallCount += info.requestCount;

                        cancelInactivityTimer();
                        _dispatchCount += info.requestCount;
                    }
                    break;
                }

                case Protocol.replyMsg:
                {
                    TraceUtil.traceRecv(info.stream, this, _logger, _traceLevels);
                    info.requestId = info.stream.readInt();

                    var outAsync = _asyncRequests.remove(info.requestId);
                    if (outAsync != null && outAsync.completed(info.stream)) {
                        info.outAsync = outAsync;
                        ++info.upcallCount;
                    }
                    if (_closeRequested && _state < StateClosing && _asyncRequests.isEmpty()) {
                        doApplicationClose();
                    }
                    break;
                }

                case Protocol.validateConnectionMsg:
                {
                    TraceUtil.traceRecv(info.stream, this, _logger, _traceLevels);
                    break;
                }

                default:
                {
                    TraceUtil.trace(
                        "received unknown message\n(invalid, closing connection)",
                        info.stream,
                        this,
                        _logger,
                        _traceLevels);
                    throw new ProtocolException(
                        "Received Ice protocol message with unknown type: " + messageType);
                }
            }
        } catch (LocalException ex) {
            if (_endpoint.datagram()) {
                if (_warn) {
                    _logger.warning("datagram connection exception:\n" + ex + '\n' + _desc);
                }
            } else {
                setState(StateClosed, ex);
            }
        }

        if (_state == StateHolding) {
            // Don't continue reading if the connection is in the holding state.
            return SocketOperation.None;
        } else if (_maxDispatches > 0 && _dispatchCount >= _maxDispatches) {
            // Don't continue reading if the _maxDispatches limit is reached or exceeded.
            if (_idleTimeoutTransceiver != null) {
                _idleTimeoutTransceiver.disableIdleCheck();
            }
            return SocketOperation.None;
        } else {
            // Continue reading.
            return SocketOperation.Read;
        }
    }

    private void dispatchAll(
            InputStream stream,
            int requestCount,
            int requestId,
            byte compress,
            ObjectAdapter adapter) {

        // Note: In contrast to other private or protected methods, this method must be called
        // *without* the mutex locked.

        Object dispatcher = adapter != null ? adapter.dispatchPipeline() : null;

        try {
            while (requestCount > 0) {
                // adapter can be null here, however we never pass a null current.adapter to the
                // application code.
                var request = new IncomingRequest(requestId, this, adapter, stream);
                final boolean isTwoWay = !_endpoint.datagram() && requestId != 0;

                if (dispatcher != null) {
                    CompletionStage<OutgoingResponse> response = null;
                    try {
                        response = dispatcher.dispatch(request);
                    } catch (Throwable ex) { // UserException or an unchecked exception
                        sendResponse(
                            request.current.createOutgoingResponse(ex), isTwoWay, (byte) 0);
                    }
                    if (response != null) {
                        response.whenComplete(
                            (result, exception) -> {
                                if (exception != null) {
                                    sendResponse(
                                        request.current.createOutgoingResponse(exception),
                                        isTwoWay,
                                        (byte) 0);
                                } else {
                                    sendResponse(result, isTwoWay, compress);
                                }
                                // Any exception thrown by this closure is effectively ignored.
                            });
                    }
                } else {
                    // Received request on a connection without an object adapter.
                    sendResponse(
                        request.current.createOutgoingResponse(
                            new ObjectNotExistException()),
                        isTwoWay,
                        (byte) 0);
                }
                --requestCount;
            }
            stream.clear();

        } catch (LocalException ex) {
            dispatchException(ex, requestCount);
        } catch (RuntimeException | Error ex) {
            // A runtime exception or an error was thrown outside of servant code (i.e., by Ice
            // code). Note that this code does NOT send a response to the client.
            var uex = new UnknownException(ex);
            var sw = new StringWriter();
            var pw = new PrintWriter(sw);
            ex.printStackTrace(pw);
            pw.flush();

            _logger.error(sw.toString());
            dispatchException(uex, requestCount);
        }
    }

    private void sendResponse(OutgoingResponse response, boolean isTwoWay, byte compress) {
        final OutputStream outputStream = response.outputStream;
        boolean finished = false;
        try {
            synchronized (this) {
                assert (_state > StateNotValidated);
                try {
                    if (--_upcallCount == 0) {
                        if (_state == StateFinished) {
                            finished = true;
                            if (_observer != null) {
                                _observer.detach();
                            }
                        }
                        notifyAll();
                    }

                    if (_state < StateClosed) {
                        if (isTwoWay) {
                            sendMessage(new OutgoingMessage(outputStream, compress != 0));
                        }

                        if (_state == StateActive
                            && _maxDispatches > 0
                            && _dispatchCount == _maxDispatches) {
                            // Resume reading if the connection is active and the dispatch count is
                            // about to be less than _maxDispatches.
                            _threadPool.update(this, SocketOperation.None, SocketOperation.Read);
                            if (_idleTimeoutTransceiver != null) {
                                _idleTimeoutTransceiver.enableIdleCheck();
                            }
                        }

                        --_dispatchCount;
                    }

                    if (_state == StateClosing && _upcallCount == 0) {
                        initiateShutdown();
                    }
                } catch (LocalException ex) {
                    setState(StateClosed, ex);
                }
            }
        } finally {
            if (finished && _removeFromFactory != null) {
                _removeFromFactory.accept(this);
            }
        }
    }

    private void dispatchException(LocalException ex, int requestCount) {
        boolean finished = false;
        synchronized (this) {
            // Fatal exception while dispatching a request. Since sendResponse isn't called in case
            // of a fatal exception we decrement _upcallCount here.

            setState(StateClosed, ex);

            if (requestCount > 0) {
                assert (_upcallCount > 0);
                _upcallCount -= requestCount;
                assert (_upcallCount >= 0);
                if (_upcallCount == 0) {
                    if (_state == StateFinished) {
                        finished = true;
                        if (_observer != null) {
                            _observer.detach();
                        }
                    }
                    notifyAll();
                }
            }
        }

        if (finished && _removeFromFactory != null) {
            _removeFromFactory.accept(this);
        }
    }

    private ConnectionInfo initConnectionInfo() {
        // Called in synchronization

        if (_state > StateNotInitialized
            && _info != null) // Update the connection information until it's initialized
            {
                return _info;
            }

        boolean incoming = _connector == null;
        _info =
            _transceiver.getInfo(
                incoming,
                _adapter != null ? _adapter.getName() : "",
                _endpoint.connectionId());
        return _info;
    }

    private ConnectionState toConnectionState(int state) {
        return connectionStateMap[state];
    }

    private void warning(String msg, Exception ex) {
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        String s = msg + ":\n" + _desc + "\n" + sw.toString();
        _logger.warning(s);
    }

    private void observerStartRead(Buffer buf) {
        if (_readStreamPos >= 0) {
            assert (!buf.empty());
            _observer.receivedBytes(buf.b.position() - _readStreamPos);
        }
        _readStreamPos = buf.empty() ? -1 : buf.b.position();
    }

    private void observerFinishRead(Buffer buf) {
        if (_readStreamPos == -1) {
            return;
        }
        assert (buf.b.position() >= _readStreamPos);
        _observer.receivedBytes(buf.b.position() - _readStreamPos);
        _readStreamPos = -1;
    }

    private void observerStartWrite(Buffer buf) {
        if (_writeStreamPos >= 0) {
            assert (!buf.empty());
            _observer.sentBytes(buf.b.position() - _writeStreamPos);
        }
        _writeStreamPos = buf.empty() ? -1 : buf.b.position();
    }

    private void observerFinishWrite(Buffer buf) {
        if (_writeStreamPos == -1) {
            return;
        }
        if (buf.b.position() > _writeStreamPos) {
            _observer.sentBytes(buf.b.position() - _writeStreamPos);
        }
        _writeStreamPos = -1;
    }

    private int read(Buffer buf) {
        int start = buf.b.position();
        int op = _transceiver.read(buf);
        if (_instance.traceLevels().network >= 3 && buf.b.position() != start) {
            StringBuffer s = new StringBuffer("received ");
            if (_endpoint.datagram()) {
                s.append(buf.b.limit());
            } else {
                s.append(buf.b.position() - start);
                s.append(" of ");
                s.append(buf.b.limit() - start);
            }
            s.append(" bytes via ");
            s.append(_endpoint.protocol());
            s.append("\n");
            s.append(toString());

            _instance
                .initializationData()
                .logger
                .trace(_instance.traceLevels().networkCat, s.toString());
        }
        return op;
    }

    private synchronized void inactivityCheck() {
        if (_inactivityTimerFuture.getDelay(TimeUnit.NANOSECONDS) <= 0) {
            _inactivityTimerFuture = null;

            if (_state == StateActive) {
                setState(
                    StateClosing,
                    new ConnectionClosedException(
                        "Connection closed because it remained inactive for longer than the inactivity timeout.",
                        false));
            }
        }
        // Else this timer was already canceled and disposed. Nothing to do.
    }

    private synchronized void connectTimedOut() {
        if (_state < StateActive) {
            setState(StateClosed, new ConnectTimeoutException());
        }
        // else ignore since we're already connected
    }

    private synchronized void closeTimedOut() {
        if (_state < StateClosed) {
            // We don't use setState(state, exception) because we want to overwrite the exception
            // set by a graceful closure.
            _exception = new CloseTimeoutException();
            setState(StateClosed);
        }
        // else ignore since we're already closed.
    }

    private int write(Buffer buf) {
        int start = buf.b.position();
        int op = _transceiver.write(buf);
        if (_instance.traceLevels().network >= 3 && buf.b.position() != start) {
            StringBuffer s = new StringBuffer("sent ");
            s.append(buf.b.position() - start);
            if (!_endpoint.datagram()) {
                s.append(" of ");
                s.append(buf.b.limit() - start);
            }
            s.append(" bytes via ");
            s.append(_endpoint.protocol());
            s.append("\n");
            s.append(toString());
            _instance
                .initializationData()
                .logger
                .trace(_instance.traceLevels().networkCat, s.toString());
        }
        return op;
    }

    private void scheduleInactivityTimer() {
        // Called within the synchronization lock
        assert _inactivityTimerFuture == null;
        assert _inactivityTimeout > 0;

        _inactivityTimerFuture =
            _timer.schedule(this::inactivityCheck, _inactivityTimeout, TimeUnit.SECONDS);
    }

    private void cancelInactivityTimer() {
        // Called within the synchronization lock
        if (_inactivityTimerFuture != null) {
            _inactivityTimerFuture.cancel(false);
            _inactivityTimerFuture = null;
        }
    }

    private void scheduleCloseTimer() {
        if (_closeTimeout > 0) {
            // Schedules a one-time check.
            _timer.schedule(this::closeTimedOut, _closeTimeout, TimeUnit.SECONDS);
        }
    }

    // Only public so that tests can initiate a closure without blocking on it to complete.
    public synchronized void doApplicationClose() {
        assert (_state < StateClosing);
        setState(
            StateClosing,
            new ConnectionClosedException(
                "connection closed gracefully by the application", true));
    }

    private static class OutgoingMessage {
        OutgoingMessage(OutputStream stream, boolean compress) {
            this.stream = stream;
            this.compress = compress;
            this.requestId = 0;
        }

        OutgoingMessage(
                OutgoingAsyncBase out, OutputStream stream, boolean compress, int requestId) {
            this.stream = stream;
            this.compress = compress;
            this.outAsync = out;
            this.requestId = requestId;
        }

        public void canceled() {
            assert (outAsync != null);
            outAsync = null;
        }

        public boolean sent() {
            if (outAsync != null) {
                return outAsync.sent();
            }
            return false;
        }

        public void completed(LocalException ex) {
            if (outAsync != null && outAsync.completed(ex)) {
                outAsync.invokeCompleted();
            }
        }

        public OutputStream stream;
        public OutgoingAsyncBase outAsync;
        public boolean compress;
        public int requestId;
        boolean prepared;
    }

    private Communicator _communicator;
    private final Instance _instance;
    private final Transceiver _transceiver;
    private final IdleTimeoutTransceiverDecorator _idleTimeoutTransceiver; // can be null
    private String _desc;
    private final String _type;
    private final Connector _connector;
    private final EndpointI _endpoint;

    private ObjectAdapter _adapter;

    private final boolean _executor;
    private final Logger _logger;
    private final TraceLevels _traceLevels;
    private final ThreadPool _threadPool;

    // All these timeouts are in seconds. A value <= 0 means infinite timeout.
    private final int _connectTimeout;
    private final int _closeTimeout;
    private final int _inactivityTimeout;

    private ScheduledFuture<?> _inactivityTimerFuture; // can be null

    private final ScheduledExecutorService _timer;

    private StartCallback _startCallback;

    private final Consumer<ConnectionI> _removeFromFactory;

    private final boolean _warn;
    private final boolean _warnUdp;

    private final int _compressionLevel;

    private int _nextRequestId;

    private final Map<Integer, OutgoingAsyncBase> _asyncRequests = new HashMap<>();

    private LocalException _exception;

    private final int _messageSizeMax;
    private final BatchRequestQueue _batchRequestQueue;

    private final LinkedList<OutgoingMessage> _sendStreams = new LinkedList<>();

    // Contains the message which is being received. If the connection is waiting to receive a
    // message (_readHeader == true), its size is Protocol.headerSize. Otherwise,
    // its size is the message size specified in the received message header.
    private final InputStream _readStream;

    // When _readHeader is true, the next bytes we'll read are the header of a new
    // message. When false, we're reading next the remainder of a message that was already partially
    // received.
    private boolean _readHeader;

    // Contains the message which is being sent. The write stream buffer is empty if no message is
    // being sent.
    private final OutputStream _writeStream;

    private ConnectionObserver _observer;
    private int _readStreamPos;
    private int _writeStreamPos;

    // The upcall count keeps track of the number of dispatches, AMI (response) continuations, sent
    // callbacks and connection establishment callbacks that have been started (or are about to be
    // started) by a thread of the thread pool associated with this connection, and have not
    // completed yet. All these operations except the connection establishment callbacks execute
    // application code or code generated from Slice definitions.
    private int _upcallCount;

    // The number of outstanding dispatches. Maintained only while state is
    // StateActive or StateHolding.
    private int _dispatchCount;

    // When we dispatch _maxDispatches concurrent requests, we stop reading the connection to
    // back-pressure the peer. _maxDispatches <= 0 means no limit.
    private final int _maxDispatches;

    private int _state; // The current state.
    private boolean _shutdownInitiated;
    private boolean _initialized;
    private boolean _validated;

    // When true, the application called close and Connection must close the
    // connection when it receives the reply for the last outstanding invocation.
    private boolean _closeRequested;

    private final ProtocolVersion _readProtocol = new ProtocolVersion();
    private final EncodingVersion _readProtocolEncoding = new EncodingVersion();

    private ConnectionInfo _info;

    private CloseCallback _closeCallback;

    private static final ConnectionState[] connectionStateMap = {
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
