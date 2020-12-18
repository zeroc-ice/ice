// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    /// <summary>A multi-stream socket represents the local end of a network connection and enables transmitting raw
    /// binary data over multiple independent streams. The data sent and received over these streams can either be
    /// transmitted using a datagram oriented transport such as Quic or a stream oriented transport such as TCP
    /// (data multiplexing is used to transmit the data from multiple concurrent streams over the same TCP socket).
    /// The Ice core relies on a multi-stream sockets to support the Ice protocol.
    /// </summary>
    public abstract class MultiStreamSocket : IDisposable
    {
        /// <summary>The endpoint from which the socket was created.</summary>
        public Endpoint Endpoint { get; }

        /// <summary>Gets or set the idle timeout.</summary>
        public abstract TimeSpan IdleTimeout { get; internal set; }

        /// <summary><c>true</c> for incoming sockets <c>false</c> otherwise. An incoming socket is created
        /// by a server-side acceptor while an outgoing socket is created from the endpoint by the client-side.
        /// </summary>
        public bool IsIncoming { get; }

        internal int IncomingFrameMaxSize { get; }
        internal int? PeerIncomingFrameMaxSize { get; set; }
        internal TimeSpan LastActivity { get; private set; }
        // The stream ID of the last received response with the Ice1 protocol. Keeping track of this stream ID is
        // necessary to avoid a race condition with the GoAway frame which could be received and processed before
        // the response is delivered to the stream.
        internal long LastResponseStreamId { get; set; }
        internal IConnectionObserver? Observer
        {
            get
            {
                lock (_mutex)
                {
                    return _observer;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _observer = value;
                    _observer?.Attach();
                }
            }
        }
        internal event EventHandler? Ping;
        internal int IncomingStreamCount => Thread.VolatileRead(ref _incomingStreamCount);
        internal int OutgoingStreamCount => Thread.VolatileRead(ref _outgoingStreamCount);

        private int _incomingStreamCount;
        // The mutex provides thread-safety for the _observer, _streamsAborted and LastActivity data members.
        private readonly object _mutex = new();
        private IConnectionObserver? _observer;
        private int _outgoingStreamCount;
        private readonly ConcurrentDictionary<long, SocketStream> _streams = new();
        private bool _streamsAborted;
        private volatile TaskCompletionSource? _streamsEmptySource;

        /// <summary>Aborts the socket.</summary>
        public abstract void Abort();

        /// <summary>Accepts an incoming stream.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The accepted stream.</return>
        public abstract ValueTask<SocketStream> AcceptStreamAsync(CancellationToken cancel);

        /// <summary>Closes the socket.</summary>
        /// <param name="exception">The exception for which the socket is closed.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public abstract ValueTask CloseAsync(Exception exception, CancellationToken cancel);

        /// <summary>Releases the resources used by the socket.</summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>Sends a ping frame to defer the idle timeout.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public abstract Task PingAsync(CancellationToken cancel);

        /// <summary>Initializes the transport.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public abstract ValueTask InitializeAsync(CancellationToken cancel);

        /// <summary>Creates an outgoing stream. Depending on the transport implementation, the stream ID might not
        /// be immediately available after the stream creation. It will be available after the first successful send
        /// call on the stream.</summary>
        /// <param name="bidirectional"><c>True</c> to create a bidirectional stream, <c>false</c> otherwise.</param>
        /// <param name="control"><c>True</c> to create a control stream, <c>false</c> otherwise.</param>
        /// <return>The outgoing stream.</return>
        public abstract SocketStream CreateStream(bool bidirectional, bool control);

        /// <summary>The MultiStreamSocket constructor.</summary>
        /// <param name="endpoint">The endpoint from which the socket was created.</param>
        /// <param name="adapter">The object adapter from which the socket was created or null if the socket
        /// is an outgoing socket created from the communicator.</param>
        protected MultiStreamSocket(Endpoint endpoint, ObjectAdapter? adapter)
        {
            Endpoint = endpoint;
            IsIncoming = adapter != null;
            IncomingFrameMaxSize = adapter?.IncomingFrameMaxSize ?? Endpoint.Communicator.IncomingFrameMaxSize;
            LastActivity = Time.Elapsed;
        }

        /// <summary>Releases the resources used by the socket.</summary>
        /// <param name="disposing">True to release both managed and unmanaged resources; false to release only
        /// unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            // Dispose of the control streams.
            foreach (SocketStream stream in _streams.Values)
            {
                Debug.Assert(stream.IsControl);
                stream.Dispose();
            }
        }

        /// <summary>Notifies the observer and traces the given received amount of data. Transport implementations
        /// should call this method to trace the received data.</summary>
        /// <param name="size">The size in bytes of the received data.</param>
        protected void Received(int size)
        {
            lock (_mutex)
            {
                Debug.Assert(size > 0);
                _observer?.ReceivedBytes(size);

                LastActivity = Time.Elapsed;
            }

            if (Endpoint.Communicator.TraceLevels.Transport >= 3)
            {
                Endpoint.Communicator.Logger.Trace(TraceLevels.TransportCategory,
                    $"received {size} bytes via {Endpoint.TransportName}\n{this}");
            }
        }

        /// <summary>Notifies event handlers of the received ping. Transport implementations should call this method
        /// when a ping is received.</summary>
        protected void ReceivedPing()
        {
            // Capture the event handler which can be modified anytime by the user code.
            EventHandler? callback = Ping;
            if (callback != null)
            {
                Task.Run(() =>
                {
                    try
                    {
                        callback.Invoke(this, EventArgs.Empty);
                    }
                    catch (Exception ex)
                    {
                        Endpoint.Communicator.Logger.Error($"ping event handler raised an exception:\n{ex}\n{this}");
                    }
                });
            }
        }

        /// <summary>Notifies the observer and traces the given sent amount of data. Transport implementations
        /// should call this method to trace the data sent.</summary>
        /// <param name="size">The size in bytes of the data sent.</param>
        protected void Sent(int size)
        {
            lock (_mutex)
            {
                Debug.Assert(size > 0);
                _observer?.SentBytes(size);

                LastActivity = Time.Elapsed;
            }

            if (Endpoint.Communicator.TraceLevels.Transport >= 3 && size > 0)
            {
                Endpoint.Communicator.Logger.Trace(TraceLevels.TransportCategory,
                    $"sent {size} bytes via {Endpoint.TransportName}\n{this}");
            }
        }

        /// <summary>Try to get a stream with the given ID. Transport implementations can use this method to lookup
        /// an existing stream.</summary>
        /// <param name="streamId">The stream ID.</param>
        /// <param name="value">If found, value is assigned to the stream value, null otherwise.</param>
        /// <return>True if the stream was found and value contains a non-null value, False otherwise.</return>
        protected bool TryGetStream<T>(long streamId, [NotNullWhen(returnValue: true)] out T? value)
            where T : SocketStream
        {
            if (_streams.TryGetValue(streamId, out SocketStream? stream))
            {
                value = (T)stream;
                return true;
            }
            value = null;
            return false;
        }

        internal async ValueTask AbortAsync(Exception exception)
        {
            // Abort the transport.
            Abort();

            // Consider the abort as graceful if the streams were already aborted.
            bool graceful;
            lock (_mutex)
            {
                graceful = _streamsAborted;
            }

            // Abort the streams if not already done and wait for all the streams to be completed. It's important to
            // call this again even if has already been called previously by graceful connection closure. Not all the
            // streams might have been aborted and at this point we want to make sure all the streams are aborted.
            AbortStreams(exception);

            await WaitForEmptyStreamsAsync().ConfigureAwait(false);

            lock (_mutex)
            {
                _observer?.Detach();
            }

            if (Endpoint.Communicator.TraceLevels.Transport >= 1)
            {
                var s = new StringBuilder();
                s.Append("closed ");
                s.Append(Endpoint.TransportName);
                s.Append(" connection\n");
                s.Append(ToString());

                // Trace the cause of unexpected connection closures
                if (!graceful && !(exception is ConnectionClosedException || exception is ObjectDisposedException))
                {
                    s.Append("\nexception = ");
                    s.Append(exception);
                }

                Endpoint.Communicator.Logger.Trace(TraceLevels.TransportCategory, s.ToString());
            }
        }

        internal virtual (long, long) AbortStreams(Exception exception, Func<SocketStream, bool>? predicate = null)
        {
            lock (_mutex)
            {
                // Set the _streamsAborted flag to prevent addition of new streams by AddStream. No more streams will
                // be added to _streams once this flag is true.
                _streamsAborted = true;
            }

            // Cancel the streams based on the given predicate. Control streams are not canceled since they are
            // still needed for sending and receiving GoAway frames.
            long largestBidirectionalStreamId = 0;
            long largestUnidirectionalStreamId = 0;
            foreach (SocketStream stream in _streams.Values)
            {
                if (!stream.IsControl && (predicate?.Invoke(stream) ?? true))
                {
                    stream.Abort(exception);
                }
                else if (stream.IsBidirectional)
                {
                    if (stream.Id > largestBidirectionalStreamId)
                    {
                        largestBidirectionalStreamId = stream.Id;
                    }
                }
                else if (stream.Id > largestUnidirectionalStreamId)
                {
                    largestUnidirectionalStreamId = stream.Id;
                }
            }
            return (largestBidirectionalStreamId, largestUnidirectionalStreamId);
        }

        internal void AddStream(long id, SocketStream stream, bool control)
        {
            lock (_mutex)
            {
                if (_streamsAborted)
                {
                    throw new ConnectionClosedException(isClosedByPeer: false, RetryPolicy.AfterDelay(TimeSpan.Zero));
                }
                _streams[id] = stream;
            }

            if (!control)
            {
                Interlocked.Increment(ref stream.IsIncoming ? ref _incomingStreamCount : ref _outgoingStreamCount);
            }
        }

        internal void CheckStreamsEmpty()
        {
            if (IncomingStreamCount == 0 && OutgoingStreamCount == 0)
            {
                _streamsEmptySource?.TrySetResult();
            }
        }

        internal void Initialized()
        {
            lock (_mutex)
            {
                LastActivity = Time.Elapsed;
            }

            if (Endpoint.Communicator.TraceLevels.Transport >= 1)
            {
                var s = new StringBuilder();
                if (Endpoint.IsDatagram)
                {
                    s.Append("starting to ");
                    s.Append(IsIncoming ? "receive" : "send");
                    s.Append(' ');
                    s.Append(Endpoint.TransportName);
                    s.Append(" datagrams\n");
                }
                else
                {
                    s.Append(IsIncoming ? "accepted" : "established");
                    s.Append(' ');
                    s.Append(Endpoint.TransportName);
                    s.Append(" connection\n");
                }
                s.Append(ToString());
                Endpoint.Communicator.Logger.Trace(TraceLevels.TransportCategory, s.ToString());
            }
        }

        internal virtual async ValueTask<SocketStream> ReceiveInitializeFrameAsync(CancellationToken cancel)
        {
            SocketStream stream = await AcceptStreamAsync(cancel).ConfigureAwait(false);
            await stream.ReceiveInitializeFrameAsync(cancel).ConfigureAwait(false);
            return stream;
        }

        internal void RemoveStream(long id)
        {
            if (_streams.TryRemove(id, out SocketStream? stream))
            {
                if (!stream.IsControl)
                {
                    Interlocked.Decrement(ref stream.IsIncoming ? ref _incomingStreamCount : ref _outgoingStreamCount);
                }
                CheckStreamsEmpty();
            }
        }

        internal virtual async ValueTask<SocketStream> SendInitializeFrameAsync(CancellationToken cancel)
        {
            SocketStream stream = CreateStream(bidirectional: false, control: true);
            await stream.SendInitializeFrameAsync(cancel).ConfigureAwait(false);
            return stream;
        }

        internal void TraceFrame(long streamId, object frame, byte type = 0, byte compress = 0)
        {
            Communicator communicator = Endpoint.Communicator;
            Protocol protocol = Endpoint.Protocol;

            Debug.Assert(communicator.TraceLevels.Protocol >= 1);

            string framePrefix;
            string frameType;
            Encoding encoding;
            int frameSize;
            ArraySegment<byte> data = ArraySegment<byte>.Empty;

            if (frame is OutgoingFrame outgoingFrame)
            {
                framePrefix = "sent";
                encoding = outgoingFrame.PayloadEncoding;
                frameType = frame is OutgoingRequestFrame ? "Request" : "Response";

                // TODO: fix tracing to trace payload size as opposed to frame size.
                frameSize = outgoingFrame.PayloadSize;
            }
            else if (frame is IncomingFrame incomingFrame)
            {
                framePrefix = "received";
                encoding = incomingFrame.PayloadEncoding;
                frameType = frame is IncomingRequestFrame ? "Request" : "Response";
                frameSize = incomingFrame.PayloadSize;
            }
            else
            {
                if (frame is IList<ArraySegment<byte>> sendBuffer)
                {
                    framePrefix = "sent";
                    data = sendBuffer.Count > 0 ? sendBuffer.AsArraySegment() : ArraySegment<byte>.Empty;
                    frameSize = sendBuffer.GetByteCount();
                }
                else if (frame is ArraySegment<byte> readBuffer)
                {
                    framePrefix = "received";
                    data = readBuffer;
                    frameSize = readBuffer.Count;
                }
                else
                {
                    Debug.Assert(false);
                    return;
                }

                if (protocol == Protocol.Ice2)
                {
                    frameType = (Ice2FrameType)type switch
                    {
                        Ice2FrameType.Initialize => "Initialize",
                        Ice2FrameType.GoAway => "GoAway",
                        _ => "Unknown"
                    };
                    encoding = Ice2Definitions.Encoding;
                }
                else
                {
                    frameType = (Ice1FrameType)type switch
                    {
                        Ice1FrameType.ValidateConnection => "ValidateConnection",
                        Ice1FrameType.CloseConnection => "CloseConnection",
                        Ice1FrameType.RequestBatch => "RequestBatch",
                        _ => "Unknown"
                    };
                    encoding = Ice1Definitions.Encoding;
                }
            }

            var s = new StringBuilder();
            s.Append(framePrefix);
            s.Append(' ');
            s.Append(frameType);
            s.Append(" via ");
            s.Append(Endpoint.TransportName);

            s.Append("\nprotocol = ");
            s.Append(protocol.GetName());
            s.Append("\nencoding = ");
            s.Append(encoding.ToString());

            s.Append("\nframe size = ");
            s.Append(frameSize);

            if (protocol == Protocol.Ice2)
            {
                s.Append("\nstream ID = ");
                s.Append(streamId);
                s.Append((streamId % 4) switch
                {
                    0 => " (client-initiated, bidirectional)",
                    1 => " (server-initiated, bidirectional)",
                    2 => " (client-initiated, unidirectional)",
                    3 => " (server-initiated, unidirectional)",
                    _ => throw new InvalidArgumentException(nameof(streamId))
                });
            }
            else if (frameType == "Request" || frameType == "Response")
            {
                s.Append("\ncompression status = ");
                s.Append(compress);
                s.Append(compress switch
                {
                    0 => " (not compressed; do not compress response, if any)",
                    1 => " (not compressed; compress response, if any)",
                    2 => " (compressed; compress response, if any)",
                    _ => " (unknown)"
                });

                s.Append("\nrequest ID = ");
                int requestId = streamId % 4 < 2 ? (int)(streamId >> 2) + 1 : 0;
                s.Append(requestId);
                if (requestId == 0)
                {
                    s.Append(" (oneway)");
                }
            }

            if (frameType == "Request")
            {
                Identity identity;
                string facet;
                string operation;
                bool isIdempotent;
                IReadOnlyDictionary<string, string> context;
                if (frame is OutgoingRequestFrame outgoingRequest)
                {
                    identity = outgoingRequest.Identity;
                    facet = outgoingRequest.Facet;
                    operation = outgoingRequest.Operation;
                    isIdempotent = outgoingRequest.IsIdempotent;
                    context = outgoingRequest.Context;
                }
                else if (frame is IncomingRequestFrame incomingRequest)
                {
                    Debug.Assert(incomingRequest != null);
                    identity = incomingRequest.Identity;
                    facet = incomingRequest.Facet;
                    operation = incomingRequest.Operation;
                    isIdempotent = incomingRequest.IsIdempotent;
                    context = incomingRequest.Context;
                }
                else
                {
                    Debug.Assert(false);
                    return;
                }

                ToStringMode toStringMode = communicator.ToStringMode;
                s.Append("\nidentity = ");
                s.Append(identity.ToString(toStringMode));

                s.Append("\nfacet = ");
                if (facet.Length > 0)
                {
                    s.Append(StringUtil.EscapeString(facet, toStringMode));
                }

                s.Append("\noperation = ");
                s.Append(operation);

                s.Append($"\nidempotent = ");
                s.Append(isIdempotent.ToString().ToLowerInvariant());

                int sz = context.Count;
                s.Append("\ncontext = ");
                foreach ((string key, string value) in context)
                {
                    s.Append(key);
                    s.Append('/');
                    s.Append(value);
                    if (--sz > 0)
                    {
                        s.Append(", ");
                    }
                }
            }
            else if (frameType == "Response")
            {
                s.Append("\nresult type = ");
                if (frame is IncomingResponseFrame incomingResponseFrame)
                {
                    s.Append(incomingResponseFrame.ResultType);
                }
                else if (frame is OutgoingResponseFrame outgoingResponseFrame)
                {
                    s.Append(outgoingResponseFrame.ResultType);
                }
            }
            else if (frameType == "RequestBatch")
            {
                s.Append("\nnumber of requests = ");
                s.Append(data.AsReadOnlySpan().ReadInt());
            }
            else if (frameType == "GoAway")
            {
                if (protocol == Protocol.Ice2)
                {
                    var istr = new InputStream(data, encoding);
                    s.Append("\nlast bidirectional stream ID = ");
                    s.Append(istr.ReadVarULong());
                    s.Append("\nlast unidirectional stream ID = ");
                    s.Append(istr.ReadVarULong());
                    s.Append("\nmessage from peer = ");
                    s.Append(istr.ReadString());
                }
                else
                {
                    s.Append("\nlast request ID = ");
                    s.Append((int)(LastResponseStreamId >> 2) + 1);
                }
            }

            s.Append('\n');
            s.Append(ToString());

            communicator.Logger.Trace(TraceLevels.ProtocolCategory, s.ToString());
        }

        internal virtual async ValueTask WaitForEmptyStreamsAsync()
        {
            if (IncomingStreamCount > 0 || OutgoingStreamCount > 0)
            {
                // Create a task completion source to wait for the streams to complete.
                _streamsEmptySource ??= new TaskCompletionSource();
                CheckStreamsEmpty();
                await _streamsEmptySource.Task.ConfigureAwait(false);
            }
        }
    }
}
