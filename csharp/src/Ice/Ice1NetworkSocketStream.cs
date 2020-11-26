// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The Ice1NetworkSocketStream class provides a stream implementation of the Ice1NetworkSocketSocket and
    /// Ice1 protocol.</summary>
    internal class Ice1NetworkSocketStream : SignaledSocketStream<(Ice1Definitions.FrameType, ArraySegment<byte>)>
    {
        protected override ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;
        protected override bool ReceivedEndOfStream => _receivedEndOfStream;
        protected override bool SentEndOfStream => _sentEndOfStream;

        private bool _receivedEndOfStream;
        private bool _sentEndOfStream;
        private readonly Ice1NetworkSocket _socket;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing)
            {
                _socket.ReleaseFlowControlCredit(this);
            }
        }

        protected override ValueTask<int> ReceiveAsync(Memory<byte> buffer, CancellationToken cancel) =>
            // This is never called because we override the default ReceiveFrameAsync implementation
            throw new NotImplementedException();

        protected override ValueTask ResetAsync(long errorCode) =>
            // Stream reset is not supported with Ice1
            new ValueTask();

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel)
        {
            if (fin)
            {
                _sentEndOfStream = true;
            }
            return _socket.SendFrameAsync(buffer, cancel);
        }

        internal Ice1NetworkSocketStream(Ice1NetworkSocket socket, long streamId)
            : base(socket, streamId) => _socket = socket;

        internal void ReceivedFrame(Ice1Definitions.FrameType frameType, ArraySegment<byte> frame)
        {
            // If we received a response, we make sure to run the continuation asynchronously since this might end
            // up calling user code and could therefore prevent receiving further data since AcceptStreamAsync
            // would be blocked calling user code through this method.
            if (frameType == Ice1Definitions.FrameType.Reply)
            {
                _socket.LastResponseStreamId = Id;
                SignalCompletion((frameType, frame), runContinuationAsynchronously: true);
            }
            else
            {
                SignalCompletion((frameType, frame), runContinuationAsynchronously: false);
            }
        }

        private protected override async ValueTask<ArraySegment<byte>> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            // Wait to be signaled for the reception of a new frame for this stream
            (Ice1Definitions.FrameType frameType, ArraySegment<byte> frame) =
                await WaitSignalAsync(cancel).ConfigureAwait(false);

            // If the received frame is not the one we expected, throw.
            if ((byte)frameType != expectedFrameType)
            {
                throw new InvalidDataException($"received frame type {frameType} but expected {expectedFrameType}");
            }

            _receivedEndOfStream = frameType != Ice1Definitions.FrameType.ValidateConnection;

            // No more data will ever be received over this stream unless it's the validation connection frame.
            return frame;
        }

        private protected override async ValueTask SendFrameAsync(OutgoingFrame frame, CancellationToken cancel)
        {
            if (frame.StreamDataWriter != null)
            {
                throw new NotSupportedException("stream parameters are not supported with ice1");
            }

            var buffer = new List<ArraySegment<byte>>(frame.Data.Count + 1);
            byte[] headerData = new byte[Ice1Definitions.HeaderSize + 4];
            if (frame is OutgoingRequestFrame)
            {
                Ice1Definitions.RequestHeaderPrologue.CopyTo(headerData.AsSpan());
            }
            else
            {
                Ice1Definitions.ResponseHeaderPrologue.CopyTo(headerData.AsSpan());
            }
            int size = frame.Size + Ice1Definitions.HeaderSize + 4;
            headerData.AsSpan(10, 4).WriteInt(size);
            // Write the request ID
            headerData.AsSpan(Ice1Definitions.HeaderSize).WriteInt(IsBidirectional ? ((int)(Id >> 2) + 1) : 0);
            buffer.Add(headerData);
            buffer.AddRange(frame.Data);

            byte compressionStatus = 0;
            if (BZip2.IsLoaded && frame.Compress)
            {
                List<ArraySegment<byte>>? compressed = null;
                if (size >= _socket.Endpoint.Communicator.CompressionMinSize)
                {
                    compressed = BZip2.Compress(buffer,
                                                size,
                                                Ice1Definitions.HeaderSize,
                                                _socket.Endpoint.Communicator.CompressionLevel);
                }

                if (compressed != null)
                {
                    // Message compressed, get the compression status and ensure we send the compressed message.
                    buffer = compressed;
                    compressionStatus = buffer[0][9];
                }
                else
                {
                    // Message not compressed, request compressed response, if any and write the compression status.
                    compressionStatus = 1;
                    ArraySegment<byte> header = buffer[0];
                    header[9] = compressionStatus; // Write the compression status
                }
            }

            await _socket.SendFrameAsync(buffer, CancellationToken.None).ConfigureAwait(false);

            _sentEndOfStream = true;

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame, compress: compressionStatus);
            }
        }
    }
}
