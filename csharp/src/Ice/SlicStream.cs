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
    internal class SlicStream : SignaledSocketStream<(int, bool)>
    {
        protected override ReadOnlyMemory<byte> TransportHeader => SlicDefinitions.FrameHeader;
        protected override bool ReceivedEndOfStream => _receivedEndOfStream;
        private int _flowControlCreditReleased;
        private int _receivedOffset;
        private int _receivedSize;
        private bool _receivedEndOfStream;
        private readonly SlicSocket _socket;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing)
            {
                try
                {
                    ValueTask<(int, bool)> valueTask = WaitSignalAsync(CancellationToken.None);
                    Debug.Assert(valueTask.IsCompleted);
                    (_receivedSize, _receivedEndOfStream) = valueTask.Result;
                    _receivedOffset = 0;
                }
                catch
                {
                    // Ignore, there's nothing to consume.
                }

                // If there's still data pending to be receive for the stream, we notify the socket that
                // we're abandoning the reading. It will finish to read the stream's frame data in order to
                // continue receiving frames for other streams.
                if (_receivedOffset < _receivedSize)
                {
                    _socket.FinishedReceivedStreamData(Id, _receivedOffset, _receivedSize, _receivedEndOfStream);
                }

                // Only release the flow control credit for incoming streams on Dispose. Flow control for Slic outgoing
                // streams are released when the StreamLast frame is received and it can be received after the stream
                // is disposed (e.g.: for oneway requests we dispose the stream as soon as the request is sent and
                // before receiving the StreamLast frame).
                if (IsIncoming)
                {
                    ReleaseFlowControlCredit(notifyPeer: true);
                }
            }
        }

        protected override async ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel)
        {
            if (_receivedSize == _receivedOffset)
            {
                if (_receivedEndOfStream)
                {
                    return 0;
                }

                // Wait to be signaled for the reception of a new stream frame for this stream.
                (_receivedSize, _receivedEndOfStream) = await WaitSignalAsync(cancel).ConfigureAwait(false);
                _receivedOffset = 0;
                if (_receivedSize == 0)
                {
                    return 0;
                }
            }

            // Read and append the received stream frame data into the given buffer.
            int size = Math.Min(_receivedSize - _receivedOffset, buffer.Length);
            await _socket.ReceiveDataAsync(buffer.Slice(0, size), CancellationToken.None).ConfigureAwait(false);
            _receivedOffset += size;

            // If we've consumed the whole Slic frame, notify the socket that it can start receiving
            // a new frame.
            if (_receivedOffset == _receivedSize)
            {
                _socket.FinishedReceivedStreamData(Id, _receivedOffset, _receivedSize, _receivedEndOfStream);
            }
            return size;
        }

        protected override ValueTask ResetAsync(long errorCode) =>
            new(_socket.PrepareAndSendFrameAsync(
                SlicDefinitions.FrameType.StreamReset,
                ostr =>
                {
                    checked
                    {
                        new StreamResetBody((ulong)errorCode).IceWrite(ostr);
                    }
                },
                Id));

        protected override async ValueTask SendAsync(
            IList<ArraySegment<byte>> buffer,
            bool fin,
            CancellationToken cancel)
        {
            // Ensure the caller reserved space for the Slic header by checking for the sentinel header.
            Debug.Assert(TransportHeader.Span.SequenceEqual(buffer[0].Slice(0, TransportHeader.Length)));

            int size = buffer.GetByteCount();

            // If the protocol buffer is larger than the configured Slic packet size, send the buffer with multiple
            // Slic stream frames.
            int packetMaxSize = _socket.Endpoint.Communicator.SlicPacketMaxSize;
            if (size > packetMaxSize)
            {
                // The send buffer for the Slic stream frame.
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
                        // If it's not the first Slic stream frame, we re-use the space reserved for the Slic header in
                        // the first segment of the protocol buffer.
                        sendBuffer.Add(buffer[0].Slice(0, TransportHeader.Length));
                        sendSize += sendBuffer[0].Count;
                    }

                    // Append data until we reach the Slic packet size or the end of the buffer to send.
                    bool lastBuffer = false;
                    for (int i = start.Segment; i < buffer.Count; ++i)
                    {
                        int segmentOffset = i == start.Segment ? start.Offset : 0;
                        if (sendSize + buffer[i].Slice(segmentOffset).Count > packetMaxSize)
                        {
                            sendBuffer.Add(buffer[i].Slice(segmentOffset, packetMaxSize - sendSize));
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

                    // Send the Slic stream frame.
                    offset += sendSize;
                    await _socket.SendStreamFrameAsync(this,
                                                       sendSize,
                                                       lastBuffer && fin,
                                                       sendBuffer,
                                                       cancel).ConfigureAwait(false);
                }
            }
            else
            {
                // If the buffer to send is small enough to fit in a single Slic stream frame, send it directly.
                await _socket.SendStreamFrameAsync(this, size, fin, buffer, cancel).ConfigureAwait(false);
            }
        }

        internal SlicStream(SlicSocket socket, long streamId)
            : base(socket, streamId) => _socket = socket;

        internal SlicStream(SlicSocket socket, bool bidirectional, bool control)
            : base(socket, bidirectional, control) => _socket = socket;

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
            SignalCompletion((size, fin));
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

        internal bool ReleaseFlowControlCredit(bool notifyPeer = false)
        {
            if (Interlocked.CompareExchange(ref _flowControlCreditReleased, 1, 0) == 0)
            {
                if (!IsControl)
                {
                    // If the flow control credit is not already released, releases it now.
                    _socket.ReleaseFlowControlCredit(this);

                    if (IsIncoming && notifyPeer)
                    {
                        // It's important to decrement the stream count before sending the StreamLast frame to prevent
                        // a race where the peer could start a new stream before the counter is decremented.
                        _ = _socket.PrepareAndSendFrameAsync(SlicDefinitions.FrameType.StreamLast, streamId: Id);
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
