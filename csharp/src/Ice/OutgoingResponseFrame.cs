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
    public sealed class OutgoingResponseFrame
    {
        /// <summary>The encoding of the frame payload</summary>
        public Encoding Encoding { get; }

        /// <summary>True for a sealed frame, false otherwise. Once sealed, a frame is read-only.</summary>
        public bool IsSealed { get; private set; }

        /// <summary>Returns a list of array segments with the contents of the frame payload.</summary>
        public IList<ArraySegment<byte>> Payload => Data;

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The reply status; see <see cref="ZeroC.Ice.ReplyStatus"/>.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The result type; see <see cref="ZeroC.Ice.ResultType"/>.</summary>
        public ResultType ResultType => Data[0][0] == 0 ? ResultType.Success : ResultType.Failure;

        /// <summary>The frame byte count.</summary>
        public int Size { get; private set; }

        // The compression status from the incoming request.
        internal byte CompressionStatus { get; }

        // Contents of the Frame
        internal List<ArraySegment<byte>> Data { get; private set; }

        private static readonly ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), OutgoingResponseFrame>
            _cachedVoidReturnValueFrames =
                new ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), OutgoingResponseFrame>();

        /// <summary>Creates a new outgoing response with a void return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithVoidReturnValue(Current current) =>
            _cachedVoidReturnValueFrames.GetOrAdd((current.Protocol, current.Encoding), key =>
            {
                (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, format: null);
                ostr.Save();
                response.Finish();
                return response;
            });

        /// <summary>Creates a new outgoing response frame with a return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
        /// <param name="value">The return value to marshal.</param>
        /// <param name="writer">A delegate that must write the value to the frame.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(Current current,
                                                               FormatType? format,
                                                               T value,
                                                               OutputStreamWriter<T> writer)
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, format);
            writer(ostr, value);
            ostr.Save();
            response.Finish();
            return response;
        }

        /// <summary>Creates a new outgoing response frame with a return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
        /// <param name="value">The return value to marshal, when the response frame contains multiple return
        /// values they must be passed in a tuple.</param>
        /// <param name="writer">A delegate that must write the value to the frame.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(Current current,
                                                               FormatType? format,
                                                               in T value,
                                                               OutputStreamValueWriter<T> writer)
            where T : struct
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, format);
            writer(ostr, value);
            ostr.Save();
            response.Finish();
            return response;
        }

        /// <summary>Creates a new outgoing response frame with the given payload.</summary>
        /// <param name="request">The incoming request for which this constructor creates a response.</param>
        /// <param name="payload">The payload for this response frame.</param>
        public OutgoingResponseFrame(IncomingRequestFrame request, ArraySegment<byte> payload)
            : this(request)
        {
            // Always reply status byte or result type byte followed by encapsulation.

            if (Protocol == Protocol.Ice1)
            {
                ReplyStatus = AsReplyStatus(payload[0]);
            }
            else
            {
                byte b = payload[0];
                if (b > 1)
                {
                    throw new ArgumentException($"invalid ice2 result type `{b}' in payload", nameof(payload));
                }
                ReplyStatus = (ReplyStatus)b; // OK or UserException, but can be fixed later, see below
            }

            // Check the encapsulation if there is one.
            if (Protocol == Protocol.Ice2 || ReplyStatus == ReplyStatus.OK || ReplyStatus == ReplyStatus.UserException)
            {
                int size;
                int sizeLength;

                // The encoding of the request payload and response payload should usually be the same, however
                // this "forwarding" constructor tolerates mismatches, and sets Encoding to the value in the
                // encapsulation.
                (size, sizeLength, Encoding) =
                    payload.AsReadOnlySpan(1).ReadEncapsulationHeader(Protocol.GetEncoding());

                if (1 + sizeLength + size != payload.Count)
                {
                    throw new ArgumentException(
                        $"{payload.Count - 1 - sizeLength - size} extra bytes in response payload",
                        nameof(payload));
                }

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V1_1 && ReplyStatus == ReplyStatus.UserException)
                {
                    // the first byte of the encapsulation is the actual ReplyStatus
                    // "+ 2" corresponds to the 2 bytes for the encoding in the encapsulation header
                    ReplyStatus = AsReplyStatus(payload[1 + sizeLength + 2]);
                }
            }

            Data.Add(payload);
            Size = Data.GetByteCount();
            IsSealed = true;

            static ReplyStatus AsReplyStatus(byte b) =>
                b <= 7 ? (ReplyStatus)b :
                    throw new ArgumentException($"invalid ice1 reply status `{b}' in payload", nameof(payload));
        }

        /// <summary>Creates a response frame that represents a failure and contains an exception.</summary>
        /// <param name="request">The incoming request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(IncomingRequestFrame request, RemoteException exception)
            : this(request)
        {
            if (Encoding == Encoding.V1_1)
            {
                ReplyStatus = exception switch
                {
                    ObjectNotExistException _ => ReplyStatus.ObjectNotExistException,
                    OperationNotExistException _ => ReplyStatus.OperationNotExistException,
                    UnhandledException _ => ReplyStatus.UnknownLocalException,
                    _ => ReplyStatus.UserException
                };
            }
            else
            {
                ReplyStatus = ReplyStatus.UserException;
            }

            OutputStream ostr;
            if (Protocol == Protocol.Ice2 || ReplyStatus == ReplyStatus.UserException)
            {
                // Write ReplyStatus.UserException or ResultType.Failure followed by encapsulation
                byte[] buffer = new byte[256];
                Debug.Assert((byte)ReplyStatus.UserException == 1 && (byte)ResultType.Failure == 1);
                buffer[0] = 1;
                Data.Add(buffer);

                ostr = new OutputStream(Protocol.GetEncoding(),
                                        Data,
                                        new OutputStream.Position(0, 1),
                                        Encoding,
                                        FormatType.Sliced);

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V1_1)
                {
                    // The first byte of the encapsulation data is the actual ReplyStatus
                    ostr.WriteByte((byte)ReplyStatus);
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1 && (byte)ReplyStatus > (byte)ReplyStatus.UserException);
                ostr = new OutputStream(Ice1Definitions.Encoding, Data); // not an encapsulation
                ostr.WriteByte((byte)ReplyStatus);
            }

            if (Encoding == Encoding.V1_1)
            {
                switch (ReplyStatus)
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

            ostr.Save();
            Size = Data.GetByteCount();
            IsSealed = true;
        }

        private static (OutgoingResponseFrame ResponseFrame, OutputStream Ostr) PrepareReturnValue(
            Current current,
            FormatType? format)
        {
            var response = new OutgoingResponseFrame(current.IncomingRequestFrame);

            // Always OK or Success (i.e. 0) followed by encapsulation.
            byte[] buffer = new byte[256];
            Debug.Assert((byte)ReplyStatus.OK == 0 && (byte)ResultType.Success == 0);
            buffer[0] = 0;
            response.Data.Add(buffer);
            var ostr = new OutputStream(response.Protocol.GetEncoding(),
                                        response.Data,
                                        new OutputStream.Position(0, 1),
                                        response.Encoding,
                                        format ?? current.Communicator.DefaultFormat);
            return (response, ostr);
        }

        private OutgoingResponseFrame(IncomingRequestFrame request)
        {
            Protocol = request.Protocol;
            Encoding = request.Encoding;
            ReplyStatus = ReplyStatus.OK; // can be overwritten by exception constructor

            Data = new List<ArraySegment<byte>>();

            // No need to keep track of the compression status for Ice2, the compression is handled by
            // the encapsulation. We need it for Ice1, since the compression is handled by protocol.
            if (Protocol == Protocol.Ice1)
            {
                CompressionStatus = request.CompressionStatus;
            }
        }

        private void Finish()
        {
            Size = Data.GetByteCount();
            IsSealed = true;
        }
    }
}
