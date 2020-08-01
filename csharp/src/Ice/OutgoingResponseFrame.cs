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

        /// <summary>The reply status <see cref="ReplyStatus"/> when the frame's payload encoding is 1.1. Otherwise,
        /// always ReplyStatus.OK.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The result type, when known.</summary>
        public ResultType? ResultType { get; }

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
            if (Protocol == Protocol.Ice1)
            {
                ReplyStatus = AsReplyStatus(payload[0]);

                // Check the encapsulation if there is one.
                if (ReplyStatus == ReplyStatus.OK || ReplyStatus == ReplyStatus.UserException)
                {
                    int size;
                    int sizeLength;

                    // The encoding of the request payload and response payload should usually be the same, however
                    // this "forwarding" constructor tolerates mismatches, and sets Encoding to the value in the
                    // encapsulation (when there is an encapsulation).
                    (size, sizeLength, Encoding) =
                        payload.AsReadOnlySpan(1).ReadEncapsulationHeader(Ice1Definitions.Encoding);

                    if (1 + sizeLength + size != payload.Count)
                    {
                        throw new ArgumentException(
                            $"{payload.Count - 1 - sizeLength - size} extra bytes in response payload",
                            nameof(payload));
                    }
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice2);
                int size;
                int sizeLength;

                (size, sizeLength, Encoding) =
                    payload.AsReadOnlySpan().ReadEncapsulationHeader(Ice2Definitions.Encoding);

                if (sizeLength + size != payload.Count)
                {
                    throw new ArgumentException(
                        $"{payload.Count - 1 - sizeLength - size} extra bytes in response payload",
                        nameof(payload));
                }

                if (Encoding == Encoding.V1_1)
                {
                    // The reply status is the first byte after the encaps header.
                    ReplyStatus = AsReplyStatus(payload[sizeLength + 2]);
                }
            }

            if (Encoding == Encoding.V1_1)
            {
                ResultType = ReplyStatus == ReplyStatus.OK ?
                    ZeroC.Ice.ResultType.Success : ZeroC.Ice.ResultType.Failure;
            }
            // else Result Type remains null (= unknown)

            Data.Add(payload);
            Size = Data.GetByteCount();
            IsSealed = true;

            static ReplyStatus AsReplyStatus(byte b) =>
                b <= 7 ? (ReplyStatus)b :
                    throw new ArgumentException($"invalid ice1 reply status `{b}' in payload", nameof(payload));
        }

        /// <summary>Creates a response frame that represents "failure" and contains an exception.</summary>
        /// <param name="request">The incoming request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(IncomingRequestFrame request, RemoteException exception)
            : this(request, ZeroC.Ice.ResultType.Failure)
        {
            OutputStream ostr;

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
            // else ReplyStatus remains OK, as set by this(request) constructor.

            if (Protocol == Protocol.Ice1)
            {
                // Always reply status followed by data that depends on this byte status.
                // replyStatus is always ReplyStatus.OK for encoding 2.0
                if (ReplyStatus == ReplyStatus.UserException || ReplyStatus == ReplyStatus.OK)
                {
                    byte[] buffer = new byte[256];
                    buffer[0] = (byte)ReplyStatus;
                    Data.Add(buffer);
                    // Start encapsulation.
                    ostr = new OutputStream(Ice1Definitions.Encoding,
                                            Data,
                                            new OutputStream.Position(0, 1),
                                            Encoding,
                                            FormatType.Sliced);
                }
                else
                {
                    ostr = new OutputStream(Ice1Definitions.Encoding, Data); // not an encapsulation
                    ostr.WriteByte((byte)ReplyStatus);
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice2);
                // Always an encapsulation.
                ostr = new OutputStream(Ice2Definitions.Encoding,
                                        Data,
                                        default,
                                        Encoding,
                                        FormatType.Sliced);

                if (Encoding == Encoding.V1_1)
                {
                    // First byte of encapsulation is reply status
                    ostr.WriteByte((byte)ReplyStatus);
                }
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
                Debug.Assert(Encoding == Encoding.V2_0);
                ostr.WriteByte((byte)ResultType!.Value);
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
            var response = new OutgoingResponseFrame(current.IncomingRequestFrame, ZeroC.Ice.ResultType.Success);

            OutputStream ostr;
            if (response.Protocol == Protocol.Ice1)
            {
                // Always byte status OK followed by encapsulation.
                byte[] buffer = new byte[256];
                buffer[0] = (byte)response.ReplyStatus;
                response.Data.Add(buffer);
                ostr = new OutputStream(Ice1Definitions.Encoding,
                                        response.Data,
                                        new OutputStream.Position(0, 1),
                                        response.Encoding,
                                        format ?? current.Communicator.DefaultFormat);
            }
            else
            {
                Debug.Assert(response.Protocol == Protocol.Ice2);
                // Always an encapsulation.
                ostr = new OutputStream(Ice2Definitions.Encoding,
                                        response.Data,
                                        default,
                                        response.Encoding,
                                        format ?? current.Communicator.DefaultFormat);

                if (response.Encoding == Encoding.V1_1)
                {
                    // First byte is reply status
                    ostr.WriteByte((byte)response.ReplyStatus);
                }
            }

            if (response.Encoding == Encoding.V2_0)
            {
                ostr.WriteByte((byte)response.ResultType!.Value);
            }

            return (response, ostr);
        }

        private OutgoingResponseFrame(IncomingRequestFrame request, ResultType? resultType = null)
        {
            Protocol = request.Protocol;
            Encoding = request.Encoding;
            ReplyStatus = ReplyStatus.OK; // can be overwritten by exception constructor
            ResultType = resultType;

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
