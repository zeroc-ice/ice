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

    public sealed class Connection : ICancellationHandler
    {
        /// <summary>Get or set the object adapter that dispatches requests received over this connection.
        /// A client can invoke an operation on a server using a proxy, and then set an object adapter for the
        /// outgoing connection used by the proxy in order to receive callbacks. This is useful if the server
        /// cannot establish a connection back to the client, for example because of firewalls.</summary>
        /// <returns>The object adapter that dispatches requests for the connection, or null if no adapter is set.
        /// </returns>
        public ObjectAdapter? Adapter
        {
            get
            {
                lock (_mutex)
                {
                    return _adapter;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _adapter = value;
                }
            }
        }

        /// <summary>
        /// Get the endpoint from which the connection was created.
        /// </summary>
        /// <returns>The endpoint from which the connection was created.</returns>
        public Endpoint Endpoint => _endpoint; // No mutex protection necessary, _endpoint is immutable.

        /// <summary>
        /// Get the timeout for the connection.
        /// </summary>
        /// <returns>The connection's timeout.</returns>
        // TODO: Remove Timeout after reviewing its usages, it's no longer used by the connection
        public int Timeout => _endpoint.Timeout; // No mutex protection necessary, _endpoint is immutable.

        internal IConnector Connector => _connector!;

        internal bool Active
        {
            get
            {
                lock (_mutex)
                {
                    return _state > State.NotInitialized && _state < State.Closing;
                }
            }
        }

        private long _acmLastActivity;
        private ObjectAdapter? _adapter;
        private Action<Connection>? _closeCallback;
        private readonly Communicator _communicator;
        private readonly int _compressionLevel;
        private readonly IConnector? _connector;
        private int _dispatchCount;
        private readonly Endpoint _endpoint;
        private System.Exception? _exception;
        private Action<Connection>? _heartbeatCallback;
        private ConnectionInfo? _info;
        private readonly int _messageSizeMax;
        private IACMMonitor? _monitor;
        private readonly object _mutex = new object();
        private int _nextRequestId;
        private IConnectionObserver? _observer;
        private readonly LinkedList<OutgoingMessage> _outgoingMessages = new LinkedList<OutgoingMessage>();
        private int _pendingIO;
        private readonly Dictionary<int, OutgoingAsyncBase> _requests = new Dictionary<int, OutgoingAsyncBase>();
        private State _state; // The current state.
        private readonly ITransceiver _transceiver;
        private bool _validated = false;
        private readonly bool _warn;
        private readonly bool _warnUdp;

        private static readonly ConnectionState[] _connectionStateMap = new ConnectionState[]
        {
            ConnectionState.ConnectionStateValidating,   // State.NotInitialized
            ConnectionState.ConnectionStateActive,       // State.Active
            ConnectionState.ConnectionStateClosing,      // State.Closing
            ConnectionState.ConnectionStateClosing,      // State.ClosingPending
            ConnectionState.ConnectionStateClosed,       // State.Closed
            ConnectionState.ConnectionStateClosed,       // State.Finished
        };

        private static readonly List<ArraySegment<byte>> _closeConnectionMessage =
            new List<ArraySegment<byte>> { Ice1Definitions.CloseConnectionMessage };
        private static readonly List<ArraySegment<byte>> _validateConnectionMessage =
            new List<ArraySegment<byte>> { Ice1Definitions.ValidateConnectionMessage };

        /// <summary>Manually close the connection using the specified closure mode.</summary>
        /// <param name="mode">Determines how the connection will be closed.</param>
        public void Close(ConnectionClose mode)
        {
            lock (_mutex)
            {
                if (mode == ConnectionClose.Forcefully)
                {
                    SetState(State.Closed, new ConnectionClosedLocallyException("connection closed forcefully"));
                }
                else if (mode == ConnectionClose.Gracefully)
                {
                    SetState(State.Closing, new ConnectionClosedLocallyException("connection closed gracefully"));
                }
                else
                {
                    Debug.Assert(mode == ConnectionClose.GracefullyWithWait);

                    //
                    // Wait until all outstanding requests have been completed.
                    //
                    while (_requests.Count != 0)
                    {
                        System.Threading.Monitor.Wait(_mutex);
                    }

                    SetState(State.Closing, new ConnectionClosedLocallyException("connection closed gracefully"));
                }
            }
        }

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

        /// <summary>Get the ACM parameters.</summary>
        /// <returns>The ACM parameters.</returns>
        public ACM GetACM()
        {
            lock (_mutex)
            {
                return _monitor != null ? _monitor.GetACM() : new ACM(0, ACMClose.CloseOff, ACMHeartbeat.HeartbeatOff);
            }
        }

        /// <summary>Returns the connection information.</summary>
        /// <returns>The connection information.</returns>
        public ConnectionInfo GetConnectionInfo()
        {
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    throw _exception!;
                }
                return InitConnectionInfo();
            }
        }

        /// <summary>Send a heartbeat message.</summary>
        public void Heartbeat() => HeartbeatAsync().Wait();

        /// <summary>Send an asynchronous heartbeat message.</summary>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public Task HeartbeatAsync(IProgress<bool>? progress = null, CancellationToken cancel = new CancellationToken())
        {
            var completed = new HeartbeatTaskCompletionCallback(progress, cancel);
            var outgoing = new HeartbeatOutgoingAsync(this, _communicator, completed);
            outgoing.Invoke();
            return completed.Task;
        }

        /// <summary>Set the active connection management parameters.</summary>
        /// <param name="timeout">The timeout value in seconds, must be &gt;= 0.</param>
        /// <param name="close">The close condition</param>
        /// <param name="heartbeat">The heartbeat condition</param>
        public void SetACM(int? timeout, ACMClose? close, ACMHeartbeat? heartbeat)
        {
            lock (_mutex)
            {
                if (timeout is int timeoutValue && timeoutValue < 0)
                {
                    throw new ArgumentException("invalid negative ACM timeout value", nameof(timeout));
                }

                if (_monitor == null || _state >= State.Closed)
                {
                    return;
                }

                if (_state == State.Active)
                {
                    _monitor.Remove(this);
                }
                _monitor = _monitor.Acm(timeout, close, heartbeat);

                if (_monitor.GetACM().Timeout <= 0)
                {
                    _acmLastActivity = -1; // Disable the recording of last activity.
                }
                else if (_state == State.Active && _acmLastActivity == -1)
                {
                    _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                }

                if (_state == State.Active)
                {
                    _monitor.Add(this);
                }
            }
        }

        /// <summary>Set the connection buffer receive/send size.</summary>
        /// <param name="rcvSize">The connection receive buffer size.</param>
        /// <param name="sndSize">The connection send buffer size.</param>
        public void SetBufferSize(int rcvSize, int sndSize)
        {
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    throw _exception!;
                }
                _transceiver.SetBufferSize(rcvSize, sndSize);
                _info = null; // Invalidate the cached connection info
            }
        }

        /// <summary>Set a close callback on the connection. The callback is called by the connection when it's
        /// closed.If the callback needs more information about the closure, it can call Connection.throwException.
        /// </summary>
        /// <param name="callback">The close callback object.</param>
        public void SetCloseCallback(Action<Connection> callback)
        {
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    if (callback != null)
                    {
                        Task.Run(() =>
                        {
                            try
                            {
                                callback(this);
                            }
                            catch (Exception ex)
                            {
                                _communicator.Logger.Error($"connection callback exception:\n{ex}\n{this}");
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

        /// <summary>Set a heartbeat callback on the connection. The callback is called by the connection when a
        /// heartbeat is received.</summary>
        /// <param name="callback">The heartbeat callback object.</param>
        public void SetHeartbeatCallback(Action<Connection> callback)
        {
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    return;
                }
                _heartbeatCallback = callback;
            }
        }

        /// <summary>Throw an exception indicating the reason for connection closure. For example,
        /// ConnectionClosedByPeerException is raised if the connection was closed gracefully by the peer, whereas
        /// ConnectionClosedLocallyException is raised if the connection was manually closed by the application. This
        /// operation does nothing if the connection is not yet closed.</summary>
        public void ThrowException()
        {
            lock (_mutex)
            {
                if (_exception != null)
                {
                    Debug.Assert(_state >= State.Closing);
                    throw _exception;
                }
            }
        }

        /// <summary>Return a description of the connection as human readable text, suitable for logging or error
        /// messages.</summary>
        /// <returns>The description of the connection as human readable text.</returns>
        public override string ToString() => _transceiver.ToString()!;

        /// <summary>Return the connection type. This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
        /// </summary>
        /// <returns>The type of the connection.</returns>
        public string Type() => _transceiver.Transport(); // No mutex lock, _type is immutable.

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
            _connector = connector;
            _endpoint = endpoint;
            _adapter = adapter;
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
            _dispatchCount = 0;
            _pendingIO = 0;
            _state = State.NotInitialized;

            _compressionLevel = communicator.GetPropertyAsInt("Ice.Compression.Level") ?? 1;
            if (_compressionLevel < 1)
            {
                _compressionLevel = 1;
            }
            else if (_compressionLevel > 9)
            {
                _compressionLevel = 9;
            }
        }

        // TODO: Benoit: This needs to be internal, ICancellationHandler needs to be fixed, for another PR.
        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, System.Exception ex)
        {
            //
            // NOTE: This isn't called from a thread pool thread.
            //

            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    return; // The request has already been or will be shortly notified of the failure.
                }

                OutgoingMessage? o = _outgoingMessages.FirstOrDefault(m => m.OutAsync == outAsync);
                if (o != null)
                {
                    if (o.RequestId > 0)
                    {
                        _requests.Remove(o.RequestId);
                    }

                    //
                    // If the request is being sent, don't remove it from the send streams,
                    // it will be removed once the sending is finished.
                    //
                    if (o != _outgoingMessages.First!.Value)
                    {
                        _outgoingMessages.Remove(o);
                    }
                    o.OutAsync = null;
                    if (outAsync.Exception(ex))
                    {
                        Task.Run(outAsync.InvokeException);
                    }
                    return;
                }

                if (outAsync is OutgoingAsync)
                {
                    foreach (KeyValuePair<int, OutgoingAsyncBase> kvp in _requests)
                    {
                        if (kvp.Value == outAsync)
                        {
                            _requests.Remove(kvp.Key);
                            if (outAsync.Exception(ex))
                            {
                                Task.Run(outAsync.InvokeException);
                            }
                            return;
                        }
                    }
                }
            }
        }

        internal void ClearAdapter(ObjectAdapter adapter)
        {
            lock (_mutex)
            {
                if (_adapter == adapter)
                {
                    _adapter = null;
                }
            }
        }

        internal void Destroy(Exception ex)
        {
            lock (_mutex)
            {
                SetState(State.Closing, ex);
            }
        }

        internal void Monitor(long now, ACMConfig acm)
        {
            lock (_mutex)
            {
                if (_state < State.Active || _state >= State.Closed)
                {
                    return;
                }

                // We send a heartbeat if there was no activity in the last (timeout / 4) period. Sending a heartbeat
                // sooner than really needed is safer to ensure that the receiver will receive the heartbeat in time.
                // Sending the heartbeat if there was no activity in the last (timeout / 2) period isn't enough since
                // monitor() is called only every (timeout / 2) period.
                //
                // Note that this doesn't imply that we are sending 4 heartbeats per timeout period because the
                // monitor() method is still only called every (timeout / 2) period.
                if (_state == State.Active &&
                    (acm.Heartbeat == ACMHeartbeat.HeartbeatAlways ||
                    (acm.Heartbeat != ACMHeartbeat.HeartbeatOff && now >= (_acmLastActivity + (acm.Timeout / 4)))))
                {
                    if (acm.Heartbeat != ACMHeartbeat.HeartbeatOnDispatch || _dispatchCount > 0)
                    {
                        Debug.Assert(_state == State.Active);
                        if (!_endpoint.IsDatagram)
                        {
                            try
                            {
                                Send(new OutgoingMessage(_validateConnectionMessage, false));
                            }
                            catch (System.Exception ex)
                            {
                                SetState(State.Closed, ex);
                            }
                        }
                    }
                }

                // TODO: We still rely on the endpoint timeout here, remove and change the override close timeout to
                // Ice.CloseTimeout (or just rely on ACM timeout)
                int timeout = acm.Timeout;
                if (_state >= State.Closing)
                {
                    timeout = _communicator.OverrideCloseTimeout ?? _endpoint.Timeout;
                }

                // ACM close is always enabled when in the closing state for connection close timeouts.
                if ((_state >= State.Closing || acm.Close != ACMClose.CloseOff) && now >= (_acmLastActivity + timeout))
                {
                    if (acm.Close == ACMClose.CloseOnIdleForceful ||
                       (acm.Close != ACMClose.CloseOnIdle && (_requests.Count > 0)))
                    {
                        //
                        // Close the connection if we didn't receive a heartbeat or if read/write didn't update the
                        // ACM activity in the last period.
                        //
                        SetState(State.Closed, new ConnectionTimeoutException());
                    }
                    else if (acm.Close != ACMClose.CloseOnInvocation && _dispatchCount == 0 && _requests.Count == 0)
                    {
                        //
                        // The connection is idle, close it.
                        //
                        SetState(State.Closing, new ConnectionIdleException());
                    }
                }
            }
        }

        // TODO: Benoit: SendAsyncRequest needs to be changed to be an awaitable method that returns
        // once the request is sent. The connection code won't have to deal with sent callback anymore,
        // it will be the job of the caller.
        internal void SendAsyncRequest(OutgoingAsyncBase outgoing, bool compress, bool response)
        {
            lock (_mutex)
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

                Debug.Assert(_state > State.NotInitialized);
                Debug.Assert(_state < State.Closing);

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

                // Ensure the message isn't bigger than what we can send with the transport.
                _transceiver.CheckSendSize(size);

                outgoing.AttachRemoteObserver(InitConnectionInfo(), _endpoint, requestId,
                    size - (Ice1Definitions.HeaderSize + 4));

                try
                {
                    Send(new OutgoingMessage(outgoing, data, compress, requestId));
                }
                catch (Exception ex)
                {
                    SetState(State.Closed, ex);
                    throw _exception!;
                }

                if (response)
                {
                    _requests[requestId] = outgoing;
                }
            }
        }

        internal async ValueTask StartAsync()
        {
            try
            {
                // TODO: for now, we continue using the endpoint timeout as the default connect timeout. This is
                // use for both connect/accept timeouts. We're leaning toward adding Ice.ConnectTimeout for
                // connection establishemnt and using the ACM timeout for accepting connections.
                int timeout = _communicator.OverrideConnectTimeout ?? _endpoint.Timeout;

                // Initialize the transport
                await AwaitWithTimeout(_transceiver.InitializeAsync().AsTask(), timeout).ConfigureAwait(false);

                ArraySegment<byte> readBuffer = default;
                if (!_endpoint.IsDatagram) // Datagram connections are always implicitly validated.
                {
                    if (_connector == null) // The server side has the active role for connection validation.
                    {
                        int offset = 0;
                        while (offset < _validateConnectionMessage.GetByteCount())
                        {
                            var writeTask = _transceiver.WriteAsync(_validateConnectionMessage, offset).AsTask();
                            await AwaitWithTimeout(writeTask, timeout).ConfigureAwait(false);
                            offset += writeTask.Result;
                        }
                        Debug.Assert(offset == _validateConnectionMessage.GetByteCount());
                    }
                    else // The client side has the passive role for connection validation.
                    {
                        readBuffer = new ArraySegment<byte>(new byte[Ice1Definitions.HeaderSize]);
                        int offset = 0;
                        while (offset < Ice1Definitions.HeaderSize)
                        {
                            var readTask = _transceiver.ReadAsync(readBuffer, offset).AsTask();
                            await AwaitWithTimeout(readTask, timeout).ConfigureAwait(false);
                            offset += readTask.Result;
                        }

                        Ice1Definitions.CheckHeader(readBuffer.AsSpan(0, 8));
                        var messageType = (Ice1Definitions.MessageType)readBuffer[8];
                        if (messageType != Ice1Definitions.MessageType.ValidateConnectionMessage)
                        {
                            throw new InvalidDataException(@$"received ice1 frame with message type `{messageType
                                }' before receiving the validate connection message");
                        }

                        int size = InputStream.ReadInt(readBuffer.AsSpan(10, 4));
                        if (size != Ice1Definitions.HeaderSize)
                        {
                            throw new InvalidDataException(
                                $"received an ice1 frame with validate connection type and a size of `{size}' bytes");
                        }
                    }
                }

                lock (_mutex)
                {
                    if (_state >= State.Closed)
                    {
                        throw _exception!;
                    }

                    if (!_endpoint.IsDatagram) // Datagram connections are always implicitly validated.
                    {
                        if (_connector == null) // The server side has the active role for connection validation.
                        {
                            TraceSentAndUpdateObserver(_validateConnectionMessage.GetByteCount());
                            TraceUtil.TraceSend(_communicator, _validateConnectionMessage);
                        }
                        else
                        {
                            TraceReceivedAndUpdateObserver(readBuffer.Count);
                            TraceUtil.TraceRecv(_communicator, readBuffer);
                        }
                    }

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
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
                    }

                    if (_acmLastActivity > -1)
                    {
                        _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                    }
                    if (_connector != null)
                    {
                        _validated = true;
                    }

                    SetState(State.Active);
                }

                // Start the asynchronous operation from the thread pool to prevent eventually reading
                // synchronously new frames from this thread.
                _ = Task.Run(() => RunIO(ReadAsync));
            }
            catch (Exception ex)
            {
                lock (_mutex)
                {
                    SetState(State.Closed, ex);
                }
                throw;
            }

            // Helper to await task with timeout
            static async ValueTask AwaitWithTimeout(Task task, int timeout)
            {
                if (timeout < 0)
                {
                    await task.ConfigureAwait(false);
                }
                else
                {
                    var cancelTimeout = new CancellationTokenSource();
                    var t = await Task.WhenAny(Task.Delay(timeout, cancelTimeout.Token), task).ConfigureAwait(false);
                    if (t == task)
                    {
                        cancelTimeout.Cancel();
                        await t.ConfigureAwait(false); // Unwrap the exception if it failed
                    }
                    else
                    {
                        throw new ConnectTimeoutException();
                    }
                }
            }
        }

        internal void UpdateObserver()
        {
            lock (_mutex)
            {
                if (_state < State.NotInitialized || _state > State.Closed)
                {
                    return;
                }

                _observer = _communicator.Observer?.GetConnectionObserver(InitConnectionInfo(), _endpoint,
                    _connectionStateMap[(int)_state], _observer);
                if (_observer != null)
                {
                    _observer.Attach();
                }
            }
        }

        internal void WaitUntilFinished()
        {
            lock (_mutex)
            {
                //
                // We wait indefinitely until the connection is finished and all
                // outstanding requests are completed. Otherwise we couldn't
                // guarantee that there are no outstanding calls when deactivate()
                // is called on the servant locators.
                //
                while (_state < State.Finished || _dispatchCount > 0)
                {
                    System.Threading.Monitor.Wait(_mutex);
                }
            }
        }

        private void Finish()
        {
            if (_outgoingMessages.Count > 0)
            {
                foreach (OutgoingMessage o in _outgoingMessages)
                {
                    if (o.OutAsync != null && o.OutAsync.Exception(_exception!))
                    {
                        o.OutAsync.InvokeException();
                    }
                    if (o.RequestId > 0) // Make sure Completed isn't called twice.
                    {
                        _requests.Remove(o.RequestId);
                    }
                }
                // Must be cleared before _requests because of Outgoing* references in OutgoingMessage
                _outgoingMessages.Clear();
            }

            foreach (OutgoingAsyncBase o in _requests.Values)
            {
                if (o.Exception(_exception!))
                {
                    o.InvokeException();
                }
            }
            _requests.Clear();

            try
            {
                _closeCallback?.Invoke(this);
            }
            catch (Exception ex)
            {
                _communicator.Logger.Error($"connection callback exception:\n{ex}\n{this}");
            }
            _closeCallback = null;
            _heartbeatCallback = null;

            lock (_mutex)
            {
                // This must be done last as this will cause waitUntilFinished() to return.
                SetState(State.Finished);
            }
        }

        private ConnectionInfo InitConnectionInfo()
        {
            if (_state > State.NotInitialized && _info != null) // Update the connection info until it's initialized
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

        private void InitiateShutdown()
        {
            Debug.Assert(_state == State.Closing && _dispatchCount == 0);

            if (!_endpoint.IsDatagram)
            {
                // Before we shut down, we send a close connection message.
                Send(new OutgoingMessage(_closeConnectionMessage, false));
            }
        }

        private async ValueTask InvokeAsync(Current current, IncomingRequestFrame request, byte compressionStatus)
        {
            IDispatchObserver? dispatchObserver = null;
            OutgoingResponseFrame? response = null;
            try
            {
                // Notify and set dispatch observer, if any.
                ICommunicatorObserver? communicatorObserver = _communicator.Observer;
                if (communicatorObserver != null)
                {
                    dispatchObserver = communicatorObserver.GetDispatchObserver(current, request.Size);
                    dispatchObserver?.Attach();
                }

                try
                {
                    IObject? servant = current.Adapter.Find(current.Identity, current.Facet);
                    if (servant == null)
                    {
                        throw new ObjectNotExistException(current.Identity, current.Facet, current.Operation);
                    }

                    ValueTask<OutgoingResponseFrame> vt = servant.DispatchAsync(request, current);
                    if (current.RequestId != 0)
                    {
                        response = await vt.ConfigureAwait(false);
                    }
                }
                catch (Exception ex)
                {
                    if (current.RequestId != 0)
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

                if (response != null)
                {
                    dispatchObserver?.Reply(response.Size);
                }
            }
            finally
            {
                lock (_mutex)
                {
                    // Send the response if there's a response
                    if (_state < State.Closed && response != null)
                    {
                        // TODO: should we await on Send for the response when Send is async?
                        Send(new OutgoingMessage(Ice1Definitions.GetResponseData(response, current.RequestId),
                             compressionStatus > 0));
                    }

                    // Decrease the dispatch count
                    Debug.Assert(_dispatchCount > 0);
                    if (--_dispatchCount == 0)
                    {
                        if (_state == State.Closing)
                        {
                            try
                            {
                                InitiateShutdown();
                            }
                            catch (Exception ex)
                            {
                                SetState(State.Closed, ex);
                            }
                        }
                        else if (_state == State.Finished)
                        {
                            Reap();
                        }
                        System.Threading.Monitor.PulseAll(_mutex);
                    }
                }

                dispatchObserver?.Detach();
            }
        }

        private async ValueTask<(Func<ValueTask>?, ObjectAdapter?)> ReadIncomingAsync()
        {
            // Read header
            ArraySegment<byte> readBuffer;
            if (_endpoint.IsDatagram)
            {
                readBuffer = await _transceiver.ReadAsync().ConfigureAwait(false);
            }
            else
            {
                readBuffer = new ArraySegment<byte>(new byte[256], 0, Ice1Definitions.HeaderSize);
                int offset = 0;
                while (offset < Ice1Definitions.HeaderSize)
                {
                    offset += await _transceiver.ReadAsync(readBuffer, offset).ConfigureAwait(false);
                }
            }

            // Check header
            Ice1Definitions.CheckHeader(readBuffer.AsSpan(0, 8));
            int size = InputStream.ReadInt(readBuffer.Slice(10, 4));
            if (size < Ice1Definitions.HeaderSize)
            {
                throw new InvalidDataException($"received ice1 frame with only {size} bytes");
            }

            if (size > _messageSizeMax)
            {
                throw new InvalidDataException($"frame with {size} bytes exceeds Ice.MessageSizeMax value");
            }

            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    Debug.Assert(_exception != null);
                    throw _exception;
                }

                TraceReceivedAndUpdateObserver(readBuffer.Count);
                if (_acmLastActivity > -1)
                {
                    _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                }

                // Connection is validated on first message. This is only used by setState() to check wether or
                // not we can print a connection warning (a client might close the connection forcefully if the
                // connection isn't validated, we don't want to print a warning in this case).
                _validated = true;
            }

            // Read the remainder of the message if needed
            if (!_endpoint.IsDatagram)
            {
                if (size > readBuffer.Array!.Length)
                {
                    // Allocate a new array and copy the header over
                    var buffer = new ArraySegment<byte>(new byte[size], 0, size);
                    readBuffer.AsSpan().CopyTo(buffer.AsSpan(0, Ice1Definitions.HeaderSize));
                    readBuffer = buffer;
                }
                else if (size > readBuffer.Count)
                {
                    readBuffer = new ArraySegment<byte>(readBuffer.Array!, 0, size);
                }
                Debug.Assert(size == readBuffer.Count);

                int offset = Ice1Definitions.HeaderSize;
                while (offset < readBuffer.Count)
                {
                    int bytesReceived = await _transceiver.ReadAsync(readBuffer, offset).ConfigureAwait(false);
                    offset += bytesReceived;

                    // Trace the receival progress within the loop as we might be receiving significant amount
                    // of data here.
                    lock (_mutex)
                    {
                        if (_state >= State.Closed)
                        {
                            Debug.Assert(_exception != null);
                            throw _exception;
                        }

                        TraceReceivedAndUpdateObserver(bytesReceived);
                        if (_acmLastActivity > -1)
                        {
                            _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                        }
                    }
                }
            }
            else if (size > readBuffer.Count)
            {
                if (_warnUdp)
                {
                    _communicator.Logger.Warning($"maximum datagram size of {readBuffer.Count} exceeded");
                }
                return default;
            }

            Func<ValueTask>? incoming = null;
            ObjectAdapter? adapter = null;
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    throw _exception!;
                }

                // The magic and version fields have already been checked.
                var messageType = (Ice1Definitions.MessageType)readBuffer[8];
                byte compressionStatus = readBuffer[9];
                if (compressionStatus == 2)
                {
                    if (BZip2.IsLoaded)
                    {
                        readBuffer = BZip2.Decompress(readBuffer, Ice1Definitions.HeaderSize, _messageSizeMax);
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
                        TraceUtil.TraceRecv(_communicator, readBuffer);
                        if (_endpoint.IsDatagram)
                        {
                            if (_warn)
                            {
                                _communicator.Logger.Warning(
                                    $"ignoring close connection message for datagram connection:\n{this}");
                            }
                        }
                        else
                        {
                            SetState(State.ClosingPending, new ConnectionClosedByPeerException());
                            throw _exception!;
                        }
                        break;
                    }

                    case Ice1Definitions.MessageType.RequestMessage:
                    {
                        if (_state >= State.Closing)
                        {
                            TraceUtil.Trace("received request during closing\n" +
                                "(ignored by server, client will retry)", _communicator, readBuffer);
                        }
                        else
                        {
                            TraceUtil.TraceRecv(_communicator, readBuffer);
                            readBuffer = readBuffer.Slice(Ice1Definitions.HeaderSize);
                            int requestId = InputStream.ReadInt(readBuffer.AsSpan(0, 4));
                            var request = new IncomingRequestFrame(_communicator, readBuffer.Slice(4));
                            if (_adapter == null)
                            {
                                throw new ObjectNotExistException(request.Identity, request.Facet,
                                    request.Operation);
                            }
                            else
                            {
                                adapter = _adapter;
                                var current = new Current(_adapter, request, requestId, this);
                                incoming = () => InvokeAsync(current, request, compressionStatus);
                                ++_dispatchCount;
                            }
                        }
                        break;
                    }

                    case Ice1Definitions.MessageType.RequestBatchMessage:
                    {
                        if (_state >= State.Closing)
                        {
                            TraceUtil.Trace("received batch request during closing\n" +
                                "(ignored by server, client will retry)", _communicator, readBuffer);
                        }
                        else
                        {
                            TraceUtil.TraceRecv(_communicator, readBuffer);
                            int invokeNum = InputStream.ReadInt(readBuffer.AsSpan(Ice1Definitions.HeaderSize, 4));
                            if (invokeNum < 0)
                            {
                                throw new InvalidDataException(
                                    $"received ice1 RequestBatchMessage with {invokeNum} batch requests");
                            }
                            Debug.Assert(false); // TODO: deal with batch requests
                        }
                        break;
                    }

                    case Ice1Definitions.MessageType.ReplyMessage:
                    {
                        TraceUtil.TraceRecv(_communicator, readBuffer);
                        readBuffer = readBuffer.Slice(Ice1Definitions.HeaderSize);
                        int requestId = InputStream.ReadInt(readBuffer.AsSpan(0, 4));
                        if (_requests.TryGetValue(requestId, out OutgoingAsyncBase? outAsync))
                        {
                            _requests.Remove(requestId);

                            if (outAsync.Response(new IncomingResponseFrame(_communicator, readBuffer.Slice(4))))
                            {
                                incoming = () => { outAsync.InvokeResponse(); return default; };
                            }

                            if (_requests.Count == 0)
                            {
                                System.Threading.Monitor.PulseAll(_mutex); // Notify threads blocked in close()
                            }
                        }
                        break;
                    }

                    case Ice1Definitions.MessageType.ValidateConnectionMessage:
                    {
                        TraceUtil.TraceRecv(_communicator, readBuffer);
                        if (_heartbeatCallback != null)
                        {
                            var callback = _heartbeatCallback;
                            incoming = () =>
                            {
                                try
                                {
                                    callback(this);
                                }
                                catch (Exception ex)
                                {
                                    _communicator.Logger.Error($"connection callback exception:\n{ex}\n{this}");
                                }
                                return default;
                            };
                        }
                        break;
                    }

                    default:
                    {
                        TraceUtil.Trace("received unknown message\n(invalid, closing connection)", _communicator,
                             readBuffer);
                        throw new InvalidDataException(
                            $"received ice1 frame with unknown message type `{messageType}'");
                    }
                }
            }

            return (incoming, adapter);
        }

        private async ValueTask ReadAsync()
        {
            // Read asynchronously incoming frames and dispatch the incoming if needed. ReadIncomingAsync can throw
            // when the connection is closed.
            while (true)
            {
                var (incoming, adapter) = await ReadIncomingAsync().ConfigureAwait(false);
                if (incoming != null)
                {
                    if (adapter != null && adapter.SerializeDispatch)
                    {
                        // Run the incoming dispatch and continue reading from this task after the dispatch completes.
                        if (adapter.TaskScheduler != null)
                        {
                            await TaskRun(incoming, adapter.TaskScheduler).ConfigureAwait(false);
                        }
                        else
                        {
                            await incoming().ConfigureAwait(false);
                        }
                    }
                    else
                    {
                        // Start a new Read IO task and run the incoming dispatch. We start the new ReadAsync from
                        // a separate task because ReadAsync could complete synchronously and we don't want the
                        // dispatch from this read to run before we actually ran the dispatch from this block. An
                        // alternative could be to start a task to run the incoming dispatch and continue reading
                        // with this loop. It would have a negative impact on latency however since execution of
                        // the incoming dispatch would potentially require a thread context switch.
                        if (adapter?.TaskScheduler != null)
                        {
                            await TaskRun(() =>
                            {
                                _ = Task.Run(() => RunIO(ReadAsync));
                                return incoming();
                            }, adapter.TaskScheduler).ConfigureAwait(false);
                        }
                        else
                        {
                            _ = Task.Run(() => RunIO(ReadAsync));
                            await incoming().ConfigureAwait(false);
                        }
                        return;
                    }
                }
            }

            static async ValueTask TaskRun(Func<ValueTask> func, TaskScheduler? scheduler)
            {
                // First await for the dispach async to be ran on the task scheduler.
                ValueTask task = await Task.Factory.StartNew(func, default, TaskCreationOptions.None,
                    scheduler ?? TaskScheduler.Default).ConfigureAwait(false);

                // Now wait for the async dispatch to complete.
                await task.ConfigureAwait(false);
            }
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

        private async ValueTask RunIO(Func<ValueTask> ioFunc)
        {
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    return;
                }

                // We keep track of the number of pending IO tasks to ensure orderly connection
                // closure. If a connection is forcefully closed while a Write task is running,
                // we want to make sure the Write returns before notifying the outgoing requests
                // of the failure. Notifying the requests before the write returns could break
                // at most once semantics if for example the Write completed bu the request got
                // notified of the connection closure before.
                ++_pendingIO;
            }

            bool closing = false;
            try
            {
                // Start the long running IO operation.
                await ioFunc().ConfigureAwait(false);
            }
            catch (ConnectionClosedByPeerException)
            {
                closing = true;
            }
            catch (Exception ex)
            {
                lock (_mutex)
                {
                    SetState(State.Closed, ex);
                }
            }

            bool finish = false;
            lock (_mutex)
            {
                --_pendingIO;
                if (_state == State.Closing && _dispatchCount == 0 && _outgoingMessages.Count == 0)
                {
                    SetState(State.ClosingPending);
                    closing = true;
                }
                else if (_state == State.Closed && _pendingIO == 0)
                {
                    // No more pending IO and in the closed state, it's time to terminate the connection
                    // and notify the pending requests of the connection closure.
                    finish = true;
                }
            }

            if (closing)
            {
                // Notify the transport of the graceful connection closure. The transport returns whether or not
                // the closing is completed. It might not be completed in case there's still a Read task pending.
                // If it's the case, the connection will be closed once the pending Read task throws an exception.
                bool completed;
                try
                {
                    completed = await _transceiver.ClosingAsync(_exception!).ConfigureAwait(false);
                }
                catch (Exception)
                {
                    // TODO: Debug.Assert(false) instead with the transport refactoring?
                    completed = true;
                }
                if (completed)
                {
                    lock (_mutex)
                    {
                        SetState(State.Closed);
                    }
                }
            }

            if (finish)
            {
                Finish();
            }
        }

        private void Send(OutgoingMessage message)
        {
            Debug.Assert(_state < State.Closed);
            // TODO: Benoit: Refactor to write and await the calling thread to avoid having writing on a thread
            // pool thread
            _outgoingMessages.AddLast(message);
            if (_outgoingMessages.Count == 1)
            {
                Task.Run(() => _ = RunIO(WriteAsync));
            }
        }

        private void SetState(State state, System.Exception ex)
        {
            // If setState() is called with an exception, then only closed and closing State.s are permissible.
            Debug.Assert(state >= State.Closing);

            if (_state == state) // Don't switch twice.
            {
                return;
            }

            if (_exception == null)
            {
                // If we are in closed state, an exception must be set.
                Debug.Assert(_state != State.Closed);
                _exception = ex;

                // We don't warn if we are not validated.
                if (_warn && _validated)
                {
                    // Don't warn about certain expected exceptions.
                    if (!(_exception is ConnectionClosedException ||
                         _exception is ConnectionIdleException ||
                         _exception is CommunicatorDestroyedException ||
                         _exception is ObjectAdapterDeactivatedException ||
                         (_exception is ConnectionLostException && _state >= State.Closing)))
                    {
                        _communicator.Logger.Warning($"connection exception:\n{_exception}\n{this}");
                    }
                }
            }

            // We must set the new state before we notify requests of any exceptions. Otherwise new requests
            // may retry on a connection that is not yet marked as closed or closing.
            SetState(state);
        }

        private void SetState(State state)
        {
            // We don't want to send close connection messages if the endpoint only supports oneway transmission
            // from client to server.
            if (_endpoint.IsDatagram && state == State.Closing)
            {
                state = State.Closed;
            }

            // Skip graceful shutdown if we are destroyed before active.
            if (_state < State.Active && state == State.Closing)
            {
                state = State.Closed;
            }

            if (_state == state) // Don't switch twice.
            {
                return;
            }

            try
            {
                switch (state)
                {
                    case State.NotInitialized:
                    {
                        Debug.Assert(false);
                        break;
                    }

                    case State.Active:
                    {
                        Debug.Assert(_state == State.NotInitialized);
                        break;
                    }

                    case State.Closing:
                    case State.ClosingPending:
                    {
                        // Can't change back from closing pending.
                        if (_state >= State.ClosingPending)
                        {
                            return;
                        }
                        break;
                    }

                    case State.Closed:
                    {
                        if (_state >= State.Closed)
                        {
                            return;
                        }

                        // Close the transceiver, this should cause pending IO async calls to return.
                        _transceiver.Close();

                        if (_state > State.NotInitialized && _communicator.TraceLevels.Network >= 1)
                        {
                            var s = new StringBuilder();
                            s.Append("closed ");
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

                            _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
                        }
                        break;
                    }

                    case State.Finished:
                    {
                        Debug.Assert(_state == State.Closed);

                        _transceiver.Destroy();

                        if (_dispatchCount == 0)
                        {
                            Reap();
                        }
                        break;
                    }
                }
            }
            catch (System.Exception ex)
            {
                _communicator.Logger.Error("unexpected connection exception:\n" + ex + "\n" + _transceiver.ToString());
            }

            // We register with the connection monitor if our new state is State.Active. ACM monitors the connection
            // once it's initalized and validated and until it's closed. Timeouts for connection establishement and
            // validation are implemented with a timer instead and setup in the outgoing connection factory.
            if (_monitor != null)
            {
                if (state == State.Active)
                {
                    if (_acmLastActivity > -1)
                    {
                        _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                    }
                    _monitor.Add(this);
                }
                else if (state == State.Closed)
                {
                    _monitor.Remove(this);
                }
            }

            if (_communicator.Observer != null)
            {
                ConnectionState oldState = _connectionStateMap[(int)_state];
                ConnectionState newState = _connectionStateMap[(int)state];
                if (oldState != newState)
                {
                    _observer = _communicator.Observer!.GetConnectionObserver(InitConnectionInfo(), _endpoint,
                        newState, _observer);
                    if (_observer != null)
                    {
                        _observer.Attach();
                    }
                }
                if (_observer != null && state == State.Closed && _exception != null)
                {
                    if (!(_exception is ConnectionClosedException ||
                         _exception is ConnectionIdleException ||
                         _exception is CommunicatorDestroyedException ||
                         _exception is ObjectAdapterDeactivatedException ||
                         (_exception is ConnectionLostException && _state >= State.Closing)))
                    {
                        _observer.Failed(_exception.GetType().FullName!);
                    }
                }
            }
            _state = state;

            System.Threading.Monitor.PulseAll(_mutex);

            if (_state == State.Closing && _dispatchCount == 0)
            {
                try
                {
                    InitiateShutdown();
                }
                catch (Exception ex)
                {
                    SetState(State.Closed, ex);
                }
            }

            // Wait for the pending IO operations to return to terminate the connection with the Finish
            // method and set its state to Finished. It's important in particular for messages being
            // written. We want to make sure WriteAsync returns and correctly reports the send status
            // of the message being sent (it is has been sent it will be removed from the outgoing
            // message queue otherwise it's left in the message queue and the exception closure will be
            // reported by Finish).
            if (_state == State.Closed && _pendingIO == 0)
            {
                if (_outgoingMessages.Count == 0 && _requests.Count == 0 && _closeCallback == null)
                {
                    // Optimization: if there's no user callbacks to call, finish the connection now.
                    SetState(State.Finished);
                }
                else
                {
                    // Otherwise, schedule a task to call Finish()
                    Task.Run(Finish);
                }
            }
        }

        private void TraceReceivedAndUpdateObserver(int length)
        {
            if (_communicator.TraceLevels.Network >= 3 && length > 0)
            {
                _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                    $"received {length} bytes via {_endpoint.Name}\n{this}");
            }

            if (_observer != null && length > 0)
            {
                _observer.ReceivedBytes(length);
            }
        }

        private void TraceSentAndUpdateObserver(int length)
        {
            if (_communicator.TraceLevels.Network >= 3 && length > 0)
            {
                _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                    $"sent {length} bytes via {_endpoint.Name}\n{this}");
            }

            if (_observer != null && length > 0)
            {
                _observer.SentBytes(length);
            }
        }

        private async ValueTask WriteAsync()
        {
            while (true)
            {
                OutgoingMessage message;
                lock (_mutex)
                {
                    if (_state > State.Closing)
                    {
                        return;
                    }
                    Debug.Assert(_outgoingMessages.Count > 0);
                    message = _outgoingMessages.First!.Value;
                    TraceUtil.TraceSend(_communicator, message.OutgoingData!);
                }

                List<ArraySegment<byte>> writeBuffer = message.OutgoingData!;

                // Compress the frame if needed and possible
                // TODO: Benoit: we should consider doing the compression at an earlier stage from the application
                // user thread instead of the WriteAsync task continuation?
                int size = writeBuffer.GetByteCount();
                if (BZip2.IsLoaded && message.Compress)
                {
                    List<ArraySegment<byte>>? compressed = null;
                    if (size >= 100)
                    {
                        compressed = BZip2.Compress(writeBuffer, size, Ice1Definitions.HeaderSize, _compressionLevel);
                    }

                    if (compressed != null)
                    {
                        writeBuffer = compressed!;
                        size = writeBuffer.GetByteCount();
                    }
                    else // Message not compressed, request compressed response, if any.
                    {
                        ArraySegment<byte> header = writeBuffer[0];
                        header[9] = (byte)1; // Write the compression status
                    }
                }

                // Write the frame
                int offset = 0;
                while (offset < size)
                {
                    int bytesSent = await _transceiver.WriteAsync(writeBuffer, offset).ConfigureAwait(false);
                    offset += bytesSent;
                    lock (_mutex)
                    {
                        Debug.Assert(_state < State.Finished); // Finish is only called once WriteAsync returns
                        if (_state > State.Closing)
                        {
                            return;
                        }

                        TraceSentAndUpdateObserver(bytesSent);
                        if (_acmLastActivity > -1)
                        {
                            _acmLastActivity = Time.CurrentMonotonicTimeMillis();
                        }
                    }
                }

                lock (_mutex)
                {
                    _outgoingMessages.RemoveFirst();

                    if (message.OutAsync != null && message.OutAsync.Sent())
                    {
                        // The progress callback is a synchronous callback, we can't call it directly
                        // from this thread since it could block further writes.
                        Task.Run(message.OutAsync.InvokeSent);
                    }

                    if (_outgoingMessages.Count == 0)
                    {
                        return;
                    }
                }
            }
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
                    _connection.SendAsyncRequest(this, false, false);
                }
                catch (RetryException ex)
                {
                    if (Exception(ex.InnerException!))
                    {
                        InvokeException();
                    }
                }
                catch (Exception ex)
                {
                    if (Exception(ex))
                    {
                        InvokeException();
                    }
                }
            }

            private readonly Connection _connection;
        }

        private class HeartbeatTaskCompletionCallback : TaskCompletionCallback<object>
        {
            public HeartbeatTaskCompletionCallback(IProgress<bool>? progress,
                                                   CancellationToken cancellationToken) :
                base(progress, cancellationToken)
            {
            }
            public override void HandleInvokeResponse(bool ok, OutgoingAsyncBase og) => SetResult(null!);
        }

        // TODO: Benoit: Remove with the refactoring of SendAsyncRequest
        private class OutgoingMessage
        {
            internal OutgoingMessage(List<ArraySegment<byte>> requestData, bool compress)
            {
                OutgoingData = requestData;
                Compress = compress;
            }

            internal OutgoingMessage(OutgoingAsyncBase outgoing, List<ArraySegment<byte>> data, bool compress,
                int requestId)
            {
                OutAsync = outgoing;
                OutgoingData = data;
                Compress = compress;
                RequestId = requestId;
            }

            internal List<ArraySegment<byte>>? OutgoingData;
            internal OutgoingAsyncBase? OutAsync;
            internal bool Compress;
            internal int RequestId;
        }

        private enum State
        {
            NotInitialized,
            Active,
            Closing,
            ClosingPending,
            Closed,
            Finished
        };
    }
}
