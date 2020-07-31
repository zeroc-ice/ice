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

        /// <summary>The frame reply status <see cref="ReplyStatus"/>. Applies only to ice1.</summary>
        public ReplyStatus ReplyStatus => Protocol == Protocol.Ice1 ? (ReplyStatus) Data[0][0] : ReplyStatus.OK;

        /// <summary>The frame byte count.</summary>
        public int Size { get; private set; }

        // The compression status from the incoming request.
        internal byte CompressionStatus { get; }
        // Contents of the Frame
        internal List<ArraySegment<byte>> Data { get; private set; }

        private static readonly ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), OutgoingResponseFrame>
            _cachedVoidReturnValueFrames =
                new ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), OutgoingResponseFrame>();

        /// <summary>Creates a new outgoing response frame with an OK reply status and a void return value.</summary>
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

        /// <summary>Creates a new outgoing response frame with an OK reply status and a return value.</summary>
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

        /// <summary>Creates a new outgoing response frame with an OK reply status and a return value.</summary>
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
        // TODO: add parameter such as "bool assumeOwnership" once we add memory pooling.
        // TODO: should we pass the payload as a list of segments, or maybe add a separate
        // ctor that accepts a list of segments instead of a single segment
        public OutgoingResponseFrame(IncomingRequestFrame request, ArraySegment<byte> payload)
            : this(request)
        {
            // TODO: do we need all these checks? If yes, add checks for ice2; if no, remove them from ice1.
            if (Protocol == Protocol.Ice1)
            {
                if (payload[0] == (byte)ReplyStatus.OK || payload[0] == (byte)ReplyStatus.UserException)
                {
                    (int size, int sizeLength, Encoding encapsEncoding) =
                        payload.AsReadOnlySpan(1).ReadEncapsulationHeader(Protocol.GetEncoding());

                    if (encapsEncoding != Encoding)
                    {
                        throw new ArgumentException(@$"the encoding of the response payload (`{encapsEncoding
                            }') must match the encoding of the request payload (`{Encoding}')",
                            nameof(payload));
                    }

                    if (1 + sizeLength + size != Payload.Count)
                    {
                        throw new ArgumentException(
                            $"{Payload.Count - 1 - sizeLength - size} extra bytes in response payload",
                            nameof(payload));
                    }
                }
            }

            Data.Add(payload);
            Size = Data.GetByteCount();
            IsSealed = true;
        }

        /// <summary>Creates a response frame that represents "failure" and contains an exception.</summary>
        /// <param name="request">The incoming request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(IncomingRequestFrame request, RemoteException exception)
            : this(request)
        {
            OutputStream ostr;
            ReplyStatus replyStatus = ReplyStatus.OK;

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

            if (Protocol == Protocol.Ice1)
            {
                // Always byte status followed by data that depends on this byte status.
                // replyStatus is always ReplyStatus.OK for encoding 2.0
                if (replyStatus == ReplyStatus.UserException || replyStatus == ReplyStatus.OK)
                {
                    byte[] buffer = new byte[256];
                    buffer[0] = (byte)replyStatus;
                    Data.Add(buffer);
                    ostr = new OutputStream(Ice1Definitions.Encoding,
                                            Data,
                                            new OutputStream.Position(0, 1),
                                            Encoding,
                                            FormatType.Sliced);
                }
                else
                {
                    ostr = new OutputStream(Ice1Definitions.Encoding, Data); // not an encapsulation
                    ostr.WriteByte((byte)replyStatus);
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

                // TODO: when Encoding = 1.1, add binary context with reply status.
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
                Debug.Assert(Encoding == Encoding.V2_0);
                ostr.WriteByte((byte)ResponseType.Failure);
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

            OutputStream ostr;
            if (response.Protocol == Protocol.Ice1)
            {
                // Always byte status OK followed by encapsulation.
                byte[] buffer = new byte[256];
                buffer[0] = (byte)ReplyStatus.OK;
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

                // TODO: when response.Encoding == 1.1, we could set the reply status binary context, unless we
                // consider that not set means OK.
            }

            if (response.Encoding == Encoding.V2_0)
            {
                ostr.WriteByte((byte)ResponseType.Success);
            }

            return (response, ostr);
        }

        private OutgoingResponseFrame(IncomingRequestFrame request, List<ArraySegment<byte>>? data = null)
        {
            Protocol = request.Protocol;
            Encoding = request.Encoding;
            // No need to keep track of the compression status for Ice2, the compression is handled by
            // the encapsulation. We need it for Ice1, since the compression is handled by protocol.
            if (Protocol == Protocol.Ice1)
            {
                CompressionStatus = request.CompressionStatus;
            }
            if (data == null)
            {
                Data = new List<ArraySegment<byte>>();
            }
            else
            {
                Data = data;
                Size = Data.GetByteCount();
                IsSealed = true;
            }
        }

        private void Finish()
        {
            Size = Data.GetByteCount();
            IsSealed = true;
        }
    }
}
