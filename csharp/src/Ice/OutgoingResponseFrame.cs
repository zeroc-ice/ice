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
            return new OutgoingResponseFrame(current.Protocol,
                                             current.Encoding,
                                             data,
                                             new OutputStream.Position(0, 1),
                                             ostr.Tail);
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
            : this(request.Protocol, request.Encoding)
        {
            bool hasEncapsulation = false;

            ArraySegment<byte> data = response.Data;
            if (Protocol == Protocol.Ice1)
            {
                byte b = data[0];
                if (b > 7)
                {
                    throw new ArgumentException($"invalid ice1 reply status `{b}' in payload", nameof(response));
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
                    throw new ArgumentException($"invalid ice2 result type `{b}' in payload", nameof(response));
                }
                hasEncapsulation = true;
            }

            Data.Add(data);
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

                int remaining = data.Count - 1 - sizeLength - size;
                if (remaining > 0)
                {
                    // For ice1 the end of the encapsulation marks the end of the data, for ice2 the binary context
                    // is encoded after the encapsulation
                    if (Protocol == Protocol.Ice1)
                    {
                        throw new ArgumentException($"{remaining} extra bytes in response payload", nameof(data));
                    }
                    else
                    {
                        _binaryContextOstr = new OutputStream(Encoding.V2_0,
                                                              Data,
                                                              new OutputStream.Position(Data.Count - 1, data.Count));
                        foreach (int key in response.BinaryContext.Keys)
                        {
                            _binaryContextKeys.Add(key);
                        }
                    }
                }
                _encapsulationStart = new OutputStream.Position(Data.Count - 1, 1);
                FinishEncapsulation(new OutputStream.Position(Data.Count - 1, 1 + sizeLength + size));
            }
            else
            {
                FinishEncapsulation(new OutputStream.Position(Data.Count - 1, data.Count));
                Size = Data.GetByteCount();
                IsSealed = true;
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
            OutputStream.Position encapsulationStart,
            OutputStream.Position encapsulationEnd)
            : this(protocol, encoding, data: data)
        {
            _encapsulationStart = encapsulationStart;
            FinishEncapsulation(encapsulationEnd);
        }

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
            response._encapsulationStart = new OutputStream.Position(0, 1);
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
        }
    }
}
