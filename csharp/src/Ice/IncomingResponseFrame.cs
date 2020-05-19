//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a response protocol frame received by the application.</summary>
    public sealed class IncomingResponseFrame
    {
        /// <summary>The encoding of the frame payload</summary>
        public Encoding Encoding { get; }

        /// <summary>The response context. Always null with Ice1.</summary>
        public Dictionary<string, string>? Context { get; }

        /// <summary>The payload of this response frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        public ArraySegment<byte> Payload { get; }

        /// <summary>The frame reply status <see cref="ReplyStatus"/>.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The frame byte count.</summary>
        public int Size => Payload.Count;

        private readonly Communicator _communicator;

        /// <summary>Reads the return value carried by this response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        /// <param name="reader">An input stream reader used to read the frame return value, when the frame
        /// return value contain multiple values the reader must use a tuple to return the values.</param>
        /// <returns>The frame return value.</returns>
        public T ReadReturnValue<T>(InputStreamReader<T> reader)
        {
            if (ReplyStatus == ReplyStatus.OK)
            {
                return InputStream.ReadEncapsulation(_communicator, Ice1Definitions.Encoding, Payload.Slice(1), reader);
            }
            else
            {
                throw ReadException();
            }
        }

        /// <summary>Reads an empty return value from the response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        public void ReadVoidReturnValue()
        {
            if (ReplyStatus == ReplyStatus.OK)
            {
                InputStream.ReadEmptyEncapsulation(_communicator, Ice1Definitions.Encoding, Payload.Slice(1));
            }
            else
            {
                throw ReadException();
            }
        }

        /// <summary>Creates a new IncomingResponse Frame</summary>
        /// <param name="communicator">The communicator to use when initializing the stream.</param>
        /// <param name="payload">The frame data as an array segment.</param>
        public IncomingResponseFrame(Communicator communicator, ArraySegment<byte> payload)
        {
            _communicator = communicator;
            byte replyStatus = payload[0];
            if (replyStatus > 7)
            {
                throw new InvalidDataException(
                    $"received ice1 response frame with unknown reply status `{replyStatus}'");
            }
            ReplyStatus = (ReplyStatus)replyStatus;
            Payload = payload;
            if (ReplyStatus == ReplyStatus.UserException || ReplyStatus == ReplyStatus.OK)
            {
                int size;
                (size, Encoding) = InputStream.ReadEncapsulationHeader(Ice1Definitions.Encoding, Payload.AsSpan(1));
                if (size + 4 + 1 != Payload.Count) // 4 = size length with 1.1 encoding
                {
                    throw new InvalidDataException($"invalid response encapsulation size: `{size}'");
                }
            }
            else
            {
                Encoding = Ice1Definitions.Encoding;
            }
        }

        // TODO avoid copy payload (ToArray) creates a copy, that should be possible when
        // the frame has a single segment.
        public IncomingResponseFrame(Communicator communicator, OutgoingResponseFrame frame)
            : this(communicator, frame.Payload.ToArray())
        {
        }

        private Exception ReadException()
        {
            switch (ReplyStatus)
            {
                case ReplyStatus.UserException:
                {
                    return InputStream.ReadEncapsulation(_communicator, Ice1Definitions.Encoding, Payload.Slice(1),
                        istr => istr.ReadException());
                }
                case ReplyStatus.ObjectNotExistException:
                case ReplyStatus.FacetNotExistException:
                case ReplyStatus.OperationNotExistException:
                {
                    return ReadDispatchException();
                }
                default:
                {
                    Debug.Assert(ReplyStatus == ReplyStatus.UnknownException ||
                                 ReplyStatus == ReplyStatus.UnknownLocalException ||
                                 ReplyStatus == ReplyStatus.UnknownUserException);
                    return ReadUnhandledException();
                }
            }
        }

        internal UnhandledException ReadUnhandledException() =>
            new UnhandledException(InputStream.Read11String(Payload.Slice(1)), Identity.Empty, "", "");

        internal DispatchException ReadDispatchException()
        {
            var istr = new InputStream(_communicator, Ice1Definitions.Encoding, Payload, 1);
            var identity = new Identity(istr);
            string facet = istr.ReadFacet();
            string operation = istr.ReadString();

            if (ReplyStatus == ReplyStatus.OperationNotExistException)
            {
                return new OperationNotExistException(identity, facet, operation);
            }
            else
            {
                return new ObjectNotExistException(identity, facet, operation);
            }
        }
    }
}
