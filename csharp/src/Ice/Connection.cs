//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice.Instrumentation;
using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Ice
{
    public delegate void CloseCallback(Connection con);
    public delegate void HeartbeatCallback(Connection con);

    public enum ACMClose
    {
        CloseOff,
        CloseOnIdle,
        CloseOnInvocation,
        CloseOnInvocationAndIdle,
        CloseOnIdleForceful
    }

    public enum ACMHeartbeat
    {
        HeartbeatOff,
        HeartbeatOnDispatch,
        HeartbeatOnIdle,
        HeartbeatAlways
    }

    [Serializable]
    public struct ACM : IEquatable<ACM>
    {
        public int Timeout;
        public ACMClose Close;
        public ACMHeartbeat Heartbeat;

        public ACM(int timeout, ACMClose close, ACMHeartbeat heartbeat)
        {
            Timeout = timeout;
            Close = close;
            Heartbeat = heartbeat;
        }

        public override int GetHashCode() => HashCode.Combine(Timeout, Close, Heartbeat);

        public bool Equals(ACM other) =>
            Timeout == other.Timeout && Close == other.Close && Heartbeat == other.Heartbeat;

        public override bool Equals(object? other) =>
            ReferenceEquals(this, other) || (other is ACM value && Equals(value));

        public static bool operator ==(ACM lhs, ACM rhs) => Equals(lhs, rhs);

        public static bool operator !=(ACM lhs, ACM rhs) => !Equals(lhs, rhs);
    }

    public enum ConnectionClose
    {
        Forcefully,
        Gracefully,
        GracefullyWithWait
    }

    public sealed class Connection : IceInternal.EventHandler, ICancellationHandler
    {
        public interface IStartCallback
        {
            void ConnectionStartCompleted(Connection connection);
            void ConnectionStartFailed(Connection connection, System.Exception ex);
        }

        private class TimeoutCallback : ITimerTask
        {
            public TimeoutCallback(Connection connection) => _connection = connection;

            public void RunTimerTask() => _connection.TimedOut();

            private readonly Connection _connection;
        }

        public void Start(IStartCallback? callback)
        {
            try
            {
                lock (this)
                {
                    //
                    // The connection might already be closed if the communicator was destroyed.
                    //
                    if (_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    if (!Initialize(SocketOperation.None) || !Validate(SocketOperation.None))
                    {
                        _startCallback = callback;
                        return;
                    }

                    SetState(StateActive);
                }
            }
            catch (System.Exception ex)
            {
                lock (this)
                {
                    SetState(StateClosed, ex);
                }
                Debug.Assert(_exception != null);
                callback?.ConnectionStartFailed(this, _exception);
                return;
            }

            callback?.ConnectionStartCompleted(this);
        }

        internal void StartAndWait()
        {
            try
            {
                lock (this)
                {
                    //
                    // The connection might already be closed if the communicator was destroyed.
                    //
                    if (_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    if (!Initialize(SocketOperation.None) || !Validate(SocketOperation.None))
                    {
                        //
                        // Wait for the connection to be validated.
                        //
                        while (_state <= StateNotValidated)
                        {
                            System.Threading.Monitor.Wait(this);
                        }

                        if (_state >= StateClosing)
                        {
                            Debug.Assert(_exception != null);
                            throw _exception;
                        }
                    }

                    SetState(StateActive);
                }
            }
            catch (System.Exception ex)
            {
                lock (this)
                {
                    SetState(StateClosed, ex);
                }
                WaitUntilFinished();
                return;
            }
        }

        internal void Activate()
        {
            lock (this)
            {
                if (_state <= StateNotValidated)
                {
                    return;
                }

                if (_acmLastActivity > -1)
                {
                    _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                }
                SetState(StateActive);
            }
        }

        internal void Destroy(Exception ex)
        {
            lock (this)
            {
                SetState(StateClosing, ex);
            }
        }

        /// <summary>Manually close the connection using the specified closure mode.</summary>
        /// <param name="mode">Determines how the connection will be closed.</param>
        public void Close(ConnectionClose mode)
        {
            lock (this)
            {
                if (mode == ConnectionClose.Forcefully)
                {
                    SetState(StateClosed, new ConnectionClosedLocallyException("connection closed forcefully"));
                }
                else if (mode == ConnectionClose.Gracefully)
                {
                    SetState(StateClosing, new ConnectionClosedLocallyException("connection closed gracefully"));
                }
                else
                {
                    Debug.Assert(mode == ConnectionClose.GracefullyWithWait);

                    //
                    // Wait until all outstanding requests have been completed.
                    //
                    while (_asyncRequests.Count != 0)
                    {
                        System.Threading.Monitor.Wait(this);
                    }

                    SetState(StateClosing, new ConnectionClosedLocallyException("connection closed gracefully"));
                }
            }
        }

        internal bool Active
        {
            get
            {
                lock (this)
                {
                    return _state > StateNotValidated && _state < StateClosing;
                }
            }
        }

        /// <summary>
        /// Throw an exception indicating the reason for connection closure.
        /// For example,
        /// ConnectionClosedByPeerException is raised if the connection was closed gracefully by the peer,
        /// whereas ConnectionClosedLocallyException is raised if the connection was
        /// manually closed by the application. This operation does nothing if the connection is
        /// not yet closed.
        /// </summary>
        public void ThrowException()
        {
            lock (this)
            {
                if (_exception != null)
                {
                    Debug.Assert(_state >= StateClosing);
                    throw _exception;
                }
            }
        }

        internal void WaitUntilFinished()
        {
            lock (this)
            {
                //
                // We wait indefinitely until the connection is finished and all
                // outstanding requests are completed. Otherwise we couldn't
                // guarantee that there are no outstanding calls when deactivate()
                // is called on the servant locators.
                //
                while (_state < StateFinished || _dispatchCount > 0)
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

        internal void UpdateObserver()
        {
            lock (this)
            {
                if (_state < StateNotValidated || _state > StateClosed)
                {
                    return;
                }

                _communicatorObserver = _communicator.Observer!;
                _observer = _communicatorObserver.GetConnectionObserver(InitConnectionInfo(), _endpoint,
                    ToConnectionState(_state), _observer);
                if (_observer != null)
                {
                    _observer.Attach();
                }
            }
        }

        internal void Monitor(long now, ACMConfig acm)
        {
            lock (this)
            {
                if (_state != StateActive)
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
                // Note that this doesn't imply that we are sending 4 heartbeats
                // per timeout period because the monitor() method is still only
                // called every (timeout / 2) period.
                //
                if (acm.Heartbeat == ACMHeartbeat.HeartbeatAlways ||
                   (acm.Heartbeat != ACMHeartbeat.HeartbeatOff && _writeBufferSize == 0 &&
                    now >= (_acmLastActivity + (acm.Timeout / 4))))
                {
                    if (acm.Heartbeat != ACMHeartbeat.HeartbeatOnDispatch || _dispatchCount > 0)
                    {
                        SendHeartbeatNow();
                    }
                }

                if (_readBuffer.Count > Ice1Definitions.HeaderSize || _writeBufferSize > 0)
                {
                    //
                    // If writing or reading, nothing to do, the connection
                    // timeout will kick-in if writes or reads don't progress.
                    // This check is necessary because the activity timer is
                    // only set when a message is fully read/written.
                    //
                    return;
                }

                if (acm.Close != ACMClose.CloseOff && now >= (_acmLastActivity + acm.Timeout))
                {
                    if (acm.Close == ACMClose.CloseOnIdleForceful ||
                       (acm.Close != ACMClose.CloseOnIdle && (_asyncRequests.Count > 0)))
                    {
                        //
                        // Close the connection if we didn't receive a heartbeat in
                        // the last period.
                        //
                        SetState(StateClosed, new ConnectionTimeoutException());
                    }
                    else if (acm.Close != ACMClose.CloseOnInvocation &&
                            _dispatchCount == 0 && _asyncRequests.Count == 0)
                    {
                        //
                        // The connection is idle, close it.
                        //
                        SetState(StateClosing, new ConnectionIdleException());
                    }
                }
            }
        }

        internal int SendAsyncRequest(OutgoingAsyncBase outgoing, bool compress, bool response)
        {
            lock (this)
            {
                //
                // If the exception is thrown before we even have a chance
                // to send our request, we always try to send the request
                // again.
                //
                if (_exception != null)
                {
                    throw new RetryException(_exception);
                }

                Debug.Assert(_state > StateNotValidated);
                Debug.Assert(_state < StateClosing);

                //
                // Notify the request that it's cancelable with this connection.
                // This will throw if the request is canceled.
                //
                outgoing.Cancelable(this);
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
                }

                List<ArraySegment<byte>> data = outgoing.GetRequestData(requestId);
                int size = data.GetByteCount();
                // Ensure the message isn't bigger than what we can send with the
                // transport.
                _transceiver.CheckSendSize(size);

                outgoing.AttachRemoteObserver(InitConnectionInfo(), _endpoint, requestId,
                    size - (Ice1Definitions.HeaderSize + 4));

                int status = OutgoingAsyncBase.AsyncStatusQueued;
                try
                {
                    status = SendMessage(new OutgoingMessage(outgoing, data, compress, requestId));
                }
                catch (Exception ex)
                {
                    SetState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                if (response)
                {
                    //
                    // Add to the asynchronous requests map.
                    //
                    _asyncRequests[requestId] = outgoing;
                }
                return status;
            }
        }

        /// <summary>
        /// Set a close callback on the connection.
        /// The callback is called by the
        /// connection when it's closed. The callback is called from the
        /// Ice thread pool associated with the connection. If the callback needs
        /// more information about the closure, it can call Connection.throwException.
        ///
        /// </summary>
        /// <param name="callback">The close callback object.</param>
        public void SetCloseCallback(CloseCallback callback)
        {
            lock (this)
            {
                if (_state >= StateClosed)
                {
                    if (callback != null)
                    {
                        ThreadPool.Dispatch(() =>
                        {
                            try
                            {
                                callback(this);
                            }
                            catch (Exception ex)
                            {
                                _logger.Error("connection callback exception:\n" + ex + '\n' + _desc);
                            }
                        });
                    }
                }
                else
                {
                    _closeCallback = callback;
                }
            }
        }

        /// <summary>
        /// Set a heartbeat callback on the connection.
        /// The callback is called by the
        /// connection when a heartbeat is received. The callback is called
        /// from the Ice thread pool associated with the connection.
        ///
        /// </summary>
        /// <param name="callback">The heartbeat callback object.</param>
        public void SetHeartbeatCallback(HeartbeatCallback callback)
        {
            lock (this)
            {
                if (_state >= StateClosed)
                {
                    return;
                }
                _heartbeatCallback = callback;
            }
        }

        /// <summary>
        /// Send a heartbeat message.
        /// </summary>
        public void Heartbeat() => HeartbeatAsync().Wait();

        private class HeartbeatTaskCompletionCallback : TaskCompletionCallback<object>
        {
            public HeartbeatTaskCompletionCallback(IProgress<bool>? progress,
                                                   CancellationToken cancellationToken) :
                base(progress, cancellationToken)
            {
            }

            public override void HandleInvokeResponse(bool ok, OutgoingAsyncBase og) => SetResult(null!);
        }

        private class HeartbeatOutgoingAsync : OutgoingAsyncBase
        {
            public HeartbeatOutgoingAsync(Connection connection,
                                          Communicator communicator,
                                          IOutgoingAsyncCompletionCallback completionCallback) :
                base(communicator, completionCallback) => _connection = connection;

            public override List<ArraySegment<byte>> GetRequestData(int requestId) => _validateConnectionMessage;

            public void Invoke()
            {
                try
                {
                    int status = _connection.SendAsyncRequest(this, false, false);

                    if ((status & AsyncStatusSent) != 0)
                    {
                        SentSynchronously = true;
                        if ((status & AsyncStatusInvokeSentCallback) != 0)
                        {
                            InvokeSent();
                        }
                    }
                }
                catch (RetryException ex)
                {
                    if (Exception(ex.InnerException!))
                    {
                        InvokeExceptionAsync();
                    }
                }
                catch (Exception ex)
                {
                    if (Exception(ex))
                    {
                        InvokeExceptionAsync();
                    }
                }
            }

            private readonly Connection _connection;
        }

        public Task HeartbeatAsync(IProgress<bool>? progress = null, CancellationToken cancel = new CancellationToken())
        {
            var completed = new HeartbeatTaskCompletionCallback(progress, cancel);
            var outgoing = new HeartbeatOutgoingAsync(this, _communicator, completed);
            outgoing.Invoke();
            return completed.Task;
        }

        /// <summary>
        /// Set the active connection management parameters.
        /// </summary>
        /// <param name="timeout">The timeout value in seconds, must be &gt;= 0.
        ///
        /// </param>
        /// <param name="close">The close condition
        ///
        /// </param>
        /// <param name="heartbeat">The heartbeat condition</param>
        public void SetACM(int? timeout, ACMClose? close, ACMHeartbeat? heartbeat)
        {
            lock (this)
            {
                if (timeout is int timeoutValue && timeoutValue < 0)
                {
                    throw new ArgumentException("invalid negative ACM timeout value", nameof(timeout));
                }

                if (_monitor == null || _state >= StateClosed)
                {
                    return;
                }

                if (_state == StateActive)
                {
                    _monitor.Remove(this);
                }
                _monitor = _monitor.Acm(timeout, close, heartbeat);

                if (_monitor.GetACM().Timeout <= 0)
                {
                    _acmLastActivity = -1; // Disable the recording of last activity.
                }
                else if (_state == StateActive && _acmLastActivity == -1)
                {
                    _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                }

                if (_state == StateActive)
                {
                    _monitor.Add(this);
                }
            }
        }

        /// <summary>
        /// Get the ACM parameters.
        /// </summary>
        /// <returns>The ACM parameters.</returns>
        public ACM GetACM()
        {
            lock (this)
            {
                return _monitor != null ? _monitor.GetACM() : new ACM(0, ACMClose.CloseOff, ACMHeartbeat.HeartbeatOff);
            }
        }

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, System.Exception ex)
        {
            //
            // NOTE: This isn't called from a thread pool thread.
            //

            lock (this)
            {
                if (_state >= StateClosed)
                {
                    return; // The request has already been or will be shortly notified of the failure.
                }

                OutgoingMessage? o = _outgoingMessages.FirstOrDefault(m => m.OutAsync == outAsync);
                if (o != null)
                {
                    if (o.RequestId > 0)
                    {
                        _asyncRequests.Remove(o.RequestId);
                    }

                    //
                    // If the request is being sent, don't remove it from the send streams,
                    // it will be removed once the sending is finished.
                    //
                    if (o == _outgoingMessages.First!.Value)
                    {
                        o.Canceled();
                    }
                    else
                    {
                        o.Canceled();
                        _outgoingMessages.Remove(o);
                    }
                    if (outAsync.Exception(ex))
                    {
                        outAsync.InvokeExceptionAsync();
                    }
                    return;
                }

                if (outAsync is OutgoingAsync)
                {
                    foreach (KeyValuePair<int, OutgoingAsyncBase> kvp in _asyncRequests)
                    {
                        if (kvp.Value == outAsync)
                        {
                            _asyncRequests.Remove(kvp.Key);
                            if (outAsync.Exception(ex))
                            {
                                outAsync.InvokeExceptionAsync();
                            }
                            return;
                        }
                    }
                }
            }
        }

        internal IConnector Connector
        {
            get
            {
                Debug.Assert(_connector != null);
                return _connector; // No mutex protection necessary, _connector is immutable.
            }
        }

        /// <summary>Explicitly sets an object adapter that dispatches requests received over this connection.
        /// A client can invoke an operation on a server using a proxy, and then set an object adapter for the
        /// outgoing connection used by the proxy in order to receive callbacks. This is useful if the server
        /// cannot establish a connection back to the client, for example because of firewalls.</summary>
        /// <param name="adapter">The object adapter. This object adapter is automatically removed from the
        /// connection when it is deactivated.</param>.
        public void SetAdapter(ObjectAdapter? adapter)
        {
            if (adapter != null)
            {
                // We're locking both the object adapter and this connection (in this order) to ensure the adapter
                // gets cleared from this connection during the deactivation of the object adapter.
                adapter.ExecuteOnlyWhenActive(() =>
                    {
                        lock (this)
                        {
                            _adapter = adapter;
                        }
                    });
            }
            else
            {
                lock (this)
                {
                    if (_state <= StateNotValidated || _state >= StateClosing)
                    {
                        return;
                    }
                    _adapter = null;
                }
            }

            // We never change the thread pool with which we were initially registered, even if we add or remove an
            // object adapter.
        }

        /// <summary>
        /// Get the object adapter that dispatches requests for this
        /// connection.
        /// </summary>
        /// <returns>The object adapter that dispatches requests for the
        /// connection, or null if no adapter is set.</returns>
        ///
        public ObjectAdapter? GetAdapter()
        {
            lock (this)
            {
                return _adapter;
            }
        }

        /// <summary>
        /// Get the endpoint from which the connection was created.
        /// </summary>
        /// <returns>The endpoint from which the connection was created.</returns>
        public Endpoint Endpoint => _endpoint; // No mutex protection necessary, _endpoint is immutable.

        /// <summary>Creates a special "fixed" proxy that always uses this connection. This proxy can be used for
        /// callbacks from a server to a client if the server cannot directly establish a connection to the client,
        /// for example because of firewalls. In this case, the server would create a proxy using an already
        /// established connection from the client.</summary>
        /// <param name="identity">The identity for which a proxy is to be created.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory, where INamePrx is the desired proxy type.
        /// </param>
        /// <returns>A proxy that matches the given identity and uses this connection.</returns>
        public T CreateProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => factory(new Reference(_communicator, this, identity));

        internal void SetAdapterImpl(ObjectAdapter adapter)
        {
            lock (this)
            {
                if (_state <= StateNotValidated || _state >= StateClosing)
                {
                    return;
                }
                _adapter = adapter;
            }
        }

        //
        // Operations from EventHandler
        //
        public override bool StartAsync(int operation, IceInternal.AsyncCallback cb, ref bool completedSynchronously)
        {
            if (_state >= StateClosed)
            {
                return false;
            }

            try
            {
                if ((operation & SocketOperation.Write) != 0)
                {
                    completedSynchronously = _transceiver.StartWrite(_writeBuffer, _writeBufferOffset, cb, this, out bool completed);
                    if (completed && _outgoingMessages.Count > 0)
                    {
                        // The whole message is written, assume it's sent now for at-most-once semantics.
                        _outgoingMessages.First!.Value.IsSent = true;
                    }
                }
                else if ((operation & SocketOperation.Read) != 0)
                {
                    int start = _readBufferOffset;
                    completedSynchronously = _transceiver.StartRead(ref _readBuffer, ref _readBufferOffset, cb, this);
                    if (start != _readBufferOffset)
                    {
                        TraceReceivedAndUpdateObserver(_readBuffer.Count, start, _readBufferOffset);
                    }
                }
            }
            catch (System.Exception ex)
            {
                SetState(StateClosed, ex);
                return false;
            }
            return true;
        }

        public override bool FinishAsync(int operation)
        {
            try
            {
                if ((operation & SocketOperation.Write) != 0)
                {
                    int start = _writeBufferOffset;
                    _transceiver.FinishWrite(_writeBuffer, ref _writeBufferOffset);
                    if (start != _writeBufferOffset)
                    {
                        TraceSentAndUpdateObserver(_writeBufferSize, start, _writeBufferOffset);
                    }
                }
                else if ((operation & SocketOperation.Read) != 0)
                {
                    int start = _readBufferOffset;
                    _transceiver.FinishRead(ref _readBuffer, ref _readBufferOffset);
                    if (start != _readBufferOffset)
                    {
                        TraceReceivedAndUpdateObserver(_readBuffer.Count, start, _readBufferOffset);
                    }
                }
            }
            catch (System.Exception ex)
            {
                SetState(StateClosed, ex);
            }
            return _state < StateClosed;
        }

        public override void Message(ref ThreadPoolCurrent current)
        {
            IStartCallback? startCB = null;
            Queue<OutgoingMessage>? sentCBs = null;
            var info = new MessageInfo();
            int dispatchCount = 0;

            var msg = new ThreadPoolMessage(this);
            try
            {
                lock (this)
                {
                    if (!msg.StartIOScope(ref current))
                    {
                        return;
                    }

                    if (_state >= StateClosed)
                    {
                        return;
                    }

                    int readyOp = current.Operation;
                    try
                    {
                        UnscheduleTimeout(current.Operation);

                        int writeOp = SocketOperation.None;
                        int readOp = SocketOperation.None;
                        if ((readyOp & SocketOperation.Write) != 0)
                        {
                            writeOp = Write(_writeBuffer, _writeBufferSize, ref _writeBufferOffset);
                        }

                        while ((readyOp & SocketOperation.Read) != 0)
                        {
                            readOp = Read(ref _readBuffer, ref _readBufferOffset);
                            if ((readOp & SocketOperation.Read) != 0)
                            {
                                break;
                            }

                            if (_readHeader) // Read header if necessary.
                            {
                                _readHeader = false;

                                //
                                // Connection is validated on first message. This is only used by
                                // setState() to check whether or not we can print a connection
                                // warning (a client might close the connection forcefully if the
                                // connection isn't validated, we don't want to print a warning
                                // in this case).
                                //
                                _validated = true;

                                if (_readBufferOffset < Ice1Definitions.HeaderSize)
                                {
                                    //
                                    // This situation is possible for small UDP packets.
                                    //
                                    throw new InvalidDataException(
                                        $"received packet with only {_readBufferOffset} bytes");
                                }

                                Ice1Definitions.CheckHeader(_readBuffer.AsSpan(0, 8));
                                int size = InputStream.ReadInt(_readBuffer.Slice(10, 4));
                                if (size < Ice1Definitions.HeaderSize)
                                {
                                    throw new InvalidDataException($"received ice1 frame with only {size} bytes");
                                }

                                if (size > _messageSizeMax)
                                {
                                    throw new InvalidDataException(
                                        $"frame with {size} bytes exceeds Ice.MessageSizeMax value");
                                }

                                if (_endpoint.IsDatagram && size > _readBufferOffset)
                                {
                                    if (_warnUdp)
                                    {
                                        _logger.Warning($"maximum datagram size of {_readBufferOffset} exceeded");
                                    }
                                    _readBuffer = ArraySegment<byte>.Empty;
                                    _readBufferOffset = 0;
                                    _readHeader = true;
                                    return;
                                }

                                if (size > _readBuffer.Array!.Length)
                                {
                                    // Allocate a new array and copy the header over
                                    byte[] readBuffer = new byte[size];
                                    _readBuffer.AsSpan().CopyTo(readBuffer.AsSpan(0, Ice1Definitions.HeaderSize));
                                    _readBuffer = readBuffer;
                                }
                                else if (size > _readBuffer.Count)
                                {
                                    _readBuffer = new ArraySegment<byte>(_readBuffer.Array, 0, size);
                                }
                                Debug.Assert(size == _readBuffer.Count);
                            }

                            if (_readBufferOffset < _readBuffer.Count)
                            {
                                Debug.Assert(!_endpoint.IsDatagram);
                                continue;
                            }
                            break;
                        }

                        int newOp = readOp | writeOp;
                        readyOp &= ~newOp;
                        Debug.Assert(readyOp != 0 || newOp != 0);

                        if (_state <= StateNotValidated)
                        {
                            if (newOp != 0)
                            {
                                //
                                // Wait for all the transceiver conditions to be
                                // satisfied before continuing.
                                //
                                ScheduleTimeout(newOp);
                                ThreadPool.Update(this, current.Operation, newOp);
                                return;
                            }

                            if (_state == StateNotInitialized && !Initialize(current.Operation))
                            {
                                return;
                            }

                            if (_state <= StateNotValidated && !Validate(current.Operation))
                            {
                                return;
                            }

                            ThreadPool.Unregister(this, current.Operation);

                            SetState(StateActive);
                            if (_startCallback != null)
                            {
                                startCB = _startCallback;
                                _startCallback = null;
                                if (startCB != null)
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
                            if ((readyOp & SocketOperation.Read) != 0)
                            {
                                newOp |= ParseMessage(ref info);
                                dispatchCount += info.MessageDispatchCount;
                            }

                            if ((readyOp & SocketOperation.Write) != 0)
                            {
                                newOp |= SendNextMessage(out sentCBs);
                                if (sentCBs != null)
                                {
                                    ++dispatchCount;
                                }
                            }

                            if (_state < StateClosed)
                            {
                                ScheduleTimeout(newOp);
                                ThreadPool.Update(this, current.Operation, newOp);
                            }
                        }

                        if (_acmLastActivity > -1)
                        {
                            _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                        }

                        if (dispatchCount == 0)
                        {
                            return; // Nothing to dispatch we're done!
                        }

                        _dispatchCount += dispatchCount;
                        msg.Completed(ref current);
                    }
                    catch (TransportException ex)
                    {
                        SetState(StateClosed, ex);
                        return;
                    }
                    catch (Exception ex)
                    {
                        if (_endpoint.IsDatagram)
                        {
                            if (_warn)
                            {
                                _logger.Warning(string.Format("datagram connection exception:\n{0}\n{1}", ex, _desc));
                            }
                            _readBuffer = ArraySegment<byte>.Empty;
                            _readBufferOffset = 0;
                            _readHeader = true;
                        }
                        else
                        {
                            SetState(StateClosed, ex);
                        }
                        return;
                    }

                    ThreadPoolCurrent c = current;
                    ThreadPool.Dispatch(() =>
                    {
                        Dispatch(startCB, sentCBs, info);
                        msg.Destroy(ref c);
                    });
                }
            }
            finally
            {
                msg.FinishIOScope(ref current);
            }
        }

        private void Dispatch(IStartCallback? startCB, Queue<OutgoingMessage>? sentCBs, MessageInfo info)
        {
            int dispatchedCount = 0;

            //
            // Notify the factory that the connection establishment and
            // validation has completed.
            //
            if (startCB != null)
            {
                startCB.ConnectionStartCompleted(this);
                ++dispatchedCount;
            }

            //
            // Notify AMI calls that the message was sent.
            //
            if (sentCBs != null)
            {
                foreach (OutgoingMessage m in sentCBs)
                {
                    if (m.InvokeSent)
                    {
                        Debug.Assert(m.OutAsync != null);
                        m.OutAsync.InvokeSent();
                    }
                    if (m.ReceivedReply)
                    {
                        Debug.Assert(m.OutAsync != null);
                        var outAsync = (OutgoingAsync)m.OutAsync;
                        if (outAsync.Response(m.IncomingData))
                        {
                            outAsync.InvokeResponse();
                        }
                    }
                }
                ++dispatchedCount;
            }

            //
            // Asynchronous replies must be handled outside the thread
            // synchronization, so that nested calls are possible.
            //
            if (info.OutAsync != null)
            {
                info.OutAsync.InvokeResponse();
                ++dispatchedCount;
            }

            if (info.HeartbeatCallback != null)
            {
                try
                {
                    info.HeartbeatCallback(this);
                }
                catch (Exception ex)
                {
                    _logger.Error("connection callback exception:\n" + ex + '\n' + _desc);
                }
                ++dispatchedCount;
            }

            //
            // Method invocation must be done outside the thread synchronization, so that nested
            // calls are possible.
            //
            if (info.InvokeNum > 0)
            {
                ValueTask vt = InvokeAllAsync(info.Data, info.InvokeNum, info.RequestId, info.Compress,
                    info.Adapter!);

                // TODO: do something with the value task

                //
                // Don't increase dispatchedCount, the dispatch count is
                // decreased when the incoming reply is sent.
                //
            }

            //
            // Decrease dispatch count.
            //
            if (dispatchedCount > 0)
            {
                lock (this)
                {
                    _dispatchCount -= dispatchedCount;
                    if (_dispatchCount == 0)
                    {
                        //
                        // Only initiate shutdown if not already done. It
                        // might have already been done if the sent callback
                        // or AMI callback was dispatched when the connection
                        // was already in the closing state.
                        //
                        if (_state == StateClosing)
                        {
                            try
                            {
                                InitiateShutdown();
                            }
                            catch (Exception ex)
                            {
                                SetState(StateClosed, ex);
                            }
                        }
                        else if (_state == StateFinished)
                        {
                            Reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }
                }
            }
        }

        public override void Finished(ref ThreadPoolCurrent current)
        {
            lock (this)
            {
                Debug.Assert(_state == StateClosed);
                UnscheduleTimeout(SocketOperation.Read | SocketOperation.Write);
            }

            //
            // If there are no callbacks to call, we don't call ioCompleted() since we're not going
            // to call code that will potentially block (this avoids promoting a new leader and
            // unnecessary thread creation, especially if this is called on shutdown).
            //
            if (_startCallback == null && _outgoingMessages.Count == 0 && _asyncRequests.Count == 0 &&
               _closeCallback == null && _heartbeatCallback == null)
            {
                Finish();
                return;
            }

            //
            // Unlike C++/Java, this method is called from an IO thread of the .NET thread
            // pool of from the communicator async IO thread. While it's fine to handle the
            // non-blocking activity of the connection from these threads, the dispatching
            // of the message must be taken care of by the Ice thread pool.
            //
            ThreadPool.Dispatch(Finish);
        }

        private new void Finish() // TODO: rename to avoid new
        {
            if (!_initialized)
            {
                if (_communicator.TraceLevels.Network >= 2)
                {
                    var s = new StringBuilder("failed to ");
                    s.Append(_connector != null ? "establish" : "accept");
                    s.Append(" ");
                    s.Append(_endpoint.Name);
                    s.Append(" connection\n");
                    s.Append(ToString());
                    s.Append("\n");
                    s.Append(_exception);
                    _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
                }
            }
            else if (_communicator.TraceLevels.Network >= 1)
            {
                var s = new StringBuilder("closed ");
                s.Append(_endpoint.Name);
                s.Append(" connection\n");
                s.Append(ToString());

                //
                // Trace the cause of unexpected connection closures
                //
                if (!(_exception is ConnectionClosedException ||
                      _exception is ConnectionIdleException ||
                      _exception is CommunicatorDestroyedException ||
                      _exception is ObjectAdapterDeactivatedException))
                {
                    s.Append("\n");
                    s.Append(_exception);
                }

                _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
            }

            if (_startCallback != null)
            {
                Debug.Assert(_exception != null);
                _startCallback.ConnectionStartFailed(this, _exception);
                _startCallback = null;
            }

            if (_outgoingMessages.Count > 0)
            {
                if (_writeBufferSize > 0)
                {
                    // Return the stream to the outgoing call. This is important for
                    // retriable AMI calls which are not marshalled again.
                    OutgoingMessage message = _outgoingMessages.First!.Value;
                    Debug.Assert(message.OutgoingData != null);
                    _writeBufferOffset = 0;
                    _writeBufferSize = 0;
                    _writeBuffer = _emptyBuffer;

                    //
                    // The current message might be sent but not yet removed from _sendStreams. If
                    // the response has been received in the meantime, we remove the message from
                    // _sendStreams to not call finished on a message which is already done.
                    //
                    if (message.IsSent || message.ReceivedReply)
                    {
                        if (message.Sent() && message.InvokeSent)
                        {
                            Debug.Assert(message.OutAsync != null);
                            message.OutAsync.InvokeSent();
                        }
                        if (message.ReceivedReply)
                        {
                            Debug.Assert(message.OutAsync != null);
                            var outAsync = (OutgoingAsync)message.OutAsync;
                            if (outAsync.Response(message.IncomingData))
                            {
                                outAsync.InvokeResponse();
                            }
                        }
                        _outgoingMessages.RemoveFirst();
                    }
                }

                foreach (OutgoingMessage o in _outgoingMessages)
                {
                    o.Completed(_exception!);
                    if (o.RequestId > 0) // Make sure finished isn't called twice.
                    {
                        _asyncRequests.Remove(o.RequestId);
                    }
                }
                _outgoingMessages.Clear(); // Must be cleared before _requests because of Outgoing* references in OutgoingMessage
            }

            foreach (OutgoingAsyncBase o in _asyncRequests.Values)
            {
                if (o.Exception(_exception!))
                {
                    o.InvokeException();
                }
            }
            _asyncRequests.Clear();

            //
            // Don't wait to be reaped to reclaim memory allocated by read/write streams.
            //
            // TODO reclaim read/write buffers
            _readBuffer = ArraySegment<byte>.Empty;
            _readBufferOffset = 0;

            if (_closeCallback != null)
            {
                try
                {
                    _closeCallback(this);
                }
                catch (Exception ex)
                {
                    _logger.Error($"connection callback exception:\n{ex}\n{_desc}");
                }
                _closeCallback = null;
            }

            _heartbeatCallback = null;

            //
            // This must be done last as this will cause waitUntilFinished() to return (and communicator
            // objects such as the timer might be destroyed too).
            //
            lock (this)
            {
                SetState(StateFinished);

                if (_dispatchCount == 0)
                {
                    Reap();
                }
            }
        }

        private void SendResponse(OutgoingResponseFrame response, int requestId, byte compressionStatus)
        {
            lock (this)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if (--_dispatchCount == 0)
                    {
                        if (_state == StateFinished)
                        {
                            Reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }

                    if (_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    SendMessage(new OutgoingMessage(response, compressionStatus > 0, requestId));

                    if (_state == StateClosing && _dispatchCount == 0)
                    {
                        InitiateShutdown();
                    }
                }
                catch (Exception ex)
                {
                    SetState(StateClosed, ex);
                }
            }
        }

        private void SendNoResponse()
        {
            lock (this)
            {
                Debug.Assert(_state > StateNotValidated);

                try
                {
                    if (--_dispatchCount == 0)
                    {
                        if (_state == StateFinished)
                        {
                            Reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }

                    if (_state >= StateClosed)
                    {
                        Debug.Assert(_exception != null);
                        throw _exception;
                    }

                    if (_state == StateClosing && _dispatchCount == 0)
                    {
                        InitiateShutdown();
                    }
                }
                catch (System.Exception ex)
                {
                    SetState(StateClosed, ex);
                }
            }
        }

        private void InvokeException(System.Exception ex, int invokeNum)
        {
            // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
            // called in case of a fatal exception we decrement _dispatchCount here.

            lock (this)
            {
                SetState(StateClosed, ex);

                if (invokeNum > 0)
                {
                    Debug.Assert(_dispatchCount >= invokeNum);
                    _dispatchCount -= invokeNum;
                    if (_dispatchCount == 0)
                    {
                        if (_state == StateFinished)
                        {
                            Reap();
                        }
                        System.Threading.Monitor.PulseAll(this);
                    }
                }
            }
        }

        /// <summary>
        /// Return a description of the connection as human readable text,
        /// suitable for logging or error messages.
        /// </summary>
        /// <returns>The description of the connection as human readable
        /// text.</returns>
        public override string ToString() => _desc; // No mutex lock, _desc is immutable.

        public void TimedOut()
        {
            lock (this)
            {
                if (_state <= StateNotValidated)
                {
                    SetState(StateClosed, new ConnectTimeoutException());
                }
                else if (_state < StateClosed)
                {
                    SetState(StateClosed, new ConnectionTimeoutException());
                }
            }
        }

        /// <summary>
        /// Return the connection type.
        /// This corresponds to the endpoint
        /// type, i.e., "tcp", "udp", etc.
        ///
        /// </summary>
        /// <returns>The type of the connection.</returns>
        public string Type() => _type; // No mutex lock, _type is immutable.

        /// <summary>
        /// Get the timeout for the connection.
        /// </summary>
        /// <returns>The connection's timeout.</returns>
        public int Timeout => _endpoint.Timeout; // No mutex protection necessary, _endpoint is immutable.

        /// <summary>
        /// Returns the connection information.
        /// </summary>
        /// <returns>The connection information.</returns>
        public ConnectionInfo GetConnectionInfo()
        {
            lock (this)
            {
                if (_state >= StateClosed)
                {
                    throw _exception!;
                }
                return InitConnectionInfo();
            }
        }

        /// <summary>
        /// Set the connection buffer receive/send size.
        /// </summary>
        /// <param name="rcvSize">The connection receive buffer size.
        /// </param>
        /// <param name="sndSize">The connection send buffer size.</param>
        public void SetBufferSize(int rcvSize, int sndSize)
        {
            lock (this)
            {
                if (_state >= StateClosed)
                {
                    throw _exception!;
                }
                _transceiver.SetBufferSize(rcvSize, sndSize);
                _info = null; // Invalidate the cached connection info
            }
        }

        public IceInternal.ThreadPool ThreadPool { get; }

        internal Connection(Communicator communicator,
                            IACMMonitor? monitor,
                            ITransceiver transceiver,
                            IConnector? connector,
                            Endpoint endpoint,
                            ObjectAdapter? adapter)
        {
            _communicator = communicator;
            _monitor = monitor;
            _transceiver = transceiver;
            _desc = transceiver.ToString()!;
            _type = transceiver.Transport();
            _connector = connector;
            _endpoint = endpoint;
            _adapter = adapter;
            _communicatorObserver = communicator.Observer;
            _logger = communicator.Logger; // Cached for better performance.
            _traceLevels = communicator.TraceLevels; // Cached for better performance.
            _timer = communicator.Timer();
            _writeTimeout = new TimeoutCallback(this);
            _writeTimeoutScheduled = false;
            _readTimeout = new TimeoutCallback(this);
            _readTimeoutScheduled = false;
            _warn = communicator.GetPropertyAsBool("Ice.Warn.Connections") ?? false;
            _warnUdp = communicator.GetPropertyAsBool("Ice.Warn.Datagrams") ?? false;

            if (_monitor != null && _monitor.GetACM().Timeout > 0)
            {
                _acmLastActivity = Time.CurrentMonotonicTimeMillis();
            }
            else
            {
                _acmLastActivity = -1;
            }
            _nextRequestId = 1;
            _messageSizeMax = adapter != null ? adapter.MessageSizeMax : communicator.MessageSizeMax;
            _readBuffer = ArraySegment<byte>.Empty;
            _readHeader = false;
            _writeBuffer = new List<ArraySegment<byte>>();
            _writeBufferSize = 0;
            _writeBufferOffset = 0;
            _dispatchCount = 0;
            _state = StateNotInitialized;

            _compressionLevel = communicator.GetPropertyAsInt("Ice.Compression.Level") ?? 1;
            if (_compressionLevel < 1)
            {
                _compressionLevel = 1;
            }
            else if (_compressionLevel > 9)
            {
                _compressionLevel = 9;
            }

            if (adapter != null)
            {
                ThreadPool = adapter.ThreadPool;
            }
            else
            {
                ThreadPool = communicator.ClientThreadPool();
            }
            ThreadPool.Initialize(this);
        }

        private const int StateNotInitialized = 0;
        private const int StateNotValidated = 1;
        private const int StateActive = 2;
        private const int StateClosing = 3;
        private const int StateClosingPending = 4;
        private const int StateClosed = 5;
        private const int StateFinished = 6;

        private void SetState(int state, System.Exception ex)
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

            if (_exception == null)
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
                    if (!(_exception is ConnectionClosedException ||
                         _exception is ConnectionIdleException ||
                         _exception is CommunicatorDestroyedException ||
                         _exception is ObjectAdapterDeactivatedException ||
                         (_exception is ConnectionLostException && _state >= StateClosing)))
                    {
                        Warning("connection exception", _exception);
                    }
                }
            }

            //
            // We must set the new state before we notify requests of any
            // exceptions. Otherwise new requests may retry on a
            // connection that is not yet marked as closed or closing.
            //
            SetState(state);
        }

        private void SetState(int state)
        {
            //
            // We don't want to send close connection messages if the endpoint
            // only supports oneway transmission from client to server.
            //
            if (_endpoint.IsDatagram && state == StateClosing)
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
                            // Can only switch from validated to active.
                            if (_state != StateNotValidated)
                            {
                                return;
                            }
                            ThreadPool.Register(this, SocketOperation.Read);
                            break;
                        }

                    case StateClosing:
                    case StateClosingPending:
                        {
                            // Can't change back from closing pending.
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

                            ThreadPool.Finish(this);
                            _transceiver.Close();
                            break;
                        }

                    case StateFinished:
                        {
                            Debug.Assert(_state == StateClosed);
                            _transceiver.Destroy();
                            break;
                        }
                }
            }
            catch (System.Exception ex)
            {
                _logger.Error("unexpected connection exception:\n" + ex + "\n" + _transceiver.ToString());
            }

            //
            // We only register with the connection monitor if our new state
            // is StateActive. Otherwise we unregister with the connection
            // monitor, but only if we were registered before, i.e., if our
            // old state was StateActive.
            //
            if (_monitor != null)
            {
                if (state == StateActive)
                {
                    if (_acmLastActivity > -1)
                    {
                        _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                    }
                    _monitor.Add(this);
                }
                else if (_state == StateActive)
                {
                    _monitor.Remove(this);
                }
            }

            if (_communicatorObserver != null)
            {
                ConnectionState oldState = ToConnectionState(_state);
                ConnectionState newState = ToConnectionState(state);
                if (oldState != newState)
                {
                    _observer = _communicatorObserver.GetConnectionObserver(InitConnectionInfo(), _endpoint,
                        newState, _observer);
                    if (_observer != null)
                    {
                        _observer.Attach();
                    }
                }
                if (_observer != null && state == StateClosed && _exception != null)
                {
                    if (!(_exception is ConnectionClosedException ||
                         _exception is ConnectionIdleException ||
                         _exception is CommunicatorDestroyedException ||
                         _exception is ObjectAdapterDeactivatedException ||
                         (_exception is ConnectionLostException && _state >= StateClosing)))
                    {
                        _observer.Failed(_exception.GetType().FullName!);
                    }
                }
            }
            _state = state;

            System.Threading.Monitor.PulseAll(this);
            if (_state == StateClosing && _dispatchCount == 0)
            {
                try
                {
                    InitiateShutdown();
                }
                catch (Exception ex)
                {
                    SetState(StateClosed, ex);
                }
            }
        }

        private void InitiateShutdown()
        {
            Debug.Assert(_state == StateClosing && _dispatchCount == 0);

            if (_shutdownInitiated)
            {
                return;
            }
            _shutdownInitiated = true;

            if (!_endpoint.IsDatagram)
            {
                //
                // Before we shut down, we send a close connection message.
                //
                if ((SendMessage(new OutgoingMessage(_closeConnectionMessage, false)) &
                    OutgoingAsyncBase.AsyncStatusSent) != 0)
                {
                    SetState(StateClosingPending);

                    //
                    // Notify the transceiver of the graceful connection closure.
                    //
                    int op = _transceiver.Closing(true, _exception);
                    if (op != 0)
                    {
                        ScheduleTimeout(op);
                        ThreadPool.Register(this, op);
                    }
                }
            }
        }

        private void SendHeartbeatNow()
        {
            Debug.Assert(_state == StateActive);

            if (!_endpoint.IsDatagram)
            {
                try
                {
                    SendMessage(new OutgoingMessage(_validateConnectionMessage, false));
                }
                catch (System.Exception ex)
                {
                    SetState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                }
            }
        }

        private bool Initialize(int operation)
        {
            int s = _transceiver.Initialize(ref _readBuffer, _writeBuffer);
            if (s != SocketOperation.None)
            {
                _writeBufferOffset = 0;
                _writeBufferSize = _writeBuffer.GetByteCount();
                ScheduleTimeout(s);
                ThreadPool.Update(this, operation, s);
                return false;
            }
            //
            // Update the connection description once the transceiver is initialized.
            //
            _desc = _transceiver.ToString()!;
            _initialized = true;
            SetState(StateNotValidated);
            return true;
        }

        private bool Validate(int operation)
        {
            if (!_endpoint.IsDatagram) // Datagram connections are always implicitly validated.
            {
                if (_adapter != null) // The server side has the active role for connection validation.
                {
                    if (_writeBufferSize == 0)
                    {
                        _writeBuffer = _validateConnectionMessage;
                        _writeBufferOffset = 0;
                        _writeBufferSize = Ice1Definitions.HeaderSize;
                        // TODO we need a better API for tracing
                        TraceUtil.TraceSend(_communicator,
                            _writeBuffer.GetSegment(0, _writeBufferSize).ToArray(), _logger, _traceLevels);
                    }

                    if (_writeBufferOffset < _writeBufferSize)
                    {
                        int op = Write(_writeBuffer, _writeBufferSize, ref _writeBufferOffset);
                        if (op != 0)
                        {
                            ScheduleTimeout(op);
                            ThreadPool.Update(this, operation, op);
                            return false;
                        }
                    }
                }
                else // The client side has the passive role for connection validation.
                {
                    if (_readBuffer.Count == 0)
                    {
                        _readBuffer = new ArraySegment<byte>(new byte[256], 0, Ice1Definitions.HeaderSize);
                        _readBufferOffset = 0;
                    }

                    if (_readBufferOffset < _readBuffer.Count)
                    {
                        int op = Read(ref _readBuffer, ref _readBufferOffset);
                        if (op != 0)
                        {
                            ScheduleTimeout(op);
                            ThreadPool.Update(this, operation, op);
                            return false;
                        }
                    }

                    _validated = true;

                    Debug.Assert(_readBufferOffset == Ice1Definitions.HeaderSize);
                    Ice1Definitions.CheckHeader(_readBuffer.AsSpan(0, 8));
                    var messageType = (Ice1Definitions.MessageType)_readBuffer[8];
                    if (messageType != Ice1Definitions.MessageType.ValidateConnectionMessage)
                    {
                        throw new InvalidDataException(@$"received ice1 frame with message type `{messageType
                            }' before receiving the validate connection message");
                    }

                    int size = InputStream.ReadInt(_readBuffer.AsSpan(10, 4));
                    if (size != Ice1Definitions.HeaderSize)
                    {
                        throw new InvalidDataException(
                            $"received an ice1 frame with validate connection type and a size of `{size}' bytes");
                    }
                }
            }

            _writeBuffer = _emptyBuffer;
            _writeBufferSize = 0;
            _writeBufferOffset = 0;

            // For datagram connections the buffer is allocated by the datagram transport
            if (!_endpoint.IsDatagram)
            {
                _readBuffer = new ArraySegment<byte>(new byte[256], 0, Ice1Definitions.HeaderSize);
            }
            _readBufferOffset = 0;
            _readHeader = true;

            if (_communicator.TraceLevels.Network >= 1)
            {
                var s = new StringBuilder();
                if (_endpoint.IsDatagram)
                {
                    s.Append("starting to ");
                    s.Append(_connector != null ? "send" : "receive");
                    s.Append(" ");
                    s.Append(_endpoint.Name);
                    s.Append(" messages\n");
                    s.Append(_transceiver.ToDetailedString());
                }
                else
                {
                    s.Append(_connector != null ? "established" : "accepted");
                    s.Append(" ");
                    s.Append(_endpoint.Name);
                    s.Append(" connection\n");
                    s.Append(ToString());
                }
                _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
            }

            return true;
        }

        private int SendNextMessage(out Queue<OutgoingMessage>? callbacks)
        {
            callbacks = null;

            if (_outgoingMessages.Count == 0)
            {
                return SocketOperation.None;
            }
            else if (_state == StateClosingPending && _writeBufferOffset == 0)
            {
                // Message wasn't sent, empty the _writeStream, we're not going to send more data.
                _writeBuffer = _emptyBuffer;
                _writeBufferOffset = 0;
                _writeBufferSize = 0;
                return SocketOperation.None;
            }

            Debug.Assert(_writeBufferSize > 0 && _writeBufferOffset == _writeBufferSize);
            try
            {
                while (true)
                {
                    //
                    // Notify the message that it was sent.
                    //
                    OutgoingMessage message = _outgoingMessages.First!.Value;
                    _writeBuffer = _emptyBuffer;
                    _writeBufferOffset = 0;
                    _writeBufferSize = 0;
                    if (message.Sent())
                    {
                        if (callbacks == null)
                        {
                            callbacks = new Queue<OutgoingMessage>();
                        }
                        callbacks.Enqueue(message);
                    }
                    _outgoingMessages.RemoveFirst();

                    //
                    // If there's nothing left to send, we're done.
                    //
                    if (_outgoingMessages.Count == 0)
                    {
                        break;
                    }

                    //
                    // If we are in the closed state or if the close is
                    // pending, don't continue sending.
                    //
                    // This can occur if ParseMessage (called before
                    // SendNextMessage by Message()) closes the connection.
                    //
                    if (_state >= StateClosingPending)
                    {
                        return SocketOperation.None;
                    }

                    //
                    // Otherwise, prepare the next message stream for writing.
                    //
                    message = _outgoingMessages.First.Value;
                    Debug.Assert(message.OutgoingData != null);
                    List<ArraySegment<byte>> data = message.OutgoingData;

                    message.OutgoingData = DoCompress(message.OutgoingData, message.Size, message.Compress);
                    message.Size = message.OutgoingData.GetByteCount();

                    TraceUtil.TraceSend(_communicator, data.GetSegment(0, message.Size).Array!, _logger, _traceLevels);
                    _writeBuffer = message.OutgoingData;
                    _writeBufferSize = message.Size;
                    _writeBufferOffset = 0;
                    //
                    // Send the message.
                    //
                    if (_writeBufferOffset < _writeBufferSize)
                    {
                        int op = Write(_writeBuffer, _writeBufferSize, ref _writeBufferOffset);
                        if (op != 0)
                        {
                            return op;
                        }
                    }
                }

                //
                // If all the messages were sent and we are in the closing state, we schedule
                // the close timeout to wait for the peer to close the connection.
                //
                if (_state == StateClosing && _shutdownInitiated)
                {
                    SetState(StateClosingPending);
                    int op = _transceiver.Closing(true, _exception);
                    if (op != 0)
                    {
                        return op;
                    }
                }
            }
            catch (System.Exception ex)
            {
                SetState(StateClosed, ex);
            }
            return SocketOperation.None;
        }

        private int SendMessage(OutgoingMessage message)
        {
            Debug.Assert(_state < StateClosed);
            if (_outgoingMessages.Count > 0)
            {
                _outgoingMessages.AddLast(message);
                return OutgoingAsyncBase.AsyncStatusQueued;
            }

            //
            // Attempt to send the message without blocking. If the send blocks, we use
            // asynchronous I/O or we request the caller to call FinishSendMessage() outside
            // the synchronization.
            //
            Debug.Assert(message.OutgoingData != null);
            List<ArraySegment<byte>> requestData = message.OutgoingData;

            message.OutgoingData = DoCompress(requestData, message.Size, message.Compress);
            message.Size = message.OutgoingData.GetByteCount();
            _writeBuffer = message.OutgoingData;
            _writeBufferSize = message.Size;
            _writeBufferOffset = 0;

            TraceUtil.TraceSend(_communicator, VectoredBufferExtensions.ToArray(requestData), _logger, _traceLevels);

            //
            // Send the message without blocking.
            //
            int op = Write(_writeBuffer, _writeBufferSize, ref _writeBufferOffset);
            if (op == 0)
            {
                _writeBuffer = _emptyBuffer;
                _writeBufferSize = 0;
                _writeBufferOffset = 0;
                int status = OutgoingAsyncBase.AsyncStatusSent;
                if (message.Sent())
                {
                    status |= OutgoingAsyncBase.AsyncStatusInvokeSentCallback;
                }

                if (_acmLastActivity > -1)
                {
                    _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                }
                return status;
            }
            _outgoingMessages.AddLast(message);
            ScheduleTimeout(op);
            ThreadPool.Register(this, op);
            return OutgoingAsyncBase.AsyncStatusQueued;
        }

        private List<ArraySegment<byte>> DoCompress(List<ArraySegment<byte>> data, int size, bool compress)
        {
            if (BZip2.IsLoaded && compress && size >= 100)
            {
                List<ArraySegment<byte>>? compressedData =
                    BZip2.Compress(data, size, Ice1Definitions.HeaderSize, _compressionLevel);
                if (compressedData != null)
                {
                    return compressedData;
                }
            }

            ArraySegment<byte> header = data[0];
            // Write the compression status and the message size.
            header[9] = (byte)(BZip2.IsLoaded && compress ? 1 : 0);
            return data;
        }

        private struct MessageInfo
        {
            public ArraySegment<byte> Data;
            public int InvokeNum;
            public int RequestId;
            public byte Compress;
            public ObjectAdapter? Adapter;
            public OutgoingAsyncBase? OutAsync;
            public HeartbeatCallback HeartbeatCallback;
            public int MessageDispatchCount;
        }

        private int ParseMessage(ref MessageInfo info)
        {
            Debug.Assert(_state > StateNotValidated && _state < StateClosed);
            info.Data = _readBuffer;

            // For datagram connections the buffer is allocated by the datagram transport
            _readBuffer = _endpoint.IsDatagram ?
                ArraySegment<byte>.Empty : new ArraySegment<byte>(new byte[256], 0, Ice1Definitions.HeaderSize);
            _readBufferOffset = 0;
            _readHeader = true;

            try
            {
                //
                // The magic and version fields have already been checked.
                //
                var messageType = (Ice1Definitions.MessageType)info.Data[8];
                info.Compress = info.Data[9];
                if (info.Compress == 2)
                {
                    if (BZip2.IsLoaded)
                    {
                        info.Data = BZip2.Decompress(info.Data, Ice1Definitions.HeaderSize, _messageSizeMax);
                    }
                    else
                    {
                        throw new LoadException("compression not supported, bzip2 library not found");
                    }
                }

                switch (messageType)
                {
                    case Ice1Definitions.MessageType.CloseConnectionMessage:
                        {
                            TraceUtil.TraceRecv(new InputStream(_communicator, info.Data), _logger, _traceLevels);
                            if (_endpoint.IsDatagram)
                            {
                                if (_warn)
                                {
                                    _logger.Warning("ignoring close connection message for datagram connection:\n" + _desc);
                                }
                            }
                            else
                            {
                                SetState(StateClosingPending, new ConnectionClosedByPeerException());

                                //
                                // Notify the transceiver of the graceful connection closure.
                                //
                                int op = _transceiver.Closing(false, _exception);
                                if (op != 0)
                                {
                                    return op;
                                }
                                SetState(StateClosed);
                            }
                            break;
                        }

                    case Ice1Definitions.MessageType.RequestMessage:
                        {
                            if (_state >= StateClosing)
                            {
                                TraceUtil.Trace("received request during closing\n" +
                                                "(ignored by server, client will retry)",
                                                new InputStream(_communicator, info.Data),
                                                _logger, _traceLevels);
                            }
                            else
                            {
                                TraceUtil.TraceRecv(new InputStream(_communicator, info.Data), _logger, _traceLevels);
                                info.RequestId = InputStream.ReadInt(info.Data.AsSpan(Ice1Definitions.HeaderSize, 4));
                                info.InvokeNum = 1;
                                info.Adapter = _adapter;
                                ++info.MessageDispatchCount;
                            }
                            break;
                        }

                    case Ice1Definitions.MessageType.RequestBatchMessage:
                        {
                            if (_state >= StateClosing)
                            {
                                TraceUtil.Trace("received batch request during closing\n" +
                                                "(ignored by server, client will retry)",
                                                new InputStream(_communicator, info.Data),
                                                _logger, _traceLevels);
                            }
                            else
                            {
                                TraceUtil.TraceRecv(new InputStream(_communicator, info.Data), _logger, _traceLevels);
                                info.InvokeNum = InputStream.ReadInt(info.Data.AsSpan(Ice1Definitions.HeaderSize, 4));
                                if (info.InvokeNum < 0)
                                {
                                    int invokeNum = info.InvokeNum;
                                    info.InvokeNum = 0;
                                    throw new InvalidDataException(
                                        $"received ice1 RequestBatchMessage with {invokeNum} batch requests");
                                }
                                info.Adapter = _adapter;
                                info.MessageDispatchCount += info.InvokeNum;
                            }
                            break;
                        }

                    case Ice1Definitions.MessageType.ReplyMessage:
                        {
                            TraceUtil.TraceRecv(new InputStream(_communicator, info.Data), _logger, _traceLevels);
                            info.RequestId = InputStream.ReadInt(info.Data.AsSpan(Ice1Definitions.HeaderSize, 4));
                            if (_asyncRequests.TryGetValue(info.RequestId, out info.OutAsync))
                            {
                                _asyncRequests.Remove(info.RequestId);

                                //
                                // If we just received the reply for a request which isn't acknowledge as
                                // sent yet, we queue the reply instead of processing it right away. It
                                // will be processed once the write callback is invoked for the message.
                                //
                                OutgoingMessage? message = _outgoingMessages.Count > 0 ? _outgoingMessages.First!.Value : null;
                                if (message != null && message.OutAsync == info.OutAsync)
                                {
                                    message.ReceivedReply = true;
                                    message.IncomingData = info.Data;
                                }
                                else if (info.OutAsync.Response(info.Data))
                                {
                                    ++info.MessageDispatchCount;
                                }
                                else
                                {
                                    info.OutAsync = null;
                                }
                                System.Threading.Monitor.PulseAll(this); // Notify threads blocked in close()
                            }
                            break;
                        }

                    case Ice1Definitions.MessageType.ValidateConnectionMessage:
                        {
                            TraceUtil.TraceRecv(new InputStream(_communicator, info.Data), _logger, _traceLevels);
                            if (_heartbeatCallback != null)
                            {
                                info.HeartbeatCallback = _heartbeatCallback;
                                ++info.MessageDispatchCount;
                            }
                            break;
                        }

                    default:
                        {
                            TraceUtil.Trace("received unknown message\n(invalid, closing connection)",
                                            new InputStream(_communicator, info.Data), _logger, _traceLevels);
                            throw new InvalidDataException(
                                $"received ice1 frame with unknown message type `{messageType}'");
                        }
                }
            }
            catch (Exception ex)
            {
                if (_endpoint.IsDatagram)
                {
                    if (_warn)
                    {
                        _logger.Warning("datagram connection exception:\n" + ex.ToString() + "\n" + _desc);
                    }
                }
                else
                {
                    SetState(StateClosed, ex);
                }
            }

            return SocketOperation.Read;
        }

        private async ValueTask InvokeAllAsync(ArraySegment<byte> data, int invokeNum, int requestId,
            byte compressionStatus, ObjectAdapter adapter)
        {
            // Note: In contrast to other private or protected methods, this method must be called *without* the
            // mutex locked.

            Debug.Assert(invokeNum > 0); // invokeNum is usually 1 but can be larger for a batch request.
            Debug.Assert(invokeNum == 1); // TODO: deal with batch requests

            IDispatchObserver? dispatchObserver = null;

            try
            {
                var request = new IncomingRequestFrame(adapter.Communicator,
                    data.Slice(Ice1Definitions.HeaderSize + 4));
                var current = new Current(adapter, request, requestId, this);

                // Then notify and set dispatch observer, if any.
                ICommunicatorObserver? communicatorObserver = adapter.Communicator.Observer;
                if (communicatorObserver != null)
                {
                    dispatchObserver = communicatorObserver.GetDispatchObserver(current, request.Size);
                    dispatchObserver?.Attach();
                }

                OutgoingResponseFrame? response = null;
                try
                {
                    IObject? servant = current.Adapter.Find(current.Identity, current.Facet);
                    if (servant == null)
                    {
                        // TODO if we want to support incoming batch request we need
                        // to skip current encapsulation.
                        throw new ObjectNotExistException(current.Identity, current.Facet, current.Operation);
                    }

                    ValueTask<OutgoingResponseFrame> vt = servant.DispatchAsync(request, current);
                    --invokeNum;
                    if (requestId != 0)
                    {
                        response = await vt.ConfigureAwait(false);
                    }
                }
                catch (Exception ex)
                {
                    if (requestId != 0)
                    {
                        RemoteException actualEx;
                        if (ex is RemoteException remoteEx && !remoteEx.ConvertToUnhandled)
                        {
                            actualEx = remoteEx;
                        }
                        else
                        {
                            actualEx = new UnhandledException(current.Identity, current.Facet, current.Operation, ex);
                        }
                        Incoming.ReportException(actualEx, dispatchObserver, current);
                        response = new OutgoingResponseFrame(current, actualEx);
                    }
                }

                if (requestId == 0)
                {
                    SendNoResponse();
                }
                else
                {
                    Debug.Assert(response != null);
                    dispatchObserver?.Reply(response.Size);
                    SendResponse(response, requestId, compressionStatus);
                }
            }
            catch (Exception ex)
            {
                InvokeException(ex, invokeNum);
            }
            finally
            {
                dispatchObserver?.Detach();
            }
        }

        private void ScheduleTimeout(int status)
        {
            int timeout;
            if (_state < StateActive)
            {
                timeout = _communicator.OverrideConnectTimeout ?? _endpoint.Timeout;
            }
            else if (_state < StateClosingPending)
            {
                if (_readHeader) // No timeout for reading the header.
                {
                    status &= ~SocketOperation.Read;
                }
                timeout = _endpoint.Timeout;
            }
            else
            {
                timeout = _communicator.OverrideCloseTimeout ?? _endpoint.Timeout;
            }

            if (timeout < 0)
            {
                return;
            }

            if ((status & SocketOperation.Read) != 0)
            {
                if (_readTimeoutScheduled)
                {
                    _timer.Cancel(_readTimeout);
                }
                _timer.Schedule(_readTimeout, timeout);
                _readTimeoutScheduled = true;
            }
            if ((status & (SocketOperation.Write | SocketOperation.Connect)) != 0)
            {
                if (_writeTimeoutScheduled)
                {
                    _timer.Cancel(_writeTimeout);
                }
                _timer.Schedule(_writeTimeout, timeout);
                _writeTimeoutScheduled = true;
            }
        }

        private void UnscheduleTimeout(int status)
        {
            if ((status & SocketOperation.Read) != 0 && _readTimeoutScheduled)
            {
                _timer.Cancel(_readTimeout);
                _readTimeoutScheduled = false;
            }
            if ((status & (SocketOperation.Write | SocketOperation.Connect)) != 0 &&
               _writeTimeoutScheduled)
            {
                _timer.Cancel(_writeTimeout);
                _writeTimeoutScheduled = false;
            }
        }

        private ConnectionInfo InitConnectionInfo()
        {
            if (_state > StateNotInitialized && _info != null) // Update the connection info until it's initialized
            {
                return _info;
            }

            try
            {
                _info = _transceiver.GetInfo();
            }
            catch (System.Exception)
            {
                _info = new ConnectionInfo();
            }
            for (ConnectionInfo? info = _info; info != null; info = info.Underlying)
            {
                info.ConnectionId = _endpoint.ConnectionId;
                info.AdapterName = _adapter != null ? _adapter.Name : "";
                info.Incoming = _connector == null;
            }
            return _info;
        }

        private void Reap()
        {
            if (_monitor != null)
            {
                _monitor.Reap(this);
            }
            if (_observer != null)
            {
                _observer.Detach();
            }
        }

        private ConnectionState ToConnectionState(int state) => _connectionStateMap[state];

        private void Warning(string msg, System.Exception ex) => _logger.Warning($"{msg}:\n{ex}\n{_transceiver}");

        private int Read(ref ArraySegment<byte> buffer, ref int offset)
        {
            int start = offset;
            int op = _transceiver.Read(ref buffer, ref offset);
            if (start != offset)
            {
                TraceReceivedAndUpdateObserver(buffer.Count, start, offset);
            }
            return op;
        }

        private void TraceSentAndUpdateObserver(int length, int start, int end)
        {
            int remaining = length - start;
            int bytesTransferred = end - start;
            if (_communicator.TraceLevels.Network >= 3 && bytesTransferred > 0)
            {
                var s = new StringBuilder("sent ");
                s.Append(bytesTransferred);
                if (!_endpoint.IsDatagram)
                {
                    s.Append(" of ");
                    s.Append(remaining);
                }
                s.Append(" bytes via ");
                s.Append(_endpoint.Name);
                s.Append("\n");
                s.Append(ToString());
                _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
            }

            if (_observer != null && bytesTransferred > 0)
            {
                _observer.SentBytes(bytesTransferred);
            }
        }

        private void TraceReceivedAndUpdateObserver(int length, int start, int end)
        {
            int remaining = length - start;
            int bytesTransferred = end - start;

            if (_communicator.TraceLevels.Network >= 3 && bytesTransferred > 0)
            {
                var s = new StringBuilder("received ");
                if (_endpoint.IsDatagram)
                {
                    s.Append(remaining);
                }
                else
                {
                    s.Append(bytesTransferred);
                    s.Append(" of ");
                    s.Append(remaining);
                }
                s.Append(" bytes via ");
                s.Append(_endpoint.Name);
                s.Append("\n");
                s.Append(ToString());
                _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
            }

            if (_observer != null && bytesTransferred > 0)
            {
                _observer.ReceivedBytes(bytesTransferred);
            }
        }

        private int Write(IList<ArraySegment<byte>> buffer, int size, ref int offset)
        {
            int start = offset;
            int socketOperation = _transceiver.Write(buffer, ref offset);
            if (start != offset)
            {
                TraceSentAndUpdateObserver(size, start, offset);
            }
            return socketOperation;
        }

        private class OutgoingMessage
        {
            internal OutgoingMessage(List<ArraySegment<byte>> requestData, bool compress)
            {
                OutgoingData = requestData;
                Size = OutgoingData.GetByteCount();
                Compress = compress;
            }

            internal OutgoingMessage(OutgoingAsyncBase outgoing, List<ArraySegment<byte>> data, bool compress, int requestId)
            {
                OutAsync = outgoing;
                OutgoingData = data;
                Size = OutgoingData.GetByteCount();
                Compress = compress;
                RequestId = requestId;
            }

            internal OutgoingMessage(OutgoingResponseFrame frame, bool compress, int requestId)
            {
                OutgoingData = Ice1Definitions.GetResponseData(frame, requestId);
                Size = OutgoingData.GetByteCount();
                Compress = compress;
            }

            internal void Canceled()
            {
                Debug.Assert(OutAsync != null); // Only requests can timeout.
                OutAsync = null;
            }

            internal bool Sent()
            {
                OutgoingData = null;
                if (OutAsync != null)
                {
                    InvokeSent = OutAsync.Sent();
                    return InvokeSent || ReceivedReply;
                }
                return false;
            }

            internal void Completed(Exception ex)
            {
                if (OutAsync != null)
                {
                    if (OutAsync.Exception(ex))
                    {
                        OutAsync.InvokeException();
                    }
                }
                OutgoingData = null;
            }

            internal List<ArraySegment<byte>>? OutgoingData;
            internal ArraySegment<byte> IncomingData;
            internal int Size;
            internal OutgoingAsyncBase? OutAsync;
            internal bool Compress;
            internal int RequestId;
            internal bool IsSent;
            internal bool InvokeSent;
            internal bool ReceivedReply;
        }

        private readonly Communicator _communicator;
        private IACMMonitor? _monitor;
        private readonly ITransceiver _transceiver;
        private string _desc;
        private readonly string _type;
        private readonly IConnector? _connector;
        private readonly Endpoint _endpoint;

        private ObjectAdapter? _adapter;

        private readonly ILogger _logger;
        private readonly TraceLevels _traceLevels;
        private readonly IceInternal.Timer _timer;
        private readonly ITimerTask _writeTimeout;
        private bool _writeTimeoutScheduled;
        private readonly ITimerTask _readTimeout;
        private bool _readTimeoutScheduled;

        private IStartCallback? _startCallback = null;

        private readonly bool _warn;
        private readonly bool _warnUdp;

        private long _acmLastActivity;

        private readonly int _compressionLevel;

        private int _nextRequestId;

        private readonly Dictionary<int, OutgoingAsyncBase> _asyncRequests = new Dictionary<int, OutgoingAsyncBase>();

        private System.Exception? _exception;

        private readonly int _messageSizeMax;

        private readonly LinkedList<OutgoingMessage> _outgoingMessages = new LinkedList<OutgoingMessage>();

        private ArraySegment<byte> _readBuffer;
        private int _readBufferOffset;
        private bool _readHeader;

        private IList<ArraySegment<byte>> _writeBuffer;
        private int _writeBufferOffset;
        private int _writeBufferSize;

        private ICommunicatorObserver? _communicatorObserver;
        private IConnectionObserver? _observer;

        private int _dispatchCount;

        private int _state; // The current state.
        private bool _shutdownInitiated = false;
        private bool _initialized = false;
        private bool _validated = false;

        private ConnectionInfo? _info;

        private CloseCallback? _closeCallback;
        private HeartbeatCallback? _heartbeatCallback;

        private static readonly ConnectionState[] _connectionStateMap = new ConnectionState[]
        {
            ConnectionState.ConnectionStateValidating,   // StateNotInitialized
            ConnectionState.ConnectionStateValidating,   // StateNotValidated
            ConnectionState.ConnectionStateActive,       // StateActive
            ConnectionState.ConnectionStateClosing,      // StateClosing
            ConnectionState.ConnectionStateClosing,      // StateClosingPending
            ConnectionState.ConnectionStateClosed,       // StateClosed
            ConnectionState.ConnectionStateClosed,       // StateFinished
        };

        private static readonly List<ArraySegment<byte>> _emptyBuffer = new List<ArraySegment<byte>>();
        private static readonly List<ArraySegment<byte>> _validateConnectionMessage =
            new List<ArraySegment<byte>> { Ice1Definitions.ValidateConnectionMessage };
        private static readonly List<ArraySegment<byte>> _closeConnectionMessage =
            new List<ArraySegment<byte>> { Ice1Definitions.CloseConnectionMessage };
    }
}
