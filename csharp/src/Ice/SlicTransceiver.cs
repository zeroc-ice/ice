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
    internal static class SlicDefinitions
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
        }

        // The header below is a sentinel header used to reserve space in the protocol frame to avoid
        // allocating again a byte buffer for the Slic header.
        internal static byte[] FrameHeader = new byte[]
        {
            0x05, // Frame type
            0x02, 0x04, 0x06, 0x08, // FrameSize (varint)
            0x03, 0x05, 0x07, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // Stream ID (varlong)
        };

        internal static readonly Encoding Encoding = Encoding.V2_0;
    }

    // The stream implementation for Slic. It implements IValueTaskSource<> directly instead of using
    // ManualResetValueTaskCompletionSource<T> to minimize the number of heap allocations.
    internal class SlicStream : Stream, IValueTaskSource<(int, bool)>
    {
        protected override ReadOnlyMemory<byte> Header => SlicDefinitions.FrameHeader;
        private volatile Exception? _exception;
        private ManualResetValueTaskSourceCore<(int, bool)> _source;
        private readonly SlicTransceiver _transceiver;
        private int _receivedSize;
        private bool _receivedEndOfStream;

        public override void Abort(Exception ex)
        {
            _exception = ex;

            // Cancel waiting ReceiveAsync
            try
            {
                _source.SetException(ex);
            }
            catch
            {
            }
        }

        protected override async ValueTask<bool> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel)
        {
            int offset = 0;
            try
            {
                while (offset < buffer.Count)
                {
                    if (_receivedSize == 0)
                    {
                        // Wait to be signaled for the reception of a new stream frame for this stream.
                        var task = new ValueTask<(int, bool)>(this, _source.Version);
                        (_receivedSize, _receivedEndOfStream) = await task.WaitAsync(cancel).ConfigureAwait(false);
                        if (_receivedSize == 0)
                        {
                            _exception ??= new StreamResetByPeerException("received reset frame");
                        }
                    }

                    if (_exception != null)
                    {
                        throw _exception;
                    }
                    else if (_receivedEndOfStream && offset + _receivedSize < buffer.Count)
                    {
                        throw new InvalidDataException("received less data than expected with last frame");
                    }

                    cancel.ThrowIfCancellationRequested();

                    // Read and append the received stream frame data into the given buffer.
                    int size = Math.Min(_receivedSize, buffer.Slice(offset).Count);
                    await _transceiver.ReceiveDataAsync(buffer.Slice(offset, size),
                                                        CancellationToken.None).ConfigureAwait(false);
                    offset += size;
                    _receivedSize -= size;

                    // If we've consumed the whole Slic frame, notify the transceiver that it can start receiving
                    // a new frame.
                    if (_receivedSize == 0)
                    {
                        _transceiver.FinishedReceivedData();
                    }
                }
            }
            catch (OperationCanceledException)
            {
                // Even if the caller cancelled the receive we still need to read the frame in order for
                // the transceiver to start reading the next Slic frame.
                if (_receivedSize > 0)
                {
                    ArraySegment<byte> data = new byte[_receivedSize];
                    await _transceiver.ReceiveDataAsync(data, CancellationToken.None).ConfigureAwait(false);
                    _receivedSize = 0;
                    _transceiver.FinishedReceivedData();
                }
                throw;
            }
            return _receivedEndOfStream;
        }

        protected override async ValueTask ResetAsync() =>
            await _transceiver.PrepareAndSendFrameAsync(SlicDefinitions.FrameType.ResetStream, ostr =>
                {
                    ostr.WriteVarLong(Id);
                    ostr.WriteVarLong(0); // TODO: reason code?
                            return 0;
                }, CancellationToken.None).ConfigureAwait(false);

        protected override async ValueTask SendAsync(
            IList<ArraySegment<byte>> buffer,
            bool fin,
            CancellationToken cancel)
        {
            // Ensure the caller reserved space for the Slic header by checking for sentinel header.
            Debug.Assert(Header.Span.SequenceEqual(buffer[0].Slice(0, Header.Length)));

            // The given buffer is supposed to include space for the Slic header in the first segment.
            int size = buffer.GetByteCount() - Header.Length;

            int maxFrameSize = 10 * 1024;
            if (size > maxFrameSize)
            {
                var sendBuffer = new List<ArraySegment<byte>>(buffer.Count);
                int offset = 0;
                var start = new OutputStream.Position();
                while (offset < size)
                {
                    int sendSize = 0;
                    sendBuffer.Clear();

                    if (offset > 0)
                    {
                        sendBuffer[0] = buffer[0].Slice(0, Header.Length);
                        sendSize += sendBuffer[0].Count;
                    }

                    for (int i = start.Segment; i < buffer.Count; ++i)
                    {
                        int segmentOffset = i == start.Segment ? start.Offset : 0;
                        if (sendSize + buffer[i].Count > maxFrameSize)
                        {
                            sendBuffer.Add(buffer[i].Slice(segmentOffset, maxFrameSize - sendSize));
                            start = new OutputStream.Position(i, segmentOffset + sendBuffer[^1].Count);
                            offset += sendBuffer[^1].Count;
                            break;
                        }
                        else
                        {
                            sendBuffer.Add(buffer[i]);
                            sendSize += buffer[i].Count;
                        }
                    }

                    offset += sendSize;
                    await SendFrameAsync(size, sendBuffer).ConfigureAwait(false);
                }
            }
            else
            {
                await SendFrameAsync(size, buffer).ConfigureAwait(false);
            }

            async ValueTask SendFrameAsync(int frameSize, IList<ArraySegment<byte>> buffer)
            {
                if (_exception != null)
                {
                    throw _exception;
                }

                int sizeLength = OutputStream.GetVarLongLength(frameSize);
                int streamIdLength = OutputStream.GetVarLongLength(Id);

                SlicDefinitions.FrameType frameType =
                    fin ? SlicDefinitions.FrameType.StreamLast : SlicDefinitions.FrameType.Stream;

                // Write the Slic frame header (frameType - byte, frameSize - varint, streamId - varlong)
                ArraySegment<byte> headerData = buffer[0].Slice(Header.Length - sizeLength - streamIdLength - 1);
                headerData[0] = (byte)frameType;
                headerData.AsSpan(1, sizeLength).WriteFixedLengthSize20(frameSize);
                headerData.AsSpan(1 + sizeLength, streamIdLength).WriteFixedLengthVarLong(Id);
                buffer[0] = headerData;

                if (_transceiver.Endpoint.Communicator.TraceLevels.Transport > 2)
                {
                    _transceiver.TraceTransportFrame("sending ", frameType, frameSize, Id);
                }

                await _transceiver.SendFrameAsync(buffer, cancel).ConfigureAwait(false);
            }
        }

        (int, bool) IValueTaskSource<(int, bool)>.GetResult(short token)
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

        ValueTaskSourceStatus IValueTaskSource<(int, bool)>.GetStatus(short token) => _source.GetStatus(token);

        void IValueTaskSource<(int, bool)>.OnCompleted(
            Action<object?> continuation,
            object? state,
            short token,
            ValueTaskSourceOnCompletedFlags flags) => _source.OnCompleted(continuation, state, token, flags);

        internal SlicStream(long streamId, SlicTransceiver transceiver) : base(streamId, transceiver) =>
            _transceiver = transceiver;

        internal void ReceivedFrame(int size, bool fin) => _source.SetResult((size, fin));
    }

    internal class SlicTransceiver : MultiStreamTransceiverWithUnderlyingTransceiver
    {
        private readonly ManualResetValueTaskCompletionSource<bool> _acceptStreamTaskSource =
            new ManualResetValueTaskCompletionSource<bool>();
        private readonly object _mutex = new object();
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;
        private long _lastBidirectionalId;
        private long _lastUnidirectionalId;
        private Task _sendTask = Task.CompletedTask;
        private readonly BufferedReadTransceiver _transceiver;

        public override async ValueTask<Stream> AcceptStreamAsync(CancellationToken cancel)
        {
            while (true)
            {
                // If a frame is being read, wait for it to be read before starting reading a new frame,
                // _acceptStreamTaskSource is signaled once the stream frame is fully read.
                await _acceptStreamTaskSource.ValueTask.ConfigureAwait(false);

                // Read the Slic frame header
                (SlicDefinitions.FrameType type, int size, long? streamId) =
                    await ReceiveHeaderAsync(cancel).ConfigureAwait(false);

                switch (type)
                {
                    case SlicDefinitions.FrameType.Ping:
                    {
                        if (size != 0)
                        {
                            throw new InvalidDataException("unexpected data for Slic Ping fame");
                        }
                        ValueTask _ = PrepareAndSendFrameAsync(SlicDefinitions.FrameType.Pong,
                                                               null,
                                                               CancellationToken.None);
                        PingReceived();
                        _acceptStreamTaskSource.SetResult(false);
                        break;
                    }
                    case SlicDefinitions.FrameType.Pong:
                    {
                        // TODO: setup a timer to expect pong frame response?
                        if (size != 0)
                        {
                            throw new InvalidDataException("unexpected data for Slic Pong fame");
                        }
                        _acceptStreamTaskSource.SetResult(false);
                        break;
                    }
                    case SlicDefinitions.FrameType.Stream:
                    case SlicDefinitions.FrameType.StreamLast:
                    {
                        Debug.Assert(streamId != null);
                        bool isIncoming = streamId.Value % 2 == (IsIncoming ? 0 : 1);
                        bool isBidirectional = streamId.Value % 4 < 2;
                        if (size == 0 && type == SlicDefinitions.FrameType.Stream)
                        {
                            throw new InvalidDataException("received empty stream frame");
                        }

                        if (TryGetStream(streamId.Value, out SlicStream? stream))
                        {
                            // Existing stream, notify the stream that data is available for read.
                            stream.ReceivedFrame(size, type == SlicDefinitions.FrameType.StreamLast);
                        }
                        else if (isIncoming &&
                                 streamId.Value > (isBidirectional ? _lastBidirectionalId : _lastUnidirectionalId))
                        {
                            if (isBidirectional)
                            {
                                _lastBidirectionalId = streamId.Value;
                            }
                            else
                            {
                                _lastUnidirectionalId = streamId.Value;
                            }

                            // Accept the new incoming stream and notify that data is available for read.
                            stream = new SlicStream(streamId.Value, this);
                            stream.ReceivedFrame(size, type == SlicDefinitions.FrameType.StreamLast);
                            return stream;
                        }
                        else
                        {
                            // The stream has probably been disposed, read and ignore the data.
                            ArraySegment<byte> data = new byte[size];
                            await ReceiveDataAsync(data, cancel).ConfigureAwait(false);
                            _acceptStreamTaskSource.SetResult(false);
                        }
                        break;
                    }
                    case SlicDefinitions.FrameType.ResetStream:
                    {
                        Debug.Assert(streamId != null);
                        ArraySegment<byte> data = new byte[size];
                        await ReceiveDataAsync(data, cancel).ConfigureAwait(false);
                        long reason = data.AsReadOnlySpan().ReadVarLong().Value; // TODO: reason code?
                        if (TryGetStream(streamId.Value, out SlicStream? stream))
                        {
                            stream.ReceivedFrame(0, true);
                        }
                        _acceptStreamTaskSource.SetResult(false);
                        break;
                    }
                    default:
                    {
                        throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
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
                    stream = new SlicStream(_nextBidirectionalId, this);
                    _nextBidirectionalId += 4;
                }
                else
                {
                    stream = new SlicStream(_nextUnidirectionalId, this);
                    _nextUnidirectionalId += 4;
                }
                return stream;
            }
        }

        public override async ValueTask InitializeAsync(CancellationToken cancel)
        {
            // Initialize the transport
            await _transceiver.InitializeAsync(cancel).ConfigureAwait(false);

            if (IsIncoming)
            {
                (SlicDefinitions.FrameType type, ArraySegment<byte> data) = await ReceiveFrameAsync(cancel);
                if (type != SlicDefinitions.FrameType.Initialize)
                {
                    throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
                }

                // Check that the Slic version is supported (we only support version 1 for now)
                var istr = new InputStream(data, SlicDefinitions.Encoding);
                if (istr.ReadUShort() != 1)
                {
                    // If unsupported Slic version, we stop reading there and reply with a VERSION frame to provide
                    // the client the supported Slic versions.
                    await PrepareAndSendFrameAsync(SlicDefinitions.FrameType.Version, ostr =>
                    {
                        ostr.WriteSequence(new ArraySegment<short>(new short[] { 1 }).AsReadOnlySpan());
                        return 0;
                    }, cancel).ConfigureAwait(false);

                    (type, data) = await ReceiveFrameAsync(cancel);
                    if (type != SlicDefinitions.FrameType.Initialize)
                    {
                        throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
                    }

                    istr = new InputStream(data, SlicDefinitions.Encoding);
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
                await PrepareAndSendFrameAsync(SlicDefinitions.FrameType.InitializeAck, istr =>
                {
                    // TODO: transport parameters
                    return 0;
                }, cancel).ConfigureAwait(false);
            }
            else
            {
                // Send the INITIALIZE frame.
                await PrepareAndSendFrameAsync(SlicDefinitions.FrameType.Initialize, ostr =>
                {
                    ostr.WriteUShort(1); // Slic V1
                    ostr.WriteString(Protocol.Ice2.GetName()); // Ice protocol name
                    // TODO: transport parameters
                    return 0;
                }, cancel).ConfigureAwait(false);

                // Read the INITIALIZE_ACK or VERSION frame from the server
                (SlicDefinitions.FrameType type, ArraySegment<byte> data) = await ReceiveFrameAsync(cancel);

                // If we receive a VERSION frame, there isn't much we can do as we only support V1 so we throw
                // with an appropriate message to abort the connection.
                if (type == SlicDefinitions.FrameType.Version)
                {
                    // Read the version sequence provided by the server.
                    short[] versions = new InputStream(data, SlicDefinitions.Encoding).ReadArray<short>();
                    throw new InvalidDataException(
                        $"unsupported Slic version, server supports Slic `{string.Join(", ", versions)}'");
                }
                else if (type != SlicDefinitions.FrameType.InitializeAck)
                {
                    throw new InvalidDataException($"unexpected Slic frame with frame type `{type}'");
                }

                // TODO: transport parameters
            }
        }

        public override ValueTask PingAsync(CancellationToken cancel) =>
            PrepareAndSendFrameAsync(SlicDefinitions.FrameType.Ping, null, cancel);

        public override string ToString() => _transceiver.ToString()!;

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _transceiver.Dispose();
            }
        }

        internal SlicTransceiver(ITransceiver transceiver, Endpoint endpoint, ObjectAdapter? adapter) :
            base(endpoint, adapter, transceiver)
        {
            _transceiver = new BufferedReadTransceiver(transceiver);
            _acceptStreamTaskSource.SetResult(false);

            // We use the same stream ID numbering scheme as Quic
            if (IsIncoming)
            {
                _nextBidirectionalId = 1;
                _nextUnidirectionalId = 3;
            }
            else
            {
                _nextBidirectionalId = 0;
                _nextUnidirectionalId = 2;
            }
            _lastBidirectionalId = -1;
            _lastUnidirectionalId = -1;
        }

        internal void FinishedReceivedData() =>
            // AcceptStreamAsync is blocked on _readTaskSource, unblock it to allow to read a new Slic frame now that
            // we are done reading the stream data.
            _acceptStreamTaskSource.SetResult(false);

        internal async ValueTask PrepareAndSendFrameAsync(
            SlicDefinitions.FrameType type,
            Func<OutputStream, long>? writer,
            CancellationToken cancel = default)
        {
            var data = new List<ArraySegment<byte>>();
            var ostr = new OutputStream(SlicDefinitions.Encoding, data);
            ostr.WriteByte((byte)type);
            OutputStream.Position sizePos = ostr.StartFixedLengthSize(4);
            long? streamId = writer?.Invoke(ostr) ?? null;
            int frameSize = ostr.Tail.Offset - sizePos.Offset - 4;
            ostr.EndFixedLengthSize(sizePos, 4);
            data[^1] = data[^1].Slice(0, ostr.Tail.Offset); // TODO: Shouldn't this be the job of ostr.Finish()?

            if (Endpoint.Communicator.TraceLevels.Transport > 2)
            {
                TraceTransportFrame("sending ", type, frameSize, streamId);
            }

            await SendFrameAsync(data, cancel).ConfigureAwait(false);
        }

        internal async ValueTask ReceiveDataAsync(ArraySegment<byte> buffer, CancellationToken cancel)
        {
            try
            {
                int offset = 0;
                while (offset != buffer.Count)
                {
                    int received = await _transceiver.ReceiveAsync(buffer.Slice(offset), cancel).ConfigureAwait(false);
                    offset += received;
                    Received(received);
                }
            }
            catch (Exception exception)
            {
                _acceptStreamTaskSource.TrySetException(exception);
                throw;
            }
        }

        internal Task SendFrameAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
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
                    await _sendTask.WaitAsync(cancel).ConfigureAwait(false);
                }
                catch (OperationCanceledException)
                {
                    // Ignore if it got canceled.
                }

                // If the send got cancelled, throw to notify the stream of the cancellation. This isn't a fatal
                // connection error, the next pending frame will be sent.
                cancel.ThrowIfCancellationRequested();

                // Perform the write
                try
                {
                    int sent = await _transceiver.SendAsync(buffer, CancellationToken.None).ConfigureAwait(false);
                    Debug.Assert(sent == buffer.GetByteCount());
                    Sent(sent);
                }
                catch (Exception exception)
                {
                    _acceptStreamTaskSource.TrySetException(exception);
                    throw;
                }
            }
        }

        internal void TraceTransportFrame(string prefix, SlicDefinitions.FrameType type, int size, long? streamId)
        {
            string frameType = "Slic " + type switch
            {
                SlicDefinitions.FrameType.Initialize => "initialize",
                SlicDefinitions.FrameType.InitializeAck => "initialize acknowledgment",
                SlicDefinitions.FrameType.Version => "version",
                SlicDefinitions.FrameType.Ping => "ping",
                SlicDefinitions.FrameType.Pong => "pong",
                SlicDefinitions.FrameType.Stream => "stream",
                SlicDefinitions.FrameType.StreamLast => "last stream",
                SlicDefinitions.FrameType.ResetStream => "reset stream",
                _ => "unknown",
            } + " frame";

            var s = new StringBuilder();
            s.Append(prefix);
            s.Append(frameType);

            s.Append("\nprotocol = ");
            s.Append(Endpoint.Protocol.GetName());

            s.Append("\nframe size = ");
            s.Append(size);

            if (streamId != null)
            {
                s.Append("\nstream ID = ");
                s.Append(streamId);
            }

            Endpoint.Communicator.Logger.Trace(Endpoint.Communicator.TraceLevels.TransportCategory, s.ToString());
        }

        private async ValueTask<(SlicDefinitions.FrameType, ArraySegment<byte>)> ReceiveFrameAsync(
            CancellationToken cancel)
        {
            (SlicDefinitions.FrameType type, int size, long? _) = await ReceiveHeaderAsync(cancel).ConfigureAwait(false);
            ArraySegment<byte> data;
            if (size > 0)
            {
                data = new byte[size];
                await ReceiveDataAsync(data, cancel).ConfigureAwait(false);
            }
            else
            {
                data = ArraySegment<byte>.Empty;
            }
            return (type, data);
        }

        private async ValueTask<(SlicDefinitions.FrameType, int, long?)> ReceiveHeaderAsync(CancellationToken cancel)
        {
            // Receive at most 2 bytes for the Slic header (the minimum size of a Slic header). The first byte
            // will be the frame type and the second is the first byte of the Slic frame size.
            ReadOnlyMemory<byte> buffer = await _transceiver.ReceiveAsync(2, cancel).ConfigureAwait(false);
            var type = (SlicDefinitions.FrameType)buffer.Span[0];
            int sizeLength = buffer.Span[1].ReadSizeLength20();
            _transceiver.Rewind(1);

            bool hasStreamId = type == SlicDefinitions.FrameType.Stream ||
                               type == SlicDefinitions.FrameType.StreamLast ||
                               type == SlicDefinitions.FrameType.ResetStream;

            // Read the varuint Slic frame size and if the frame contains a stream ID, we also read the first byte
            // of the stream ID varlong.
            buffer = await _transceiver.ReceiveAsync(sizeLength + (hasStreamId ? 1 : 0), cancel).ConfigureAwait(false);
            int size = buffer.Span.ReadSize20().Size;

            // If the frame contains a stream ID, now read the var long stream ID.
            (long? streamId, int streamIdLength) = (null, 0);
            if (hasStreamId)
            {
                streamIdLength = buffer.Span[sizeLength].ReadVarLongLength();
                _transceiver.Rewind(1);

                // Read the varlong stream ID
                buffer = await _transceiver.ReceiveAsync(streamIdLength, cancel).ConfigureAwait(false);
                streamId = buffer.Span.ReadVarLong().Value;
            }

            Received(1 + sizeLength + streamIdLength);

            if (Endpoint.Communicator.TraceLevels.Transport > 2)
            {
                TraceTransportFrame("received ", type, size, streamId);
            }
            return (type, size, streamId);
        }
    }
}
