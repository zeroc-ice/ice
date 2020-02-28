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

        public override bool Equals(object other) =>
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
            void ConnectionStartFailed(Connection connection, LocalException ex);
        }

        private class TimeoutCallback : ITimerTask
        {
            public TimeoutCallback(Connection connection) => _connection = connection;

            public void RunTimerTask() => _connection.TimedOut();

            private readonly Connection _connection;
        }

        public void Start(IStartCallback callback)
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

                    //
                    // We start out in holding state.
                    //
                    SetState(StateHolding);
                }
            }
            catch (LocalException ex)
            {
                Exception(ex);
                Debug.Assert(_exception != null);
                callback.ConnectionStartFailed(this, _exception);
                return;
            }

            callback.ConnectionStartCompleted(this);
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

                    //
                    // We start out in holding state.
                    //
                    SetState(StateHolding);
                }
            }
            catch (LocalException ex)
            {
                Exception(ex);
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

        internal void Hold()
        {
            lock (this)
            {
                if (_state <= StateNotValidated)
                {
                    return;
                }

                SetState(StateHolding);
            }
        }

        // DestructionReason.
        public const int ObjectAdapterDeactivated = 0;
        public const int CommunicatorDestroyed = 1;

        internal void Destroy(int reason)
        {
            lock (this)
            {
                switch (reason)
                {
                    case ObjectAdapterDeactivated:
                        {
                            SetState(StateClosing, new ObjectAdapterDeactivatedException());
                            break;
                        }

                    case CommunicatorDestroyed:
                        {
                            SetState(StateClosing, new CommunicatorDestroyedException());
                            break;
                        }
                }
            }
        }

        /// <summary>
        /// Manually close the connection using the specified closure mode.
        /// </summary>
        /// <param name="mode">Determines how the connection will be closed.
        ///
        /// </param>
        public void Close(ConnectionClose mode)
        {
            lock (this)
            {
                if (mode == ConnectionClose.Forcefully)
                {
                    SetState(StateClosed, new ConnectionManuallyClosedException(false));
                }
                else if (mode == ConnectionClose.Gracefully)
                {
                    SetState(StateClosing, new ConnectionManuallyClosedException(true));
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

                    SetState(StateClosing, new ConnectionManuallyClosedException(true));
                }
            }
        }

        internal bool ActiveOrHolding
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
        /// CloseConnectionException is raised if the connection was closed gracefully,
        /// whereas ConnectionManuallyClosedException is raised if the connection was
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

        internal void WaitUntilHolding()
        {
            lock (this)
            {
                while (_state < StateHolding || _dispatchCount > 0)
                {
                    System.Threading.Monitor.Wait(this);
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
                else
                {
                    _readStreamPos = -1;
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

                if (_readStream.Size > Protocol.headerSize || _writeBufferSize > 0)
                {
                    //
                    // If writing or reading, nothing to do, the connection
                    // timeout will kick-in if writes or reads don't progress.
                    // This check is necessary because the actitivy timer is
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
                        SetState(StateClosing, new ConnectionTimeoutException());
                    }
                }
            }
        }

        internal int SendAsyncRequest(OutgoingAsyncBase og, bool compress, bool response)
        {
            OutputStream os = og.GetOs();
            lock (this)
            {
                //
                // If the exception is closed before we even have a chance
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
                // Ensure the message isn't bigger than what we can send with the
                // transport.
                //
                _transceiver.CheckSendSize(os.Size);

                //
                // Notify the request that it's cancelable with this connection.
                // This will throw if the request is canceled.
                //
                og.Cancelable(this);
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
                    os.RewriteInt(requestId, new OutputStream.Position(0, Protocol.headerSize));
                }

                og.AttachRemoteObserver(InitConnectionInfo(), _endpoint, requestId);

                int status = OutgoingAsyncBase.AsyncStatusQueued;
                try
                {
                    var message = new OutgoingMessage(og, os, compress, requestId);
                    status = SendMessage(message);
                }
                catch (LocalException ex)
                {
                    SetState(StateClosed, ex);
                    Debug.Assert(_exception != null);
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
                            catch (System.Exception ex)
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

            public void Invoke()
            {
                try
                {
                    Debug.Assert(Os != null);
                    Os.WriteSpan(Protocol.magic.AsSpan());
                    Os.WriteByte(Util.CurrentProtocol.Major);
                    Os.WriteByte(Util.CurrentProtocol.Minor);
                    Os.WriteByte(Util.CurrentProtocolEncoding.Major);
                    Os.WriteByte(Util.CurrentProtocolEncoding.Minor);
                    Os.WriteByte(Protocol.validateConnectionMsg);
                    Os.WriteByte(0);
                    Os.WriteInt(Protocol.headerSize); // Message size.

                    int status = _connection.SendAsyncRequest(this, false, false);

                    if ((status & AsyncStatusSent) != 0)
                    {
                        sentSynchronously_ = true;
                        if ((status & AsyncStatusInvokeSentCallback) != 0)
                        {
                            InvokeSent();
                        }
                    }
                }
                catch (RetryException ex)
                {
                    try
                    {
                        throw ex.Get();
                    }
                    catch (LocalException ee)
                    {
                        if (Exception(ee))
                        {
                            InvokeExceptionAsync();
                        }
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
                    throw new ArgumentException("invalid negative ACM timeout value");
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

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex)
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

                OutgoingMessage? o = _sendStreams.FirstOrDefault(m => m.OutAsync == outAsync);
                if (o != null)
                {
                    if (o.RequestId > 0)
                    {
                        _asyncRequests.Remove(o.RequestId);
                    }

                    if (ex is ConnectionTimeoutException)
                    {
                        SetState(StateClosed, ex);
                    }
                    else
                    {
                        //
                        // If the request is being sent, don't remove it from the send streams,
                        // it will be removed once the sending is finished.
                        //
                        if (o == _sendStreams.First.Value)
                        {
                            o.Canceled();
                        }
                        else
                        {
                            o.Canceled();
                            _sendStreams.Remove(o);
                        }
                        if (outAsync.Exception(ex))
                        {
                            outAsync.InvokeExceptionAsync();
                        }
                    }
                    return;
                }

                if (outAsync is OutgoingAsync)
                {
                    foreach (KeyValuePair<int, OutgoingAsyncBase> kvp in _asyncRequests)
                    {
                        if (kvp.Value == outAsync)
                        {
                            if (ex is ConnectionTimeoutException)
                            {
                                SetState(StateClosed, ex);
                            }
                            else
                            {
                                _asyncRequests.Remove(kvp.Key);
                                if (outAsync.Exception(ex))
                                {
                                    outAsync.InvokeExceptionAsync();
                                }
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
        public IEndpoint Endpoint => _endpoint; // No mutex protection necessary, _endpoint is immutable.

        /// <summary>Creates a special "fixed" proxy that always uses this connection. This proxy can be used for
        /// callbacks from a server to a client if the server cannot directly establish a connection to the client,
        /// for example because of firewalls. In this case, the server would create a proxy using an already
        /// established connection from the client.</summary>
        /// <param name="identity">The identity for which a proxy is to be created.</param>
        /// <param name="factory">The proxy facetory. Use INamePrx.Factory, where INamePrx is the desired proxy type.
        /// </param>
        /// <returns>A proxy that matches the given identity and uses this connection.</returns>
        public T CreateProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => factory(_communicator.CreateReference(identity, this));

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
                    if (completed && _sendStreams.Count > 0)
                    {
                        // The whole message is written, assume it's sent now for at-most-once semantics.
                        _sendStreams.First.Value.IsSent = true;
                    }
                }
                else if ((operation & SocketOperation.Read) != 0)
                {
                    if (_observer != null && !_readHeader)
                    {
                        ObserverStartRead(_readStream.GetBuffer());
                    }

                    completedSynchronously = _transceiver.StartRead(_readStream.GetBuffer(), cb, this);
                }
            }
            catch (LocalException ex)
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
                    int remaining = _writeBufferSize - _writeBufferOffset;
                    _transceiver.FinishWrite(_writeBuffer, ref _writeBufferOffset);
                    int bytesTransferred = remaining - (_writeBufferSize - _writeBufferOffset);
                    if (_communicator.TraceLevels.Network >= 3 && bytesTransferred > 0)
                    {
                        var s = new StringBuilder("sent ");
                        s.Append(bytesTransferred);
                        if (!_endpoint.Datagram())
                        {
                            s.Append(" of ");
                            s.Append(remaining);
                        }
                        s.Append(" bytes via ");
                        s.Append(_endpoint.Transport());
                        s.Append("\n");
                        s.Append(ToString());
                        _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
                    }

                    if (_observer != null && bytesTransferred > 0)
                    {
                        _observer.SentBytes(bytesTransferred);
                    }
                }
                else if ((operation & SocketOperation.Read) != 0)
                {
                    IceInternal.Buffer buf = _readStream.GetBuffer();
                    int start = buf.B.Position();
                    _transceiver.FinishRead(buf);
                    if (_communicator.TraceLevels.Network >= 3 && buf.B.Position() != start)
                    {
                        var s = new StringBuilder("received ");
                        if (_endpoint.Datagram())
                        {
                            s.Append(buf.B.Limit());
                        }
                        else
                        {
                            s.Append(buf.B.Position() - start);
                            s.Append(" of ");
                            s.Append(buf.B.Limit() - start);
                        }
                        s.Append(" bytes via ");
                        s.Append(_endpoint.Transport());
                        s.Append("\n");
                        s.Append(ToString());
                        _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
                    }

                    if (_observer != null && !_readHeader)
                    {
                        ObserverFinishRead(_readStream.GetBuffer());
                    }
                }
            }
            catch (LocalException ex)
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
                            IceInternal.Buffer buf = _readStream.GetBuffer();

                            if (_observer != null && !_readHeader)
                            {
                                ObserverStartRead(buf);
                            }

                            readOp = Read(buf);
                            if ((readOp & SocketOperation.Read) != 0)
                            {
                                break;
                            }
                            if (_observer != null && !_readHeader)
                            {
                                Debug.Assert(!buf.B.HasRemaining());
                                ObserverFinishRead(buf);
                            }

                            if (_readHeader) // Read header if necessary.
                            {
                                _readHeader = false;

                                if (_observer != null)
                                {
                                    _observer.ReceivedBytes(Protocol.headerSize);
                                }

                                //
                                // Connection is validated on first message. This is only used by
                                // setState() to check wether or not we can print a connection
                                // warning (a client might close the connection forcefully if the
                                // connection isn't validated, we don't want to print a warning
                                // in this case).
                                //
                                _validated = true;

                                int pos = _readStream.Pos;
                                if (pos < Protocol.headerSize)
                                {
                                    //
                                    // This situation is possible for small UDP packets.
                                    //
                                    throw new IllegalMessageSizeException();
                                }

                                _readStream.Pos = 0;
                                byte[] m = new byte[4];
                                m[0] = _readStream.ReadByte();
                                m[1] = _readStream.ReadByte();
                                m[2] = _readStream.ReadByte();
                                m[3] = _readStream.ReadByte();
                                if (m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
                                    m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
                                {
                                    throw new BadMagicException
                                    {
                                        BadMagic = m
                                    };
                                }

                                byte major = _readStream.ReadByte();
                                byte minor = _readStream.ReadByte();
                                var pv = new ProtocolVersion(major, minor);
                                Protocol.checkSupportedProtocol(pv);
                                major = _readStream.ReadByte();
                                minor = _readStream.ReadByte();
                                var ev = new EncodingVersion(major, minor);
                                Protocol.checkSupportedProtocolEncoding(ev);

                                _readStream.ReadByte(); // messageType
                                _readStream.ReadByte(); // compress
                                int size = _readStream.ReadInt();
                                if (size < Protocol.headerSize)
                                {
                                    throw new IllegalMessageSizeException();
                                }

                                if (size > _messageSizeMax)
                                {
                                    Ex.ThrowMemoryLimitException(size, _messageSizeMax);
                                }
                                if (size > _readStream.Size)
                                {
                                    _readStream.Resize(size);
                                }
                                _readStream.Pos = pos;
                            }

                            if (buf.B.HasRemaining())
                            {
                                if (_endpoint.Datagram())
                                {
                                    throw new DatagramLimitException(); // The message was truncated.
                                }
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

                            //
                            // We start out in holding state.
                            //
                            SetState(StateHolding);
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
                    catch (DatagramLimitException) // Expected.
                    {
                        if (_warnUdp)
                        {
                            _logger.Warning(string.Format("maximum datagram size of {0} exceeded", _readStream.Pos));
                        }
                        _readStream.Resize(Protocol.headerSize);
                        _readStream.Pos = 0;
                        _readHeader = true;
                        return;
                    }
                    catch (SocketException ex)
                    {
                        SetState(StateClosed, ex);
                        return;
                    }
                    catch (LocalException ex)
                    {
                        if (_endpoint.Datagram())
                        {
                            if (_warn)
                            {
                                _logger.Warning(string.Format("datagram connection exception:\n{0}\n{1}", ex, _desc));
                            }
                            _readStream.Resize(Protocol.headerSize);
                            _readStream.Pos = 0;
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
                        if (outAsync.Response())
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
                catch (System.Exception ex)
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
                ValueTask vt = InvokeAllAsync(info.Stream, info.InvokeNum, info.RequestId, info.Compress,
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
                            catch (Ice.LocalException ex)
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
            // unecessary thread creation, especially if this is called on shutdown).
            //
            if (_startCallback == null && _sendStreams.Count == 0 && _asyncRequests.Count == 0 &&
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
                    s.Append(_endpoint.Transport());
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
                s.Append(_endpoint.Transport());
                s.Append(" connection\n");
                s.Append(ToString());

                //
                // Trace the cause of unexpected connection closures
                //
                if (!(_exception is CloseConnectionException ||
                      _exception is ConnectionManuallyClosedException ||
                      _exception is ConnectionTimeoutException ||
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

            if (_sendStreams.Count > 0)
            {
                if (_writeBufferSize > 0)
                {
                    // Return the stream to the outgoing call. This is important for
                    // retriable AMI calls which are not marshalled again.
                    OutgoingMessage message = _sendStreams.First.Value;
                    Debug.Assert(message.Stream != null);
                    _writeStream.Swap(message.Stream);
                    _writeBufferOffset = 0;
                    _writeBufferSize = 0;
                    _writeBuffer.Clear();

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
                            if (outAsync.Response())
                            {
                                outAsync.InvokeResponse();
                            }
                        }
                        _sendStreams.RemoveFirst();
                    }
                }

                foreach (OutgoingMessage o in _sendStreams)
                {
                    o.Completed(_exception!);
                    if (o.RequestId > 0) // Make sure finished isn't called twice.
                    {
                        _asyncRequests.Remove(o.RequestId);
                    }
                }
                _sendStreams.Clear(); // Must be cleared before _requests because of Outgoing* references in OutgoingMessage
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
            _readStream.Clear();
            _readStream.GetBuffer().Clear();

            if (_closeCallback != null)
            {
                try
                {
                    _closeCallback(this);
                }
                catch (System.Exception ex)
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

        private void SendResponse(OutputStream os, byte compressionStatus)
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

                    SendMessage(new OutgoingMessage(os, compressionStatus > 0));

                    if (_state == StateClosing && _dispatchCount == 0)
                    {
                        InitiateShutdown();
                    }
                }
                catch (LocalException ex)
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
                catch (LocalException ex)
                {
                    SetState(StateClosed, ex);
                }
            }
        }

        private void InvokeException(LocalException ex, int invokeNum)
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
                else if (_state < StateClosing)
                {
                    SetState(StateClosed, new TimeoutException());
                }
                else if (_state < StateClosed)
                {
                    SetState(StateClosed, new CloseTimeoutException());
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
        public int Timeout => _endpoint.Timeout(); // No mutex protection necessary, _endpoint is immutable.

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

        public void Exception(LocalException ex)
        {
            lock (this)
            {
                SetState(StateClosed, ex);
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
            _desc = transceiver.ToString();
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
            _warn = communicator.GetPropertyAsInt("Ice.Warn.Connections") > 0;
            _warnUdp = communicator.GetPropertyAsInt("Ice.Warn.Datagrams") > 0;
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
            _readStream = new InputStream(communicator, Util.CurrentProtocolEncoding);
            _readHeader = false;
            _readStreamPos = -1;
            _writeStream = new OutputStream(communicator, Util.CurrentProtocolEncoding);
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

            try
            {
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
            catch (LocalException)
            {
                throw;
            }
            catch (System.Exception ex)
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

        private void SetState(int state, LocalException ex)
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
                    if (!(_exception is CloseConnectionException ||
                         _exception is ConnectionManuallyClosedException ||
                         _exception is ConnectionTimeoutException ||
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
            if (_endpoint.Datagram() && state == StateClosing)
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
                            //
                            // Can only switch from holding or not validated to
                            // active.
                            //
                            if (_state != StateHolding && _state != StateNotValidated)
                            {
                                return;
                            }
                            ThreadPool.Register(this, SocketOperation.Read);
                            break;
                        }

                    case StateHolding:
                        {
                            //
                            // Can only switch from active or not validated to
                            // holding.
                            //
                            if (_state != StateActive && _state != StateNotValidated)
                            {
                                return;
                            }
                            if (_state == StateActive)
                            {
                                ThreadPool.Unregister(this, SocketOperation.Read);
                            }
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
            catch (LocalException ex)
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
                    else
                    {
                        _readStreamPos = -1;
                    }
                }
                if (_observer != null && state == StateClosed && _exception != null)
                {
                    if (!(_exception is CloseConnectionException ||
                         _exception is ConnectionManuallyClosedException ||
                         _exception is ConnectionTimeoutException ||
                         _exception is CommunicatorDestroyedException ||
                         _exception is ObjectAdapterDeactivatedException ||
                         (_exception is ConnectionLostException && _state >= StateClosing)))
                    {
                        _observer.Failed(_exception.ice_id());
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
                catch (LocalException ex)
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

            if (!_endpoint.Datagram())
            {
                //
                // Before we shut down, we send a close connection message.
                //
                var os = new OutputStream(_communicator, Util.CurrentProtocolEncoding);
                os.WriteSpan(Protocol.magic.AsSpan());
                os.WriteByte(Util.CurrentProtocol.Major);
                os.WriteByte(Util.CurrentProtocol.Minor);
                os.WriteByte(Util.CurrentProtocolEncoding.Major);
                os.WriteByte(Util.CurrentProtocolEncoding.Minor);
                os.WriteByte(Protocol.closeConnectionMsg);
                os.WriteByte(_compressionSupported ? (byte)1 : (byte)0);
                os.WriteInt(Protocol.headerSize); // Message size.

                if ((SendMessage(new OutgoingMessage(os, false)) & OutgoingAsyncBase.AsyncStatusSent) != 0)
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

            if (!_endpoint.Datagram())
            {
                var os = new OutputStream(_communicator, Util.CurrentProtocolEncoding);
                os.WriteBlob(Protocol.magic);
                os.WriteByte(Util.CurrentProtocol.Major);
                os.WriteByte(Util.CurrentProtocol.Minor);
                os.WriteByte(Util.CurrentProtocolEncoding.Major);
                os.WriteByte(Util.CurrentProtocolEncoding.Minor);
                os.WriteByte(Protocol.validateConnectionMsg);
                os.WriteByte(0);
                os.WriteInt(Protocol.headerSize); // Message size.
                try
                {
                    SendMessage(new OutgoingMessage(os, false));
                }
                catch (LocalException ex)
                {
                    SetState(StateClosed, ex);
                    Debug.Assert(_exception != null);
                }
            }
        }

        private bool Initialize(int operation)
        {
            int s = _transceiver.Initialize(_readStream.GetBuffer(), _writeBuffer, ref HasMoreData);
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
            _desc = _transceiver.ToString();
            _initialized = true;
            SetState(StateNotValidated);

            return true;
        }

        private bool Validate(int operation)
        {
            if (!_endpoint.Datagram()) // Datagram connections are always implicitly validated.
            {
                if (_adapter != null) // The server side has the active role for connection validation.
                {
                    if (_writeStream.Size == 0)
                    {
                        _writeStream.WriteSpan(Protocol.magic.AsSpan());
                        _writeStream.WriteByte(Util.CurrentProtocol.Major);
                        _writeStream.WriteByte(Util.CurrentProtocol.Minor);
                        _writeStream.WriteByte(Util.CurrentProtocolEncoding.Major);
                        _writeStream.WriteByte(Util.CurrentProtocolEncoding.Minor);
                        _writeStream.WriteByte(Protocol.validateConnectionMsg);
                        _writeStream.WriteByte(0); // Compression status (always zero for validate connection).
                        _writeStream.WriteInt(Protocol.headerSize); // Message size.
                        TraceUtil.TraceSend(_writeStream, _logger, _traceLevels);
                        _writeBuffer = _writeStream.GetUnderlyingBuffer();
                        _writeBufferOffset = 0;
                        _writeBufferSize = _writeStream.Size;
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
                    if (_readStream.Size == 0)
                    {
                        _readStream.Resize(Protocol.headerSize);
                        _readStream.Pos = 0;
                    }

                    if (_observer != null)
                    {
                        ObserverStartRead(_readStream.GetBuffer());
                    }

                    if (_readStream.Pos != _readStream.Size)
                    {
                        int op = Read(_readStream.GetBuffer());
                        if (op != 0)
                        {
                            ScheduleTimeout(op);
                            ThreadPool.Update(this, operation, op);
                            return false;
                        }
                    }

                    if (_observer != null)
                    {
                        ObserverFinishRead(_readStream.GetBuffer());
                    }

                    _validated = true;

                    Debug.Assert(_readStream.Pos == Protocol.headerSize);
                    _readStream.Pos = 0;
                    byte[] m = _readStream.ReadBlob(4);
                    if (m[0] != Protocol.magic[0] || m[1] != Protocol.magic[1] ||
                        m[2] != Protocol.magic[2] || m[3] != Protocol.magic[3])
                    {
                        throw new BadMagicException
                        {
                            BadMagic = m
                        };
                    }

                    byte major = _readStream.ReadByte();
                    byte minor = _readStream.ReadByte();
                    var pv = new ProtocolVersion(major, minor);
                    Protocol.checkSupportedProtocol(pv);
                    major = _readStream.ReadByte();
                    minor = _readStream.ReadByte();
                    var ev = new EncodingVersion(major, minor);
                    Protocol.checkSupportedProtocolEncoding(ev);

                    byte messageType = _readStream.ReadByte();
                    if (messageType != Protocol.validateConnectionMsg)
                    {
                        throw new ConnectionNotValidatedException();
                    }
                    _readStream.ReadByte(); // Ignore compression status for validate connection.
                    int size = _readStream.ReadInt();
                    if (size != Protocol.headerSize)
                    {
                        throw new IllegalMessageSizeException();
                    }
                    TraceUtil.TraceRecv(_readStream, _logger, _traceLevels);
                }
            }

            _writeStream.Reset();
            _writeBuffer.Clear();
            _writeBufferSize = 0;
            _writeBufferOffset = 0;

            _readStream.Resize(Protocol.headerSize);
            _readStream.Pos = 0;
            _readHeader = true;

            if (_communicator.TraceLevels.Network >= 1)
            {
                var s = new StringBuilder();
                if (_endpoint.Datagram())
                {
                    s.Append("starting to ");
                    s.Append(_connector != null ? "send" : "receive");
                    s.Append(" ");
                    s.Append(_endpoint.Transport());
                    s.Append(" messages\n");
                    s.Append(_transceiver.ToDetailedString());
                }
                else
                {
                    s.Append(_connector != null ? "established" : "accepted");
                    s.Append(" ");
                    s.Append(_endpoint.Transport());
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

            if (_sendStreams.Count == 0)
            {
                return SocketOperation.None;
            }
            else if (_state == StateClosingPending && _writeBufferOffset == 0)
            {
                // Message wasn't sent, empty the _writeStream, we're not going to send more data.
                OutgoingMessage message = _sendStreams.First.Value;
                _writeStream.Swap(message.Stream!);
                _writeBuffer.Clear();
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
                    OutgoingMessage message = _sendStreams.First.Value;
                    _writeStream.Swap(message.Stream!);
                    _writeBuffer.Clear();
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
                    _sendStreams.RemoveFirst();

                    //
                    // If there's nothing left to send, we're done.
                    //
                    if (_sendStreams.Count == 0)
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
                    if (_state >= StateClosingPending)
                    {
                        return SocketOperation.None;
                    }

                    //
                    // Otherwise, prepare the next message stream for writing.
                    //
                    message = _sendStreams.First.Value;
                    Debug.Assert(message.Stream != null);
                    OutputStream stream = message.Stream;

                    message.Stream = DoCompress(message.Stream, message.Compress);

                    TraceUtil.TraceSend(stream, _logger, _traceLevels);
                    _writeStream.Swap(message.Stream);
                    _writeBuffer = _writeStream.GetUnderlyingBuffer();
                    _writeBufferSize = _writeStream.Size;
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
            catch (LocalException ex)
            {
                SetState(StateClosed, ex);
            }
            return SocketOperation.None;
        }

        private int SendMessage(OutgoingMessage message)
        {
            Debug.Assert(_state < StateClosed);

            if (_sendStreams.Count > 0)
            {
                _sendStreams.AddLast(message);
                return OutgoingAsyncBase.AsyncStatusQueued;
            }

            //
            // Attempt to send the message without blocking. If the send blocks, we use
            // asynchronous I/O or we request the caller to call finishSendMessage() outside
            // the synchronization.
            //
            Debug.Assert(message.Stream != null);
            OutputStream stream = message.Stream;

            message.Stream = DoCompress(stream, message.Compress);
            _writeBuffer = message.Stream.GetUnderlyingBuffer();
            _writeBufferSize = message.Stream.Size;
            _writeBufferOffset = 0;

            TraceUtil.TraceSend(stream, _logger, _traceLevels);

            //
            // Send the message without blocking.
            //
            int op = Write(_writeBuffer, _writeBufferSize, ref _writeBufferOffset);
            if (op == 0)
            {
                _writeBuffer.Clear();
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

            _writeStream.Swap(message.Stream);
            _sendStreams.AddLast(message);
            ScheduleTimeout(op);
            ThreadPool.Register(this, op);
            return OutgoingAsyncBase.AsyncStatusQueued;
        }

        private OutputStream DoCompress(OutputStream uncompressed, bool compress)
        {
            if (_compressionSupported && compress && uncompressed.Size >= 100)
            {
                // Do compression.
                OutputStream? cstream = BZip2.Compress(uncompressed, Protocol.headerSize, _compressionLevel);
                if (cstream != null)
                {
                    // Write the compression status and the size of the compressed
                    // stream into the header.
                    cstream.RewriteByte(2, new OutputStream.Position(0, 9));
                    cstream.RewriteInt(cstream.Size, new OutputStream.Position(0, 10));

                    // Write the compression status and size of the compressed stream
                    // into the header of the uncompressed stream -- we need this to
                    // trace requests correctly.
                    uncompressed.RewriteByte(2, new OutputStream.Position(0, 9));
                    uncompressed.RewriteInt(cstream.Size, new OutputStream.Position(0, 10));
                    return cstream;
                }
            }

            // Write the compression status and the message size.
            uncompressed.RewriteByte((byte)(_compressionSupported && compress ? 1 : 0),
                new OutputStream.Position(0, 9));
            uncompressed.RewriteInt(uncompressed.Size, new OutputStream.Position(0, 10));
            return uncompressed;
        }

        private struct MessageInfo
        {
            public InputStream Stream;
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

            info.Stream = new InputStream(_communicator, Util.CurrentProtocolEncoding);
            _readStream.Swap(info.Stream);
            _readStream.Resize(Protocol.headerSize);
            _readStream.Pos = 0;
            _readHeader = true;

            Debug.Assert(info.Stream.Pos == info.Stream.Size);

            try
            {
                //
                // The magic and version fields have already been checked.
                //
                info.Stream.Pos = 8;
                byte messageType = info.Stream.ReadByte();
                info.Compress = info.Stream.ReadByte();
                if (info.Compress == 2)
                {
                    if (_compressionSupported)
                    {
                        IceInternal.Buffer ubuf = BZip2.Uncompress(info.Stream.GetBuffer(), Protocol.headerSize,
                                                                   _messageSizeMax);
                        info.Stream = new InputStream(info.Stream.Communicator, info.Stream.Encoding, ubuf, true);
                    }
                    else
                    {
                        string lib = AssemblyUtil.IsWindows ? "bzip2.dll" : "libbz2.so.1";
                        throw new FeatureNotSupportedException($"Cannot uncompress compressed message: {lib} not found");
                    }
                }
                info.Stream.Pos = Protocol.headerSize;

                switch (messageType)
                {
                    case Protocol.closeConnectionMsg:
                        {
                            TraceUtil.TraceRecv(info.Stream, _logger, _traceLevels);
                            if (_endpoint.Datagram())
                            {
                                if (_warn)
                                {
                                    _logger.Warning("ignoring close connection message for datagram connection:\n" + _desc);
                                }
                            }
                            else
                            {
                                SetState(StateClosingPending, new CloseConnectionException());

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

                    case Protocol.requestMsg:
                        {
                            if (_state >= StateClosing)
                            {
                                TraceUtil.Trace("received request during closing\n" +
                                                "(ignored by server, client will retry)", info.Stream, _logger,
                                                _traceLevels);
                            }
                            else
                            {
                                TraceUtil.TraceRecv(info.Stream, _logger, _traceLevels);
                                info.RequestId = info.Stream.ReadInt();
                                info.InvokeNum = 1;
                                info.Adapter = _adapter;
                                ++info.MessageDispatchCount;
                            }
                            break;
                        }

                    case Protocol.requestBatchMsg:
                        {
                            if (_state >= StateClosing)
                            {
                                TraceUtil.Trace("received batch request during closing\n" +
                                                "(ignored by server, client will retry)", info.Stream, _logger,
                                                _traceLevels);
                            }
                            else
                            {
                                TraceUtil.TraceRecv(info.Stream, _logger, _traceLevels);
                                info.InvokeNum = info.Stream.ReadInt();
                                if (info.InvokeNum < 0)
                                {
                                    info.InvokeNum = 0;
                                    throw new UnmarshalOutOfBoundsException();
                                }
                                info.Adapter = _adapter;
                                info.MessageDispatchCount += info.InvokeNum;
                            }
                            break;
                        }

                    case Protocol.replyMsg:
                        {
                            TraceUtil.TraceRecv(info.Stream, _logger, _traceLevels);
                            info.RequestId = info.Stream.ReadInt();
                            if (_asyncRequests.TryGetValue(info.RequestId, out info.OutAsync))
                            {
                                _asyncRequests.Remove(info.RequestId);

                                info.OutAsync.GetIs().Swap(info.Stream);

                                //
                                // If we just received the reply for a request which isn't acknowledge as
                                // sent yet, we queue the reply instead of processing it right away. It
                                // will be processed once the write callback is invoked for the message.
                                //
                                OutgoingMessage? message = _sendStreams.Count > 0 ? _sendStreams.First.Value : null;
                                if (message != null && message.OutAsync == info.OutAsync)
                                {
                                    message.ReceivedReply = true;
                                }
                                else if (info.OutAsync.Response())
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

                    case Protocol.validateConnectionMsg:
                        {
                            TraceUtil.TraceRecv(info.Stream, _logger, _traceLevels);
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
                                            info.Stream, _logger, _traceLevels);
                            throw new UnknownMessageException();
                        }
                }
            }
            catch (LocalException ex)
            {
                if (_endpoint.Datagram())
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

            return _state == StateHolding ? SocketOperation.None : SocketOperation.Read;
        }

        private async ValueTask InvokeAllAsync(InputStream requestFrame, int invokeNum, int requestId,
            byte compressionStatus, ObjectAdapter adapter)
        {
            // Note: In contrast to other private or protected methods, this method must be called *without* the
            // mutex locked.

            Debug.Assert(invokeNum > 0); // invokeNum is usually 1 but can be larger for a batch request.
            Debug.Assert(invokeNum == 1); // TODO: deal with batch requests

            Ice.Instrumentation.IDispatchObserver? dispatchObserver = null;

            try
            {
                int start = requestFrame.Pos;
                Current current = Protocol.CreateCurrent(requestId, requestFrame, adapter, this);

                // Then notify and set dispatch observer, if any.
                Ice.Instrumentation.ICommunicatorObserver? communicatorObserver = adapter.Communicator.Observer;
                if (communicatorObserver != null)
                {
                    int encapsSize = requestFrame.GetEncapsulationSize();

                    dispatchObserver = communicatorObserver.GetDispatchObserver(current,
                        requestFrame.Pos - start + encapsSize);
                    dispatchObserver?.Attach();
                }

                Ice.OutputStream? responseFrame = null;
                try
                {
                    Ice.IObject? servant = current.Adapter.Find(current.Id, current.Facet);

                    if (servant == null)
                    {
                        requestFrame.SkipCurrentEncapsulation(); // Required for batch requests, and incoming batch
                                                                 // requests are still supported in Ice 4.x.

                        throw new Ice.ObjectNotExistException(current.Id, current.Facet, current.Operation);
                    }

                    ValueTask<OutputStream> vt = servant.DispatchAsync(requestFrame, current);
                    --invokeNum;
                    if (requestId != 0)
                    {
                        responseFrame = await vt.ConfigureAwait(false);
                    }
                }
                catch (System.Exception ex)
                {
                    if (requestId != 0)
                    {
                        Incoming.ReportException(ex, dispatchObserver, current);
                        responseFrame = new OutgoingResponseFrame(current, ex);
                    }
                }

                if (requestId == 0)
                {
                    SendNoResponse();
                }
                else
                {
                    Debug.Assert(responseFrame != null);
                    dispatchObserver?.Reply(responseFrame.Size - Protocol.headerSize - 4);
                    SendResponse(responseFrame, compressionStatus);
                }
            }
            catch (LocalException ex)
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
                DefaultsAndOverrides defaultsAndOverrides = _communicator.DefaultsAndOverrides;
                if (defaultsAndOverrides.OverrideConnectTimeout)
                {
                    timeout = defaultsAndOverrides.OverrideConnectTimeoutValue;
                }
                else
                {
                    timeout = _endpoint.Timeout();
                }
            }
            else if (_state < StateClosingPending)
            {
                if (_readHeader) // No timeout for reading the header.
                {
                    status &= ~SocketOperation.Read;
                }
                timeout = _endpoint.Timeout();
            }
            else
            {
                DefaultsAndOverrides defaultsAndOverrides = _communicator.DefaultsAndOverrides;
                if (defaultsAndOverrides.OverrideCloseTimeout)
                {
                    timeout = defaultsAndOverrides.OverrideCloseTimeoutValue;
                }
                else
                {
                    timeout = _endpoint.Timeout();
                }
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
            catch (LocalException)
            {
                _info = new ConnectionInfo();
            }
            for (ConnectionInfo? info = _info; info != null; info = info.Underlying)
            {
                info.ConnectionId = _endpoint.ConnectionId();
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

        private void Warning(string msg, System.Exception ex) => _logger.Warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());

        private void ObserverStartRead(IceInternal.Buffer buf)
        {
            if (_readStreamPos >= 0)
            {
                Debug.Assert(!buf.Empty());
                _observer!.ReceivedBytes(buf.B.Position() - _readStreamPos);
            }
            _readStreamPos = buf.Empty() ? -1 : buf.B.Position();
        }

        private void ObserverFinishRead(IceInternal.Buffer buf)
        {
            if (_readStreamPos == -1)
            {
                return;
            }
            Debug.Assert(buf.B.Position() >= _readStreamPos);
            _observer!.ReceivedBytes(buf.B.Position() - _readStreamPos);
            _readStreamPos = -1;
        }

        private int Read(IceInternal.Buffer buf)
        {
            int start = buf.B.Position();
            int op = _transceiver.Read(buf, ref HasMoreData);
            if (_communicator.TraceLevels.Network >= 3 && buf.B.Position() != start)
            {
                var s = new StringBuilder("received ");
                if (_endpoint.Datagram())
                {
                    s.Append(buf.B.Limit());
                }
                else
                {
                    s.Append(buf.B.Position() - start);
                    s.Append(" of ");
                    s.Append(buf.B.Limit() - start);
                }
                s.Append(" bytes via ");
                s.Append(_endpoint.Transport());
                s.Append("\n");
                s.Append(ToString());
                _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
            }
            return op;
        }

        private int Write(IList<ArraySegment<byte>> buffer, int size, ref int offset)
        {
            int remainig = size - offset;
            int socketOperation = _transceiver.Write(buffer, ref offset);
            int bytesTransferred = remainig - (size - offset);
            if (_communicator.TraceLevels.Network >= 3 && bytesTransferred > 0)
            {
                var s = new StringBuilder("sent ");
                s.Append(bytesTransferred);
                if (!_endpoint.Datagram())
                {
                    s.Append(" of ");
                    s.Append(remainig);
                }
                s.Append(" bytes via ");
                s.Append(_endpoint.Transport());
                s.Append("\n");
                s.Append(ToString());
                _logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
            }

            if (_observer != null && bytesTransferred > 0)
            {
                _observer.SentBytes(bytesTransferred);
            }
            return socketOperation;
        }

        private class OutgoingMessage
        {
            internal OutgoingMessage(OutputStream stream, bool compress)
            {
                Stream = stream;
                Compress = compress;
            }

            internal OutgoingMessage(OutgoingAsyncBase outAsync, OutputStream stream, bool compress, int requestId)
            {
                OutAsync = outAsync;
                Stream = stream;
                Compress = compress;
                RequestId = requestId;
            }

            internal void Canceled()
            {
                Debug.Assert(OutAsync != null); // Only requests can timeout.
                OutAsync = null;
            }

            internal bool Sent()
            {
                Stream = null;
                if (OutAsync != null)
                {
                    InvokeSent = OutAsync.Sent();
                    return InvokeSent || ReceivedReply;
                }
                return false;
            }

            internal void Completed(LocalException ex)
            {
                if (OutAsync != null)
                {
                    if (OutAsync.Exception(ex))
                    {
                        OutAsync.InvokeException();
                    }
                }
                Stream = null;
            }

            internal OutputStream? Stream;
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

        private LocalException? _exception;

        private readonly int _messageSizeMax;

        private readonly LinkedList<OutgoingMessage> _sendStreams = new LinkedList<OutgoingMessage>();

        private readonly InputStream _readStream;
        private bool _readHeader;

        private readonly OutputStream _writeStream;
        private IList<ArraySegment<byte>> _writeBuffer;
        private int _writeBufferOffset;
        private int _writeBufferSize;

        private ICommunicatorObserver? _communicatorObserver;
        private IConnectionObserver? _observer;
        private int _readStreamPos;

        private int _dispatchCount;

        private int _state; // The current state.
        private bool _shutdownInitiated = false;
        private bool _initialized = false;
        private bool _validated = false;

        private static readonly bool _compressionSupported = BZip2.Supported();

        private ConnectionInfo? _info;

        private CloseCallback? _closeCallback;
        private HeartbeatCallback? _heartbeatCallback;

        private static readonly ConnectionState[] _connectionStateMap = new ConnectionState[] {
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
