//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Sources;

namespace ZeroC.Ice
{
    internal class LegacyStream : Stream, IValueTaskSource<(Ice1Definitions.FrameType, ArraySegment<byte>)>
    {
        private static readonly List<ArraySegment<byte>> _closeConnectionFrame =
           new List<ArraySegment<byte>> { Ice1Definitions.CloseConnectionFrame };

        protected override ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;
        private int RequestId => IsBidirectional ? (int)(Id >> 2) + 1 : 0;
        private ManualResetValueTaskSourceCore<(Ice1Definitions.FrameType, ArraySegment<byte>)> _source;
        private readonly LegacyTransceiver _transceiver;

        public override void Abort(Exception ex)
        {
            // Cancel waiting ReceiveAsync
            try
            {
                _source.SetException(ex);
            }
            catch
            {
            }
        }

        protected override ValueTask<bool> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel) =>
            // This is never called because we override the default ReceiveFrameAsync implementation
            throw new NotImplementedException();
        protected override ValueTask ResetAsync() => new ValueTask();

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel) =>
            // This is never called because we override the default Async implementation
            throw new NotImplementedException();

        (Ice1Definitions.FrameType, ArraySegment<byte>) IValueTaskSource<(Ice1Definitions.FrameType, ArraySegment<byte>)>.GetResult(
            short token)
        {
            Debug.Assert(token == _source.Version);
            try
            {
                return _source.GetResult(token);
            }
            finally
            {
                _source.Reset();
            }
        }

        ValueTaskSourceStatus IValueTaskSource<(Ice1Definitions.FrameType, ArraySegment<byte>)>.GetStatus(
            short token) => _source.GetStatus(token);

        void IValueTaskSource<(Ice1Definitions.FrameType, ArraySegment<byte>)>.OnCompleted(
            Action<object?> continuation,
            object? state,
            short token,
            ValueTaskSourceOnCompletedFlags flags) => _source.OnCompleted(continuation, state, token, flags);

        internal LegacyStream(long streamId, LegacyTransceiver transceiver) : base(streamId, transceiver) =>
            _transceiver = transceiver;

        internal void ReceivedFrame(Ice1Definitions.FrameType frameType, ArraySegment<byte> frame) =>
            _source.SetResult((frameType, frame));

        internal override async ValueTask<(long, string message)> ReceiveCloseFrameAsync()
        {
            // Wait to be signaled for the reception of a new frame for this stream
            (ArraySegment<byte> frame, bool _) = await ReceiveFrameAsync((byte)Ice1Definitions.FrameType.CloseConnection,
                                                                         CancellationToken.None).ConfigureAwait(false);

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                ProtocolTrace.TraceFrame(_transceiver.Endpoint,
                                         Id,
                                         frame,
                                         (byte)Ice1Definitions.FrameType.CloseConnection);
            }

            return (0, "connection gracefully closed by peer");
        }

        internal override ValueTask SendCloseFrameAsync(long streamId, string reason, CancellationToken cancel)
        {
            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                ProtocolTrace.TraceFrame(_transceiver.Endpoint,
                                         Id,
                                         _closeConnectionFrame,
                                         (byte)Ice1Definitions.FrameType.CloseConnection);
            }
            return _transceiver.SendFrameAsync(_closeConnectionFrame, cancel);
        }

        private protected override async ValueTask<(ArraySegment<byte>, bool)> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            // Wait to be signaled for the reception of a new frame for this stream
            (Ice1Definitions.FrameType frameType, ArraySegment<byte> frame) =
                await new ValueTask<(Ice1Definitions.FrameType, ArraySegment<byte>)>(
                    this,
                    _source.Version).ConfigureAwait(false);
            if ((byte)frameType != expectedFrameType)
            {
                throw new InvalidDataException($"received frame type {frameType} but expected {expectedFrameType}");
            }
            return (frame, true);
        }

        private protected override ValueTask SendFrameAsync(OutgoingFrame frame, bool fin, CancellationToken cancel)
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

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                ProtocolTrace.TraceFrame(_transceiver.Endpoint, Id, frame, 0, compressionStatus);
            }
            return _transceiver.SendFrameAsync(buffer, CancellationToken.None);
        }
    }

    internal class LegacyTransceiver : MultiStreamTransceiverWithUnderlyingTransceiver
    {
        private static readonly List<ArraySegment<byte>> _validateConnectionFrame =
            new List<ArraySegment<byte>> { Ice1Definitions.ValidateConnectionFrame };

        private readonly object _mutex = new object();
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;
        private long _nextPeerUnidirectionalId;
        private Task _sendTask = Task.CompletedTask;
        private readonly ITransceiver _transceiver;

        public override async ValueTask<Stream> AcceptStreamAsync(CancellationToken cancel)
        {
            while (true)
            {
                ArraySegment<byte> buffer = await PerformReceiveFrameAsync().ConfigureAwait(false);
                if (buffer.Count > 0) // Can be empty if invalid datagram.
                {
                    (long streamId, Ice1Definitions.FrameType frameType, ArraySegment<byte> frame) = ParseFrame(buffer);
                    if (streamId >= 0)
                    {
                        if (frameType == Ice1Definitions.FrameType.Request)
                        {
                            var stream = new LegacyStream(streamId, this);
                            stream.ReceivedFrame(frameType, frame);
                            return stream;
                        }
                        else if (TryGetStream(streamId, out LegacyStream? stream))
                        {
                            stream.ReceivedFrame(frameType, frame);
                        }
                        else
                        {
                            throw new InvalidDataException("received unexpected frame");
                        }
                    }
                }
            }
        }

        public override ValueTask CloseAsync(Exception exception, CancellationToken cancel) =>
            _transceiver.CloseAsync(exception, cancel);

        public override Stream CreateStream(bool bidirectional)
        {
            lock (_mutex)
            {
                Stream stream;
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

        public override async ValueTask InitializeAsync(CancellationToken cancel)
        {
            // Initialize the transport
            await _transceiver.InitializeAsync(cancel).ConfigureAwait(false);

            if (!Endpoint.IsDatagram) // Datagram connections are always implicitly validated.
            {
                if (IsIncoming) // The server side has the active role for connection validation.
                {
                    await SendAsync(_validateConnectionFrame, cancel).ConfigureAwait(false);
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        TraceSendProtocolFrame(Ice1Definitions.FrameType.ValidateConnection);
                    }
                }
                else // The client side has the passive role for connection validation.
                {
                    var readBuffer = new ArraySegment<byte>(new byte[Ice1Definitions.HeaderSize]);
                    await ReceiveAsync(readBuffer, cancel).ConfigureAwait(false);

                    Ice1Definitions.CheckHeader(readBuffer);
                    var frameType = (Ice1Definitions.FrameType)readBuffer[8];
                    if (frameType != Ice1Definitions.FrameType.ValidateConnection)
                    {
                        throw new InvalidDataException(@$"received ice1 frame with frame type `{frameType
                            }' before receiving the validate connection frame");
                    }

                    int size = readBuffer.AsReadOnlySpan(10, 4).ReadInt();
                    if (size != Ice1Definitions.HeaderSize)
                    {
                        throw new InvalidDataException(
                            @$"received an ice1 frame with validate connection type and a size of `{size
                            }' bytes");
                    }

                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        TraceReceivedProtocolFrame(Ice1Definitions.FrameType.ValidateConnection);
                    }
                }
            }
        }

        public override async ValueTask PingAsync(CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();
            if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceSendProtocolFrame(Ice1Definitions.FrameType.ValidateConnection);
            }
            await SendFrameAsync(_validateConnectionFrame, CancellationToken.None).ConfigureAwait(false);
        }

        public override string ToString() => _transceiver.ToString()!;

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _transceiver.Dispose();
            }
        }

        internal LegacyTransceiver(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter) :
            base(endpoint, adapter, transceiver)
        {
            _transceiver = transceiver;

            // We use the same stream ID numbering scheme as Quic
            if (IsIncoming)
            {
                _nextBidirectionalId = 1;
                _nextUnidirectionalId = 7;
                _nextPeerUnidirectionalId = 2;
            }
            else
            {
                _nextBidirectionalId = 0;
                _nextUnidirectionalId = 6;
                _nextPeerUnidirectionalId = 3;
            }
        }

        internal override ValueTask<Stream> ReceiveInitializeFrameAsync(CancellationToken cancel) =>
            // Ice1 doesn't have initialize frames
            new ValueTask<Stream>(new LegacyStream(IsIncoming ? 2 : 3, this));

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

        internal override ValueTask<Stream> SendInitializeFrameAsync(CancellationToken cancel) =>
            // Ice1 doesn't have initialize frames
            new ValueTask<Stream>(new LegacyStream(IsIncoming ? 3 : 2, this));

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
                    long streamId = requestId == 0 ? (_nextPeerUnidirectionalId += 4) : (requestId - 1 ) << 2;
                    return (streamId, frameType, readBuffer.Slice(Ice1Definitions.HeaderSize + 4));
                }

                case Ice1Definitions.FrameType.RequestBatch:
                {
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        TraceReceivedProtocolFrame(frameType);
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
                    long streamId = (requestId - 1) << 2;
                    return (streamId, frameType, readBuffer.Slice(Ice1Definitions.HeaderSize + 4));
                }

                case Ice1Definitions.FrameType.ValidateConnection:
                {
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        TraceReceivedProtocolFrame(Ice1Definitions.FrameType.ValidateConnection);
                    }
                    PingReceived();
                    return (-1, frameType, default);
                }

                default:
                {
                    throw new InvalidDataException($"received ice1 frame with unknown frame type `{frameType}'");
                }
            }
        }

        private async ValueTask<ArraySegment<byte>> PerformReceiveFrameAsync()
        {
            // Read header
            ArraySegment<byte> readBuffer;
            if (Endpoint.IsDatagram)
            {
                readBuffer = await _transceiver.ReceiveDatagramAsync(default).ConfigureAwait(false);
                if (readBuffer.Count == 0)
                {
                    // The transport failed to read a datagram which was too big or it received an empty datagram.
                    return readBuffer;
                }
                Received(readBuffer.Count);
            }
            else
            {
                readBuffer = new ArraySegment<byte>(new byte[256], 0, Ice1Definitions.HeaderSize);
                await ReceiveAsync(readBuffer).ConfigureAwait(false);
            }

            // Check header
            Ice1Definitions.CheckHeader(readBuffer);
            int size = readBuffer.AsReadOnlySpan(10, 4).ReadInt();
            if (size < Ice1Definitions.HeaderSize)
            {
                throw new InvalidDataException($"received ice1 frame with only {size} bytes");
            }

            if (size > IncomingFrameSizeMax)
            {
                throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
            }

            // Read the remainder of the frame if needed
            if (size > readBuffer.Count)
            {
                if (!Endpoint.IsDatagram)
                {
                    if (size > readBuffer.Array!.Length)
                    {
                        // Allocate a new array and copy the header over
                        var buffer = new ArraySegment<byte>(new byte[size], 0, size);
                        readBuffer.AsSpan().CopyTo(buffer.AsSpan(0, Ice1Definitions.HeaderSize));
                        readBuffer = buffer;
                    }
                    else
                    {
                        readBuffer = new ArraySegment<byte>(readBuffer.Array!, 0, size);
                    }
                    Debug.Assert(size == readBuffer.Count);

                    await ReceiveAsync(readBuffer.Slice(Ice1Definitions.HeaderSize)).ConfigureAwait(false);
                }
                else
                {
                    if (Endpoint.Communicator.WarnDatagrams)
                    {
                        Endpoint.Communicator.Logger.Warning($"maximum datagram size of {readBuffer.Count} exceeded");
                    }
                    return default;
                }
            }
            return readBuffer;
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

        private void TraceSendProtocolFrame(Ice1Definitions.FrameType type) => TraceFrame("sending ", type);

        private void TraceReceivedProtocolFrame(Ice1Definitions.FrameType type) => TraceFrame("received ", type);

        private void TraceFrame(string prefix, Ice1Definitions.FrameType type)
        {
            string? frameType = type switch
            {
                Ice1Definitions.FrameType.RequestBatch => "batch request",
                Ice1Definitions.FrameType.ValidateConnection => "validate connection",
                Ice1Definitions.FrameType.CloseConnection => "close connection",
                _ => null,
            };
            Debug.Assert(frameType != null);

            // Validation and close connection frames are Ice1 protocol frame so we continue to trace them with
            // the protocol tracing.
            var s = new StringBuilder();
            s.Append(prefix);
            s.Append(frameType);
            s.Append("\nprotocol = ");
            s.Append(Endpoint.Protocol.GetName());
            Endpoint.Communicator.Logger.Trace(Endpoint.Communicator.TraceLevels.ProtocolCategory, s.ToString());
        }
    }
}
