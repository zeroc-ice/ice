//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Text;

namespace ZeroC.Ice
{
    internal class Ice1BinaryConnection : IBinaryConnection
    {
        public Endpoint Endpoint { get; }
        public ITransceiver Transceiver { get; }

        private readonly int _incomingFrameSizeMax;
        private Action? _heartbeatCallback;
        private readonly bool _incoming;
        private int _nextRequestId;
        private Task _receiveTask = Task.CompletedTask;
        private Action<int>? _receivedCallback;
        private Task _sendTask = Task.CompletedTask;
        private Action<int>? _sentCallback;

        private static readonly List<ArraySegment<byte>> _closeConnectionFrame =
            new List<ArraySegment<byte>> { Ice1Definitions.CloseConnectionFrame };

        private static readonly List<ArraySegment<byte>> _validateConnectionFrame =
            new List<ArraySegment<byte>> { Ice1Definitions.ValidateConnectionFrame };

        public async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            if (!(exception is ConnectionClosedByPeerException))
            {
                // Write and wait for the close connection frame to be written
                try
                {
                    await SendFrameAsync(0, _closeConnectionFrame, cancel).ConfigureAwait(false);
                }
                catch
                {
                    // Ignore
                }
            }

            // Notify the transport of the graceful connection closure.
            try
            {
                await Transceiver.ClosingAsync(exception, cancel).ConfigureAwait(false);
            }
            catch
            {
                // Ignore
            }

            // Wait for the connection closure from the peer
            try
            {
                await _receiveTask.WaitAsync(cancel).ConfigureAwait(false);
            }
            catch
            {
                // Ignore
            }
        }

        public ValueTask DisposeAsync()
        {
            Transceiver.ThreadSafeClose();
            Transceiver.Destroy();
            return default;
        }

        public async ValueTask HeartbeatAsync(CancellationToken cancel) =>
            await SendFrameAsync(0, _validateConnectionFrame, cancel).ConfigureAwait(false);

        public async ValueTask InitializeAsync(
            Action heartbeatCallback,
            Action<int> sentCallback,
            Action<int> receivedCallback,
            CancellationToken cancel)
        {
            _heartbeatCallback = heartbeatCallback;
            _sentCallback = sentCallback;
            _receivedCallback = receivedCallback;

            // Initialize the transport
            await Transceiver.InitializeAsync(cancel).ConfigureAwait(false);

            ArraySegment<byte> readBuffer = default;
            if (!Endpoint.IsDatagram) // Datagram connections are always implicitly validated.
            {
                if (_incoming) // The server side has the active role for connection validation.
                {
                    int offset = 0;
                    int length = _validateConnectionFrame.GetByteCount();
                    while (offset < length)
                    {
                        offset += await Transceiver.WriteAsync(_validateConnectionFrame,
                                                               offset,
                                                               cancel).ConfigureAwait(false);
                    }
                    Debug.Assert(offset == length);
                }
                else // The client side has the passive role for connection validation.
                {
                    readBuffer = new ArraySegment<byte>(new byte[Ice1Definitions.HeaderSize]);
                    int offset = 0;
                    while (offset < Ice1Definitions.HeaderSize)
                    {
                        offset += await Transceiver.ReadAsync(readBuffer,
                                                              offset,
                                                              cancel).ConfigureAwait(false);
                    }

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
                }
            }

            if (!Endpoint.IsDatagram) // Datagram connections are always implicitly validated.
            {
                if (_incoming) // The server side has the active role for connection validation.
                {
                    ProtocolTrace.TraceSend(Endpoint.Communicator,
                                            Endpoint.Protocol,
                                            Ice1Definitions.ValidateConnectionFrame);
                    _sentCallback!(Ice1Definitions.ValidateConnectionFrame.Length);
                }
                else
                {
                    ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, readBuffer);
                    _receivedCallback!(readBuffer.Count);
                }
            }

            if (Endpoint.Communicator.TraceLevels.Network >= 1)
            {
                var s = new StringBuilder();
                if (Endpoint.IsDatagram)
                {
                    s.Append("starting to ");
                    s.Append(_incoming ? "send" : "receive");
                    s.Append(' ');
                    s.Append(Endpoint.TransportName);
                    s.Append(" datagrams\n");
                    s.Append(Transceiver.ToDetailedString());
                }
                else
                {
                    s.Append(_incoming ? "established" : "accepted");
                    s.Append(' ');
                    s.Append(Endpoint.TransportName);
                    s.Append(" connection\n");
                    s.Append(ToString());
                }
                Endpoint.Communicator.Logger.Trace(Endpoint.Communicator.TraceLevels.NetworkCategory,
                                                    s.ToString());
            }
        }

        public async ValueTask<(long StreamId, object? Frame, bool Fin)> ReceiveAsync(CancellationToken cancel)
        {
            while (true)
            {
                int requestId = 0;
                object? frame = null;
                Task<ArraySegment<byte>>? task = null;
                ValueTask<ArraySegment<byte>> receiveTask = PerformReceiveFrameAsync();
                if (receiveTask.IsCompletedSuccessfully)
                {
                    _receiveTask = Task.CompletedTask;
                    (requestId, frame) = ParseFrame(receiveTask.Result);
                }
                else
                {
                    _receiveTask = task = receiveTask.AsTask();
                }

                if (task != null)
                {
                    (requestId, frame) = ParseFrame(await task.ConfigureAwait(false));
                }

                if (frame != null)
                {
                    return (StreamId: requestId, Frame: frame, Fin: requestId == 0 || frame is IncomingResponseFrame);
                }
            }
        }

        public long NewStream(bool bidirectional)
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

        public ValueTask ResetAsync(long streamId) =>
            throw new NotSupportedException("ice1 transports don't support stream reset");

        public async ValueTask SendAsync(long streamId, object frame, bool fin, CancellationToken cancel) =>
            await SendFrameAsync(streamId, frame, cancel);

        public override string ToString() => Transceiver.ToString()!;

        internal Ice1BinaryConnection(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter)
        {
            Transceiver = transceiver;
            Endpoint = endpoint;

            _incoming = adapter != null;
            _incomingFrameSizeMax = adapter?.IncomingFrameSizeMax ?? Endpoint.Communicator.IncomingFrameSizeMax;
        }

        private (int, object?) ParseFrame(ArraySegment<byte> readBuffer)
        {
            // The magic and version fields have already been checked.
            var frameType = (Ice1Definitions.FrameType)readBuffer[8];
            byte compressionStatus = readBuffer[9];
            if (compressionStatus == 2)
            {
                if (BZip2.IsLoaded)
                {
                    readBuffer = BZip2.Decompress(readBuffer, Ice1Definitions.HeaderSize, _incomingFrameSizeMax);
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
                    ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, readBuffer);
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
                        throw new ConnectionClosedByPeerException();
                    }
                    return default;
                }

                case Ice1Definitions.FrameType.Request:
                {
                    var request = new IncomingRequestFrame(Endpoint.Protocol,
                                                           readBuffer.Slice(Ice1Definitions.HeaderSize + 4),
                                                           _incomingFrameSizeMax);
                    ProtocolTrace.TraceFrame(Endpoint.Communicator, readBuffer, request);
                    return (readBuffer.AsReadOnlySpan(Ice1Definitions.HeaderSize, 4).ReadInt(), request);
                }

                case Ice1Definitions.FrameType.RequestBatch:
                {
                    ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, readBuffer);
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
                    var responseFrame = new IncomingResponseFrame(Endpoint.Protocol,
                                                                  readBuffer.Slice(Ice1Definitions.HeaderSize + 4),
                                                                  _incomingFrameSizeMax);
                    ProtocolTrace.TraceFrame(Endpoint.Communicator, readBuffer, responseFrame);
                    return (readBuffer.AsReadOnlySpan(14, 4).ReadInt(), responseFrame);
                }

                case Ice1Definitions.FrameType.ValidateConnection:
                {
                    ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, readBuffer);
                    _heartbeatCallback!();
                    return default;
                }

                default:
                {
                    ProtocolTrace.Trace(
                        "received unknown frame\n(invalid, closing connection)",
                        Endpoint.Communicator,
                        Endpoint.Protocol,
                        readBuffer);
                    throw new InvalidDataException(
                        $"received ice1 frame with unknown frame type `{frameType}'");
                }
            }
        }

        private async ValueTask<ArraySegment<byte>> PerformReceiveFrameAsync()
        {
            // Read header
            ArraySegment<byte> readBuffer;
            if (Endpoint.IsDatagram)
            {
                readBuffer = await Transceiver.ReadAsync().ConfigureAwait(false);
                _receivedCallback!(readBuffer.Count);
            }
            else
            {
                readBuffer = new ArraySegment<byte>(new byte[256], 0, Ice1Definitions.HeaderSize);
                int offset = 0;
                while (offset < Ice1Definitions.HeaderSize)
                {
                    offset += await Transceiver.ReadAsync(readBuffer, offset).ConfigureAwait(false);
                    _receivedCallback!(readBuffer.Count);
                }
            }

            // Check header
            Ice1Definitions.CheckHeader(readBuffer.AsSpan(0, 8));
            int size = readBuffer.AsReadOnlySpan(10, 4).ReadInt();
            if (size < Ice1Definitions.HeaderSize)
            {
                throw new InvalidDataException($"received ice1 frame with only {size} bytes");
            }

            if (size > _incomingFrameSizeMax)
            {
                throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
            }

            // Read the remainder of the frame if needed
            if (!Endpoint.IsDatagram)
            {
                if (size > readBuffer.Array!.Length)
                {
                    // Allocate a new array and copy the header over
                    var buffer = new ArraySegment<byte>(new byte[size], 0, size);
                    readBuffer.AsSpan().CopyTo(buffer.AsSpan(0, Ice1Definitions.HeaderSize));
                    readBuffer = buffer;
                }
                else if (size > readBuffer.Count)
                {
                    readBuffer = new ArraySegment<byte>(readBuffer.Array!, 0, size);
                }
                Debug.Assert(size == readBuffer.Count);

                int offset = Ice1Definitions.HeaderSize;
                while (offset < readBuffer.Count)
                {
                    int bytesReceived = await Transceiver.ReadAsync(readBuffer, offset).ConfigureAwait(false);
                    offset += bytesReceived;

                    // Trace the receive progress within the loop as we might be receiving significant amount
                    // of data here.
                    _receivedCallback!(bytesReceived);
                }
            }
            else if (size > readBuffer.Count)
            {
                if (Endpoint.Communicator.WarnDatagrams)
                {
                    Endpoint.Communicator.Logger.Warning($"maximum datagram size of {readBuffer.Count} exceeded");
                }
                return default;
            }
            return readBuffer;
        }

        private async ValueTask PerformSendFrameAsync(long streamId, object frame)
        {
            List<ArraySegment<byte>> writeBuffer;

            // TODO: add abstract OutgoingFrame class with an abstract GetRequestData(streamId) method?
            bool compress = false;
            if (frame is OutgoingRequestFrame requestFrame)
            {
                writeBuffer = Ice1Definitions.GetRequestData(requestFrame, (int)streamId);
                compress = requestFrame.Compress;
                ProtocolTrace.TraceFrame(Endpoint.Communicator, writeBuffer[0], requestFrame);
            }
            else if (frame is OutgoingResponseFrame responseFrame)
            {
                Debug.Assert(streamId > 0);
                writeBuffer = Ice1Definitions.GetResponseData(responseFrame, (int)streamId);
                compress = responseFrame.Compress;
                ProtocolTrace.TraceFrame(Endpoint.Communicator, writeBuffer[0], responseFrame);
            }
            else
            {
                Debug.Assert(frame is List<ArraySegment<byte>>);
                writeBuffer = (List<ArraySegment<byte>>)frame;
                ProtocolTrace.TraceSend(Endpoint.Communicator, Endpoint.Protocol, writeBuffer[0]);
            }

            // Compress the frame if needed and possible
            int size = writeBuffer.GetByteCount();
            if (BZip2.IsLoaded && compress)
            {
                List<ArraySegment<byte>>? compressed = null;
                if (size >= 100)
                {
                    compressed = BZip2.Compress(writeBuffer,
                                                size,
                                                Ice1Definitions.HeaderSize,
                                                Endpoint.Communicator.CompressionLevel);
                }

                if (compressed != null)
                {
                    writeBuffer = compressed;
                    size = writeBuffer.GetByteCount();
                }
                else // Message not compressed, request compressed response, if any.
                {
                    ArraySegment<byte> header = writeBuffer[0];
                    header[9] = 1; // Write the compression status
                }
            }

            // Ensure the frame isn't bigger than what we can send with the transport.
            Transceiver.CheckSendSize(size);

            // Write the frame
            int offset = 0;
            while (offset < size)
            {
                int bytesSent = await Transceiver.WriteAsync(writeBuffer, offset).ConfigureAwait(false);
                offset += bytesSent;
                _sentCallback!(bytesSent);
            }
        }

        private Task SendFrameAsync(long streamId, object frame, CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();
            ValueTask sendTask = QueueAsync(streamId, frame, cancel);
            _sendTask = sendTask.IsCompletedSuccessfully ? Task.CompletedTask : sendTask.AsTask();
            return _sendTask;

            async ValueTask QueueAsync(long streamId, object frame, CancellationToken cancel)
            {
                // Wait for the previous send to complete
                try
                {
                    await _sendTask.ConfigureAwait(false);
                }
                catch (DatagramLimitException)
                {
                    // If the send failed because the datagram was too large, ignore and continue sending.
                }
                catch (OperationCanceledException)
                {
                    // If the send was canceled, ignore and continue sending.
                }

                // If the send got cancelled, throw now. This isn't a fatal connection error, the next pending
                // outgoing will be sent because we ignore the cancelation exception above.
                // TODO: is it really a good idea to cancel the request here with ice1?  The stream/request ID assigned
                // for the request won't be used.
                cancel.ThrowIfCancellationRequested();

                // Perform the write
                await PerformSendFrameAsync(streamId, frame).ConfigureAwait(false);
            }
        }
    }
}
