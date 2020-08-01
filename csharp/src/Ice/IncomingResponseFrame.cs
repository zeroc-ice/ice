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

        /// <summary>The encoding of the frame payload.</summary>
        public Encoding Encoding { get; }

        /// <summary>The payload of this response frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        public ArraySegment<byte> Payload { get; }

        /// <summary>The Ice protocol of this frame.</summary>
        public Protocol Protocol { get; }

        /// <summary>The reply status <see cref="ReplyStatus"/> when the frame's payload encoding is 1.1. Otherwise,
        /// always ReplyStatus.OK.</summary>
        public ReplyStatus ReplyStatus { get; } = ReplyStatus.OK;

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
                    // Byte status OK followed by encapsulation (empty for 1.1, with Success result type for 2.0)
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

                    if (key.Encoding == Encoding.V1_1)
                    {
                        ostr.WriteByte((byte)ReplyStatus.OK);
                    }
                }

                if (key.Encoding == Encoding.V2_0)
                {
                    ostr.WriteByte((byte)ResultType.Success);
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
            (InputStream istr, ResultType resultType) = PrepareReadReturnValue(communicator);

            if (resultType == ResultType.Success)
            {
                T returnValue = reader(istr);
                // If the reader throws an exception such as InvalidDataException, we don't check we reached the
                // end of the buffer.
                istr.CheckEndOfBuffer(skipTaggedParams: true);
                return returnValue;
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
            (InputStream istr, ResultType resultType) = PrepareReadReturnValue(communicator);

            if (resultType == ResultType.Success)
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
            Payload = payload;

            if (Protocol == Protocol.Ice1)
            {
                ReplyStatus = AsReplyStatus(Payload[0]);

                if (ReplyStatus == ReplyStatus.OK || ReplyStatus == ReplyStatus.UserException)
                {
                    int size;
                    int sizeLength;

                    (size, sizeLength, Encoding) =
                        Payload.AsReadOnlySpan(1).ReadEncapsulationHeader(Ice1Definitions.Encoding);

                    if (1 + sizeLength + size != Payload.Count)
                    {
                        throw new InvalidDataException(
                            $"{Payload.Count - 1 - sizeLength - size} extra bytes in ice1 response frame");
                    }
                }
                else
                {
                    Encoding = Ice1Definitions.Encoding;
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice2);
                int size;
                int sizeLength;

                (size, sizeLength, Encoding) =
                        Payload.AsReadOnlySpan().ReadEncapsulationHeader(Ice2Definitions.Encoding);

                if (sizeLength + size != Payload.Count)
                {
                    throw new InvalidDataException(
                        $"{Payload.Count - sizeLength - size} extra bytes in ice2 response frame");
                }

                if (Encoding == Encoding.V1_1)
                {
                    // The reply status is the first byte after the encaps header.
                    ReplyStatus = AsReplyStatus(Payload[sizeLength + 2]);
                }
            }

            static ReplyStatus AsReplyStatus(byte b) =>
                b <= 7 ? (ReplyStatus)b :
                    throw new InvalidDataException($"received response frame with unknown reply status `{b}'");
        }

        private (InputStream Istr, ResultType ResultType) PrepareReadReturnValue(Communicator communicator)
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

                    if (ReplyStatus == ReplyStatus.UserException && istr.Encoding != Encoding.V1_1)
                    {
                        throw new InvalidDataException(
                            $"ice1 user exception encoded using encoding `{istr.Encoding}'");
                    }
                    // ReplyStatus.OK can have any encoding.
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
                if (istr.Encoding == Encoding.V1_1)
                {
                    byte b = istr.ReadByte();
                    Debug.Assert(b == (byte)ReplyStatus); // we already read and checked this byte in the constructor
                }
            }

            ResultType resultType;

            if (istr.Encoding == Encoding.V2_0)
            {
                byte resultTypeByte = istr.ReadByte();
                if (resultTypeByte > 1)
                {
                    throw new InvalidDataException($"`{resultTypeByte}' is not a valid result type");
                }
                resultType = (ResultType)resultTypeByte;
            }
            else
            {
                resultType = ReplyStatus == ReplyStatus.OK ? ResultType.Success : ResultType.Failure;
            }

            return (istr, resultType);
        }
    }
}
