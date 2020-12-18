// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame : OutgoingFrame
    {
        /// <inheritdoc/>
        public override IReadOnlyDictionary<int, ReadOnlyMemory<byte>> InitialBinaryContext { get; } =
            ImmutableDictionary<int, ReadOnlyMemory<byte>>.Empty;

        /// <inheritdoc/>
        public override Encoding PayloadEncoding { get; }

        /// <summary>The result type; see <see cref="Ice.ResultType"/>.</summary>
        public ResultType ResultType => Payload[0][0] == 0 ? ResultType.Success : ResultType.Failure;

        // When a response frame contains an encapsulation, it always starts at position 1 of the first segment,
        // and the first segment has always at least 2 bytes.
        private static readonly OutputStream.Position _encapsulationStart = new OutputStream.Position(0, 1);

        /// <summary>Creates a new <see cref="OutgoingResponseFrame"/> for an operation that returns void.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithVoidReturnValue(Current current)
        {
            var response = new OutgoingResponseFrame(current.Protocol, current.Encoding);
            response.Payload.Add(current.Protocol.GetVoidReturnPayload(current.Encoding));
            return response;
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
            ostr.Finish();
            if (compress && current.Encoding == Encoding.V20)
            {
                response.CompressPayload();
            }
            return response;
        }

        /// <summary>Creates a new <see cref="OutgoingResponseFrame"/> for an operation with a single stream return
        /// value.</summary>
        /// <typeparam name="T">The type of the return value.</typeparam>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="compress">True if the response should be compressed, false otherwise.</param>
        /// <param name="format">The format to use when writing class instances in case <c>returnValue</c> contains
        /// class instances.</param>
        /// <param name="returnValue">The return value to write into the frame.</param>
        /// <param name="writer">The delegate that will send the stream return value.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        [System.Diagnostics.CodeAnalysis.SuppressMessage(
            "Microsoft.Performance",
            "CA1801: Review unused parameters",
            Justification = "TODO")]
        public static OutgoingResponseFrame WithReturnValue<T>(
            Current current,
            bool compress,
            FormatType format,
            T returnValue,
            Action<SocketStream, T, System.Threading.CancellationToken> writer)
        {
            OutgoingResponseFrame response = WithVoidReturnValue(current);
            // TODO: deal with compress, format and cancellation token
            response.StreamDataWriter = socketStream => writer(socketStream, returnValue, default);
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
            writer(ostr, in returnValue);
            ostr.Finish();
            if (compress && current.Encoding == Encoding.V20)
            {
                response.CompressPayload();
            }
            return response;
        }

        /// <summary>Creates a new <see cref="OutgoingResponseFrame"/> for an operation with a tuple return
        /// type where the tuple return type contains a stream return value.</summary>
        /// <typeparam name="T">The type of the return value.</typeparam>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="compress">True if the response should be compressed, false otherwise.</param>
        /// <param name="format">The format to use when writing class instances in case <c>returnValue</c> contains
        /// class instances.</param>
        /// <param name="returnValue">The return value to write into the frame.</param>
        /// <param name="writer">The delegate that writes the return value into the frame.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(
            Current current,
            bool compress,
            FormatType format,
            in T returnValue,
            OutputStreamValueWriterWithStreamable<T> writer)
            where T : struct
        {
            (OutgoingResponseFrame response, OutputStream ostr) = PrepareReturnValue(current, compress, format);
            // TODO: deal with compress, format and cancellation token
            response.StreamDataWriter = writer(ostr, in returnValue, default);
            ostr.Finish();
            if (compress && current.Encoding == Encoding.V20)
            {
                response.CompressPayload();
            }
            return response;
        }

        /// <summary>Constructs an outgoing response frame from the given incoming response frame. The new response will
        /// use the protocol of the <paramref name="request"/> and the encoding of <paramref name="response"/>.</summary>
        /// <param name="request">The request for which this constructor creates a response.</param>
        /// <param name="response">The incoming response used to construct the new outgoing response frame.</param>
        /// <param name="forwardBinaryContext">When true (the default), the new frame uses the incoming response frame's
        /// binary context as a fallback - all the entries in this binary context are added before the frame is sent,
        /// except for entries previously added by dispatch interceptors.</param>
        public OutgoingResponseFrame(
            IncomingRequestFrame request,
            IncomingResponseFrame response,
            bool forwardBinaryContext = true)
            : this(request.Protocol, response.PayloadEncoding)
        {
            if (Protocol == response.Protocol)
            {
                Payload.Add(response.Payload);
                if (Protocol == Protocol.Ice2 && forwardBinaryContext)
                {
                    // Don't forward RetryPolicy context
                    InitialBinaryContext =
                        response.BinaryContext.ToImmutableDictionary().Remove((int)BinaryContextKey.RetryPolicy);
                }
            }
            else
            {
                int sizeLength = response.Protocol == Protocol.Ice1 ? 4 : response.Payload[1].ReadSizeLength20();

                // Create a small buffer to hold the result type or reply status plus the encapsulation header.
                Debug.Assert(Payload.Count == 0);
                byte[] buffer = new byte[8];
                Payload.Add(buffer);

                if (response.ResultType == ResultType.Failure && PayloadEncoding == Encoding.V11)
                {
                    // When the response carries a failure encoded with 1.1, we need to perform a small adjustment
                    // between ice1 and ice2 response frames.
                    // ice1: [failure reply status][encapsulation or special exception]
                    // ice2: [failure][encapsulation with reply status + encapsulation bytes or special exception]
                    // There is no such adjustment with other encoding, or when the response does not carry a failure.

                    if (Protocol == Protocol.Ice1)
                    {
                        Debug.Assert(response.Protocol == Protocol.Ice2);

                        // Read the reply status byte located immediately after the encapsulation header; +1
                        // corresponds to the result type and +2 corresponds to the encoding in the encapsulation
                        // header.
                        ReplyStatus replyStatus = response.Payload[1 + sizeLength + 2].AsReplyStatus();

                        var ostr = new OutputStream(Ice1Definitions.Encoding, Payload);
                        ostr.Write(replyStatus);
                        if (replyStatus == ReplyStatus.UserException)
                        {
                            // We are forwarding a user exception encoded using 1.1 and received over ice2 to ice1.
                            // The size of the new encapsulation is the size of the payload -1 byte of the ice2 result
                            // type, -1 byte of the reply status, sizeLength is not included in the encapsulation size.

                            ostr.WriteEncapsulationHeader(response.Payload.Count - 1 - sizeLength - 1, PayloadEncoding);
                        }
                        // else we are forwarding a system exception encoded using 1.1 and received over ice2 to ice1.
                        // and we include only the reply status

                        ostr.Finish();

                        // We need to get rid of the extra reply status byte and the start of the encapsulation added
                        // when 1.1 exceptions are marshaled using ice2.

                        // 1 for the result type in the response, then sizeLength + 2 to skip the encapsulation header,
                        // then + 1 to skip the reply status byte
                        Payload.Add(response.Payload.Slice(1 + sizeLength + 2 + 1));
                    }
                    else
                    {
                        Debug.Assert(response.Protocol == Protocol.Ice1);
                        var ostr = new OutputStream(Ice2Definitions.Encoding, Payload);
                        ostr.Write(ResultType.Failure);

                        var replyStatus = (ReplyStatus)response.Payload[0];
                        if (replyStatus == ReplyStatus.UserException)
                        {
                            // We are forwarding a user exception encoded using 1.1 and received over ice1 to ice2. We
                            // create an ice2 encapsulation and write the 1.1 reply status followed by the 1.1 encoded
                            // data, sizeLength is not included in the encapsulation size.

                            ostr.WriteEncapsulationHeader(response.Payload.Count - sizeLength, PayloadEncoding);
                            ostr.Write(replyStatus);
                            ostr.Finish();
                            Payload.Add(response.Payload.Slice(1 + sizeLength + 2));
                        }
                        else
                        {
                            // We are forwarding a system exception encoded using 1.1 and received over ice1 to ice2.
                            // We create an ice2 encapsulation and write the 1.1 encoded exception in it (which
                            // includes the 1.1 reply status, followed by the exception). The size of the new
                            // encapsulation is the size of the payload +2 bytes for the encapsulations encoding.

                            ostr.WriteEncapsulationHeader(response.Payload.Count + 2, PayloadEncoding);
                            ostr.Finish();
                            Payload.Add(response.Payload);
                        }
                    }
                }
                else
                {
                    // In this case we always have an encapsulation it is either an ice1 response with result type
                    // success or an ice2 response that always uses an encapsulation. We just need to rewrite the
                    // encapsulation header with the new encoding and keep the rest of the encapsulation data as is.
                    var ostr = new OutputStream(Protocol.GetEncoding(), Payload);
                    ostr.Write(response.ResultType);
                    ostr.WriteEncapsulationHeader(response.Payload.Count - 1 - sizeLength, PayloadEncoding);
                    ostr.Finish();
                    Payload.Add(response.Payload.Slice(1 + sizeLength + 2));
                }

                Debug.Assert(Payload.Count == 2);
            }
        }

        /// <summary>Constructs a response frame that represents a failure and contains an exception.</summary>
        /// <param name="request">The incoming request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(IncomingRequestFrame request, RemoteException exception)
            : this(request.Protocol, request.PayloadEncoding)
        {
            ReplyStatus replyStatus = ReplyStatus.UserException;
            if (PayloadEncoding == Encoding.V11)
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
                                        _encapsulationStart,
                                        PayloadEncoding,
                                        FormatType.Sliced);

                if (Protocol == Protocol.Ice2 && PayloadEncoding == Encoding.V11)
                {
                    // The first byte of the encapsulation data is the actual ReplyStatus
                    ostr.Write(replyStatus);
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1 && (byte)replyStatus > (byte)ReplyStatus.UserException);
                ostr = new OutputStream(Ice1Definitions.Encoding, Payload); // not an encapsulation
                ostr.Write(replyStatus);
            }

            exception.Origin = new RemoteExceptionOrigin(request.Identity, request.Facet, request.Operation);
            if (PayloadEncoding == Encoding.V11)
            {
                switch (replyStatus)
                {
                    case ReplyStatus.ObjectNotExistException:
                    case ReplyStatus.OperationNotExistException:
                        request.Identity.IceWrite(ostr);
                        ostr.WriteIce1Facet(request.Facet);
                        ostr.WriteString(request.Operation);
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

            ostr.Finish();

            if (Protocol == Protocol.Ice2 && exception.RetryPolicy.Retryable != Retryable.No)
            {
                RetryPolicy retryPolicy = exception.RetryPolicy;

                BinaryContextOverride.Add(
                    (int)BinaryContextKey.RetryPolicy,
                    ostr =>
                    {
                        ostr.Write(retryPolicy.Retryable);
                        if (retryPolicy.Retryable == Retryable.AfterDelay)
                        {
                            ostr.WriteVarUInt((uint)retryPolicy.Delay.TotalMilliseconds);
                        }
                    });
            }
        }

        /// <inheritdoc/>
        internal override IncomingFrame ToIncoming() => new IncomingResponseFrame(this);

        /// <inheritdoc/>
        internal override void WriteHeader(OutputStream ostr)
        {
            Debug.Assert(ostr.Encoding == Protocol.GetEncoding());

            if (Protocol == Protocol.Ice2)
            {
                OutputStream.Position startPos = ostr.StartFixedLengthSize(2);
                WriteBinaryContext(ostr);
                ostr.EndFixedLengthSize(startPos, 2);
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1);
                // no header
            }
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
            response.Payload.Add(buffer);
            var ostr = new OutputStream(response.Protocol.GetEncoding(),
                                        response.Payload,
                                        _encapsulationStart,
                                        response.PayloadEncoding,
                                        format);
            return (response, ostr);
        }

        private OutgoingResponseFrame(
            Protocol protocol,
            Encoding encoding,
            bool compress = false,
            CompressionLevel compressionLevel = CompressionLevel.Fastest,
            int compressionMinSize = 100)
            : base(protocol,
                   compress,
                   compressionLevel,
                   compressionMinSize) =>
            PayloadEncoding = encoding;
    }
}
