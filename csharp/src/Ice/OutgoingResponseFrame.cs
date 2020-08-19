//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame : OutgoingFrame
    {
        public override Encoding Encoding { get; }

        /// <summary>The result type; see <see cref="ZeroC.Ice.ResultType"/>.</summary>
        public ResultType ResultType => Data[0][0] == 0 ? ResultType.Success : ResultType.Failure;

        /// <summary>Creates a new outgoing response frame with a void return value.</summary>
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
            FormatType format,
            T value,
            OutputStreamWriter<T> writer)
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, compress, format);
            writer(ostr, value);
            response.FinishEncapsulation(ostr.Finish());
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
            FormatType format,
            in T value,
            OutputStreamValueWriter<T> writer)
            where T : struct
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, compress, format);
            writer(ostr, value);
            response.FinishEncapsulation(ostr.Finish());
            if (compress && current.Encoding == Encoding.V2_0)
            {
                response.CompressPayload();
            }
            return response;
        }

        /// <summary>Creates a new outgoing response frame from the given incoming response frame.</summary>
        /// <param name="request">The incoming request for which this constructor creates an outgoing response frame.
        /// </param>
        /// <param name="response">The incoming response for which this constructor creates an outgoing response frame.
        /// </param>
        internal OutgoingResponseFrame(IncomingRequestFrame request, IncomingResponseFrame response)
            : this(request.Protocol, response.Encoding)
        {
            if (Protocol == response.Protocol)
            {
                Data.Add(response.Data); // payload and binary context
                if (response.Encapsulation.Count > 0)
                {
                    _encapsulationEnd = new OutputStream.Position(0,
                        response.Encapsulation.Offset - response.Data.Offset + response.Encapsulation.Count);
                }
                if (response.BinaryContext.Count > 0)
                {
                    _binaryContextOstr = new OutputStream(Encoding.V2_0,
                                                          Data,
                                                          new OutputStream.Position(0, response.Data.Count));
                    foreach (int key in response.BinaryContext.Keys)
                    {
                        _binaryContextKeys.Add(key);
                    }
                }
            }
            else
            {
                // TODO: is there a more elegant way to get this value?
                int sizeLength = response.Protocol == Protocol.Ice1 ? 4 : (1 << (response.Encapsulation[0] & 0x03));

                // Create a small buffer to hold the result type or reply status plus the encapsulation header.
                Debug.Assert(Data.Count == 0);
                byte[] buffer = new byte[8];
                Data.Add(buffer);

                if (response.ResultType == ResultType.Failure && Encoding == Encoding.V1_1)
                {
                    // When the response carries a failure encoded with 1.1, we need to perform a small adjustment
                    // between ice1 and ice2 response frames.
                    // ice1: [failure reply status][encapsulation or special exception]
                    // ice2: [failure][encapsulation with reply status + encapsulation bytes or special exception]
                    // There is no such adjustment with other encoding, or when the response does not carry a failure.

                    if (Protocol == Protocol.Ice1)
                    {
                        Debug.Assert(response.Protocol == Protocol.Ice2);

                        // Read the reply status byte immediately after the encapsulation header; +2 corresponds to the
                        // encoding in the header
                        byte b = response.Encapsulation[sizeLength + 2];
                        ReplyStatus replyStatus = b >= 1 && b <= 7 ? (ReplyStatus)b :
                            throw new InvalidDataException(
                                $"received ice2 response frame with invalid reply status `{b}'");

                        buffer[0] = b;
                        if (replyStatus == ReplyStatus.UserException)
                        {
                            OutputStream.Position tail =
                                OutputStream.WriteEncapsulationHeader(Data,
                                                                      _encapsulationStart,
                                                                      Ice1Definitions.Encoding,
                                                                      response.Encapsulation.Count - sizeLength - 1,
                                                                      Encoding);
                            Debug.Assert(tail.Segment == 0);
                            Data[0] = Data[0].Slice(0, tail.Offset);
                        }
                        else
                        {
                            Data[0] = Data[0].Slice(0, 1);
                        }
                        // sizeLength + 2 to skip the encapsulation header, then + 1 to skip the reply status byte
                        Data.Add(response.Encapsulation.Slice(sizeLength + 2 + 1));
                        if (replyStatus == ReplyStatus.UserException)
                        {
                            _encapsulationEnd = new OutputStream.Position(1, Data[1].Count);
                        }
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
                                                                      _encapsulationStart,
                                                                      Ice2Definitions.Encoding,
                                                                      response.Encapsulation.Count - sizeLength + 1,
                                                                      Encoding);
                            buffer[tail.Offset++] = (byte)replyStatus;
                            Data[0] = Data[0].Slice(0, tail.Offset);
                            Data.Add(response.Encapsulation.Slice(sizeLength + 2));
                        }
                        else
                        {
                            OutputStream.Position tail =
                                OutputStream.WriteEncapsulationHeader(Data,
                                                                      _encapsulationStart,
                                                                      Ice2Definitions.Encoding,
                                                                      response.Payload.Count,
                                                                      Encoding);
                            buffer[tail.Offset++] = (byte)replyStatus;
                            Data[0] = Data[0].Slice(0, tail.Offset);
                            Data.Add(response.Payload.Slice(1));
                        }
                        _encapsulationEnd = new OutputStream.Position(1, Data[1].Count);
                    }
                }
                else
                {
                    buffer[0] = (byte)response.ResultType;
                    OutputStream.Position tail =
                                OutputStream.WriteEncapsulationHeader(Data,
                                                                      _encapsulationStart,
                                                                      Protocol.GetEncoding(),
                                                                      response.Encapsulation.Count - sizeLength,
                                                                      Encoding);
                    Data[0] = Data[0].Slice(0, tail.Offset);
                    Data.Add(response.Encapsulation.Slice(sizeLength + 2));
                    _encapsulationEnd = new OutputStream.Position(1, Data[1].Count);
                }
            }

            Size = Data.GetByteCount();
            IsSealed = Protocol == Protocol.Ice1;
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
                                        new OutputStream.Position(0, 1),
                                        Encoding,
                                        FormatType.Sliced);
                _encapsulationStart = ostr.Tail;
                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V1_1)
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

            OutputStream.Position end = ostr.Finish();
            FinishEncapsulation(end);
            if (!hasEncapsulation)
            {
                Data[^1] = Data[^1].Slice(0, end.Offset);
                Size = Data.GetByteCount();
                IsSealed = true;
            }
        }

        internal OutgoingResponseFrame(
            Protocol protocol,
            Encoding encoding,
            List<ArraySegment<byte>> data,
            OutputStream.Position encapsulationEnd)
            : this(protocol, encoding, data: data) => _encapsulationEnd = encapsulationEnd;

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
                                        response._encapsulationStart,
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

            // A response encapsulation (when there is one) always start at position 1. Note that we consider there is
            // an encapsulation only when _encapsulationEnd is not null.
            _encapsulationStart = new OutputStream.Position(0, 1);
        }
    }
}
