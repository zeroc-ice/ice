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
    /// <summary>The MultiStreamTransceiver abstract base class to implement multi-stream transports.</summary>
    public abstract class MultiStreamTransceiver : IDisposable
    {
        /// <summary>The endpoint from which the transceiver was created.</summary>
        public Endpoint Endpoint { get; }

        /// <summary><c>true</c> for incoming transceivers <c>false</c> otherwise. An incoming transceiver is created
        /// by a server-side acceptor while an outgoing transceiver is created from the endpoint by the client-side.
        /// </summary>
        public bool IsIncoming { get; }

        internal int IncomingFrameSizeMax { get; }
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
        internal int StreamCount => _streams.Count;

        // The mutex provides thread-safety for the _observer and LastActivity data members.
        private readonly object _mutex = new ();
        private IConnectionObserver? _observer;
        private readonly ConcurrentDictionary<long, TransceiverStream> _streams = new ();
        private volatile bool _streamsAborted;
        private volatile TaskCompletionSource? _streamsEmptySource;

        /// <summary>Aborts the transceiver.</summary>
        public abstract void Abort();

        /// <summary>Accepts an incoming stream.</summary>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>The accepted stream.</return>
        public abstract ValueTask<TransceiverStream> AcceptStreamAsync(CancellationToken cancel);

        /// <summary>Closes the transceiver.</summary>
        /// <param name="exception">The exception for which the transceiver is closed.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        public abstract ValueTask CloseAsync(Exception exception, CancellationToken cancel);

        /// <summary>Releases the resources used by the transceiver.</summary>
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
        /// <return>The outgoing stream.</return>
        public abstract TransceiverStream CreateStream(bool bidirectional);

        /// <summary>The MultiStreamTransceiver constructor.</summary>
        /// <param name="endpoint">The endpoint from which the transceiver was created.</param>
        /// <param name="adapter">The object adapter from which the transceiver was created or null if the transceiver
        /// is an outgoing transceiver created from the communicator.</param>
        protected MultiStreamTransceiver(Endpoint endpoint, ObjectAdapter? adapter)
        {
            Endpoint = endpoint;
            IsIncoming = adapter != null;
            IncomingFrameSizeMax = adapter?.IncomingFrameSizeMax ?? Endpoint.Communicator.IncomingFrameSizeMax;
            LastActivity = Time.Elapsed;
        }

        /// <summary>Releases the resources used by the transceiver.</summary>
        /// <param name="disposing">True to release both managed and unmanaged resources; false to release only
        /// unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            // The only streams left at this point should be the control stream. The connection ensures other streams
            // are aborted and disposed when the connection is aborted.
            foreach (TransceiverStream stream in _streams.Values)
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
            where T : TransceiverStream
        {
            if (_streams.TryGetValue(streamId, out TransceiverStream? stream))
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
            bool graceful = _streamsAborted;

            // Abort the streams if not already done and wait for all the streams to be completed.
            if (!_streamsAborted)
            {
                AbortStreams(exception);
            }
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

        internal virtual (long, long) AbortStreams(Exception exception, Func<TransceiverStream, bool>? predicate = null)
        {
            // Set the _streamsAborted flag to prevent addition of new streams to the _streams collection.
            _streamsAborted = true;

            // Cancel the streams based on the given predicate. Control streams are not canceled since they are
            // still needed for sending and receiving GoAway frames.
            long largestBidirectionalStreamId = 0;
            long largestUnidirectionalStreamId = 0;
            foreach (TransceiverStream stream in _streams.Values)
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

        internal void AddStream(long id, TransceiverStream stream)
        {
            if (_streamsAborted)
            {
                throw new ConnectionClosedException(isClosedByPeer: false, RetryPolicy.AfterDelay(TimeSpan.Zero));
            }
            _streams[id] = stream;
        }

        internal void CheckStreamsEmpty()
        {
            if (_streams.Count <= 2)
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

        internal virtual async ValueTask<TransceiverStream> ReceiveInitializeFrameAsync(CancellationToken cancel)
        {
            TransceiverStream stream = await AcceptStreamAsync(cancel).ConfigureAwait(false);
            await stream.ReceiveInitializeFrameAsync(cancel).ConfigureAwait(false);
            return stream;
        }

        internal void RemoveStream(long id)
        {
            if (_streams.TryRemove(id, out TransceiverStream _))
            {
                CheckStreamsEmpty();
            }
        }

        internal virtual async ValueTask<TransceiverStream> SendInitializeFrameAsync(CancellationToken cancel)
        {
            TransceiverStream stream = CreateControlStream();
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
                encoding = outgoingFrame.Encoding;
                frameType = frame is OutgoingRequestFrame ? "Request" : "Response";
                frameSize = outgoingFrame.Size;
            }
            else if (frame is IncomingFrame incomingFrame)
            {
                framePrefix = "received";
                encoding = incomingFrame.Encoding;
                frameType = frame is IncomingRequestFrame ? "Request" : "Response";
                frameSize = incomingFrame.Size;
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
                    frameType = (Ice2Definitions.FrameType)type switch
                    {
                        Ice2Definitions.FrameType.Initialize => "Initialize",
                        Ice2Definitions.FrameType.GoAway => "GoAway",
                        _ => "Unknown"
                    };
                    encoding = Ice2Definitions.Encoding;
                }
                else
                {
                    frameType = (Ice1Definitions.FrameType)type switch
                    {
                        Ice1Definitions.FrameType.ValidateConnection => "ValidateConnection",
                        Ice1Definitions.FrameType.CloseConnection => "CloseConnection",
                        Ice1Definitions.FrameType.RequestBatch => "RequestBatch",
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
            else if (protocol == Protocol.Ice2 && frameType == "GoAway")
            {
                var istr = new InputStream(data, encoding);
                s.Append("\nlast bidirectional stream ID = ");
                s.Append(istr.ReadVarLong());
                s.Append("\nlast unidirectional stream ID = ");
                s.Append(istr.ReadVarLong());
                s.Append("\nmessage from peer = ");
                s.Append(istr.ReadString());
            }

            s.Append('\n');
            s.Append(ToString());

            communicator.Logger.Trace(TraceLevels.ProtocolCategory, s.ToString());
        }

        internal async ValueTask WaitForEmptyStreamsAsync()
        {
            if (_streams.Count > 2)
            {
                // Create a task completion source to wait for the streams to complete.
                _streamsEmptySource ??= new TaskCompletionSource();
                CheckStreamsEmpty();
                await _streamsEmptySource.Task.ConfigureAwait(false);
            }
        }

        private protected virtual TransceiverStream CreateControlStream() => CreateStream(bidirectional: false);
    }
}
