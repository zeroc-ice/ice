// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Channels;

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
            StreamReset = 0x08,
            StreamUnidirectionalFin = 0x09,
        }

        // The header below is a sentinel header used to reserve space in the protocol frame to avoid
        // allocating again a byte buffer for the Slic header.
        internal static byte[] FrameHeader = new byte[]
        {
            0x05, // Frame type
            0x02, 0x04, 0x06, 0x08, // FrameSize (varint)
            0x03, 0x05, 0x07, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // Stream ID (varlong)
        };

        internal static readonly Encoding Encoding = Encoding.V20;
    }

    // The stream implementation for Slic. It implements IValueTaskSource<> directly instead of using
    // ManualResetValueTaskCompletionSource<T> to minimize the number of heap allocations.
    internal class SlicStream : SignaledTransceiverStream<(int, bool)>
    {
        protected override ReadOnlyMemory<byte> Header => SlicDefinitions.FrameHeader;
        private volatile Exception? _exception;
        private readonly SlicTransceiver _transceiver;
        private int _receivedOffset;
        private int _receivedSize;
        private bool _receivedEndOfStream;

        public override void Abort(Exception ex)
        {
            base.Abort(ex);
            _exception = ex;
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing)
            {
                if (IsSignaled && _exception == null)
                {
                    // If the stream is signaled and wasn't aborted, it was canceled or discarded. We get the
                    // information for the frame to receive in order to pass it back to the transceiver below.
                    ValueTask<(int, bool)> valueTask = WaitSignalAsync(CancellationToken.None);
                    Debug.Assert(valueTask.IsCompleted);
                    (_receivedSize, _receivedEndOfStream) = valueTask.Result;
                    _receivedOffset = 0;
                }

                // If there's still data pending to be receive for the stream, we notify the transceiver that
                // we're abandoning the reading. It will finish to read the stream's frame data in order to
                // continue receiving frames for other streams.
                if (_receivedOffset < _receivedSize)
                {
                    _transceiver.FinishedReceivedStreamData(Id, _receivedOffset, _receivedSize, _receivedEndOfStream);
                }

                if (IsIncoming && !IsBidirectional && !IsControl)
                {
                    Interlocked.Decrement(ref _transceiver.UnidirectionalStreamCount);
                    _transceiver.PrepareAndSendFrameAsync(SlicDefinitions.FrameType.StreamUnidirectionalFin, null);
                }
                else if (!IsIncoming && IsBidirectional && IsStarted)
                {
                    Debug.Assert(IsBidirectional);
                    _transceiver.BidirectionalStreamSemaphore!.Release();
                }
            }
        }

        protected override async ValueTask<bool> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel)
        {
            int offset = 0;
            while (offset < buffer.Count)
            {
                if (_receivedSize == _receivedOffset)
                {
                    // Wait to be signaled for the reception of a new stream frame for this stream.
                    (_receivedSize, _receivedEndOfStream) = await WaitSignalAsync(cancel).ConfigureAwait(false);
                    _receivedOffset = 0;
                    if (_receivedEndOfStream && offset + _receivedSize < buffer.Count)
                    {
                        throw new InvalidDataException("received less data than expected with last frame");
                    }
                }

                if (_exception != null)
                {
                    throw _exception;
                }

                // Read and append the received stream frame data into the given buffer.
                int size = Math.Min(_receivedSize - _receivedOffset, buffer.Slice(offset).Count);
                await _transceiver.ReceiveDataAsync(buffer.Slice(offset, size),
                                                    CancellationToken.None).ConfigureAwait(false);
                offset += size;
                _receivedOffset += size;

                // If we've consumed the whole Slic frame, notify the transceiver that it can start receiving
                // a new frame.
                if (_receivedOffset == _receivedSize)
                {
                    _transceiver.FinishedReceivedStreamData(Id, _receivedOffset, _receivedSize, _receivedEndOfStream);
                }
            }
            return _receivedEndOfStream;
        }

        protected override async ValueTask ResetAsync() =>
            await _transceiver.PrepareAndSendFrameAsync(SlicDefinitions.FrameType.StreamReset, ostr =>
                {
                    ostr.WriteVarLong(Id);
                    ostr.WriteVarLong(0); // TODO: reason code?
                    return Id;
                }, CancellationToken.None).ConfigureAwait(false);

        protected override async ValueTask SendAsync(
            IList<ArraySegment<byte>> buffer,
            bool fin,
            CancellationToken cancel)
        {
            // Ensure the caller reserved space for the Slic header by checking for the sentinel header.
            Debug.Assert(Header.Span.SequenceEqual(buffer[0].Slice(0, Header.Length)));

            int size = buffer.GetByteCount();

            int maxFrameSize = _transceiver.Options.PacketSize;
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
                        sendBuffer.Add(buffer[0].Slice(0, Header.Length));
                        sendSize += sendBuffer[0].Count;
                    }

                    bool lastBuffer = false;
                    for (int i = start.Segment; i < buffer.Count; ++i)
                    {
                        int segmentOffset = i == start.Segment ? start.Offset : 0;
                        if (sendSize + buffer[i].Slice(segmentOffset).Count > maxFrameSize)
                        {
                            sendBuffer.Add(buffer[i].Slice(segmentOffset, maxFrameSize - sendSize));
                            start = new OutputStream.Position(i, segmentOffset + sendBuffer[^1].Count);
                            sendSize += sendBuffer[^1].Count;
                            break;
                        }
                        else
                        {
                            sendBuffer.Add(buffer[i].Slice(segmentOffset));
                            sendSize += sendBuffer[^1].Count;
                            lastBuffer = i + 1 == buffer.Count;
                        }
                    }

                    offset += sendSize;
                    await SendFrameAsync(sendSize, lastBuffer && fin, sendBuffer).ConfigureAwait(false);
                }
            }
            else
            {
                await SendFrameAsync(size, fin, buffer).ConfigureAwait(false);
            }

            async Task SendFrameAsync(int frameSize, bool fin, IList<ArraySegment<byte>> buffer)
            {
                if (_exception != null)
                {
                    throw _exception;
                }
                else if (!IsStarted)
                {
                    Debug.Assert(!IsIncoming);

                    // Ensure we don't open more streams than the peer allows.
                    if (IsBidirectional)
                    {
                        await _transceiver.BidirectionalStreamSemaphore!.WaitAsync(cancel).ConfigureAwait(false);
                    }
                    else
                    {
                        await _transceiver.UnidirectionalStreamSemaphore!.WaitAsync(cancel).ConfigureAwait(false);
                    }

                    // Ensure we allocate and queue the first stream frame atomically to ensure the receiver won't
                    // receive stream frames with out-of-order stream IDs.
                    Task task;
                    lock (_transceiver.Mutex)
                    {
                        Id = _transceiver.AllocateId(IsBidirectional);
                        task = SendFrameAsync(frameSize, fin, buffer);
                    }
                    Debug.Assert(!IsControl);
                    await task.ConfigureAwait(false);
                    return;
                }

                // The given buffer includes space for the Slic header, we substract the header size from the given
                // frame size.
                Debug.Assert(frameSize > Header.Length);
                frameSize -= Header.Length;

                int sizeLength = OutputStream.GetVarLongLength(frameSize);
                int streamIdLength = OutputStream.GetVarLongLength(Id);
                frameSize += streamIdLength;

                SlicDefinitions.FrameType frameType =
                    fin ? SlicDefinitions.FrameType.StreamLast : SlicDefinitions.FrameType.Stream;

                // Write the Slic frame header (frameType - byte, frameSize - varint, streamId - varlong). Since
                // we might not need the full space reserved for the header, we modify the send buffer to ensure
                // the first element points at the start of the Slic header. We'll restore the send buffer once
                // the send is complete (it's important for the tracing code which might rely on the encoded
                // data).
                ArraySegment<byte> previous = buffer[0];
                ArraySegment<byte> headerData = buffer[0].Slice(Header.Length - sizeLength - streamIdLength - 1);
                headerData[0] = (byte)frameType;
                headerData.AsSpan(1, sizeLength).WriteFixedLengthSize20(frameSize);
                headerData.AsSpan(1 + sizeLength, streamIdLength).WriteFixedLengthVarLong(Id);
                buffer[0] = headerData;

                if (_transceiver.Endpoint.Communicator.TraceLevels.Transport > 2)
                {
                    _transceiver.TraceTransportFrame("sending ", frameType, frameSize, Id);
                }

                if (IsIncoming && fin)
                {
                    Debug.Assert(IsBidirectional);
                    Interlocked.Decrement(ref _transceiver.BidirectionalStreamCount);
                }

                try
                {
                    await _transceiver.SendFrameAsync(buffer, cancel).ConfigureAwait(false);
                }
                finally
                {
                    // Restore the original value of the send buffer.
                    buffer[0] = previous;
                }
            }
        }

        internal SlicStream(long streamId, SlicTransceiver transceiver)
            : base(streamId, transceiver) => _transceiver = transceiver;

        internal SlicStream(bool bidirectional, SlicTransceiver transceiver)
            : base(bidirectional, transceiver) => _transceiver = transceiver;

        internal void ReceivedFrame(int size, bool fin) =>
            // Ensure to run the continuation asynchronously in case the continuation ends up calling user-code.
            SignalCompletion((size, fin), runContinuationAsynchronously: true);
    }

    internal class SlicTransceiver : MultiStreamTransceiverWithUnderlyingTransceiver
    {
        internal int BidirectionalStreamCount;
        internal AsyncSemaphore? BidirectionalStreamSemaphore;
        internal TimeSpan IdleTimeout;
        internal readonly object Mutex = new object();
        internal SlicOptions Options { get; }
        internal int UnidirectionalStreamCount;
        internal AsyncSemaphore? UnidirectionalStreamSemaphore;

        private long _lastBidirectionalId;
        private long _lastUnidirectionalId;
        private long _nextBidirectionalId;
        private long _nextUnidirectionalId;
        private readonly ManualResetValueTaskCompletionSource<int> _receiveStreamCompletionTaskSource =
            new ManualResetValueTaskCompletionSource<int>();
        private Task _sendTask = Task.CompletedTask;
        private readonly BufferedReadTransceiver _transceiver;

        public override async ValueTask<TransceiverStream> AcceptStreamAsync(CancellationToken cancel)
        {
            // Wait for the previous stream data receive to complete.
            await WaitForReceivedStreamDataCompletionAsync(cancel).ConfigureAwait(false);

            while (true)
            {
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
                        _ = PrepareAndSendFrameAsync(SlicDefinitions.FrameType.Pong, null, CancellationToken.None);
                        ReceivedPing();
                        if (Endpoint.Communicator.TraceLevels.Transport > 2)
                        {
                            TraceTransportFrame("received ", type, size, streamId);
                        }
                        break;
                    }
                    case SlicDefinitions.FrameType.Pong:
                    {
                        // TODO: setup a timer to expect pong frame response?
                        if (size != 0)
                        {
                            throw new InvalidDataException("unexpected data for Slic Pong fame");
                        }
                        if (Endpoint.Communicator.TraceLevels.Transport > 2)
                        {
                            TraceTransportFrame("received ", type, size, streamId);
                        }
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
                            // Existing stream, notify the stream that data is available for read. We ensure that the
                            // continuation is ran asynchronously as otherwise this could end up blocking and it would
                            // prevent receiving further data.
                            stream.ReceivedFrame(size, type == SlicDefinitions.FrameType.StreamLast);

                            // Wait for the stream data receive to complete.
                            await WaitForReceivedStreamDataCompletionAsync(cancel).ConfigureAwait(false);
                        }
                        else if (isIncoming &&
                                 streamId.Value > (isBidirectional ? _lastBidirectionalId : _lastUnidirectionalId))
                        {
                            // Create a new stream if it's an incoming stream ID and if it's larger than the last
                            // known stream ID (the client could be sending frames for old canceled incoming streams).
                            if (isBidirectional)
                            {
                                if (BidirectionalStreamCount == Options.MaxBidirectionalStreams)
                                {
                                    throw new InvalidDataException(
                                        $"maximum bidirectional stream count {Options.MaxBidirectionalStreams} reached");
                                }
                                Interlocked.Increment(ref BidirectionalStreamCount);
                                _lastBidirectionalId = streamId.Value;
                            }
                            else if (streamId > 3)
                            {
                                if (UnidirectionalStreamCount == Options.MaxUnidirectionalStreams)
                                {
                                    throw new InvalidDataException(
                                        $"maximum unidirectional stream count {Options.MaxUnidirectionalStreams} reached");
                                }
                                Interlocked.Increment(ref UnidirectionalStreamCount);
                                _lastUnidirectionalId = streamId.Value;
                            }

                            // Accept the new incoming stream and notify that data is available for read.
                            stream = new SlicStream(streamId.Value, this);
                            stream.ReceivedFrame(size, type == SlicDefinitions.FrameType.StreamLast);
                            return stream;
                        }
                        else
                        {
                            // The stream has been disposed, read and ignore the data.
                            ArraySegment<byte> data = new byte[size];
                            await ReceiveDataAsync(data, cancel).ConfigureAwait(false);
                            if (Endpoint.Communicator.TraceLevels.Transport > 2)
                            {
                                TraceTransportFrame("received ", type, size, streamId);
                            }
                        }
                        break;
                    }
                    case SlicDefinitions.FrameType.StreamReset:
                    {
                        Debug.Assert(streamId != null);
                        ArraySegment<byte> data = new byte[size];
                        await ReceiveDataAsync(data, cancel).ConfigureAwait(false);
                        long reason = data.AsReadOnlySpan().ReadVarLong().Value; // TODO: do something with the reason code?
                        if (TryGetStream(streamId.Value, out SlicStream? stream))
                        {
                            stream.ReceivedReset();
                        }
                        if (Endpoint.Communicator.TraceLevels.Transport > 2)
                        {
                            TraceTransportFrame("received ", type, size, streamId);
                        }
                        break;
                    }
                    case SlicDefinitions.FrameType.StreamUnidirectionalFin:
                    {
                        if (Endpoint.Communicator.TraceLevels.Transport > 2)
                        {
                            TraceTransportFrame("received ", type, size, streamId);
                        }
                        UnidirectionalStreamSemaphore!.Release();
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

        public override TransceiverStream CreateStream(bool bidirectional) => new SlicStream(bidirectional, this);

        public override async ValueTask InitializeAsync(CancellationToken cancel)
        {
            // Initialize the underlying transport
            await _transceiver.InitializeAsync(cancel).ConfigureAwait(false);

            TimeSpan peerIdleTimeout;
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

                BidirectionalStreamSemaphore = new AsyncSemaphore(istr.ReadVarInt());
                UnidirectionalStreamSemaphore = new AsyncSemaphore(istr.ReadVarInt());
                peerIdleTimeout = TimeSpan.FromMilliseconds((double)istr.ReadVarLong());

                // Send back an INITIALIZE_ACK frame.
                await PrepareAndSendFrameAsync(SlicDefinitions.FrameType.InitializeAck, ostr =>
                {
                    ostr.WriteVarInt(Options.MaxBidirectionalStreams);
                    ostr.WriteVarInt(Options.MaxUnidirectionalStreams);
                    ostr.WriteVarLong((long)Options.IdleTimeout.TotalMilliseconds);
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
                    ostr.WriteVarInt(Options.MaxBidirectionalStreams);
                    ostr.WriteVarInt(Options.MaxUnidirectionalStreams);
                    ostr.WriteVarLong((long)Options.IdleTimeout.TotalMilliseconds);
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

                var istr = new InputStream(data, SlicDefinitions.Encoding);
                BidirectionalStreamSemaphore = new AsyncSemaphore(istr.ReadVarInt());
                UnidirectionalStreamSemaphore = new AsyncSemaphore(istr.ReadVarInt());
                peerIdleTimeout = TimeSpan.FromMilliseconds(istr.ReadVarLong());
            }

            // Use the smallest idle timeout.
            IdleTimeout = peerIdleTimeout < Options.IdleTimeout ? peerIdleTimeout : Options.IdleTimeout;
        }

        public override Task PingAsync(CancellationToken cancel) =>
            PrepareAndSendFrameAsync(SlicDefinitions.FrameType.Ping, null, cancel);

        internal SlicTransceiver(
            ITransceiver transceiver,
            Endpoint endpoint,
            ObjectAdapter? adapter)
            : base(endpoint, adapter, transceiver)
        {
            _transceiver = new BufferedReadTransceiver(transceiver);
            _receiveStreamCompletionTaskSource.SetResult(0);

            Options = new SlicOptions();
            if (adapter?.SerializeDispatch ?? false)
            {
                Options.MaxBidirectionalStreams = 1;
                Options.MaxUnidirectionalStreams = 1;
            }
            else
            {
                Options.MaxBidirectionalStreams = Endpoint.Communicator.SlicOptions.MaxBidirectionalStreams;
                Options.MaxUnidirectionalStreams = Endpoint.Communicator.SlicOptions.MaxUnidirectionalStreams;
            }
            Options.IdleTimeout = Endpoint.Communicator.SlicOptions.IdleTimeout;

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

        internal long AllocateId(bool bidirectional)
        {
            lock (Mutex)
            {
                long id;
                if (bidirectional)
                {
                    id = _nextBidirectionalId;
                    _nextBidirectionalId += 4;
                }
                else
                {
                    id = _nextUnidirectionalId;
                    _nextUnidirectionalId += 4;
                }
                return id;
            }
        }

        internal void FinishedReceivedStreamData(long streamId, int frameOffset, int frameSize, bool fin)
        {
            if (Endpoint.Communicator.TraceLevels.Transport > 2)
            {
                TraceTransportFrame("received ",
                                    fin ? SlicDefinitions.FrameType.StreamLast : SlicDefinitions.FrameType.Stream,
                                    frameSize,
                                    streamId);
            }
            _receiveStreamCompletionTaskSource.SetResult(frameSize - frameOffset);
        }

        internal Task PrepareAndSendFrameAsync(
            SlicDefinitions.FrameType type,
            Func<OutputStream, long>? writer,
            CancellationToken cancel = default)
        {
            var data = new List<ArraySegment<byte>>();
            var ostr = new OutputStream(SlicDefinitions.Encoding, data);
            ostr.WriteByte((byte)type);
            OutputStream.Position sizePos = ostr.StartFixedLengthSize(4);
            long? streamId = writer?.Invoke(ostr);
            int frameSize = ostr.Tail.Offset - sizePos.Offset - 4;
            ostr.EndFixedLengthSize(sizePos, 4);
            data[^1] = data[^1].Slice(0, ostr.Tail.Offset); // TODO: Shouldn't this be the job of ostr.Finish()?

            if (Endpoint.Communicator.TraceLevels.Transport > 2)
            {
                TraceTransportFrame("sending ", type, frameSize, streamId);
            }

            return SendFrameAsync(data, cancel);
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
                _receiveStreamCompletionTaskSource.TrySetException(exception);
                throw;
            }
        }

        internal Task SendFrameAsync(IList<ArraySegment<byte>> buffer, CancellationToken cancel)
        {
            cancel.ThrowIfCancellationRequested();

            // Synchronization is required here because this might be called concurrently by the connection code
            lock (Mutex)
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
                    _receiveStreamCompletionTaskSource.TrySetException(exception);
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
                SlicDefinitions.FrameType.StreamReset => "reset stream",
                SlicDefinitions.FrameType.StreamUnidirectionalFin => "unidirectional stream fin",
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

            s.Append('\n');
            s.Append(Underlying.ToString());

            Endpoint.Communicator.Logger.Trace(Endpoint.Communicator.TraceLevels.TransportCategory, s.ToString());
        }

        private async ValueTask<(SlicDefinitions.FrameType, ArraySegment<byte>)> ReceiveFrameAsync(
            CancellationToken cancel)
        {
            (SlicDefinitions.FrameType type, int size, long? streamId) =
                await ReceiveHeaderAsync(cancel).ConfigureAwait(false);
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
            if (Endpoint.Communicator.TraceLevels.Transport > 2)
            {
                TraceTransportFrame("received ", type, size, streamId);
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
            int size;
            if (sizeLength > 1)
            {
                _transceiver.Rewind(1);
                buffer = await _transceiver.ReceiveAsync(sizeLength, cancel).ConfigureAwait(false);
                size = buffer.Span.ReadSize20().Size;
            }
            else
            {
                size = buffer.Span[1..2].ReadSize20().Size;
            }

            // Receive the stream ID if the frame includes a stream ID. We receive at most 8 or size bytes and rewind
            // the transceiver buffered position if we read too much data.
            (long? streamId, int streamIdLength) = (null, 0);
            if (type == SlicDefinitions.FrameType.Stream ||
               type == SlicDefinitions.FrameType.StreamLast ||
               type == SlicDefinitions.FrameType.StreamReset)
            {
                int receiveSize = Math.Min(size, 8);
                buffer = await _transceiver.ReceiveAsync(receiveSize, cancel).ConfigureAwait(false);
                (streamId, streamIdLength) = buffer.Span.ReadVarLong();
                _transceiver.Rewind(receiveSize - streamIdLength);
            }

            Received(1 + sizeLength + streamIdLength);
            return (type, size - streamIdLength, streamId);
        }

        internal async ValueTask WaitForReceivedStreamDataCompletionAsync(CancellationToken cancel)
        {
            // If the stream didn't fully read the stream data, finish reading it here before returning.
            int size = await _receiveStreamCompletionTaskSource.ValueTask.WaitAsync(cancel).ConfigureAwait(false);
            if (size > 0)
            {
                ArraySegment<byte> data = new byte[size];
                await ReceiveDataAsync(data, cancel).ConfigureAwait(false);
            }
        }

        private protected override TransceiverStream CreateControlStream() =>
            // We make sure to allocate the ID for the control stream right away. Otherwise, it would be considered
            // like a regular stream and if serialization is enabled, it would acquire the semaphore.
            new SlicStream(AllocateId(false), this);
    }
}
