//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal class SlicBinaryConnection : BinaryConnection
    {
        internal enum FrameType : byte
        {
            Initialize = 0x01,
            InitializeAck = 0x02,
            Version = 0x03,
            Ping = 0x04,
            Pong = 0x05,
            Stream = 0x06,
            StreamLast = 0x07,
            ResetStream = 0x08,
            Close = 0x09
        }

        internal static readonly Encoding Encoding = Encoding.V2_0;

        // The mutex provides thread-safety for the _sendTask data member.
        private readonly object _mutex = new object();
        private long _nextStreamId;
        private Task _sendTask = Task.CompletedTask;
        private readonly BufferedReadTransceiver _transceiver;

        public override ValueTask DisposeAsync()
        {
            Closed();
            return _transceiver.DisposeAsync();
        }

    public override string ToString() => _transceiver.ToString()!;

        internal SlicBinaryConnection(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter) :
            base(endpoint, adapter) => _transceiver = new BufferedReadTransceiver(transceiver);

        internal override async ValueTask CloseAsync(Exception exception, CancellationToken cancel)
        {
            // Write the close connection frame.
            await PrepareAndSendFrameAsync(FrameType.Close, ostr =>
            {
                ostr.WriteVarLong(0);
#if DEBUG
                ostr.WriteString(exception.ToString());
#else
                ostr.WriteString(exception.Message);
#endif
            }, cancel);

            // Notify the transport of the graceful connection closure.
            await _transceiver.CloseAsync(exception, cancel).ConfigureAwait(false);
        }

        internal override ValueTask HeartbeatAsync(CancellationToken cancel) =>
            PrepareAndSendFrameAsync(FrameType.Ping, null, cancel);

        internal override async ValueTask InitializeAsync(Action heartbeatCallback, CancellationToken cancel)
        {
            HeartbeatCallback = heartbeatCallback;

            // Initialize the transport
            await _transceiver.InitializeAsync(cancel).ConfigureAwait(false);

            if (IsIncoming)
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
                    await PrepareAndSendFrameAsync(FrameType.Version, ostr =>
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
                    throw new NotSupportedException($"application protocol `{protocol}' is not supported with Slic");
                }

                // TODO: transport parameters

                // Send back an INITIALIZE_ACK frame.
                await PrepareAndSendFrameAsync(FrameType.InitializeAck, istr =>
                {
                    // TODO: transport parameters
                }, cancel).ConfigureAwait(false);
            }
            else
            {
                // Send the INITIALIZE frame.
                await PrepareAndSendFrameAsync(FrameType.Initialize, ostr =>
                {
                    ostr.WriteUShort(1); // Slic V1
                    ostr.WriteString(Protocol.Ice2.GetName()); // Ice protocol name
                    // TODO: transport parameters
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

                // TODO: transport parameters
            }

            if (Endpoint.Communicator.TraceLevels.Transport >= 1)
            {
                var s = new StringBuilder();
                s.Append(IsIncoming ? "accepted" : "established");
                s.Append(' ');
                s.Append(Endpoint.TransportName);
                s.Append(" connection\n");
                s.Append(ToString());
                Endpoint.Communicator.Logger.Trace(Endpoint.Communicator.TraceLevels.TransportCategory, s.ToString());
            }
        }

        internal override long NewStream(bool bidirectional) => bidirectional ? ++_nextStreamId : 0;

        internal override async ValueTask<(long StreamId, IncomingFrame? Frame, bool Fin)> ReceiveAsync(
            CancellationToken cancel)
        {
            FrameType type;
            ArraySegment<byte> data;
            while (true)
            {
                // Read Slic frame header
                (type, data) = await ReceiveFrameAsync(CancellationToken.None).ConfigureAwait(false);
                switch (type)
                {
                    case FrameType.Ping:
                    {
                        ValueTask task = PrepareAndSendFrameAsync(FrameType.Pong, null, CancellationToken.None);
                        HeartbeatCallback!();
                        break;
                    }
                    case FrameType.Pong:
                    {
                        // TODO: setup a timer to expect pong frame response?
                        break;
                    }
                    case FrameType.Stream:
                    case FrameType.StreamLast:
                    {
                        (long streamId, int streamIdSize) = data.AsReadOnlySpan().ReadVarLong();
                        data = data.Slice(streamIdSize);
                        IncomingFrame frame = ParseIce2Frame(data);
                        if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
                        {
                            ProtocolTrace.TraceFrame(Endpoint.Communicator, streamId, frame);
                        }
                        return (StreamId: streamId, Frame: frame, Fin: type == FrameType.StreamLast);
                    }
                    case FrameType.ResetStream:
                    {
                        var istr = new InputStream(data, Encoding);
                        long streamId = istr.ReadVarLong();
                        long reason = istr.ReadVarLong();
                        return (StreamId: streamId, Frame: null, Fin: true);
                    }
                    case FrameType.Close:
                    {
                        var istr = new InputStream(data, Encoding);
                        long code = istr.ReadVarLong(); // TODO: is this really useful?
                        string reason = istr.ReadString();
                        throw new ConnectionClosedByPeerException(reason);
                    }
                    default:
                    {
                        throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
                    }
                }
            }
        }

        // TODO: check that the stream is active. If it's not active, there's no need to send this frame.
        internal override ValueTask ResetAsync(long streamId) => PrepareAndSendFrameAsync(FrameType.ResetStream, ostr =>
            {
                ostr.WriteVarLong(streamId);
                ostr.WriteVarLong(0);
            }, CancellationToken.None);

        internal override async ValueTask SendAsync(
            long streamId,
            OutgoingFrame frame,
            bool fin,
            CancellationToken cancel)
        {
            var data = new List<ArraySegment<byte>>();
            var ostr = new OutputStream(Encoding, data);
            FrameType frameType = fin ? FrameType.StreamLast : FrameType.Stream;
            ostr.WriteByte((byte)frameType);
            OutputStream.Position sizePos = ostr.StartFixedLengthSize(4);
            ostr.WriteVarLong(streamId);
            OutputStream.Position ice2HeaderPos = ostr.Tail;
            if (frame is OutgoingRequestFrame requestFrame)
            {
                ostr.WriteByte((byte)Ice2Definitions.FrameType.Request);
            }
            else if (frame is OutgoingResponseFrame responseFrame)
            {
                ostr.WriteByte((byte)Ice2Definitions.FrameType.Response);
            }
            else
            {
                Debug.Assert(false);
                return;
            }
            ostr.WriteSize(frame.Size);
            int ice2HeaderSize = ostr.Tail.Offset - ice2HeaderPos.Offset;
            data[^1] = data[^1].Slice(0, ostr.Tail.Offset); // TODO: Shouldn't this be the job of ostr.Finish()?
            int frameSize = ostr.Tail.Offset - sizePos.Offset - 4 + frame.Size;
            ostr.RewriteFixedLengthSize20(frameSize, sizePos, 4);
            data.AddRange(frame.Data);

            // TODO: split large protocol frames to allow multiplexing. For now, we send one Slic frame for each
            // Ice protocol frame.

            if (Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                ProtocolTrace.TraceFrame(Endpoint.Communicator, streamId, frame);
            }

            if (Endpoint.Communicator.TraceLevels.Transport > 2)
            {
                TraceTransportFrame("sending ", frameType, frameSize, streamId);
            }

            await SendSlicFrameAsync(data, cancel).ConfigureAwait(false);
        }

        private IncomingFrame ParseIce2Frame(ArraySegment<byte> data)
        {
            // Get the Ice2 frame type and size
            var frameType = (Ice2Definitions.FrameType)data[0];
            (int size, int sizeLength) = data.Slice(1).AsReadOnlySpan().ReadSize20();
            if (size > IncomingFrameSizeMax)
            {
                throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
            }

            // TODO: support receiving an Ice2 frame with multiple Slic frame, for now we only support one Slic frame
            // for each Ice2 protocol frame.

            ArraySegment<byte> buffer = data.Slice(sizeLength + 1);
            if (size != buffer.Count)
            {
                throw new InvalidDataException($"frame with size {buffer.Count} doesn't match expected size {size}");
            }

            return frameType switch
            {
                Ice2Definitions.FrameType.Request =>
                    new IncomingRequestFrame(Endpoint.Protocol, buffer, IncomingFrameSizeMax),
                Ice2Definitions.FrameType.Response =>
                    new IncomingResponseFrame(Endpoint.Protocol, buffer, IncomingFrameSizeMax),
                _ => throw new InvalidDataException($"received ice2 frame with unknown frame type `{frameType}'")
            };
        }

        private async ValueTask PrepareAndSendFrameAsync(
            FrameType type,
            Action<OutputStream>? writer,
            CancellationToken cancel)
        {
            var data = new List<ArraySegment<byte>>();
            var ostr = new OutputStream(Encoding, data);
            ostr.WriteByte((byte)type);
            OutputStream.Position sizePos = ostr.StartFixedLengthSize(4);
            writer?.Invoke(ostr);
            int frameSize = ostr.Tail.Offset - sizePos.Offset - 4;
            ostr.EndFixedLengthSize(sizePos, 4);
            data[^1] = data[^1].Slice(0, ostr.Tail.Offset); // TODO: Shouldn't this be the job of ostr.Finish()?

            if (Endpoint.Communicator.TraceLevels.Transport > 2)
            {
                long streamId = 0;
                if (type == FrameType.ResetStream)
                {
                    streamId = data[0].Slice(sizePos.Offset + 4).AsReadOnlySpan().ReadVarLong().Value;
                }
                TraceTransportFrame("sending ", type, frameSize, streamId);
            }

            await SendSlicFrameAsync(data, cancel).ConfigureAwait(false);
        }

        private async ValueTask<(FrameType, ArraySegment<byte>)> ReceiveFrameAsync(CancellationToken cancel)
        {
            ReadOnlyMemory<byte> buffer = await _transceiver.ReceiveAsync(2, cancel).ConfigureAwait(false);
            var frameType = (FrameType)buffer.Span[0];
            byte firstSizeByte = buffer.Span[1];
            int sizeLength = firstSizeByte.ReadSizeLength20();

            _transceiver.Rewind(1);
            buffer = await _transceiver.ReceiveAsync(sizeLength, cancel).ConfigureAwait(false);
            Received(sizeLength + 1);

            int frameSize = buffer.Span.ReadSize20().Size;

            ArraySegment<byte> frame = new byte[frameSize];
            int offset = 0;
            while (offset != frameSize)
            {
                int received = await _transceiver.ReceiveAsync(frame.Slice(offset), cancel).ConfigureAwait(false);
                offset += received;
                Received(received);
            }

            if (Endpoint.Communicator.TraceLevels.Transport > 2)
            {
                long streamId = frameType switch
                {
                    FrameType.Stream => frame.AsReadOnlySpan().ReadVarLong().Value,
                    FrameType.StreamLast => frame.AsReadOnlySpan().ReadVarLong().Value,
                    FrameType.ResetStream => frame.AsReadOnlySpan().ReadVarLong().Value,
                    _ => 0,
                };
                TraceTransportFrame("received ", frameType, frameSize, streamId);
            }

            return (frameType, frame);
        }

        private Task SendSlicFrameAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();

            // Synchronization is required here because this might be called concurrently by the connection code
            lock (_mutex)
            {
                ValueTask sendTask = QueueAsync(buffer, cancel);
                _sendTask = sendTask.IsCompletedSuccessfully ? Task.CompletedTask : sendTask.AsTask();
                return _sendTask;
            }

            async ValueTask QueueAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
            {
                try
                {
                    // Wait for the previous send to complete
                    await _sendTask.ConfigureAwait(false);
                }
                catch (OperationCanceledException)
                {
                    // Ignore if it got canceled.
                }

                // If the send got cancelled, throw to notify the connection of the cancellation. This isn't a fatal
                // connection error, the next pending frame will be sent.
                cancel.ThrowIfCancellationRequested();

                // Perform the write
                int sent = await _transceiver.SendAsync(buffer, CancellationToken.None).ConfigureAwait(false);
                Debug.Assert(sent == buffer.GetByteCount());
                Sent(sent);
            }
        }

        private void TraceTransportFrame(string prefix, FrameType type, int size, long streamId = 0)
        {
            string frameType = "Slic " + type switch
            {
                FrameType.Initialize => "initialize",
                FrameType.InitializeAck => "initialize acknowledgment",
                FrameType.Version => "version",
                FrameType.Ping => "ping",
                FrameType.Pong => "pong",
                FrameType.Stream => "stream",
                FrameType.StreamLast => "last stream",
                FrameType.ResetStream => "reset stream",
                FrameType.Close => "close",
                _ => "unknown",
            } + " frame";

            var s = new StringBuilder();
            s.Append(prefix);
            s.Append(frameType);

            s.Append("\nprotocol = ");
            s.Append(Endpoint.Protocol.GetName());

            s.Append("\nframe size = ");
            s.Append(size);

            if (streamId > 0)
            {
                s.Append("\nstream id = ");
                s.Append(streamId);
            }

            Endpoint.Communicator.Logger.Trace(Endpoint.Communicator.TraceLevels.TransportCategory, s.ToString());
        }
    }
}
