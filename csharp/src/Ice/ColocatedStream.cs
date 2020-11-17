// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
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
        private readonly ColocatedSocket _socket;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);

            if (disposing)
            {
                if (IsIncoming && !IsBidirectional && !IsControl)
                {
                    _socket.PeerUnidirectionalSerializeSemaphore?.Release();
                }
                else if (!IsIncoming && IsBidirectional && IsStarted)
                {
                    _socket.BidirectionalSerializeSemaphore?.Release();
                }
            }
        }

        protected override ValueTask<bool> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel) =>
            // This is never called because we override the default ReceiveFrameAsync implementation
            throw new NotImplementedException();

        protected override ValueTask ResetAsync(long errorCode) =>
            // A null frame indicates a stream reset.
            // TODO: Provide the error code?
            _socket.SendFrameAsync(Id, frame: null, fin: true, CancellationToken.None);

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel)
        {
            // This is only called for Initialize/GoAway frame on the control streams. Requests or responses are
            // handled by the SendFrameAsync method override below.
            if (!IsStarted)
            {
                Id = _socket.AllocateId(false);
            }
            Debug.Assert(IsControl && !IsBidirectional);
            return _socket.SendFrameAsync(Id, buffer, fin, cancel);
        }

        /// <summary>Constructor for incoming colocated stream</summary>
        internal ColocatedStream(long streamId, ColocatedSocket socket)
            : base(streamId, socket) => _socket = socket;

        /// <summary>Constructor for outgoing colocated stream</summary>
        internal ColocatedStream(bool bidirectional, ColocatedSocket socket)
            : base(bidirectional, socket) => _socket = socket;

        internal void ReceivedFrame(object frame, bool fin) =>
            // Run the continuation asynchronously if it's a response to ensure we don't end up calling user
            // code which could end up blocking the AcceptStreamAsync task.
            SignalCompletion((frame, fin), runContinuationAsynchronously: frame is OutgoingResponseFrame);

        private protected override async ValueTask<(ArraySegment<byte>, bool)> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            (object frame, bool fin) = await WaitSignalAsync(cancel).ConfigureAwait(false);
            if (frame is OutgoingRequestFrame request)
            {
                return (request.Data.AsArraySegment(), fin);
            }
            else if (frame is OutgoingResponseFrame response)
            {
                return (response.Data.AsArraySegment(), fin);
            }
            else if (frame is List<ArraySegment<byte>> data)
            {
                // Initialize or GoAway frame.
                if (_socket.Endpoint.Protocol == Protocol.Ice1)
                {
                    Debug.Assert(expectedFrameType == data[0][8]);
                    return (ArraySegment<byte>.Empty, fin);
                }
                else
                {
                    Debug.Assert(expectedFrameType == data[0][0]);
                    (int size, int sizeLength) = data[0].Slice(1).AsReadOnlySpan().ReadSize20();
                    return (data[0].Slice(1 + sizeLength, size), fin);
                }
            }
            else
            {
                Debug.Assert(false);
                throw new InvalidDataException("unexpected frame");
            }
        }

        private protected override async ValueTask SendFrameAsync(
            OutgoingFrame frame,
            bool fin,
            CancellationToken cancel)
        {
            if (!IsStarted)
            {
                Debug.Assert(!IsIncoming);

                // If serialization is enabled on the adapter, we wait on the semaphore to ensure that no more than
                // one stream is active. The wait is done the client side to ensure the sent callback for the request
                // isn't called until the adapter is ready to dispatch a new request.
                if (IsBidirectional)
                {
                    if (_socket.BidirectionalSerializeSemaphore != null)
                    {
                        await _socket.BidirectionalSerializeSemaphore.WaitAsync(cancel).ConfigureAwait(false);
                    }
                }
                else if (_socket.UnidirectionalSerializeSemaphore != null)
                {
                    await _socket.UnidirectionalSerializeSemaphore.WaitAsync(cancel).ConfigureAwait(false);
                }

                // Ensure we allocate and queue the first stream frame atomically to ensure the receiver won't
                // receive stream frames with out-of-order stream IDs.
                ValueTask task;
                lock (_socket.Mutex)
                {
                    Id = _socket.AllocateId(IsBidirectional);
                    task = _socket.SendFrameAsync(Id, frame, fin, cancel);
                }
                await task.ConfigureAwait(false);
            }
            else
            {
                await _socket.SendFrameAsync(Id, frame, fin, cancel).ConfigureAwait(false);
            }

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame);
            }
        }
    }
}
