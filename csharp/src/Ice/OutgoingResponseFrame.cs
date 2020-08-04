//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame : OutgoingFrame
    {
        /// <summary>The result type; see <see cref="ZeroC.Ice.ResultType"/>.</summary>
        public ResultType ResultType => Payload[0][0] == 0 ? ResultType.Success : ResultType.Failure;

        private static readonly ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), OutgoingResponseFrame>
            _cachedVoidReturnValueFrames =
                new ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), OutgoingResponseFrame>();

        /// <summary>Creates a new outgoing response frame with a void return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithVoidReturnValue(Current current) =>
            _cachedVoidReturnValueFrames.GetOrAdd((current.Protocol, current.Encoding), key =>
            {
                var data = new List<ArraySegment<byte>>();
                var ostr = new OutputStream(key.Protocol.GetEncoding(), data);
                ostr.WriteByte((byte)ResultType.Success);
                _ = ostr.WriteEmptyEncapsulation(key.Encoding);
                return new OutgoingResponseFrame(key.Protocol, key.Encoding, data: data);
            });

        /// <summary>Creates a new outgoing response frame with a return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="compress">True if the response should be compressed, false otherwise.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
        /// <param name="value">The return value to marshal.</param>
        /// <param name="writer">A delegate that must write the value to the frame.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(
            Current current,
            bool compress,
            FormatType? format,
            T value,
            OutputStreamWriter<T> writer)
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, compress, format);
            writer(ostr, value);
            response.Finish(ostr.Save());
            if (compress && current.Encoding == Encoding.V2_0)
            {
                response.CompressPayload();
            }
            return response;
        }

        /// <summary>Creates a new outgoing response frame with a return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="compress">True if the response should be compressed, false otherwise.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
        /// <param name="value">The return value to marshal, when the response frame contains multiple return
        /// values they must be passed in a tuple.</param>
        /// <param name="writer">A delegate that must write the value to the frame.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(
            Current current,
            bool compress,
            FormatType? format,
            in T value,
            OutputStreamValueWriter<T> writer)
            where T : struct
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, compress, format);
            writer(ostr, value);
            response.Finish(ostr.Save());
            if (compress && current.Encoding == Encoding.V2_0)
            {
                response.CompressPayload();
            }
            return response;
        }

        /// <summary>Creates a new outgoing response frame with the given payload.</summary>
        /// <param name="request">The incoming request for which this constructor creates a response.</param>
        /// <param name="data">The payload for this response frame.</param>
        public OutgoingResponseFrame(IncomingRequestFrame request, ArraySegment<byte> data)
            : this(request.Protocol, request.Encoding)
        {
            bool hasEncapsulation = false;

            if (Protocol == Protocol.Ice1)
            {
                byte b = data[0];
                if (b > 7)
                {
                    throw new ArgumentException($"invalid ice1 reply status `{b}' in payload", nameof(data));
                }

                if (b <= (byte)ReplyStatus.UserException)
                {
                    hasEncapsulation = true;
                }
                else
                {
                    Encoding = Encoding.V1_1;
                }
            }
            else
            {
                byte b = data[0];
                if (b > 1)
                {
                    throw new ArgumentException($"invalid ice2 result type `{b}' in payload", nameof(data));
                }
                hasEncapsulation = true;
            }

            // Check the encapsulation if there is one, and read the payload's encoding.
            if (hasEncapsulation)
            {
                int size;
                int sizeLength;

                // The encoding of the request payload and response payload should usually be the same, however
                // this "forwarding" constructor tolerates mismatches, and sets Encoding to the value in the
                // encapsulation.
                (size, sizeLength, Encoding) =
                    data.AsReadOnlySpan(1).ReadEncapsulationHeader(Protocol.GetEncoding());

                if (1 + sizeLength + size != data.Count)
                {
                    throw new ArgumentException(
                        $"{data.Count - 1 - sizeLength - size} extra bytes in response payload",
                        nameof(data));
                }
            }

            Payload.Add(data);
            if (hasEncapsulation)
            {
                _encapsulationStart = new OutputStream.Position(Payload.Count - 1, 1);
                Finish(new OutputStream.Position(Payload.Count - 1, data.Count));
            }
        }

        /// <summary>Creates a response frame that represents a failure and contains an exception.</summary>
        /// <param name="request">The incoming request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(IncomingRequestFrame request, RemoteException exception)
            : this(request.Protocol, request.Encoding)
        {
            ReplyStatus replyStatus = ReplyStatus.UserException;
            if (Encoding == Encoding.V1_1)
            {
                replyStatus = exception switch
                {
                    ObjectNotExistException _ => ReplyStatus.ObjectNotExistException,
                    OperationNotExistException _ => ReplyStatus.OperationNotExistException,
                    UnhandledException _ => ReplyStatus.UnknownLocalException,
                    _ => ReplyStatus.UserException
                };
            }

            OutputStream ostr;
            if (Protocol == Protocol.Ice2 || replyStatus == ReplyStatus.UserException)
            {
                // Write ResultType.Failure or ReplyStatus.UserException (both have the same value, 1) followed by an
                // encapsulation.
                byte[] buffer = new byte[256];
                buffer[0] = (byte)ResultType.Failure;
                Payload.Add(buffer);

                ostr = new OutputStream(Protocol.GetEncoding(),
                                        Payload,
                                        new OutputStream.Position(0, 1),
                                        Encoding,
                                        FormatType.Sliced);

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V1_1)
                {
                    // The first byte of the encapsulation data is the actual ReplyStatus
                    ostr.WriteByte((byte)replyStatus);
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1 && (byte)replyStatus > (byte)ReplyStatus.UserException);
                ostr = new OutputStream(Ice1Definitions.Encoding, Payload); // not an encapsulation
                ostr.WriteByte((byte)replyStatus);
            }

            if (Encoding == Encoding.V1_1)
            {
                switch (replyStatus)
                {
                    case ReplyStatus.ObjectNotExistException:
                    case ReplyStatus.OperationNotExistException:
                        var dispatchException = (DispatchException)exception;
                        dispatchException.Identity.IceWrite(ostr);
                        ostr.WriteFacet(dispatchException.Facet);
                        ostr.WriteString(dispatchException.Operation);
                        break;

                    case ReplyStatus.UnknownLocalException:
                        ostr.WriteString(exception.Message);
                        break;

                    default:
                        ostr.WriteException(exception);
                        break;
                }
            }
            else
            {
                ostr.WriteException(exception);
            }
            Finish(ostr.Save());
        }

        private static (OutgoingResponseFrame ResponseFrame, OutputStream Ostr) PrepareReturnValue(
            Current current,
            bool compress,
            FormatType? format)
        {
            var response = new OutgoingResponseFrame(current.Protocol,
                                                     current.Encoding,
                                                     compress,
                                                     current.Communicator.CompressionLevel,
                                                     current.Communicator.CompressionMinSize);

            // Write result type Success or reply status OK (both have the same value, 0) followed by an encapsulation.
            byte[] buffer = new byte[256];
            buffer[0] = (byte)ResultType.Success;
            response.Payload.Add(buffer);
            response._encapsulationStart = new OutputStream.Position(0, 1);
            var ostr = new OutputStream(response.Protocol.GetEncoding(),
                                        response.Payload,
                                        response._encapsulationStart,
                                        response.Encoding,
                                        format ?? current.Communicator.DefaultFormat);
            return (response, ostr);
        }

        private OutgoingResponseFrame(
            Protocol protocol,
            Encoding encoding,
            bool compress = false,
            CompressionLevel compressionLevel = CompressionLevel.Fastest,
            int compressionMinSize = 100,
            List<ArraySegment<byte>>? data = null)
            : base(protocol,
                   encoding,
                   compress,
                   compressionLevel,
                   compressionMinSize,
                   data ?? new List<ArraySegment<byte>>()) =>
                Size = Payload?.GetByteCount() ?? 0;
    }
}
