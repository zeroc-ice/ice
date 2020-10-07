// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    /// <summary>The TransceiverStream abstract base class to be overridden by multi-stream transport implementations.
    /// </summary>
    public abstract class TransceiverStream : IDisposable
    {
        /// <summary>The stream ID. If the stream ID hasn't been assigned yet, an exception is thrown. Assigning the
        /// stream ID registers the stream with the transceiver.</summary>
        /// <exception cref="InvalidOperationException">If the stream ID has not been assigned yet.</exception>
        public long Id
        {
            get
            {
                if (_id == -1)
                {
                    throw new InvalidOperationException("stream ID isn't allocated yet");
                }
                return _id;
            }
            set
            {
                Debug.Assert(_id == -1);
                _id = value;
                _transceiver.AddStream(_id, this);
            }
        }

        /// <summary>Returns True if the stream is an incoming stream, False otherwise.</summary>
        public bool IsIncoming => _id % 2 == (_transceiver.IsIncoming ? 0 : 1);

        /// <summary>Returns True if the stream is a bidirectional stream, False otherwise.</summary>
        public bool IsBidirectional { get; }

        /// <summary>Returns True if the stream is a control stream, False otherwise.</summary>
        public bool IsControl => _id == 2 || _id == 3;

        /// <summary>The transport header sentinel. Transport implementations that need to add an additional header
        /// to transmit data over the stream can provide the header data here. This will can improve performances by
        /// reducing the number of allocation as Ice will allocated buffer space for both the transport header and
        /// the Ice protocol header. If a header is returned here, the implementation of the SendAsync method should
        /// this header to be set at the start of the first segment.</summary>
        protected virtual ReadOnlyMemory<byte> Header => ArraySegment<byte>.Empty;

        /// <summary>The Reset event is triggered when a reset frame is received.</summary>
        internal event Action? Reset;

        private protected bool IsStarted => _id != -1;
        private long _id = -1;
        private readonly MultiStreamTransceiver _transceiver;

        /// <summary>Aborts the stream. This is called by the connection when it's being closed. If needed, the stream
        /// implementation should abort the pending receive task.</summary>
        public abstract void Abort(Exception ex);

        /// <summary>Releases the stream resources.</summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>Receives data in the given buffer and returns once the buffer is filled.</summary>
        /// <param name="buffer">The buffer to store the received data.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <return>True if no more data will be received over this stream, False otherwise.</return>
        protected abstract ValueTask<bool> ReceiveAsync(ArraySegment<byte> buffer, CancellationToken cancel);

        /// <summary>Resets the stream.</summary>
        protected abstract ValueTask ResetAsync();

        /// <summary>Sends data from the given buffer and returns once the buffer is sent.</summary>
        /// <param name="buffer">The buffer with the data to send.</param>
        /// <param name="fin">True if no more data will be sent over this stream, False otherwise.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        protected abstract ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel);

        /// <summary>Constructs a stream with the given ID.</summary>
        /// <param name="streamId">The stream ID.</param>
        /// <param name="transceiver">The parent transceiver.</param>
        protected TransceiverStream(long streamId, MultiStreamTransceiver transceiver)
        {
            _transceiver = transceiver;
            _id = streamId;
            _transceiver.AddStream(_id, this);
            IsBidirectional = _id % 4 < 2;
        }

        /// <summary>Constructs an outgoing stream.</summary>
        /// <param name="bidirectional">True to create a bidirectional, False otherwise.</param>
        /// <param name="transceiver">The parent transceiver.</param>
        protected TransceiverStream(bool bidirectional, MultiStreamTransceiver transceiver)
        {
            IsBidirectional = bidirectional;
            _transceiver = transceiver;
        }

        /// <summary>Releases the resources used by the transceiver.</summary>
        /// <param name="disposing">True to release both managed and unmanaged resources; false to release only
        /// unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (IsStarted)
            {
                _transceiver.RemoveStream(Id);
            }
        }

        internal virtual async ValueTask<(long, string message)> ReceiveCloseFrameAsync()
        {
            byte frameType = _transceiver.Endpoint.Protocol == Protocol.Ice1 ?
                (byte)Ice1Definitions.FrameType.CloseConnection : (byte)Ice2Definitions.FrameType.Close;

            (ArraySegment<byte> data, bool fin) =
                await ReceiveFrameAsync(frameType, CancellationToken.None).ConfigureAwait(false);
            if (!fin)
            {
                throw new InvalidDataException($"expected end of stream after Close frame");
            }

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(data, frameType);
            }

            if (_transceiver.Endpoint.Protocol == Protocol.Ice1)
            {
                return (0, "connection gracefully closed by peer");
            }
            else
            {
                var istr = new InputStream(data, Ice2Definitions.Encoding);
                return (istr.ReadVarLong(), istr.ReadString());
            }
        }

        internal void ReceivedReset() => Reset?.Invoke();

        internal virtual async ValueTask ReceiveInitializeFrameAsync(CancellationToken cancel)
        {
            byte frameType = _transceiver.Endpoint.Protocol == Protocol.Ice1 ?
                (byte)Ice1Definitions.FrameType.ValidateConnection : (byte)Ice2Definitions.FrameType.Initialize;

            (ArraySegment<byte> data, bool fin) = await ReceiveFrameAsync(frameType, cancel).ConfigureAwait(false);
            if (fin)
            {
                throw new InvalidDataException($"received unexpected end of stream after initialize frame");
            }

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(data, (byte)frameType);
            }

            if (_transceiver.Endpoint.Protocol == Protocol.Ice1)
            {
                if (data.Count > 0)
                {
                    throw new InvalidDataException(
                        @$"received an ice1 frame with validate connection type and a size of `{data.Count}' bytes");
                }
            }
            else
            {
                // TODO: read initialize settings?
            }
        }

        internal async ValueTask<(IncomingRequestFrame, bool)> ReceiveRequestFrameAsync(CancellationToken cancel)
        {
            byte frameType = _transceiver.Endpoint.Protocol == Protocol.Ice1 ?
                (byte)Ice1Definitions.FrameType.Request : (byte)Ice2Definitions.FrameType.Request;

            (ArraySegment<byte> data, bool fin) = await ReceiveFrameAsync(frameType, cancel).ConfigureAwait(false);

            var request = new IncomingRequestFrame(_transceiver.Endpoint.Protocol,
                                                   data,
                                                   _transceiver.IncomingFrameSizeMax);

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                _transceiver.TraceFrame(Id, request);
            }

            return (request, fin);
        }

        internal async ValueTask<(IncomingResponseFrame, bool)> ReceiveResponseFrameAsync(CancellationToken cancel)
        {
            try
            {
                byte frameType = _transceiver.Endpoint.Protocol == Protocol.Ice1 ?
                    (byte)Ice1Definitions.FrameType.Reply : (byte)Ice2Definitions.FrameType.Response;

                (ArraySegment<byte> data, bool fin) = await ReceiveFrameAsync(frameType, cancel).ConfigureAwait(false);

                var response = new IncomingResponseFrame(_transceiver.Endpoint.Protocol,
                                                         data,
                                                         _transceiver.IncomingFrameSizeMax);

                if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
                {
                    TraceFrame(response);
                }
                return (response, fin);
            }
            catch (OperationCanceledException)
            {
                await ResetAsync().ConfigureAwait(false);
                throw;
            }
        }

        internal virtual async ValueTask SendCloseFrameAsync(long streamId, string reason, CancellationToken cancel)
        {
            if (_transceiver.Endpoint.Protocol == Protocol.Ice1)
            {
                await SendAsync(Ice1Definitions.CloseConnectionFrame, true, cancel).ConfigureAwait(false);

                if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
                {
                    TraceFrame(new List<ArraySegment<byte>>(), (byte)Ice1Definitions.FrameType.CloseConnection);
                }
            }
            else
            {
                var data = new List<ArraySegment<byte>>() { new byte[1024] };
                var ostr = new OutputStream(Ice2Definitions.Encoding, data);
                if (!Header.IsEmpty)
                {
                    ostr.WriteByteSpan(Header.Span);
                }
                ostr.WriteByte((byte)Ice2Definitions.FrameType.Close);
                OutputStream.Position sizePos = ostr.StartFixedLengthSize();
                OutputStream.Position pos = ostr.Tail;
                ostr.WriteVarLong(streamId);
                ostr.WriteString(reason);
                ostr.EndFixedLengthSize(sizePos);
                data[^1] = data[^1].Slice(0, ostr.Finish().Offset);

                await SendAsync(data, true, cancel).ConfigureAwait(false);

                if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
                {
                    TraceFrame(data.Slice(pos, ostr.Tail), (byte)Ice2Definitions.FrameType.Close);
                }
            }
        }

        internal virtual async ValueTask SendInitializeFrameAsync(CancellationToken cancel)
        {
            if (_transceiver.Endpoint.Protocol == Protocol.Ice1)
            {
                await SendAsync(Ice1Definitions.ValidateConnectionFrame, false, cancel).ConfigureAwait(false);

                if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
                {
                    TraceFrame(new List<ArraySegment<byte>>(), (byte)Ice1Definitions.FrameType.ValidateConnection);
                }
            }
            else
            {
                var data = new List<ArraySegment<byte>>() { new byte[Header.Length + 2] };
                Header.CopyTo(data[0]);
                data[0].Slice(Header.Length)[0] = (byte)Ice2Definitions.FrameType.Initialize;
                data[0].Slice(Header.Length + 1).AsSpan().WriteFixedLengthSize20(0);

                // TODO: send protocol specific settings from the frame?

                await SendAsync(data, false, cancel).ConfigureAwait(false);

                if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
                {
                    TraceFrame(new List<ArraySegment<byte>>(), (byte)Ice2Definitions.FrameType.Initialize);
                }
            }
        }

        internal async ValueTask SendRequestFrameAsync(
            OutgoingRequestFrame request,
            bool fin,
            CancellationToken cancel)
        {
            try
            {
                await SendFrameAsync(request, fin, cancel).ConfigureAwait(false);
            }
            catch (OperationCanceledException)
            {
                if (IsStarted)
                {
                    await ResetAsync().ConfigureAwait(false);
                }
                throw;
            }
        }

        internal ValueTask SendResponseFrameAsync(OutgoingResponseFrame response, bool fin, CancellationToken cancel) =>
             SendFrameAsync(response, fin, cancel);

        private protected virtual async ValueTask<(ArraySegment<byte>, bool)> ReceiveFrameAsync(
            byte expectedFrameType,
            CancellationToken cancel)
        {
            // The default implementation only supports the Ice2 protocol
            Debug.Assert(_transceiver.Endpoint.Protocol == Protocol.Ice2);

            // Read the Ice2 protocol header (byte frameType, varuint size)
            ArraySegment<byte> buffer = new byte[256];
            bool fin = await ReceiveAsync(buffer.Slice(0, 2), cancel).ConfigureAwait(false);
            var frameType = (Ice2Definitions.FrameType)buffer[0];
            if ((byte)frameType != expectedFrameType)
            {
                throw new InvalidDataException($"received frame type {frameType} but expected {expectedFrameType}");
            }

            // Read the remainder of the varint size if needed.
            int sizeLength = buffer[1].ReadSizeLength20();
            if (sizeLength > 1)
            {
                fin = await ReceiveAsync(buffer.Slice(2, sizeLength - 1), cancel).ConfigureAwait(false);
            }
            int size = buffer.Slice(1).AsReadOnlySpan().ReadSize20().Size;

            // Read the frame data
            if (size > 0)
            {
                if (size > _transceiver.IncomingFrameSizeMax)
                {
                    throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameSizeMax value");
                }
                buffer = size > buffer.Array!.Length ? new ArraySegment<byte>(new byte[size]) : buffer.Slice(0, size);
                fin = await ReceiveAsync(buffer, cancel).ConfigureAwait(false);
            }
            return (buffer, fin);
        }

        private protected virtual async ValueTask SendFrameAsync(
            OutgoingFrame frame,
            bool fin,
            CancellationToken cancel)
        {
            // The default implementation only supports the Ice2 protocol
            Debug.Assert(_transceiver.Endpoint.Protocol == Protocol.Ice2);

            var data = new List<ArraySegment<byte>>(frame.Data.Count + 1);
            int headerLength = Header.Length;
            byte[] headerData = new byte[headerLength + 1 + OutputStream.GetVarLongLength(frame.Size)];
            Header.CopyTo(headerData);
            if (frame is OutgoingRequestFrame)
            {
                headerData[headerLength] = (byte)Ice2Definitions.FrameType.Request;
            }
            else
            {
                headerData[headerLength] = (byte)Ice2Definitions.FrameType.Response;
            }
            headerData.AsSpan(headerLength + 1).WriteFixedLengthSize20(frame.Size);
            data.Add(headerData);
            data.AddRange(frame.Data);

            await SendAsync(data, fin, cancel).ConfigureAwait(false);

            if (_transceiver.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame);
            }
        }

        internal void TraceFrame(object frame, byte type = 0, byte compress = 0) =>
            _transceiver.TraceFrame(Id, frame, type, compress);
    }
}
