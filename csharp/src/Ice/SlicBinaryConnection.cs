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
            // Frame data length (uint)
            // Frame data: StreamID (varlong) - Protocol Data (bytes)
        };

        private static readonly ReadOnlyMemory<byte> _lastStreamFrame = new byte[]
        {
            0x07, // Frame type
            // Frame data length (uint)
            // Frame data: StreamID (varlong) - Protocol Data (bytes)
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

        public async ValueTask SendAsync(long streamId, OutgoingFrame frame, bool fin, CancellationToken cancel)
        {
            ArraySegment<byte> header = new byte[32];
            int pos = 0;
            header[pos++] = fin ? _lastStreamFrame.Span[0] : _streamFrame.Span[0];
            pos +=4 ; // Placeholder for Slic frame size (int)
            int slicFrameSize = header.Slice(pos).AsSpan().WriteVarLong(streamId);
            pos += slicFrameSize;
            if (frame is OutgoingRequestFrame requestFrame)
            {
                header[pos++] = (byte)Ice2Definitions.FrameType.Request;
                //ProtocolTrace.TraceFrame(Endpoint.Communicator, header, requestFrame);
            }
            else if (frame is OutgoingResponseFrame responseFrame)
            {
                header[pos++] = (byte)Ice2Definitions.FrameType.Response;
                //ProtocolTrace.TraceFrame(Endpoint.Communicator, header, responseFrame);
            }
            else
            {
                Debug.Assert(false);
                return;
            }
            slicFrameSize += header.Slice(pos + slicFrameSize).AsSpan().WriteVarLong(frame.Size);

            // Write the Slic frame size
            header.Slice(1).AsSpan().WriteInt(slicFrameSize);

            // TODO: split large protocol frames to allow multiplexing. For now, we send one Slic frame for each
            // Ice protocol frame.

            var buffer = new List<ArraySegment<byte>>() { header.Slice(0, 5 + slicFrameSize) };
            buffer.AddRange(frame.Data);
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

        private IncomingFrame? ParseProtocolFrame(ArraySegment<byte> buffer)
        {
            // The magic and version fields have already been checked.
            var frameType = (Ice2Definitions.FrameType)buffer[0];

            (long size, int sizeLength) = buffer.AsReadOnlySpan().ReadVarLong();
            if (size > _frameSizeMax)
            {
                throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
            }

            buffer = buffer.Slice(1 + sizeLength);

            // TODO: support receiving an Ice2 frame with multiple Slic frame, for now we only support one Slic frame
            // for each Ice2 protocol frame.
            Debug.Assert(size == buffer.Count);

            switch (frameType)
            {
                case Ice2Definitions.FrameType.Request:
                {
                    var request = new IncomingRequestFrame(Endpoint.Protocol, buffer, _frameSizeMax);
                    //ProtocolTrace.TraceFrame(Endpoint.Communicator, buffer, request);
                    return request;
                }

                case Ice2Definitions.FrameType.Response:
                {
                    var responseFrame = new IncomingResponseFrame(Endpoint.Protocol, buffer, _frameSizeMax);
                    //ProtocolTrace.TraceFrame(Endpoint.Communicator, buffer, responseFrame);
                    return responseFrame;
                }

                default:
                {
                    // ProtocolTrace.Trace(
                    //     "received unknown frame\n(invalid, closing connection)",
                    //     Endpoint.Communicator,
                    //     Endpoint.Protocol,
                    //     buffer);
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
