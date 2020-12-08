// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a response protocol frame received by the application.</summary>
    public sealed class IncomingResponseFrame : IncomingFrame, IDisposable
    {
        /// <summary>The encoding of the frame payload.</summary>
        public override Encoding Encoding { get; }

        /// <summary>The <see cref="ZeroC.Ice.ResultType"/> of this response frame.</summary>
        public ResultType ResultType => Payload[0] == 0 ? ResultType.Success : ResultType.Failure;

        private static readonly ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), IncomingResponseFrame>
            _cachedVoidReturnValueFrames =
                new ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), IncomingResponseFrame>();

        // The optional socket stream. The stream is non-null if there's still data to read over the stream
        // after the reading of the response frame.
        private SocketStream? _socketStream;

        /// <summary>Constructs an incoming response frame.</summary>
        /// <param name="protocol">The Ice protocol of this frame.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="maxSize">The maximum payload size, checked during decompress.</param>
        public IncomingResponseFrame(Protocol protocol, ArraySegment<byte> data, int maxSize)
            : this(protocol, data, maxSize, null)
        {
        }

        /// <summary>Releases resources used by the response frame.</summary>
        public void Dispose() => _socketStream?.TryDispose();

        /// <summary>Reads the return value. If this response frame carries a failure, reads and throws this exception.
        /// </summary>
        /// <paramtype name="T">The type of the return value.</paramtype>
        /// <param name="proxy">The proxy used to send the request. <c>proxy</c> is used to read relative proxies.
        /// </param>
        /// <param name="reader">An input stream reader used to read the frame return value, when the frame
        /// return value contain multiple values the reader must use a tuple to return the values.</param>
        /// <returns>The frame return value.</returns>
        public T ReadReturnValue<T>(IObjectPrx proxy, InputStreamReader<T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (_socketStream != null)
            {
                throw new InvalidDataException("stream data available for operation without stream parameter");
            }

            if (ResultType == ResultType.Success)
            {
                return Payload.AsReadOnlyMemory(1).ReadEncapsulation(Protocol.GetEncoding(), reader, proxy: proxy);
            }
            else
            {
                throw ReadException(proxy);
            }
        }

        /// <summary>Reads the return value which contains a stream return value. If this response frame carries a
        /// failure, reads and throws this exception.</summary>
        /// <paramtype name="T">The type of the return value.</paramtype>
        /// <param name="proxy">The proxy used to send the request. <c>proxy</c> is used to read relative proxies.
        /// </param>
        /// <param name="reader">A reader used to read the frame return value, when the frame return value contain
        /// multiple values the reader must use a tuple to return the values.</param>
        /// <returns>The frame return value.</returns>
        public T ReadReturnValue<T>(IObjectPrx proxy, InputStreamReaderWithStreamable<T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (ResultType == ResultType.Success)
            {
                if (_socketStream == null)
                {
                    throw new InvalidDataException("no stream data available for operation with stream parameter");
                }

                var istr = new InputStream(Payload.AsReadOnlyMemory(1),
                                           Protocol.GetEncoding(),
                                           reference: proxy?.IceReference,
                                           startEncapsulation: true);
                T value = reader(istr, _socketStream);
                // Clear the socket stream to ensure it's not disposed with the response frame. It's now the
                // responsibility of the stream parameter object to dispose the socket stream.
                _socketStream = null;
                istr.CheckEndOfBuffer(skipTaggedParams: true);
                return value;
            }
            else
            {
                if (_socketStream != null)
                {
                    throw new InvalidDataException("stream data available with remote exception result");
                }
                throw ReadException(proxy);
            }
        }

        /// <summary>Reads the return value which is a stream return value only. If this response frame carries a
        /// failure, reads and throws this exception.</summary>
        /// <paramtype name="T">The type of the return value.</paramtype>
        /// <param name="proxy">The proxy used to send the request. <c>proxy</c> is used to read relative proxies.
        /// </param>
        /// <param name="reader">A reader used to read the frame return value.</param>
        /// <returns>The frame return value.</returns>
        public T ReadReturnValue<T>(IObjectPrx proxy, Func<SocketStream, T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (ResultType == ResultType.Success)
            {
                if (_socketStream == null)
                {
                    throw new InvalidDataException("no stream data available for operation with stream parameter");
                }
                Payload.AsReadOnlyMemory(1).ReadEmptyEncapsulation(Protocol.GetEncoding());
                T value = reader(_socketStream);
                // Clear the socket stream to ensure it's not disposed with the response frame. It's now the
                // responsibility of the stream parameter object to dispose the socket stream.
                _socketStream = null;
                return value;
            }
            else
            {
                if (_socketStream != null)
                {
                    throw new InvalidDataException("stream data available with remote exception result");
                }
                throw ReadException(proxy);
            }
        }

        /// <summary>Reads the return value and makes sure this return value is empty (void) or has only unknown tagged
        /// members. If this response frame carries a failure, reads and throws this exception.</summary>
        /// <param name="proxy">The proxy used to send the request. <c>proxy</c> is used to read relative proxies.
        /// </param>
        public void ReadVoidReturnValue(IObjectPrx proxy)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (_socketStream != null)
            {
                throw new InvalidDataException("stream data available for operation without stream parameter");
            }

            if (ResultType == ResultType.Success)
            {
                Payload.AsReadOnlyMemory(1).ReadEmptyEncapsulation(Protocol.GetEncoding());
            }
            else
            {
                throw ReadException(proxy);
            }
        }

        /// <summary>Returns an <see cref="IncomingResponseFrame"/> that represents a oneway pseudo response.</summary>
        internal static IncomingResponseFrame WithVoidReturnValue(Protocol protocol, Encoding encoding) =>
            _cachedVoidReturnValueFrames.GetOrAdd((protocol, encoding), key =>
            {
                var data = new List<ArraySegment<byte>>();
                var ostr = new OutputStream(key.Protocol.GetEncoding(), data);
                ostr.Write(ResultType.Success);
                _ = ostr.WriteEmptyEncapsulation(key.Encoding);
                Debug.Assert(data.Count == 1);
                return new IncomingResponseFrame(key.Protocol, data[0], int.MaxValue);
            });

        /// <summary>Constructs an incoming response frame.</summary>
        /// <param name="protocol">The Ice protocol of this frame.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="maxSize">The maximum payload size, checked during decompress.</param>
        /// <param name="socketStream">The optional socket stream. The stream is non-null if there's still data to
        /// read on the stream after the reading the response frame.</param>
        internal IncomingResponseFrame(
            Protocol protocol,
            ArraySegment<byte> data,
            int maxSize,
            SocketStream? socketStream)
            : base(data, protocol, maxSize)
        {
            _socketStream = socketStream;

            bool hasEncapsulation = false;

            if (Protocol == Protocol.Ice1)
            {
                ReplyStatus replyStatus = Data[0].AsReplyStatus();

                if ((byte)replyStatus <= (byte)ReplyStatus.UserException)
                {
                    hasEncapsulation = true;
                }
                else
                {
                    Encoding = Encoding.V11;
                }
            }
            else
            {
                _ = Data[0].AsResultType(); // just to check the value
                hasEncapsulation = true;
            }

            // Read encapsulation header, in particular the payload encoding.
            if (hasEncapsulation)
            {
                int size;
                int sizeLength;

                (size, sizeLength, Encoding) =
                    Data.Slice(1).AsReadOnlySpan().ReadEncapsulationHeader(Protocol.GetEncoding());

                Payload = Data.Slice(0, 1 + size + sizeLength);
                HasCompressedPayload = Encoding == Encoding.V20 && Payload[1 + sizeLength + 2] != 0;
            }
            else
            {
                Payload = Data;
            }
        }

        internal RetryPolicy GetRetryPolicy(Reference reference)
        {
            RetryPolicy retryPolicy = RetryPolicy.NoRetry;
            if (Encoding == Encoding.V11)
            {
                retryPolicy = Ice1Definitions.GetRetryPolicy(this, reference);
            }
            else if (BinaryContext.TryGetValue((int)Ice.BinaryContext.RetryPolicy,
                                               out ReadOnlyMemory<byte> value))
            {
                retryPolicy = value.Read(istr => new RetryPolicy(istr));
            }
            return retryPolicy;
        }

        private protected override ArraySegment<byte> GetEncapsulation()
        {
            // Can only be called for a frame with an encapsulation:
            Debug.Assert(Protocol == Protocol.Ice2 || Payload[0] <= (byte)ReplyStatus.UserException);
            return Payload.Slice(1);
        }

        private Exception ReadException(IObjectPrx proxy)
        {
            Debug.Assert(ResultType != ResultType.Success);

            var replyStatus = (ReplyStatus)Payload[0]; // can be reassigned below

            InputStream istr;

            if (Protocol == Protocol.Ice2 || replyStatus == ReplyStatus.UserException)
            {
                istr = new InputStream(Payload.Slice(1),
                                       Protocol.GetEncoding(),
                                       reference: proxy.IceReference,
                                       startEncapsulation: true);

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V11)
                {
                    replyStatus = istr.ReadReplyStatus();
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1 && Encoding == Encoding.V11);
                istr = new InputStream(Payload.Slice(1), Encoding.V11);
            }

            Exception exception;
            if (Encoding == Encoding.V11 && replyStatus != ReplyStatus.UserException)
            {
                exception = istr.ReadIce1SystemException(replyStatus);
                istr.CheckEndOfBuffer(skipTaggedParams: false);
            }
            else
            {
                exception = istr.ReadException();
                istr.CheckEndOfBuffer(skipTaggedParams: true);
            }
            return exception;
        }
    }
}
