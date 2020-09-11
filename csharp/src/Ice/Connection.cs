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
        /// <summary>Wait for all pending invocations to complete before closing the connection.</summary>
        GracefullyWithWait
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
    public abstract class Connection : IRequestHandler
    {
        /// <summary>Gets or set the connection Acm (Active Connection Management) configuration.</summary>
        public Acm Acm
        {
            get
            {
                lock (_mutex)
                {
                    return _monitor.Acm;
                }
            }
            set
            {
                lock (_mutex)
                {
                    if (_state >= ConnectionState.Closing)
                    {
                        return;
                    }

                    if (_state == ConnectionState.Active)
                    {
                        _monitor.Remove(this);
                    }

                    _monitor = value == _manager.AcmMonitor.Acm ?
                        _manager.AcmMonitor : new ConnectionAcmMonitor(value, _communicator.Logger);

                    if (_state == ConnectionState.Active)
                    {
                        _monitor.Add(this);
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

        private ObjectAdapter? _adapter;
        private EventHandler? _closed;
        private Task? _closeTask;
        private readonly Communicator _communicator;
        private readonly IConnector? _connector;
        private Exception? _exception;
        private readonly IConnectionManager _manager;
        private IAcmMonitor _monitor;
        private readonly object _mutex = new object();
        private Task _acceptStreamTask = Task.CompletedTask;
        private ConnectionState _state; // The current state.

        /// <summary>Manually closes the connection using the specified closure mode.</summary>
        /// <param name="mode">Determines how the connection will be closed.</param>
        public void Close(ConnectionClose mode)
        {
            // TODO: We should consider removing this method and expose GracefulCloseAsync and CloseAsync
            // instead. This would remove the support for ConnectionClose.GracefullyWithWait. Is it
            // useful? Not waiting implies that the pending requests implies these requests will fail and
            // won't be retried. GracefulCloseAsync could wait for pending requests to complete?
            if (mode == ConnectionClose.Forcefully)
            {
                _ = AbortAsync(new ConnectionClosedLocallyException("connection closed forcefully"));
            }
            else if (mode == ConnectionClose.Gracefully)
            {
                _ = CloseAsync(new ConnectionClosedLocallyException("connection closed gracefully"));
            }
            else
            {
                Debug.Assert(mode == ConnectionClose.GracefullyWithWait);

                // Wait until all outstanding requests have been completed.
                lock (_mutex)
                {
                    // TODO
                    // while (_requests.Count > 0)
                    // {
                    //     System.Threading.Monitor.Wait(_mutex);
                    // }
                }

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
            lock (_mutex)
            {
                if (_exception != null)
                {
                    Debug.Assert(_state >= ConnectionState.Closing);
                    throw _exception;
                }
            }
        }

        /// <summary>Returns a description of the connection as human readable text, suitable for logging or error
        /// messages.</summary>
        /// <returns>The description of the connection as human readable text.</returns>
        public override string ToString() => Transceiver.ToString()!;

        internal Connection(
            IConnectionManager manager,
            Endpoint endpoint,
            MultiStreamTransceiver connection,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter)
        {
            _communicator = endpoint.Communicator;
            _manager = manager;
            _monitor = manager.AcmMonitor;
            Transceiver = connection;
            _connector = connector;
            ConnectionId = connectionId;
            Endpoint = endpoint;
            Endpoints = new List<Endpoint>() { endpoint };
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
                    if (_state == ConnectionState.Active)
                    {
                        SetState(ConnectionState.Closing, exception);
                        closeTask = PerformCloseAsync(exception);
                        Debug.Assert(_closeTask == null);
                        _closeTask = closeTask;
                    }
                    else
                    {
                        closeTask = _closeTask!;
                    }
                }

                await closeTask.ConfigureAwait(false);
            }
            catch
            {
            }

            async Task PerformCloseAsync(Exception exception)
            {
                CancellationTokenSource? source = null;
                TimeSpan timeout = _communicator.CloseTimeout;
                if (timeout > TimeSpan.Zero)
                {
                    source = new CancellationTokenSource(timeout);
                }

                try
                {
                    CancellationToken cancel = source?.Token ?? default;

                    // Gracefully close the connection.
                    await Transceiver.CloseAsync(exception, cancel);

                    // Wait the failure of the accept stream task which indicates that the peer closed the connection
                    // or for the connection closure to be complete once it reaches the close state.
                    while (true)
                    {
                        Task task;
                        lock (_mutex)
                        {
                            task = _state == ConnectionState.Closed ? _closeTask! : _acceptStreamTask;
                        }
                        await task.WaitAsync(cancel).ConfigureAwait(false);
                    }
                }
                catch (OperationCanceledException)
                {
                    await AbortAsync(new TimeoutException());
                }
                finally
                {
                    source?.Dispose();
                }
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

        async ValueTask<Stream> IRequestHandler.SendRequestAsync(
            OutgoingRequestFrame request,
            bool bidirectional,
            IInvocationObserver? observer,
            CancellationToken cancel)
        {
            if (request.Protocol != Endpoint.Protocol)
            {
                throw new ArgumentException(
                    $"the frame protocol `{request.Protocol}' doesn't match the connection protocol `{Endpoint.Protocol}'");
            }

            ValueTask writeTask;
            Stream stream;
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
                cancel.ThrowIfCancellationRequested();

                Debug.Assert(_state > ConnectionState.Validating);
                Debug.Assert(_state < ConnectionState.Closing);

                stream = Transceiver.CreateStream(bidirectional);

                stream.Observer = observer?.GetRemoteObserver(this, stream.Id, request.Size);

                // It's important to call SendAsync from the synchronization here to ensure the requests are queued
                // for sending in the same order as the stream IDs are allocated.
                writeTask = stream.SendRequestFrameAsync(request, !bidirectional, cancel);
            }
            await writeTask.ConfigureAwait(false);
            return stream;
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

                // Initialize the transport
                await Transceiver.InitializeAsync(cancel).ConfigureAwait(false);

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
                // Yield to ensure the code below is executed without the mutex locked. PerformCloseAsync, the code
                // below is not safe to call with this mutex locked.
                await Task.Yield();

                // Close the transport
                try
                {
                    await Transceiver.AbortAsync(exception);
                }
                catch (Exception ex)
                {
                    _communicator.Logger.Error($"unexpected connection exception:\n{ex}\n{Transceiver}");
                }

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
                _manager.Remove(this);
            }
        }

        private async ValueTask AcceptStreamAsync()
        {
            Stream? stream = null;
            try
            {
                // Accept a new stream
                stream = await Transceiver.AcceptStreamAsync(CancellationToken.None).ConfigureAwait(false);

                // Start a new accept stream
                // TODO: as long as the .NET thread pool schedules this, we'll continue to accept new streams. This
                // can be problematic if the OA task scheduler can't keep up with the dispatching of the incoming
                // requests.
                _acceptStreamTask = Task.Run(async () => await AcceptStreamAsync());

                // Receive the request from the stream
                (IncomingRequestFrame request, bool fin)
                    = await stream.ReceiveRequestFrameAsync(CancellationToken.None).ConfigureAwait(false);

                // Cancel the given cancellation token source if the stream is closed by the peer.
                stream.CancelIfClosedByPeer();

                ObjectAdapter? adapter = _adapter;
                if (adapter == null)
                {
                    var exception = new ObjectNotExistException(request.Identity, request.Facet, request.Operation);
                    var response = new OutgoingResponseFrame(request, exception);
                    await stream.SendResponseFrameAsync(response, true, stream.CancellationToken).ConfigureAwait(false);
                    return;
                }
                else
                {
                    var current = new Current(adapter, request, stream, this);

                    // Dispatch the request and get the response
                    OutgoingResponseFrame response;
                    if (adapter.TaskScheduler != null)
                    {
                        (response, fin) = await TaskRun(() => adapter.DispatchAsync(request, stream, current),
                                                        stream.CancellationToken,
                                                        adapter.TaskScheduler).ConfigureAwait(false);
                    }
                    else
                    {
                        (response, fin) = await adapter.DispatchAsync(request, stream, current).ConfigureAwait(false);
                    }

                    // Send the response over the stream
                    await stream.SendResponseFrameAsync(response, fin, stream.CancellationToken);

                    if (!fin)
                    {
                        // TODO: send streamable data.
                    }
                }
            }
            catch (ObjectDisposedException)
            {
                // Ignore, the connection is being closed.
            }
            catch (OperationCanceledException)
            {
                // Ignore, the peer closed the stream.
            }
            catch (StreamClosedByPeerException)
            {
                // Ignore, the peer closed the stream.
            }
            catch (Exception ex)
            {
                // Other exceptions are considered fatal, abort the connection
                await AbortAsync(ex);
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
                    _acceptStreamTask = Task.Run(async () => await AcceptStreamAsync());
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
                _monitor.Add(this);
            }
            else if (_state == ConnectionState.Active)
            {
                Debug.Assert(state > ConnectionState.Active);
                _monitor.Remove(this);
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
            IConnectionManager manager,
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
            IConnectionManager manager,
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
