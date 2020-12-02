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
        protected override ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;
        protected override bool ReceivedEndOfStream => _receivedEndOfStream;
        protected override bool SentEndOfStream => _sentEndOfStream;

        private bool _receivedEndOfStream;
        private bool _sentEndOfStream;
        private volatile object? _streamable;
        private readonly ColocatedSocket _socket;

        public override System.IO.Stream ReceiveDataIntoIOStream()
        {
            if (ReceiveStreamable() is not System.IO.Stream ioStream)
            {
                throw new InvalidDataException("unexpected data");
            }
            return ioStream;
        }

        public override void SendDataFromIOStream(System.IO.Stream ioStream, CancellationToken cancel) =>
            Task.Run(() => _socket.SendFrameAsync(this, frame: ioStream, fin: true, cancel));

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);

            if (disposing)
            {
                _socket.ReleaseFlowControlCredit(this);
            }
        }

        protected override ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel) =>
            // This is never called because we override the default ReceiveFrameAsync implementation
            throw new NotImplementedException();

        protected override ValueTask ResetAsync(long errorCode) =>
            // A null frame indicates a stream reset.
            // TODO: Provide the error code?
            _socket.SendFrameAsync(this, frame: null, fin: true, CancellationToken.None);

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel)
        {
            if (fin)
            {
                _sentEndOfStream = true;
            }
            return _socket.SendFrameAsync(this, frame: buffer, fin: fin, cancel);
        }

        /// <summary>Constructor for incoming colocated stream</summary>
        internal ColocatedStream(ColocatedSocket socket, long streamId)
            : base(socket, streamId) => _socket = socket;

        /// <summary>Constructor for outgoing colocated stream</summary>
        internal ColocatedStream(ColocatedSocket socket, bool isBidirectional, bool isControl)
            : base(socket, isBidirectional, isControl) => _socket = socket;

        internal void ReceivedFrame(object frame, bool fin)
        {
            if (IsSignaled)
            {
                Debug.Assert(fin);
                _streamable = frame;
                if (!IsSignaled)
                {
                    SignalCompletion((frame, true));
                }
            }
            else
            {
                // Run the continuation asynchronously if it's a response to ensure we don't end up calling user
                // code which could end up blocking the AcceptStreamAsync task.
                SignalCompletion((frame, fin), runContinuationAsynchronously: frame is OutgoingResponseFrame);
            }
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

            if (frame is OutgoingFrame outgoingFrame)
            {
                return outgoingFrame.Data.AsArraySegment();
            }
            else if (frame is List<ArraySegment<byte>> data)
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
                    (int size, int sizeLength) = data[0].Slice(1).AsReadOnlySpan().ReadSize20();
                    return data[0].Slice(1 + sizeLength, size);
                }
            }
            else
            {
                Debug.Assert(false);
                throw new InvalidDataException("unexpected frame");
            }
        }

        private object ReceiveStreamable()
        {
            object frame;
            if (_streamable != null)
            {
                _receivedEndOfStream = true;
                frame = _streamable;
            }
            else
            {
                (frame, _receivedEndOfStream) = WaitSignalAsync().AsTask().Result;
                Debug.Assert(_receivedEndOfStream);
            }
            TryDispose();
            return frame;
        }

        private protected override async ValueTask SendFrameAsync(OutgoingFrame frame, CancellationToken cancel)
        {
            bool fin = frame.StreamDataWriter == null;
            await _socket.SendFrameAsync(this, frame, fin, cancel).ConfigureAwait(false);
            if (fin)
            {
                _sentEndOfStream = true;
            }

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame);
            }
        }
    }
}
