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
        public ITransceiver Transceiver => _transceiver.Underlying;

        private readonly int _frameSizeMax;
        private Action _heartbeatCallback;
        private readonly bool _incoming;
        private long _nextStreamId;
        private Action<int> _receivedCallback;
        private Task _sendTask = Task.CompletedTask;
        private Action<int> _sentCallback;
        private readonly BufferedReadTransceiver _transceiver;

        private static readonly List<ArraySegment<byte>> _initializeFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x01, // Frame type (byte)
                0x00, 0x01, // Slic protocol V1 (ushort)
                (byte)Protocol.Ice2, // Ice2 protocol version (byte)
            }
        };

        private static readonly List<ArraySegment<byte>> _initializeAckFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x02, // Frame type (byte)
            }
        };

        private static readonly List<ArraySegment<byte>> _versionFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x03, // Frame type (byte)
                0x01, // Slic protocol version sequence length (byte)
                0x00, 0x01, // Slic protocol V1 (short)
            }
        };

        private static readonly List<ArraySegment<byte>> _pingFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x04, // Frame type (byte)
            }
        };

        private static readonly List<ArraySegment<byte>> _pongFrame = new List<ArraySegment<byte>> { new byte[]
            {
                0x05, // Frame type (byte)
            }
        };

        private static readonly ReadOnlyMemory<byte> _streamFrame = new byte[]
        {
            0x06, // Frame type
            //0x00, 0x00, 0x00, 0x00, // Frame data length (uint)
            //0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Frame data: StreamID (varlong) - Data (bytes)
        };

        private static readonly ReadOnlyMemory<byte> _lastStreamFrame = new byte[]
        {
            0x07, // Frame type
            //0x00, 0x00, 0x00, 0x00, // Frame data length (uint)
            //0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Frame data: StreamID (varlong) - Data (bytes)
        };

        private static readonly ReadOnlyMemory<byte> _resetStreamFrame = new byte[]
        {
            0x08, // Frame type (0x08 and 0x09 for stream frame with FIN bit set)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // stream ID (long)
            0x00, 0x00, // error code (ushort)
        };

        private static readonly ReadOnlyMemory<byte> _closeFrame = new byte[]
        {
            0x09, // Frame type (0x08 and 0x09 for stream frame with FIN bit set)
            0x00, 0x00, // error code (ushort)
            0x00, // reason (string)
        };

        public async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            // Write the close connection frame.
            ArraySegment<byte> frame = _closeFrame.ToArray();
            // TODO: encode reason and reason string
            await PerformSendFrameAsync(new List<ArraySegment<byte>>() { frame }, cancel).ConfigureAwait(false);

            // Notify the transport of the graceful connection closure.
            await _transceiver.CloseAsync(exception, cancel).ConfigureAwait(false);
        }

        public ValueTask DisposeAsync() => _transceiver.DisposeAsync();

        public async ValueTask HeartbeatAsync(CancellationToken cancel) =>
            await PerformSendFrameAsync(_pingFrame, cancel).ConfigureAwait(false);

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
            await _transceiver.InitializeAsync(cancel).ConfigureAwait(false);

            if (_incoming)
            {
                ArraySegment<byte> frame;

                // Read the INITIALIZE frame from the client
                frame = await _transceiver.ReceiveAsync(_initializeFrame[0].Count, cancel).ConfigureAwait(false);

                // Check the frame type
                if (frame[0] != _initializeFrame[0][0])
                {
                    throw new InvalidDataException($"unexpected Slic frame with frame type `{frame[0]}'");
                }

                // Check that the Slic version is supported (we only support version 1 for now)
                if (frame.Slice(1).AsReadOnlySpan().ReadUShort() != 1)
                {
                    // Unsupported Slic version, stop reading there and reply with a VERSION frame to provide the
                    // client the supported Slic versions.
                    await _transceiver.SendAsync(_versionFrame, cancel).ConfigureAwait(false);

                    // Wait for a new INITIALIZE frame with hopefully this time a supported Slic version
                    frame = await _transceiver.ReceiveAsync(_initializeFrame[0].Count, cancel).ConfigureAwait(false);

                    // Check the frame type and Slic version
                    if (frame[0] != _initializeFrame[0][0])
                    {
                        throw new InvalidDataException($"unexpected Slic frame with frame type `{frame[0]}'");
                    }
                    if (frame.Slice(1).AsReadOnlySpan().ReadUShort() != 1)
                    {
                        throw new InvalidDataException($"unsupported Ice protocol version `{frame[2]}'");
                    }
                }

                // Send back an INITIALIZE_ACK frame
                await _transceiver.SendAsync(_initializeAckFrame, cancel).ConfigureAwait(false);
            }
            else
            {
                // Send the initialization frame to the server
                await _transceiver.SendAsync(_initializeFrame, cancel).ConfigureAwait(false);

                // Read the INITIALIZE_ACK or VERSION frame from the server
                byte frameType = await _transceiver.ReceiveByteAsync(cancel).ConfigureAwait(false);

                if (frameType == _versionFrame[0][0])
                {
                    // We only support Slic version 1!
                    byte length = await _transceiver.ReceiveByteAsync(cancel).ConfigureAwait(false);
                    ArraySegment<byte> buffer =
                        await _transceiver.ReceiveAsync(length * 2, cancel).ConfigureAwait(false);
                    ushort[] versions = new ushort[length];
                    for (int i = 0; i < length; ++i)
                    {
                        versions[i] = buffer.Slice(i * 2).AsReadOnlySpan().ReadUShort();
                    }
                    throw new InvalidDataException(
                        $"unsupported Slic version, server supports versions `{string.Join(", ", versions)}'");
                }
                else if (frameType != _initializeAckFrame[0][0])
                {
                    throw new InvalidDataException($"unexpected Slic frame with frame type `{frameType}'");
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
                (long streamId, ArraySegment<byte> buffer, bool fin) =
                    await PerformReceiveFrameAsync().ConfigureAwait(false);
                if (streamId > 0)
                {
                    if (buffer.Count == 0)
                    {
                        // TODO: stream reset!
                    }
                    else
                    {
                        IncomingFrame? frame = ParseProtocolFrame(buffer);
                        if (frame != null)
                        {
                            return (StreamId: streamId, Frame: frame, Fin: fin);
                        }
                    }
                }
            }
        }

        public long NewStream(bool bidirectional) => bidirectional ? ++_nextStreamId : 0;

        public async ValueTask ResetAsync(long streamId)
        {
            ArraySegment<byte> frame = _resetStreamFrame.ToArray();
            frame.Slice(1).AsSpan().WriteLong(streamId);
            // TODO: encode reason
            await PerformSendFrameAsync(new List<ArraySegment<byte>>() { frame },
                                        CancellationToken.None).ConfigureAwait(false);
        }

        public async ValueTask SendAsync(long streamId, OutgoingFrame protocolFrame, bool fin, CancellationToken cancel)
        {
            List<ArraySegment<byte>> buffer;

            // The header is as follow:
            // - Slic frame type (byte)
            // - Slic frame frame size (uint)
            // - stream ID (varlong)
            // - Ice2 frame type (byte)
            // - Ice2 frame size (varlong)

            byte[] header = new byte[protocolFrame.Size];

            if (protocolFrame is OutgoingRequestFrame requestFrame)
            {
                buffer = Ice2Definitions.GetRequestData(requestFrame, streamId);
                ProtocolTrace.TraceFrame(Endpoint.Communicator, buffer[0], requestFrame);
            }
            else if (protocolFrame is OutgoingResponseFrame responseFrame)
            {
                Debug.Assert(streamId > 0);
                buffer = Ice2Definitions.GetResponseData(responseFrame, streamId);
                ProtocolTrace.TraceFrame(Endpoint.Communicator, buffer[0], responseFrame);
            }
            else
            {
                Debug.Assert(false);
                return;
            }

            int size = buffer.GetByteCount();
            ArraySegment<byte> frame = (fin ? _lastStreamFrame : _streamFrame).ToArray();
            frame.Slice(1).AsSpan().WriteUInt((uint)size);
            int sizeLength = frame.Slice(5).AsSpan().WriteVarLong(size);
            buffer.Insert(0, frame.Slice(0, 5 + sizeLength));
            await PerformSendFrameAsync(buffer, cancel);
        }

        public override string ToString() => _transceiver.ToString()!;

        internal SlicBinaryConnection(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter)
        {
            Endpoint = endpoint;

            _incoming = adapter != null;
            _frameSizeMax = adapter?.IncomingFrameSizeMax ?? Endpoint.Communicator.IncomingFrameSizeMax;
            _sentCallback = _receivedCallback = _ => {};
            _heartbeatCallback = () => {};
            _transceiver = new BufferedReadTransceiver(transceiver);
        }

        private IncomingFrame? ParseProtocolFrame(ArraySegment<byte> readBuffer)
        {
            // The magic and version fields have already been checked.
            var frameType = (Ice2Definitions.FrameType)readBuffer[0];

            switch (frameType)
            {
                case Ice2Definitions.FrameType.Request:
                {
                    var request = new IncomingRequestFrame(Endpoint.Protocol,
                                                           readBuffer.Slice(Ice2Definitions.HeaderSize + 4),
                                                           _frameSizeMax);
                    ProtocolTrace.TraceFrame(Endpoint.Communicator, readBuffer, request);
                    return request;
                }

                case Ice2Definitions.FrameType.Reply:
                {
                    var responseFrame = new IncomingResponseFrame(Endpoint.Protocol,
                                                                  readBuffer.Slice(Ice2Definitions.HeaderSize + 4),
                                                                  _frameSizeMax);
                    ProtocolTrace.TraceFrame(Endpoint.Communicator, readBuffer, responseFrame);
                    return responseFrame;
                }

                default:
                {
                    ProtocolTrace.Trace(
                        "received unknown frame\n(invalid, closing connection)",
                        Endpoint.Communicator,
                        Endpoint.Protocol,
                        readBuffer);
                    throw new InvalidDataException($"received ice2 frame with unknown frame type `{frameType}'");
                }
            }
        }

        private async ValueTask<(long, ArraySegment<byte>, bool)> PerformReceiveFrameAsync()
        {
            // Read Slice frame header
            byte frameType = await _transceiver.ReceiveByteAsync().ConfigureAwait(false);
            if (frameType == _pingFrame[0][0])
            {
                // TODO: send pong frame
                //ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, frame);
                _receivedCallback(1);
                _heartbeatCallback();
                return default;
            }
            else if (frameType == _pongFrame[0][0])
            {
                // TODO
                //ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, frame);
                _receivedCallback(1);
                return default;
            }
            else if (frameType == _streamFrame.Span[0] || frameType == _lastStreamFrame.Span[0])
            {
                // Read the frame size
                ArraySegment<byte> buffer = await _transceiver.ReceiveAsync(4).ConfigureAwait(false);
                uint size = buffer.AsReadOnlySpan().ReadUInt();
                if (size > _frameSizeMax)
                {
                    throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
                }
                _receivedCallback(5);

                // Read the frame data
                ArraySegment<byte> data = new byte[size];
                int offset = 0;
                while (offset != size)
                {
                    int received = await _transceiver.ReceiveAsync(data.Slice(offset)).ConfigureAwait(false);
                    offset += received;
                    _receivedCallback!(received);
                }

                // Read the stream ID from the frame data and return the stream ID and remaining data.
                (long streamId, int streamIdSize) = data.AsReadOnlySpan().ReadVarLong();

                return (streamId, data.Slice(streamIdSize), frameType == _lastStreamFrame.Span[0]);
            }
            else if (frameType == _resetStreamFrame.Span[0])
            {
                ArraySegment<byte> buffer = await _transceiver.ReceiveAsync(10).ConfigureAwait(false);
                long streamId = buffer.AsReadOnlySpan().ReadLong();
                ushort reason = buffer.Slice(8).AsReadOnlySpan().ReadUShort();
                _receivedCallback(11);
                return (streamId, ArraySegment<byte>.Empty, true);
            }
            else if (frameType == _closeFrame.Span[0])
            {
                ArraySegment<byte> buffer = await _transceiver.ReceiveAsync(2).ConfigureAwait(false);
                ushort reason = buffer.AsReadOnlySpan().ReadUShort();
                // TODO: read reason string
                _receivedCallback(3);
                throw new ConnectionClosedByPeerException();
            }
            else
            {
                throw new InvalidDataException($"unexpected Slic frame with frame type `{frameType}'");
            }
        }

        private Task PerformSendFrameAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();
            ValueTask sendTask = QueueAsync(buffer, cancel);
            _sendTask = sendTask.IsCompletedSuccessfully ? Task.CompletedTask : sendTask.AsTask();
            return _sendTask;

            async ValueTask QueueAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
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
                int sent = await _transceiver.SendAsync(buffer, CancellationToken.None).ConfigureAwait(false);
                Debug.Assert(sent == buffer.GetByteCount()); // TODO: do we need to support partial writes?
                _sentCallback!(sent);
            }
        }
    }
}
