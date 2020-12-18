// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The MultiStreamSocket class for the colocated transport.</summary>
    internal class ColocatedSocket : MultiStreamSocket
    {
        public override TimeSpan IdleTimeout
        {
            get => Timeout.InfiniteTimeSpan;
            internal set => throw new NotSupportedException("IdleTimeout is not supported with colocated connections");
        }

        private readonly AsyncSemaphore? _bidirectionalSerializeSemaphore;
        private readonly long _id;
        private readonly object _mutex = new();
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;
        private AsyncSemaphore? _peerUnidirectionalSerializeSemaphore;
        private readonly ChannelReader<(long, object?, bool)> _reader;
        private readonly AsyncSemaphore? _unidirectionalSerializeSemaphore;
        private readonly ChannelWriter<(long, object?, bool)> _writer;

        public override void Abort() => _writer.TryComplete();

        public override async ValueTask<SocketStream> AcceptStreamAsync(CancellationToken cancel)
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
                    else if (frame is IncomingRequestFrame || streamId == (IsIncoming ? 2 : 3))
                    {
                        // If we received an incoming request frame or a frame for the incoming control stream,
                        // create a new stream and provide it the received frame.
                        Debug.Assert(frame != null);
                        try
                        {
                            stream = new ColocatedStream(this, streamId);
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

        public override SocketStream CreateStream(bool bidirectional, bool control) =>
            new ColocatedStream(this, bidirectional, control);

        public async override ValueTask InitializeAsync(CancellationToken cancel)
        {
            // Send our unidirectional semaphore to the peer. The peer will decrease the semaphore when the stream is
            // disposed.
            await _writer.WriteAsync((-1, _unidirectionalSerializeSemaphore, false), cancel).ConfigureAwait(false);
            (_, object? semaphore, _) = await _reader.ReadAsync(cancel).ConfigureAwait(false);

            // Get the peer's unidirectional semaphore and keep track of it to be able to release it once an
            // unidirectional stream is disposed.
            _peerUnidirectionalSerializeSemaphore = (AsyncSemaphore?)semaphore;
        }

        public override Task PingAsync(CancellationToken cancel) => Task.CompletedTask;

        public override string ToString() =>
            $"colocated ID = {_id}\nobject adapter = {((ColocatedEndpoint)Endpoint).Adapter.Name}\nincoming = {IsIncoming}";

        internal ColocatedSocket(
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
                _bidirectionalSerializeSemaphore = new AsyncSemaphore(1);
                _unidirectionalSerializeSemaphore = new AsyncSemaphore(1);
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

        internal override (long, long) AbortStreams(Exception exception, Func<SocketStream, bool>? predicate)
        {
            (long, long) streamIds = base.AbortStreams(exception, predicate);

            // Unblock requests waiting on the semaphores.
            _bidirectionalSerializeSemaphore?.CancelAwaiters(exception);
            _unidirectionalSerializeSemaphore?.CancelAwaiters(exception);

            return streamIds;
        }

        internal void ReleaseFlowControlCredit(ColocatedStream stream)
        {
            if (stream.IsIncoming && !stream.IsBidirectional && !stream.IsControl)
            {
                // This client side stream acquires the semaphore before opening an unidirectional stream. The
                // semaphore is released when this server side stream is disposed.
                _peerUnidirectionalSerializeSemaphore?.Release();
            }
            else if (!stream.IsIncoming && stream.IsBidirectional && stream.IsStarted)
            {
                // This client side stream acquires the semaphore before opening a bidirectional stream. The
                // semaphore is released when this client side stream is disposed.
                _bidirectionalSerializeSemaphore?.Release();
            }
        }

        internal async ValueTask SendFrameAsync(
            ColocatedStream stream,
            object? frame,
            bool fin,
            CancellationToken cancel)
        {
            try
            {
                if (stream.IsStarted)
                {
                    await _writer.WriteAsync((stream.Id, frame, fin), cancel).ConfigureAwait(false);
                }
                else
                {
                    Debug.Assert(!stream.IsIncoming);

                    if (!stream.IsControl)
                    {
                        // If serialization is enabled on the adapter, we wait on the semaphore to ensure that no more
                        // than one stream is active. The wait is done on the client side to ensure the sent callback
                        // for the request isn't called until the adapter is ready to dispatch a new request.
                        AsyncSemaphore? semaphore = stream.IsBidirectional ?
                            _bidirectionalSerializeSemaphore : _unidirectionalSerializeSemaphore;
                        if (semaphore != null)
                        {
                            await semaphore.WaitAsync(cancel).ConfigureAwait(false);
                        }
                    }

                    // Ensure we allocate and queue the first stream frame atomically to ensure the receiver won't
                    // receive stream frames with out-of-order stream IDs.
                    ValueTask task;
                    lock (_mutex)
                    {
                        // Allocate a new ID according to the Quic numbering scheme.
                        if (stream.IsBidirectional)
                        {
                            stream.Id = _nextBidirectionalId;
                            _nextBidirectionalId += 4;
                        }
                        else
                        {
                            stream.Id = _nextUnidirectionalId;
                            _nextUnidirectionalId += 4;
                        }

                        // The write is not cancelable here, we want to make sure that at this point the peer
                        // receives the frame in order for the serialization semaphore to be released (otherwise,
                        // the peer would receive a reset for a stream for which it never received a frame).
                        task = _writer.WriteAsync((stream.Id, frame, fin), cancel);
                    }
                    await task.ConfigureAwait(false);
                }
            }
            catch (OperationCanceledException)
            {
                throw;
            }
            catch (Exception ex)
            {
                throw new TransportException(ex, RetryPolicy.AfterDelay(TimeSpan.Zero));
            }
        }
    }
}
