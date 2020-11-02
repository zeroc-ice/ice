// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Slic;

namespace ZeroC.Ice
{
    /// <summary>The stream implementation for Slic.</summary>
    internal class SlicStream : SignaledTransceiverStream<(int, bool)>
    {
        protected override ReadOnlyMemory<byte> Header => SlicDefinitions.FrameHeader;
        private readonly SlicTransceiver _transceiver;
        private int _receivedOffset;
        private int _receivedSize;
        private bool _receivedEndOfStream;
        private int _flowControlCreditReleased;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing)
            {
                if (IsSignaled)
                {
                    // If the stream is signaled, it was canceled or discarded. We get the information for the frame
                    // to receive in order to consume it below.
                    try
                    {
                        ValueTask<(int, bool)> valueTask = WaitSignalAsync(CancellationToken.None);
                        Debug.Assert(valueTask.IsCompleted);
                        (_receivedSize, _receivedEndOfStream) = valueTask.Result;
                        _receivedOffset = 0;
                    }
                    catch
                    {
                        // Ignore, the stream got aborted and there's nothing to consume.
                    }
                }

                // If there's still data pending to be receive for the stream, we notify the transceiver that
                // we're abandoning the reading. It will finish to read the stream's frame data in order to
                // continue receiving frames for other streams.
                if (_receivedOffset < _receivedSize)
                {
                    _transceiver.FinishedReceivedStreamData(Id, _receivedOffset, _receivedSize, _receivedEndOfStream);
                }

                // Only release incoming streams on Dispose. Slic outgoing streams are released when the StreamLast
                // frame is received and it can be received after the stream is disposed (e.g.: for oneway requests
                // we dispose of the stream as soon as the request is sent and before receiving the StreamLast frame).
                if (IsIncoming)
                {
                    ReleaseFlowControlCredit(notifyPeer: true);
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
                        throw new InvalidDataException("received last frame with less data than expected");
                    }
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

        protected override async ValueTask ResetAsync(long errorCode) =>
            await _transceiver.PrepareAndSendFrameAsync(
                SlicDefinitions.FrameType.StreamReset,
                ostr =>
                {
                    checked
                    {
                        new StreamResetBody((ulong)errorCode).IceWrite(ostr);
                    }
                },
                Id).ConfigureAwait(false);

        protected override async ValueTask SendAsync(
            IList<ArraySegment<byte>> buffer,
            bool fin,
            CancellationToken cancel)
        {
            // Ensure the caller reserved space for the Slic header by checking for the sentinel header.
            Debug.Assert(Header.Span.SequenceEqual(buffer[0].Slice(0, Header.Length)));

            int size = buffer.GetByteCount();

            // If the protocol buffer is larger than the configure Slic packet size, send it over multiple Slic packets.
            int maxFrameSize = _transceiver.Options.PacketSize;
            if (size > maxFrameSize)
            {
                // The send buffer for the Slic packet.
                var sendBuffer = new List<ArraySegment<byte>>(buffer.Count);

                // The amount of data sent so far.
                int offset = 0;

                // The position of the data to send next.
                var start = new OutputStream.Position();

                while (offset < size)
                {
                    sendBuffer.Clear();

                    int sendSize = 0;
                    if (offset > 0)
                    {
                        // If it's not the first Slic packet, we re-use the space reserved for Slic header in the first
                        // segment of the protocol buffer.
                        sendBuffer.Add(buffer[0].Slice(0, Header.Length));
                        sendSize += sendBuffer[0].Count;
                    }

                    // Append data until we reach the Slic packet size or the end of the protocol buffer.
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

                    // Send the Slic packet.
                    offset += sendSize;
                    await PerformSendFrameAsync(sendSize, lastBuffer && fin, sendBuffer).ConfigureAwait(false);
                }
            }
            else
            {
                // If the protocol buffer is small enough to fit in a single Slic packet, send it directly.
                await PerformSendFrameAsync(size, fin, buffer).ConfigureAwait(false);
            }

            // Send a Slic packet. The first segment of the given buffer always contain space reserved for the Slic
            // header.
            async Task PerformSendFrameAsync(int frameSize, bool fin, IList<ArraySegment<byte>> buffer)
            {
                if (!IsStarted)
                {
                    Debug.Assert(!IsIncoming);

                    // If the outgoing stream isn't started, it's the first send call. We need to acquire flow
                    // control credit to ensure we don't open more streams than the peer allows. The wait on the
                    // semaphore provides FIFO guarantee to ensure that the sending of requests is serialized.
                    Debug.Assert(!IsIncoming);
                    if (IsBidirectional)
                    {
                        await _transceiver.BidirectionalStreamSemaphore!.WaitAsync(cancel).ConfigureAwait(false);
                    }
                    else
                    {
                        await _transceiver.UnidirectionalStreamSemaphore!.WaitAsync(cancel).ConfigureAwait(false);
                    }

                    // Ensure we allocate and queue the first stream frame atomically to ensure the receiver won't
                    // receive stream frames with out-of-order stream IDs. The ID assignment will throw if the
                    // connection is being closed and no new streams can be created.
                    Task task;
                    lock (_transceiver.Mutex)
                    {
                        Id = _transceiver.AllocateId(IsBidirectional);
                        task = PerformSendFrameAsync(frameSize, fin, buffer);
                    }
                    Debug.Assert(!IsControl);
                    await task.ConfigureAwait(false);
                    return;
                }

                // The incoming bidirectional stream is considered completed once no more data will be written on
                // the stream. It's important to release the flow control credit here before the peer receives the
                // last stream frame to prevent a race where the peer could start a new stream before the credit
                // counter is released. If the credit is already released, this indicates that got reset. In this
                // case, we return since an empty stream last frame has been sent already.
                if (IsIncoming && fin && !ReleaseFlowControlCredit())
                {
                    return;
                }

                // The given buffer includes space for the Slic header, we subtract the header size from the given
                // frame size.
                Debug.Assert(frameSize > Header.Length);
                frameSize -= Header.Length;

                // Compute how much space the size and stream ID require to figure out the start of the Slic header.
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

                try
                {
                    await _transceiver.SendFrameAsync(buffer, cancel).ConfigureAwait(false);
                }
                finally
                {
                    buffer[0] = previous; // Restore the original value of the send buffer.
                }
            }
        }

        internal SlicStream(long streamId, SlicTransceiver transceiver)
            : base(streamId, transceiver)
        {
            _transceiver = transceiver;

            if (IsIncoming && !IsControl)
            {
                // Increment the counter to ensure the peer doesn't open more streams than allowed. This can't be
                // called concurrently for a given connection so if it's fine to check the counter and increment
                // it after.
                if (IsBidirectional)
                {
                    if (_transceiver.BidirectionalStreamCount == _transceiver.Options.MaxBidirectionalStreams)
                    {
                        throw new InvalidDataException(
                            $"maximum bidirectional stream count {_transceiver.Options.MaxBidirectionalStreams} reached");
                    }
                    Interlocked.Increment(ref _transceiver.BidirectionalStreamCount);
                }
                else
                {
                    if (_transceiver.UnidirectionalStreamCount == _transceiver.Options.MaxUnidirectionalStreams)
                    {
                        throw new InvalidDataException(
                            $"maximum unidirectional stream count {_transceiver.Options.MaxUnidirectionalStreams} reached");
                    }
                    Interlocked.Increment(ref _transceiver.UnidirectionalStreamCount);
                }
            }
        }

        internal SlicStream(bool bidirectional, SlicTransceiver transceiver)
            : base(bidirectional, transceiver) => _transceiver = transceiver;

        internal void ReceivedFrame(int size, bool fin)
        {
            // If an outgoing stream and this is the last stream frame, we release the flow control
            // credit to eventually allow a new outgoing stream to be opened. If the flow control credit
            // are already released,
            if (!IsIncoming && fin && !ReleaseFlowControlCredit())
            {
                throw new InvalidDataException("already received last stream frame");
            }

            // Ensure to run the continuation asynchronously in case the continuation ends up calling user-code.
            SignalCompletion((size, fin), runContinuationAsynchronously: true);
        }

        internal override void ReceivedReset(long errorCode)
        {
            // We ignore the stream reset if the stream flow control credit is already released (the last
            // stream frame has already been sent)
            if (ReleaseFlowControlCredit(notifyPeer: true))
            {
                base.ReceivedReset(errorCode);
            }
        }

        private bool ReleaseFlowControlCredit(bool notifyPeer = false)
        {
            if (IsControl)
            {
                return true;
            }

            // If the flow control credit is not already released, decreases the bidirectional or unidirectional
            // semaphore (for outgoing streams) / count (for incoming streams).
            if (Interlocked.CompareExchange(ref _flowControlCreditReleased, 1, 0) == 0)
            {
                if (IsIncoming)
                {
                    if (IsBidirectional)
                    {
                        Interlocked.Decrement(ref _transceiver.BidirectionalStreamCount);
                    }
                    else
                    {
                        Interlocked.Decrement(ref _transceiver.UnidirectionalStreamCount);
                    }

                    if (notifyPeer)
                    {
                        // It's important to decrement the stream count before sending the StreamLast frame to prevent
                        // a race where the peer could start a new stream before the counter is decremented.
                        _transceiver.PrepareAndSendFrameAsync(SlicDefinitions.FrameType.StreamLast, streamId: Id);
                    }
                }
                else if (IsStarted)
                {
                    if (IsBidirectional)
                    {
                        _transceiver.BidirectionalStreamSemaphore!.Release();
                    }
                    else
                    {
                        _transceiver.UnidirectionalStreamSemaphore!.Release();
                    }
                }
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}
