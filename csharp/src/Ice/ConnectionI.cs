// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Linq;

    using Instrumentation;
    using IceInternal;

    public sealed class ConnectionI : IceInternal.EventHandler, ResponseHandler, CancellationHandler, Connection
    {
        public interface StartCallback
        {
            void connectionStartCompleted(ConnectionI connection);
            void connectionStartFailed(ConnectionI connection, LocalException ex);
        }

        private class TimeoutCallback : TimerTask
        {
            public TimeoutCallback(ConnectionI connection)
            {
                _connection = connection;
            }

            public void runTimerTask()
            {
                _connection.timedOut();
            }

            private Ice.ConnectionI _connection;
        }

        public void start(StartCallback callback)
        {
            try
            {
                lock(this)
                {
                    //
                    // The connection might already be closed if the communicator was destroyed.
                    //
                    if(_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
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

        public void startAndWait()
        {
            try
            {
                lock(this)
                {
                    //
                    // The connection might already be closed if the communicator was destroyed.
                    //
                    if(_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    if(!initialize(SocketOperation.None) || !validate(SocketOperation.None))
                    {
                        //
                        // Wait for the connection to be validated.
                        //
                        while(_state <= StateNotValidated)
                        {
                            System.Threading.Monitor.Wait(this);
                        }

                        if(_state >= StateClosing)
                        {
                            Debug.Assert(_exception != null);
                            throw _exception;
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
                return;
            }
        }

        public void activate()
        {
            lock(this)
            {
                if(_state <= StateNotValidated)
                {
                    return;
                }

                if(_acmLastActivity > -1)
                {
                    _acmLastActivity = Time.currentMonotonicTimeMillis();
                }
                setState(StateActive);
            }
        }

        public void hold()
        {
            lock(this)
            {
                if(_state <= StateNotValidated)
                {
                    return;
                }

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
                    //
                    // If we do a graceful shutdown, then we wait until all
                    // outstanding requests have been completed. Otherwise,
                    // the CloseConnectionException will cause all outstanding
                    // requests to be retried, regardless of whether the
                    // server has processed them or not.
                    //
                    while(_asyncRequests.Count != 0)
                    {
                        System.Threading.Monitor.Wait(this);
                    }

                    setState(StateClosing, new CloseConnectionException());
                }
            }
        }

        public bool isActiveOrHolding()
        {
            lock(this)
            {
                return _state > StateNotValidated && _state < StateClosing;
            }
        }

        public bool isFinished()
        {
            //
            // We can use TryLock here, because as long as there are still
            // threads operating in this connection object, connection
            // destruction is considered as not yet finished.
            //
            if(!System.Threading.Monitor.TryEnter(this))
            {
                return false;
            }

            try
            {
                if(_state != StateFinished || _dispatchCount != 0)
                {
                    return false;
                }

                Debug.Assert(_state == StateFinished);
                return true;
            }
            finally
            {
                System.Threading.Monitor.Exit(this);
            }
        }

        public void throwException()
        {
            lock(this)
            {
                if(_exception != null)
                {
                    Debug.Assert(_state >= StateClosing);
                    throw _exception;
                }
            }
        }

        public void waitUntilHolding()
        {
            lock(this)
            {
                while(_state < StateHolding || _dispatchCount > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        public void waitUntilFinished()
        {
            lock(this)
            {
                //
                // We wait indefinitely until the connection is finished and all
                // outstanding requests are completed. Otherwise we couldn't
                // guarantee that there are no outstanding calls when deactivate()
                // is called on the servant locators.
                //
                while(_state < StateFinished || _dispatchCount > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }

                Debug.Assert(_state == StateFinished);

                //
                // Clear the OA. See bug 1673 for the details of why this is necessary.
                //
                _adapter = null;
            }
        }

        public void updateObserver()
        {
            lock(this)
            {
                if(_state < StateNotValidated || _state > StateClosed)
                {
                    return;
                }

                Debug.Assert(_instance.initializationData().observer != null);
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
        }

        public void monitor(long now, ACMConfig acm)
        {
            lock(this)
            {
                if(_state != StateActive)
                {
                    return;
                }

                //
                // We send a heartbeat if there was no activity in the last
                // (timeout / 4) period. Sending a heartbeat sooner than
                // really needed is safer to ensure that the receiver will
                // receive in time the heartbeat. Sending the heartbeat if
                // there was no activity in the last (timeout / 2) period
                // isn't enough since monitor() is called only every (timeout
                // / 2) period.
                //
                // Note that this doesn't imply that we are sending 4 heartbeats
                // per timeout period because the monitor() method is sill only
                // called every (timeout / 2) period.
                //
                if(acm.heartbeat == ACMHeartbeat.HeartbeatAlways ||
                   (acm.heartbeat != ACMHeartbeat.HeartbeatOff && _writeStream.isEmpty() &&
                    now >= (_acmLastActivity + acm.timeout / 4)))
                {
                    if(acm.heartbeat != ACMHeartbeat.HeartbeatOnInvocation || _dispatchCount > 0)
                    {
                        heartbeat();
                    }
                }

                if(_readStream.size() > Protocol.headerSize || !_writeStream.isEmpty())
                {
                    //
                    // If writing or reading, nothing to do, the connection
                    // timeout will kick-in if writes or reads don't progress.
                    // This check is necessary because the actitivy timer is
                    // only set when a message is fully read/written.
                    //
                    return;
                }

                if(acm.close != ACMClose.CloseOff && now >= (_acmLastActivity + acm.timeout))
                {
                    if(acm.close == ACMClose.CloseOnIdleForceful ||
                       (acm.close != ACMClose.CloseOnIdle && (_asyncRequests.Count > 0)))
                    {
                        //
                        // Close the connection if we didn't receive a heartbeat in
                        // the last period.
                        //
                        setState(StateClosed, new ConnectionTimeoutException());
                    }
                    else if(acm.close != ACMClose.CloseOnInvocation &&
                            _dispatchCount == 0 && _batchRequestQueue.isEmpty()  &&
                            _asyncRequests.Count == 0)
                    {
                        //
                        // The connection is idle, close it.
                        //
                        setState(StateClosing, new ConnectionTimeoutException());
                    }
                }
            }
        }

        public int sendAsyncRequest(OutgoingAsyncBase og, bool compress, bool response,
                                    int batchRequestNum)
        {
            OutputStream os = og.getOs();

            lock(this)
            {
                //
                // If the exception is closed before we even have a chance
                // to send our request, we always try to send the request
                // again.
                //
                if(_exception != null)
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

                og.attachRemoteObserver(initConnectionInfo(), _endpoint, requestId);

                int status = OutgoingAsyncBase.AsyncStatusQueued;
                try
                {
                    OutgoingMessage message = new OutgoingMessage(og, os, compress, requestId);
                    status = sendMessage(message);
                }
                catch(Ice.LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                if(response)
                {
                    //
                    // Add to the async requests map.
                    //
                    _asyncRequests[requestId] = og;
                }
                return status;
            }
        }

        public BatchRequestQueue getBatchRequestQueue()
        {
            return _batchRequestQueue;
        }

        public void flushBatchRequests()
        {
            flushBatchRequestsAsync().Wait();
        }

        private class ConnectionFlushBatchCompletionCallback : AsyncResultCompletionCallback
        {
            public ConnectionFlushBatchCompletionCallback(Ice.Connection connection,
                                                          Ice.Communicator communicator,
                                                          Instance instance,
                                                          string op,
                                                          object cookie,
                                                          Ice.AsyncCallback callback)
                : base(communicator, instance, op, cookie, callback)
            {
                _connection = connection;
            }

            public override Ice.Connection getConnection()
            {
                return _connection;
            }

            protected override Ice.AsyncCallback getCompletedCallback()
            {
                return (Ice.AsyncResult result) =>
                {
                    try
                    {
                        result.throwLocalException();
                    }
                    catch(Ice.Exception ex)
                    {
                        if(exceptionCallback_ != null)
                        {
                            exceptionCallback_.Invoke(ex);
                        }
                    }
                };
            }

            private Ice.Connection _connection;
        }

        public Task flushBatchRequestsAsync(IProgress<bool> progress = null,
                                            CancellationToken cancel = new CancellationToken())
        {
            var completed = new FlushBatchTaskCompletionCallback(progress, cancel);
            var outgoing = new ConnectionFlushBatchAsync(this, _instance, completed);
            outgoing.invoke(_flushBatchRequests_name);
            return completed.Task;
        }

        public AsyncResult begin_flushBatchRequests(AsyncCallback cb = null, object cookie = null)
        {
            var result = new ConnectionFlushBatchCompletionCallback(this, _communicator, _instance,
                                                                    _flushBatchRequests_name, cookie, cb);
            var outgoing = new ConnectionFlushBatchAsync(this, _instance, result);
            outgoing.invoke(_flushBatchRequests_name);
            return result;
        }

        public void end_flushBatchRequests(AsyncResult r)
        {
            if(r != null && r.getConnection() != this)
            {
                const string msg = "Connection for call to end_" + _flushBatchRequests_name +
                                   " does not match connection that was used to call corresponding begin_" +
                                   _flushBatchRequests_name + " method";
                throw new ArgumentException(msg);
            }
            AsyncResultI.check(r, _flushBatchRequests_name).wait();
        }

        private const string _flushBatchRequests_name = "flushBatchRequests";

        public void setCloseCallback(CloseCallback callback)
        {
            lock(this)
            {
                if(_state >= StateClosed)
                {
                    if(callback != null)
                    {
                        _threadPool.dispatch(() =>
                        {
                            try
                            {
                                callback(this);
                            }
                            catch(System.Exception ex)
                            {
                                _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                            }
                        } , this);
                    }
                }
                else
                {
                    _closeCallback = callback;
                }
            }
        }

        public void setHeartbeatCallback(HeartbeatCallback callback)
        {
            lock(this)
            {
                _heartbeatCallback = callback;
            }
        }

        public void setACM(Optional<int> timeout, Optional<ACMClose> close, Optional<ACMHeartbeat> heartbeat)
        {
            lock(this)
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
                    _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
                }

                if(_state == StateActive)
                {
                    _monitor.add(this);
                }
            }
        }

        public ACM getACM()
        {
            lock(this)
            {
                return _monitor != null ? _monitor.getACM() : new ACM(0, ACMClose.CloseOff, ACMHeartbeat.HeartbeatOff);
            }
        }

        public void asyncRequestCanceled(IceInternal.OutgoingAsyncBase outAsync, Ice.LocalException ex)
        {
            //
            // NOTE: This isn't called from a thread pool thread.
            //

            lock(this)
            {
                if(_state >= StateClosed)
                {
                    return; // The request has already been or will be shortly notified of the failure.
                }


                OutgoingMessage o = _sendStreams.FirstOrDefault(m => m.outAsync == outAsync);
                if(o != null)
                {
                    if(o.requestId > 0)
                    {
                        _asyncRequests.Remove(o.requestId);
                    }

                    if(ex is Ice.ConnectionTimeoutException)
                    {
                        setState(StateClosed, ex);
                    }
                    else
                    {
                        //
                        // If the request is being sent, don't remove it from the send streams,
                        // it will be removed once the sending is finished.
                        //
                        if(o == _sendStreams.First.Value)
                        {
                            o.canceled();
                        }
                        else
                        {
                            o.canceled();
                            _sendStreams.Remove(o);
                        }
                        if(outAsync.exception(ex))
                        {
                            outAsync.invokeExceptionAsync();
                        }
                    }
                    return;
                }

                if(outAsync is IceInternal.OutgoingAsync)
                {
                    foreach(KeyValuePair<int, IceInternal.OutgoingAsyncBase> kvp in _asyncRequests)
                    {
                        if(kvp.Value == outAsync)
                        {
                            if(ex is Ice.ConnectionTimeoutException)
                            {
                                setState(StateClosed, ex);
                            }
                            else
                            {
                                _asyncRequests.Remove(kvp.Key);
                                if(outAsync.exception(ex))
                                {
                                    outAsync.invokeExceptionAsync();
                                }
                            }
                            return;
                        }
                    }
                }
            }
        }

        public void sendResponse(int requestId, OutputStream os, byte compressFlag, bool amd)
        {
            lock(this)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if(--_dispatchCount == 0)
                    {
                        if(_state == StateFinished)
                        {
                            reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }

                    if(_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    sendMessage(new OutgoingMessage(os, compressFlag > 0, true));

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

        public void sendNoResponse()
        {
            lock(this)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if(--_dispatchCount == 0)
                    {
                        if(_state == StateFinished)
                        {
                            reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }

                    if(_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
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

        public bool systemException(int requestId, Ice.SystemException ex, bool amd)
        {
            return false; // System exceptions aren't marshalled.
        }

        public void invokeException(int requestId, LocalException ex, int invokeNum, bool amd)
        {
            //
            // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
            // called in case of a fatal exception we decrement _dispatchCount here.
            //

            lock(this)
            {
                setState(StateClosed, ex);

                if(invokeNum > 0)
                {
                    Debug.Assert(_dispatchCount >= invokeNum);
                    _dispatchCount -= invokeNum;
                    if(_dispatchCount == 0)
                    {
                        if(_state == StateFinished)
                        {
                            reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }
                }
            }
        }

        public IceInternal.EndpointI endpoint()
        {
            return _endpoint; // No mutex protection necessary, _endpoint is immutable.
        }

        public IceInternal.Connector connector()
        {
            return _connector; // No mutex protection necessary, _endpoint is immutable.
        }

        public void setAdapter(ObjectAdapter adapter)
        {
            lock(this)
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
                // We never change the thread pool with which we were initially
                // registered, even if we add or remove an object adapter.
                //
            }
        }

        public ObjectAdapter getAdapter()
        {
            lock(this)
            {
                return _adapter;
            }
        }

        public Endpoint getEndpoint()
        {
            return _endpoint; // No mutex protection necessary, _endpoint is immutable.
        }

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
        public override bool startAsync(int operation, IceInternal.AsyncCallback cb, ref bool completedSynchronously)
        {
            if(_state >= StateClosed)
            {
                return false;
            }

            try
            {
                if((operation & IceInternal.SocketOperation.Write) != 0)
                {
                    if(_observer != null)
                    {
                        observerStartWrite(_writeStream.getBuffer());
                    }

                    bool completed;
                    completedSynchronously = _transceiver.startWrite(_writeStream.getBuffer(), cb, this, out completed);
                    if(completed && _sendStreams.Count > 0)
                    {
                        // The whole message is written, assume it's sent now for at-most-once semantics.
                        _sendStreams.First.Value.isSent = true;
                    }
                }
                else if((operation & IceInternal.SocketOperation.Read) != 0)
                {
                    if(_observer != null && !_readHeader)
                    {
                        observerStartRead(_readStream.getBuffer());
                    }

                    completedSynchronously = _transceiver.startRead(_readStream.getBuffer(), cb, this);
                }
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
                return false;
            }
            return true;
        }

        public override bool finishAsync(int operation)
        {
            try
            {
                if((operation & IceInternal.SocketOperation.Write) != 0)
                {
                    IceInternal.Buffer buf = _writeStream.getBuffer();
                    int start = buf.b.position();
                    _transceiver.finishWrite(buf);
                    if(_instance.traceLevels().network >= 3 && buf.b.position() != start)
                    {
                        StringBuilder s = new StringBuilder("sent ");
                        s.Append(buf.b.position() - start);
                        if(!_endpoint.datagram())
                        {
                            s.Append(" of ");
                            s.Append(buf.b.limit() - start);
                        }
                        s.Append(" bytes via ");
                        s.Append(_endpoint.protocol());
                        s.Append("\n");
                        s.Append(ToString());
                        _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                    }

                    if(_observer != null)
                    {
                        observerFinishWrite(_writeStream.getBuffer());
                    }
                }
                else if((operation & IceInternal.SocketOperation.Read) != 0)
                {
                    IceInternal.Buffer buf = _readStream.getBuffer();
                    int start = buf.b.position();
                    _transceiver.finishRead(buf);
                    if(_instance.traceLevels().network >= 3 && buf.b.position() != start)
                    {
                        StringBuilder s = new StringBuilder("received ");
                        if(_endpoint.datagram())
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
                        s.Append("\n");
                        s.Append(ToString());
                        _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                    }

                    if(_observer != null && !_readHeader)
                    {
                        observerFinishRead(_readStream.getBuffer());
                    }
                }
            }
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
            }
            return _state < StateClosed;
        }

        public override void message(ref IceInternal.ThreadPoolCurrent current)
        {
            StartCallback startCB = null;
            Queue<OutgoingMessage> sentCBs = null;
            MessageInfo info = new MessageInfo();
            int dispatchCount = 0;

            IceInternal.ThreadPoolMessage msg = new IceInternal.ThreadPoolMessage(this);
            try
            {
                lock(this)
                {
                    if(!msg.startIOScope(ref current))
                    {
                        return;
                    }

                    if(_state >= StateClosed)
                    {
                        return;
                    }

                    int readyOp = current.operation;
                    try
                    {
                        unscheduleTimeout(current.operation);

                        int writeOp = IceInternal.SocketOperation.None;
                        int readOp = IceInternal.SocketOperation.None;
                        if((readyOp & IceInternal.SocketOperation.Write) != 0)
                        {
                            if(_observer != null)
                            {
                                observerStartWrite(_writeStream.getBuffer());
                            }
                            writeOp = write(_writeStream.getBuffer());
                            if(_observer != null && (writeOp & IceInternal.SocketOperation.Write) == 0)
                            {
                                observerFinishWrite(_writeStream.getBuffer());
                            }
                        }

                        while((readyOp & IceInternal.SocketOperation.Read) != 0)
                        {
                            IceInternal.Buffer buf = _readStream.getBuffer();

                            if(_observer != null && !_readHeader)
                            {
                                observerStartRead(buf);
                            }

                            readOp = read(buf);
                            if((readOp & IceInternal.SocketOperation.Read) != 0)
                            {
                                break;
                            }
                            if(_observer != null && !_readHeader)
                            {
                                Debug.Assert(!buf.b.hasRemaining());
                                observerFinishRead(buf);
                            }

                            if(_readHeader) // Read header if necessary.
                            {
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

                                ProtocolVersion pv  = new ProtocolVersion();
                                pv.ice_readMembers(_readStream);
                                IceInternal.Protocol.checkSupportedProtocol(pv);
                                EncodingVersion ev = new EncodingVersion();
                                ev.ice_readMembers(_readStream);
                                IceInternal.Protocol.checkSupportedProtocolEncoding(ev);

                                _readStream.readByte(); // messageType
                                _readStream.readByte(); // compress
                                int size = _readStream.readInt();
                                if(size < IceInternal.Protocol.headerSize)
                                {
                                    throw new Ice.IllegalMessageSizeException();
                                }
                                if(size > _messageSizeMax)
                                {
                                    IceInternal.Ex.throwMemoryLimitException(size, _messageSizeMax);
                                }
                                if(size > _readStream.size())
                                {
                                    _readStream.resize(size);
                                }
                                _readStream.pos(pos);
                            }

                            if(buf.b.hasRemaining())
                            {
                                if(_endpoint.datagram())
                                {
                                    throw new Ice.DatagramLimitException(); // The message was truncated.
                                }
                                continue;
                            }
                            break;
                        }

                        int newOp = readOp | writeOp;
                        readyOp &= ~newOp;
                        Debug.Assert(readyOp != 0 || newOp != 0);

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
                            Debug.Assert(_state <= StateClosingPending);

                            //
                            // We parse messages first, if we receive a close
                            // connection message we won't send more messages.
                            //
                            if((readyOp & IceInternal.SocketOperation.Read) != 0)
                            {
                                newOp |= parseMessage(ref info);
                                dispatchCount += info.messageDispatchCount;
                            }

                            if((readyOp & IceInternal.SocketOperation.Write) != 0)
                            {
                                newOp |= sendNextMessage(out sentCBs);
                                if(sentCBs != null)
                                {
                                    ++dispatchCount;
                                }
                            }

                            if(_state < StateClosed)
                            {
                                scheduleTimeout(newOp);
                                _threadPool.update(this, current.operation, newOp);
                            }
                        }

                        if(_acmLastActivity > -1)
                        {
                            _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
                        }

                        if(dispatchCount == 0)
                        {
                            return; // Nothing to dispatch we're done!
                        }

                        _dispatchCount += dispatchCount;

                        msg.completed(ref current);
                    }
                    catch(DatagramLimitException) // Expected.
                    {
                        if(_warnUdp)
                        {
                            _logger.warning("maximum datagram size of " + _readStream.pos() + " exceeded");
                        }
                        _readStream.resize(IceInternal.Protocol.headerSize);
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
                            _readStream.resize(IceInternal.Protocol.headerSize);
                            _readStream.pos(0);
                            _readHeader = true;
                        }
                        else
                        {
                            setState(StateClosed, ex);
                        }
                        return;
                    }

                    IceInternal.ThreadPoolCurrent c = current;
                    _threadPool.dispatch(() =>
                    {
                        dispatch(startCB, sentCBs, info);
                        msg.destroy(ref c);
                    }, this);
                }
            }
            finally
            {
                msg.finishIOScope(ref current);
            }

        }

        private void dispatch(StartCallback startCB, Queue<OutgoingMessage> sentCBs, MessageInfo info)
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
                foreach(OutgoingMessage m in sentCBs)
                {
                    if(m.invokeSent)
                    {
                        m.outAsync.invokeSent();
                    }
                    if(m.receivedReply)
                    {
                        IceInternal.OutgoingAsync outAsync = (IceInternal.OutgoingAsync)m.outAsync;
                        if(outAsync.response())
                        {
                            outAsync.invokeResponse();
                        }
                    }
                }
                ++dispatchedCount;
            }

            //
            // Asynchronous replies must be handled outside the thread
            // synchronization, so that nested calls are possible.
            //
            if(info.outAsync != null)
            {
                info.outAsync.invokeResponse();
                ++dispatchedCount;
            }

            if(info.heartbeatCallback != null)
            {
                try
                {
                    info.heartbeatCallback(this);
                }
                catch(System.Exception ex)
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

            //
            // Decrease dispatch count.
            //
            if(dispatchedCount > 0)
            {
                lock(this)
                {
                    _dispatchCount -= dispatchedCount;
                    if(_dispatchCount == 0)
                    {
                        //
                        // Only initiate shutdown if not already done. It
                        // might have already been done if the sent callback
                        // or AMI callback was dispatched when the connection
                        // was already in the closing state.
                        //
                        if(_state == StateClosing)
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
                            reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }
                }
            }
        }

        public override void finished(ref IceInternal.ThreadPoolCurrent current)
        {
            lock(this)
            {
                Debug.Assert(_state == StateClosed);
                unscheduleTimeout(IceInternal.SocketOperation.Read | IceInternal.SocketOperation.Write);
            }

            //
            // If there are no callbacks to call, we don't call ioCompleted() since we're not going
            // to call code that will potentially block (this avoids promoting a new leader and
            // unecessary thread creation, especially if this is called on shutdown).
            //
            if(_startCallback == null && _sendStreams.Count == 0 && _asyncRequests.Count == 0 &&
               _closeCallback == null && _heartbeatCallback == null)
            {
                finish();
                return;
            }

            //
            // Unlike C++/Java, this method is called from an IO thread of the .NET thread
            // pool of from the communicator async IO thread. While it's fine to handle the
            // non-blocking activity of the connection from these threads, the dispatching
            // of the message must be taken care of by the Ice thread pool.
            //
            _threadPool.dispatch(finish, this);
        }

        private void finish()
        {
            if(!_initialized)
            {
                if(_instance.traceLevels().network >= 2)
                {
                    StringBuilder s = new StringBuilder("failed to ");
                    s.Append(_connector != null ? "establish" : "accept");
                    s.Append(" ");
                    s.Append(_endpoint.protocol());
                    s.Append(" connection\n");
                    s.Append(ToString());
                    s.Append("\n");
                    s.Append(_exception);
                    _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                }
            }
            else
            {
                if(_instance.traceLevels().network >= 1)
                {
                    StringBuilder s = new StringBuilder("closed ");
                    s.Append(_endpoint.protocol());
                    s.Append(" connection\n");
                    s.Append(ToString());

                    //
                    // Trace the cause of unexpected connection closures
                    //
                    if(!(_exception is CloseConnectionException ||
                         _exception is ForcedCloseConnectionException ||
                         _exception is ConnectionTimeoutException ||
                         _exception is CommunicatorDestroyedException ||
                         _exception is ObjectAdapterDeactivatedException))
                    {
                        s.Append("\n");
                        s.Append(_exception);
                    }

                    _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
                }
            }

            if(_startCallback != null)
            {
                _startCallback.connectionStartFailed(this, _exception);
                _startCallback = null;
            }

            if(_sendStreams.Count > 0)
            {
                if(!_writeStream.isEmpty())
                {
                    //
                    // Return the stream to the outgoing call. This is important for
                    // retriable AMI calls which are not marshalled again.
                    //
                    OutgoingMessage message = _sendStreams.First.Value;
                    _writeStream.swap(message.stream);

                    //
                    // The current message might be sent but not yet removed from _sendStreams. If
                    // the response has been received in the meantime, we remove the message from
                    // _sendStreams to not call finished on a message which is already done.
                    //
                    if(message.isSent || message.receivedReply)
                    {
                        if(message.sent() && message.invokeSent)
                        {
                            message.outAsync.invokeSent();
                        }
                        if(message.receivedReply)
                        {
                            IceInternal.OutgoingAsync outAsync = (IceInternal.OutgoingAsync)message.outAsync;
                            if(outAsync.response())
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
                    if(o.requestId > 0) // Make sure finished isn't called twice.
                    {
                        _asyncRequests.Remove(o.requestId);
                    }
                }
                _sendStreams.Clear(); // Must be cleared before _requests because of Outgoing* references in OutgoingMessage
            }

            foreach(IceInternal.OutgoingAsyncBase o in _asyncRequests.Values)
            {
                if(o.exception(_exception))
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
            _incomingCache = null;

            if(_closeCallback != null)
            {
                try
                {
                    _closeCallback(this);
                }
                catch(System.Exception ex)
                {
                    _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                }
                _closeCallback = null;
            }

            _heartbeatCallback = null;

            //
            // This must be done last as this will cause waitUntilFinished() to return (and communicator
            // objects such as the timer might be destroyed too).
            //
            lock(this)
            {
                setState(StateFinished);

                if(_dispatchCount == 0)
                {
                    reap();
                }
            }
        }

        public override string ToString()
        {
            return _desc; // No mutex lock, _desc is immutable.
        }

        public void timedOut()
        {
            lock(this)
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
        }

        public string type()
        {
            return _type; // No mutex lock, _type is immutable.
        }

        public int timeout()
        {
            return _endpoint.timeout(); // No mutex protection necessary, _endpoint is immutable.
        }

        public ConnectionInfo getInfo()
        {
            lock(this)
            {
                if(_state >= StateClosed)
                {
                    throw _exception;
                }
                return initConnectionInfo();
            }
        }

        public void setBufferSize(int rcvSize, int sndSize)
        {
            lock(this)
            {
                if(_state >= StateClosed)
                {
                    throw _exception;
                }
                _transceiver.setBufferSize(rcvSize, sndSize);
                _info = null; // Invalidate the cached connection info
            }
        }

        public string ice_toString_()
        {
            return ToString();
        }

        public void exception(LocalException ex)
        {
            lock(this)
            {
                setState(StateClosed, ex);
            }
        }

        static ConnectionI()
        {
            _compressionSupported = IceInternal.BZip2.supported();
        }

        internal ConnectionI(Communicator communicator, IceInternal.Instance instance,
                             IceInternal.ACMMonitor monitor, IceInternal.Transceiver transceiver,
                             IceInternal.Connector connector, IceInternal.EndpointI endpoint, ObjectAdapterI adapter)
        {
            _communicator = communicator;
            _instance = instance;
            _monitor = monitor;
            _transceiver = transceiver;
            _desc = transceiver.ToString();
            _type = transceiver.protocol();
            _connector = connector;
            _endpoint = endpoint;
            _adapter = adapter;
            InitializationData initData = instance.initializationData();
            _logger = initData.logger; // Cached for better performance.
            _traceLevels = instance.traceLevels(); // Cached for better performance.
            _timer = instance.timer();
            _writeTimeout = new TimeoutCallback(this);
            _writeTimeoutScheduled = false;
            _readTimeout = new TimeoutCallback(this);
            _readTimeoutScheduled = false;
            _warn = initData.properties.getPropertyAsInt("Ice.Warn.Connections") > 0;
            _warnUdp = initData.properties.getPropertyAsInt("Ice.Warn.Datagrams") > 0;
            _cacheBuffers = instance.cacheMessageBuffers() > 0;
            if(_monitor != null && _monitor.getACM().timeout > 0)
            {
                _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
            }
            else
            {
                _acmLastActivity = -1;
            }
            _nextRequestId = 1;
            _messageSizeMax = adapter != null ? adapter.messageSizeMax() : instance.messageSizeMax();
            _batchRequestQueue = new IceInternal.BatchRequestQueue(instance, _endpoint.datagram());
            _readStream = new InputStream(instance, Util.currentProtocolEncoding);
            _readHeader = false;
            _readStreamPos = -1;
            _writeStream = new OutputStream(instance, Util.currentProtocolEncoding);
            _writeStreamPos = -1;
            _dispatchCount = 0;
            _state = StateNotInitialized;

            _compressionLevel = initData.properties.getPropertyAsIntWithDefault("Ice.Compression.Level", 1);
            if(_compressionLevel < 1)
            {
                _compressionLevel = 1;
            }
            else if(_compressionLevel > 9)
            {
                _compressionLevel = 9;
            }

            if(adapter != null)
            {
                _servantManager = adapter.getServantManager();
            }

            try
            {
                if(adapter != null)
                {
                    _threadPool = adapter.getThreadPool();
                }
                else
                {
                    _threadPool = instance.clientThreadPool();
                }
                _threadPool.initialize(this);
            }
            catch(LocalException)
            {
                throw;
            }
            catch(System.Exception ex)
            {
                throw new SyscallException(ex);
            }
        }

        private const int StateNotInitialized = 0;
        private const int StateNotValidated = 1;
        private const int StateActive = 2;
        private const int StateHolding = 3;
        private const int StateClosing = 4;
        private const int StateClosingPending = 5;
        private const int StateClosed = 6;
        private const int StateFinished = 7;

        private void setState(int state, LocalException ex)
        {
            //
            // If setState() is called with an exception, then only closed
            // and closing states are permissible.
            //
            Debug.Assert(state >= StateClosing);

            if(_state == state) // Don't switch twice.
            {
                return;
            }

            if(_exception == null)
            {
                //
                // If we are in closed state, an exception must be set.
                //
                Debug.Assert(_state != StateClosed);

                _exception = ex;

                //
                // We don't warn if we are not validated.
                //
                if(_warn && _validated)
                {
                    //
                    // Don't warn about certain expected exceptions.
                    //
                    if(!(_exception is CloseConnectionException ||
                         _exception is ForcedCloseConnectionException ||
                         _exception is ConnectionTimeoutException ||
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
                    Debug.Assert(false);
                    break;
                }

                case StateNotValidated:
                {
                    if(_state != StateNotInitialized)
                    {
                        Debug.Assert(_state == StateClosed);
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
                    _threadPool.finish(this);
                    _transceiver.close();
                    break;
                }

                case StateFinished:
                {
                    Debug.Assert(_state == StateClosed);
                    _transceiver.destroy();
                    _communicator = null;
                    break;
                }
                }
            }
            catch(Ice.LocalException ex)
            {
                _logger.error("unexpected connection exception:\n" + ex + "\n" + _transceiver.ToString());
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
                    if(_acmLastActivity > -1)
                    {
                        _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
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
                    if(!(_exception is CloseConnectionException ||
                         _exception is ForcedCloseConnectionException ||
                         _exception is ConnectionTimeoutException ||
                         _exception is CommunicatorDestroyedException ||
                         _exception is ObjectAdapterDeactivatedException ||
                         (_exception is ConnectionLostException && _state >= StateClosing)))
                    {
                        _observer.failed(_exception.ice_id());
                    }
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
                OutputStream os = new OutputStream(_instance, Util.currentProtocolEncoding);
                os.writeBlob(IceInternal.Protocol.magic);
                Ice.Util.currentProtocol.ice_writeMembers(os);
                Ice.Util.currentProtocolEncoding.ice_writeMembers(os);
                os.writeByte(IceInternal.Protocol.closeConnectionMsg);
                os.writeByte(_compressionSupported ? (byte)1 : (byte)0);
                os.writeInt(IceInternal.Protocol.headerSize); // Message size.

                if((sendMessage(new OutgoingMessage(os, false, false)) &
                    IceInternal.OutgoingAsyncBase.AsyncStatusSent) != 0)
                {
                    setState(StateClosingPending);

                    //
                    // Notify the the transceiver of the graceful connection closure.
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

        private void heartbeat()
        {
            Debug.Assert(_state == StateActive);

            if(!_endpoint.datagram())
            {
                OutputStream os = new OutputStream(_instance, Util.currentProtocolEncoding);
                os.writeBlob(IceInternal.Protocol.magic);
                Ice.Util.currentProtocol.ice_writeMembers(os);
                Ice.Util.currentProtocolEncoding.ice_writeMembers(os);
                os.writeByte(IceInternal.Protocol.validateConnectionMsg);
                os.writeByte((byte)0);
                os.writeInt(IceInternal.Protocol.headerSize); // Message size.
                try
                {
                    sendMessage(new OutgoingMessage(os, false, false));
                }
                catch(Ice.LocalException ex)
                {
                    setState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                }
            }
        }

        private bool initialize(int operation)
        {
            int s = _transceiver.initialize(_readStream.getBuffer(), _writeStream.getBuffer(), ref _hasMoreData);
            if(s != IceInternal.SocketOperation.None)
            {
                scheduleTimeout(s);
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
            if(!_endpoint.datagram()) // Datagram connections are always implicitly validated.
            {
                if(_adapter != null) // The server side has the active role for connection validation.
                {
                    if(_writeStream.size() == 0)
                    {
                        _writeStream.writeBlob(IceInternal.Protocol.magic);
                        Ice.Util.currentProtocol.ice_writeMembers(_writeStream);
                        Ice.Util.currentProtocolEncoding.ice_writeMembers(_writeStream);
                        _writeStream.writeByte(IceInternal.Protocol.validateConnectionMsg);
                        _writeStream.writeByte((byte)0); // Compression status (always zero for validate connection).
                        _writeStream.writeInt(IceInternal.Protocol.headerSize); // Message size.
                        IceInternal.TraceUtil.traceSend(_writeStream, _logger, _traceLevels);
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
                else // The client side has the passive role for connection validation.
                {
                    if(_readStream.size() == 0)
                    {
                        _readStream.resize(IceInternal.Protocol.headerSize);
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

                    Debug.Assert(_readStream.pos() == IceInternal.Protocol.headerSize);
                    _readStream.pos(0);
                    byte[] m = _readStream.readBlob(4);
                    if(m[0] != IceInternal.Protocol.magic[0] || m[1] != IceInternal.Protocol.magic[1] ||
                       m[2] != IceInternal.Protocol.magic[2] || m[3] != IceInternal.Protocol.magic[3])
                    {
                        BadMagicException ex = new BadMagicException();
                        ex.badMagic = m;
                        throw ex;
                    }

                    ProtocolVersion pv  = new ProtocolVersion();
                    pv.ice_readMembers(_readStream);
                    IceInternal.Protocol.checkSupportedProtocol(pv);

                    EncodingVersion ev = new EncodingVersion();
                    ev.ice_readMembers(_readStream);
                    IceInternal.Protocol.checkSupportedProtocolEncoding(ev);

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

            _writeStream.resize(0);
            _writeStream.pos(0);

            _readStream.resize(IceInternal.Protocol.headerSize);
            _readStream.pos(0);
            _readHeader = true;

            if(_instance.traceLevels().network >= 1)
            {
                StringBuilder s = new StringBuilder();
                if(_endpoint.datagram())
                {
                    s.Append("starting to ");
                    s.Append(_connector != null ? "send" : "receive");
                    s.Append(" ");
                    s.Append(_endpoint.protocol());
                    s.Append(" messages\n");
                    s.Append(_transceiver.toDetailedString());
                }
                else
                {
                    s.Append(_connector != null ? "established" : "accepted");
                    s.Append(" ");
                    s.Append(_endpoint.protocol());
                    s.Append(" connection\n");
                    s.Append(ToString());
                }
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
            }

            return true;
        }

        private int sendNextMessage(out Queue<OutgoingMessage> callbacks)
        {
            callbacks = null;

            if(_sendStreams.Count == 0)
            {
                return IceInternal.SocketOperation.None;
            }
            else if(_state == StateClosingPending && _writeStream.pos() == 0)
            {
                // Message wasn't sent, empty the _writeStream, we're not going to send more data.
                OutgoingMessage message = _sendStreams.First.Value;
                _writeStream.swap(message.stream);
                return IceInternal.SocketOperation.None;
            }

            Debug.Assert(!_writeStream.isEmpty() && _writeStream.pos() == _writeStream.size());
            try
            {
                while(true)
                {
                    //
                    // Notify the message that it was sent.
                    //
                    OutgoingMessage message = _sendStreams.First.Value;
                    _writeStream.swap(message.stream);
                    if(message.sent())
                    {
                        if(callbacks == null)
                        {
                            callbacks = new Queue<OutgoingMessage>();
                        }
                        callbacks.Enqueue(message);
                    }
                    _sendStreams.RemoveFirst();

                    //
                    // If there's nothing left to send, we're done.
                    //
                    if(_sendStreams.Count == 0)
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
                        return IceInternal.SocketOperation.None;
                    }

                    //
                    // Otherwise, prepare the next message stream for writing.
                    //
                    message = _sendStreams.First.Value;
                    Debug.Assert(!message.prepared);
                    OutputStream stream = message.stream;

                    message.stream = doCompress(message.stream, message.compress);
                    message.stream.prepareWrite();
                    message.prepared = true;

                    IceInternal.TraceUtil.traceSend(stream, _logger, _traceLevels);
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
            catch(Ice.LocalException ex)
            {
                setState(StateClosed, ex);
            }
            return IceInternal.SocketOperation.None;
        }

        private int sendMessage(OutgoingMessage message)
        {
            Debug.Assert(_state < StateClosed);

            if(_sendStreams.Count > 0)
            {
                message.adopt();
                _sendStreams.AddLast(message);
                return IceInternal.OutgoingAsyncBase.AsyncStatusQueued;
            }

            //
            // Attempt to send the message without blocking. If the send blocks, we use
            // asynchronous I/O or we request the caller to call finishSendMessage() outside
            // the synchronization.
            //

            Debug.Assert(!message.prepared);

            OutputStream stream = message.stream;

            message.stream = doCompress(stream, message.compress);
            message.stream.prepareWrite();
            message.prepared = true;

            IceInternal.TraceUtil.traceSend(stream, _logger, _traceLevels);

            //
            // Send the message without blocking.
            //
            if(_observer != null)
            {
                observerStartWrite(message.stream.getBuffer());
            }
            int op = write(message.stream.getBuffer());
            if(op == 0)
            {
                if(_observer != null)
                {
                    observerFinishWrite(message.stream.getBuffer());
                }

                int status = IceInternal.OutgoingAsyncBase.AsyncStatusSent;
                if(message.sent())
                {
                    status = status | IceInternal.OutgoingAsyncBase.AsyncStatusInvokeSentCallback;
                }

                if(_acmLastActivity > -1)
                {
                    _acmLastActivity = IceInternal.Time.currentMonotonicTimeMillis();
                }
                return status;
            }

            message.adopt();

            _writeStream.swap(message.stream);
            _sendStreams.AddLast(message);
            scheduleTimeout(op);
            _threadPool.register(this, op);
            return IceInternal.OutgoingAsyncBase.AsyncStatusQueued;
        }

        private OutputStream doCompress(OutputStream uncompressed, bool compress)
        {
            if(_compressionSupported)
            {
                if(compress && uncompressed.size() >= 100)
                {
                    //
                    // Do compression.
                    //
                    IceInternal.Buffer cbuf = IceInternal.BZip2.compress(uncompressed.getBuffer(),
                                                                         IceInternal.Protocol.headerSize,
                                                                         _compressionLevel);
                    if(cbuf != null)
                    {
                        OutputStream cstream =
                            new OutputStream(uncompressed.instance(), uncompressed.getEncoding(), cbuf, true);

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

        private struct MessageInfo
        {
            public InputStream stream;
            public int invokeNum;
            public int requestId;
            public byte compress;
            public ServantManager servantManager;
            public ObjectAdapter adapter;
            public OutgoingAsyncBase outAsync;
            public HeartbeatCallback heartbeatCallback;
            public int messageDispatchCount;
        }

        private int parseMessage(ref MessageInfo info)
        {
            Debug.Assert(_state > StateNotValidated && _state < StateClosed);

            info.stream = new InputStream(_instance, Util.currentProtocolEncoding);
            _readStream.swap(info.stream);
            _readStream.resize(IceInternal.Protocol.headerSize);
            _readStream.pos(0);
            _readHeader = true;

            Debug.Assert(info.stream.pos() == info.stream.size());

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
                // The magic and version fields have already been checked.
                //
                info.stream.pos(8);
                byte messageType = info.stream.readByte();
                info.compress = info.stream.readByte();
                if(info.compress == (byte)2)
                {
                    if(_compressionSupported)
                    {
                        IceInternal.Buffer ubuf = IceInternal.BZip2.uncompress(info.stream.getBuffer(),
                                                                               IceInternal.Protocol.headerSize,
                                                                               _messageSizeMax);
                        info.stream = new InputStream(info.stream.instance(), info.stream.getEncoding(), ubuf, true);
                    }
                    else
                    {
                        FeatureNotSupportedException ex = new FeatureNotSupportedException();
                        ex.unsupportedFeature = "Cannot uncompress compressed message: bzip2 DLL not found";
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
                            setState(StateClosingPending, new CloseConnectionException());

                            //
                            // Notify the the transceiver of the graceful connection closure.
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

                    case IceInternal.Protocol.requestMsg:
                    {
                        if(_state >= StateClosing)
                        {
                            IceInternal.TraceUtil.trace("received request during closing\n" +
                                                        "(ignored by server, client will retry)", info.stream, _logger,
                                                        _traceLevels);
                        }
                        else
                        {
                            IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                            info.requestId = info.stream.readInt();
                            info.invokeNum = 1;
                            info.servantManager = _servantManager;
                            info.adapter = _adapter;
                            ++info.messageDispatchCount;
                        }
                        break;
                    }

                    case IceInternal.Protocol.requestBatchMsg:
                    {
                        if(_state >= StateClosing)
                        {
                            IceInternal.TraceUtil.trace("received batch request during closing\n" +
                                                        "(ignored by server, client will retry)", info.stream, _logger,
                                                        _traceLevels);
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
                            info.messageDispatchCount += info.invokeNum;
                        }
                        break;
                    }

                    case Protocol.replyMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        info.requestId = info.stream.readInt();
                        if(_asyncRequests.TryGetValue(info.requestId, out info.outAsync))
                        {
                            _asyncRequests.Remove(info.requestId);

                            info.outAsync.getIs().swap(info.stream);

                            //
                            // If we just received the reply for a request which isn't acknowledge as
                            // sent yet, we queue the reply instead of processing it right away. It
                            // will be processed once the write callback is invoked for the message.
                            //
                            OutgoingMessage message = _sendStreams.Count > 0 ? _sendStreams.First.Value : null;
                            if(message != null && message.outAsync == info.outAsync)
                            {
                                message.receivedReply = true;
                            }
                            else if(info.outAsync.response())
                            {
                                ++info.messageDispatchCount;
                            }
                            else
                            {
                                info.outAsync = null;
                            }
                            Monitor.PulseAll(this); // Notify threads blocked in close(false)
                        }
                        break;
                    }

                    case IceInternal.Protocol.validateConnectionMsg:
                    {
                        IceInternal.TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                        if(_heartbeatCallback != null)
                        {
                            info.heartbeatCallback = _heartbeatCallback;
                            ++info.messageDispatchCount;
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
                        _logger.warning("datagram connection exception:\n" + ex.ToString() + "\n" + _desc);
                    }
                }
                else
                {
                    setState(StateClosed, ex);
                }
            }

            return _state == StateHolding ? IceInternal.SocketOperation.None : IceInternal.SocketOperation.Read;
        }

        private void invokeAll(InputStream stream, int invokeNum, int requestId, byte compress,
                               IceInternal.ServantManager servantManager, ObjectAdapter adapter)
        {
            //
            // Note: In contrast to other private or protected methods, this
            // operation must be called *without* the mutex locked.
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
                    Debug.Assert(!response || invokeNum == 1);

                    inc = getIncoming(adapter, response, compress, requestId);

                    //
                    // Dispatch the invocation.
                    //
                    inc.invoke(servantManager, stream);

                    --invokeNum;

                    reclaimIncoming(inc);
                    inc = null;
                }

                stream.clear();
            }
            catch(LocalException ex)
            {
                invokeException(requestId, ex, invokeNum, false);
            }
            finally
            {
                if(inc != null)
                {
                    reclaimIncoming(inc);
                }
            }
        }

        private void scheduleTimeout(int status)
        {
            int timeout;
            if(_state < StateActive)
            {
                IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
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
                    status &= ~IceInternal.SocketOperation.Read;
                }
                timeout = _endpoint.timeout();
            }
            else
            {
                IceInternal.DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
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

            if((status & IceInternal.SocketOperation.Read) != 0)
            {
                if(_readTimeoutScheduled)
                {
                    _timer.cancel(_readTimeout);
                }
                _timer.schedule(_readTimeout, timeout);
                _readTimeoutScheduled = true;
            }
            if((status & (IceInternal.SocketOperation.Write | IceInternal.SocketOperation.Connect)) != 0)
            {
                if(_writeTimeoutScheduled)
                {
                    _timer.cancel(_writeTimeout);
                }
                _timer.schedule(_writeTimeout, timeout);
                _writeTimeoutScheduled = true;
            }
        }

        private void unscheduleTimeout(int status)
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

        private ConnectionInfo initConnectionInfo()
        {
            if(_state > StateNotInitialized && _info != null) // Update the connection info until it's initialized
            {
                return _info;
            }

            try
            {
                _info = _transceiver.getInfo();
            }
            catch(Ice.LocalException)
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

        ConnectionState toConnectionState(int state)
        {
            return connectionStateMap[state];
        }

        private void warning(string msg, System.Exception ex)
        {
            _logger.warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());
        }

        private void observerStartRead(IceInternal.Buffer buf)
        {
            if(_readStreamPos >= 0)
            {
                Debug.Assert(!buf.empty());
                _observer.receivedBytes(buf.b.position() - _readStreamPos);
            }
            _readStreamPos = buf.empty() ? -1 : buf.b.position();
        }

        private void observerFinishRead(IceInternal.Buffer buf)
        {
            if(_readStreamPos == -1)
            {
                return;
            }
            Debug.Assert(buf.b.position() >= _readStreamPos);
            _observer.receivedBytes(buf.b.position() - _readStreamPos);
            _readStreamPos = -1;
        }

        private void observerStartWrite(IceInternal.Buffer buf)
        {
            if(_writeStreamPos >= 0)
            {
                Debug.Assert(!buf.empty());
                _observer.sentBytes(buf.b.position() - _writeStreamPos);
            }
            _writeStreamPos = buf.empty() ? -1 : buf.b.position();
        }

        private void observerFinishWrite(IceInternal.Buffer buf)
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

        private IceInternal.Incoming getIncoming(ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            IceInternal.Incoming inc = null;

            if(_cacheBuffers)
            {
                lock(_incomingCacheMutex)
                {
                    if(_incomingCache == null)
                    {
                        inc = new IceInternal.Incoming(_instance, this, this, adapter, response, compress, requestId);
                    }
                    else
                    {
                        inc = _incomingCache;
                        _incomingCache = _incomingCache.next;
                        inc.reset(_instance, this, this, adapter, response, compress, requestId);
                        inc.next = null;
                    }
                }
            }
            else
            {
                inc = new IceInternal.Incoming(_instance, this, this, adapter, response, compress, requestId);
            }

            return inc;
        }

        internal void reclaimIncoming(IceInternal.Incoming inc)
        {
            if(_cacheBuffers && inc.reclaim())
            {
                lock(_incomingCacheMutex)
                {
                    inc.next = _incomingCache;
                    _incomingCache = inc;
                }
            }
        }

        private int read(IceInternal.Buffer buf)
        {
            int start = buf.b.position();
            int op = _transceiver.read(buf, ref _hasMoreData);
            if(_instance.traceLevels().network >= 3 && buf.b.position() != start)
            {
                StringBuilder s = new StringBuilder("received ");
                if(_endpoint.datagram())
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
                s.Append("\n");
                s.Append(ToString());
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
            }
            return op;
        }

        private int write(IceInternal.Buffer buf)
        {
            int start = buf.b.position();
            int op = _transceiver.write(buf);
            if(_instance.traceLevels().network >= 3 && buf.b.position() != start)
            {
                StringBuilder s = new StringBuilder("sent ");
                s.Append(buf.b.position() - start);
                if(!_endpoint.datagram())
                {
                    s.Append(" of ");
                    s.Append(buf.b.limit() - start);
                }
                s.Append(" bytes via ");
                s.Append(_endpoint.protocol());
                s.Append("\n");
                s.Append(ToString());
                _instance.initializationData().logger.trace(_instance.traceLevels().networkCat, s.ToString());
            }
            return op;
        }

        private class OutgoingMessage
        {
            internal OutgoingMessage(OutputStream stream, bool compress, bool adopt)
            {
                this.stream = stream;
                this.compress = compress;
                this._adopt = adopt;
            }

            internal OutgoingMessage(IceInternal.OutgoingAsyncBase outAsync, OutputStream stream,
                                     bool compress, int requestId)
            {
                this.outAsync = outAsync;
                this.stream = stream;
                this.compress = compress;
                this.requestId = requestId;
            }

            internal void canceled()
            {
                Debug.Assert(outAsync != null); // Only requests can timeout.
                outAsync = null;
            }

            internal void adopt()
            {
                if(_adopt)
                {
                    OutputStream stream = new OutputStream(this.stream.instance(), Util.currentProtocolEncoding);
                    stream.swap(this.stream);
                    this.stream = stream;
                    _adopt = false;
                }
            }

            internal bool sent()
            {
                stream = null;
                if(outAsync != null)
                {
                    invokeSent = outAsync.sent();
                    return invokeSent ||receivedReply;
                }
                return false;
            }

            internal void completed(LocalException ex)
            {
                if(outAsync != null)
                {
                    if(outAsync.exception(ex))
                    {
                        outAsync.invokeException();
                    }
                }
                stream = null;
            }

            internal Ice.OutputStream stream;
            internal IceInternal.OutgoingAsyncBase outAsync;
            internal bool compress;
            internal int requestId;
            internal bool _adopt;
            internal bool prepared;
            internal bool isSent;
            internal bool invokeSent;
            internal bool receivedReply;
        }

        private Communicator _communicator;
        private IceInternal.Instance _instance;
        private IceInternal.ACMMonitor _monitor;
        private IceInternal.Transceiver _transceiver;
        private string _desc;
        private string _type;
        private IceInternal.Connector _connector;
        private IceInternal.EndpointI _endpoint;

        private ObjectAdapter _adapter;
        private IceInternal.ServantManager _servantManager;

        private Logger _logger;
        private IceInternal.TraceLevels _traceLevels;
        private IceInternal.ThreadPool _threadPool;

        private IceInternal.Timer _timer;
        private IceInternal.TimerTask _writeTimeout;
        private bool _writeTimeoutScheduled;
        private IceInternal.TimerTask _readTimeout;
        private bool _readTimeoutScheduled;

        private StartCallback _startCallback = null;

        private bool _warn;
        private bool _warnUdp;

        private long _acmLastActivity;

        private int _compressionLevel;

        private int _nextRequestId;

        private Dictionary<int, IceInternal.OutgoingAsyncBase> _asyncRequests =
            new Dictionary<int, IceInternal.OutgoingAsyncBase>();

        private LocalException _exception;

        private readonly int _messageSizeMax;
        private IceInternal.BatchRequestQueue _batchRequestQueue;

        private LinkedList<OutgoingMessage> _sendStreams = new LinkedList<OutgoingMessage>();

        private InputStream _readStream;
        private bool _readHeader;
        private OutputStream _writeStream;

        private ConnectionObserver _observer;
        private int _readStreamPos;
        private int _writeStreamPos;

        private int _dispatchCount;

        private int _state; // The current state.
        private bool _shutdownInitiated = false;
        private bool _initialized = false;
        private bool _validated = false;

        private IceInternal.Incoming _incomingCache;
        private object _incomingCacheMutex = new object();

        private static bool _compressionSupported;

        private bool _cacheBuffers;

        private Ice.ConnectionInfo _info;

        private Ice.CloseCallback _closeCallback;
        private Ice.HeartbeatCallback _heartbeatCallback;

        private static ConnectionState[] connectionStateMap = new ConnectionState[] {
            ConnectionState.ConnectionStateValidating,   // StateNotInitialized
            ConnectionState.ConnectionStateValidating,   // StateNotValidated
            ConnectionState.ConnectionStateActive,       // StateActive
            ConnectionState.ConnectionStateHolding,      // StateHolding
            ConnectionState.ConnectionStateClosing,      // StateClosing
            ConnectionState.ConnectionStateClosing,      // StateClosingPending
            ConnectionState.ConnectionStateClosed,       // StateClosed
            ConnectionState.ConnectionStateClosed,       // StateFinished
        };
    }
}
