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
    internal class SlicBinaryConnection : IBinaryConnection
    {
        public Endpoint Endpoint { get; }
        public ITransceiver Transceiver { get; }

        private readonly int _frameSizeMax;
        private Action _heartbeatCallback;
        private readonly bool _incoming;
        private long _nextStreamId;
        private Action<int> _receivedCallback;
        private Task _sendTask = Task.CompletedTask;
        private Action<int> _sentCallback;

        private static readonly List<ArraySegment<byte>> _closeConnectionFrame =
            new List<ArraySegment<byte>> { Ice2Definitions.CloseConnectionFrame };

        private static readonly List<ArraySegment<byte>> _validateConnectionFrame =
            new List<ArraySegment<byte>> { Ice2Definitions.ValidateConnectionFrame };

        public async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            // Write the close connection frame.
            await SendFrameAsync(0, _closeConnectionFrame, cancel).ConfigureAwait(false);

            // Notify the transport of the graceful connection closure.
            await Transceiver.ClosingAsync(exception, cancel).ConfigureAwait(false);
        }

        public ValueTask DisposeAsync() => Transceiver.DisposeAsync();

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

            ArraySegment<byte> readBuffer;
            if (_incoming) // The server side has the active role for connection validation.
            {
                await SendAsync(_validateConnectionFrame, cancel).ConfigureAwait(false);

                ProtocolTrace.TraceSend(Endpoint.Communicator,
                                        Endpoint.Protocol,
                                        Ice2Definitions.ValidateConnectionFrame);
            }
            else // The client side has the passive role for connection validation.
            {
                readBuffer = new ArraySegment<byte>(new byte[Ice2Definitions.HeaderSize]);
                await ReceiveAsync(readBuffer, cancel).ConfigureAwait(false);

                Ice2Definitions.CheckHeader(readBuffer.AsSpan(0, 8));
                var frameType = (Ice2Definitions.FrameType)readBuffer[8];
                if (frameType != Ice2Definitions.FrameType.ValidateConnection)
                {
                    throw new InvalidDataException(@$"received ice2 frame with frame type `{frameType
                        }' before receiving the validate connection frame");
                }

                // TODO: this is temporary code. With the 2.0 encoding, sizes are always variable-length
                // with the length encoded on the first 2 bits of the size. Assuming the size is encoded
                // on 4 bytes (like we do below) is not correct.
                int size = readBuffer.AsReadOnlySpan(10, 4).ReadFixedLengthSize(Endpoint.Protocol.GetEncoding());
                if (size != Ice2Definitions.HeaderSize)
                {
                    throw new InvalidDataException(
                        @$"received an ice2 frame with validate connection type and a size of `{size
                        }' bytes");
                }

                ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, readBuffer);
            }

            if (Endpoint.Communicator.TraceLevels.Network >= 1)
            {
                var s = new StringBuilder();
                s.Append(_incoming ? "accepted" : "established");
                s.Append(' ');
                s.Append(Endpoint.TransportName);
                s.Append(" connection\n");
                s.Append(ToString());
                Endpoint.Communicator.Logger.Trace(Endpoint.Communicator.TraceLevels.NetworkCategory, s.ToString());
            }
        }

        public async ValueTask<(long StreamId, IncomingFrame? Frame, bool Fin)> ReceiveAsync(CancellationToken cancel)
        {
            while (true)
            {
                ArraySegment<byte> buffer = await PerformReceiveFrameAsync().ConfigureAwait(false);
                (int requestId, IncomingFrame? frame) = ParseFrame(buffer);
                if (frame != null)
                {
                    return (StreamId: requestId, Frame: frame, Fin: requestId == 0 || frame is IncomingResponseFrame);
                }
            }
        }

        public long NewStream(bool bidirectional) => bidirectional ? ++_nextStreamId : 0;

        public ValueTask ResetAsync(long streamId) =>
            throw new NotSupportedException("ice2 transports don't support stream reset");

        public async ValueTask SendAsync(long streamId, OutgoingFrame frame, bool fin, CancellationToken cancel) =>
            await SendFrameAsync(streamId, frame, cancel);

        public override string ToString() => Transceiver.ToString()!;

        internal SlicBinaryConnection(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter)
        {
            Transceiver = transceiver;
            Endpoint = endpoint;

            _incoming = adapter != null;
            _frameSizeMax = adapter?.IncomingFrameSizeMax ?? Endpoint.Communicator.IncomingFrameSizeMax;
            _sentCallback = _receivedCallback = _ => {};
            _heartbeatCallback = () => {};
        }

        private (int, IncomingFrame?) ParseFrame(ArraySegment<byte> readBuffer)
        {
            // The magic and version fields have already been checked.
            var frameType = (Ice2Definitions.FrameType)readBuffer[8];

            switch (frameType)
            {
                case Ice2Definitions.FrameType.CloseConnection:
                {
                    ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, readBuffer);
                    throw new ConnectionClosedByPeerException();
                }

                case Ice2Definitions.FrameType.Request:
                {
                    var request = new IncomingRequestFrame(Endpoint.Protocol,
                                                           readBuffer.Slice(Ice2Definitions.HeaderSize + 4),
                                                           _frameSizeMax);
                    ProtocolTrace.TraceFrame(Endpoint.Communicator, readBuffer, request);
                    return (readBuffer.AsReadOnlySpan(Ice2Definitions.HeaderSize, 4).ReadInt(), request);
                }

                case Ice2Definitions.FrameType.Reply:
                {
                    var responseFrame = new IncomingResponseFrame(Endpoint.Protocol,
                                                                  readBuffer.Slice(Ice2Definitions.HeaderSize + 4),
                                                                  _frameSizeMax);
                    ProtocolTrace.TraceFrame(Endpoint.Communicator, readBuffer, responseFrame);
                    return (readBuffer.AsReadOnlySpan(14, 4).ReadInt(), responseFrame);
                }

                case Ice2Definitions.FrameType.ValidateConnection:
                {
                    ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, readBuffer);
                    _heartbeatCallback();
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
                        $"received ice2 frame with unknown frame type `{frameType}'");
                }
            }
        }

        private async ValueTask<ArraySegment<byte>> PerformReceiveFrameAsync()
        {
            // Read header
            var readBuffer = new ArraySegment<byte>(new byte[256], 0, Ice2Definitions.HeaderSize);
            await ReceiveAsync(readBuffer).ConfigureAwait(false);

            // Check header
            Ice2Definitions.CheckHeader(readBuffer.AsSpan(0, 8));
            int size = readBuffer.AsReadOnlySpan(10, 4).ReadFixedLengthSize(Endpoint.Protocol.GetEncoding());
            if (size < Ice2Definitions.HeaderSize)
            {
                throw new InvalidDataException($"received ice2 frame with only {size} bytes");
            }

            if (size > _frameSizeMax)
            {
                throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
            }

            // Read the remainder of the frame if needed
            if (size > readBuffer.Count)
            {
                if (size > readBuffer.Array!.Length)
                {
                    // Allocate a new array and copy the header over
                    var buffer = new ArraySegment<byte>(new byte[size], 0, size);
                    readBuffer.AsSpan().CopyTo(buffer.AsSpan(0, Ice2Definitions.HeaderSize));
                    readBuffer = buffer;
                }
                else
                {
                    readBuffer = new ArraySegment<byte>(readBuffer.Array!, 0, size);
                }
                Debug.Assert(size == readBuffer.Count);

                await ReceiveAsync(readBuffer.Slice(Ice2Definitions.HeaderSize)).ConfigureAwait(false);
            }
            return readBuffer;
        }

        private async ValueTask PerformSendFrameAsync(long streamId, object frame)
        {
            List<ArraySegment<byte>> writeBuffer;

            // TODO: add abstract OutgoingFrame class with an abstract GetRequestData(streamId) method?
            if (frame is OutgoingRequestFrame requestFrame)
            {
                writeBuffer = Ice2Definitions.GetRequestData(requestFrame, streamId);
                ProtocolTrace.TraceFrame(Endpoint.Communicator, writeBuffer[0], requestFrame);
            }
            else if (frame is OutgoingResponseFrame responseFrame)
            {
                Debug.Assert(streamId > 0);
                writeBuffer = Ice2Definitions.GetResponseData(responseFrame, streamId);
                ProtocolTrace.TraceFrame(Endpoint.Communicator, writeBuffer[0], responseFrame);
            }
            else
            {
                Debug.Assert(frame is List<ArraySegment<byte>>);
                writeBuffer = (List<ArraySegment<byte>>)frame;
                ProtocolTrace.TraceSend(Endpoint.Communicator, Endpoint.Protocol, writeBuffer[0]);
            }

            // Write the frame
            await SendAsync(writeBuffer).ConfigureAwait(false);
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
                catch (OperationCanceledException)
                {
                    // If the send was canceled, ignore and continue sending.
                }

                // If the send got cancelled, throw now. This isn't a fatal connection error, the next pending
                // outgoing will be sent because we ignore the cancelation exception above.
                // TODO: is it really a good idea to cancel the request here? The stream/request ID assigned for the
                // the request won't be used.
                cancel.ThrowIfCancellationRequested();

                // Perform the write
                await PerformSendFrameAsync(streamId, frame).ConfigureAwait(false);
            }
        }
        private async ValueTask ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel = default)
        {
            int offset = 0;
            while (offset != buffer.Count)
            {
                int received = await Transceiver.ReceiveAsync(buffer.Slice(offset), cancel).ConfigureAwait(false);
                offset += received;
                _receivedCallback!(received);
            }
        }

        private async ValueTask SendAsync(IList<ArraySegment<byte>> buffers, CancellationToken cancel = default)
        {
            int sent = await Transceiver.SendAsync(buffers, cancel).ConfigureAwait(false);
            Debug.Assert(sent == buffers.GetByteCount()); // TODO: do we need to support partial writes?
            _sentCallback!(sent);
        }
    }
}
