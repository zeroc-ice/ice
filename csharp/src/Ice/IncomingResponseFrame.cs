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

        /// <summary>The reply status <see cref="ZeroC.Ice.ReplyStatus"/>.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The result type <see cref="ZeroC.Ice.ResultType"/>.</summary>
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
                ostr.WriteByte(0); // Success or OK
                ostr.WriteEmptyEncapsulation(key.Encoding);
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
            InputStream istr = PrepareReadReturnValue(communicator);
            Debug.Assert(ResultType == ResultType.Success && ReplyStatus == ReplyStatus.OK);

            T returnValue = reader(istr);
            // If the reader throws an exception such as InvalidDataException, we don't check we reached the
            // end of the buffer.
            istr.CheckEndOfBuffer(skipTaggedParams: true);
            return returnValue;
        }

        /// <summary>Reads an empty return value from the response frame. If the response frame carries a failure, reads
        /// and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        public void ReadVoidReturnValue(Communicator communicator)
        {
            InputStream istr = PrepareReadReturnValue(communicator);
            Debug.Assert(ResultType == ResultType.Success && ReplyStatus == ReplyStatus.OK);

            // If the reader throws an exception such as InvalidDataException, we don't check we reached the
            // end of the buffer.
            istr.CheckEndOfBuffer(skipTaggedParams: true);
        }

        /// <summary>Creates a new IncomingResponse Frame</summary>
        /// <param name="protocol">The Ice protocol of this frame.</param>
        /// <param name="payload">The frame data as an array segment.</param>
        public IncomingResponseFrame(Protocol protocol, ArraySegment<byte> payload)
        {
            protocol.CheckSupported();
            Protocol = protocol;
            Payload = payload;

            if (Protocol == Protocol.Ice1)
            {
                ReplyStatus = AsReplyStatus(Payload[0]);

                if ((byte)ReplyStatus > (byte)ReplyStatus.UserException)
                {
                    Encoding = Encoding.V1_1;
                }
            }
            else
            {
                byte b = Payload[0];
                if (b > 1)
                {
                    throw new ArgumentException($"invalid ice2 result type `{b}' in payload", nameof(payload));
                }
                ReplyStatus = (ReplyStatus)b; // OK or UserException, but can be fixed again, see below
            }

            // Check the encapsulation if there is one.
            if (Protocol == Protocol.Ice2 || ReplyStatus == ReplyStatus.OK || ReplyStatus == ReplyStatus.UserException)
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

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V1_1 && ReplyStatus == ReplyStatus.UserException)
                {
                    // the first byte of the encapsulation is the actual ReplyStatus
                    // "+ 2" corresponds to the 2 bytes for the encoding in the encapsulation header
                    ReplyStatus = AsReplyStatus(Payload[1 + sizeLength + 2]);
                    if (ReplyStatus == ReplyStatus.OK)
                    {
                        throw new InvalidDataException("received ice2 response frame with invalid reply status");
                    }
                }
            }

            static ReplyStatus AsReplyStatus(byte b) =>
                b <= 7 ? (ReplyStatus)b :
                    throw new InvalidDataException($"received response frame with unknown reply status `{b}'");
        }

        // Returns an InputStream positioned on the first "real" byte of the payload
        private InputStream PrepareReadReturnValue(Communicator communicator)
        {
            InputStream istr;

            if (Protocol == Protocol.Ice2 || ReplyStatus == ReplyStatus.OK || ReplyStatus == ReplyStatus.UserException)
            {
                istr = new InputStream(Payload.Slice(1),
                                       Protocol.GetEncoding(),
                                       communicator,
                                       startEncapsulation: true);

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V1_1 && ReplyStatus != ReplyStatus.OK)
                {
                    // Skip ReplyStatus byte read in the constructor.
                    istr.Skip(1);
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1 && Encoding == Encoding.V1_1);
                istr = new InputStream(Payload.Slice(1), Encoding);
            }

            // These two conditions are actually the same
            if (ResultType != ResultType.Success || ReplyStatus != ReplyStatus.OK)
            {
                if (Protocol == Protocol.Ice2 || ReplyStatus == ReplyStatus.UserException)
                {
                    Exception exception = istr.ReadException();
                    istr.CheckEndOfBuffer(skipTaggedParams: true);
                    throw exception;
                }
                else
                {
                    Debug.Assert(Protocol == Protocol.Ice1 && (byte)ReplyStatus > (byte)ReplyStatus.UserException);
                    Exception exception = istr.ReadSpecialException11(ReplyStatus);
                    istr.CheckEndOfBuffer(skipTaggedParams: false);
                    throw exception;
                }
            }

            return istr;
        }
    }
}
