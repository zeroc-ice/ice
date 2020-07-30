//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a response protocol frame received by the application.</summary>
    public sealed class IncomingResponseFrame
    {
        /// <summary>The response context. Always null with Ice1.</summary>
        public Dictionary<string, string>? Context { get; }

        /// <summary>The payload of this response frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        public ArraySegment<byte> Payload { get; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The frame reply status <see cref="ReplyStatus"/>. Applies only to ice1 frames.</summary>
        public ReplyStatus ReplyStatus => Protocol == Protocol.Ice1 ? (ReplyStatus)Payload[0] : ReplyStatus.OK;

        /// <summary>The frame byte count.</summary>
        public int Size => Payload.Count;

        private static readonly ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), IncomingResponseFrame>
            _cachedVoidReturnValueFrames =
                new ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), IncomingResponseFrame>();

        // Oneway pseudo-response
        internal static IncomingResponseFrame WithVoidReturnValue(Protocol protocol, Encoding encoding) =>
            _cachedVoidReturnValueFrames.GetOrAdd((protocol, encoding), key =>
            {
                var data = new List<ArraySegment<byte>>();
                OutputStream ostr;
                if (key.Protocol == Protocol.Ice1)
                {
                    // Byte status OK followed by encapsulation (empty for 1.1, with Success response type for 2.0)
                    byte[] buffer = new byte[256];
                    buffer[0] = (byte)ReplyStatus.OK;
                    data.Add(buffer);
                    ostr = new OutputStream(Ice1Definitions.Encoding,
                                            data,
                                            new OutputStream.Position(0, 1),
                                            key.Encoding,
                                            FormatType.Compact);
                }
                else
                {
                    Debug.Assert(key.Protocol == Protocol.Ice2);
                    // Just an encapsulation
                    ostr = new OutputStream(Ice2Definitions.Encoding,
                                            data,
                                            default,
                                            key.Encoding,
                                            FormatType.Compact);
                }

                if (key.Encoding == Encoding.V2_0)
                {
                    ostr.WriteByte((byte)ResponseType.Success);
                }
                ostr.Save();
                Debug.Assert(data.Count == 1);
                return new IncomingResponseFrame(key.Protocol, data[0]);
            });

        /// <summary>Reads the return value carried by this response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        /// <param name="reader">An input stream reader used to read the frame return value, when the frame
        /// return value contain multiple values the reader must use a tuple to return the values.</param>
        /// <returns>The frame return value.</returns>
        public T ReadReturnValue<T>(Communicator communicator, InputStreamReader<T> reader)
        {
            // TODO: for now, we assume ReplyStatus is set properly with ice2/1.1, which actually requires a binary
            // context.

            (InputStream istr, ResponseType responseType) = PrepareReadReturnValue(communicator);

            if (responseType == ResponseType.Success)
            {
                T result = reader(istr);
                // If the reader throws an exception such as InvalidDataException, we don't check we reached the
                // end of the buffer.
                istr.CheckEndOfBuffer(skipTaggedParams: true);
                return result;
            }
            else
            {
                Exception exception = istr.Encoding == Encoding.V1_1 ?
                    istr.ReadIce1Exception(ReplyStatus) : istr.ReadException();
                istr.CheckEndOfBuffer(skipTaggedParams: true);
                throw exception;
            }
        }

        /// <summary>Reads an empty return value from the response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        public void ReadVoidReturnValue(Communicator communicator)
        {
            // TODO: for now, we assume ReplyStatus is set properly with ice2/1.1, which actually requires a binary
            // context.

            (InputStream istr, ResponseType responseType) = PrepareReadReturnValue(communicator);

            if (responseType == ResponseType.Success)
            {
                istr.CheckEndOfBuffer(skipTaggedParams: true);
            }
            else
            {
                Exception exception = istr.Encoding == Encoding.V1_1 ?
                    istr.ReadIce1Exception(ReplyStatus) : istr.ReadException();
                istr.CheckEndOfBuffer(skipTaggedParams: true);
                throw exception;
            }
        }

        /// <summary>Creates a new IncomingResponse Frame</summary>
        /// <param name="protocol">The Ice protocol of this frame.</param>
        /// <param name="payload">The frame data as an array segment.</param>
        public IncomingResponseFrame(Protocol protocol, ArraySegment<byte> payload)
        {
            Protocol = protocol;

            if (Protocol == Protocol.Ice1)
            {
                byte replyStatus = payload[0];
                if (replyStatus > 7)
                {
                    throw new InvalidDataException(
                        $"received ice1 response frame with unknown reply status `{replyStatus}'");
                }
            }
            Payload = payload;
        }

        private (InputStream Istr, ResponseType ResponseType) PrepareReadReturnValue(Communicator communicator)
        {
            InputStream istr;

            if (Protocol == Protocol.Ice1)
            {
                // Reply status followed by data that depends on this byte status.
                if (ReplyStatus == ReplyStatus.OK || ReplyStatus == ReplyStatus.UserException)
                {
                    istr = new InputStream(Payload.Slice(1),
                                           Ice1Definitions.Encoding,
                                           communicator,
                                           startEncapsulation: true);
                }
                else
                {
                    istr = new InputStream(Payload.Slice(1), Ice1Definitions.Encoding);
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice2);
                // Always an encapsulation.
                istr = new InputStream(Payload, Ice2Definitions.Encoding, communicator, startEncapsulation: true);
            }

            ResponseType responseType;

            if (istr.Encoding == Encoding.V2_0)
            {
                byte responseTypeByte = istr.ReadByte();
                if (responseTypeByte > 1)
                {
                    throw new InvalidDataException($"`{responseTypeByte}' is not a valid response type");
                }
                responseType = (ResponseType)responseTypeByte;
            }
            else
            {
                responseType = ReplyStatus == ReplyStatus.OK ? ResponseType.Success : ResponseType.Failure;
            }

            return (istr, responseType);
        }
    }
}
