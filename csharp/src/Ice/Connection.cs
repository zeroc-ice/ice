//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using IceInternal;
using Ice.Instrumentation;

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

        public override int GetHashCode()
        {
            int h_ = 5381;
            HashUtil.hashAdd(ref h_, "::Ice::ACM");
            HashUtil.hashAdd(ref h_, Timeout);
            HashUtil.hashAdd(ref h_, Close);
            HashUtil.hashAdd(ref h_, Heartbeat);
            return h_;
        }

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

    public sealed class Connection : IceInternal.EventHandler, IResponseHandler, ICancellationHandler
    {
        public interface StartCallback
        {
            void connectionStartCompleted(Connection connection);
            void connectionStartFailed(Connection connection, LocalException ex);
        }

        private class TimeoutCallback : ITimerTask
        {
            public TimeoutCallback(Connection connection) => _connection = connection;

            public void RunTimerTask() => _connection.TimedOut();

            private readonly Connection _connection;
        }

        public void Start(StartCallback callback)
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
                callback.connectionStartFailed(this, _exception);
                return;
            }

            callback.connectionStartCompleted(this);
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
                    _acmLastActivity = Time.currentMonotonicTimeMillis();
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
                _observer = _communicatorObserver.getConnectionObserver(InitConnectionInfo(), _endpoint,
                    ToConnectionState(_state), _observer);
                if (_observer != null)
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
                if (acm.heartbeat == ACMHeartbeat.HeartbeatAlways ||
                   (acm.heartbeat != ACMHeartbeat.HeartbeatOff && _writeStream.isEmpty() &&
                    now >= (_acmLastActivity + acm.timeout / 4)))
                {
                    if (acm.heartbeat != ACMHeartbeat.HeartbeatOnDispatch || _dispatchCount > 0)
                    {
                        SendHeartbeatNow();
                    }
                }

                if (_readStream.Size > Protocol.headerSize || !_writeStream.isEmpty())
                {
                    //
                    // If writing or reading, nothing to do, the connection
                    // timeout will kick-in if writes or reads don't progress.
                    // This check is necessary because the actitivy timer is
                    // only set when a message is fully read/written.
                    //
                    return;
                }

                if (acm.close != ACMClose.CloseOff && now >= (_acmLastActivity + acm.timeout))
                {
                    if (acm.close == ACMClose.CloseOnIdleForceful ||
                       (acm.close != ACMClose.CloseOnIdle && (_asyncRequests.Count > 0)))
                    {
                        //
                        // Close the connection if we didn't receive a heartbeat in
                        // the last period.
                        //
                        SetState(StateClosed, new ConnectionTimeoutException());
                    }
                    else if (acm.close != ACMClose.CloseOnInvocation &&
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
            OutputStream os = og.getOs();

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
                _transceiver.checkSendSize(os.GetBuffer());

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
                    os.WriteInt(requestId);
                }

                og.attachRemoteObserver(InitConnectionInfo(), _endpoint, requestId);

                int status = OutgoingAsyncBase.AsyncStatusQueued;
                try
                {
                    OutgoingMessage message = new OutgoingMessage(og, os, compress, requestId);
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
                        _threadPool.dispatch(() =>
                        {
                            try
                            {
                                callback(this);
                            }
                            catch (System.Exception ex)
                            {
                                _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                            }
                        }, this);
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
        public void Heartbeat() => heartbeatAsync().Wait();

        private class HeartbeatTaskCompletionCallback : TaskCompletionCallback<object>
        {
            public HeartbeatTaskCompletionCallback(IProgress<bool>? progress,
                                                   CancellationToken cancellationToken) :
                base(progress, cancellationToken)
            {
            }

            public override void handleInvokeResponse(bool ok, OutgoingAsyncBase og)
            {
                SetResult(null!);
            }
        }

        private class HeartbeatAsync : OutgoingAsyncBase
        {
            public HeartbeatAsync(Connection connection,
                                  Communicator communicator,
                                  IOutgoingAsyncCompletionCallback completionCallback) :
                base(communicator, completionCallback)
            {
                _connection = connection;
            }

            public void invoke()
            {
                try
                {
                    Debug.Assert(os_ != null);
                    os_.WriteBlob(Protocol.magic);
                    os_.WriteByte(Util.currentProtocol.major);
                    os_.WriteByte(Util.currentProtocol.minor);
                    os_.WriteByte(Util.currentProtocolEncoding.major);
                    os_.WriteByte(Util.currentProtocolEncoding.minor);
                    os_.WriteByte(Protocol.validateConnectionMsg);
                    os_.WriteByte(0);
                    os_.WriteInt(Protocol.headerSize); // Message size.

                    int status = _connection.SendAsyncRequest(this, false, false);

                    if ((status & AsyncStatusSent) != 0)
                    {
                        sentSynchronously_ = true;
                        if ((status & AsyncStatusInvokeSentCallback) != 0)
                        {
                            invokeSent();
                        }
                    }
                }
                catch (RetryException ex)
                {
                    try
                    {
                        throw ex.get();
                    }
                    catch (LocalException ee)
                    {
                        if (exception(ee))
                        {
                            invokeExceptionAsync();
                        }
                    }
                }
                catch (Exception ex)
                {
                    if (exception(ex))
                    {
                        invokeExceptionAsync();
                    }
                }
            }

            private readonly Connection _connection;
        }

        public Task heartbeatAsync(IProgress<bool>? progress = null, CancellationToken cancel = new CancellationToken())
        {
            var completed = new HeartbeatTaskCompletionCallback(progress, cancel);
            var outgoing = new HeartbeatAsync(this, _communicator, completed);
            outgoing.invoke();
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
        /// <param name="heartbeat">The hertbeat condition</param>
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
                    _monitor.remove(this);
                }
                _monitor = _monitor.acm(timeout, close, heartbeat);

                if (_monitor.getACM().Timeout <= 0)
                {
                    _acmLastActivity = -1; // Disable the recording of last activity.
                }
                else if (_state == StateActive && _acmLastActivity == -1)
                {
                    _acmLastActivity = Time.currentMonotonicTimeMillis();
                }

                if (_state == StateActive)
                {
                    _monitor.add(this);
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
                return _monitor != null ? _monitor.getACM() : new ACM(0, ACMClose.CloseOff, ACMHeartbeat.HeartbeatOff);
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

                OutgoingMessage o = _sendStreams.FirstOrDefault(m => m.outAsync == outAsync);
                if (o != null)
                {
                    if (o.requestId > 0)
                    {
                        _asyncRequests.Remove(o.requestId);
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
                        if (outAsync.exception(ex))
                        {
                            outAsync.invokeExceptionAsync();
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
                                if (outAsync.exception(ex))
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

        public void SendResponse(int requestId, OutputStream os, byte compressFlag, bool amd)
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

                    SendMessage(new OutgoingMessage(os, compressFlag > 0, true));

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

        public void SendNoResponse()
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

        public bool SystemException(int requestId, SystemException ex, bool amd) => false; // System exceptions aren't marshalled.

        public void InvokeException(int requestId, LocalException ex, int invokeNum, bool amd)
        {
            //
            // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
            // called in case of a fatal exception we decrement _dispatchCount here.
            //

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

        internal IConnector Connector
        {
            get
            {
                Debug.Assert(_connector != null);
                return _connector; // No mutex protection necessary, _connector is immutable.
            }
        }

        /// <summary>
        /// Explicitly set an object adapter that dispatches requests that
        /// are received over this connection.
        /// A client can invoke an
        /// operation on a server using a proxy, and then set an object
        /// adapter for the outgoing connection that is used by the proxy
        /// in order to receive callbacks. This is useful if the server
        /// cannot establish a connection back to the client, for example
        /// because of firewalls.
        ///
        /// </summary>
        /// <param name="adapter">The object adapter that should be used by this
        /// connection to dispatch requests. The object adapter must be
        /// activated. When the object adapter is deactivated, it is
        /// automatically removed from the connection. Attempts to use a
        /// deactivated object adapter raise ObjectAdapterDeactivatedException
        ///
        /// </param>
        public void SetAdapter(ObjectAdapter? adapter)
        {
            if (adapter != null)
            {
                // Go through the adapter to set the adapter and servant manager on this connection
                // to ensure the object adapter is still active.
                adapter.setAdapterOnConnection(this);
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
                    _servantManager = null;
                }
            }

            //
            // We never change the thread pool with which we were initially
            // registered, even if we add or remove an object adapter.
            //
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
        public IEndpoint Endpoint
        {
            get
            {
                return _endpoint; // No mutex protection necessary, _endpoint is immutable.
            }
        }

        /// <summary>
        /// Create a special proxy that always uses this connection.
        /// This
        /// can be used for callbacks from a server to a client if the
        /// server cannot directly establish a connection to the client,
        /// for example because of firewalls. In this case, the server
        /// would create a proxy using an already established connection
        /// from the client.
        ///
        /// </summary>
        /// <param name="id">The identity for which a proxy is to be created.
        ///
        /// </param>
        /// <returns>A proxy that matches the given identity and uses this
        /// connection.
        ///
        /// </returns>
        public IObjectPrx CreateProxy(Identity id) => new ObjectPrx(_communicator.CreateReference(id, this));

        public void SetAdapterAndServantManager(ObjectAdapter adapter, ServantManager servantManager)
        {
            lock (this)
            {
                if (_state <= StateNotValidated || _state >= StateClosing)
                {
                    return;
                }
                Debug.Assert(adapter != null); // Called by ObjectAdapterI::setAdapterOnConnection
                _adapter = adapter;
                _servantManager = servantManager;
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
                    if (_observer != null)
                    {
                        ObserverStartWrite(_writeStream.GetBuffer());
                    }

                    completedSynchronously = _transceiver.startWrite(_writeStream.GetBuffer(), cb, this, out bool completed);
                    if (completed && _sendStreams.Count > 0)
                    {
                        // The whole message is written, assume it's sent now for at-most-once semantics.
                        _sendStreams.First.Value.isSent = true;
                    }
                }
                else if ((operation & SocketOperation.Read) != 0)
                {
                    if (_observer != null && !_readHeader)
                    {
                        ObserverStartRead(_readStream.GetBuffer());
                    }

                    completedSynchronously = _transceiver.startRead(_readStream.GetBuffer(), cb, this);
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
                    IceInternal.Buffer buf = _writeStream.GetBuffer();
                    int start = buf.b.position();
                    _transceiver.finishWrite(buf);
                    if (_communicator.traceLevels().network >= 3 && buf.b.position() != start)
                    {
                        var s = new StringBuilder("sent ");
                        s.Append(buf.b.position() - start);
                        if (!_endpoint.datagram())
                        {
                            s.Append(" of ");
                            s.Append(buf.b.limit() - start);
                        }
                        s.Append(" bytes via ");
                        s.Append(_endpoint.protocol());
                        s.Append("\n");
                        s.Append(ToString());
                        _logger.trace(_communicator.traceLevels().networkCat, s.ToString());
                    }

                    if (_observer != null)
                    {
                        ObserverFinishWrite(_writeStream.GetBuffer());
                    }
                }
                else if ((operation & SocketOperation.Read) != 0)
                {
                    IceInternal.Buffer buf = _readStream.GetBuffer();
                    int start = buf.b.position();
                    _transceiver.finishRead(buf);
                    if (_communicator.traceLevels().network >= 3 && buf.b.position() != start)
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
                        s.Append("\n");
                        s.Append(ToString());
                        _logger.trace(_communicator.traceLevels().networkCat, s.ToString());
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
            StartCallback? startCB = null;
            Queue<OutgoingMessage>? sentCBs = null;
            MessageInfo info = new MessageInfo();
            int dispatchCount = 0;

            ThreadPoolMessage msg = new ThreadPoolMessage(this);
            try
            {
                lock (this)
                {
                    if (!msg.startIOScope(ref current))
                    {
                        return;
                    }

                    if (_state >= StateClosed)
                    {
                        return;
                    }

                    int readyOp = current.operation;
                    try
                    {
                        UnscheduleTimeout(current.operation);

                        int writeOp = SocketOperation.None;
                        int readOp = SocketOperation.None;
                        if ((readyOp & SocketOperation.Write) != 0)
                        {
                            if (_observer != null)
                            {
                                ObserverStartWrite(_writeStream.GetBuffer());
                            }
                            writeOp = Write(_writeStream.GetBuffer());
                            if (_observer != null && (writeOp & SocketOperation.Write) == 0)
                            {
                                ObserverFinishWrite(_writeStream.GetBuffer());
                            }
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
                                Debug.Assert(!buf.b.hasRemaining());
                                ObserverFinishRead(buf);
                            }

                            if (_readHeader) // Read header if necessary.
                            {
                                _readHeader = false;

                                if (_observer != null)
                                {
                                    _observer.receivedBytes(Protocol.headerSize);
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
                                        badMagic = m
                                    };
                                }

                                byte major = _readStream.ReadByte();
                                byte minor = _readStream.ReadByte();
                                ProtocolVersion pv = new ProtocolVersion(major, minor);
                                Protocol.checkSupportedProtocol(pv);
                                major = _readStream.ReadByte();
                                minor = _readStream.ReadByte();
                                EncodingVersion ev = new EncodingVersion(major, minor);
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
                                    Ex.throwMemoryLimitException(size, _messageSizeMax);
                                }
                                if (size > _readStream.Size)
                                {
                                    _readStream.Resize(size);
                                }
                                _readStream.Pos = pos;
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
                                _threadPool.update(this, current.operation, newOp);
                                return;
                            }

                            if (_state == StateNotInitialized && !Initialize(current.operation))
                            {
                                return;
                            }

                            if (_state <= StateNotValidated && !Validate(current.operation))
                            {
                                return;
                            }

                            _threadPool.unregister(this, current.operation);

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
                                dispatchCount += info.messageDispatchCount;
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
                                _threadPool.update(this, current.operation, newOp);
                            }
                        }

                        if (_acmLastActivity > -1)
                        {
                            _acmLastActivity = Time.currentMonotonicTimeMillis();
                        }

                        if (dispatchCount == 0)
                        {
                            return; // Nothing to dispatch we're done!
                        }

                        _dispatchCount += dispatchCount;

                        msg.completed(ref current);
                    }
                    catch (DatagramLimitException) // Expected.
                    {
                        if (_warnUdp)
                        {
                            _logger.warning(string.Format("maximum datagram size of {0} exceeded", _readStream.Pos));
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
                        if (_endpoint.datagram())
                        {
                            if (_warn)
                            {
                                _logger.warning(string.Format("datagram connection exception:\n{0}\n{1}", ex, _desc));
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
                    _threadPool.dispatch(() =>
                    {
                        Dispatch(startCB, sentCBs, info);
                        msg.destroy(ref c);
                    }, this);
                }
            }
            finally
            {
                msg.finishIOScope(ref current);
            }

        }

        private void Dispatch(StartCallback? startCB, Queue<OutgoingMessage>? sentCBs, MessageInfo info)
        {
            int dispatchedCount = 0;

            //
            // Notify the factory that the connection establishment and
            // validation has completed.
            //
            if (startCB != null)
            {
                startCB.connectionStartCompleted(this);
                ++dispatchedCount;
            }

            //
            // Notify AMI calls that the message was sent.
            //
            if (sentCBs != null)
            {
                foreach (OutgoingMessage m in sentCBs)
                {
                    if (m.invokeSent)
                    {
                        Debug.Assert(m.outAsync != null);
                        m.outAsync.invokeSent();
                    }
                    if (m.receivedReply)
                    {
                        Debug.Assert(m.outAsync != null);
                        OutgoingAsync outAsync = (OutgoingAsync)m.outAsync;
                        if (outAsync.response())
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
            if (info.outAsync != null)
            {
                info.outAsync.invokeResponse();
                ++dispatchedCount;
            }

            if (info.heartbeatCallback != null)
            {
                try
                {
                    info.heartbeatCallback(this);
                }
                catch (System.Exception ex)
                {
                    _logger.error("connection callback exception:\n" + ex + '\n' + _desc);
                }
                ++dispatchedCount;
            }

            //
            // Method invocation must be done outside the thread synchronization, so that nested
            // calls are possible.
            //
            if (info.invokeNum > 0)
            {
                InvokeAll(info.stream, info.invokeNum, info.requestId, info.compress, info.servantManager,
                          info.adapter);

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
            _threadPool.dispatch(Finish, this);
        }

        private void Finish()
        {
            if (!_initialized)
            {
                if (_communicator.traceLevels().network >= 2)
                {
                    var s = new StringBuilder("failed to ");
                    s.Append(_connector != null ? "establish" : "accept");
                    s.Append(" ");
                    s.Append(_endpoint.protocol());
                    s.Append(" connection\n");
                    s.Append(ToString());
                    s.Append("\n");
                    s.Append(_exception);
                    _logger.trace(_communicator.traceLevels().networkCat, s.ToString());
                }
            }
            else if (_communicator.traceLevels().network >= 1)
            {
                var s = new StringBuilder("closed ");
                s.Append(_endpoint.protocol());
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

                _logger.trace(_communicator.traceLevels().networkCat, s.ToString());
            }

            if (_startCallback != null)
            {
                Debug.Assert(_exception != null);
                _startCallback.connectionStartFailed(this, _exception);
                _startCallback = null;
            }

            if (_sendStreams.Count > 0)
            {
                if (!_writeStream.isEmpty())
                {
                    //
                    // Return the stream to the outgoing call. This is important for
                    // retriable AMI calls which are not marshalled again.
                    //
                    OutgoingMessage message = _sendStreams.First.Value;
                    Debug.Assert(message.stream != null);
                    _writeStream.Swap(message.stream);

                    //
                    // The current message might be sent but not yet removed from _sendStreams. If
                    // the response has been received in the meantime, we remove the message from
                    // _sendStreams to not call finished on a message which is already done.
                    //
                    if (message.isSent || message.receivedReply)
                    {
                        if (message.Sent() && message.invokeSent)
                        {
                            Debug.Assert(message.outAsync != null);
                            message.outAsync.invokeSent();
                        }
                        if (message.receivedReply)
                        {
                            Debug.Assert(message.outAsync != null);
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
                    o.Completed(_exception!);
                    if (o.requestId > 0) // Make sure finished isn't called twice.
                    {
                        _asyncRequests.Remove(o.requestId);
                    }
                }
                _sendStreams.Clear(); // Must be cleared before _requests because of Outgoing* references in OutgoingMessage
            }

            foreach (OutgoingAsyncBase o in _asyncRequests.Values)
            {
                if (o.exception(_exception!))
                {
                    o.invokeException();
                }
            }
            _asyncRequests.Clear();

            //
            // Don't wait to be reaped to reclaim memory allocated by read/write streams.
            //
            _writeStream.Clear();
            _writeStream.GetBuffer().clear();
            _readStream.Clear();
            _readStream.GetBuffer().clear();
            _incomingCache = null;

            if (_closeCallback != null)
            {
                try
                {
                    _closeCallback(this);
                }
                catch (System.Exception ex)
                {
                    _logger.error($"connection callback exception:\n{ex}\n{_desc}");
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

        /// <summary>
        /// Return a description of the connection as human readable text,
        /// suitable for logging or error messages.
        /// </summary>
        /// <returns>The description of the connection as human readable
        /// text.</returns>
        public override string ToString()
        {
            return _desc; // No mutex lock, _desc is immutable.
        }

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
        public string type()
        {
            return _type; // No mutex lock, _type is immutable.
        }

        /// <summary>
        /// Get the timeout for the connection.
        /// </summary>
        /// <returns>The connection's timeout.</returns>
        public int Timeout
        {
            get
            {
                return _endpoint.timeout(); // No mutex protection necessary, _endpoint is immutable.
            }
        }

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
                _transceiver.setBufferSize(rcvSize, sndSize);
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

        public IceInternal.ThreadPool ThreadPool
        {
            get
            {
                return _threadPool;
            }
        }

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
            _type = transceiver.protocol();
            _connector = connector;
            _endpoint = endpoint;
            _adapter = adapter;
            _communicatorObserver = communicator.Observer;
            _logger = communicator.Logger; // Cached for better performance.
            _traceLevels = communicator.traceLevels(); // Cached for better performance.
            _timer = communicator.timer();
            _writeTimeout = new TimeoutCallback(this);
            _writeTimeoutScheduled = false;
            _readTimeout = new TimeoutCallback(this);
            _readTimeoutScheduled = false;
            _warn = communicator.GetPropertyAsInt("Ice.Warn.Connections") > 0;
            _warnUdp = communicator.GetPropertyAsInt("Ice.Warn.Datagrams") > 0;
            _cacheBuffers = communicator.CacheMessageBuffers > 0;
            if (_monitor != null && _monitor.getACM().Timeout > 0)
            {
                _acmLastActivity = Time.currentMonotonicTimeMillis();
            }
            else
            {
                _acmLastActivity = -1;
            }
            _nextRequestId = 1;
            _messageSizeMax = adapter != null ? adapter.messageSizeMax() : communicator.messageSizeMax();
            _readStream = new InputStream(communicator, Util.currentProtocolEncoding);
            _readHeader = false;
            _readStreamPos = -1;
            _writeStream = new OutputStream(communicator, Util.currentProtocolEncoding);
            _writeStreamPos = -1;
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
                _servantManager = adapter.getServantManager();
            }

            try
            {
                if (adapter != null)
                {
                    _threadPool = adapter.getThreadPool();
                }
                else
                {
                    _threadPool = communicator.clientThreadPool();
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
                            _threadPool.register(this, SocketOperation.Read);
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
                                _threadPool.unregister(this, SocketOperation.Read);
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
                        _acmLastActivity = Time.currentMonotonicTimeMillis();
                    }
                    _monitor.add(this);
                }
                else if (_state == StateActive)
                {
                    _monitor.remove(this);
                }
            }

            if (_communicatorObserver != null)
            {
                ConnectionState oldState = ToConnectionState(_state);
                ConnectionState newState = ToConnectionState(state);
                if (oldState != newState)
                {
                    _observer = _communicatorObserver.getConnectionObserver(InitConnectionInfo(), _endpoint,
                        newState, _observer);
                    if (_observer != null)
                    {
                        _observer.attach();
                    }
                    else
                    {
                        _writeStreamPos = -1;
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
                        _observer.failed(_exception.ice_id());
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

            if (!_endpoint.datagram())
            {
                //
                // Before we shut down, we send a close connection message.
                //
                OutputStream os = new OutputStream(_communicator, Util.currentProtocolEncoding);
                os.WriteBlob(Protocol.magic);
                os.WriteByte(Util.currentProtocol.major);
                os.WriteByte(Util.currentProtocol.minor);
                os.WriteByte(Util.currentProtocolEncoding.major);
                os.WriteByte(Util.currentProtocolEncoding.minor);
                os.WriteByte(Protocol.closeConnectionMsg);
                os.WriteByte(_compressionSupported ? (byte)1 : (byte)0);
                os.WriteInt(Protocol.headerSize); // Message size.

                if ((SendMessage(new OutgoingMessage(os, false, false)) & OutgoingAsyncBase.AsyncStatusSent) != 0)
                {
                    SetState(StateClosingPending);

                    //
                    // Notify the transceiver of the graceful connection closure.
                    //
                    int op = _transceiver.closing(true, _exception);
                    if (op != 0)
                    {
                        ScheduleTimeout(op);
                        _threadPool.register(this, op);
                    }
                }
            }
        }

        private void SendHeartbeatNow()
        {
            Debug.Assert(_state == StateActive);

            if (!_endpoint.datagram())
            {
                OutputStream os = new OutputStream(_communicator, Util.currentProtocolEncoding);
                os.WriteBlob(Protocol.magic);
                os.WriteByte(Util.currentProtocol.major);
                os.WriteByte(Util.currentProtocol.minor);
                os.WriteByte(Util.currentProtocolEncoding.major);
                os.WriteByte(Util.currentProtocolEncoding.minor);
                os.WriteByte(Protocol.validateConnectionMsg);
                os.WriteByte(0);
                os.WriteInt(Protocol.headerSize); // Message size.
                try
                {
                    SendMessage(new OutgoingMessage(os, false, false));
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
            int s = _transceiver.initialize(_readStream.GetBuffer(), _writeStream.GetBuffer(), ref _hasMoreData);
            if (s != SocketOperation.None)
            {
                ScheduleTimeout(s);
                _threadPool.update(this, operation, s);
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
            if (!_endpoint.datagram()) // Datagram connections are always implicitly validated.
            {
                if (_adapter != null) // The server side has the active role for connection validation.
                {
                    if (_writeStream.size() == 0)
                    {
                        _writeStream.WriteBlob(Protocol.magic);
                        _writeStream.WriteByte(Util.currentProtocol.major);
                        _writeStream.WriteByte(Util.currentProtocol.minor);
                        _writeStream.WriteByte(Util.currentProtocolEncoding.major);
                        _writeStream.WriteByte(Util.currentProtocolEncoding.minor);
                        _writeStream.WriteByte(Protocol.validateConnectionMsg);
                        _writeStream.WriteByte(0); // Compression status (always zero for validate connection).
                        _writeStream.WriteInt(Protocol.headerSize); // Message size.
                        TraceUtil.traceSend(_writeStream, _logger, _traceLevels);
                        _writeStream.PrepareWrite();
                    }

                    if (_observer != null)
                    {
                        ObserverStartWrite(_writeStream.GetBuffer());
                    }

                    if (_writeStream.pos() != _writeStream.size())
                    {
                        int op = Write(_writeStream.GetBuffer());
                        if (op != 0)
                        {
                            ScheduleTimeout(op);
                            _threadPool.update(this, operation, op);
                            return false;
                        }
                    }

                    if (_observer != null)
                    {
                        ObserverFinishWrite(_writeStream.GetBuffer());
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
                            _threadPool.update(this, operation, op);
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
                            badMagic = m
                        };
                    }

                    byte major = _readStream.ReadByte();
                    byte minor = _readStream.ReadByte();
                    ProtocolVersion pv = new ProtocolVersion(major, minor);
                    Protocol.checkSupportedProtocol(pv);
                    major = _readStream.ReadByte();
                    minor = _readStream.ReadByte();
                    EncodingVersion ev = new EncodingVersion(major, minor);
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
                    TraceUtil.traceRecv(_readStream, _logger, _traceLevels);
                }
            }

            _writeStream.Resize(0);
            _writeStream.pos(0);

            _readStream.Resize(Protocol.headerSize);
            _readStream.Pos = 0;
            _readHeader = true;

            if (_communicator.traceLevels().network >= 1)
            {
                StringBuilder s = new StringBuilder();
                if (_endpoint.datagram())
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
                _logger.trace(_communicator.traceLevels().networkCat, s.ToString());
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
            else if (_state == StateClosingPending && _writeStream.pos() == 0)
            {
                // Message wasn't sent, empty the _writeStream, we're not going to send more data.
                OutgoingMessage message = _sendStreams.First.Value;
                _writeStream.Swap(message.stream);
                return SocketOperation.None;
            }

            Debug.Assert(!_writeStream.isEmpty() && _writeStream.pos() == _writeStream.size());
            try
            {
                while (true)
                {
                    //
                    // Notify the message that it was sent.
                    //
                    OutgoingMessage message = _sendStreams.First.Value;
                    _writeStream.Swap(message.stream);
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
                    Debug.Assert(!message.prepared);
                    Debug.Assert(message.stream != null);
                    OutputStream stream = message.stream;

                    message.stream = DoCompress(message.stream, message.compress);
                    message.stream.PrepareWrite();
                    message.prepared = true;

                    TraceUtil.traceSend(stream, _logger, _traceLevels);
                    _writeStream.Swap(message.stream);

                    //
                    // Send the message.
                    //
                    if (_observer != null)
                    {
                        ObserverStartWrite(_writeStream.GetBuffer());
                    }
                    if (_writeStream.pos() != _writeStream.size())
                    {
                        int op = Write(_writeStream.GetBuffer());
                        if (op != 0)
                        {
                            return op;
                        }
                    }
                    if (_observer != null)
                    {
                        ObserverFinishWrite(_writeStream.GetBuffer());
                    }
                }

                //
                // If all the messages were sent and we are in the closing state, we schedule
                // the close timeout to wait for the peer to close the connection.
                //
                if (_state == StateClosing && _shutdownInitiated)
                {
                    SetState(StateClosingPending);
                    int op = _transceiver.closing(true, _exception);
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
                message.Adopt();
                _sendStreams.AddLast(message);
                return OutgoingAsyncBase.AsyncStatusQueued;
            }

            //
            // Attempt to send the message without blocking. If the send blocks, we use
            // asynchronous I/O or we request the caller to call finishSendMessage() outside
            // the synchronization.
            //

            Debug.Assert(!message.prepared);
            Debug.Assert(message.stream != null);
            OutputStream stream = message.stream;

            message.stream = DoCompress(stream, message.compress);
            message.stream.PrepareWrite();
            message.prepared = true;

            TraceUtil.traceSend(stream, _logger, _traceLevels);

            //
            // Send the message without blocking.
            //
            if (_observer != null)
            {
                ObserverStartWrite(message.stream.GetBuffer());
            }
            int op = Write(message.stream.GetBuffer());
            if (op == 0)
            {
                if (_observer != null)
                {
                    ObserverFinishWrite(message.stream.GetBuffer());
                }

                int status = OutgoingAsyncBase.AsyncStatusSent;
                if (message.Sent())
                {
                    status |= OutgoingAsyncBase.AsyncStatusInvokeSentCallback;
                }

                if (_acmLastActivity > -1)
                {
                    _acmLastActivity = Time.currentMonotonicTimeMillis();
                }
                return status;
            }

            message.Adopt();

            _writeStream.Swap(message.stream);
            _sendStreams.AddLast(message);
            ScheduleTimeout(op);
            _threadPool.register(this, op);
            return OutgoingAsyncBase.AsyncStatusQueued;
        }

        private OutputStream DoCompress(OutputStream uncompressed, bool compress)
        {
            if (_compressionSupported)
            {
                if (compress && uncompressed.size() >= 100)
                {
                    //
                    // Do compression.
                    //
                    IceInternal.Buffer? cbuf = BZip2.compress(uncompressed.GetBuffer(), Protocol.headerSize,
                                                              _compressionLevel);
                    if (cbuf != null)
                    {
                        OutputStream cstream =
                            new OutputStream(uncompressed.communicator(), uncompressed.GetEncoding(), cbuf, true);

                        //
                        // Set compression status.
                        //
                        cstream.pos(9);
                        cstream.WriteByte(2);

                        //
                        // Write the size of the compressed stream into the header.
                        //
                        cstream.pos(10);
                        cstream.WriteInt(cstream.size());

                        //
                        // Write the compression status and size of the compressed stream into the header of the
                        // uncompressed stream -- we need this to trace requests correctly.
                        //
                        uncompressed.pos(9);
                        uncompressed.WriteByte(2);
                        uncompressed.WriteInt(cstream.size());

                        return cstream;
                    }
                }
            }

            uncompressed.pos(9);
            uncompressed.WriteByte((byte)((_compressionSupported && compress) ? 1 : 0));

            //
            // Not compressed, fill in the message size.
            //
            uncompressed.pos(10);
            uncompressed.WriteInt(uncompressed.size());

            return uncompressed;
        }

        private struct MessageInfo
        {
            public InputStream stream;
            public int invokeNum;
            public int requestId;
            public byte compress;
            public ServantManager? servantManager;
            public ObjectAdapter? adapter;
            public OutgoingAsyncBase? outAsync;
            public HeartbeatCallback heartbeatCallback;
            public int messageDispatchCount;
        }

        private int ParseMessage(ref MessageInfo info)
        {
            Debug.Assert(_state > StateNotValidated && _state < StateClosed);

            info.stream = new InputStream(_communicator, Util.currentProtocolEncoding);
            _readStream.Swap(info.stream);
            _readStream.Resize(Protocol.headerSize);
            _readStream.Pos = 0;
            _readHeader = true;

            Debug.Assert(info.stream.Pos == info.stream.Size);

            try
            {
                //
                // The magic and version fields have already been checked.
                //
                info.stream.Pos = 8;
                byte messageType = info.stream.ReadByte();
                info.compress = info.stream.ReadByte();
                if (info.compress == 2)
                {
                    if (_compressionSupported)
                    {
                        IceInternal.Buffer ubuf = BZip2.uncompress(info.stream.GetBuffer(), Protocol.headerSize,
                                                                   _messageSizeMax);
                        info.stream = new InputStream(info.stream.Communicator, info.stream.Encoding, ubuf, true);
                    }
                    else
                    {
                        string lib = AssemblyUtil.isWindows ? "bzip2.dll" : "libbz2.so.1";
                        throw new FeatureNotSupportedException($"Cannot uncompress compressed message: {lib} not found");
                    }
                }
                info.stream.Pos = Protocol.headerSize;

                switch (messageType)
                {
                    case Protocol.closeConnectionMsg:
                        {
                            TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                            if (_endpoint.datagram())
                            {
                                if (_warn)
                                {
                                    _logger.warning("ignoring close connection message for datagram connection:\n" + _desc);
                                }
                            }
                            else
                            {
                                SetState(StateClosingPending, new CloseConnectionException());

                                //
                                // Notify the transceiver of the graceful connection closure.
                                //
                                int op = _transceiver.closing(false, _exception);
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
                                TraceUtil.trace("received request during closing\n" +
                                                "(ignored by server, client will retry)", info.stream, _logger,
                                                _traceLevels);
                            }
                            else
                            {
                                TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                                info.requestId = info.stream.ReadInt();
                                info.invokeNum = 1;
                                info.servantManager = _servantManager;
                                info.adapter = _adapter;
                                ++info.messageDispatchCount;
                            }
                            break;
                        }

                    case Protocol.requestBatchMsg:
                        {
                            if (_state >= StateClosing)
                            {
                                TraceUtil.trace("received batch request during closing\n" +
                                                "(ignored by server, client will retry)", info.stream, _logger,
                                                _traceLevels);
                            }
                            else
                            {
                                TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                                info.invokeNum = info.stream.ReadInt();
                                if (info.invokeNum < 0)
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
                            TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                            info.requestId = info.stream.ReadInt();
                            if (_asyncRequests.TryGetValue(info.requestId, out info.outAsync))
                            {
                                _asyncRequests.Remove(info.requestId);

                                info.outAsync.getIs().Swap(info.stream);

                                //
                                // If we just received the reply for a request which isn't acknowledge as
                                // sent yet, we queue the reply instead of processing it right away. It
                                // will be processed once the write callback is invoked for the message.
                                //
                                OutgoingMessage? message = _sendStreams.Count > 0 ? _sendStreams.First.Value : null;
                                if (message != null && message.outAsync == info.outAsync)
                                {
                                    message.receivedReply = true;
                                }
                                else if (info.outAsync.response())
                                {
                                    ++info.messageDispatchCount;
                                }
                                else
                                {
                                    info.outAsync = null;
                                }
                                System.Threading.Monitor.PulseAll(this); // Notify threads blocked in close()
                            }
                            break;
                        }

                    case Protocol.validateConnectionMsg:
                        {
                            TraceUtil.traceRecv(info.stream, _logger, _traceLevels);
                            if (_heartbeatCallback != null)
                            {
                                info.heartbeatCallback = _heartbeatCallback;
                                ++info.messageDispatchCount;
                            }
                            break;
                        }

                    default:
                        {
                            TraceUtil.trace("received unknown message\n(invalid, closing connection)",
                                            info.stream, _logger, _traceLevels);
                            throw new UnknownMessageException();
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
                    SetState(StateClosed, ex);
                }
            }

            return _state == StateHolding ? SocketOperation.None : SocketOperation.Read;
        }

        private void InvokeAll(InputStream stream, int invokeNum, int requestId, byte compress,
                               ServantManager? servantManager, ObjectAdapter? adapter)
        {
            //
            // Note: In contrast to other private or protected methods, this
            // operation must be called *without* the mutex locked.
            //

            Incoming? inc = null;
            try
            {
                while (invokeNum > 0)
                {
                    //
                    // Prepare the invocation.
                    //
                    bool response = !_endpoint.datagram() && requestId != 0;
                    Debug.Assert(!response || invokeNum == 1);

                    inc = GetIncoming(adapter, response, compress, requestId);

                    //
                    // Dispatch the invocation.
                    //
                    inc.invoke(servantManager, stream);

                    --invokeNum;

                    ReclaimIncoming(inc);
                    inc = null;
                }

                stream.Clear();
            }
            catch (LocalException ex)
            {
                InvokeException(requestId, ex, invokeNum, false);
            }
            finally
            {
                if (inc != null)
                {
                    ReclaimIncoming(inc);
                }
            }
        }

        private void ScheduleTimeout(int status)
        {
            int timeout;
            if (_state < StateActive)
            {
                DefaultsAndOverrides defaultsAndOverrides = _communicator.defaultsAndOverrides();
                if (defaultsAndOverrides.overrideConnectTimeout)
                {
                    timeout = defaultsAndOverrides.overrideConnectTimeoutValue;
                }
                else
                {
                    timeout = _endpoint.timeout();
                }
            }
            else if (_state < StateClosingPending)
            {
                if (_readHeader) // No timeout for reading the header.
                {
                    status &= ~SocketOperation.Read;
                }
                timeout = _endpoint.timeout();
            }
            else
            {
                DefaultsAndOverrides defaultsAndOverrides = _communicator.defaultsAndOverrides();
                if (defaultsAndOverrides.overrideCloseTimeout)
                {
                    timeout = defaultsAndOverrides.overrideCloseTimeoutValue;
                }
                else
                {
                    timeout = _endpoint.timeout();
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
                    _timer.cancel(_readTimeout);
                }
                _timer.schedule(_readTimeout, timeout);
                _readTimeoutScheduled = true;
            }
            if ((status & (SocketOperation.Write | SocketOperation.Connect)) != 0)
            {
                if (_writeTimeoutScheduled)
                {
                    _timer.cancel(_writeTimeout);
                }
                _timer.schedule(_writeTimeout, timeout);
                _writeTimeoutScheduled = true;
            }
        }

        private void UnscheduleTimeout(int status)
        {
            if ((status & SocketOperation.Read) != 0 && _readTimeoutScheduled)
            {
                _timer.cancel(_readTimeout);
                _readTimeoutScheduled = false;
            }
            if ((status & (SocketOperation.Write | SocketOperation.Connect)) != 0 &&
               _writeTimeoutScheduled)
            {
                _timer.cancel(_writeTimeout);
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
                _info = _transceiver.getInfo();
            }
            catch (LocalException)
            {
                _info = new ConnectionInfo();
            }
            for (ConnectionInfo? info = _info; info != null; info = info.Underlying)
            {
                info.ConnectionId = _endpoint.connectionId();
                info.AdapterName = _adapter != null ? _adapter.GetName() : "";
                info.Incoming = _connector == null;
            }
            return _info;
        }

        private void Reap()
        {
            if (_monitor != null)
            {
                _monitor.reap(this);
            }
            if (_observer != null)
            {
                _observer.detach();
            }
        }

        private ConnectionState ToConnectionState(int state)
        {
            return connectionStateMap[state];
        }

        private void Warning(string msg, System.Exception ex)
        {
            _logger.warning(msg + ":\n" + ex + "\n" + _transceiver.ToString());
        }

        private void ObserverStartRead(IceInternal.Buffer buf)
        {
            if (_readStreamPos >= 0)
            {
                Debug.Assert(!buf.empty());
                _observer!.receivedBytes(buf.b.position() - _readStreamPos);
            }
            _readStreamPos = buf.empty() ? -1 : buf.b.position();
        }

        private void ObserverFinishRead(IceInternal.Buffer buf)
        {
            if (_readStreamPos == -1)
            {
                return;
            }
            Debug.Assert(buf.b.position() >= _readStreamPos);
            _observer!.receivedBytes(buf.b.position() - _readStreamPos);
            _readStreamPos = -1;
        }

        private void ObserverStartWrite(IceInternal.Buffer buf)
        {
            if (_writeStreamPos >= 0)
            {
                Debug.Assert(!buf.empty());
                _observer!.sentBytes(buf.b.position() - _writeStreamPos);
            }
            _writeStreamPos = buf.empty() ? -1 : buf.b.position();
        }

        private void ObserverFinishWrite(IceInternal.Buffer buf)
        {
            if (_writeStreamPos == -1)
            {
                return;
            }
            if (buf.b.position() > _writeStreamPos)
            {
                _observer!.sentBytes(buf.b.position() - _writeStreamPos);
            }
            _writeStreamPos = -1;
        }

        private Incoming GetIncoming(ObjectAdapter adapter, bool response, byte compress, int requestId)
        {
            Incoming inc;

            if (_cacheBuffers)
            {
                lock (_incomingCacheMutex)
                {
                    if (_incomingCache == null)
                    {
                        inc = new Incoming(_communicator, this, this, adapter, response, compress, requestId);
                    }
                    else
                    {
                        inc = _incomingCache;
                        _incomingCache = _incomingCache.next;
                        inc.reset(_communicator, this, this, adapter, response, compress, requestId);
                        inc.next = null;
                    }
                }
            }
            else
            {
                inc = new Incoming(_communicator, this, this, adapter, response, compress, requestId);
            }

            return inc;
        }

        internal void ReclaimIncoming(Incoming inc)
        {
            if (_cacheBuffers && inc.reclaim())
            {
                lock (_incomingCacheMutex)
                {
                    inc.next = _incomingCache;
                    _incomingCache = inc;
                }
            }
        }

        private int Read(IceInternal.Buffer buf)
        {
            int start = buf.b.position();
            int op = _transceiver.read(buf, ref _hasMoreData);
            if (_communicator.traceLevels().network >= 3 && buf.b.position() != start)
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
                s.Append("\n");
                s.Append(ToString());
                _logger.trace(_communicator.traceLevels().networkCat, s.ToString());
            }
            return op;
        }

        private int Write(IceInternal.Buffer buf)
        {
            int start = buf.b.position();
            int op = _transceiver.write(buf);
            if (_communicator.traceLevels().network >= 3 && buf.b.position() != start)
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
                s.Append("\n");
                s.Append(ToString());
                _logger.trace(_communicator.traceLevels().networkCat, s.ToString());
            }
            return op;
        }

        private class OutgoingMessage
        {
            internal OutgoingMessage(OutputStream stream, bool compress, bool adopt)
            {
                this.stream = stream;
                this.compress = compress;
                _adopt = adopt;
            }

            internal OutgoingMessage(OutgoingAsyncBase outAsync, OutputStream stream, bool compress, int requestId)
            {
                this.outAsync = outAsync;
                this.stream = stream;
                this.compress = compress;
                this.requestId = requestId;
            }

            internal void Canceled()
            {
                Debug.Assert(outAsync != null); // Only requests can timeout.
                outAsync = null;
            }

            internal void Adopt()
            {
                if (_adopt)
                {
                    var stream = new OutputStream(this.stream!.communicator(), Util.currentProtocolEncoding);
                    stream.Swap(this.stream);
                    this.stream = stream;
                    _adopt = false;
                }
            }

            internal bool Sent()
            {
                stream = null;
                if (outAsync != null)
                {
                    invokeSent = outAsync.sent();
                    return invokeSent || receivedReply;
                }
                return false;
            }

            internal void Completed(LocalException ex)
            {
                if (outAsync != null)
                {
                    if (outAsync.exception(ex))
                    {
                        outAsync.invokeException();
                    }
                }
                stream = null;
            }

            internal OutputStream? stream;
            internal OutgoingAsyncBase? outAsync;
            internal bool compress;
            internal int requestId;
            internal bool _adopt;
            internal bool prepared;
            internal bool isSent;
            internal bool invokeSent;
            internal bool receivedReply;
        }

        private readonly Communicator _communicator;
        private IACMMonitor? _monitor;
        private readonly ITransceiver _transceiver;
        private string _desc;
        private readonly string _type;
        private readonly IConnector? _connector;
        private readonly Endpoint _endpoint;

        private ObjectAdapter? _adapter;
        private ServantManager? _servantManager;

        private readonly ILogger _logger;
        private readonly TraceLevels _traceLevels;
        private readonly IceInternal.ThreadPool _threadPool;

        private readonly IceInternal.Timer _timer;
        private readonly ITimerTask _writeTimeout;
        private bool _writeTimeoutScheduled;
        private readonly ITimerTask _readTimeout;
        private bool _readTimeoutScheduled;

        private StartCallback? _startCallback = null;

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

        private ICommunicatorObserver? _communicatorObserver;
        private IConnectionObserver? _observer;
        private int _readStreamPos;
        private int _writeStreamPos;

        private int _dispatchCount;

        private int _state; // The current state.
        private bool _shutdownInitiated = false;
        private bool _initialized = false;
        private bool _validated = false;

        private Incoming? _incomingCache;
        private readonly object _incomingCacheMutex = new object();

        private static readonly bool _compressionSupported = BZip2.supported();

        private readonly bool _cacheBuffers;

        private ConnectionInfo? _info;

        private CloseCallback? _closeCallback;
        private HeartbeatCallback? _heartbeatCallback;

        private static readonly ConnectionState[] connectionStateMap = new ConnectionState[] {
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
