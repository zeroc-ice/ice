// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The state of an Ice connection.</summary>
    public enum ConnectionState : byte
    {
        /// <summary>The connection is being initialized.</summary>
        Initializing = 0,
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
                    return _monitor?.Acm ?? Acm.Disabled;
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
            get => _adapter;
            set => _adapter = value;
        }

        /// <summary>Get the connection ID which was used to create the connection.</summary>
        /// <value>The connection ID used to create the connection.</value>
        public string ConnectionId { get; }

        /// <summary>Get the endpoint from which the connection was created.</summary>
        /// <value>The endpoint from which the connection was created.</value>
        public Endpoint Endpoint { get; }

        /// <summary><c>true</c> for incoming connections <c>false</c> otherwise.</summary>
        public bool IsIncoming => _connector == null;

        /// <summary>The protocol used by the connection.</summary>
        public Protocol Protocol => Endpoint.Protocol;

        // The connector from which the connection was created. This is used by the outgoing connection factory.
        internal IConnector Connector => _connector!;

        // The endpoints which are associated with this connection. This is populated by the outgoing connection
        // factory when an endpoint resolves to the same connector as this connection's connector. Two endpoints
        // can be different but resolve to the same connector (e.g.: endpoints with the IPs "::1", "0:0:0:0:0:0:0:1"
        // or "localhost" are different endpoints but they all end up resolving to the same connector and can use
        // the same connection).
        internal List<Endpoint> Endpoints { get; }

        private protected MultiStreamTransceiver Transceiver { get; }
        // The accept stream task is assigned each time a new accept stream async operation is started.
        private volatile Task _acceptStreamTask = Task.CompletedTask;
        private volatile ObjectAdapter? _adapter;
        // The control stream is assigned on the connection initialization and is immutable once the connection
        // reaches the Active state.
        private TransceiverStream? _controlStream;
        private EventHandler? _closed;
        // The close task is assigned when GoAwayAsync or AbortAsync are called, it's protected with _mutex.
        private Task? _closeTask;
        private readonly Communicator _communicator;
        private readonly IConnector? _connector;
        // The last incoming stream IDs are setup when the streams are aborted, it's protected with _mutex.
        private (long Bidirectional, long Unidirectional) _lastIncomingStreamIds;
        private readonly IConnectionManager? _manager;
        private IAcmMonitor? _monitor;
        // The mutex protects mutable non-volatile data members and ensures the logic for some operations is
        // performed atomically.
        private readonly object _mutex = new ();
        private volatile ConnectionState _state; // The current state.

        /// <summary>Aborts the connection.</summary>
        /// <param name="message">A description of the connection abortion reason.</param>
        public Task AbortAsync(string? message = null) =>
            AbortAsync(new ConnectionClosedException(message ?? "connection closed forcefully",
                                                     isClosedByPeer: false,
                                                     RetryPolicy.AfterDelay(TimeSpan.Zero),
                                                     Connector));

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

        /// <summary>Gracefully closes the connection by sending a GoAway frame to the peer.</summary>
        /// <param name="message">The message transmitted to the peer with the GoAway frame.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public Task GoAwayAsync(string? message = null, CancellationToken cancel = default) =>
            GoAwayAsync(new ConnectionClosedException(message ?? "connection closed gracefully",
                                                      isClosedByPeer: false,
                                                      RetryPolicy.AfterDelay(TimeSpan.Zero),
                                                      Connector),
                        cancel);

        /// <summary>This event is raised when the connection receives a ping frame. The connection object is
        /// passed as the event sender argument.</summary>
        public event EventHandler? PingReceived
        {
            add => Transceiver.Ping += value;
            remove => Transceiver.Ping -= value;
        }

        /// <summary>Returns <c>true</c> if the connection is active. Outgoing streams can be created and incoming
        /// streams accepted when the connection is active. The connection is no longer considered active as soon
        /// as <see cref="GoAwayAsync(string?, CancellationToken)"/> is called to initiate a graceful connection
        /// closure.</summary>
        /// <return><c>true</c> if the connection is active, <c>false</c> if it's closing or closed.</return>
        public bool IsActive => _state == ConnectionState.Active;

        /// <summary>Sends a ping frame.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public void Ping(CancellationToken cancel = default)
        {
            try
            {
                PingAsync(cancel: cancel).Wait(cancel);
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }
        }

        /// <summary>Sends an asynchronous ping frame.</summary>
        /// <param name="progress">Sent progress provider.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public async Task PingAsync(IProgress<bool>? progress = null, CancellationToken cancel = default)
        {
            await Transceiver.PingAsync(cancel).ConfigureAwait(false);
            progress?.Report(true);
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
            Endpoints = new List<Endpoint>() { endpoint };
            _adapter = adapter;
            _state = ConnectionState.Initializing;
        }

        internal void ClearAdapter(ObjectAdapter adapter) => Interlocked.CompareExchange(ref _adapter, null, adapter);

        internal TransceiverStream CreateStream(bool bidirectional)
        {
            // Ensure the stream is created in the active state only, no new streams should be created if the
            // connection is closing or closed.
            lock (_mutex)
            {
                if (_state != ConnectionState.Active)
                {
                    throw new ConnectionClosedException(isClosedByPeer: false,
                                                        RetryPolicy.AfterDelay(TimeSpan.Zero),
                                                        Connector);
                }
                return Transceiver.CreateStream(bidirectional);
            }
        }

        internal async Task GoAwayAsync(Exception exception, CancellationToken cancel = default)
        {
            try
            {
                Task goAwayTask;
                lock (_mutex)
                {
                    if (_state == ConnectionState.Active && !Endpoint.IsDatagram)
                    {
                        SetState(ConnectionState.Closing, exception);
                        _closeTask ??= PerformGoAwayAsync(exception);
                        Debug.Assert(_closeTask != null);
                    }
                    goAwayTask = _closeTask ?? AbortAsync(exception);
                }
                await goAwayTask.WaitAsync(cancel).ConfigureAwait(false);
            }
            catch
            {
                // Ignore
            }

            async Task PerformGoAwayAsync(Exception exception)
            {
                // Abort outgoing streams and get the largest incoming stream ID. With Ice2, we don't wait for
                // the incoming streams to complete before sending the GoAway frame but instead provide the ID
                // of the latest incoming stream ID to the peer. The peer will close the connection once it
                // received the response for this stream ID.
                _lastIncomingStreamIds = Transceiver.AbortStreams(exception, stream => !stream.IsIncoming);

                // Yield to ensure the code below is executed without the mutex locked.
                await Task.Yield();

                // With Ice1, we first wait for all incoming streams to complete before sending the GoAway frame.
                if (Endpoint.Protocol == Protocol.Ice1)
                {
                    await Transceiver.WaitForEmptyStreamsAsync().ConfigureAwait(false);
                }

                try
                {
                    Debug.Assert(_communicator.CloseTimeout != TimeSpan.Zero);
                    using var source = new CancellationTokenSource(_communicator.CloseTimeout);
                    CancellationToken cancel = source.Token;

                    // Write the close frame
                    await _controlStream!.SendGoAwayFrameAsync(_lastIncomingStreamIds,
                                                               exception.Message,
                                                               cancel).ConfigureAwait(false);

                    // Wait for the peer to close the connection.
                    while (true)
                    {
                        // We can't just wait for the accept stream task failure as the task can sometime succeeds
                        // depending on the thread scheduling. So we also check for the state to ensure the loop
                        // eventually terminates once the peer connection is closed.
                        if (_state == ConnectionState.Closed)
                        {
                            throw exception;
                        }
                        await _acceptStreamTask.WaitAsync(cancel).ConfigureAwait(false);
                    }
                }
                catch (OperationCanceledException)
                {
                    await AbortAsync(new TimeoutException()).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    await AbortAsync(ex).ConfigureAwait(false);
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
                if (_state == ConnectionState.Active &&
                    (acm.Heartbeat == AcmHeartbeat.Always ||
                    (acm.Heartbeat != AcmHeartbeat.Off && now >= (Transceiver.LastActivity + (acm.Timeout / 4)))))
                {
                    if (acm.Heartbeat != AcmHeartbeat.OnDispatch || Transceiver.StreamCount > 2)
                    {
                        Debug.Assert(_state == ConnectionState.Active);
                        if (!Endpoint.IsDatagram)
                        {
                            _ = Transceiver.PingAsync(default);
                        }
                    }
                }

                if (acm.Close != AcmClose.Off && now >= Transceiver.LastActivity + acm.Timeout)
                {
                    if (acm.Close == AcmClose.OnIdleForceful ||
                        (acm.Close != AcmClose.OnIdle && (Transceiver.StreamCount > 2)))
                    {
                        // Close the connection if we didn't receive a heartbeat or if read/write didn't update the
                        // ACM activity in the last period.
                        _ = AbortAsync(new ConnectionClosedException("connection timed out",
                                                                     isClosedByPeer: false,
                                                                     RetryPolicy.AfterDelay(TimeSpan.Zero),
                                                                     Connector));
                    }
                    else if (acm.Close != AcmClose.OnInvocation && Transceiver.StreamCount <= 2)
                    {
                        // The connection is idle, close it.
                        _ = GoAwayAsync(new ConnectionClosedException("connection idle",
                                                                      isClosedByPeer: false,
                                                                      RetryPolicy.AfterDelay(TimeSpan.Zero),
                                                                      Connector));
                    }
                }
            }
        }

        internal async Task InitializeAsync()
        {
            try
            {
                Debug.Assert(_communicator.ConnectTimeout > TimeSpan.Zero);
                using var source = new CancellationTokenSource(_communicator.ConnectTimeout);
                CancellationToken cancel = source.Token;

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
                    _ = Task.Run(async () => await WaitForGoAwayAsync(peerControlStream).ConfigureAwait(false));
                }

                Transceiver.Initialized();

                lock (_mutex)
                {
                    if (_state >= ConnectionState.Closed)
                    {
                        // This can occur if the communicator or object adapter is disposed while the connection
                        // initializes.
                        throw new ConnectionClosedException(isClosedByPeer: false,
                                                            RetryPolicy.AfterDelay(TimeSpan.Zero),
                                                            Connector);
                    }
                    SetState(ConnectionState.Active);

                    // Start the asynchronous AcceptStream operation from the thread pool to prevent eventually reading
                    // synchronously new frames from this thread.
                    _acceptStreamTask = Task.Run(async () => await AcceptStreamAsync().ConfigureAwait(false));
                }
            }
            catch (OperationCanceledException)
            {
                var ex = new ConnectTimeoutException(RetryPolicy.AfterDelay(TimeSpan.Zero), Connector);
                _ = AbortAsync(ex);
                throw ex;
            }
            catch (Exception ex)
            {
                _ = AbortAsync(ex);
                throw;
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

                // Yield to ensure the code below is executed without the mutex locked (PerformAbortAsync is called
                // with the mutex locked).
                await Task.Yield();

                // Dispose of the transceiver.
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
                while (true)
                {
                    // Accept a new stream.
                    stream = await Transceiver.AcceptStreamAsync(CancellationToken.None).ConfigureAwait(false);

                    // Ignore the stream if the connection is being closed and the stream ID superior to the last
                    // incoming stream ID to be processed. The loop will eventually terminate when the peer closes
                    // the connection.
                    if (_state != ConnectionState.Active)
                    {
                        lock (_mutex)
                        {
                            if (stream.Id > (stream.IsBidirectional ? _lastIncomingStreamIds.Bidirectional :
                                                                      _lastIncomingStreamIds.Unidirectional))
                            {
                                stream.Dispose();
                                continue;
                            }
                        }
                    }

                    // Start a new accept stream task otherwise to accept another stream.
                    _acceptStreamTask = Task.Run(() => AcceptStreamAsync().AsTask());
                    break;
                }

                using var cancelSource = new CancellationTokenSource();
                CancellationToken cancel = cancelSource.Token;
                if (stream.IsBidirectional)
                {
                    // Be notified if the peer resets the stream to cancel the dispatch.
                    // TODO: the error code is ignored here as we can't provide it to the CancelationTokenSource. We
                    // could consider setting the error code into Ice.Current to allow the user to figure out the
                    // reason of the stream reset.
                    stream.Reset += (long errorCode) => cancelSource.Cancel();
                }

                // Receives the request frame from the stream
                (IncomingRequestFrame request, bool fin)
                    = await stream.ReceiveRequestFrameAsync(cancel).ConfigureAwait(false);

                // If no adapter is configure to dispatch the request, return an ObjectNotExistException to the caller.
                OutgoingResponseFrame response;
                ObjectAdapter? adapter = _adapter;
                if (adapter == null)
                {
                    if (stream.IsBidirectional)
                    {
                        var exception = new ObjectNotExistException();
                        response = new OutgoingResponseFrame(request, exception);
                        await stream.SendResponseFrameAsync(response, true, cancel).ConfigureAwait(false);
                    }
                    return;
                }

                // Dispatch the request and get the response
                var current = new Current(adapter, request, stream, fin, this);
                if (adapter.TaskScheduler != null)
                {
                    (response, fin) = await TaskRun(() => adapter.DispatchAsync(request, current, cancel),
                                                    cancel,
                                                    adapter.TaskScheduler).ConfigureAwait(false);
                }
                else
                {
                    (response, fin) = await adapter.DispatchAsync(request, current, cancel).ConfigureAwait(false);
                }

                // No need to send the response if the dispatch is canceled.
                cancel.ThrowIfCancellationRequested();

                if (stream.IsBidirectional)
                {
                    // Send the response over the stream
                    await stream.SendResponseFrameAsync(response, fin, cancel).ConfigureAwait(false);
                }
            }
            catch (OperationCanceledException)
            {
                // Ignore, the dispatch got canceled
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

            Debug.Assert(_state < state); // Don't switch twice and only switch to a higher value state.

            // If the connection is active and the new state is Closed without first going into the Closing state
            // we print a warning, the connection was closed non-gracefully.
            if (_state == ConnectionState.Active &&
                state == ConnectionState.Closed &&
                !Endpoint.IsDatagram &&
                ((Transceiver as LegacyTransceiver)?.IsValidated ?? true) &&
                _communicator.WarnConnections)
            {
                Debug.Assert(exception != null);
                _communicator.Logger.Warning($"connection exception:\n{exception}\n{this}");
            }

            // We register with the connection monitor if our new state is Active. ACM monitors the connection
            // once it's initialized and validated and until it's closed. Timeouts for connection establishment and
            // validation are implemented with a timer instead and setup in the outgoing connection factory.
            if (state == ConnectionState.Active)
            {
                _monitor?.Add(this);
            }
            else if (_state == ConnectionState.Active)
            {
                _monitor?.Remove(this);
            }

            if (_communicator.Observer != null)
            {
                Transceiver.Observer = _communicator.Observer.GetConnectionObserver(this, state, Transceiver.Observer);

                if (Transceiver.Observer != null && state == ConnectionState.Closed)
                {
                    Debug.Assert(exception != null);
                    if (_state != ConnectionState.Closing)
                    {
                        Transceiver.Observer.Failed(exception.GetType().FullName!);
                    }
                }
            }
            _state = state;
        }

        private async Task WaitForGoAwayAsync(TransceiverStream peerControlStream)
        {
            try
            {
                // Wait to receive the close frame on the control stream.
                ((long Bidirectional, long Unidirectional) lastStreamIds, string message) =
                    await peerControlStream.ReceiveGoAwayFrameAsync().ConfigureAwait(false);

                Task goAwayTask;
                lock (_mutex)
                {
                    var exception = new ConnectionClosedException(message,
                                                                  isClosedByPeer: true,
                                                                  RetryPolicy.AfterDelay(TimeSpan.Zero),
                                                                  Connector);
                    if (_state == ConnectionState.Active)
                    {
                        SetState(ConnectionState.Closing, exception);
                        goAwayTask = PerformGoAwayAsync(lastStreamIds, exception);
                        if (_closeTask == null)
                        {
                            _closeTask = goAwayTask;
                        }
                    }
                    else if (_state == ConnectionState.Closing)
                    {
                        // We already initiated graceful connection closure. If the peer did as well, we can cancel
                        // incoming/outgoing streams.
                        goAwayTask = PerformGoAwayAsync(lastStreamIds, exception);
                    }
                    else
                    {
                        goAwayTask = _closeTask!;
                    }
                }

                await goAwayTask.ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                await AbortAsync(ex).ConfigureAwait(false);
            }

            async Task PerformGoAwayAsync((long Bidirectional, long Unidirectional) lastStreamIds, Exception exception)
            {
                // Abort non-processed outgoing streams and all incoming streams.
                Transceiver.AbortStreams(exception,
                                         stream => stream.IsIncoming ||
                                                   stream.IsBidirectional ?
                                                       stream.Id > lastStreamIds.Bidirectional :
                                                       stream.Id > lastStreamIds.Unidirectional);

                // Yield to ensure the code below is executed without the mutex locked (PerformGoAwayAsync is called
                // with the mutex locked).
                await Task.Yield();

                // Wait for all the streams to complete.
                await Transceiver.WaitForEmptyStreamsAsync().ConfigureAwait(false);

                try
                {
                    // Close the transport
                    await Transceiver.CloseAsync(exception, CancellationToken.None).ConfigureAwait(false);
                }
                finally
                {
                    // Abort the connection once all the streams have completed.
                    await AbortAsync(exception).ConfigureAwait(false);
                }
            }
        }
    }

    /// <summary>Represents a connection to a colocated object adapter.</summary>
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

        internal IPConnection(
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

        internal TcpConnection(
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

        internal UdpConnection(
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

        internal WSConnection(
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
