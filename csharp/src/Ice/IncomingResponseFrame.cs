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

        // TODO: this is really a oneway pseudo-response. Should we rename the method? Simplify further the frame?
        internal static IncomingResponseFrame WithVoidReturnValue(Protocol protocol, Encoding encoding) =>
            _cachedVoidReturnValueFrames.GetOrAdd((protocol, encoding), key =>
            {
                var data = new List<ArraySegment<byte>>();
                if (key.Protocol == Protocol.Ice1)
                {
                    var ostr = new OutputStream(Ice1Definitions.Encoding, data);
                    ostr.WriteByte((byte)ReplyStatus.OK);
                    _ = ostr.WriteEmptyEncapsulation(key.Encoding);
                }
                else
                {
                    Debug.Assert(key.Protocol == Protocol.Ice2);
                    var ostr =
                        new OutputStream(Ice2Definitions.Encoding, data, default, key.Encoding, FormatType.Compact);
                    ostr.WriteByte((byte)ResponseType.Success);
                    ostr.Save();
                }
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
            if (Protocol == Protocol.Ice1)
            {
                return ReplyStatus == ReplyStatus.OK ?
                    Payload.AsReadOnlyMemory(1).ReadEncapsulation(Ice1Definitions.Encoding, communicator, reader) :
                    throw ReadException(communicator);
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice2);
                var istr = new InputStream(Payload, Ice2Definitions.Encoding, communicator, startEncapsulation: true);
                byte responseTypeByte = istr.ReadByte();
                if (responseTypeByte > 1)
                {
                    throw new InvalidDataException($"`{responseTypeByte}' is not a valid response type");
                }

                if ((ResponseType)responseTypeByte == ResponseType.Success)
                {
                    return reader(istr);
                }
                else
                {
                    throw istr.ReadException();
                }
            }
        }

        /// <summary>Reads an empty return value from the response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        public void ReadVoidReturnValue(Communicator communicator)
        {
            if (Protocol == Protocol.Ice1)
            {
                if (ReplyStatus == ReplyStatus.OK)
                {
                    Payload.AsReadOnlyMemory(1).ReadEmptyEncapsulation(Ice1Definitions.Encoding, communicator);
                }
                else
                {
                    throw ReadException(communicator);
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice2);
                var istr = new InputStream(Payload, Ice2Definitions.Encoding, communicator, startEncapsulation: true);
                byte responseTypeByte = istr.ReadByte();
                if (responseTypeByte > 1)
                {
                    throw new InvalidDataException($"`{responseTypeByte}' is not a valid response type");
                }

                if ((ResponseType)responseTypeByte == ResponseType.Success)
                {
                    istr.CheckEndOfBuffer(skipTaggedParams: true);
                }
                else
                {
                    throw istr.ReadException();
                }
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

        // TODO avoid copy payload (ToArray) creates a copy, that should be possible when
        // the frame has a single segment.
        public IncomingResponseFrame(OutgoingResponseFrame frame)
            : this(frame.Protocol, frame.Payload.ToArray())
        {
        }

        private Exception ReadException(Communicator communicator)
        {
            Debug.Assert(Protocol == Protocol.Ice1);
            switch (ReplyStatus)
            {
                case ReplyStatus.UserException:
                    return Payload.AsReadOnlyMemory(1).ReadEncapsulation(Protocol.GetEncoding(),
                                                                         communicator,
                                                                         istr => istr.ReadException());

                case ReplyStatus.ObjectNotExistException:
                case ReplyStatus.FacetNotExistException:
                case ReplyStatus.OperationNotExistException:
                    return ReadDispatchException();

                default:
                    Debug.Assert(ReplyStatus == ReplyStatus.UnknownException ||
                                 ReplyStatus == ReplyStatus.UnknownLocalException ||
                                 ReplyStatus == ReplyStatus.UnknownUserException);
                    return ReadUnhandledException();
            }
        }

        internal UnhandledException ReadUnhandledException()
        {
            Debug.Assert(Protocol == Protocol.Ice1);
            var buffer = Payload.AsReadOnlySpan(1);
            (int size, int sizeLength) = buffer.ReadSize(Ice1Definitions.Encoding);

            if (size + sizeLength != buffer.Length)
            {
                throw new InvalidDataException(@$"buffer size {
                    buffer.Length} for message does not match size length plus message size of {size + sizeLength}");
            }

            return new UnhandledException(buffer.Slice(sizeLength).ReadString(), Identity.Empty, "", "");
        }

        internal DispatchException ReadDispatchException()
        {
            Debug.Assert(Protocol == Protocol.Ice1);
            var istr = new InputStream(Payload.Slice(1), Ice1Definitions.Encoding);
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
