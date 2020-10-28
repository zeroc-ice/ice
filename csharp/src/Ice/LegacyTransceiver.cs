// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The legacy transceiver implements a multi-stream transport using the Ice1 protocol. A new incoming
    /// stream is created for each incoming Ice1 request and an outgoing stream is created for outgoing requests.
    /// The streams created by the legacy transceiver are always finished once the request or response frames are
    /// sent or received. Data streaming is not supported. Initialize or GoAway frames sent over the control streams
    /// are translated to connection validation or close connection Ice1 frames.</summary>
    internal class LegacyTransceiver : MultiStreamTransceiverWithUnderlyingTransceiver
    {
        internal AsyncSemaphore? BidirectionalSerializeSemaphore { get; }
        internal AsyncSemaphore? UnidirectionalSerializeSemaphore { get; }
        internal bool IsValidated { get; private set; }

        private readonly object _mutex = new ();
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;
        private long _nextPeerUnidirectionalId;
        private Task _sendTask = Task.CompletedTask;
        private readonly ITransceiver _transceiver;

        public override async ValueTask<TransceiverStream> AcceptStreamAsync(CancellationToken cancel)
        {
            while (true)
            {
                // Receive the Ice1 frame header.
                ArraySegment<byte> buffer;
                if (Endpoint.IsDatagram)
                {
                    buffer = await _transceiver.ReceiveDatagramAsync(cancel).ConfigureAwait(false);
                    if (buffer.Count < Ice1Definitions.HeaderSize)
                    {
                        ReceivedInvalidData($"received datagram with {buffer.Count} bytes");
                        continue;
                    }
                    Received(buffer.Count);
                }
                else
                {
                    buffer = new ArraySegment<byte>(new byte[256], 0, Ice1Definitions.HeaderSize);
                    await ReceiveAsync(buffer, cancel).ConfigureAwait(false);
                }

                // Check the header
                Ice1Definitions.CheckHeader(buffer.AsReadOnlySpan(0, Ice1Definitions.HeaderSize));
                int size = buffer.AsReadOnlySpan(10, 4).ReadInt();
                if (size < Ice1Definitions.HeaderSize)
                {
                    ReceivedInvalidData($"received ice1 frame with only {size} bytes");
                    continue;
                }
                if (size > IncomingFrameSizeMax)
                {
                    ReceivedInvalidData($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
                    continue;
                }

                // Read the remainder of the frame if needed.
                if (size > buffer.Count)
                {
                    if (Endpoint.IsDatagram)
                    {
                        ReceivedInvalidData($"maximum datagram size of {buffer.Count} exceeded");
                        continue;
                    }

                    if (size > buffer.Array!.Length)
                    {
                        // Allocate a new array and copy the header over.
                        var tmpBuffer = new ArraySegment<byte>(new byte[size], 0, size);
                        buffer.AsSpan().CopyTo(tmpBuffer.AsSpan(0, Ice1Definitions.HeaderSize));
                        buffer = tmpBuffer;
                    }
                    else
                    {
                        buffer = new ArraySegment<byte>(buffer.Array!, 0, size);
                    }
                    Debug.Assert(size == buffer.Count);

                    await ReceiveAsync(buffer.Slice(Ice1Definitions.HeaderSize), cancel).ConfigureAwait(false);
                }

                // Make sure the transceiver is marked as validated. This flag is necessary because incoming
                // connection initialization doesn't wait for connection validation message. So the connection
                // is considered validated on the server side only once the first frame is received. This is
                // only useful for connection warnings, to prevent a warning from showing up if the server side
                // connection is closed before the first message is received (which can occur with SSL for
                // example if the certification validation fails on the client side).
                IsValidated = true;

                // Parse the received frame and translate it into a stream ID, frame type and frame data. The returned
                // stream ID can be negative if the Ice1 frame is no longer supported (batch requests).
                (long streamId, Ice1Definitions.FrameType frameType, ArraySegment<byte> frame) = ParseFrame(buffer);
                if (streamId >= 0)
                {
                    if (TryGetStream(streamId, out LegacyStream? stream))
                    {
                        // If this is a known stream, pass the data to the stream.
                        if (frameType == Ice1Definitions.FrameType.ValidateConnection)
                        {
                            // Except for the validate connection frame, subsequent validate connection messages are
                            // heartbeats sent by the peer. We just handle it here and don't pass it over the control
                            // stream which only expect the close frame at this point.
                            Debug.Assert(stream.IsControl);
                            continue;
                        }
                        try
                        {
                            stream.ReceivedFrame(frameType, frame);
                        }
                        catch
                        {
                            // Ignore, the stream has been aborted
                        }
                    }
                    else if (frameType == Ice1Definitions.FrameType.Request)
                    {
                        // Create a new input stream for the request. If serialization is enabled, ensure we acquire
                        // the semaphore first to serialize the dispatching.
                        try
                        {
                            stream = new LegacyStream(streamId, this);
                            if (stream.IsBidirectional)
                            {
                                if (BidirectionalSerializeSemaphore != null)
                                {
                                    await BidirectionalSerializeSemaphore.WaitAsync(cancel).ConfigureAwait(false);
                                }
                            }
                            else if (UnidirectionalSerializeSemaphore != null)
                            {
                                await UnidirectionalSerializeSemaphore.WaitAsync(cancel).ConfigureAwait(false);
                            }
                            stream.ReceivedFrame(frameType, frame);
                            return stream;
                        }
                        catch
                        {
                            // Ignore, if the connection is being closed or the stream has been aborted.
                            stream?.Dispose();
                        }
                    }
                    else if (frameType == Ice1Definitions.FrameType.ValidateConnection)
                    {
                        // If we received a connection validation frame and the stream is not known, it's the first
                        // received connection validation message, create the control stream and return it.
                        stream = new LegacyStream(streamId, this);
                        Debug.Assert(stream.IsControl);
                        stream.ReceivedFrame(frameType, frame);
                        return stream;
                    }
                    else
                    {
                        // The stream has been disposed, ignore the data.
                    }
                }
            }

            void ReceivedInvalidData(string message)
            {
                // Invalid data on a datagram connection doesn't kill the connection. The bogus data could be sent by
                // a bogus or malicious peer. For non-datagram connections however, we raise an exception to abort the
                // connection.
                if (!Endpoint.IsDatagram)
                {
                    throw new InvalidDataException(message);
                }
                else if (Endpoint.Communicator.WarnDatagrams)
                {
                    Endpoint.Communicator.Logger.Warning(message);
                }
            }
        }

        public override ValueTask CloseAsync(Exception exception, CancellationToken cancel) =>
            _transceiver.CloseAsync(exception, cancel);

        public override TransceiverStream CreateStream(bool bidirectional)
        {
            lock (_mutex)
            {
                TransceiverStream stream;
                if (bidirectional)
                {
                    stream = new LegacyStream(_nextBidirectionalId, this);
                    _nextBidirectionalId += 4;
                }
                else
                {
                    stream = new LegacyStream(_nextUnidirectionalId, this);
                    _nextUnidirectionalId += 4;
                }
                return stream;
            }
        }

        public override ValueTask InitializeAsync(CancellationToken cancel) =>
            _transceiver.InitializeAsync(cancel);

        public override async Task PingAsync(CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();

            await SendFrameAsync(Ice1Definitions.ValidateConnectionFrame, CancellationToken.None).ConfigureAwait(false);

            if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(0,
                           ImmutableList<ArraySegment<byte>>.Empty,
                           (byte)Ice1Definitions.FrameType.ValidateConnection);
            }
        }

        internal LegacyTransceiver(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter)
            : base(endpoint, adapter, transceiver)
        {
            _transceiver = transceiver;

            // If serialization is enabled on the object adapter, create semaphore to limit the number of concurrent
            // dispatch per connection.
            if (adapter?.SerializeDispatch ?? false)
            {
                BidirectionalSerializeSemaphore = new AsyncSemaphore(1);
                UnidirectionalSerializeSemaphore = new AsyncSemaphore(1);
            }

            // We use the same stream ID numbering scheme as Quic
            if (IsIncoming)
            {
                _nextBidirectionalId = 1;
                _nextUnidirectionalId = 3;
                _nextPeerUnidirectionalId = 2;
            }
            else
            {
                _nextBidirectionalId = 0;
                _nextUnidirectionalId = 2;
                _nextPeerUnidirectionalId = 3;
            }
        }

        internal override ValueTask<TransceiverStream> ReceiveInitializeFrameAsync(CancellationToken cancel)
        {
            // With Ice1, the connection validation message is only sent by the server to the client. So here we
            // only expect the connection validation message for an outgoing connection and just return the
            // control stream immediately for an incoming connection.
            if (IsIncoming)
            {
                return new ValueTask<TransceiverStream>(new LegacyStream(2, this));
            }
            else
            {
                return base.ReceiveInitializeFrameAsync(cancel);
            }
        }

        internal async ValueTask SendFrameAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();

            // Synchronization is required here because this might be called concurrently by the connection code.
            Task task;
            lock (_mutex)
            {
                ValueTask sendTask = QueueAsync(buffer, cancel);
                if (sendTask.IsCompletedSuccessfully)
                {
                    _sendTask = Task.CompletedTask;
                    return;
                }
                _sendTask = task = sendTask.AsTask();
            }
            await task.ConfigureAwait(false);

            async ValueTask QueueAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
            {
                try
                {
                    // Wait for the previous send to complete
                    await _sendTask.ConfigureAwait(false);
                }
                catch (DatagramLimitException)
                {
                    // If the send failed because the datagram was too large, ignore and continue sending.
                }
                catch (OperationCanceledException)
                {
                    // Ignore if it got canceled.
                }

                // If the send got cancelled, throw to notify the connection of the cancellation. This isn't a fatal
                // connection error, the next pending frame will be sent.
                cancel.ThrowIfCancellationRequested();

                // Perform the sending.
                await SendAsync(buffer, cancel).ConfigureAwait(false);
            }
        }

        internal override ValueTask<TransceiverStream> SendInitializeFrameAsync(CancellationToken cancel)
        {
            // With Ice1, the connection validation message is only sent by the server to the client. So here
            // we only expect the connection validation message for an incoming connection and just return the
            // control stream immediately for an outgoing connection.
            if (IsIncoming)
            {
                return base.SendInitializeFrameAsync(cancel);
            }
            else
            {
                return new ValueTask<TransceiverStream>(CreateStream(false));
            }
        }

        private (long, Ice1Definitions.FrameType, ArraySegment<byte>) ParseFrame(ArraySegment<byte> readBuffer)
        {
            // The magic and version fields have already been checked.
            var frameType = (Ice1Definitions.FrameType)readBuffer[8];
            byte compressionStatus = readBuffer[9];
            if (compressionStatus == 2)
            {
                if (BZip2.IsLoaded)
                {
                    readBuffer = BZip2.Decompress(readBuffer, Ice1Definitions.HeaderSize, IncomingFrameSizeMax);
                }
                else
                {
                    throw new LoadException("compression not supported, bzip2 library not found");
                }
            }

            switch (frameType)
            {
                case Ice1Definitions.FrameType.CloseConnection:
                {
                    if (Endpoint.IsDatagram)
                    {
                        if (Endpoint.Communicator.WarnConnections)
                        {
                            Endpoint.Communicator.Logger.Warning(
                                $"ignoring close connection frame for datagram connection:\n{this}");
                        }
                    }
                    return (IsIncoming ? 2 : 3, frameType, default);
                }

                case Ice1Definitions.FrameType.Request:
                {
                    int requestId = readBuffer.AsReadOnlySpan(Ice1Definitions.HeaderSize, 4).ReadInt();

                    // Compute the stream ID out of the request ID. For one-way requests which use a null request ID,
                    // we generated a new stream ID using the _nextPeerUnidirectionalId counter.
                    long streamId;
                    if (requestId == 0)
                    {
                        streamId = _nextPeerUnidirectionalId += 4;
                    }
                    else
                    {
                        streamId = ((requestId - 1) << 2) + (IsIncoming ? 0 : 1);
                    }
                    return (streamId, frameType, readBuffer.Slice(Ice1Definitions.HeaderSize + 4));
                }

                case Ice1Definitions.FrameType.RequestBatch:
                {
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        TraceFrame(0,
                                   readBuffer.Slice(Ice1Definitions.HeaderSize),
                                   (byte)Ice1Definitions.FrameType.RequestBatch);
                    }
                    int invokeNum = readBuffer.AsReadOnlySpan(Ice1Definitions.HeaderSize, 4).ReadInt();
                    if (invokeNum < 0)
                    {
                        throw new InvalidDataException(
                            $"received ice1 RequestBatchMessage with {invokeNum} batch requests");
                    }
                    return (-1, frameType, default);
                }

                case Ice1Definitions.FrameType.Reply:
                {
                    int requestId = readBuffer.AsReadOnlySpan(Ice1Definitions.HeaderSize, 4).ReadInt();
                    long streamId = ((requestId - 1) << 2) + (IsIncoming ? 1 : 0);
                    return (streamId, frameType, readBuffer.Slice(Ice1Definitions.HeaderSize + 4));
                }

                case Ice1Definitions.FrameType.ValidateConnection:
                {
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        TraceFrame(0, ArraySegment<byte>.Empty, (byte)Ice1Definitions.FrameType.ValidateConnection);
                    }
                    // Notify the control stream of the reception of a Ping frame.
                    ReceivedPing();
                    return (IsIncoming ? 2 : 3, frameType, default);
                }

                default:
                {
                    throw new InvalidDataException($"received ice1 frame with unknown frame type `{frameType}'");
                }
            }
        }

        private async ValueTask ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel = default)
        {
            int offset = 0;
            while (offset != buffer.Count)
            {
                int received = await _transceiver.ReceiveAsync(buffer.Slice(offset), cancel).ConfigureAwait(false);
                offset += received;
                Received(received);
            }
        }

        private async ValueTask SendAsync(IList<ArraySegment<byte>> buffers, CancellationToken cancel = default)
        {
            int sent = await _transceiver.SendAsync(buffers, cancel).ConfigureAwait(false);
            Debug.Assert(sent == buffers.GetByteCount());
            Sent(sent);
        }
    }
}
