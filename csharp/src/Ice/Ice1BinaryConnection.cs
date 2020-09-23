// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal class Ice1BinaryConnection : BinaryConnection
    {
        private static readonly List<ArraySegment<byte>> _closeConnectionFrame =
            new List<ArraySegment<byte>> { Ice1Definitions.CloseConnectionFrame };

        private static readonly List<ArraySegment<byte>> _validateConnectionFrame =
            new List<ArraySegment<byte>> { Ice1Definitions.ValidateConnectionFrame };

        // The mutex provides thread-safety for the _sendTask data member.
        private readonly object _mutex = new object();
        private int _nextRequestId;
        private Task _sendTask = Task.CompletedTask;
        private readonly ITransceiver _transceiver;

        public override ValueTask DisposeAsync()
        {
            Closed();
            return _transceiver.DisposeAsync();
        }

        public override string ToString() => _transceiver.ToString()!;

        internal Ice1BinaryConnection(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter)
            : base(endpoint, adapter) => _transceiver = transceiver;

        internal override async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            // Write the close connection frame.
            await SendFrameAsync(0, _closeConnectionFrame, cancel).ConfigureAwait(false);

            // Notify the transport of the graceful connection closure.
            await _transceiver.CloseAsync(exception, cancel).ConfigureAwait(false);
        }

        internal override async ValueTask HeartbeatAsync(CancellationToken cancel) =>
            await SendFrameAsync(0, _validateConnectionFrame, cancel).ConfigureAwait(false);

        internal override async ValueTask InitializeAsync(Action heartbeatCallback, CancellationToken cancel)
        {
            HeartbeatCallback = heartbeatCallback;

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

                    Ice1Definitions.CheckHeader(readBuffer.AsSpan(0, 8));
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

            Initialized();
        }

        internal override long NewStream(bool bidirectional)
        {
            if (bidirectional)
            {
                int streamId = ++_nextRequestId;
                if (streamId <= 0)
                {
                    _nextRequestId = 0;
                    streamId = ++_nextRequestId;
                }
                return streamId;
            }
            else
            {
                return 0;
            }
        }

        internal override async ValueTask<(long StreamId, IncomingFrame? Frame, bool Fin)> ReceiveAsync(
            CancellationToken cancel)
        {
            while (true)
            {
                ArraySegment<byte> buffer = await PerformReceiveFrameAsync().ConfigureAwait(false);
                if (buffer.Count > 0) // Can be empty if invalid datagram.
                {
                    (int requestId, IncomingFrame? frame) = ParseFrame(buffer);
                    if (frame != null)
                    {
                        return (StreamId: requestId, Frame: frame, Fin: requestId == 0 || frame is IncomingResponseFrame);
                    }
                }
            }
        }

        internal override ValueTask ResetAsync(long streamId) => new ValueTask();

        internal override async ValueTask SendAsync(
            long streamId,
            OutgoingFrame frame,
            bool fin,
            CancellationToken cancel) =>
            await SendFrameAsync(streamId, frame, cancel);

        private (int, IncomingFrame?) ParseFrame(ArraySegment<byte> readBuffer)
        {
            // The magic and version fields have already been checked.
            var frameType = (Ice1Definitions.FrameType)readBuffer[8];
            byte compressionStatus = readBuffer[9];
            int size = readBuffer.Count;
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
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        TraceReceivedProtocolFrame(frameType);
                    }
                    if (Endpoint.IsDatagram)
                    {
                        if (Endpoint.Communicator.WarnConnections)
                        {
                            Endpoint.Communicator.Logger.Warning(
                                $"ignoring close connection frame for datagram connection:\n{this}");
                        }
                    }
                    else
                    {
                        throw new ConnectionClosedByPeerException("close connection frame received from the peer");
                    }
                    return default;
                }

                case Ice1Definitions.FrameType.Request:
                {
                    var request = new IncomingRequestFrame(Endpoint.Protocol,
                                                           readBuffer.Slice(Ice1Definitions.HeaderSize + 4),
                                                           IncomingFrameSizeMax);
                    int requestId = readBuffer.AsReadOnlySpan(Ice1Definitions.HeaderSize, 4).ReadInt();
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        ProtocolTrace.TraceFrame(Endpoint.Communicator, requestId, request, compressionStatus);
                    }
                    return (requestId, request);
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
                    return default;
                }

                case Ice1Definitions.FrameType.Reply:
                {
                    var response = new IncomingResponseFrame(Endpoint.Protocol,
                                                             readBuffer.Slice(Ice1Definitions.HeaderSize + 4),
                                                             IncomingFrameSizeMax);
                    int requestId = readBuffer.AsReadOnlySpan(Ice1Definitions.HeaderSize, 4).ReadInt();
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        ProtocolTrace.TraceFrame(Endpoint.Communicator, requestId, response, compressionStatus);
                    }
                    return (requestId, response);
                }

                case Ice1Definitions.FrameType.ValidateConnection:
                {
                    if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                    {
                        TraceReceivedProtocolFrame(frameType);
                    }
                    HeartbeatCallback!();
                    return default;
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
            Ice1Definitions.CheckHeader(readBuffer.AsSpan(0, 8));
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

        private async ValueTask PerformSendFrameAsync(long streamId, object frame)
        {
            List<ArraySegment<byte>> writeBuffer;

            bool compress = false;
            if (frame is OutgoingRequestFrame requestFrame)
            {
                writeBuffer = Ice1Definitions.GetRequestData(requestFrame, (int)streamId);
                compress = requestFrame.Compress;
            }
            else if (frame is OutgoingResponseFrame responseFrame)
            {
                Debug.Assert(streamId > 0);
                writeBuffer = Ice1Definitions.GetResponseData(responseFrame, (int)streamId);
                compress = responseFrame.Compress;
            }
            else
            {
                Debug.Assert(frame is List<ArraySegment<byte>>);
                writeBuffer = (List<ArraySegment<byte>>)frame;
            }

            // Compress the frame if needed and possible
            int size = writeBuffer.GetByteCount();
            byte compressionStatus = 0;
            if (BZip2.IsLoaded && compress)
            {
                List<ArraySegment<byte>>? compressed = null;
                if (size >= Endpoint.Communicator.CompressionMinSize)
                {
                    compressed = BZip2.Compress(writeBuffer,
                                                size,
                                                Ice1Definitions.HeaderSize,
                                                Endpoint.Communicator.CompressionLevel);
                }

                ArraySegment<byte> header;
                if (compressed != null)
                {
                    writeBuffer = compressed;
                    header = writeBuffer[0];
                    size = writeBuffer.GetByteCount();
                }
                else // Message not compressed, request compressed response, if any.
                {
                    header = writeBuffer[0];
                    header[9] = 1; // Write the compression status
                }
                compressionStatus = header[9];
            }

            if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                if (writeBuffer == frame)
                {
                    TraceSendProtocolFrame((Ice1Definitions.FrameType)writeBuffer[0][8]);
                }
                else
                {
                    ProtocolTrace.TraceFrame(Endpoint.Communicator, (int)streamId, frame, compressionStatus);
                }
            }

            // Ensure the frame isn't bigger than what we can send with the transport.
            _transceiver.CheckSendSize(size);

            // Write the frame
            await SendAsync(writeBuffer).ConfigureAwait(false);
        }

        private Task SendFrameAsync(long streamId, object frame, CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();

            // Synchronization is required here because this might be called concurrently by the connection code
            lock (_mutex)
            {
                ValueTask sendTask = QueueAsync(streamId, frame, cancel);
                _sendTask = sendTask.IsCompletedSuccessfully ? Task.CompletedTask : sendTask.AsTask();
                return _sendTask;
            }

            async ValueTask QueueAsync(long streamId, object frame, CancellationToken cancel)
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
                await PerformSendFrameAsync(streamId, frame).ConfigureAwait(false);
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
