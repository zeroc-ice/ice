// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The SocketStream class for the colocated transport.</summary>
    internal class ColocatedStream : SignaledSocketStream<(object, bool)>
    {
        protected override bool ReceivedEndOfStream => _receivedEndOfStream;

        private ConcurrentQueue<ArraySegment<byte>>? _receivedData;
        private bool _receivedEndOfStream;
        private ArraySegment<byte> _receiveSegment;
        private readonly ColocatedSocket _socket;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);

            if (disposing)
            {
                _socket.ReleaseFlowControlCredit(this);
            }
        }

        protected override async ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel)
        {
            int received = 0;
            while (buffer.Length > 0)
            {
                if (_receiveSegment.Count > 0 || (_receivedData?.TryDequeue(out _receiveSegment) ?? false))
                {
                    if (_receiveSegment.Count == 0)
                    {
                        _receivedEndOfStream = true;
                        return received;
                    }
                    else if (buffer.Length < _receiveSegment.Count)
                    {
                        _receiveSegment[0..buffer.Length].AsMemory().CopyTo(buffer);
                        received += buffer.Length;
                        _receiveSegment = _receiveSegment[buffer.Length..];
                        buffer = buffer[buffer.Length..];
                    }
                    else
                    {
                        _receiveSegment.AsMemory().CopyTo(buffer);
                        received += _receiveSegment.Count;
                        _receiveSegment = new ArraySegment<byte>();
                        buffer = buffer[_receiveSegment.Count..];
                    }
                }
                else
                {
                    await WaitSignalAsync(cancel).ConfigureAwait(false);
                }
            }
            return received;
        }

        protected override ValueTask ResetAsync(long errorCode) =>
            // A null frame indicates a stream reset.
            // TODO: Provide the error code?
            _socket.SendFrameAsync(this, frame: null, fin: true, CancellationToken.None);

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel) =>
            _socket.SendFrameAsync(this, frame: new List<ArraySegment<byte>>(buffer), fin: fin, cancel);

        /// <summary>Constructor for incoming colocated stream</summary>
        internal ColocatedStream(ColocatedSocket socket, long streamId)
            : base(socket, streamId) => _socket = socket;

        /// <summary>Constructor for outgoing colocated stream</summary>
        internal ColocatedStream(ColocatedSocket socket, bool bidirectional, bool control)
            : base(socket, bidirectional, control) => _socket = socket;

        internal void ReceivedFrame(object frame, bool fin)
        {
            if (_receivedData != null)
            {
                Debug.Assert(frame is List<ArraySegment<byte>>);
                var data = (List<ArraySegment<byte>>)frame;
                Debug.Assert(data.Count == 1);
                _receivedData.Enqueue(data[0]);
                if (fin)
                {
                    _receivedData.Enqueue(ArraySegment<byte>.Empty);
                }
            }
            else if (frame is IncomingFrame && !fin)
            {
                // If it's a request or response and the stream is not finished, create a concurrent queue to
                // keep track of additional data frames.
                _receivedData = new ConcurrentQueue<ArraySegment<byte>>();
            }

            // Run the continuation asynchronously if it's a response to ensure we don't end up calling user
            // code which could end up blocking the AcceptStreamAsync task.
            if (!IsSignaled)
            {
                SignalCompletion((frame, fin));
            }
        }

        internal override async ValueTask<IncomingRequestFrame> ReceiveRequestFrameAsync(CancellationToken cancel)
        {
            (object frameObject, bool fin) = await WaitSignalAsync(cancel).ConfigureAwait(false);
            Debug.Assert(frameObject is IncomingRequestFrame);
            var frame = (IncomingRequestFrame)frameObject;

            if (fin)
            {
                _receivedEndOfStream = true;
            }
            else
            {
                frame.SocketStream = this;
                Interlocked.Increment(ref UseCount);
            }
            return frame;
        }

        internal override async ValueTask<IncomingResponseFrame> ReceiveResponseFrameAsync(CancellationToken cancel)
        {
            object frameObject;
            bool fin;

            try
            {
                (frameObject, fin) = await WaitSignalAsync(cancel).ConfigureAwait(false);
            }
            catch (OperationCanceledException)
            {
                if (_socket.Endpoint.Protocol != Protocol.Ice1)
                {
                    await ResetAsync((long)StreamResetErrorCode.RequestCanceled).ConfigureAwait(false);
                }
                throw;
            }

            Debug.Assert(frameObject is IncomingResponseFrame);
            var frame = (IncomingResponseFrame)frameObject;

            if (fin)
            {
                _receivedEndOfStream = true;
            }
            else
            {
                frame.SocketStream = this;
                Interlocked.Increment(ref UseCount);
            }

            return frame;
        }

        private protected override async ValueTask<ArraySegment<byte>> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            (object frame, bool fin) = await WaitSignalAsync(cancel).ConfigureAwait(false);
            if (fin)
            {
                _receivedEndOfStream = true;
            }

            if (frame is List<ArraySegment<byte>> data)
            {
                // Initialize or GoAway frame.
                if (_socket.Endpoint.Protocol == Protocol.Ice1)
                {
                    Debug.Assert(expectedFrameType == data[0][8]);
                    return ArraySegment<byte>.Empty;
                }
                else
                {
                    Debug.Assert(expectedFrameType == data[0][0]);
                    (int size, int sizeLength) = data[0][1..].AsReadOnlySpan().ReadSize20();
                    return data[0].Slice(1 + sizeLength, size);
                }
            }
            else
            {
                Debug.Assert(false);
                throw new InvalidDataException("unexpected frame");
            }
        }

        private protected override async ValueTask SendFrameAsync(OutgoingFrame frame, CancellationToken cancel)
        {
            await _socket.SendFrameAsync(this, frame.ToIncoming(), fin: frame.StreamDataWriter == null, cancel).
                ConfigureAwait(false);

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame);
            }
        }
    }
}
