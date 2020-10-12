// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The LegacyStream class provides a stream implementation of the LegacyTransceiver and Ice1 protocol.
    /// </summary>
    internal class LegacyStream : SignaledTransceiverStream<(Ice1Definitions.FrameType, ArraySegment<byte>)>
    {
        protected override ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;
        private int RequestId => IsBidirectional ? ((int)(Id >> 2) + 1) : 0;
        private readonly LegacyTransceiver _transceiver;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing && IsIncoming)
            {
                if (IsBidirectional)
                {
                    _transceiver.BidirectionalSerializeSemaphore?.Release();
                }
                else if (!IsControl)
                {
                    _transceiver.UnidirectionalSerializeSemaphore?.Release();
                }
            }
        }

        protected override ValueTask<bool> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel) =>
            // This is never called because we override the default ReceiveFrameAsync implementation
            throw new NotImplementedException();

        protected override ValueTask ResetAsync(long errorCode) =>
            // Stream reset is not supported with Ice1
            new ValueTask();

        protected override ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel) =>
            _transceiver.SendFrameAsync(buffer, cancel);

        internal LegacyStream(long streamId, LegacyTransceiver transceiver)
            : base(streamId, transceiver) => _transceiver = transceiver;

        internal void ReceivedFrame(Ice1Definitions.FrameType frameType, ArraySegment<byte> frame)
        {
            // If we received a response, we make sure to run the continuation asynchronously since this might end
            // up calling user code and could therefore prevent receiving further data since AcceptStreamAsync
            // would be blocked calling user code through this method.
            if (frameType == Ice1Definitions.FrameType.Reply)
            {
                _transceiver.LastResponseStreamId = Id;
                SignalCompletion((frameType, frame), runContinuationAsynchronously: true);
            }
            else
            {
                SignalCompletion((frameType, frame), runContinuationAsynchronously: false);
            }
        }

        private protected override async ValueTask<(ArraySegment<byte>, bool)> ReceiveFrameAsync(
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

            // No more data will ever be received over this stream unless it's the validation connection frame.
            return (frame, frameType != Ice1Definitions.FrameType.ValidateConnection);
        }

        private protected override async ValueTask SendFrameAsync(
            OutgoingFrame frame,
            bool fin,
            CancellationToken cancel)
        {
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
            headerData.AsSpan(Ice1Definitions.HeaderSize).WriteInt(RequestId);
            buffer.Add(headerData);
            buffer.AddRange(frame.Data);

            byte compressionStatus = 0;
            if (BZip2.IsLoaded && frame.Compress)
            {
                List<ArraySegment<byte>>? compressed = null;
                if (size >= _transceiver.Endpoint.Communicator.CompressionMinSize)
                {
                    compressed = BZip2.Compress(buffer,
                                                size,
                                                Ice1Definitions.HeaderSize,
                                                _transceiver.Endpoint.Communicator.CompressionLevel);
                }

                ArraySegment<byte> header;
                if (compressed != null)
                {
                    buffer = compressed;
                    header = buffer[0];
                    size = buffer.GetByteCount();
                }
                else // Message not compressed, request compressed response, if any.
                {
                    header = buffer[0];
                    header[9] = 1; // Write the compression status
                }
                compressionStatus = header[9];
            }

            // Ensure the frame isn't bigger than what we can send with the transport.
            _transceiver.Underlying.CheckSendSize(size);

            await _transceiver.SendFrameAsync(buffer, CancellationToken.None).ConfigureAwait(false);

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame, 0, compressionStatus);
            }
        }
    }
}
