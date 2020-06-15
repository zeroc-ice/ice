//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
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

    public sealed class Connection
    {
        /// <summary>Get or set the object adapter that dispatches requests received over this connection.
        /// A client can invoke an operation on a server using a proxy, and then set an object adapter for the
        /// outgoing connection used by the proxy in order to receive callbacks. This is useful if the server
        /// cannot establish a connection back to the client, for example because of firewalls.</summary>
        /// <value>The object adapter that dispatches requests for the connection, or null if no adapter is set.
        /// </value>
        public ObjectAdapter? Adapter
        {
            // We don't use a volatile for _adapter to avoid extra-memory barriers when accessing _adapter with
            // the mutex locked.
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
        /// <value>The endpoint from which the connection was created.</value>
        public Endpoint Endpoint { get; }

        // TODO: Remove Timeout after reviewing its usages, it's no longer used by the connection
        internal int Timeout => Endpoint.Timeout;

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
        private Task? _closeTask = null;
        private readonly Communicator _communicator;
        private readonly int _compressionLevel;
        private readonly IConnector? _connector;
        private int _dispatchCount;
        private TaskCompletionSource<bool>? _pendingDispatchTask;
        private Exception? _exception;
        private Action<Connection>? _heartbeatCallback;
        private ConnectionInfo? _info;
        private readonly int _messageSizeMax;
        private IACMMonitor? _monitor;
        private readonly object _mutex = new object();
        private int _nextRequestId;
        private IConnectionObserver? _observer;
        private Task _receiveTask = Task.CompletedTask;
        private readonly Dictionary<int, TaskCompletionSource<IncomingResponseFrame>> _requests =
            new Dictionary<int, TaskCompletionSource<IncomingResponseFrame>>();
        private Task _sendTask = Task.CompletedTask;
        private State _state; // The current state.
        private readonly ITransceiver _transceiver;
        private bool _validated = false;
        private readonly bool _warn;
        private readonly bool _warnUdp;

        // Map internal connection states to Ice.Instrumentation.ConnectionState state values.
        private static readonly ConnectionState[] _connectionStateMap = new ConnectionState[]
        {
            ConnectionState.ConnectionStateValidating,   // State.NotInitialized
            ConnectionState.ConnectionStateActive,       // State.Active
            ConnectionState.ConnectionStateClosing,      // State.Closing
            ConnectionState.ConnectionStateClosed,       // State.Closed
        };

        private static readonly List<ArraySegment<byte>> _closeConnectionFrame =
            new List<ArraySegment<byte>> { Ice1Definitions.CloseConnectionFrame };
        private static readonly List<ArraySegment<byte>> _validateConnectionFrame =
            new List<ArraySegment<byte>> { Ice1Definitions.ValidateConnectionFrame };

        /// <summary>Manually close the connection using the specified closure mode.</summary>
        /// <param name="mode">Determines how the connection will be closed.</param>
        public void Close(ConnectionClose mode)
        {
            // TODO: We should consider removing this method and expose GracefulCloseAsync and CloseAsync
            // instead. This would remove the support for ConnectionClose.GracefullyWithWait. Is it
            // useful? Not waiting implies that the pending requests implies these requests will fail and
            // won't be retried. GracefulCloseAsync could wait for pending requests to complete?
            if (mode == ConnectionClose.Forcefully)
            {
                _ = CloseAsync(new ConnectionClosedLocallyException("connection closed forcefully"));
            }
            else if (mode == ConnectionClose.Gracefully)
            {
                _ = GracefulCloseAsync(new ConnectionClosedLocallyException("connection closed gracefully"));
            }
            else
            {
                Debug.Assert(mode == ConnectionClose.GracefullyWithWait);

                // Wait until all outstanding requests have been completed.
                lock (_mutex)
                {
                    while (_requests.Count > 0)
                    {
                        System.Threading.Monitor.Wait(_mutex);
                    }
                }

                _ = GracefulCloseAsync(new ConnectionClosedLocallyException("connection closed gracefully"));
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
        public T CreateProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx =>
            factory(new Reference(_communicator, this, identity));

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
        public void Heartbeat()
        {
            try
            {
                HeartbeatAsync().AsTask().Wait();
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ex.InnerException;
            }
        }

        /// <summary>Send an asynchronous heartbeat message.</summary>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public async ValueTask HeartbeatAsync(IProgress<bool>? progress = null, CancellationToken cancel = default)
        {
            Task writeTask;
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    throw _exception!;
                }
                writeTask = SendFrameAsync(() => GetProtocolFrameData(_validateConnectionFrame));
            }
            await CancelableTask.WhenAny(writeTask, cancel).ConfigureAwait(false);
            progress?.Report(true);
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
        /// closed. If the callback needs more information about the closure, it can call Connection.throwException.
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
        public string Type() => _transceiver.Transport; // No mutex lock, _type is immutable.

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
            Endpoint = endpoint;
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

        internal async Task GracefulCloseAsync(Exception exception)
        {
            // Don't gracefully close connections for datagram endpoints
            if (!Endpoint.IsDatagram)
            {
                try
                {
                    Task? closingTask = null;
                    lock (_mutex)
                    {
                        if (_state == State.Active)
                        {
                            SetState(State.Closing, exception);
                            if (_dispatchCount > 0)
                            {
                                _pendingDispatchTask = new TaskCompletionSource<bool>();
                            }
                            closingTask = _closeTask = PerformGracefulCloseAsync();
                        }
                        else if (_state == State.Closing)
                        {
                            closingTask = _closeTask;
                        }
                    }
                    if (closingTask != null)
                    {
                        await closingTask.ConfigureAwait(false);
                    }
                }
                catch
                {
                    // Ignore
                }
            }

            await CloseAsync(exception).ConfigureAwait(false);
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
                        if (!Endpoint.IsDatagram)
                        {
                            SendFrameAsync(() => GetProtocolFrameData(_validateConnectionFrame));
                        }
                    }
                }

                // TODO: We still rely on the endpoint timeout here, remove and change the override close timeout to
                // Ice.CloseTimeout (or just rely on ACM timeout)
                int timeout = acm.Timeout;
                if (_state >= State.Closing)
                {
                    timeout = _communicator.OverrideCloseTimeout ?? Endpoint.Timeout;
                }

                // ACM close is always enabled when in the closing state for connection close timeouts.
                if ((_state >= State.Closing || acm.Close != ACMClose.CloseOff) && now >= (_acmLastActivity + timeout))
                {
                    if (_state == State.Closing || acm.Close == ACMClose.CloseOnIdleForceful ||
                       (acm.Close != ACMClose.CloseOnIdle && (_requests.Count > 0)))
                    {
                        //
                        // Close the connection if we didn't receive a heartbeat or if read/write didn't update the
                        // ACM activity in the last period.
                        //
                        _ = CloseAsync(new ConnectionTimeoutException());
                    }
                    else if (acm.Close != ACMClose.CloseOnInvocation && _dispatchCount == 0 && _requests.Count == 0)
                    {
                        //
                        // The connection is idle, close it.
                        //
                        _ = GracefulCloseAsync(new ConnectionIdleException());
                    }
                }
            }
        }

        internal async ValueTask<Task<IncomingResponseFrame>?> SendRequestAsync(OutgoingRequestFrame request,
            bool oneway, bool compress, IInvocationObserver? observer)
        {
            IChildInvocationObserver? childObserver = null;
            Task writeTask;
            Task<IncomingResponseFrame>? responseTask = null;
            lock (_mutex)
            {
                //
                // If the exception is thrown before we even have a chance to send our request, we always try to
                // send the request again.
                //
                if (_exception != null)
                {
                    throw new RetryException(_exception);
                }

                Debug.Assert(_state > State.NotInitialized);
                Debug.Assert(_state < State.Closing);

                int requestId = 0;
                if (!oneway)
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

                    var responseTaskSource = new TaskCompletionSource<IncomingResponseFrame>();
                    _requests[requestId] = responseTaskSource;
                    responseTask = responseTaskSource.Task;
                }

                // Ensure the message isn't bigger than what we can send with the transport.
                // TODO: remove?
                _transceiver.CheckSendSize(request.Size + Ice1Definitions.HeaderSize + 4);

                if (observer != null)
                {
                    childObserver = observer.GetRemoteObserver(InitConnectionInfo(), Endpoint, requestId, request.Size);
                    childObserver?.Attach();
                }

                writeTask = SendFrameAsync(() => GetRequestFrameData(request, requestId, compress));
            }

            try
            {
                await writeTask.ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                childObserver?.Failed(ex.GetType().FullName ?? "System.Exception");
                childObserver?.Detach();
                throw;
            }

            if (oneway)
            {
                childObserver?.Detach();
                return null;
            }
            else
            {
                return WaitForResponseAsync(responseTask!, childObserver);
            }

            static async Task<IncomingResponseFrame> WaitForResponseAsync(Task<IncomingResponseFrame> task,
                IChildInvocationObserver? observer)
            {
                try
                {
                    IncomingResponseFrame response = await task.ConfigureAwait(false);
                    observer?.Reply(response.Size);
                    return response;
                }
                catch (Exception ex)
                {
                    observer?.Failed(ex.GetType().FullName ?? "System.Exception");
                    throw;
                }
                finally
                {
                    observer?.Detach();
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
                int timeout = _communicator.OverrideConnectTimeout ?? Endpoint.Timeout;
                CancellationToken timeoutToken;
                if (timeout > 0)
                {
                     var source = new CancellationTokenSource();
                     source.CancelAfter(timeout);
                     timeoutToken = source.Token;
                }

                // Initialize the transport
                await CancelableTask.WhenAny(_transceiver.InitializeAsync(), timeoutToken).ConfigureAwait(false);

                ArraySegment<byte> readBuffer = default;
                if (!Endpoint.IsDatagram) // Datagram connections are always implicitly validated.
                {
                    if (_connector == null) // The server side has the active role for connection validation.
                    {
                        int offset = 0;
                        while (offset < _validateConnectionFrame.GetByteCount())
                        {
                            ValueTask<int> writeTask = _transceiver.WriteAsync(_validateConnectionFrame, offset);
                            await CancelableTask.WhenAny(writeTask, timeoutToken).ConfigureAwait(false);
                            offset += writeTask.Result;
                        }
                        Debug.Assert(offset == _validateConnectionFrame.GetByteCount());
                    }
                    else // The client side has the passive role for connection validation.
                    {
                        readBuffer = new ArraySegment<byte>(new byte[Ice1Definitions.HeaderSize]);
                        int offset = 0;
                        while (offset < Ice1Definitions.HeaderSize)
                        {
                            ValueTask<int> readTask = _transceiver.ReadAsync(readBuffer, offset);
                            await CancelableTask.WhenAny(readTask, timeoutToken).ConfigureAwait(false);
                            offset += readTask.Result;
                        }

                        Ice1Definitions.CheckHeader(readBuffer.AsSpan(0, 8));
                        var messageType = (Ice1Definitions.FrameType)readBuffer[8];
                        if (messageType != Ice1Definitions.FrameType.ValidateConnection)
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

                    if (!Endpoint.IsDatagram) // Datagram connections are always implicitly validated.
                    {
                        if (_connector == null) // The server side has the active role for connection validation.
                        {
                            TraceSentAndUpdateObserver(_validateConnectionFrame.GetByteCount());
                            ProtocolTrace.TraceSend(_communicator, Ice1Definitions.ValidateConnectionFrame);
                        }
                        else
                        {
                            TraceReceivedAndUpdateObserver(readBuffer.Count);
                            ProtocolTrace.TraceReceived(_communicator, readBuffer);
                        }
                    }

                    if (_communicator.TraceLevels.Network >= 1)
                    {
                        var s = new StringBuilder();
                        if (Endpoint.IsDatagram)
                        {
                            s.Append("starting to ");
                            s.Append(_connector != null ? "send" : "receive");
                            s.Append(" ");
                            s.Append(Endpoint.Name);
                            s.Append(" messages\n");
                            s.Append(_transceiver.ToDetailedString());
                        }
                        else
                        {
                            s.Append(_connector != null ? "established" : "accepted");
                            s.Append(" ");
                            s.Append(Endpoint.Name);
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
            }
            catch (OperationCanceledException)
            {
                _ = CloseAsync(new ConnectTimeoutException());
                throw _exception!;
            }
            catch (Exception ex)
            {
                _ = CloseAsync(ex);
                throw;
            }
        }

        internal void UpdateObserver()
        {
            lock (_mutex)
            {
                // The observer is attached once the connection is active and detached when closed and the last
                // dispatch completed.
                if (_state < State.Active || (_state == State.Closed && _dispatchCount == 0))
                {
                    return;
                }

                _observer = _communicator.Observer?.GetConnectionObserver(InitConnectionInfo(), Endpoint,
                    _connectionStateMap[(int)_state], _observer);
                if (_observer != null)
                {
                    _observer.Attach();
                }
            }
        }

        private async Task CloseAsync(Exception? exception)
        {
            lock (_mutex)
            {
                if (_state < State.Closed)
                {
                    SetState(State.Closed, exception ?? _exception!);
                    if (_dispatchCount > 0)
                    {
                        _pendingDispatchTask ??= new TaskCompletionSource<bool>();
                    }
                    _closeTask = PerformCloseAsync();
                }
            }
            await _closeTask!.ConfigureAwait(false);
        }

        private (List<ArraySegment<byte>>, bool) GetProtocolFrameData(List<ArraySegment<byte>> frame)
        {
            // TODO: Review the protocol tracing? We print out the trace when the frame is about to be sent. It would
            // be simpler to trace the frame before it's queued. This would avoid having these GetXxxData methods.
            // This would also allow to compress the frame from the user thread.
            if (_communicator.TraceLevels.Protocol > 0)
            {
                ProtocolTrace.TraceSend(_communicator, frame[0]);
            }
            return (frame, false);
        }

        private (List<ArraySegment<byte>>, bool) GetRequestFrameData(OutgoingRequestFrame request, int requestId,
            bool compress)
        {
            // TODO: Review the protocol tracing? We print out the trace when the frame is about to be sent. It would
            // be simpler to trace the frame before it's queued. This would avoid having these GetXxxData methods.
            // This would also allow to compress the frame from the user thread.
            List<ArraySegment<byte>> writeBuffer = Ice1Definitions.GetRequestData(request, requestId);
            if (_communicator.TraceLevels.Protocol >= 1)
            {
                ProtocolTrace.TraceFrame(_communicator, writeBuffer[0], request);
            }
            return (writeBuffer, compress);
        }

        private (List<ArraySegment<byte>>, bool) GetResponseFrameData(OutgoingResponseFrame response, int requestId,
            bool compress)
        {
            // TODO: Review the protocol tracing? We print out the trace when the frame is about to be sent. It would
            // be simpler to trace the frame before it's queued. This would avoid having these GetXxxData methods.
            // This would also allow to compress the frame from the user thread.
            List<ArraySegment<byte>> writeBuffer = Ice1Definitions.GetResponseData(response, requestId);
            if (_communicator.TraceLevels.Protocol > 0)
            {
                ProtocolTrace.TraceFrame(_communicator, writeBuffer[0], response);
            }
            return (writeBuffer, compress);
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
                info.ConnectionId = Endpoint.ConnectionId;
                info.AdapterName = _adapter != null ? _adapter.Name : "";
                info.Incoming = _connector == null;
            }
            return _info;
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
                        SendFrameAsync(() => GetResponseFrameData(response, current.RequestId, compressionStatus > 0));
                    }

                    // Decrease the dispatch count
                    Debug.Assert(_dispatchCount > 0);
                    if (--_dispatchCount == 0 && _pendingDispatchTask != null)
                    {
                        Debug.Assert(_state > State.Active);
                        _pendingDispatchTask.SetResult(true);
                    }
                }

                dispatchObserver?.Detach();
            }
        }

        private (Func<ValueTask>?, ObjectAdapter?) ParseFrame(ArraySegment<byte> readBuffer)
        {
            Func<ValueTask>? incoming = null;
            ObjectAdapter? adapter = null;
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    throw _exception!;
                }

                // The magic and version fields have already been checked.
                var messageType = (Ice1Definitions.FrameType)readBuffer[8];
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
                    case Ice1Definitions.FrameType.CloseConnection:
                    {
                        ProtocolTrace.TraceReceived(_communicator, readBuffer);
                        if (Endpoint.IsDatagram)
                        {
                            if (_warn)
                            {
                                _communicator.Logger.Warning(
                                    $"ignoring close connection message for datagram connection:\n{this}");
                            }
                        }
                        else
                        {
                            throw new ConnectionClosedByPeerException();
                        }
                        break;
                    }

                    case Ice1Definitions.FrameType.Request:
                    {
                        if (_state >= State.Closing)
                        {
                            ProtocolTrace.Trace(
                                "received request during closing\n(ignored by server, client will retry)",
                                _communicator,
                                readBuffer);
                        }
                        else
                        {
                            var request = new IncomingRequestFrame(_communicator,
                                readBuffer.Slice(Ice1Definitions.HeaderSize + 4));
                            ProtocolTrace.TraceFrame(_communicator, readBuffer, request);
                            if (_adapter == null)
                            {
                                throw new ObjectNotExistException(request.Identity, request.Facet,
                                    request.Operation);
                            }
                            else
                            {
                                adapter = _adapter;
                                int requestId = InputStream.ReadInt(readBuffer.AsSpan(Ice1Definitions.HeaderSize, 4));
                                var current = new Current(_adapter, request, requestId, this);
                                incoming = () => InvokeAsync(current, request, compressionStatus);
                                ++_dispatchCount;
                            }
                        }
                        break;
                    }

                    case Ice1Definitions.FrameType.RequestBatch:
                    {
                        if (_state >= State.Closing)
                        {
                            ProtocolTrace.Trace(
                                "received batch request during closing\n(ignored by server, client will retry)",
                                _communicator,
                                readBuffer);
                        }
                        else
                        {
                            ProtocolTrace.TraceReceived(_communicator, readBuffer);
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

                    case Ice1Definitions.FrameType.Reply:
                    {
                        int requestId = InputStream.ReadInt(readBuffer.AsSpan(14, 4));
                        var responseFrame = new IncomingResponseFrame(_communicator,
                            readBuffer.Slice(Ice1Definitions.HeaderSize + 4));
                        ProtocolTrace.TraceFrame(_communicator, readBuffer, responseFrame);
                        if (_requests.TryGetValue(requestId, out TaskCompletionSource<IncomingResponseFrame>? response))
                        {
                            _requests.Remove(requestId);
                            response.SetResult(responseFrame);
                            if (_requests.Count == 0)
                            {
                                System.Threading.Monitor.PulseAll(_mutex); // Notify threads blocked in Close()
                            }
                        }
                        break;
                    }

                    case Ice1Definitions.FrameType.ValidateConnection:
                    {
                        ProtocolTrace.TraceReceived(_communicator, readBuffer);
                        if (_heartbeatCallback != null)
                        {
                            Action<Connection> callback = _heartbeatCallback;
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
                        ProtocolTrace.Trace(
                            "received unknown message\n(invalid, closing connection)",
                            _communicator,
                            readBuffer);
                        throw new InvalidDataException(
                            $"received ice1 frame with unknown message type `{messageType}'");
                    }
                }
            }
            return (incoming, adapter);
        }

        private async Task PerformGracefulCloseAsync()
        {
            if (!(_exception is ConnectionClosedByPeerException))
            {
                // Wait for the all the dispatch to be completed to ensure the responses are sent.
                if (_pendingDispatchTask != null)
                {
                    await _pendingDispatchTask.Task.ConfigureAwait(false);
                }

                // Write and wait for the close connection frame to be written
                await SendFrameAsync(() => GetProtocolFrameData(_closeConnectionFrame)).ConfigureAwait(false);
            }

            // Notify the transport of the graceful connection closure.
            try
            {
                await _transceiver.ClosingAsync(_exception!).ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                _communicator.Logger.Error("unexpected connection exception:\n" + ex + "\n" + _transceiver.ToString());
            }

            // Wait for the connection closure from the peer
            try
            {
                await _receiveTask.ConfigureAwait(false);
            }
            catch
            {
            }
        }

        private async Task PerformCloseAsync()
        {
            // Close the transceiver, this should cause pending IO async calls to return.
            try
            {
                _transceiver.ThreadSafeClose();
            }
            catch (Exception ex)
            {
                _communicator.Logger.Error("unexpected connection exception:\n" + ex + "\n" + _transceiver.ToString());
            }

            if (_state > State.NotInitialized && _communicator.TraceLevels.Network >= 1)
            {
                var s = new StringBuilder();
                s.Append("closed ");
                s.Append(Endpoint.Name);
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

            // Wait for pending receives and sends to complete
            try
            {
                await _sendTask.ConfigureAwait(false);
            }
            catch
            {
            }
            try
            {
                await _receiveTask.ConfigureAwait(false);
            }
            catch
            {
            }

            // Destroy the transport
            try
            {
                _transceiver.Destroy();
            }
            catch (Exception ex)
            {
                _communicator.Logger.Error("unexpected connection exception:\n" + ex + "\n" + _transceiver.ToString());
            }

            // Notify pending requests of the failure
            foreach (TaskCompletionSource<IncomingResponseFrame> response in _requests.Values)
            {
                response.SetException(_exception!);
            }
            _requests.Clear();

            // Invoke the close callback
            try
            {
                _closeCallback?.Invoke(this);
            }
            catch (Exception ex)
            {
                _communicator.Logger.Error($"connection callback exception:\n{ex}\n{this}");
            }

            // Wait for all the dispatch to complete before reaping the connection and notifying the observer
            if (_pendingDispatchTask != null)
            {
                await _pendingDispatchTask.Task.ConfigureAwait(false);
            }

            _monitor?.Reap(this);
            _observer?.Detach();
        }

        private async ValueTask<ArraySegment<byte>> PerformReceiveFrameAsync()
        {
            // Read header
            ArraySegment<byte> readBuffer;
            if (Endpoint.IsDatagram)
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

                // Connection is validated on first message. This is only used by setState() to check whether or
                // not we can print a connection warning (a client might close the connection forcefully if the
                // connection isn't validated, we don't want to print a warning in this case).
                _validated = true;
            }

            // Read the remainder of the message if needed
            if (!Endpoint.IsDatagram)
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

                    // Trace the receive progress within the loop as we might be receiving significant amount
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
            return readBuffer;
        }

        private async ValueTask PerformSendFrameAsync(Func<(List<ArraySegment<byte>>, bool)> getFrameData)
        {
            List<ArraySegment<byte>> writeBuffer;
            bool compress;
            lock (_mutex)
            {
                if (_state >= State.Closed)
                {
                    throw _exception!;
                }
                (writeBuffer, compress) = getFrameData();
            }

            // Compress the frame if needed and possible
            // TODO: Benoit: we should consider doing the compression at an earlier stage from the application
            // user thread instead?
            int size = writeBuffer.GetByteCount();
            if (BZip2.IsLoaded && compress)
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
                    header[9] = 1; // Write the compression status
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
        }

        private async ValueTask ReceiveAndDispatchFrameAsync()
        {
            try
            {
                while (true)
                {
                    ArraySegment<byte> readBuffer = await ReceiveFrameAsync().ConfigureAwait(false);
                    if (readBuffer.Count == 0)
                    {
                        // If received without reading, start another receive. This can occur with datagram transports
                        // if the datagram was truncated.
                        continue;
                    }

                    (Func<ValueTask>? incoming, ObjectAdapter? adapter) = ParseFrame(readBuffer);
                    if (incoming != null)
                    {
                        bool serialize = adapter?.SerializeDispatch ?? false;
                        if (!serialize)
                        {
                            // Start a new receive task before running the incoming dispatch. We start the new receive
                            // task from a separate task because ReadAsync could complete synchronously and we don't
                            // want the dispatch from this read to run before we actually ran the dispatch from this
                            // block. An alternative could be to start a task to run the incoming dispatch and continue
                            // reading with this loop. It would have a negative impact on latency however since
                            // execution of the incoming dispatch would potentially require a thread context switch.
                            if (adapter?.TaskScheduler != null)
                            {
                                _ = TaskRun(ReceiveAndDispatchFrameAsync, adapter.TaskScheduler);
                            }
                            else
                            {
                                _ = Task.Run(ReceiveAndDispatchFrameAsync);
                            }
                        }

                        // Run the received incoming frame
                        if (adapter?.TaskScheduler != null)
                        {
                            await TaskRun(incoming, adapter.TaskScheduler).ConfigureAwait(false);
                        }
                        else
                        {
                            await incoming().ConfigureAwait(false);
                        }

                        // Don't continue reading from this task if we're not using serialization, we started
                        // another receive task above.
                        if (!serialize)
                        {
                            return;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                await CloseAsync(ex);
            }

            static async ValueTask TaskRun(Func<ValueTask> func, TaskScheduler scheduler)
            {
                // First await for the dispatch to be ran on the task scheduler.
                ValueTask task = await Task.Factory.StartNew(func, default, TaskCreationOptions.None,
                    scheduler).ConfigureAwait(false);

                // Now wait for the async dispatch to complete.
                await task.ConfigureAwait(false);
            }
        }

        private async ValueTask<ArraySegment<byte>> ReceiveFrameAsync()
        {
            Task<ArraySegment<byte>> task;
            lock (_mutex)
            {
                if (_state == State.Closed)
                {
                    throw _exception!;
                }
                ValueTask<ArraySegment<byte>> readTask = PerformAsync(this);
                if (readTask.IsCompleted)
                {
                    _receiveTask = Task.CompletedTask;
                    return readTask.Result;
                }
                else
                {
                    _receiveTask = task = readTask.AsTask();
                }
            }
            return await task.ConfigureAwait(false);

            static async ValueTask<ArraySegment<byte>> PerformAsync(Connection self)
            {
                try
                {
                    return await self.PerformReceiveFrameAsync().ConfigureAwait(false);
                }
                catch (ConnectionClosedByPeerException ex)
                {
                    _ = self.GracefulCloseAsync(ex);
                    throw;
                }
                catch (Exception ex)
                {
                    _ = self.CloseAsync(ex);
                    throw;
                }
            }
        }

        private Task SendFrameAsync(Func<(List<ArraySegment<byte>>, bool)> getFrameData)
        {
            lock (_mutex)
            {
                Debug.Assert(_state < State.Closed);
                ValueTask sendTask = QueueAsync(this, _sendTask, getFrameData);
                _sendTask = sendTask.IsCompleted ? Task.CompletedTask : sendTask.AsTask();
                return _sendTask;
            }

            static async ValueTask QueueAsync(Connection self, Task previous,
                Func<(List<ArraySegment<byte>>, bool)> getFrameData)
            {
                // Wait for the previous write to complete
                await previous.ConfigureAwait(false);

                // Perform the write
                try
                {
                    await self.PerformSendFrameAsync(getFrameData).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    _ = self.CloseAsync(ex);
                    throw;
                }
            }
        }

        private void SetState(State state, Exception? exception = null)
        {
            // If SetState() is called with an exception, then only closed and closing states are permissible.
            Debug.Assert((exception == null && state < State.Closing) || (exception != null && state >= State.Closing));

            if (_exception == null && exception != null)
            {
                // If we are in closed state, an exception must be set.
                Debug.Assert(_state != State.Closed);

                _exception = exception;

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

            Debug.Assert(_state != state); // Don't switch twice.
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
                    // Start the asynchronous operation from the thread pool to prevent eventually reading
                    // synchronously new frames from this thread.
                    _ = Task.Run(ReceiveAndDispatchFrameAsync);
                    break;
                }

                case State.Closing:
                {
                    Debug.Assert(_state == State.Active);
                    break;
                }

                case State.Closed:
                {
                    Debug.Assert(_state < State.Closed);
                    break;
                }
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
                    _observer = _communicator.Observer!.GetConnectionObserver(InitConnectionInfo(), Endpoint,
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
        }

        private void TraceReceivedAndUpdateObserver(int length)
        {
            if (_communicator.TraceLevels.Network >= 3 && length > 0)
            {
                _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                    $"received {length} bytes via {Endpoint.Name}\n{this}");
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
                    $"sent {length} bytes via {Endpoint.Name}\n{this}");
            }

            if (_observer != null && length > 0)
            {
                _observer.SentBytes(length);
            }
        }

        private enum State
        {
            NotInitialized,
            Active,
            Closing,
            Closed,
        };
    }
}
