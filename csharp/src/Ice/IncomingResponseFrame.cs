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

        /// <summary>The result type; see <see cref="ZeroC.Ice.ResultType"/>.</summary>
        public ResultType ResultType => Payload[0] == 0 ? ResultType.Success : ResultType.Failure;

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
                var ostr = new OutputStream(key.Protocol.GetEncoding(), data);
                ostr.WriteByte((byte)ResultType.Success);
                _ = ostr.WriteEmptyEncapsulation(key.Encoding);
                Debug.Assert(data.Count == 1);
                return new IncomingResponseFrame(key.Protocol, data[0]);
            });

        /// <summary>Reads the return value carried by this response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        /// <param name="reader">An input stream reader used to read the frame return value, when the frame
        /// return value contain multiple values the reader must use a tuple to return the values.</param>
        /// <returns>The frame return value.</returns>
        public T ReadReturnValue<T>(Communicator communicator, InputStreamReader<T> reader) =>
            ResultType == ResultType.Success ?
                Payload.AsReadOnlyMemory(1).ReadEncapsulation(Protocol.GetEncoding(), communicator, reader) :
                throw ReadException(communicator);

        /// <summary>Reads an empty return value from the response frame. If the response frame carries a failure, reads
        /// and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        public void ReadVoidReturnValue(Communicator communicator)
        {
            if (ResultType == ResultType.Success)
            {
                Payload.AsReadOnlyMemory(1).ReadEmptyEncapsulation(Protocol.GetEncoding());
            }
            else
            {
                throw ReadException(communicator);
            }
        }

        /// <summary>Creates a new IncomingResponse Frame</summary>
        /// <param name="protocol">The Ice protocol of this frame.</param>
        /// <param name="payload">The frame data as an array segment.</param>
        public IncomingResponseFrame(Protocol protocol, ArraySegment<byte> payload)
        {
            protocol.CheckSupported();
            Protocol = protocol;
            Payload = payload;
            bool hasEncapsulation = false;

            if (Protocol == Protocol.Ice1)
            {
                byte b = Payload[0];
                if (b > 7)
                {
                    throw new InvalidDataException($"received response frame with unknown reply status `{b}'");
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
                byte b = Payload[0];
                if (b > 1)
                {
                    throw new InvalidDataException($"invalid result type `{b}' in ice2 response frame");
                }
                hasEncapsulation = true;
            }

            // Read encapsulation header, in particular the payload encoding.
            if (hasEncapsulation)
            {
                int size;
                int sizeLength;

                (size, sizeLength, Encoding) =
                    Payload.AsReadOnlySpan(1).ReadEncapsulationHeader(Protocol.GetEncoding());

                if (1 + sizeLength + size != Payload.Count)
                {
                    throw new InvalidDataException(
                        $"{Payload.Count - 1 - sizeLength - size} extra bytes in payload of response frame");
                }
            }
        }

        /// <summary>If this response holds a 1.1-encoded system exception, reads and throws this exception.</summary>
        internal void ThrowIfSystemException(Communicator communicator)
        {
            if (ResultType == ResultType.Failure && Encoding == Encoding.V1_1)
            {
                var replyStatus = (ReplyStatus)Payload[0]; // can be reassigned below

                InputStream? istr = null;
                if (Protocol == Protocol.Ice1)
                {
                    if (replyStatus != ReplyStatus.UserException)
                    {
                        istr = new InputStream(Payload.Slice(1), Encoding.V1_1);
                    }
                }
                else
                {
                    istr = new InputStream(Payload.Slice(1),
                                           Ice2Definitions.Encoding,
                                           communicator,
                                           startEncapsulation: true);

                    byte b = istr.ReadByte();
                    replyStatus = b >= 1 && b <= 7 ? (ReplyStatus)b :
                        throw new InvalidDataException($"received ice2 response frame with invalid reply status `{b}'");

                    if (replyStatus == ReplyStatus.UserException)
                    {
                        istr = null; // we are not throwing this user exception here
                    }
                }

                if (istr != null)
                {
                    throw istr.ReadSystemException11(replyStatus);
                }
            }
        }

        private Exception ReadException(Communicator communicator)
        {
            Debug.Assert(ResultType != ResultType.Success);

            var replyStatus = (ReplyStatus)Payload[0]; // can be reassigned below

            InputStream istr;

            if (Protocol == Protocol.Ice2 || replyStatus == ReplyStatus.UserException)
            {
                istr = new InputStream(Payload.Slice(1),
                                       Protocol.GetEncoding(),
                                       communicator,
                                       startEncapsulation: true);

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V1_1)
                {
                    byte b = istr.ReadByte();
                    replyStatus = b >= 1 && b <= 7 ? (ReplyStatus)b :
                        throw new InvalidDataException($"received ice2 response frame with invalid reply status `{b}'");
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1 && Encoding == Encoding.V1_1);
                istr = new InputStream(Payload.Slice(1), Encoding.V1_1);
            }

            Exception exception;
            if (Encoding == Encoding.V1_1 && replyStatus != ReplyStatus.UserException)
            {
                exception = istr.ReadSystemException11(replyStatus);
                istr.CheckEndOfBuffer(skipTaggedParams: false);
            }
            else
            {
                exception = istr.ReadException();
                istr.CheckEndOfBuffer(skipTaggedParams: true);
            }
            return exception;
        }
    }
}
