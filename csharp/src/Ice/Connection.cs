//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    /// <summary>Determines the behavior when manually closing a connection.</summary>
    public enum ConnectionClose
    {
        /// <summary>Close the connection immediately without sending a close connection protocol message to the peer
        /// and waiting for the peer to acknowledge it.</summary>
        Forcefully,
        /// <summary>Close the connection by notifying the peer but do not wait for pending outgoing invocations to
        /// complete.</summary>
        Gracefully,
    }

    /// <summary>The state of an Ice connection</summary>
    public enum ConnectionState : byte
    {
        /// <summary>The connection is being validated.</summary>
        Validating = 0,
        /// <summary>The connection is active and can send and receive messages.</summary>
        Active,
        /// <summary>The connection is being gracefully shutdown and waits for the peer to close its end of the
        /// connection.</summary>
        Closing,
        /// <summary>The connection is closed and eventually waits for potential dispatch to be finished before being
        /// destroyed.</summary>
        Closed
    }

    /// <summary>Represents a connection used to send and receive Ice frames.</summary>
    public abstract class Connection
    {
        /// <summary>Gets or set the connection Acm (Active Connection Management) configuration.</summary>
        public Acm Acm
        {
            get
            {
                lock (_mutex)
                {
                    return _monitor?.Acm ?? new Acm();
                }
            }
            set
            {
                if (_manager != null)
                {
                    lock (_mutex)
                    {
                        if (_state >= ConnectionState.Closing)
                        {
                            return;
                        }

                        if (_state == ConnectionState.Active)
                        {
                            _monitor?.Remove(this);
                        }

                        _monitor = value == _manager.AcmMonitor.Acm ?
                            _manager.AcmMonitor : new ConnectionAcmMonitor(value, _communicator.Logger);

                        if (_state == ConnectionState.Active)
                        {
                            _monitor?.Add(this);
                        }
                    }
                }
            }
        }

        /// <summary>Gets or sets the object adapter that dispatches requests received over this connection.
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

        /// <summary>Get the connection ID which was used to create the connection.</summary>
        /// <value>The connection ID used to create the connection.</value>
        public string ConnectionId { get; }

        /// <summary>Get the endpoint from which the connection was created.</summary>
        /// <value>The endpoint from which the connection was created.</value>
        public Endpoint Endpoint { get; }

        /// <summary>True for incoming connections false otherwise.</summary>
        public bool IsIncoming => _connector == null;

        /// <summary>The protocol used by the connection.</summary>
        public Protocol Protocol => Endpoint.Protocol;

        internal bool Active
        {
            get
            {
                lock (_mutex)
                {
                    return _state > ConnectionState.Validating && _state < ConnectionState.Closing;
                }
            }
        }

        // The connector from which the connection was created. This is used by the outgoing connection factory.
        internal IConnector Connector => _connector!;

        // The endpoints which are associated with this connection. This is populated by the outgoing connection
        // factory when an endpoint resolves to the same connector as this connection's connector. Two endpoints
        // can be different but resolve to the same connector (e.g.: endpoints with the IPs "::1", "0:0:0:0:0:0:0:1"
        // or "localhost" are different endpoints but they all end up resolving to the same connector and can use
        // the same connection).
        internal List<Endpoint> Endpoints { get; }

        private protected MultiStreamTransceiver Transceiver { get; }

        private volatile Task _acceptStreamTask = Task.CompletedTask;
        private ObjectAdapter? _adapter;
        private TransceiverStream? _controlStream;
        private EventHandler? _closed;
        private Task? _closeTask;
        private readonly Communicator _communicator;
        private readonly IConnector? _connector;
        private volatile Exception? _exception;
        private readonly IConnectionManager? _manager;
        private IAcmMonitor? _monitor;
        private readonly object _mutex = new object();
        private ConnectionState _state; // The current state.

        /// <summary>Manually closes the connection using the specified closure mode.</summary>
        /// <param name="mode">Determines how the connection will be closed.</param>
        public void Close(ConnectionClose mode)
        {
            // TODO: Should we simply make AbortAsync/CloseAsync public instead?
            if (mode == ConnectionClose.Forcefully)
            {
                _ = AbortAsync(new ConnectionClosedLocallyException("connection closed forcefully"));
            }
            else if (mode == ConnectionClose.Gracefully)
            {
                _ = CloseAsync(new ConnectionClosedLocallyException("connection closed gracefully"));
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

        /// <summary>Sends a heartbeat frame.</summary>
        public void Heartbeat()
        {
            try
            {
                HeartbeatAsync().AsTask().Wait();
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Sends an asynchronous heartbeat frame.</summary>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public async ValueTask HeartbeatAsync(IProgress<bool>? progress = null, CancellationToken cancel = default)
        {
            await Transceiver.PingAsync(cancel).ConfigureAwait(false);
            progress?.Report(true);
        }

        /// <summary>This event is raised when the connection is closed. If the subscriber needs more information about
        /// the closure, it can call Connection.ThrowException. The connection object is passed as the event sender
        /// argument.</summary>
        public event EventHandler? Closed
        {
            add
            {
                lock (_mutex)
                {
                    if (_state >= ConnectionState.Closed)
                    {
                        Task.Run(() => value?.Invoke(this, EventArgs.Empty));
                    }
                    _closed += value;
                }
            }
            remove => _closed -= value;
        }

        /// <summary>This event is raised when the connection receives a heartbeat. The connection object is passed as
        /// the event sender argument.</summary>
        public event EventHandler? HeartbeatReceived
        {
            add
            {
                Transceiver.Ping += value;
            }
            remove
            {
                Transceiver.Ping -= value;
            }
        }

        /// <summary>Throws an exception indicating the reason for connection closure. For example,
        /// ConnectionClosedByPeerException is raised if the connection was closed gracefully by the peer, whereas
        /// ConnectionClosedLocallyException is raised if the connection was manually closed by the application. This
        /// operation does nothing if the connection is not yet closed.</summary>
        public void ThrowException()
        {
            if (_exception != null)
            {
                Debug.Assert(_state >= ConnectionState.Closing);
                throw _exception;
            }
        }

        /// <summary>Returns a description of the connection as human readable text, suitable for logging or error
        /// messages.</summary>
        /// <returns>The description of the connection as human readable text.</returns>
        public override string ToString() => Transceiver.ToString()!;

        internal Connection(
            IConnectionManager? manager,
            Endpoint endpoint,
            MultiStreamTransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter)
        {
            _communicator = endpoint.Communicator;
            _manager = manager;
            _monitor = manager?.AcmMonitor;
            Transceiver = transceiver;
            _connector = connector;
            ConnectionId = connectionId;
            Endpoint = endpoint;
            Endpoints = endpoint == null ? new List<Endpoint>() : new List<Endpoint>() { endpoint };
            _adapter = adapter;
            _state = ConnectionState.Validating;
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

        internal async Task CloseAsync(Exception exception)
        {
            try
            {
                Task closeTask;
                lock (_mutex)
                {
                    if (_state == ConnectionState.Active && _controlStream != null)
                    {
                        SetState(ConnectionState.Closing, exception);
                        _closeTask ??= PerformCloseAsync(exception);
                        Debug.Assert(_closeTask != null);
                    }
                    else if (_closeTask == null)
                    {
                        _ = AbortAsync(exception);
                    }
                    closeTask = _closeTask!;
                }

                await closeTask.ConfigureAwait(false);
            }
            catch
            {
            }

            async Task PerformCloseAsync(Exception exception)
            {
                // Abort outgoing streams and get the largest incoming stream ID. With Ice2, we don't wait for the
                // incoming streams to complete before sending the close frame but instead provide the ID of the
                // latest incoming stream ID to the peer. The peer will close the connection once it received the
                // response for this stream ID.
                long lastIncomingStreamId = Transceiver.AbortStreams(exception, stream => !stream.IsIncoming);

                if (Endpoint.Protocol == Protocol.Ice1)
                {
                    // With Ice1, we first wait for all incoming streams to complete before sending the close frame.
                    await Transceiver.WaitForEmptyStreamsAsync().ConfigureAwait(false);
                }

                CancellationTokenSource? source = null;
                TimeSpan timeout = _communicator.CloseTimeout;
                if (timeout > TimeSpan.Zero)
                {
                    source = new CancellationTokenSource(timeout);
                }

                try
                {
                    CancellationToken cancel = source?.Token ?? default;

                    // TODO: send a better message?
                    string message = exception.ToString();

                    // Write the close frame
                    await _controlStream!.SendCloseFrameAsync(lastIncomingStreamId,
                                                              message,
                                                              cancel).ConfigureAwait(false);

                    // Wait the peer to close the stream.
                    while (true)
                    {
                        await _acceptStreamTask.WaitAsync(cancel).ConfigureAwait(false);
                    }
                }
                catch (OperationCanceledException)
                {
                    await AbortAsync(new TimeoutException());
                }
                catch (Exception ex)
                {
                    await AbortAsync(ex);
                }
                finally
                {
                    source?.Dispose();
                }
            }
        }

        internal TransceiverStream CreateStream(bool bidirectional)
        {
            // Ensure the stream is created in the active state only, no new streams should be created if the
            // connection is closing or closed.
            lock (_mutex)
            {
                if (_exception != null)
                {
                    throw _exception;
                }
                Debug.Assert(_state == ConnectionState.Active);
                return Transceiver.CreateStream(bidirectional);
            }
        }

        internal async Task WaitForCloseAsync(TransceiverStream peerControlStream)
        {
            try
            {
                // Wait to receive the close frame on the control stream.
                (long lastStreamId, string message) =
                    await peerControlStream.ReceiveCloseFrameAsync().ConfigureAwait(false);

                Task closeTask;
                lock (_mutex)
                {
                    if (_state == ConnectionState.Active)
                    {
                        SetState(ConnectionState.Closing, new ConnectionClosedByPeerException(message));
                        closeTask = PerformCloseAsync(lastStreamId, _exception!);
                        if (_closeTask == null)
                        {
                            _closeTask = closeTask;
                        }
                    }
                    else
                    {
                        closeTask = _closeTask!;
                    }
                }

                await closeTask.ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                await AbortAsync(ex);
            }

            async Task PerformCloseAsync(long lastStreamId, Exception exception)
            {
                // Abort non-processed outgoing streams and all incoming streams.
                Transceiver.AbortStreams(exception, stream => stream.IsIncoming || stream.Id > lastStreamId);

                // Wait for all the streams to complete.
                await Transceiver.WaitForEmptyStreamsAsync().ConfigureAwait(false);

                // Close the transport
                await Transceiver.CloseAsync(exception, CancellationToken.None);

                // Abort the connection once all the streams have completed.
                await AbortAsync(exception).ConfigureAwait(false);
            }
        }

        internal void Monitor(TimeSpan now, Acm acm)
        {
            lock (_mutex)
            {
                if (_state != ConnectionState.Active)
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
                // if (_state == ConnectionState.Active &&
                //     (acm.Heartbeat == AcmHeartbeat.Always ||
                //     (acm.Heartbeat != AcmHeartbeat.Off && now >= (Transceiver.LastActivity + (acm.Timeout / 4)))))
                // {
                //     if (acm.Heartbeat != AcmHeartbeat.OnDispatch || _dispatchCount > 0)
                //     {
                //         Debug.Assert(_state == ConnectionState.Active);
                //         if (!Endpoint.IsDatagram)
                //         {
                //             ValueTask ignored = Transceiver.PingAsync(default);
                //         }
                //     }
                // }

                // if (acm.Close != AcmClose.Off && now >= Transceiver.LastActivity + acm.Timeout)
                // {
                //     if (acm.Close == AcmClose.OnIdleForceful || (acm.Close != AcmClose.OnIdle && (_requests.Count > 0)))
                //     {
                //         // Close the connection if we didn't receive a heartbeat or if read/write didn't update the
                //         // ACM activity in the last period.
                //         _ = AbortAsync(new ConnectionTimeoutException());
                //     }
                //     else if (acm.Close != AcmClose.OnInvocation && _dispatchCount == 0 && _requests.Count == 0)
                //     {
                //         // The connection is idle, close it.
                //         _ = CloseAsync(new ConnectionIdleException());
                //     }
                // }
            }
        }

        internal async Task StartAsync()
        {
            CancellationTokenSource? source = null;
            try
            {
                CancellationToken cancel = default;
                TimeSpan timeout = _communicator.ConnectTimeout;
                if (timeout > TimeSpan.Zero)
                {
                    source = new CancellationTokenSource(timeout);
                    cancel = source.Token;
                }

                // Initialize the transport.
                await Transceiver.InitializeAsync(cancel).ConfigureAwait(false);

                if (!Endpoint.IsDatagram)
                {
                    // Create the control stream and send the initialize frame
                    _controlStream = await Transceiver.SendInitializeFrameAsync(cancel).ConfigureAwait(false);

                    // Wait for the peer control stream to be accepted and read the initialize frame
                    TransceiverStream peerControlStream =
                        await Transceiver.ReceiveInitializeFrameAsync(cancel).ConfigureAwait(false);

                    // Setup a task to wait for the close frame on the peer's control stream.
                    _ = Task.Run(async () => await WaitForCloseAsync(peerControlStream).ConfigureAwait(false));
                }

                Transceiver.Initialized();

                lock (_mutex)
                {
                    if (_state >= ConnectionState.Closed)
                    {
                        throw _exception!;
                    }
                    SetState(ConnectionState.Active);
                }
            }
            catch (OperationCanceledException)
            {
                _ = AbortAsync(new ConnectTimeoutException());
                throw _exception!;
            }
            catch (Exception ex)
            {
                _ = AbortAsync(ex);
                throw;
            }
            finally
            {
                source?.Dispose();
            }
        }

        internal void UpdateObserver()
        {
            lock (_mutex)
            {
                // The observer is attached once the connection is active and detached once the close task completes.
                if (_state < ConnectionState.Active || (_state == ConnectionState.Closed && _closeTask!.IsCompleted))
                {
                    return;
                }

                Transceiver.Observer = _communicator.Observer?.GetConnectionObserver(this,
                                                                                     _state,
                                                                                     Transceiver.Observer);
            }
        }

        private async Task AbortAsync(Exception exception)
        {
            lock (_mutex)
            {
                if (_state < ConnectionState.Closed)
                {
                    SetState(ConnectionState.Closed, exception);
                    _closeTask = PerformAbortAsync();
                }
            }
            await _closeTask!.ConfigureAwait(false);

            async Task PerformAbortAsync()
            {
                await Transceiver.AbortAsync(exception).ConfigureAwait(false);

                // Yield to ensure the code below is executed without the mutex locked. PerformCloseAsync, the code
                // below is not safe to call with this mutex locked.
                await Task.Yield();

                Transceiver.Dispose();

                // Raise the Closed event
                try
                {
                    _closed?.Invoke(this, EventArgs.Empty);
                }
                catch (Exception ex)
                {
                    _communicator.Logger.Error($"connection callback exception:\n{ex}\n{this}");
                }

                // Remove the connection from the factory, must be called without the connection mutex locked
                _manager?.Remove(this);
            }
        }

        private async ValueTask AcceptStreamAsync()
        {
            TransceiverStream? stream = null;
            try
            {
                // Accept a new stream
                while (true)
                {
                    stream = await Transceiver.AcceptStreamAsync(CancellationToken.None).ConfigureAwait(false);
                    if (_exception != null)
                    {
                        // Ignore the stream if the connection is being closed. The loop will eventually terminate when
                        // the peer closes the connection.
                        stream.Dispose();
                    }
                    else
                    {
                        // Start a new accept stream task
                        _acceptStreamTask = Task.Run(async () => await AcceptStreamAsync().ConfigureAwait(false));
                        break;
                    }
                }

                ObjectAdapter? adapter = _adapter;

                using var cancelSource = new CancellationTokenSource();
                CancellationToken cancel = cancelSource.Token;
                if (stream.IsBidirectional)
                {
                    // Be notified if the peer reset the stream and cancel the dispatch cancellation token source.
                    stream.Reset += () => cancelSource.Cancel();
                }

                // Receive the request from the stream
                (IncomingRequestFrame request, bool fin)
                    = await stream.ReceiveRequestFrameAsync(cancel).ConfigureAwait(false);

                OutgoingResponseFrame response;
                if (adapter == null)
                {
                    if (stream.IsBidirectional)
                    {
                        var exception = new ObjectNotExistException(request.Identity, request.Facet, request.Operation);
                        response = new OutgoingResponseFrame(request, exception);
                        await stream.SendResponseFrameAsync(response, true, cancel).ConfigureAwait(false);
                    }
                    return;
                }

                var current = new Current(adapter, request, stream, fin, this, cancel);

                // Dispatch the request and get the response
                if (adapter.TaskScheduler != null)
                {
                    (response, fin) = await TaskRun(() => adapter.DispatchAsync(request, stream, current),
                                                    cancel,
                                                    adapter.TaskScheduler).ConfigureAwait(false);
                }
                else
                {
                    (response, fin) = await adapter.DispatchAsync(request, stream, current).ConfigureAwait(false);
                }

                if (stream.IsBidirectional)
                {
                    // Send the response over the stream
                    await stream.SendResponseFrameAsync(response, fin, cancel);

                    if (!fin)
                    {
                        // TODO: send streamable data.
                    }
                }
            }
            catch (OperationCanceledException)
            {
                // Ignore, the dispatch got canceled
            }
            catch (StreamResetByPeerException)
            {
                // Ignore, the peer closed the stream.
            }
            catch (Exception ex)
            {
                // Other exceptions are considered fatal, abort the connection
                _ = AbortAsync(ex);
                throw;
            }
            finally
            {
                stream?.Dispose();
            }

            static async ValueTask<(OutgoingResponseFrame, bool)> TaskRun(
                Func<ValueTask<(OutgoingResponseFrame, bool)>> func,
                CancellationToken cancel,
                TaskScheduler scheduler)
            {
                // First await for the dispatch to be ran on the task scheduler.
                ValueTask<(OutgoingResponseFrame, bool)> task =
                    await Task.Factory.StartNew(func, cancel, TaskCreationOptions.None, scheduler).ConfigureAwait(false);

                // Now wait for the async dispatch to complete.
                return await task.ConfigureAwait(false);
            }
        }

        private void SetState(ConnectionState state, Exception? exception = null)
        {
            // If SetState() is called with an exception, then only closed and closing states are permissible.
            Debug.Assert((exception == null && state < ConnectionState.Closing) ||
                         (exception != null && state >= ConnectionState.Closing));

            if (_exception == null && exception != null)
            {
                // If we are in closed state, an exception must be set.
                Debug.Assert(_state != ConnectionState.Closed);

                _exception = exception;

                // We don't warn if we are not validated.
                if (_state > ConnectionState.Validating && _communicator.WarnConnections)
                {
                    // Don't warn about certain expected exceptions.
                    if (!(_exception is ConnectionClosedException ||
                         _exception is ConnectionIdleException ||
                         _exception is ObjectDisposedException ||
                         (_exception is ConnectionLostException && _state >= ConnectionState.Closing)))
                    {
                        _communicator.Logger.Warning($"connection exception:\n{_exception}\n{this}");
                    }
                }
            }

            Debug.Assert(_state != state); // Don't switch twice.
            switch (state)
            {
                case ConnectionState.Validating:
                {
                    Debug.Assert(false);
                    break;
                }

                case ConnectionState.Active:
                {
                    Debug.Assert(_state == ConnectionState.Validating);
                    // Start the asynchronous operation from the thread pool to prevent eventually reading
                    // synchronously new frames from this thread.
                    _acceptStreamTask = Task.Run(async () => await AcceptStreamAsync().ConfigureAwait(false));
                    break;
                }

                case ConnectionState.Closing:
                {
                    Debug.Assert(_state == ConnectionState.Active);
                    break;
                }

                case ConnectionState.Closed:
                {
                    Debug.Assert(_state < ConnectionState.Closed);
                    break;
                }
            }

            // We register with the connection monitor if our new state is State.Active. ACM monitors the connection
            // once it's initialized and validated and until it's closed. Timeouts for connection establishment and
            // validation are implemented with a timer instead and setup in the outgoing connection factory.
            if (state == ConnectionState.Active)
            {
                _monitor?.Add(this);
            }
            else if (_state == ConnectionState.Active)
            {
                Debug.Assert(state > ConnectionState.Active);
                _monitor?.Remove(this);
            }

            if (_communicator.Observer != null)
            {
                if (_state != state)
                {
                    Transceiver.Observer =
                        _communicator.Observer.GetConnectionObserver(this, state, Transceiver.Observer);
                }
                if (Transceiver.Observer != null && state == ConnectionState.Closed)
                {
                    Debug.Assert(_exception != null);
                    if (!(_exception is ConnectionClosedException ||
                          _exception is ConnectionIdleException ||
                          _exception is ObjectDisposedException ||
                         (_exception is ConnectionLostException && _state >= ConnectionState.Closing)))
                    {
                        Transceiver.Observer.Failed(_exception.GetType().FullName!);
                    }
                }
            }
            _state = state;
        }
    }

    /// <summary>Represents a colocated connection to an object adapter</summary>
    public class ColocatedConnection : Connection
    {
        internal ColocatedConnection(
            IConnectionManager? manager,
            Endpoint endpoint,
            ColocatedTransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter)
            : base(manager, endpoint, transceiver, connector, connectionId, adapter)
        {
        }
    }

    /// <summary>Represents a connection to an IP-endpoint.</summary>
    public abstract class IPConnection : Connection
    {
        private protected readonly MultiStreamTransceiverWithUnderlyingTransceiver _transceiver;

        /// <summary>The socket local IP-endpoint or null if it is not available.</summary>
        public System.Net.IPEndPoint? LocalEndpoint
        {
            get
            {
                try
                {
                    return _transceiver.Underlying.Socket?.LocalEndPoint as System.Net.IPEndPoint;
                }
                catch
                {
                    return null;
                }
            }
        }

        /// <summary>The socket remote IP-endpoint or null if it is not available.</summary>
        public System.Net.IPEndPoint? RemoteEndpoint
        {
            get
            {
                try
                {
                    return _transceiver.Underlying.Socket?.RemoteEndPoint as System.Net.IPEndPoint;
                }
                catch
                {
                    return null;
                }
            }
        }

        protected IPConnection(
            IConnectionManager? manager,
            Endpoint endpoint,
            MultiStreamTransceiverWithUnderlyingTransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter)
            : base(manager, endpoint, transceiver, connector, connectionId, adapter) => _transceiver = transceiver;
    }

    /// <summary>Represents a connection to a TCP-endpoint.</summary>
    public class TcpConnection : IPConnection
    {
        /// <summary>Gets a Boolean value that indicates whether the certificate revocation list is checked during the
        /// certificate validation process.</summary>
        public bool CheckCertRevocationStatus => SslStream?.CheckCertRevocationStatus ?? false;
        /// <summary>Gets a Boolean value that indicates whether this SslStream uses data encryption.</summary>
        public bool IsEncrypted => SslStream?.IsEncrypted ?? false;
        /// <summary>Gets a Boolean value that indicates whether both server and client have been authenticated.
        /// </summary>
        public bool IsMutuallyAuthenticated => SslStream?.IsMutuallyAuthenticated ?? false;
        /// <summary>Gets a Boolean value that indicates whether the data sent using this stream is signed.</summary>
        public bool IsSigned => SslStream?.IsSigned ?? false;

        /// <summary>Gets the certificate used to authenticate the local endpoint or null if no certificate was
        /// supplied.</summary>
        public X509Certificate? LocalCertificate => SslStream?.LocalCertificate;

        /// <summary>The negotiated application protocol in TLS handshake.</summary>
        public SslApplicationProtocol? NegotiatedApplicationProtocol => SslStream?.NegotiatedApplicationProtocol;

        /// <summary>Gets the cipher suite which was negotiated for this connection.</summary>
        public TlsCipherSuite? NegotiatedCipherSuite => SslStream?.NegotiatedCipherSuite;
        /// <summary>Gets the certificate used to authenticate the remote endpoint or null if no certificate was
        /// supplied.</summary>
        public X509Certificate? RemoteCertificate => SslStream?.RemoteCertificate;

        /// <summary>Gets a value that indicates the security protocol used to authenticate this connection or
        /// null if the connection is not secure.</summary>
        public SslProtocols? SslProtocol => SslStream?.SslProtocol;

        private SslStream? SslStream => _transceiver.Underlying.SslStream;

        protected internal TcpConnection(
            IConnectionManager manager,
            Endpoint endpoint,
            MultiStreamTransceiverWithUnderlyingTransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter)
            : base(manager, endpoint, transceiver, connector, connectionId, adapter)
        {
        }
    }

    /// <summary>Represents a connection to a UDP-endpoint.</summary>
    public class UdpConnection : IPConnection
    {
        /// <summary>The multicast IP-endpoint for a multicast connection otherwise null.</summary>
        public System.Net.IPEndPoint? MulticastEndpoint => _udpTransceiver.MulticastAddress;

        private readonly UdpTransceiver _udpTransceiver;

        protected internal UdpConnection(
            IConnectionManager? manager,
            Endpoint endpoint,
            MultiStreamTransceiverWithUnderlyingTransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter)
            : base(manager, endpoint, transceiver, connector, connectionId, adapter) =>
            _udpTransceiver = (UdpTransceiver)_transceiver.Underlying;
    }

    /// <summary>Represents a connection to a WS-endpoint.</summary>
    public class WSConnection : TcpConnection
    {
        /// <summary>The HTTP headers in the WebSocket upgrade request.</summary>
        public IReadOnlyDictionary<string, string> Headers => _wsTransceiver.Headers;

        private readonly WSTransceiver _wsTransceiver;

        protected internal WSConnection(
            IConnectionManager manager,
            Endpoint endpoint,
            MultiStreamTransceiverWithUnderlyingTransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter)
            : base(manager, endpoint, transceiver, connector, connectionId, adapter) =>
            _wsTransceiver = (WSTransceiver)_transceiver.Underlying;
    }
}
