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
        internal static readonly Encoding Encoding = Encoding.V2_0;
        private readonly int _frameSizeMax;
        private Action _heartbeatCallback;
        private readonly bool _incoming;
        private long _nextStreamId;
        private Action<int> _receivedCallback;
        private Task _sendTask = Task.CompletedTask;
        private Action<int> _sentCallback;
        private readonly BufferedReadTransceiver _transceiver;

        private enum FrameType : byte
        {
            Initialize,
            InitializeAck,
            Version,
            Ping,
            Pong,
            Stream,
            StreamLast,
            ResetStream,
            Close
        }

        private static readonly List<ArraySegment<byte>> _pingFrame = new List<ArraySegment<byte>> { new byte[]
            {
                (byte)FrameType.Ping, // Frame type (byte)
            }
        };

        private static readonly List<ArraySegment<byte>> _pongFrame = new List<ArraySegment<byte>> { new byte[]
            {
                (byte)FrameType.Pong, // Frame type (byte)
            }
        };

        private static readonly ReadOnlyMemory<byte> _resetStreamFrame = new byte[]
        {
            (byte)FrameType.ResetStream, // Frame type (0x08 and 0x09 for stream frame with FIN bit set)
            // stream ID (long)
            // error code (ushort)
        };

        private static readonly ReadOnlyMemory<byte> _closeFrame = new byte[]
        {
            (byte)FrameType.Close, // Frame type (0x08 and 0x09 for stream frame with FIN bit set)
            // Frame data length (varlong encoded on 4 bytes)
            // Frame data:
            // - error code (ushort)
            // - reason (string)
        };

        public async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            // Write the close connection frame.
            var data = new List<ArraySegment<byte>>();
            var ostr = new OutputStream(Encoding, data);
            ostr.WriteByte((byte)FrameType.Close);
            OutputStream.Position sizePos = ostr.StartFixedLengthSize();
            ostr.WriteShort(0); // TODO: error code?
            ostr.WriteString(exception.ToString()); // TODO: is this a good string for reason?
            ostr.EndFixedLengthSize(sizePos);
            data[^1] = data[^1].Slice(0, ostr.Tail.Offset); // TODO: Shouldn't this be the job of ostr.Finish()?

            await PerformSendFrameAsync(data, cancel).ConfigureAwait(false);

            // Notify the transport of the graceful connection closure.
            await _transceiver.CloseAsync(exception, cancel).ConfigureAwait(false);
        }

        public ValueTask DisposeAsync() => _transceiver.DisposeAsync();

        public async ValueTask HeartbeatAsync(CancellationToken cancel) =>
            await PerformSendFrameAsync(_pingFrame, cancel).ConfigureAwait(false);

        private async ValueTask SendFrameAsync(FrameType type, Action<OutputStream> writer, CancellationToken cancel)
        {
            var data = new List<ArraySegment<byte>>();
            var ostr = new OutputStream(Encoding, data);
            ostr.WriteByte((byte)type);
            OutputStream.Position sizePos = ostr.StartFixedLengthSize();
            writer(ostr);
            ostr.EndFixedLengthSize(sizePos);
            data[^1] = data[^1].Slice(0, ostr.Tail.Offset); // TODO: Shouldn't this be the job of ostr.Finish()?
            await _transceiver.SendAsync(data, cancel).ConfigureAwait(false);
        }

        private async ValueTask<(FrameType, ArraySegment<byte>)> ReceiveFrameAsync(CancellationToken cancel)
        {
            ArraySegment<byte> buffer = await _transceiver.ReceiveAsync(2, cancel).ConfigureAwait(false);
            var frameType = (FrameType)buffer[0];
            byte firstSizeByte = buffer[1];
            int sizeLength = firstSizeByte.ReadSizeLength20();
            buffer = await _transceiver.ReceiveAsync(sizeLength - 1, cancel).ConfigureAwait(false);
            _receivedCallback(sizeLength + 1);

            int frameSize = ComputeSize20(firstSizeByte, buffer.AsReadOnlySpan());

            ArraySegment<byte> frame = new byte[frameSize];
            await ReceiveAsync(frame).ConfigureAwait(false);
            return (frameType, frame);

            static int ComputeSize20(byte firstByte, ReadOnlySpan<byte> otherBytes)
            {
                Span<byte> buf = stackalloc byte[otherBytes.Length + 1];
                buf[0] = firstByte;
                otherBytes.CopyTo(buf[1..]);
                return ((ReadOnlySpan<byte>)buf).ReadSize20().Size;
            }
        }

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
                (FrameType type, ArraySegment<byte> data) = await ReceiveFrameAsync(cancel);
                if (type != FrameType.Initialize)
                {
                    throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
                }

                // Check that the Slic version is supported (we only support version 1 for now)
                var istr = new InputStream(data, Encoding);
                if (istr.ReadUShort() != 1)
                {
                    // If unsupported Slic version, we stop reading there and reply with a VERSION frame to provide
                    // the client the supported Slic versions.
                    await SendFrameAsync(FrameType.Version, ostr =>
                    {
                        ostr.WriteSequence(new ArraySegment<short>(new short[] { 1 }).AsReadOnlySpan());
                    }, cancel).ConfigureAwait(false);

                    (type, data) = await ReceiveFrameAsync(cancel);
                    if (type != FrameType.Initialize)
                    {
                        throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
                    }

                    istr = new InputStream(data, Encoding);
                    ushort version = istr.ReadUShort();
                    if (version != 1)
                    {
                        throw new InvalidDataException($"unsupported Slic version `{version}'");
                    }
                }

                string protocol = istr.ReadString();
                if (ProtocolExtensions.Parse(protocol) != Protocol.Ice2)
                {
                    throw new NotSupportedException($"Ice protocol `{protocol}' is not supported with Slic");
                }

                // Send back an INITIALIZE_ACK frame.
                await SendFrameAsync(FrameType.InitializeAck, istr =>
                {
                    // TODO: write properties?
                }, cancel).ConfigureAwait(false);
            }
            else
            {
                // Send the the INITIALIZE frame.
                await SendFrameAsync(FrameType.Initialize, ostr =>
                {
                    ostr.WriteUShort(1); // Slic V1
                    ostr.WriteString(Protocol.Ice2.GetName()); // Ice protocol name
                }, cancel).ConfigureAwait(false);

                // Read the INITIALIZE_ACK or VERSION frame from the server
                (FrameType type, ArraySegment<byte> data) = await ReceiveFrameAsync(cancel);

                // If we receive a VERSION frame, there isn't much we can do as we only support V1 so we throw
                // with an appropriate message to abort the connection.
                if (type == FrameType.Version)
                {
                    // Read the version sequence provided by the server.
                    short[] versions = new InputStream(data, Encoding).ReadArray<short>();
                    throw new InvalidDataException(
                        $"unsupported Slic version, server supports Slic `{string.Join(", ", versions)}'");
                }
                else if (type != FrameType.InitializeAck)
                {
                    throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
                }

                // TODO: read the properties of the INITIALIZE_ACK frame
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
                (long streamId, ArraySegment<byte> data, bool fin) =
                    await PerformReceiveFrameAsync().ConfigureAwait(false);
                if (streamId > 0)
                {
                    if (data.Count == 0)
                    {
                        // TODO: stream reset!
                    }
                    else
                    {
                        IncomingFrame? frame = ParseProtocolFrame(data);
                        if (frame != null)
                        {
                            return (StreamId: streamId, Frame: frame, Fin: fin);
                        }
                    }
                }
            }
        }

        public long NewStream(bool bidirectional) => bidirectional ? ++_nextStreamId : 0;

        public ValueTask ResetAsync(long streamId) => SendFrameAsync(FrameType.Close, ostr =>
            {
                ostr.WriteVarLong(streamId);
                // TODO: encode reason
            }, CancellationToken.None);

        public async ValueTask SendAsync(long streamId, OutgoingFrame frame, bool fin, CancellationToken cancel)
        {
            var data = new List<ArraySegment<byte>>();
            var ostr = new OutputStream(Encoding, data);
            ostr.WriteByte((byte)(fin ? FrameType.StreamLast : FrameType.Stream));
            OutputStream.Position sizePos = ostr.StartFixedLengthSize();
            ostr.WriteVarLong(streamId);
            if (frame is OutgoingRequestFrame requestFrame)
            {
                ostr.WriteByte((byte)Ice2Definitions.FrameType.Request);
                //ProtocolTrace.TraceFrame(Endpoint.Communicator, header, requestFrame);
            }
            else if (frame is OutgoingResponseFrame responseFrame)
            {
                ostr.WriteByte((byte)Ice2Definitions.FrameType.Response);
                //ProtocolTrace.TraceFrame(Endpoint.Communicator, header, responseFrame);
            }
            else
            {
                Debug.Assert(false);
                return;
            }
            ostr.WriteSize(frame.Size);
            ostr.EndFixedLengthSize(sizePos);
            data[^1] = data[^1].Slice(0, ostr.Tail.Offset); // TODO: Shouldn't this be the job of ostr.Finish()?
            data.AddRange(frame.Data);

            // TODO: split large protocol frames to allow multiplexing. For now, we send one Slic frame for each
            // Ice protocol frame.

            await PerformSendFrameAsync(data, cancel);
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

        private IncomingFrame? ParseProtocolFrame(ArraySegment<byte> data)
        {
            // The magic and version fields have already been checked.
            var frameType = (Ice2Definitions.FrameType)data[0];
            (int size, int sizeLength) = data.Slice(1).AsReadOnlySpan().ReadSize20();
            if (size > _frameSizeMax)
            {
                throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
            }

            // TODO: support receiving an Ice2 frame with multiple Slic frame, for now we only support one Slic frame
            // for each Ice2 protocol frame.
            ArraySegment<byte> buffer = data.Slice(sizeLength + 1);
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
            (FrameType type, ArraySegment<byte> data) =
            await ReceiveFrameAsync(CancellationToken.None).ConfigureAwait(false);
            switch (type)
            {
                case FrameType.Ping:
                {
                    // TODO: send pong frame
                    //ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, frame);
                    _heartbeatCallback();
                    return default;
                }
                case FrameType.Pong:
                {
                    //ProtocolTrace.TraceReceived(Endpoint.Communicator, Endpoint.Protocol, frame);
                    return default;
                }
                case FrameType.Stream:
                case FrameType.StreamLast:
                {
                    (long streamId, int streamIdSize) = data.AsReadOnlySpan().ReadVarLong();
                    return (streamId, data.Slice(streamIdSize), type == FrameType.StreamLast);
                }
                case FrameType.ResetStream:
                {
                    (long streamId, int streamIdSize) = data.AsReadOnlySpan().ReadVarLong();
                    ushort reason = data.Slice(streamIdSize).AsReadOnlySpan().ReadUShort();
                    return (streamId, ArraySegment<byte>.Empty, true);
                }
                case FrameType.Close:
                {
                    var istr = new InputStream(data, Encoding);
                    string reason = istr.ReadString();
                    ushort code = istr.ReadUShort();
                    throw new ConnectionClosedByPeerException();
                }
                default:
                    throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
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

        private async ValueTask ReceiveAsync(ArraySegment<byte> buffer)
        {
            int offset = 0;
            while (offset != buffer.Count)
            {
                int received = await _transceiver.ReceiveAsync(buffer.Slice(offset)).ConfigureAwait(false);
                offset += received;
                _receivedCallback!(received);
            }
        }
    }
}
