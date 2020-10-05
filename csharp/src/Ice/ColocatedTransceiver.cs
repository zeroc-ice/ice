//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;
using System.Threading.Channels;

namespace ZeroC.Ice
{
    internal class ColocatedStream : SignaledTransceiverStream<(object, bool)>
    {
        protected override ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;
        private readonly ColocatedTransceiver _transceiver;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);

            if (disposing && !IsIncoming && IsStarted)
            {
                if (IsBidirectional)
                {
                    _transceiver.BidirectionalSerializeSemaphore?.Release();
                }
                else if (!IsControl)
                {
                    _transceiver.UnidirectionalSerializeSemaphore?.Release();
                }
            }
        }

        protected override ValueTask<bool> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel) =>
            // This is never called because we override the default ReceiveFrameAsync implementation
            throw new NotImplementedException();

        protected override ValueTask ResetAsync() =>
            // A null frame indicates a stream reset.
            _transceiver.SendFrameAsync(Id, null, true, CancellationToken.None);

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel)
        {
            if (!IsStarted)
            {
                Id = _transceiver.AllocateId(false);
            }
            // This is only called for initialize/close frame on the control streams. Requests or responses are handled
            // by the SendFrameAsync method below.
            Debug.Assert(IsControl && !IsBidirectional);
            return _transceiver.SendFrameAsync(Id, buffer, fin, cancel);
        }

        internal ColocatedStream(long streamId, ColocatedTransceiver transceiver) : base(streamId, transceiver) =>
            _transceiver = transceiver;

        internal ColocatedStream(bool bidirectional, ColocatedTransceiver transceiver) :
            base(bidirectional, transceiver) => _transceiver = transceiver;

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
                if (_transceiver.Endpoint.Protocol == Protocol.Ice1)
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
                // stream is being dispatch. The wait is done the client side to ensure the sent callback for the
                // request isn't called until the adapter is ready to dispatch a new request.
                if (IsBidirectional)
                {
                    if (_transceiver.BidirectionalSerializeSemaphore != null)
                    {
                        await _transceiver.BidirectionalSerializeSemaphore.WaitAsync(cancel).ConfigureAwait(false);
                    }
                }
                else if (_transceiver.UnidirectionalSerializeSemaphore != null)
                {
                    await _transceiver.UnidirectionalSerializeSemaphore.WaitAsync(cancel).ConfigureAwait(false);
                }

                // Ensure we allocate and queue the first stream frame atomically to ensure the receiver won't
                // receive stream frames with out-of-order stream IDs.
                ValueTask task;
                lock (_transceiver.Mutex)
                {
                    Id = _transceiver.AllocateId(IsBidirectional);
                    task = _transceiver.SendFrameAsync(Id, frame, fin, cancel);
                }
                await task.ConfigureAwait(false);
            }
            else
            {
                await _transceiver.SendFrameAsync(Id, frame, fin, cancel).ConfigureAwait(false);
            }

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame);
            }
        }
    }

    internal class ColocatedTransceiver : MultiStreamTransceiver
    {
        internal AsyncSemaphore? BidirectionalSerializeSemaphore { get; }
        internal AsyncSemaphore? UnidirectionalSerializeSemaphore { get; }
        internal readonly object Mutex = new object();

        private readonly long _id;
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;
        private readonly ChannelReader<(long, object?, bool)> _reader;
        private readonly ChannelWriter<(long, object?, bool)> _writer;

        public override void Abort() => _writer.TryComplete();

        public override async ValueTask<TransceiverStream> AcceptStreamAsync(CancellationToken cancel)
        {
            while (true)
            {
                try
                {
                    (long streamId, object? frame, bool fin) = await _reader.ReadAsync(cancel).ConfigureAwait(false);
                    if (TryGetStream(streamId, out ColocatedStream? stream))
                    {
                        if (frame == null)
                        {
                            stream.ReceivedReset();
                        }
                        else
                        {
                            stream.ReceivedFrame(frame, fin);
                        }
                    }
                    else if (frame is OutgoingRequestFrame || streamId == (IsIncoming ? 2 : 3))
                    {
                        Debug.Assert(frame != null);
                        stream = new ColocatedStream(streamId, this);
                        stream.ReceivedFrame(frame, fin);
                        return stream;
                    }
                    else
                    {
                        // Canceled request, ignore
                    }
                }
                catch (ChannelClosedException exception)
                {
                    throw new ConnectionLostException(exception);
                }
            }
        }

        public override async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            _writer.Complete();
            await _reader.Completion.ConfigureAwait(false);
        }

        public override TransceiverStream CreateStream(bool bidirectional) => new ColocatedStream(bidirectional, this);

        public override ValueTask InitializeAsync(CancellationToken cancel) => default;

        public override Task PingAsync(CancellationToken cancel) => Task.CompletedTask;

        public override string ToString() =>
            $"colocated ID = {_id}\nobject adapter = {((ColocatedEndpoint)Endpoint).Adapter.Name}\nincoming = {IsIncoming}";

        internal ColocatedTransceiver(
            ColocatedEndpoint endpoint,
            long id,
            ChannelWriter<(long, object?, bool)> writer,
            ChannelReader<(long, object?, bool)> reader,
            bool isIncoming) : base(endpoint, isIncoming ? endpoint.Adapter : null)
        {
            if (!isIncoming && endpoint.Adapter.SerializeDispatch)
            {
                BidirectionalSerializeSemaphore = new AsyncSemaphore(1);
                UnidirectionalSerializeSemaphore = new AsyncSemaphore(1);
            }

            // We use the same stream ID numbering scheme as Quic
            if (IsIncoming)
            {
                _nextBidirectionalId = 1;
                _nextUnidirectionalId = 3;
            }
            else
            {
                _nextBidirectionalId = 0;
                _nextUnidirectionalId = 2;
            }
            _id = id;
            _writer = writer;
            _reader = reader;
        }

        internal long AllocateId(bool bidirectional)
        {
            lock (Mutex)
            {
                long id;
                if (bidirectional)
                {
                    id = _nextBidirectionalId;
                    _nextBidirectionalId += 4;
                }
                else
                {
                    id = _nextUnidirectionalId;
                    _nextUnidirectionalId += 4;
                }
                return id;
            }
        }

        internal ValueTask SendFrameAsync(long streamId, object? frame, bool fin, CancellationToken cancel) =>
            _writer.WriteAsync((streamId, frame, fin), cancel);
    }
}
