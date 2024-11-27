// Copyright (c) ZeroC, Inc.

using Ice.Instrumentation;
using Ice.Internal;
using System.Diagnostics;
using System.Text;

namespace Ice;

#pragma warning disable CA1001 // _inactivityTimer is disposed by cancelInactivityTimer.
public sealed class ConnectionI : Internal.EventHandler, CancellationHandler, Connection
#pragma warning restore CA1001
{
    internal interface StartCallback
    {
        void connectionStartCompleted(ConnectionI connection);

        void connectionStartFailed(ConnectionI connection, LocalException ex);
    }

    internal void start(StartCallback callback)
    {
        try
        {
            lock (_mutex)
            {
                //
                // The connection might already be closed if the communicator was destroyed.
                //
                if (_state >= StateClosed)
                {
                    Debug.Assert(_exception is not null);
                    throw _exception;
                }

                if (!initialize(SocketOperation.None) || !validate(SocketOperation.None))
                {
                    if (_connectTimeout > TimeSpan.Zero)
                    {
#pragma warning disable CA2000 // connectTimer is disposed by connectTimedOut.
                        var connectTimer = new System.Threading.Timer(
                            timerObj => connectTimedOut((System.Threading.Timer)timerObj));
                        // schedule timer to run once; connectTimedOut disposes the timer too.
                        connectTimer.Change(_connectTimeout, Timeout.InfiniteTimeSpan);
#pragma warning restore CA2000
                    }

                    _startCallback = callback;
                    return;
                }

                // The connection starts in the holding state. It will be activated by the connection factory.
                setState(StateHolding);
            }
        }
        catch (LocalException ex)
        {
            exception(ex);
            callback.connectionStartFailed(this, _exception);
            return;
        }

        callback.connectionStartCompleted(this);
    }

    internal void startAndWait()
    {
        try
        {
            lock (_mutex)
            {
                //
                // The connection might already be closed if the communicator was destroyed.
                //
                if (_state >= StateClosed)
                {
                    Debug.Assert(_exception is not null);
                    throw _exception;
                }

                if (!initialize(SocketOperation.None) || !validate(SocketOperation.None))
                {
                    //
                    // Wait for the connection to be validated.
                    //
                    while (_state <= StateNotValidated)
                    {
                        Monitor.Wait(_mutex);
                    }

                    if (_state >= StateClosing)
                    {
                        Debug.Assert(_exception is not null);
                        throw _exception;
                    }
                }

                //
                // We start out in holding state.
                //
                setState(StateHolding);
            }
        }
        catch (LocalException ex)
        {
            exception(ex);
            waitUntilFinished();
            return;
        }
    }

    internal void activate()
    {
        lock (_mutex)
        {
            if (_state <= StateNotValidated)
            {
                return;
            }

            setState(StateActive);
        }
    }

    internal void hold()
    {
        lock (_mutex)
        {
            if (_state <= StateNotValidated)
            {
                return;
            }

            setState(StateHolding);
        }
    }

    // DestructionReason.
    public const int ObjectAdapterDeactivated = 0;
    public const int CommunicatorDestroyed = 1;

    internal void destroy(int reason)
    {
        lock (_mutex)
        {
            switch (reason)
            {
                case ObjectAdapterDeactivated:
                {
                    setState(StateClosing, new ObjectAdapterDeactivatedException(_adapter?.getName() ?? ""));
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

    public void abort()
    {
        lock (_mutex)
        {
            setState(
                StateClosed,
                new ConnectionAbortedException(
                    "The connection was aborted by the application.",
                    closedByApplication: true));
        }
    }

    public Task closeAsync()
    {
        lock (_mutex)
        {
            if (_state < StateClosing)
            {
                if (_asyncRequests.Count == 0)
                {
                    doApplicationClose();
                }
                else
                {
                    _closeRequested = true;
                    scheduleCloseTimer(); // we don't wait forever for outstanding invocations to complete
                }
            }
            // else nothing to do, already closing or closed.
        }

        return _closed.Task;
    }

    internal bool isActiveOrHolding()
    {
        lock (_mutex)
        {
            return _state > StateNotValidated && _state < StateClosing;
        }
    }

    public void throwException()
    {
        lock (_mutex)
        {
            if (_exception is not null)
            {
                Debug.Assert(_state >= StateClosing);
                throw _exception;
            }
        }
    }

    internal void waitUntilHolding()
    {
        lock (_mutex)
        {
            while (_state < StateHolding || _upcallCount > 0)
            {
                Monitor.Wait(_mutex);
            }
        }
    }

    internal void waitUntilFinished()
    {
        lock (_mutex)
        {
            //
            // We wait indefinitely until the connection is finished and all
            // outstanding requests are completed. Otherwise we couldn't
            // guarantee that there are no outstanding calls when deactivate()
            // is called on the servant locators.
            //
            while (_state < StateFinished || _upcallCount > 0)
            {
                Monitor.Wait(_mutex);
            }

            Debug.Assert(_state == StateFinished);

            //
            // Clear the OA. See bug 1673 for the details of why this is necessary.
            //
            _adapter = null;
        }
    }

    internal void updateObserver()
    {
        lock (_mutex)
        {
            if (_state < StateNotValidated || _state > StateClosed)
            {
                return;
            }

            Debug.Assert(_instance.initializationData().observer is not null);
            _observer = _instance.initializationData().observer.getConnectionObserver(
                initConnectionInfo(),
                _endpoint,
                toConnectionState(_state),
                _observer);
            if (_observer is not null)
            {
                _observer.attach();
            }
            else
            {
                _writeStreamPos = -1;
                _readStreamPos = -1;
            }
        }
    }

    internal int sendAsyncRequest(
        OutgoingAsyncBase og,
        bool compress,
        bool response,
        int batchRequestCount)
    {
        OutputStream os = og.getOs();

        lock (_mutex)
        {
            //
            // If the exception is closed before we even have a chance
            // to send our request, we always try to send the request
            // again.
            //
            if (_exception is not null)
            {
                throw new RetryException(_exception);
            }

            Debug.Assert(_state > StateNotValidated);
            Debug.Assert(_state < StateClosing);

            //
            // Ensure the message isn't bigger than what we can send with the
            // transport.
            //
            _transceiver.checkSendSize(os.getBuffer());

            //
            // Notify the request that it's cancelable with this connection.
            // This will throw if the request is canceled.
            //
            og.cancelable(this);
            int requestId = 0;
            if (response)
            {
                //
                // Create a new unique request ID.
                //
                requestId = _nextRequestId++;
                if (requestId <= 0)
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
            else if (batchRequestCount > 0)
            {
                os.pos(Protocol.headerSize);
                os.writeInt(batchRequestCount);
            }

            og.attachRemoteObserver(initConnectionInfo(), _endpoint, requestId);

            // We're just about to send a request, so we are not inactive anymore.
            cancelInactivityTimer();

            int status = OutgoingAsyncBase.AsyncStatusQueued;
            try
            {
                OutgoingMessage message = new OutgoingMessage(og, os, compress, requestId);
                status = sendMessage(message);
            }
            catch (LocalException ex)
            {
                setState(StateClosed, ex);
                Debug.Assert(_exception is not null);
                throw _exception;
            }

            if (response)
            {
                //
                // Add to the async requests map.
                //
                _asyncRequests[requestId] = og;
            }
            return status;
        }
    }

    internal BatchRequestQueue getBatchRequestQueue() => _batchRequestQueue;

    public void flushBatchRequests(CompressBatch compress)
    {
        try
        {
            var completed = new FlushBatchTaskCompletionCallback();
            var outgoing = new ConnectionFlushBatchAsync(this, _instance, completed);
            outgoing.invoke(_flushBatchRequests_name, compress, true);
            completed.Task.Wait();
        }
        catch (AggregateException ex)
        {
            throw ex.InnerException;
        }
    }

    public Task flushBatchRequestsAsync(
        CompressBatch compress,
        IProgress<bool> progress = null,
        CancellationToken cancel = default)
    {
        var completed = new FlushBatchTaskCompletionCallback(progress, cancel);
        var outgoing = new ConnectionFlushBatchAsync(this, _instance, completed);
        outgoing.invoke(_flushBatchRequests_name, compress, false);
        return completed.Task;
    }

    private const string _flushBatchRequests_name = "flushBatchRequests";

    public void setCloseCallback(CloseCallback callback)
    {
        lock (_mutex)
        {
            if (_state >= StateClosed)
            {
                if (callback is not null)
                {
                    _threadPool.execute(
                        () =>
                        {
                            try
                            {
                                callback(this);
                            }
                            catch (System.Exception ex)
                            {
                                _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                            }
                        },
                        this);
                }
            }
            else
            {
                _closeCallback = callback;
            }
        }
    }

    public void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex)
    {
        //
        // NOTE: This isn't called from a thread pool thread.
        //

        lock (_mutex)
        {
            if (_state >= StateClosed)
            {
                return; // The request has already been or will be shortly notified of the failure.
            }

            OutgoingMessage o = _sendStreams.FirstOrDefault(m => m.outAsync == outAsync);
            if (o is not null)
            {
                if (o.requestId > 0)
                {
                    _asyncRequests.Remove(o.requestId);
                }

                if (ex is ConnectionAbortedException)
                {
                    setState(StateClosed, ex);
                }
                else
                {
                    //
                    // If the request is being sent, don't remove it from the send streams,
                    // it will be removed once the sending is finished.
                    //
                    if (o == _sendStreams.First.Value)
                    {
                        o.canceled();
                    }
                    else
                    {
                        o.canceled();
                        _sendStreams.Remove(o);
                    }
                    if (outAsync.exception(ex))
                    {
                        outAsync.invokeExceptionAsync();
                    }
                }

                if (_closeRequested && _state < StateClosing && _asyncRequests.Count == 0)
                {
                    doApplicationClose();
                }
                return;
            }

            if (outAsync is OutgoingAsync)
            {
                foreach (KeyValuePair<int, OutgoingAsyncBase> kvp in _asyncRequests)
                {
                    if (kvp.Value == outAsync)
                    {
                        if (ex is ConnectionAbortedException)
                        {
                            setState(StateClosed, ex);
                        }
                        else
                        {
                            _asyncRequests.Remove(kvp.Key);
                            if (outAsync.exception(ex))
                            {
                                outAsync.invokeExceptionAsync();
                            }
                        }

                        if (_closeRequested && _state < StateClosing && _asyncRequests.Count == 0)
                        {
                            doApplicationClose();
                        }
                        return;
                    }
                }
            }
        }
    }

    internal EndpointI endpoint()
    {
        return _endpoint; // No mutex protection necessary, _endpoint is immutable.
    }

    internal Connector connector()
    {
        return _connector; // No mutex protection necessary, _endpoint is immutable.
    }

    public void setAdapter(ObjectAdapter adapter)
    {
        if (adapter is not null)
        {
            // Go through the adapter to set the adapter and servant manager on this connection
            // to ensure the object adapter is still active.
            adapter.setAdapterOnConnection(this);
        }
        else
        {
            lock (_mutex)
            {
                if (_state <= StateNotValidated || _state >= StateClosing)
                {
                    return;
                }
                _adapter = null;
            }
        }

        //
        // We never change the thread pool with which we were initially
        // registered, even if we add or remove an object adapter.
        //
    }

    public ObjectAdapter getAdapter()
    {
        lock (_mutex)
        {
            return _adapter;
        }
    }

    public Endpoint getEndpoint()
    {
        return _endpoint; // No mutex protection necessary, _endpoint is immutable.
    }

    public ObjectPrx createProxy(Identity id)
    {
        ObjectAdapter.checkIdentity(id);
        return new ObjectPrxHelper(_instance.referenceFactory().create(id, this));
    }

    public void setAdapterFromAdapter(ObjectAdapter adapter)
    {
        lock (_mutex)
        {
            if (_state <= StateNotValidated || _state >= StateClosing)
            {
                return;
            }
            Debug.Assert(adapter is not null); // Called by ObjectAdapter::setAdapterOnConnection
            _adapter = adapter;

            // Clear cached connection info (if any) as it's no longer accurate.
            _info = null;
        }
    }

    //
    // Operations from EventHandler
    //
    public override bool startAsync(int operation, Ice.Internal.AsyncCallback completedCallback)
    {
        if (_state >= StateClosed)
        {
            return false;
        }

        // Run the IO operation on a .NET thread pool thread to ensure the IO operation won't be interrupted if the
        // Ice thread pool thread is terminated (.NET Socket read/write fail with a SocketError.OperationAborted
        // error if started from a thread which is later terminated).
        Task.Run(() =>
        {
            lock (_mutex)
            {
                if (_state >= StateClosed)
                {
                    completedCallback(this);
                    return;
                }

                try
                {
                    if ((operation & SocketOperation.Write) != 0)
                    {
                        if (_observer != null)
                        {
                            observerStartWrite(_writeStream.getBuffer());
                        }

                        bool messageWritten = false;
                        bool completedSynchronously =
                            _transceiver.startWrite(
                                _writeStream.getBuffer(),
                                completedCallback,
                                this,
                                out messageWritten);
                        // If the startWrite call consumed the complete buffer, we assume the message is sent now for
                        // at-most-once semantics.
                        if (messageWritten && _sendStreams.Count > 0)
                        {
                            _sendStreams.First.Value.isSent = true;
                        }

                        if (completedSynchronously)
                        {
                            // If the write completed synchronously, we need to call the completedCallback.
                            completedCallback(this);
                        }
                    }
                    else if ((operation & SocketOperation.Read) != 0)
                    {
                        if (_observer != null && !_readHeader)
                        {
                            observerStartRead(_readStream.getBuffer());
                        }

                        if (_transceiver.startRead(_readStream.getBuffer(), completedCallback, this))
                        {
                            completedCallback(this);
                        }
                    }
                }
                catch (LocalException ex)
                {
                    setState(StateClosed, ex);
                    completedCallback(this);
                }
            }
        });

        return true;
    }

    public override bool finishAsync(int operation)
    {
        if (_state >= StateClosed)
        {
            return false;
        }

        try
        {
            if ((operation & SocketOperation.Write) != 0)
            {
                Ice.Internal.Buffer buf = _writeStream.getBuffer();
                int start = buf.b.position();
                _transceiver.finishWrite(buf);
                if (_instance.traceLevels().network >= 3 && buf.b.position() != start)
                {
                    StringBuilder s = new StringBuilder("sent ");
                    s.Append(buf.b.position() - start);
                    if (!_endpoint.datagram())
                    {
                        s.Append(" of ");
                        s.Append(buf.b.limit() - start);
                    }
                    s.Append(" bytes via ");
                    s.Append(_endpoint.protocol());
                    s.Append('\n');
                    s.Append(ToString());
                    _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                }

                if (_observer is not null)
                {
                    observerFinishWrite(_writeStream.getBuffer());
                }
            }
            else if ((operation & SocketOperation.Read) != 0)
            {
                Ice.Internal.Buffer buf = _readStream.getBuffer();
                int start = buf.b.position();
                _transceiver.finishRead(buf);
                if (_instance.traceLevels().network >= 3 && buf.b.position() != start)
                {
                    StringBuilder s = new StringBuilder("received ");
                    if (_endpoint.datagram())
                    {
                        s.Append(buf.b.limit());
                    }
                    else
                    {
                        s.Append(buf.b.position() - start);
                        s.Append(" of ");
                        s.Append(buf.b.limit() - start);
                    }
                    s.Append(" bytes via ");
                    s.Append(_endpoint.protocol());
                    s.Append('\n');
                    s.Append(ToString());
                    _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                }

                if (_observer is not null && !_readHeader)
                {
                    observerFinishRead(_readStream.getBuffer());
                }
            }
        }
        catch (LocalException ex)
        {
            setState(StateClosed, ex);
        }
        return _state < StateClosed;
    }

    public override void message(ThreadPoolCurrent current)
    {
        StartCallback startCB = null;
        Queue<OutgoingMessage> sentCBs = null;
        MessageInfo info = new MessageInfo();
        int upcallCount = 0;

        using ThreadPoolMessage msg = new ThreadPoolMessage(current, _mutex);
        lock (_mutex)
        {
            try
            {
                if (!msg.startIOScope())
                {
                    return;
                }

                if (_state >= StateClosed)
                {
                    return;
                }

                try
                {
                    int writeOp = SocketOperation.None;
                    int readOp = SocketOperation.None;

                    // If writes are ready, write the data from the connection's write buffer (_writeStream)
                    if ((current.operation & SocketOperation.Write) != 0)
                    {
                        if (_observer is not null)
                        {
                            observerStartWrite(_writeStream.getBuffer());
                        }
                        writeOp = write(_writeStream.getBuffer());
                        if (_observer is not null && (writeOp & SocketOperation.Write) == 0)
                        {
                            observerFinishWrite(_writeStream.getBuffer());
                        }
                    }

                    // If reads are ready, read the data into the connection's read buffer (_readStream). The data is
                    // read until:
                    // - the full message is read (the transport read returns SocketOperationNone) and
                    //   the read buffer is fully filled
                    // - the read operation on the transport can't continue without blocking
                    if ((current.operation & SocketOperation.Read) != 0)
                    {
                        while (true)
                        {
                            Ice.Internal.Buffer buf = _readStream.getBuffer();

                            if (_observer is not null && !_readHeader)
                            {
                                observerStartRead(buf);
                            }

                            readOp = read(buf);
                            if ((readOp & SocketOperation.Read) != 0)
                            {
                                // Can't continue without blocking, exit out of the loop.
                                break;
                            }
                            if (_observer is not null && !_readHeader)
                            {
                                Debug.Assert(!buf.b.hasRemaining());
                                observerFinishRead(buf);
                            }

                            // If read header is true, we're reading a new Ice protocol message and we need to read
                            // the message header.
                            if (_readHeader)
                            {
                                // The next read will read the remainder of the message.
                                _readHeader = false;

                                if (_observer is not null)
                                {
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

                                // Full header should be read because the size of _readStream is always headerSize (14)
                                // when reading a new message (see the code that sets _readHeader = true).
                                int pos = _readStream.pos();
                                if (pos < Protocol.headerSize)
                                {
                                    //
                                    // This situation is possible for small UDP packets.
                                    //
                                    throw new MarshalException("Received Ice message with too few bytes in header.");
                                }

                                // Decode the header.
                                _readStream.pos(0);
                                byte[] m = new byte[4];
                                m[0] = _readStream.readByte();
                                m[1] = _readStream.readByte();
                                m[2] = _readStream.readByte();
                                m[3] = _readStream.readByte();
                                if (m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
                                m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
                                {
                                    throw new ProtocolException(
                                        $"Bad magic in message header: {m[0]:X2} {m[1]:X2} {m[2]:X2} {m[3]:X2}");
                                }

                                var pv = new ProtocolVersion(_readStream);
                                if (pv != Util.currentProtocol)
                                {
                                    throw new MarshalException(
                                        $"Invalid protocol version in message header: {pv.major}.{pv.minor}");
                                }
                                var ev = new EncodingVersion(_readStream);
                                if (ev != Util.currentProtocolEncoding)
                                {
                                    throw new MarshalException(
                                        $"Invalid protocol encoding version in message header: {ev.major}.{ev.minor}");
                                }

                                _readStream.readByte(); // messageType
                                _readStream.readByte(); // compress
                                int size = _readStream.readInt();
                                if (size < Protocol.headerSize)
                                {
                                    throw new MarshalException($"Received Ice message with unexpected size {size}.");
                                }

                                // Resize the read buffer to the message size.
                                if (size > _messageSizeMax)
                                {
                                    Ex.throwMemoryLimitException(size, _messageSizeMax);
                                }
                                if (size > _readStream.size())
                                {
                                    _readStream.resize(size);
                                }
                                _readStream.pos(pos);
                            }

                            if (buf.b.hasRemaining())
                            {
                                if (_endpoint.datagram())
                                {
                                    throw new DatagramLimitException(); // The message was truncated.
                                }
                                continue;
                            }
                            break;
                        }
                    }

                    // readOp and writeOp are set to the operations that the transport read or write calls from above
                    // returned. They indicate which operations will need to be monitored by the thread pool's selector
                    // when this method returns.
                    int newOp = readOp | writeOp;

                    // Operations that are ready. For example, if message was called with SocketOperationRead and the
                    // transport read returned SocketOperationNone, reads are considered done: there's no additional
                    // data to read.
                    int readyOp = current.operation & ~newOp;

                    if (_state <= StateNotValidated)
                    {
                        // If the connection is still not validated and there's still data to read or write, continue
                        // waiting for data to read or write.
                        if (newOp != 0)
                        {
                            _threadPool.update(this, current.operation, newOp);
                            return;
                        }

                        // Initialize the connection if it's not initialized yet.
                        if (_state == StateNotInitialized && !initialize(current.operation))
                        {
                            return;
                        }

                        // Validate the connection if it's not validated yet.
                        if (_state <= StateNotValidated && !validate(current.operation))
                        {
                            return;
                        }

                        // The connection is validated and doesn't need additional data to be read or written. So
                        // unregister it from the thread pool's selector.
                        _threadPool.unregister(this, current.operation);

                        //
                        // We start out in holding state.
                        //
                        setState(StateHolding);
                        if (_startCallback is not null)
                        {
                            startCB = _startCallback;
                            _startCallback = null;
                            if (startCB is not null)
                            {
                                ++upcallCount;
                            }
                        }
                    }
                    else
                    {
                        Debug.Assert(_state <= StateClosingPending);

                        //
                        // We parse messages first, if we receive a close
                        // connection message we won't send more messages.
                        //
                        if ((readyOp & SocketOperation.Read) != 0)
                        {
                            // At this point, the protocol message is fully read and can therefore be decoded by
                            // parseMessage. parseMessage returns the operation to wait for readiness next.
                            newOp |= parseMessage(ref info);
                            upcallCount += info.upcallCount;
                        }

                        if ((readyOp & SocketOperation.Write) != 0)
                        {
                            // At this point the message from _writeStream is fully written and the next message can be
                            // written.

                            newOp |= sendNextMessage(out sentCBs);
                            if (sentCBs is not null)
                            {
                                ++upcallCount;
                            }
                        }

                        // If the connection is not closed yet, we can update the thread pool selector to wait for
                        // readiness of read, write or both operations.
                        if (_state < StateClosed)
                        {
                            _threadPool.update(this, current.operation, newOp);
                        }
                    }

                    if (upcallCount == 0)
                    {
                        return; // Nothing to execute, we're done!
                    }

                    _upcallCount += upcallCount;

                    // There's something to execute so we mark IO as completed to elect a new leader thread and let IO
                    // be performed on this new leader thread while this thread continues with executing the upcalls.
                    msg.ioCompleted();
                }
                catch (DatagramLimitException) // Expected.
                {
                    if (_warnUdp)
                    {
                        _logger.warning($"maximum datagram size of {_readStream.pos()} exceeded");
                    }
                    _readStream.resize(Protocol.headerSize);
                    _readStream.pos(0);
                    _readHeader = true;
                    return;
                }
                catch (SocketException ex)
                {
                    setState(StateClosed, ex);
                    return;
                }
                catch (LocalException ex)
                {
                    if (_endpoint.datagram())
                    {
                        if (_warn)
                        {
                            _logger.warning($"datagram connection exception:\n{ex}\n{_desc}");
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
            finally
            {
                msg.finishIOScope();
            }
        }

        _threadPool.executeFromThisThread(() => upcall(startCB, sentCBs, info), this);
    }

    private void upcall(StartCallback startCB, Queue<OutgoingMessage> sentCBs, MessageInfo info)
    {
        int completedUpcallCount = 0;

        //
        // Notify the factory that the connection establishment and
        // validation has completed.
        //
        if (startCB is not null)
        {
            startCB.connectionStartCompleted(this);
            ++completedUpcallCount;
        }

        //
        // Notify AMI calls that the message was sent.
        //
        if (sentCBs is not null)
        {
            foreach (OutgoingMessage m in sentCBs)
            {
                if (m.invokeSent)
                {
                    m.outAsync.invokeSent();
                }
                if (m.receivedReply)
                {
                    OutgoingAsync outAsync = (OutgoingAsync)m.outAsync;
                    if (outAsync.response())
                    {
                        outAsync.invokeResponse();
                    }
                }
            }
            ++completedUpcallCount;
        }

        //
        // Asynchronous replies must be handled outside the thread
        // synchronization, so that nested calls are possible.
        //
        if (info.outAsync is not null)
        {
            info.outAsync.invokeResponse();
            ++completedUpcallCount;
        }

        //
        // Method invocation (or multiple invocations for batch messages)
        // must be done outside the thread synchronization, so that nested
        // calls are possible.
        //
        if (info.requestCount > 0)
        {
            dispatchAll(info.stream, info.requestCount, info.requestId, info.compress, info.adapter);
        }

        //
        // Decrease the upcall count.
        //
        bool finished = false;
        if (completedUpcallCount > 0)
        {
            lock (_mutex)
            {
                _upcallCount -= completedUpcallCount;
                if (_upcallCount == 0)
                {
                    // Only initiate shutdown if not already initiated. It might have already been initiated if the sent
                    // callback or AMI callback was called when the connection was in the closing state.
                    if (_state == StateClosing)
                    {
                        try
                        {
                            initiateShutdown();
                        }
                        catch (Ice.LocalException ex)
                        {
                            setState(StateClosed, ex);
                        }
                    }
                    else if (_state == StateFinished)
                    {
                        finished = true;
                        _observer?.detach();
                    }
                    Monitor.PulseAll(_mutex);
                }
            }
        }

        if (finished && _removeFromFactory is not null)
        {
            _removeFromFactory(this);
        }
    }

    public override void finished(ThreadPoolCurrent current)
    {
        // Lock the connection here to ensure setState() completes before the code below is executed. This method can
        // be called by the thread pool as soon as setState() calls _threadPool->finish(...). There's no need to lock
        // the mutex for the remainder of the code because the data members accessed by finish() are immutable once
        // _state == StateClosed (and we don't want to hold the mutex when calling upcalls).
        lock (_mutex)
        {
            Debug.Assert(_state == StateClosed);
        }

        //
        // If there are no callbacks to call, we don't call ioCompleted() since we're not going
        // to call code that will potentially block (this avoids promoting a new leader and
        // unnecessary thread creation, especially if this is called on shutdown).
        //
        if (_startCallback is null && _sendStreams.Count == 0 && _asyncRequests.Count == 0 && _closeCallback is null)
        {
            finish();
            return;
        }

        current.ioCompleted();
        _threadPool.executeFromThisThread(finish, this);
    }

    private void finish()
    {
        if (!_initialized)
        {
            if (_instance.traceLevels().network >= 2)
            {
                StringBuilder s = new StringBuilder("failed to ");
                s.Append(_connector is not null ? "establish" : "accept");
                s.Append(' ');
                s.Append(_endpoint.protocol());
                s.Append(" connection\n");
                s.Append(ToString());
                s.Append('\n');
                s.Append(_exception);
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
            }
        }
        else
        {
            if (_instance.traceLevels().network >= 1)
            {
                StringBuilder s = new StringBuilder("closed ");
                s.Append(_endpoint.protocol());
                s.Append(" connection\n");
                s.Append(ToString());

                //
                // Trace the cause of unexpected connection closures
                //
                if (!(_exception is CloseConnectionException ||
                     _exception is ConnectionAbortedException ||
                     _exception is ConnectionClosedException ||
                     _exception is CommunicatorDestroyedException ||
                     _exception is ObjectAdapterDeactivatedException))
                {
                    s.Append('\n');
                    s.Append(_exception);
                }

                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
            }
        }

        if (_startCallback is not null)
        {
            _startCallback.connectionStartFailed(this, _exception);
            _startCallback = null;
        }

        if (_sendStreams.Count > 0)
        {
            if (!_writeStream.isEmpty())
            {
                //
                // Return the stream to the outgoing call. This is important for
                // retriable AMI calls which are not marshaled again.
                //
                OutgoingMessage message = _sendStreams.First.Value;
                _writeStream.swap(message.stream);

                //
                // The current message might be sent but not yet removed from _sendStreams. If
                // the response has been received in the meantime, we remove the message from
                // _sendStreams to not call finished on a message which is already done.
                //
                if (message.isSent || message.receivedReply)
                {
                    if (message.sent() && message.invokeSent)
                    {
                        message.outAsync.invokeSent();
                    }
                    if (message.receivedReply)
                    {
                        OutgoingAsync outAsync = (OutgoingAsync)message.outAsync;
                        if (outAsync.response())
                        {
                            outAsync.invokeResponse();
                        }
                    }
                    _sendStreams.RemoveFirst();
                }
            }

            foreach (OutgoingMessage o in _sendStreams)
            {
                o.completed(_exception);
                if (o.requestId > 0) // Make sure finished isn't called twice.
                {
                    _asyncRequests.Remove(o.requestId);
                }
            }
            _sendStreams.Clear(); // Must be cleared before _requests because of Outgoing* references in OutgoingMessage
        }

        foreach (OutgoingAsyncBase o in _asyncRequests.Values)
        {
            if (o.exception(_exception))
            {
                o.invokeException();
            }
        }
        _asyncRequests.Clear();

        //
        // Don't wait to be reaped to reclaim memory allocated by read/write streams.
        //
        _writeStream.clear();
        _writeStream.getBuffer().clear();
        _readStream.clear();
        _readStream.getBuffer().clear();

        if (_exception is ConnectionClosedException or
            CloseConnectionException or
            CommunicatorDestroyedException or
            ObjectAdapterDeactivatedException)
        {
            // Can execute synchronously. Note that we're not within a lock(this) here.
            _closed.SetResult();
        }
        else
        {
            Debug.Assert(_exception is not null);
            _closed.SetException(_exception);
        }

        if (_closeCallback is not null)
        {
            try
            {
                _closeCallback(this);
            }
            catch (System.Exception ex)
            {
                _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
            }
            _closeCallback = null;
        }

        //
        // This must be done last as this will cause waitUntilFinished() to return (and communicator
        // objects such as the timer might be destroyed too).
        //
        bool finished = false;
        lock (_mutex)
        {
            setState(StateFinished);

            if (_upcallCount == 0)
            {
                finished = true;
                _observer?.detach();
            }
        }

        if (finished && _removeFromFactory is not null)
        {
            _removeFromFactory(this);
        }
    }

    /// <inheritdoc/>
    public override string ToString()
    {
        return _desc; // No mutex lock, _desc is immutable.
    }

    /// <inheritdoc/>
    public string type()
    {
        return _type; // No mutex lock, _type is immutable.
    }

    /// <inheritdoc/>
    public ConnectionInfo getInfo()
    {
        lock (_mutex)
        {
            if (_state >= StateClosed)
            {
                throw _exception;
            }
            return initConnectionInfo();
        }
    }

    /// <inheritdoc/>
    public void setBufferSize(int rcvSize, int sndSize)
    {
        lock (_mutex)
        {
            if (_state >= StateClosed)
            {
                throw _exception;
            }
            _transceiver.setBufferSize(rcvSize, sndSize);
            _info = null; // Invalidate the cached connection info
        }
    }

    public void exception(LocalException ex)
    {
        lock (_mutex)
        {
            setState(StateClosed, ex);
        }
    }

    public Ice.Internal.ThreadPool getThreadPool()
    {
        return _threadPool;
    }

    internal ConnectionI(
        Instance instance,
        Transceiver transceiver,
        Connector connector, // null for incoming connections, non-null for outgoing connections
        EndpointI endpoint,
        ObjectAdapter adapter,
        Action<ConnectionI> removeFromFactory, // can be null
        ConnectionOptions options)
    {
        _instance = instance;
        _desc = transceiver.ToString();
        _type = transceiver.protocol();
        _connector = connector;
        _endpoint = endpoint;
        _adapter = adapter;
        InitializationData initData = instance.initializationData();
        _logger = initData.logger; // Cached for better performance.
        _traceLevels = instance.traceLevels(); // Cached for better performance.
        _connectTimeout = options.connectTimeout;
        _closeTimeout = options.closeTimeout; // not used for datagram connections
        // suppress inactivity timeout for datagram connections
        _inactivityTimeout = endpoint.datagram() ? TimeSpan.Zero : options.inactivityTimeout;
        _maxDispatches = options.maxDispatches;
        _removeFromFactory = removeFromFactory;
        _warn = initData.properties.getIcePropertyAsInt("Ice.Warn.Connections") > 0;
        _warnUdp = initData.properties.getIcePropertyAsInt("Ice.Warn.Datagrams") > 0;
        _nextRequestId = 1;
        _messageSizeMax = connector is null ? adapter.messageSizeMax() : instance.messageSizeMax();
        _batchRequestQueue = new BatchRequestQueue(instance, _endpoint.datagram());
        _readStream = new InputStream(instance, Util.currentProtocolEncoding);
        _readHeader = false;
        _readStreamPos = -1;
        _writeStream = new OutputStream(); // temporary stream
        _writeStreamPos = -1;
        _upcallCount = 0;
        _state = StateNotInitialized;

        _compressionLevel = initData.properties.getIcePropertyAsInt("Ice.Compression.Level");
        if (_compressionLevel < 1)
        {
            _compressionLevel = 1;
        }
        else if (_compressionLevel > 9)
        {
            _compressionLevel = 9;
        }

        if (options.idleTimeout > TimeSpan.Zero && !endpoint.datagram())
        {
            _idleTimeoutTransceiver = new IdleTimeoutTransceiverDecorator(
                transceiver,
                this,
                options.idleTimeout,
                options.enableIdleCheck);
            transceiver = _idleTimeoutTransceiver;
        }
        _transceiver = transceiver;

        try
        {
            if (connector is null)
            {
                // adapter is always set for incoming connections
                Debug.Assert(adapter is not null);
                _threadPool = adapter.getThreadPool();
            }
            else
            {
                // we use the client thread pool for outgoing connections, even if there is an
                // object adapter with its own thread pool.
                _threadPool = instance.clientThreadPool();
            }
            _threadPool.initialize(this);
        }
        catch (LocalException)
        {
            throw;
        }
        catch (System.Exception ex)
        {
            throw new SyscallException(ex);
        }
    }

    /// <summary>Aborts the connection with a <see cref="ConnectionAbortedException" /> if the connection is active and
    /// does not receive a byte for some time. See the IdleTimeoutTransceiverDecorator.</summary>
    internal void idleCheck(TimeSpan idleTimeout)
    {
        lock (_mutex)
        {
            if (_state == StateActive && _idleTimeoutTransceiver!.idleCheckEnabled)
            {
                int idleTimeoutInSeconds = (int)idleTimeout.TotalSeconds;

                if (_instance.traceLevels().network >= 1)
                {
                    _instance.initializationData().logger.trace(
                        _instance.traceLevels().networkCat,
                        $"connection aborted by the idle check because it did not receive any bytes for {idleTimeoutInSeconds}s\n{_transceiver.toDetailedString()}");
                }

                setState(
                    StateClosed,
                    new ConnectionAbortedException(
                        $"Connection aborted by the idle check because it did not receive any bytes for {idleTimeoutInSeconds}s.",
                        closedByApplication: false));
            }
            // else nothing to do
        }
    }

    internal void sendHeartbeat()
    {
        Debug.Assert(!_endpoint.datagram());

        lock (_mutex)
        {
            if (_state == StateActive || _state == StateHolding || _state == StateClosing)
            {
                // We check if the connection has become inactive.
                if (
                    _inactivityTimer is null &&           // timer not already scheduled
                    _inactivityTimeout > TimeSpan.Zero && // inactivity timeout is enabled
                    _state == StateActive &&              // only schedule the timer if the connection is active
                    _dispatchCount == 0 &&                // no pending dispatch
                    _asyncRequests.Count == 0 &&          // no pending invocation
                    _readHeader &&                        // we're not waiting for the remainder of an incoming message
                    _sendStreams.Count <= 1)              // there is at most one pending outgoing message
                {
                    // We may become inactive while the peer is back-pressuring us. In this case, we only schedule the
                    // inactivity timer if there is no pending outgoing message or the pending outgoing message is a
                    // heartbeat.

                    // The stream of the first _sendStreams message is in _writeStream.
                    if (_sendStreams.Count == 0 || isHeartbeat(_writeStream))
                    {
                        scheduleInactivityTimer();
                    }
                }

                // We send a heartbeat to the peer to generate a "write" on the connection. This write in turns creates
                // a read on the peer, and resets the peer's idle check timer. When _sendStream is not empty, there is
                // already an outstanding write, so we don't need to send a heartbeat. It's possible the first message
                // of _sendStreams was already sent but not yet removed from _sendStreams: it means the last write
                // occurred very recently, which is good enough with respect to the idle check.
                // As a result of this optimization, the only possible heartbeat in _sendStreams is the first
                // _sendStreams message.
                if (_sendStreams.Count == 0)
                {
                    var os = new OutputStream(Util.currentProtocolEncoding);
                    os.writeBlob(Protocol.magic);
                    Util.currentProtocol.ice_writeMembers(os);
                    Util.currentProtocolEncoding.ice_writeMembers(os);
                    os.writeByte(Protocol.validateConnectionMsg);
                    os.writeByte(0);
                    os.writeInt(Protocol.headerSize); // Message size.
                    try
                    {
                        _ = sendMessage(new OutgoingMessage(os, compress: false));
                    }
                    catch (LocalException ex)
                    {
                        setState(StateClosed, ex);
                    }
                }
            }
            // else nothing to do
        }

        static bool isHeartbeat(OutputStream stream) =>
            stream.getBuffer().b.get(8) == Protocol.validateConnectionMsg;
    }

    private const int StateNotInitialized = 0;
    private const int StateNotValidated = 1;
    private const int StateActive = 2;
    private const int StateHolding = 3;
    private const int StateClosing = 4;
    private const int StateClosingPending = 5;
    private const int StateClosed = 6;
    private const int StateFinished = 7;

    private static ConnectionState toConnectionState(int state)
    {
        return connectionStateMap[state];
    }

    private void setState(int state, LocalException ex)
    {
        //
        // If setState() is called with an exception, then only closed
        // and closing states are permissible.
        //
        Debug.Assert(state >= StateClosing);

        if (_state == state) // Don't switch twice.
        {
            return;
        }

        if (_exception is null)
        {
            //
            // If we are in closed state, an exception must be set.
            //
            Debug.Assert(_state != StateClosed);

            _exception = ex;

            //
            // We don't warn if we are not validated.
            //
            if (_warn && _validated)
            {
                //
                // Don't warn about certain expected exceptions.
                //
                if (!(_exception is CloseConnectionException ||
                     _exception is ConnectionAbortedException ||
                     _exception is ConnectionClosedException ||
                     _exception is CommunicatorDestroyedException ||
                     _exception is ObjectAdapterDeactivatedException ||
                     (_exception is ConnectionLostException && _state >= StateClosing)))
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
        if (_endpoint.datagram() && state == StateClosing)
        {
            state = StateClosed;
        }

        //
        // Skip graceful shutdown if we are destroyed before validation.
        //
        if (_state <= StateNotValidated && state == StateClosing)
        {
            state = StateClosed;
        }

        if (_state == state) // Don't switch twice.
        {
            return;
        }

        if (state > StateActive)
        {
            // Dispose the inactivity timer, if not null.
            cancelInactivityTimer();
        }

        try
        {
            switch (state)
            {
                case StateNotInitialized:
                {
                    Debug.Assert(false);
                    break;
                }

                case StateNotValidated:
                {
                    if (_state != StateNotInitialized)
                    {
                        Debug.Assert(_state == StateClosed);
                        return;
                    }
                    break;
                }

                case StateActive:
                {
                    //
                    // Can only switch to active from holding or not validated.
                    //
                    if (_state != StateHolding && _state != StateNotValidated)
                    {
                        return;
                    }

                    if (_maxDispatches <= 0 || _dispatchCount < _maxDispatches)
                    {
                        _threadPool.register(this, SocketOperation.Read);
                        _idleTimeoutTransceiver?.enableIdleCheck();
                    }
                    // else don't resume reading since we're at or over the _maxDispatches limit.

                    break;
                }

                case StateHolding:
                {
                    //
                    // Can only switch to holding from active or not validated.
                    //
                    if (_state != StateActive && _state != StateNotValidated)
                    {
                        return;
                    }

                    if (_state == StateActive && (_maxDispatches <= 0 || _dispatchCount < _maxDispatches))
                    {
                        _threadPool.unregister(this, SocketOperation.Read);
                        _idleTimeoutTransceiver?.disableIdleCheck();
                    }
                    // else reads are already disabled because the _maxDispatches limit is reached or exceeded.

                    break;
                }

                case StateClosing:
                case StateClosingPending:
                {
                    //
                    // Can't change back from closing pending.
                    //
                    if (_state >= StateClosingPending)
                    {
                        return;
                    }
                    break;
                }

                case StateClosed:
                {
                    if (_state == StateFinished)
                    {
                        return;
                    }

                    _batchRequestQueue.destroy(_exception);
                    _threadPool.finish(this);
                    _transceiver.close();
                    break;
                }

                case StateFinished:
                {
                    Debug.Assert(_state == StateClosed);
                    _transceiver.destroy();
                    break;
                }
            }
        }
        catch (LocalException ex)
        {
            _logger.error("unexpected connection exception:\n" + ex + "\n" + _transceiver.ToString());
        }

        if (_instance.initializationData().observer is not null)
        {
            ConnectionState oldState = toConnectionState(_state);
            ConnectionState newState = toConnectionState(state);
            if (oldState != newState)
            {
                _observer = _instance.initializationData().observer.getConnectionObserver(
                    initConnectionInfo(),
                    _endpoint,
                    newState,
                    _observer);
                if (_observer is not null)
                {
                    _observer.attach();
                }
                else
                {
                    _writeStreamPos = -1;
                    _readStreamPos = -1;
                }
            }
            if (_observer is not null && state == StateClosed && _exception is not null)
            {
                if (!(_exception is CloseConnectionException ||
                     _exception is ConnectionAbortedException ||
                     _exception is ConnectionClosedException ||
                     _exception is CommunicatorDestroyedException ||
                     _exception is ObjectAdapterDeactivatedException ||
                     (_exception is ConnectionLostException && _state >= StateClosing)))
                {
                    _observer.failed(_exception.ice_id());
                }
            }
        }
        _state = state;

        Monitor.PulseAll(_mutex);

        if (_state == StateClosing && _upcallCount == 0)
        {
            try
            {
                initiateShutdown();
            }
            catch (LocalException ex)
            {
                setState(StateClosed, ex);
            }
        }
    }

    private void initiateShutdown()
    {
        Debug.Assert(_state == StateClosing && _upcallCount == 0);

        if (_shutdownInitiated)
        {
            return;
        }
        _shutdownInitiated = true;

        if (!_endpoint.datagram())
        {
            //
            // Before we shut down, we send a close connection message.
            //
            var os = new OutputStream(Util.currentProtocolEncoding);
            os.writeBlob(Protocol.magic);
            Util.currentProtocol.ice_writeMembers(os);
            Util.currentProtocolEncoding.ice_writeMembers(os);
            os.writeByte(Protocol.closeConnectionMsg);
            os.writeByte(0); // Compression status: always zero for close connection.
            os.writeInt(Protocol.headerSize); // Message size.

            scheduleCloseTimer();

            if ((sendMessage(new OutgoingMessage(os, compress: false)) & OutgoingAsyncBase.AsyncStatusSent) != 0)
            {
                setState(StateClosingPending);

                //
                // Notify the transceiver of the graceful connection closure.
                //
                int op = _transceiver.closing(true, _exception);
                if (op != 0)
                {
                    _threadPool.register(this, op);
                }
            }
        }
    }

    private bool initialize(int operation)
    {
        int s = _transceiver.initialize(_readStream.getBuffer(), _writeStream.getBuffer(), ref _hasMoreData);
        if (s != SocketOperation.None)
        {
            _threadPool.update(this, operation, s);
            return false;
        }

        //
        // Update the connection description once the transceiver is initialized.
        //
        _desc = _transceiver.ToString();
        _initialized = true;
        setState(StateNotValidated);

        return true;
    }

    private bool validate(int operation)
    {
        if (!_endpoint.datagram()) // Datagram connections are always implicitly validated.
        {
            if (_connector is null) // The server side has the active role for connection validation.
            {
                if (_writeStream.size() == 0)
                {
                    _writeStream.writeBlob(Protocol.magic);
                    Util.currentProtocol.ice_writeMembers(_writeStream);
                    Util.currentProtocolEncoding.ice_writeMembers(_writeStream);
                    _writeStream.writeByte(Protocol.validateConnectionMsg);
                    _writeStream.writeByte(0); // Compression status (always zero for validate connection).
                    _writeStream.writeInt(Protocol.headerSize); // Message size.
                    TraceUtil.traceSend(_writeStream, _instance, this, _logger, _traceLevels);
                    _writeStream.prepareWrite();
                }

                if (_observer is not null)
                {
                    observerStartWrite(_writeStream.getBuffer());
                }

                if (_writeStream.pos() != _writeStream.size())
                {
                    int op = write(_writeStream.getBuffer());
                    if (op != 0)
                    {
                        _threadPool.update(this, operation, op);
                        return false;
                    }
                }

                if (_observer is not null)
                {
                    observerFinishWrite(_writeStream.getBuffer());
                }
            }
            else // The client side has the passive role for connection validation.
            {
                if (_readStream.size() == 0)
                {
                    _readStream.resize(Protocol.headerSize);
                    _readStream.pos(0);
                }

                if (_observer is not null)
                {
                    observerStartRead(_readStream.getBuffer());
                }

                if (_readStream.pos() != _readStream.size())
                {
                    int op = read(_readStream.getBuffer());
                    if (op != 0)
                    {
                        _threadPool.update(this, operation, op);
                        return false;
                    }
                }

                if (_observer is not null)
                {
                    observerFinishRead(_readStream.getBuffer());
                }

                _validated = true;

                Debug.Assert(_readStream.pos() == Protocol.headerSize);
                _readStream.pos(0);
                byte[] m = _readStream.readBlob(4);
                if (m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
                   m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
                {
                    throw new ProtocolException(
                        $"Bad magic in message header: {m[0]:X2} {m[1]:X2} {m[2]:X2} {m[3]:X2}");
                }

                var pv = new ProtocolVersion(_readStream);
                if (pv != Util.currentProtocol)
                {
                    throw new MarshalException(
                        $"Invalid protocol version in message header: {pv.major}.{pv.minor}");
                }
                var ev = new EncodingVersion(_readStream);
                if (ev != Util.currentProtocolEncoding)
                {
                    throw new MarshalException(
                        $"Invalid protocol encoding version in message header: {ev.major}.{ev.minor}");
                }

                byte messageType = _readStream.readByte();
                if (messageType != Protocol.validateConnectionMsg)
                {
                    throw new ProtocolException(
                        $"Received message of type {messageType} over a connection that is not yet validated.");
                }
                _readStream.readByte(); // Ignore compression status for validate connection.
                int size = _readStream.readInt();
                if (size != Protocol.headerSize)
                {
                    throw new MarshalException($"Received ValidateConnection message with unexpected size {size}.");
                }
                TraceUtil.traceRecv(_readStream, this, _logger, _traceLevels);

                // Client connection starts sending heartbeats once it's received the ValidateConnection message.
                _idleTimeoutTransceiver?.scheduleHeartbeat();
            }
        }

        _writeStream.resize(0);
        _writeStream.pos(0);

        _readStream.resize(Protocol.headerSize);
        _readStream.pos(0);
        _readHeader = true;

        if (_instance.traceLevels().network >= 1)
        {
            StringBuilder s = new StringBuilder();
            if (_endpoint.datagram())
            {
                s.Append("starting to ");
                s.Append(_connector is not null ? "send" : "receive");
                s.Append(' ');
                s.Append(_endpoint.protocol());
                s.Append(" messages\n");
                s.Append(_transceiver.toDetailedString());
            }
            else
            {
                s.Append(_connector is not null ? "established" : "accepted");
                s.Append(' ');
                s.Append(_endpoint.protocol());
                s.Append(" connection\n");
                s.Append(ToString());
            }
            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
        }

        return true;
    }

    /// <summary>
    /// Sends the next queued messages. This method is called by message() once the message which is being sent
    /// (_sendStreams.First) is fully sent. Before sending the next message, this message is removed from _sendsStream
    /// If any, its sent callback is also queued in given callback queue.
    /// </summary>
    /// <param name="callbacks">The sent callbacks to call for the messages that were sent.</param>
    /// <returns>The socket operation to register with the thread pool's selector to send the remainder of the pending
    /// message being sent (_sendStreams.First).</returns>
    private int sendNextMessage(out Queue<OutgoingMessage> callbacks)
    {
        callbacks = null;

        if (_sendStreams.Count == 0)
        {
            // This can occur if no message was being written and the socket write operation was registered with the
            // thread pool (a transceiver read method can request writing data).
            return SocketOperation.None;
        }
        else if (_state == StateClosingPending && _writeStream.pos() == 0)
        {
            // Message wasn't sent, empty the _writeStream, we're not going to send more data because the connection
            // is being closed.
            OutgoingMessage message = _sendStreams.First.Value;
            _writeStream.swap(message.stream);
            return SocketOperation.None;
        }

        // Assert that the message was fully written.
        Debug.Assert(!_writeStream.isEmpty() && _writeStream.pos() == _writeStream.size());

        try
        {
            while (true)
            {
                //
                // The message that was being sent is sent. We can swap back the write stream buffer to the
                // outgoing message (required for retry) and queue its sent callback (if any).
                //
                OutgoingMessage message = _sendStreams.First.Value;
                _writeStream.swap(message.stream);
                if (message.sent())
                {
                    if (callbacks is null)
                    {
                        callbacks = new Queue<OutgoingMessage>();
                    }
                    callbacks.Enqueue(message);
                }
                _sendStreams.RemoveFirst();

                //
                // If there's nothing left to send, we're done.
                //
                if (_sendStreams.Count == 0)
                {
                    break;
                }

                //
                // If we are in the closed state or if the close is pending, don't continue sending. This can occur if
                // parseMessage (called before sendNextMessage by message()) closes the connection.
                //
                if (_state >= StateClosingPending)
                {
                    return SocketOperation.None;
                }

                //
                // Otherwise, prepare the next message.
                //
                message = _sendStreams.First.Value;
                Debug.Assert(!message.prepared);
                OutputStream stream = message.stream;

                message.stream = doCompress(message.stream, message.compress);
                message.stream.prepareWrite();
                message.prepared = true;

                TraceUtil.traceSend(stream, _instance, this, _logger, _traceLevels);

                //
                // Send the message.
                //
                _writeStream.swap(message.stream);
                if (_observer is not null)
                {
                    observerStartWrite(_writeStream.getBuffer());
                }
                if (_writeStream.pos() != _writeStream.size())
                {
                    int op = write(_writeStream.getBuffer());
                    if (op != 0)
                    {
                        return op;
                    }
                }
                if (_observer is not null)
                {
                    observerFinishWrite(_writeStream.getBuffer());
                }

                // If the message was sent right away, loop to send the next queued message.
            }

            // Once the CloseConnection message is sent, we transition to the StateClosingPending state.
            if (_state == StateClosing && _shutdownInitiated)
            {
                setState(StateClosingPending);
                int op = _transceiver.closing(true, _exception);
                if (op != 0)
                {
                    return op;
                }
            }
        }
        catch (LocalException ex)
        {
            setState(StateClosed, ex);
        }
        return SocketOperation.None;
    }

    /// <summary>
    /// Sends or queues the given message.
    /// </summary>
    /// <param name="message">The message to send.</param>
    /// <returns>The send status.</returns>
    private int sendMessage(OutgoingMessage message)
    {
        Debug.Assert(_state >= StateActive);
        Debug.Assert(_state < StateClosed);

        // Some messages are queued for sending. Just adds the message to the send queue and tell the caller that
        // the message was queued.
        if (_sendStreams.Count > 0)
        {
            _sendStreams.AddLast(message);
            return OutgoingAsyncBase.AsyncStatusQueued;
        }

        // Prepare the message for sending.
        Debug.Assert(!message.prepared);

        OutputStream stream = message.stream;

        message.stream = doCompress(stream, message.compress);
        message.stream.prepareWrite();
        message.prepared = true;

        TraceUtil.traceSend(stream, _instance, this, _logger, _traceLevels);

        // Send the message without blocking.
        if (_observer is not null)
        {
            observerStartWrite(message.stream.getBuffer());
        }
        int op = write(message.stream.getBuffer());
        if (op == 0)
        {
            // The message was sent so we're done.

            if (_observer is not null)
            {
                observerFinishWrite(message.stream.getBuffer());
            }

            int status = OutgoingAsyncBase.AsyncStatusSent;
            if (message.sent())
            {
                // If there's a sent callback, indicate the caller that it should invoke the sent callback.
                status = status | OutgoingAsyncBase.AsyncStatusInvokeSentCallback;
            }

            return status;
        }

        // The message couldn't be sent right away so we add it to the send stream queue (which is empty) and swap its
        // stream with `_writeStream`. The socket operation returned by the transceiver write is registered with the
        // thread pool. At this point the message() method will take care of sending the whole message (held by
        // _writeStream) when the transceiver is ready to write more of the message buffer.

        _writeStream.swap(message.stream);
        _sendStreams.AddLast(message);
        _threadPool.register(this, op);
        return OutgoingAsyncBase.AsyncStatusQueued;
    }

    private OutputStream doCompress(OutputStream decompressed, bool compress)
    {
        if (BZip2.isLoaded(_logger) && compress && decompressed.size() >= 100)
        {
            //
            // Do compression.
            //
            Ice.Internal.Buffer cbuf = BZip2.compress(
                decompressed.getBuffer(),
                Protocol.headerSize,
                _compressionLevel);
            if (cbuf is not null)
            {
                OutputStream cstream = new OutputStream(new Internal.Buffer(cbuf, true), decompressed.getEncoding());

                //
                // Set compression status.
                //
                cstream.pos(9);
                cstream.writeByte(2);

                //
                // Write the size of the compressed stream into the header.
                //
                cstream.pos(10);
                cstream.writeInt(cstream.size());

                //
                // Write the compression status and size of the compressed stream into the header of the
                // decompressed stream -- we need this to trace requests correctly.
                //
                decompressed.pos(9);
                decompressed.writeByte(2);
                decompressed.writeInt(cstream.size());

                return cstream;
            }
        }

        // Write the compression status. If BZip2 is loaded and compress is set to true, we write 1, to request a
        // compressed reply. Otherwise, we write 0 either BZip2 is not loaded or we are sending an uncompressed reply.
        decompressed.pos(9);
        decompressed.writeByte((byte)((BZip2.isLoaded(_logger) && compress) ? 1 : 0));

        //
        // Not compressed, fill in the message size.
        //
        decompressed.pos(10);
        decompressed.writeInt(decompressed.size());

        return decompressed;
    }

    private struct MessageInfo
    {
        public InputStream stream;
        public int requestCount;
        public int requestId;
        public byte compress;
        public ObjectAdapter adapter;
        public OutgoingAsyncBase outAsync;
        public int upcallCount;
    }

    private int parseMessage(ref MessageInfo info)
    {
        Debug.Assert(_state > StateNotValidated && _state < StateClosed);

        info.stream = new InputStream(_instance, Util.currentProtocolEncoding);
        _readStream.swap(info.stream);
        _readStream.resize(Protocol.headerSize);
        _readStream.pos(0);
        _readHeader = true;

        Debug.Assert(info.stream.pos() == info.stream.size());

        try
        {
            //
            // The magic and version fields have already been checked.
            //
            info.stream.pos(8);
            byte messageType = info.stream.readByte();
            info.compress = info.stream.readByte();
            if (info.compress == 2)
            {
                if (BZip2.isLoaded(_logger))
                {
                    Ice.Internal.Buffer ubuf = BZip2.decompress(
                        info.stream.getBuffer(),
                        Protocol.headerSize,
                        _messageSizeMax);
                    info.stream = new InputStream(info.stream.instance(), info.stream.getEncoding(), ubuf, true);
                }
                else
                {
                    throw new FeatureNotSupportedException(
                        "Cannot decompress compressed message: BZip2 library is not loaded.");
                }
            }
            info.stream.pos(Protocol.headerSize);

            switch (messageType)
            {
                case Protocol.closeConnectionMsg:
                {
                    TraceUtil.traceRecv(info.stream, this, _logger, _traceLevels);
                    if (_endpoint.datagram())
                    {
                        if (_warn)
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
                        if (op != 0)
                        {
                            scheduleCloseTimer();
                            return op;
                        }
                        setState(StateClosed);
                    }
                    break;
                }

                case Protocol.requestMsg:
                {
                    if (_state >= StateClosing)
                    {
                        TraceUtil.trace(
                            "received request during closing\n(ignored by server, client will retry)",
                            info.stream,
                            this,
                            _logger,
                            _traceLevels);
                    }
                    else
                    {
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
                    if (_state >= StateClosing)
                    {
                        TraceUtil.trace(
                            "received batch request during closing\n(ignored by server, client will retry)",
                            info.stream,
                            this,
                            _logger,
                            _traceLevels);
                    }
                    else
                    {
                        TraceUtil.traceRecv(info.stream, this, _logger, _traceLevels);
                        int requestCount = info.stream.readInt();
                        if (requestCount < 0)
                        {
                            throw new MarshalException($"Received batch request with {requestCount} batches.");
                        }
                        info.requestCount = requestCount;
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
                    if (_asyncRequests.TryGetValue(info.requestId, out info.outAsync))
                    {
                        _asyncRequests.Remove(info.requestId);

                        info.outAsync.getIs().swap(info.stream);

                        //
                        // If we just received the reply for a request which isn't acknowledge as
                        // sent yet, we queue the reply instead of processing it right away. It
                        // will be processed once the write callback is invoked for the message.
                        //
                        OutgoingMessage message = _sendStreams.Count > 0 ? _sendStreams.First.Value : null;
                        if (message is not null && message.outAsync == info.outAsync)
                        {
                            message.receivedReply = true;
                        }
                        else if (info.outAsync.response())
                        {
                            ++info.upcallCount;
                        }
                        else
                        {
                            info.outAsync = null;
                        }
                        if (_closeRequested && _state < StateClosing && _asyncRequests.Count == 0)
                        {
                            doApplicationClose();
                        }
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

                    throw new ProtocolException($"Received Ice protocol message with unknown type: {messageType}");
                }
            }
        }
        catch (LocalException ex)
        {
            if (_endpoint.datagram())
            {
                if (_warn)
                {
                    _logger.warning("datagram connection exception:\n" + ex.ToString() + "\n" + _desc);
                }
            }
            else
            {
                setState(StateClosed, ex);
            }
        }

        if (_state == StateHolding)
        {
            // Don't continue reading if the connection is in the holding state.
            return SocketOperation.None;
        }
        else if (_maxDispatches > 0 && _dispatchCount >= _maxDispatches)
        {
            // Don't continue reading if the _maxDispatches limit is reached or exceeded.
            _idleTimeoutTransceiver?.disableIdleCheck();
            return SocketOperation.None;
        }
        else
        {
            // Continue reading.
            return SocketOperation.Read;
        }
    }

    private void dispatchAll(
        InputStream stream,
        int requestCount,
        int requestId,
        byte compress,
        ObjectAdapter adapter)
    {
        // Note: In contrast to other private or protected methods, this method must be called *without* the mutex
        // locked.

        Object dispatcher = adapter?.dispatchPipeline;

        try
        {
            while (requestCount > 0)
            {
                // adapter can be null here, however the adapter set in current can't be null, and we never pass
                // a null current.adapter to the application code. Once this file enables nullable, adapter should be
                // adapter! below.
                var request = new IncomingRequest(requestId, this, adapter, stream);

                if (dispatcher is not null)
                {
                    // We don't and can't await the dispatchAsync: with batch requests, we want all the dispatches to
                    // execute in the current Ice thread pool thread. If we awaited the dispatchAsync, we could
                    // switch to a .NET thread pool thread.
                    _ = dispatchAsync(request);
                }
                else
                {
                    // Received request on a connection without an object adapter.
                    sendResponse(
                        request.current.createOutgoingResponse(new ObjectNotExistException()),
                        isTwoWay: !_endpoint.datagram() && requestId != 0,
                        compress: 0);
                }
                --requestCount;
            }

            stream.clear();
        }
        catch (LocalException ex) // TODO: catch all exceptions
        {
            // Typically, the IncomingRequest constructor throws an exception, and we can't continue.
            dispatchException(ex, requestCount);
        }

        async Task dispatchAsync(IncomingRequest request)
        {
            try
            {
                OutgoingResponse response;

                try
                {
                    response = await dispatcher.dispatchAsync(request).ConfigureAwait(false);
                }
                catch (System.Exception ex)
                {
                    response = request.current.createOutgoingResponse(ex);
                }

                sendResponse(response, isTwoWay: !_endpoint.datagram() && requestId != 0, compress);
            }
            catch (LocalException ex) // TODO: catch all exceptions to avoid UnobservedTaskException
            {
                dispatchException(ex, requestCount: 1);
            }
        }
    }

    private void sendResponse(OutgoingResponse response, bool isTwoWay, byte compress)
    {
        bool finished = false;
        try
        {
            lock (_mutex)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if (--_upcallCount == 0)
                    {
                        if (_state == StateFinished)
                        {
                            finished = true;
                            _observer?.detach();
                        }
                        Monitor.PulseAll(_mutex);
                    }

                    if (_state >= StateClosed)
                    {
                        Debug.Assert(_exception is not null);
                        throw _exception;
                    }

                    if (isTwoWay)
                    {
                        sendMessage(new OutgoingMessage(response.outputStream, compress > 0));
                    }

                    if (_state == StateActive && _maxDispatches > 0 && _dispatchCount == _maxDispatches)
                    {
                        // Resume reading if the connection is active and the dispatch count is about to be less than
                        // _maxDispatches.
                        _threadPool.update(this, SocketOperation.None, SocketOperation.Read);
                        _idleTimeoutTransceiver?.enableIdleCheck();
                    }

                    --_dispatchCount;

                    if (_state == StateClosing && _upcallCount == 0)
                    {
                        initiateShutdown();
                    }
                }
                catch (LocalException ex)
                {
                    setState(StateClosed, ex);
                }
            }
        }
        finally
        {
            if (finished && _removeFromFactory is not null)
            {
                _removeFromFactory(this);
            }
        }
    }

    private void dispatchException(LocalException ex, int requestCount)
    {
        bool finished = false;

        // Fatal exception while dispatching a request. Since sendResponse isn't called in case of a fatal exception
        // we decrement _upcallCount here.
        lock (_mutex)
        {
            setState(StateClosed, ex);

            if (requestCount > 0)
            {
                Debug.Assert(_upcallCount >= requestCount);
                _upcallCount -= requestCount;
                if (_upcallCount == 0)
                {
                    if (_state == StateFinished)
                    {
                        finished = true;
                        _observer?.detach();
                    }
                    Monitor.PulseAll(_mutex);
                }
            }
        }

        if (finished && _removeFromFactory is not null)
        {
            _removeFromFactory(this);
        }
    }

    private void inactivityCheck(System.Threading.Timer inactivityTimer)
    {
        lock (_mutex)
        {
            // If the timers are different, it means this inactivityTimer is no longer current.
            if (inactivityTimer == _inactivityTimer)
            {
                _inactivityTimer = null;
                inactivityTimer.Dispose(); // non-blocking

                if (_state == StateActive)
                {
                    setState(
                        StateClosing,
                        new ConnectionClosedException(
                            "Connection closed because it remained inactive for longer than the inactivity timeout.",
                            closedByApplication: false));
                }
            }
            // Else this timer was already canceled and disposed. Nothing to do.
        }
    }

    private void connectTimedOut(System.Threading.Timer connectTimer)
    {
        lock (_mutex)
        {
            if (_state < StateActive)
            {
                setState(StateClosed, new ConnectTimeoutException());
            }
        }
        // else ignore since we're already connected.
        connectTimer.Dispose();
    }

    private void closeTimedOut(System.Threading.Timer closeTimer)
    {
        lock (_mutex)
        {
            if (_state < StateClosed)
            {
                // We don't use setState(state, exception) because we want to overwrite the exception set by a
                // graceful closure.
                _exception = new CloseTimeoutException();
                setState(StateClosed);
            }
        }
        // else ignore since we're already closed.
        closeTimer.Dispose();
    }

    private ConnectionInfo initConnectionInfo()
    {
        // Called with _mutex locked.

        if (_state > StateNotInitialized && _info is not null) // Update the connection info until it's initialized
        {
            return _info;
        }

        _info = _transceiver.getInfo(incoming: _connector is null, _adapter?.getName() ?? "", _endpoint.connectionId());
        return _info;
    }

    private void warning(string msg, System.Exception ex)
    {
        _logger.warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());
    }

    private void observerStartRead(Ice.Internal.Buffer buf)
    {
        if (_readStreamPos >= 0)
        {
            Debug.Assert(!buf.empty());
            _observer.receivedBytes(buf.b.position() - _readStreamPos);
        }
        _readStreamPos = buf.empty() ? -1 : buf.b.position();
    }

    private void observerFinishRead(Ice.Internal.Buffer buf)
    {
        if (_readStreamPos == -1)
        {
            return;
        }
        Debug.Assert(buf.b.position() >= _readStreamPos);
        _observer.receivedBytes(buf.b.position() - _readStreamPos);
        _readStreamPos = -1;
    }

    private void observerStartWrite(Ice.Internal.Buffer buf)
    {
        if (_writeStreamPos >= 0)
        {
            Debug.Assert(!buf.empty());
            _observer.sentBytes(buf.b.position() - _writeStreamPos);
        }
        _writeStreamPos = buf.empty() ? -1 : buf.b.position();
    }

    private void observerFinishWrite(Ice.Internal.Buffer buf)
    {
        if (_writeStreamPos == -1)
        {
            return;
        }
        if (buf.b.position() > _writeStreamPos)
        {
            _observer.sentBytes(buf.b.position() - _writeStreamPos);
        }
        _writeStreamPos = -1;
    }

    private int read(Ice.Internal.Buffer buf)
    {
        int start = buf.b.position();
        int op = _transceiver.read(buf, ref _hasMoreData);
        if (_instance.traceLevels().network >= 3 && buf.b.position() != start)
        {
            StringBuilder s = new StringBuilder("received ");
            if (_endpoint.datagram())
            {
                s.Append(buf.b.limit());
            }
            else
            {
                s.Append(buf.b.position() - start);
                s.Append(" of ");
                s.Append(buf.b.limit() - start);
            }
            s.Append(" bytes via ");
            s.Append(_endpoint.protocol());
            s.Append('\n');
            s.Append(ToString());
            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
        }
        return op;
    }

    private int write(Ice.Internal.Buffer buf)
    {
        int start = buf.b.position();
        int op = _transceiver.write(buf);
        if (_instance.traceLevels().network >= 3 && buf.b.position() != start)
        {
            StringBuilder s = new StringBuilder("sent ");
            s.Append(buf.b.position() - start);
            if (!_endpoint.datagram())
            {
                s.Append(" of ");
                s.Append(buf.b.limit() - start);
            }
            s.Append(" bytes via ");
            s.Append(_endpoint.protocol());
            s.Append('\n');
            s.Append(ToString());
            _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
        }
        return op;
    }

    private void scheduleInactivityTimer()
    {
        // Called with the ConnectionI mutex locked.
        Debug.Assert(_inactivityTimer is null);
        Debug.Assert(_inactivityTimeout > TimeSpan.Zero);

        _inactivityTimer = new System.Threading.Timer(
            inactivityTimer => inactivityCheck((System.Threading.Timer)inactivityTimer));
        _inactivityTimer.Change(_inactivityTimeout, Timeout.InfiniteTimeSpan);
    }

    private void cancelInactivityTimer()
    {
        // Called with the ConnectionI mutex locked.
        if (_inactivityTimer is not null)
        {
            _inactivityTimer.Change(Timeout.InfiniteTimeSpan, Timeout.InfiniteTimeSpan);
            _inactivityTimer.Dispose();
            _inactivityTimer = null;
        }
    }

    private void scheduleCloseTimer()
    {
        if (_closeTimeout > TimeSpan.Zero)
        {
#pragma warning disable CA2000 // closeTimer is disposed by closeTimedOut.
            var closeTimer = new System.Threading.Timer(
                timerObj => closeTimedOut((System.Threading.Timer)timerObj));
            // schedule timer to run once; closeTimedOut disposes the timer too.
            closeTimer.Change(_closeTimeout, Timeout.InfiniteTimeSpan);
#pragma warning restore CA2000
        }
    }

    private void doApplicationClose()
    {
        // Called with the ConnectionI mutex locked.
        Debug.Assert(_state < StateClosing);
        setState(
            StateClosing,
            new ConnectionClosedException(
                "The connection was closed gracefully by the application.",
                closedByApplication: true));
    }

    private class OutgoingMessage
    {
        internal OutgoingMessage(OutputStream stream, bool compress)
        {
            this.stream = stream;
            this.compress = compress;
        }

        internal OutgoingMessage(OutgoingAsyncBase outAsync, OutputStream stream, bool compress, int requestId)
        {
            this.outAsync = outAsync;
            this.stream = stream;
            this.compress = compress;
            this.requestId = requestId;
        }

        internal void canceled()
        {
            Debug.Assert(outAsync is not null); // Only requests can timeout.
            outAsync = null;
        }

        internal bool sent()
        {
            stream = null;
            if (outAsync is not null)
            {
                invokeSent = outAsync.sent();
                return invokeSent || receivedReply;
            }
            return false;
        }

        internal void completed(LocalException ex)
        {
            if (outAsync is not null)
            {
                if (outAsync.exception(ex))
                {
                    outAsync.invokeException();
                }
            }
            stream = null;
        }

        internal OutputStream stream;
        internal OutgoingAsyncBase outAsync;
        internal bool compress;
        internal int requestId;
        internal bool prepared;
        internal bool isSent;
        internal bool invokeSent;
        internal bool receivedReply;
    }

    private static ConnectionState[] connectionStateMap = [
        ConnectionState.ConnectionStateValidating,   // StateNotInitialized
        ConnectionState.ConnectionStateValidating,   // StateNotValidated
        ConnectionState.ConnectionStateActive,       // StateActive
        ConnectionState.ConnectionStateHolding,      // StateHolding
        ConnectionState.ConnectionStateClosing,      // StateClosing
        ConnectionState.ConnectionStateClosing,      // StateClosingPending
        ConnectionState.ConnectionStateClosed,       // StateClosed
        ConnectionState.ConnectionStateClosed,       // StateFinished
    ];

    private Instance _instance;
    private readonly Transceiver _transceiver;
    private readonly IdleTimeoutTransceiverDecorator _idleTimeoutTransceiver; // can be null

    private string _desc;
    private string _type;
    private readonly Connector _connector;
    private EndpointI _endpoint;

    private ObjectAdapter _adapter;

    private Logger _logger;
    private TraceLevels _traceLevels;
    private Ice.Internal.ThreadPool _threadPool;

    private readonly TimeSpan _connectTimeout;
    private readonly TimeSpan _closeTimeout;
    private readonly TimeSpan _inactivityTimeout;

    private System.Threading.Timer _inactivityTimer; // can be null

    private StartCallback _startCallback;

    // This action must be called outside the ConnectionI lock to avoid lock acquisition deadlocks.
    private readonly Action<ConnectionI> _removeFromFactory;

    private bool _warn;
    private bool _warnUdp;

    private int _compressionLevel;

    private int _nextRequestId;

    private Dictionary<int, OutgoingAsyncBase> _asyncRequests = new Dictionary<int, OutgoingAsyncBase>();

    private LocalException _exception;

    private readonly int _messageSizeMax;
    private BatchRequestQueue _batchRequestQueue;

    private LinkedList<OutgoingMessage> _sendStreams = new LinkedList<OutgoingMessage>();

    // Contains the message which is being received. If the connection is waiting to receive a message (_readHeader ==
    // true), its size is Protocol.headerSize. Otherwise, its size is the message size specified in the received message
    // header.
    private InputStream _readStream;

    // When _readHeader is true, the next bytes we'll read are the header of a new message. When false, we're reading
    // next the remainder of a message that was already partially received.
    private bool _readHeader;

    // Contains the message which is being sent. The write stream buffer is empty if no message is being sent.
    private OutputStream _writeStream;

    private ConnectionObserver _observer;
    private int _readStreamPos;
    private int _writeStreamPos;

    // The upcall count keeps track of the number of dispatches, AMI (response) continuations, sent callbacks and
    // connection establishment callbacks that have been started (or are about to be started) by a thread of the thread
    // pool associated with this connection, and have not completed yet. All these operations except the connection
    // establishment callbacks execute application code or code generated from Slice definitions.
    private int _upcallCount;

    // The number of outstanding dispatches. Maintained only while state is StateActive or StateHolding.
    // _dispatchCount can be greater than a non-0 _maxDispatches when a receive a batch with multiples requests.
    private int _dispatchCount;

    // When we dispatch _maxDispatches concurrent requests, we stop reading the connection to back-pressure the peer.
    // _maxDispatches <= 0 means no limit.
    private readonly int _maxDispatches;

    private int _state; // The current state.
    private bool _shutdownInitiated;
    private bool _initialized;
    private bool _validated;

    // When true, the application called close and Connection must close the connection when it receives the reply
    // for the last outstanding invocation.
    private bool _closeRequested;

    private ConnectionInfo _info;

    private CloseCallback _closeCallback;

    // We need to run the continuation asynchronously since it can be completed by an Ice thread pool thread.
    private readonly TaskCompletionSource _closed = new(TaskCreationOptions.RunContinuationsAsynchronously);
    private readonly object _mutex = new();
}
