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
    public sealed class IncomingResponseFrame : IncomingFrame
    {
        /// <summary>The response context. Always null with Ice1.</summary>
        public Dictionary<string, string>? Context { get; }

        /// <summary>The encoding of the frame payload.</summary>
        public override Encoding Encoding { get; }

        public override ArraySegment<byte> Payload { get; internal set; }

        /// <summary>The frame reply status <see cref="ReplyStatus"/>.</summary>
        public ReplyStatus ReplyStatus { get; }

        private static readonly ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), IncomingResponseFrame>
            _cachedVoidReturnValueFrames =
                new ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), IncomingResponseFrame>();

        public static IncomingResponseFrame WithVoidReturnValue(Protocol protocol, Encoding encoding) =>
            _cachedVoidReturnValueFrames.GetOrAdd((protocol, encoding), key =>
            {
                var data = new List<ArraySegment<byte>>();
                var ostr = new OutputStream(key.Protocol.GetEncoding(), data);
                ostr.WriteByte((byte)ReplyStatus.OK);
                _ = ostr.WriteEmptyEncapsulation(key.Encoding);
                Debug.Assert(data.Count == 1);
                return new IncomingResponseFrame(key.Protocol, data[0], int.MaxValue);
            });

        /// <summary>Reads the return value carried by this response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        /// <param name="reader">An input stream reader used to read the frame return value, when the frame
        /// return value contain multiple values the reader must use a tuple to return the values.</param>
        /// <returns>The frame return value.</returns>
        public T ReadReturnValue<T>(Communicator communicator, InputStreamReader<T> reader)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (ReplyStatus == ReplyStatus.OK)
            {
                return Payload.AsReadOnlyMemory().ReadEncapsulation(Protocol.GetEncoding(), communicator, reader);
            }
            else
            {
                throw ReadException(communicator);
            }
        }

        /// <summary>Reads an empty return value from the response frame. If the response frame carries
        /// a failure, reads and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        public void ReadVoidReturnValue(Communicator communicator)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (ReplyStatus == ReplyStatus.OK)
            {
                Payload.AsReadOnlyMemory().ReadEmptyEncapsulation(Protocol.GetEncoding(), communicator);
            }
            else
            {
                throw ReadException(communicator);
            }
        }

        /// <summary>Creates a new IncomingResponse Frame</summary>
        /// <param name="protocol">The Ice protocol of this frame.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="sizeMax">The maximum payload size, checked during decompress.</param>
        public IncomingResponseFrame(Protocol protocol, ArraySegment<byte> data, int sizeMax)
            : base(protocol, data, sizeMax)
        {
            byte replyStatus = data[0];
            if (replyStatus > 7)
            {
                throw new InvalidDataException(
                    $"received {Protocol.GetName()} response frame with unknown reply status `{replyStatus}'");
            }
            ReplyStatus = (ReplyStatus)replyStatus;
            Payload = data.Slice(1);
            if (ReplyStatus == ReplyStatus.UserException || ReplyStatus == ReplyStatus.OK)
            {
                int size;
                (size, Encoding) = Payload.AsReadOnlySpan().ReadEncapsulationHeader(Protocol.GetEncoding());
                if (Protocol == Protocol.Ice1 && size + 4 != Payload.Count) // 4 = size length with 1.1 encoding
                {
                    throw new InvalidDataException($"invalid response encapsulation size: `{size}'");
                }
            }
            else
            {
                Encoding = Protocol.GetEncoding();
            }

            HasCompressedPayload =
                Encoding == Encoding.V2_0 &&
                (Protocol == Protocol.Ice2 ||
                (Protocol == Protocol.Ice1 && (ReplyStatus == ReplyStatus.OK ||
                                               ReplyStatus == ReplyStatus.UserException))) &&
                Payload[Payload.AsReadOnlySpan().ReadSize(Protocol.GetEncoding()).SizeLength + 2] != 0;
        }

        private Exception ReadException(Communicator communicator)
        {
            switch (ReplyStatus)
            {
                case ReplyStatus.UserException:
                    return Payload.AsReadOnlyMemory().ReadEncapsulation(Protocol.GetEncoding(),
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
            ReadOnlySpan<byte> buffer = Payload.AsReadOnlySpan();
            (int size, int sizeLength) = buffer.ReadSize(Protocol.GetEncoding());

            if (size + sizeLength != buffer.Length)
            {
                throw new InvalidDataException(@$"buffer size {
                    buffer.Length} for message does not match size length plus message size of {size + sizeLength}");
            }

            return new UnhandledException(buffer.Slice(sizeLength).ReadString(), Identity.Empty, "", "");
        }

        internal DispatchException ReadDispatchException()
        {
            var istr = new InputStream(Payload, Protocol.GetEncoding());
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
