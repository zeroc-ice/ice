// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The MultiStreamTransceiver class for the colocated transport.</summary>
    internal class ColocatedTransceiver : MultiStreamTransceiver
    {
        internal AsyncSemaphore? BidirectionalSerializeSemaphore { get; }
        internal readonly object Mutex = new ();
        internal AsyncSemaphore? PeerUnidirectionalSerializeSemaphore { get; private set; }
        internal AsyncSemaphore? UnidirectionalSerializeSemaphore { get; }

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
                        try
                        {
                            if (frame == null)
                            {
                                stream.ReceivedReset(0);
                            }
                            else
                            {
                                stream.ReceivedFrame(frame, fin);
                            }
                        }
                        catch
                        {
                            // Ignore the stream has been aborted.
                        }
                    }
                    else if (frame is OutgoingRequestFrame || streamId == (IsIncoming ? 2 : 3))
                    {
                        // If we received an outgoing request frame or a frame for the incoming control stream,
                        // create a new stream and provide it the received frame.
                        Debug.Assert(frame != null);
                        try
                        {
                            stream = new ColocatedStream(streamId, this);
                            stream.ReceivedFrame(frame, fin);
                            return stream;
                        }
                        catch
                        {
                            // Ignore, the connection is being closed or the stream got aborted.
                            stream?.Dispose();
                        }
                    }
                    else
                    {
                        // Canceled request, ignore
                    }
                }
                catch (ChannelClosedException exception)
                {
                    throw new ConnectionLostException(exception, RetryPolicy.AfterDelay(TimeSpan.Zero));
                }
            }
        }

        public override async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            _writer.Complete();
            await _reader.Completion.ConfigureAwait(false);
        }

        public override TransceiverStream CreateStream(bool bidirectional) => new ColocatedStream(bidirectional, this);

        public async override ValueTask InitializeAsync(CancellationToken cancel)
        {
            // Send our unidirectional semaphore to the peer. The peer will decrease the semaphore when the stream is
            // disposed.
            await _writer.WriteAsync((-1, UnidirectionalSerializeSemaphore, false), cancel).ConfigureAwait(false);
            (_, object? semaphore, _) = await _reader.ReadAsync(cancel).ConfigureAwait(false);

            // Get the peer's unidirectional semaphore and keep track of it to be able to release it once a
            // unidirectional stream is disposed.
            PeerUnidirectionalSerializeSemaphore = (AsyncSemaphore?)semaphore;
        }

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

            if (endpoint.Adapter.SerializeDispatch)
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

        internal override (long, long) AbortStreams(Exception exception, Func<TransceiverStream, bool>? predicate)
        {
            (long, long) streamIds = base.AbortStreams(exception, predicate);

            // Unblock requests waiting on the semaphores.
            BidirectionalSerializeSemaphore?.CancelAwaiters(exception);
            UnidirectionalSerializeSemaphore?.CancelAwaiters(exception);

            return streamIds;
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
