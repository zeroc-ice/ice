// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The Ice1NetworkSocketStream class provides a stream implementation of the Ice1NetworkSocketSocket and
    /// Ice1 protocol.</summary>
    internal class Ice1NetworkSocketStream : SignaledSocketStream<(Ice1FrameType, ArraySegment<byte>)>
    {
        protected override bool ReceivedEndOfStream => _receivedEndOfStream;
        internal int RequestId => IsBidirectional ? ((int)(Id >> 2) + 1) : 0;
        private bool _receivedEndOfStream;
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

        protected async override ValueTask SendAsync(
            IList<ArraySegment<byte>> buffer,
            bool fin,
            CancellationToken cancel) =>
            await _socket.SendFrameAsync(this, buffer, false, cancel).ConfigureAwait(false);

        internal Ice1NetworkSocketStream(Ice1NetworkSocket socket, long streamId)
            : base(socket, streamId) => _socket = socket;

        internal Ice1NetworkSocketStream(Ice1NetworkSocket socket, bool bidirectional, bool control)
            : base(socket, bidirectional, control) => _socket = socket;

        internal void ReceivedFrame(Ice1FrameType frameType, ArraySegment<byte> frame)
        {
            // If we received a response, we make sure to run the continuation asynchronously since this might end
            // up calling user code and could therefore prevent receiving further data since AcceptStreamAsync
            // would be blocked calling user code through this method.
            if (frameType == Ice1FrameType.Reply)
            {
                if (_socket.LastResponseStreamId < Id)
                {
                    _socket.LastResponseStreamId = Id;
                }
            }
            SignalCompletion((frameType, frame));
        }

        private protected override async ValueTask<ArraySegment<byte>> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            // Wait to be signaled for the reception of a new frame for this stream
            (Ice1FrameType frameType, ArraySegment<byte> frame) =
                await WaitSignalAsync(cancel).ConfigureAwait(false);

            // If the received frame is not the one we expected, throw.
            if ((byte)frameType != expectedFrameType)
            {
                throw new InvalidDataException($"received frame type {frameType} but expected {expectedFrameType}");
            }

            _receivedEndOfStream = frameType != Ice1FrameType.ValidateConnection;

            // No more data will ever be received over this stream unless it's the validation connection frame.
            return frame;
        }

        private protected override async ValueTask SendFrameAsync(OutgoingFrame frame, CancellationToken cancel)
        {
            if (frame.StreamDataWriter != null)
            {
                throw new NotSupportedException("stream parameters are not supported with ice1");
            }

            var buffer = new List<ArraySegment<byte>>(frame.Payload.Count + 1);
            var ostr = new OutputStream(Encoding.V11, buffer);

            ostr.WriteByteSpan(Ice1Definitions.FramePrologue);
            ostr.Write(frame is OutgoingRequestFrame ? Ice1FrameType.Request : Ice1FrameType.Reply);
            ostr.WriteByte(0); // placeholder for compression status
            OutputStream.Position start = ostr.StartFixedLengthSize();

            // Note: we don't write the request ID here if the stream ID is not allocated yet. We want to allocate
            // it from the send queue to ensure requests are sent in the same order as the request ID values.
            ostr.WriteInt(IsStarted ? RequestId : 0);
            frame.WriteHeader(ostr);
            ostr.Finish();

            buffer.AddRange(frame.Payload);
            int frameSize = buffer.GetByteCount();
            ostr.RewriteFixedLengthSize11(frameSize, start);

            byte compressionStatus =
                await _socket.SendFrameAsync(this, buffer, frame.Compress, cancel).ConfigureAwait(false);

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame, compress: compressionStatus);
            }
        }
    }
}
