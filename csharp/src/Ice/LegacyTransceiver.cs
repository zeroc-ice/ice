// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal class LegacyStream : SignaledTransceiverStream<(Ice1Definitions.FrameType, ArraySegment<byte>)>
    {
        protected override ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;
        private int RequestId => IsBidirectional ? (int)(Id >> 2) + 1 : 0;
        private readonly LegacyTransceiver _transceiver;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing && IsIncoming)
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

        protected override ValueTask ResetAsync() => new ValueTask();

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel) =>
            _transceiver.SendFrameAsync(buffer, cancel);

        internal LegacyStream(long streamId, LegacyTransceiver transceiver)
            : base(streamId, transceiver) => _transceiver = transceiver;

        internal void ReceivedFrame(Ice1Definitions.FrameType frameType, ArraySegment<byte> frame) =>
            // If we received a response, we make sure to run the continuation asynchronously since this might end
            // up calling user code and could therefore prevent receiving further data since AcceptStreamAsync
            // would be blocked calling user code through this method.
            SignalCompletion((frameType, frame), frameType == Ice1Definitions.FrameType.Reply);

        private protected override async ValueTask<(ArraySegment<byte>, bool)> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            // Wait to be signaled for the reception of a new frame for this stream
            (Ice1Definitions.FrameType frameType, ArraySegment<byte> frame) =
                await WaitSignalAsync(cancel).ConfigureAwait(false);
            if ((byte)frameType != expectedFrameType)
            {
                throw new InvalidDataException($"received frame type {frameType} but expected {expectedFrameType}");
            }
            // fin = true unless it's the validation connection frame.
            return (frame, frameType != Ice1Definitions.FrameType.ValidateConnection);
        }

        private protected override async ValueTask SendFrameAsync(
            OutgoingFrame frame,
            bool fin,
            CancellationToken cancel)
        {
            var buffer = new List<ArraySegment<byte>>(frame.Data.Count + 1);
            byte[] headerData = new byte[Ice1Definitions.HeaderSize + 4];
            if (frame is OutgoingRequestFrame)
            {
                Ice1Definitions.RequestHeader.CopyTo(headerData.AsSpan());
            }
            else
            {
                Ice1Definitions.ReplyHeader.CopyTo(headerData.AsSpan());
            }
            int size = frame.Size + Ice1Definitions.HeaderSize + 4;
            headerData.AsSpan(10, 4).WriteInt(size);
            headerData.AsSpan(Ice1Definitions.HeaderSize).WriteInt(RequestId);
            buffer.Add(headerData);
            buffer.AddRange(frame.Data);

            byte compressionStatus = 0;
            if (BZip2.IsLoaded && frame.Compress)
            {
                List<ArraySegment<byte>>? compressed = null;
                if (size >= _transceiver.Endpoint.Communicator.CompressionMinSize)
                {
                    compressed = BZip2.Compress(buffer,
                                                size,
                                                Ice1Definitions.HeaderSize,
                                                _transceiver.Endpoint.Communicator.CompressionLevel);
                }

                ArraySegment<byte> header;
                if (compressed != null)
                {
                    buffer = compressed;
                    header = buffer[0];
                    size = buffer.GetByteCount();
                }
                else // Message not compressed, request compressed response, if any.
                {
                    header = buffer[0];
                    header[9] = 1; // Write the compression status
                }
                compressionStatus = header[9];
            }

            // Ensure the frame isn't bigger than what we can send with the transport.
            _transceiver.Underlying.CheckSendSize(size);

            await _transceiver.SendFrameAsync(buffer, CancellationToken.None).ConfigureAwait(false);

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame, 0, compressionStatus);
            }
        }
    }

    internal class LegacyTransceiver : MultiStreamTransceiverWithUnderlyingTransceiver
    {
        internal AsyncSemaphore? BidirectionalSerializeSemaphore { get; }
        internal AsyncSemaphore? UnidirectionalSerializeSemaphore { get; }
        private readonly object _mutex = new object();
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;
        private long _nextPeerUnidirectionalId;
        private Task _sendTask = Task.CompletedTask;
        private readonly ITransceiver _transceiver;

        public override async ValueTask<TransceiverStream> AcceptStreamAsync(CancellationToken cancel)
        {
            while (true)
            {
                // Read header
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

                // Check header
                Ice1Definitions.CheckHeader(buffer.Slice(0, Ice1Definitions.HeaderSize));
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

                // Read the remainder of the frame if needed
                if (size > buffer.Count)
                {
                    if (Endpoint.IsDatagram)
                    {
                        ReceivedInvalidData($"maximum datagram size of {buffer.Count} exceeded");
                        continue;
                    }

                    if (size > buffer.Array!.Length)
                    {
                        // Allocate a new array and copy the header over
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

                (long streamId, Ice1Definitions.FrameType frameType, ArraySegment<byte> frame) = ParseFrame(buffer);
                if (streamId >= 0)
                {
                    if (TryGetStream(streamId, out LegacyStream? stream))
                    {
                        if (frameType == Ice1Definitions.FrameType.ValidateConnection)
                        {
                            // Except for the validate connection frame, subsequent validate connection messages are
                            // heartbeats sent by the peer. We just handle it here and don't pass it over the control
                            // stream which only expect the close frame at this point.
                            Debug.Assert(stream.IsControl);
                            continue;
                        }
                        stream.ReceivedFrame(frameType, frame);
                    }
                    else if (frameType == Ice1Definitions.FrameType.Request)
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
                    else if (frameType == Ice1Definitions.FrameType.ValidateConnection)
                    {
                        stream = new LegacyStream(streamId, this);
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

        public override ValueTask InitializeAsync(CancellationToken cancel) => _transceiver.InitializeAsync(cancel);

        public override async Task PingAsync(CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();

            await SendFrameAsync(Ice1Definitions.ValidateConnectionFrame, CancellationToken.None).ConfigureAwait(false);

            if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(0, new List<ArraySegment<byte>>(), (byte)Ice1Definitions.FrameType.ValidateConnection);
            }
        }

        internal LegacyTransceiver(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter)
            : base(endpoint, adapter, transceiver)
        {
            _transceiver = transceiver;
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

            // Synchronization is required here because this might be called concurrently by the connection code
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
                    long streamId;
                    if (requestId == 0)
                    {
                        // Assign an ID for incoming oneway requests.
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
                    Debug.Assert(false); // TODO: deal with batch requests
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
