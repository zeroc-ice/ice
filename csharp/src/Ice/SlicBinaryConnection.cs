//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Text;
using System.IO;

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
        private Stream _readStream;
        private BinaryReader _reader;
        private Action<int> _receivedCallback;
        private Task _sendTask = Task.CompletedTask;
        private Action<int> _sentCallback;

        private const int HeaderSizeMax = 16;

        private enum SlicVersion : byte
        {
            Unsupported = 0,
            Slic1 = 1,
        };

        private static readonly List<ArraySegment<byte>> _initializeFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x00, // Frame type
                (byte)SlicVersion.Slic1, // Slic1 version
                (byte)Protocol.Ice2, // Ice2 protocol version
            }
        };

        private static readonly List<ArraySegment<byte>> _initializeAckFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x01, // Frame type
                (byte)SlicVersion.Slic1, // Slic1 version
                (byte)Protocol.Ice2, // Ice2 protocol version
            }
        };

        private static readonly List<ArraySegment<byte>> _versionFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x02, // Frame type
                0x01, // Version sequence length
                (byte)SlicVersion.Slic1, // Slic1 version
                0x01, // Ice protocol version sequence length
                (byte)Protocol.Ice2, // Ice protocol version
            }
        };

        private static readonly List<ArraySegment<byte>> _pingFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x03, // Frame type
            }
        };
        private static readonly List<ArraySegment<byte>> _pongFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x04, // Frame type
            }
        };

        private static readonly List<ArraySegment<byte>> _streamFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x08, // Frame type (0x08 and 0x09 for stream frame with FIN bit set)
                // stream ID (varlong)
                // stream data length (varlong)
                // stream data
            }
        };

        private static readonly List<ArraySegment<byte>> _resetStreamFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x04, // Frame type (0x08 and 0x09 for stream frame with FIN bit set)
                // stream ID (varlong)
                // error code (varlong)
            }
        };

        private static readonly List<ArraySegment<byte>> _closeFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x1c, // Frame type (0x08 and 0x09 for stream frame with FIN bit set)
                // stream ID (varlong)
                // error code (varlong)
                // reason (string)
            }
        };

        public async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            // Write the close connection frame.
            await SendFrameAsync(0, _closeFrame, cancel).ConfigureAwait(false);

            // Notify the transport of the graceful connection closure.
            await Transceiver.CloseAsync(exception, cancel).ConfigureAwait(false);
        }

        public ValueTask DisposeAsync() => Transceiver.DisposeAsync();

        public async ValueTask HeartbeatAsync(CancellationToken cancel) =>
            await SendFrameAsync(0, _pingFrame, cancel).ConfigureAwait(false);

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

            if (_incoming)
            {
                // Read the INITIALIZE frame from the client
                ArraySegment<byte> frame;
                frame = await ReceiveBufferedAsync(_initializeFrame.GetByteCount(), cancel).ConfigureAwait(false);

                // Check the frame type
                if (frame[0] != _initializeFrame[0][0])
                {
                    throw new InvalidDataException(@$"unexpected Slic frame with frame type `{frame[0]}'");
                }

                // Check that the Slic version and Ice protocol are supported
                if ((SlicVersion)frame[1] != SlicVersion.Slic1 || (Protocol)frame[2] != Protocol.Ice2)
                {
                    // Unsupported Slic or Ice protocol version, stop reading there and reply with a VERSION frame to
                    // provide the client with the supported Slic/Ice protocol versions.
                    await SendAsync(_versionFrame, cancel).ConfigureAwait(false);

                    // Wait for a new INITIALIZE frame with hopefully this time supported versions
                    await ReceiveAsync(frame, cancel).ConfigureAwait(false);

                    // Check the frame type
                    if (frame[0] != _initializeFrame[0][0])
                    {
                        throw new InvalidDataException(@$"unexpected Slic frame with frame type `{readBuffer[0]}'");
                    }

                    if ((SlicVersion)frame[1] != SlicVersion.Slic1)
                    {
                        throw new InvalidDataException(@$"unsupported Slic version `{readBuffer[1]}'");
                    }
                    if ((Protocol)frame[2] != Protocol.Ice2)
                    {
                        throw new InvalidDataException(@$"unsupported Ice protocol version `{readBuffer[2]}'");
                    }
                }

                // Send back an INITIALIZE_ACK frame
                await SendAsync(_initializeAckFrame, cancel).ConfigureAwait(false);
            }
            else
            {
                // Send the initialization frame to the server
                await SendAsync(_initializeFrame, cancel).ConfigureAwait(false);

                // Wait for the INITIALIZE_ACK frame or VERSION frame
                ArraySegment<byte> frame;
                frame = await ReceiveBufferedAsync(1, cancel).ConfigureAwait(false);

                if (frame[0] == _initializeAckFrame[0][0])
                {
                    frame = await ReceiveBufferedAsync(2, cancel).ConfigureAwait(false);
                    if ((SlicVersion)frame[0] != SlicVersion.Slic1)
                    {
                        throw new InvalidDataException(@$"unsupported Slic version `{frame[1]}'");
                    }
                    if ((Protocol)frame[1] != Protocol.Ice2)
                    {
                        throw new InvalidDataException(@$"unsupported Ice protocol version `{frame[2]}'");
                    }
                }
                else if (frame[0] == _versionFrame[0][0])
                {
                    SlicVersion slicVersion = SlicVersion.Unsupported;
                    int size = (int)frame[1];
                    frame = await ReceiveBufferedAsync((int)frame[1], cancel).ConfigureAwait(false);
                    for (int i = 0; i < (int)frame[1]; ++i)
                    {
                        if ((SlicVersion)frame[1 + i] == SlicVersion.Slic1)
                        {
                            slicVersion = SlicVersion.Slic1;
                            break;
                        }
                    }
                    if (slicVersion == SlicVersion.Unsupported)
                    {
                        throw new InvalidDataException(@$"unsupported Slic version `{frame[1]}'");
                    }
                }
                else
                {
                    throw new InvalidDataException(@$"unexpected Slic frame with frame type `{frame[0]}'");
                }

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
            _readStream = new BufferedStream(new TransceiverReadStream(transceiver), 256);
            _reader = new BinaryReader(_readStream);
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
            int received;
            if (_receivePayloadOffset == _receivePayloadLength)
            {
                // If the payload has been fully read, read a new frame.
                received = await ReceiveFrameAsync(buffer, cancel).ConfigureAwait(false);
                Debug.Assert(_receivePayloadLength > 0 && _receivePayloadOffset == 0);
            }
            else if (_receiveBuffer.Count > 0)
            {
                // Otherwise, if there's still data buffered for the payload, consume the buffered data.
                int length = Math.Min(_receiveBuffer.Count, buffer.Count);
                ArraySegment<byte> buffered = await ReceiveBufferedAsync(length, cancel).ConfigureAwait(false);
                buffered.CopyTo(buffer);
                received = length;
            }
            else
            {
                // No buffered data, we'll read the payload directly from the underlying transport.
                received = 0;
            }

            // Read the reminder of the payload from the underlying transport
            if (received < buffer.Count)
            {
                int length = Math.Min(_receivePayloadLength - _receivePayloadOffset, buffer.Count);
                received += await _underlying.ReceiveAsync(buffer.Slice(received, length - received),
                                                           cancel).ConfigureAwait(false);
            }

            int offset = 0;
            while (offset != buffer.Count)
            {
                int received = await Transceiver.ReceiveAsync(buffer.Slice(offset), cancel).ConfigureAwait(false);
                offset += received;
                _receivedCallback!(received);
            }
        }

        private async ValueTask<ArraySegment<byte>> ReceiveBufferedAsync(int byteCount, CancellationToken cancel)
        {
            int offset = _receiveBuffer.Count;
            if (_receiveBuffer.Count < byteCount)
            {
                // If there's not enough data buffered for byteCount we read more data in the buffer. We first
                // need to make sure there's enough space in the buffer to read it however.
                if (_receiveBuffer.Count == 0)
                {
                    // Use the full buffer array if there's no more buffered data.
                    _receiveBuffer = new ArraySegment<byte>(_receiveBuffer.Array!);
                }
                else if (_receiveBuffer.Offset + _receiveBuffer.Count + byteCount > _receiveBuffer.Array!.Length)
                {
                    // There's still buffered data but not enough space left in the array to read the given bytes.
                    // In theory, the number of bytes to read should always be lower than the un-used buffer space
                    // at the start of the buffer. We move the data at the end of the buffer to the begining to
                    // make space to read the given number of bytes.
                    Debug.Assert(_receiveBuffer.Offset >= byteCount);
                    _receiveBuffer.CopyTo(_receiveBuffer.Array!, 0);
                    _receiveBuffer = new ArraySegment<byte>(_receiveBuffer.Array);
                }
                else
                {
                    // There's still buffered data and enough space to read the given bytes after the buffered
                    // data.
                    _receiveBuffer = new ArraySegment<byte>(
                        _receiveBuffer.Array,
                        _receiveBuffer.Offset,
                        _receiveBuffer.Array.Length - _receiveBuffer.Offset);
                }

                while (offset < byteCount)
                {
                    offset += await _underlying.ReceiveAsync(_receiveBuffer.Slice(offset), cancel);
                }
            }

            ArraySegment<byte> buffer = _receiveBuffer.Slice(0, byteCount);
            if (byteCount < offset)
            {
                _receiveBuffer = _receiveBuffer.Slice(byteCount, offset - byteCount);
            }
            else
            {
                _receiveBuffer = _receiveBuffer.Slice(0, 0);
            }
            return buffer;
        }

        private async ValueTask SendAsync(IList<ArraySegment<byte>> buffers, CancellationToken cancel = default)
        {
            int sent = await Transceiver.SendAsync(buffers, cancel).ConfigureAwait(false);
            Debug.Assert(sent == buffers.GetByteCount()); // TODO: do we need to support partial writes?
            _sentCallback!(sent);
        }
    }
}
