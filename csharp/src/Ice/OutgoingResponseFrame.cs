// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame : OutgoingFrame
    {
        /// <summary>The response payload encoding.</summary>
        public override Encoding Encoding { get; }

        /// <summary>The result type; see <see cref="Ice.ResultType"/>.</summary>
        public ResultType ResultType => Data[0][0] == 0 ? ResultType.Success : ResultType.Failure;

        // When a response frame contains an encapsulation, it always starts at position 1 of the first segment,
        // and the first segment has always at least 2 bytes.
        private static readonly OutputStream.Position EncapsulationStart = new OutputStream.Position(0, 1);

        private readonly ArraySegment<byte> _defaultBinaryContext;

        /// <summary>Creates a new <see cref="OutgoingResponseFrame"/> for an operation that returns void.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithVoidReturnValue(Current current)
        {
            var data = new List<ArraySegment<byte>>();
            var ostr = new OutputStream(current.Protocol.GetEncoding(), data);
            ostr.WriteByte((byte)ResultType.Success);
            _ = ostr.WriteEmptyEncapsulation(current.Encoding);
            return new OutgoingResponseFrame(current.Protocol, current.Encoding, data, ostr.Tail);
        }

        /// <summary>Creates a new <see cref="OutgoingResponseFrame"/> for an operation with a non-tuple non-struct
        /// return type.</summary>
        /// <typeparam name="T">The type of the return value.</typeparam>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="compress">True if the response should be compressed, false otherwise.</param>
        /// <param name="format">The format to use when writing class instances in case <c>returnValue</c> contains
        /// class instances.</param>
        /// <param name="returnValue">The return value to write into the frame.</param>
        /// <param name="writer">The <see cref="OutputStreamWriter{T}"/> that writes the return value into the frame.
        /// </param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(
            Current current,
            bool compress,
            FormatType format,
            T returnValue,
            OutputStreamWriter<T> writer)
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, compress, format);
            writer(ostr, returnValue);
            response.PayloadEnd = ostr.Finish();
            if (compress && current.Encoding == Encoding.V20)
            {
                response.CompressPayload();
            }
            return response;
        }

        /// <summary>Creates a new <see cref="OutgoingResponseFrame"/> for an operation with a tuple or struct return
        /// type.</summary>
        /// <typeparam name="T">The type of the return value.</typeparam>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="compress">True if the response should be compressed, false otherwise.</param>
        /// <param name="format">The format to use when writing class instances in case <c>returnValue</c> contains
        /// class instances.</param>
        /// <param name="returnValue">The return value to write into the frame.</param>
        /// <param name="writer">The <see cref="OutputStreamWriter{T}"/> that writes the return value into the frame.
        /// </param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(
            Current current,
            bool compress,
            FormatType format,
            in T returnValue,
            OutputStreamValueWriter<T> writer)
            where T : struct
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, compress, format);
            writer(ostr, returnValue);
            response.PayloadEnd = ostr.Finish();
            if (compress && current.Encoding == Encoding.V20)
            {
                response.CompressPayload();
            }
            return response;
        }

        /// <summary>Constructs an outgoing response frame from the given incoming response frame.</summary>
        /// <param name="request">The request for which this constructor creates a response.</param>
        /// <param name="response">The incoming response used to construct the new outgoing response frame.</param>
        /// <param name="forwardBinaryContext">When true (the default), the new frame uses the incoming response frame's
        /// binary context as a fallback - all the entries in this binary context are added before the frame is sent,
        /// except for entries previously added by dispatch interceptors.</param>
        public OutgoingResponseFrame(
            IncomingRequestFrame request,
            IncomingResponseFrame response,
            bool forwardBinaryContext = true)
            : this(request.Protocol, response.Encoding)
        {
            if (Protocol == response.Protocol)
            {
                if (Protocol == Protocol.Ice1)
                {
                    Data.Add(response.Data);
                    PayloadEnd = new OutputStream.Position(0, response.Data.Count);
                }
                else
                {
                    // i.e. result type and encapsulation but not the binary context
                    Data.Add(response.Payload);
                    PayloadEnd = new OutputStream.Position(0, response.Payload.Count);

                    if (forwardBinaryContext)
                    {
                        _defaultBinaryContext = response.Data.Slice(response.Payload.Count); // can be empty
                    }
                }
            }
            else
            {
                int sizeLength = response.Protocol == Protocol.Ice1 ? 4 : response.Payload[1].ReadSizeLength20();

                // Create a small buffer to hold the result type or reply status plus the encapsulation header.
                Debug.Assert(Data.Count == 0);
                byte[] buffer = new byte[8];
                Data.Add(buffer);

                if (response.ResultType == ResultType.Failure && Encoding == Encoding.V11)
                {
                    // When the response carries a failure encoded with 1.1, we need to perform a small adjustment
                    // between ice1 and ice2 response frames.
                    // ice1: [failure reply status][encapsulation or special exception]
                    // ice2: [failure][encapsulation with reply status + encapsulation bytes or special exception]
                    // There is no such adjustment with other encoding, or when the response does not carry a failure.

                    if (Protocol == Protocol.Ice1)
                    {
                        Debug.Assert(response.Protocol == Protocol.Ice2);

                        // Read the reply status byte immediately after the encapsulation header; + 2 corresponds to the
                        // encoding in the header.
                        byte b = response.Payload[1 + sizeLength + 2];
                        ReplyStatus replyStatus = b >= 1 && b <= 7 ? (ReplyStatus)b :
                            throw new InvalidDataException(
                                $"received ice2 response frame with invalid reply status `{b}'");

                        buffer[0] = b;
                        if (replyStatus == ReplyStatus.UserException)
                        {
                            OutputStream.Position tail =
                                OutputStream.WriteEncapsulationHeader(Data,
                                                                      EncapsulationStart,
                                                                      Ice1Definitions.Encoding,
                                                                      response.Payload.Count - 1 - sizeLength - 1,
                                                                      Encoding);
                            Debug.Assert(tail.Segment == 0);
                            Data[0] = Data[0].Slice(0, tail.Offset);
                        }
                        else
                        {
                            // TODO: this code is currently unreachable because the application never gets an incoming
                            // response frame carrying a system exception - this system exception is always thrown.
                            Debug.Assert(false);

                            Data[0] = Data[0].Slice(0, 1);
                        }
                        // 1 for the result type in the response, then sizeLength + 2 to skip the encapsulation header,
                        // then + 1 to skip the reply status byte
                        Data.Add(response.Payload.Slice(1 + sizeLength + 2 + 1));
                    }
                    else
                    {
                        Debug.Assert(response.Protocol == Protocol.Ice1);
                        buffer[0] = (byte)ResultType.Failure;
                        var replyStatus = (ReplyStatus)response.Payload[0];
                        if (replyStatus == ReplyStatus.UserException)
                        {
                            OutputStream.Position tail =
                                OutputStream.WriteEncapsulationHeader(Data,
                                                                      EncapsulationStart,
                                                                      Ice2Definitions.Encoding,
                                                                      response.Payload.Count - 1 - sizeLength + 1,
                                                                      Encoding);
                            buffer[tail.Offset++] = (byte)replyStatus;
                            Data[0] = Data[0].Slice(0, tail.Offset);
                            Data.Add(response.Payload.Slice(1 + sizeLength + 2));
                        }
                        else
                        {
                            // TODO: this code is currently unreachable because the application never gets an incoming
                            // response frame carrying a system exception - this system exception is always thrown.
                            Debug.Assert(false);

                            OutputStream.Position tail =
                                OutputStream.WriteEncapsulationHeader(Data,
                                                                      EncapsulationStart,
                                                                      Ice2Definitions.Encoding,
                                                                      response.Payload.Count,
                                                                      Encoding);
                            buffer[tail.Offset++] = (byte)replyStatus;
                            Data[0] = Data[0].Slice(0, tail.Offset);
                            Data.Add(response.Payload);
                        }
                    }
                }
                else
                {
                    buffer[0] = (byte)response.ResultType;
                    OutputStream.Position tail =
                                OutputStream.WriteEncapsulationHeader(Data,
                                                                      EncapsulationStart,
                                                                      Protocol.GetEncoding(),
                                                                      response.Payload.Count - 1 - sizeLength,
                                                                      Encoding);
                    Data[0] = Data[0].Slice(0, tail.Offset);
                    Data.Add(response.Payload.Slice(1 + sizeLength + 2));
                }

                // There is never a binary context in this case.
                Debug.Assert(Data.Count == 2);
                PayloadEnd = new OutputStream.Position(1, Data[1].Count);
            }

            Size = Data.GetByteCount();
            IsSealed = Protocol == Protocol.Ice1;
        }

        /// <summary>Constructs a response frame that represents a failure and contains an exception.</summary>
        /// <param name="request">The incoming request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(IncomingRequestFrame request, RemoteException exception)
            : this(request.Protocol, request.Encoding)
        {
            ReplyStatus replyStatus = ReplyStatus.UserException;
            if (Encoding == Encoding.V11)
            {
                replyStatus = exception switch
                {
                    ObjectNotExistException _ => ReplyStatus.ObjectNotExistException,
                    OperationNotExistException _ => ReplyStatus.OperationNotExistException,
                    UnhandledException _ => ReplyStatus.UnknownLocalException,
                    _ => ReplyStatus.UserException
                };
            }

            bool hasEncapsulation;
            OutputStream ostr;
            if (Protocol == Protocol.Ice2 || replyStatus == ReplyStatus.UserException)
            {
                // Write ResultType.Failure or ReplyStatus.UserException (both have the same value, 1) followed by an
                // encapsulation.
                byte[] buffer = new byte[256];
                buffer[0] = (byte)ResultType.Failure;
                Data.Add(buffer);

                ostr = new OutputStream(Protocol.GetEncoding(),
                                        Data,
                                        EncapsulationStart,
                                        Encoding,
                                        FormatType.Sliced);

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V11)
                {
                    // The first byte of the encapsulation data is the actual ReplyStatus
                    ostr.WriteByte((byte)replyStatus);
                }
                hasEncapsulation = true;
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1 && (byte)replyStatus > (byte)ReplyStatus.UserException);
                ostr = new OutputStream(Ice1Definitions.Encoding, Data); // not an encapsulation
                ostr.WriteByte((byte)replyStatus);
                hasEncapsulation = false;
            }

            if (Encoding == Encoding.V11)
            {
                switch (replyStatus)
                {
                    case ReplyStatus.ObjectNotExistException:
                    case ReplyStatus.OperationNotExistException:
                        var dispatchException = (DispatchException)exception;
                        dispatchException.Identity.IceWrite(ostr);
                        ostr.WriteFacet11(dispatchException.Facet);
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

            PayloadEnd = ostr.Finish();
            if (!hasEncapsulation)
            {
                Data[^1] = Data[^1].Slice(0, PayloadEnd.Offset);
                Size = Data.GetByteCount();
                IsSealed = true;
            }
        }

        internal OutgoingResponseFrame(
            Protocol protocol,
            Encoding encoding,
            List<ArraySegment<byte>> data,
            OutputStream.Position encapsulationEnd)
            : this(protocol, encoding, data: data) => PayloadEnd = encapsulationEnd;

        private protected override ArraySegment<byte> GetDefaultBinaryContext() => _defaultBinaryContext;

        private static (OutgoingResponseFrame ResponseFrame, OutputStream Ostr) PrepareReturnValue(
            Current current,
            bool compress,
            FormatType format)
        {
            var response = new OutgoingResponseFrame(current.Protocol,
                                                     current.Encoding,
                                                     compress,
                                                     current.Communicator.CompressionLevel,
                                                     current.Communicator.CompressionMinSize);

            // Write result type Success or reply status OK (both have the same value, 0) followed by an encapsulation.
            byte[] buffer = new byte[256];
            buffer[0] = (byte)ResultType.Success;
            response.Data.Add(buffer);
            var ostr = new OutputStream(response.Protocol.GetEncoding(),
                                        response.Data,
                                        EncapsulationStart,
                                        response.Encoding,
                                        format);
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
                   compress,
                   compressionLevel,
                   compressionMinSize,
                   data ?? new List<ArraySegment<byte>>())
        {
            Encoding = encoding;
            Size = Data?.GetByteCount() ?? 0;
            PayloadStart = default;
        }
    }
}
