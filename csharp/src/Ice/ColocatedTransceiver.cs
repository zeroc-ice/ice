// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Channels;

namespace ZeroC.Ice
{
    /// <summary>The MultiStreamTransceiver class for the colocated transport.</summary>
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
                        // If we received a frame for a known stream, signal the stream of the frame reception. A null
                        // frame indicates a stream reset so reset the stream in this case.
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
                        // If we received an outgoing request frame or a frame for the incoming control stream,
                        // create a new stream and provide it the received frame.
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
            bool isIncoming)
            : base(endpoint, isIncoming ? endpoint.Adapter : null)
        {
            _id = id;
            _writer = writer;
            _reader = reader;

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
