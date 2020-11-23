// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The SocketStream abstract base class to be overridden by multi-stream transport implementations.
    /// There's an instance of this class for each active stream managed by the multi-stream socket.</summary>
    public abstract class SocketStream : IDisposable
    {
        /// <summary>The stream ID. If the stream ID hasn't been assigned yet, an exception is thrown. Assigning the
        /// stream ID registers the stream with the socket.</summary>
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
                _socket.AddStream(_id, this);
            }
        }

        /// <summary>Returns True if the stream is an incoming stream, False otherwise.</summary>
        public bool IsIncoming => _id % 2 == (_socket.IsIncoming ? 0 : 1);

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
        internal event Action<long>? Reset;

        /// <summary>Returns true if the stream ID is assigned</summary>
        internal bool IsStarted => _id != -1;

        private long _id = -1;
        private readonly MultiStreamSocket _socket;

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
        /// <param name="errorCode">The error code indicating the reason of the reset to transmit to the peer.</param>
        protected abstract ValueTask ResetAsync(long errorCode);

        /// <summary>Sends data from the given buffer and returns once the buffer is sent.</summary>
        /// <param name="buffer">The buffer with the data to send.</param>
        /// <param name="fin">True if no more data will be sent over this stream, False otherwise.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        protected abstract ValueTask SendAsync(IList<ArraySegment<byte>> buffer, bool fin, CancellationToken cancel);

        /// <summary>Constructs a stream with the given ID.</summary>
        /// <param name="streamId">The stream ID.</param>
        /// <param name="socket">The parent socket.</param>
        protected SocketStream(long streamId, MultiStreamSocket socket)
        {
            _socket = socket;
            _id = streamId;
            _socket.AddStream(_id, this);
            IsBidirectional = _id % 4 < 2;
        }

        /// <summary>Constructs an outgoing stream.</summary>
        /// <param name="bidirectional">True to create a bidirectional, False otherwise.</param>
        /// <param name="socket">The parent socket.</param>
        protected SocketStream(bool bidirectional, MultiStreamSocket socket)
        {
            IsBidirectional = bidirectional;
            _socket = socket;
        }

        /// <summary>Releases the resources used by the socket.</summary>
        /// <param name="disposing">True to release both managed and unmanaged resources; false to release only
        /// unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (IsStarted)
            {
                _socket.RemoveStream(Id);
            }
        }

        internal virtual async ValueTask<((long, long), string)> ReceiveGoAwayFrameAsync()
        {
            byte frameType = _socket.Endpoint.Protocol == Protocol.Ice1 ?
                (byte)Ice1Definitions.FrameType.CloseConnection : (byte)Ice2Definitions.FrameType.GoAway;

            (ArraySegment<byte> data, bool fin) =
                await ReceiveFrameAsync(frameType, CancellationToken.None).ConfigureAwait(false);
            if (!fin)
            {
                throw new InvalidDataException($"expected end of stream after GoAway frame");
            }

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(data, frameType);
            }

            if (_socket.Endpoint.Protocol == Protocol.Ice1)
            {
                // LastResponseStreamId contains the stream ID of the last received response. We make sure to return
                // this stream ID to ensure the request with this stream ID will complete successfully in case the
                // close connection message is received shortly after the response and potentially processed before
                // due to the thread scheduling.
                return ((_socket.LastResponseStreamId, 0), "connection gracefully closed by peer");
            }
            else
            {
                var goAwayFrame = new Ice2GoAwayBody(new InputStream(data, Ice2Definitions.Encoding));
                return (((long)goAwayFrame.LastBidirectionalStreamId, (long)goAwayFrame.LastUnidirectionalStreamId),
                        goAwayFrame.Message);
            }
        }

        internal virtual void ReceivedReset(long errorCode) => Reset?.Invoke(errorCode);

        internal virtual async ValueTask ReceiveInitializeFrameAsync(CancellationToken cancel)
        {
            byte frameType = _socket.Endpoint.Protocol == Protocol.Ice1 ?
                (byte)Ice1Definitions.FrameType.ValidateConnection : (byte)Ice2Definitions.FrameType.Initialize;

            (ArraySegment<byte> data, bool fin) = await ReceiveFrameAsync(frameType, cancel).ConfigureAwait(false);
            if (fin)
            {
                throw new InvalidDataException($"received unexpected end of stream after initialize frame");
            }

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(data, frameType);
            }

            if (_socket.Endpoint.Protocol == Protocol.Ice1)
            {
                if (data.Count > 0)
                {
                    throw new InvalidDataException(
                        @$"received an ice1 frame with validate connection type and a size of `{data.Count}' bytes");
                }
            }
            else
            {
                // Read the protocol parameters which are encoded with the binary context encoding.
                var istr = new InputStream(data, Ice2Definitions.Encoding);
                int dictionarySize = istr.ReadSize();
                for (int i = 0; i < dictionarySize; ++i)
                {
                    (int key, ReadOnlyMemory<byte> value) = istr.ReadBinaryContextEntry();
                    if (key == (int)Ice2ParameterKey.IncomingFrameMaxSize)
                    {
                        checked
                        {
                            _socket.PeerIncomingFrameMaxSize = (int)value.Span.ReadVarULong().Value;
                        }

                        if (_socket.PeerIncomingFrameMaxSize < 1024)
                        {
                            throw new InvalidDataException($@"the peer's IncomingFrameMaxSize ({
                                _socket.PeerIncomingFrameMaxSize} bytes) value is inferior to 1KB");
                        }
                    }
                    else
                    {
                        // Ignore unsupported parameters.
                    }
                }

                if (_socket.PeerIncomingFrameMaxSize == null)
                {
                    throw new InvalidDataException("missing IncomingFrameMaxSize Ice2 connection parameter");
                }
            }
        }

        internal async ValueTask<(IncomingRequestFrame, bool)> ReceiveRequestFrameAsync(CancellationToken cancel)
        {
            byte frameType = _socket.Endpoint.Protocol == Protocol.Ice1 ?
                (byte)Ice1Definitions.FrameType.Request : (byte)Ice2Definitions.FrameType.Request;

            (ArraySegment<byte> data, bool fin) = await ReceiveFrameAsync(frameType, cancel).ConfigureAwait(false);

            var request = new IncomingRequestFrame(_socket.Endpoint.Protocol,
                                                   data,
                                                   _socket.IncomingFrameMaxSize);

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                _socket.TraceFrame(Id, request);
            }

            return (request, fin);
        }

        internal async ValueTask<(IncomingResponseFrame, bool)> ReceiveResponseFrameAsync(CancellationToken cancel)
        {
            try
            {
                byte frameType = _socket.Endpoint.Protocol == Protocol.Ice1 ?
                    (byte)Ice1Definitions.FrameType.Reply : (byte)Ice2Definitions.FrameType.Response;

                (ArraySegment<byte> data, bool fin) = await ReceiveFrameAsync(frameType, cancel).ConfigureAwait(false);

                var response = new IncomingResponseFrame(_socket.Endpoint.Protocol,
                                                         data,
                                                         _socket.IncomingFrameMaxSize);

                if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
                {
                    TraceFrame(response);
                }
                return (response, fin);
            }
            catch (OperationCanceledException)
            {
                if (_socket.Endpoint.Protocol != Protocol.Ice1)
                {
                    await ResetAsync((long)StreamResetErrorCode.RequestCanceled).ConfigureAwait(false);
                }
                throw;
            }
        }

        internal virtual async ValueTask SendGoAwayFrameAsync(
            (long Bidirectional, long Unidirectional) streamIds,
            string reason,
            CancellationToken cancel)
        {
            if (_socket.Endpoint.Protocol == Protocol.Ice1)
            {
                await SendAsync(Ice1Definitions.CloseConnectionFrame, true, cancel).ConfigureAwait(false);

                if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
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
                ostr.WriteByte((byte)Ice2Definitions.FrameType.GoAway);
                OutputStream.Position sizePos = ostr.StartFixedLengthSize();
                OutputStream.Position pos = ostr.Tail;
                var goAwayFrameBody = new Ice2GoAwayBody(
                    (ulong)streamIds.Bidirectional,
                    (ulong)streamIds.Unidirectional,
                    reason);
                goAwayFrameBody.IceWrite(ostr);
                ostr.EndFixedLengthSize(sizePos);
                data[^1] = data[^1].Slice(0, ostr.Finish().Offset);

                await SendAsync(data, true, cancel).ConfigureAwait(false);

                if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
                {
                    TraceFrame(data.Slice(pos, ostr.Tail), (byte)Ice2Definitions.FrameType.GoAway);
                }
            }
        }

        internal virtual async ValueTask SendInitializeFrameAsync(CancellationToken cancel)
        {
            if (_socket.Endpoint.Protocol == Protocol.Ice1)
            {
                await SendAsync(Ice1Definitions.ValidateConnectionFrame, false, cancel).ConfigureAwait(false);

                if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
                {
                    TraceFrame(new List<ArraySegment<byte>>(), (byte)Ice1Definitions.FrameType.ValidateConnection);
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
                ostr.WriteByte((byte)Ice2Definitions.FrameType.Initialize);
                OutputStream.Position sizePos = ostr.StartFixedLengthSize();
                OutputStream.Position pos = ostr.Tail;

                // Encode the transport parameters with the binary context encoding.
                ostr.WriteSize(1);

                // Transmit out local incoming frame maximum size
                Debug.Assert(_socket.IncomingFrameMaxSize > 0);
                ostr.WriteBinaryContextEntry((int)Ice2ParameterKey.IncomingFrameMaxSize,
                                             (ulong)_socket.IncomingFrameMaxSize,
                                             OutputStream.IceWriterFromVarULong);

                ostr.EndFixedLengthSize(sizePos);
                data[^1] = data[^1].Slice(0, ostr.Finish().Offset);

                await SendAsync(data, false, cancel).ConfigureAwait(false);

                if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
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
                if (IsStarted && _socket.Endpoint.Protocol != Protocol.Ice1)
                {
                    await ResetAsync((long)StreamResetErrorCode.RequestCanceled).ConfigureAwait(false);
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
            Debug.Assert(_socket.Endpoint.Protocol == Protocol.Ice2);

            // Read the Ice2 protocol header (byte frameType, varulong size)
            ArraySegment<byte> buffer = new byte[256];
            bool fin = await ReceiveAsync(buffer.Slice(0, 2), cancel).ConfigureAwait(false);
            var frameType = (Ice2Definitions.FrameType)buffer[0];
            if ((byte)frameType != expectedFrameType)
            {
                throw new InvalidDataException($"received frame type {frameType} but expected {expectedFrameType}");
            }

            // Read the remainder of the size if needed.
            int sizeLength = buffer[1].ReadSizeLength20();
            if (sizeLength > 1)
            {
                fin = await ReceiveAsync(buffer.Slice(2, sizeLength - 1), cancel).ConfigureAwait(false);
            }
            int size = buffer.Slice(1).AsReadOnlySpan().ReadSize20().Size;

            // Read the frame data
            if (size > 0)
            {
                if (size > _socket.IncomingFrameMaxSize)
                {
                    throw new InvalidDataException($"frame with {size} bytes exceeds Ice.IncomingFrameMaxSize value");
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
            Debug.Assert(_socket.Endpoint.Protocol == Protocol.Ice2);

            if (frame.Size > _socket.PeerIncomingFrameMaxSize)
            {
                if (frame is OutgoingRequestFrame)
                {
                    throw new ArgumentOutOfRangeException(
                        $@"the request size is larger than the peer's IncomingFrameSizeMax ({
                        _socket.PeerIncomingFrameMaxSize} bytes)");
                }
                else
                {
                    // Throw a remote exception instead of this response, the Ice connection will catch it and send it
                    // as the response instead of sending this response which is too large.
                    throw new LimitExceededException(
                        $@"the response size is larger than the peer's IncomingFrameSizeMax ({
                        _socket.PeerIncomingFrameMaxSize} bytes)");
                }
            }

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

            if (_socket.Endpoint.Communicator.TraceLevels.Protocol >= 1)
            {
                TraceFrame(frame);
            }
        }

        internal void TraceFrame(object frame, byte type = 0, byte compress = 0) =>
            _socket.TraceFrame(Id, frame, type, compress);
    }
}
