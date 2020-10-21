// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Represents a response protocol frame received by the application.</summary>
    public sealed class IncomingResponseFrame : IncomingFrame
    {
        /// <summary>The encoding of the frame payload.</summary>
        public override Encoding Encoding { get; }

        /// <summary>The <see cref="ZeroC.Ice.ResultType"/> of this response frame.</summary>
        public ResultType ResultType => Payload[0] == 0 ? ResultType.Success : ResultType.Failure;

        private static readonly ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), IncomingResponseFrame>
            _cachedVoidReturnValueFrames =
                new ConcurrentDictionary<(Protocol Protocol, Encoding Encoding), IncomingResponseFrame>();

        /// <summary>Returns an <see cref="IncomingResponseFrame"/> that represents a oneway pseudo response.</summary>
        internal static IncomingResponseFrame WithVoidReturnValue(Protocol protocol, Encoding encoding) =>
            _cachedVoidReturnValueFrames.GetOrAdd((protocol, encoding), key =>
            {
                var data = new List<ArraySegment<byte>>();
                var ostr = new OutputStream(key.Protocol.GetEncoding(), data);
                ostr.Write(ResultType.Success);
                _ = ostr.WriteEmptyEncapsulation(key.Encoding);
                Debug.Assert(data.Count == 1);
                return new IncomingResponseFrame(key.Protocol, data[0], int.MaxValue);
            });

        /// <summary>Reads the return value. If this response frame carries a failure, reads and throws this exception.
        /// </summary>
        /// <paramtype name="T">The type of the return value.</paramtype>
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

            if (ResultType == ResultType.Success)
            {
                return Payload.AsReadOnlyMemory(1).ReadEncapsulation(Protocol.GetEncoding(), communicator, reader);
            }
            else
            {
                throw ReadException(communicator);
            }
        }

        /// <summary>Reads the return value and makes sure this return value is empty (void) or has only unknown tagged
        /// members. If this response frame carries a failure, reads and throws this exception.</summary>
        /// <param name="communicator">The communicator.</param>
        public void ReadVoidReturnValue(Communicator communicator)
        {
            if (HasCompressedPayload)
            {
                DecompressPayload();
            }

            if (ResultType == ResultType.Success)
            {
                Payload.AsReadOnlyMemory(1).ReadEmptyEncapsulation(Protocol.GetEncoding());
            }
            else
            {
                throw ReadException(communicator);
            }
        }

        /// <summary>Constructs an incoming response frame.</summary>
        /// <param name="protocol">The Ice protocol of this frame.</param>
        /// <param name="data">The frame data as an array segment.</param>
        /// <param name="sizeMax">The maximum payload size, checked during decompress.</param>
        public IncomingResponseFrame(Protocol protocol, ArraySegment<byte> data, int sizeMax)
            : base(data, protocol, sizeMax)
        {
            bool hasEncapsulation = false;

            if (Protocol == Protocol.Ice1)
            {
                ReplyStatus replyStatus = Data[0].AsReplyStatus();

                if ((byte)replyStatus <= (byte)ReplyStatus.UserException)
                {
                    hasEncapsulation = true;
                }
                else
                {
                    Encoding = Encoding.V11;
                }
            }
            else
            {
                _ = Data[0].AsResultType(); // just to check the value
                hasEncapsulation = true;
            }

            // Read encapsulation header, in particular the payload encoding.
            if (hasEncapsulation)
            {
                int size;
                int sizeLength;

                (size, sizeLength, Encoding) =
                    Data.Slice(1).AsReadOnlySpan().ReadEncapsulationHeader(Protocol.GetEncoding());

                Payload = Data.Slice(0, 1 + size + sizeLength);
                HasCompressedPayload = Encoding == Encoding.V20 && Payload[1 + sizeLength + 2] != 0;
            }
            else
            {
                Payload = Data;
            }
        }

        private protected override ArraySegment<byte> GetEncapsulation()
        {
            // Can only be called for a frame with an encapsulation:
            Debug.Assert(Protocol == Protocol.Ice2 || Payload[0] <= (byte)ReplyStatus.UserException);
            return Payload.Slice(1);
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

                if (Protocol == Protocol.Ice2 && Encoding == Encoding.V11)
                {
                    replyStatus = istr.ReadReplyStatus();
                }
            }
            else
            {
                Debug.Assert(Protocol == Protocol.Ice1 && Encoding == Encoding.V11);
                istr = new InputStream(Payload.Slice(1), Encoding.V11);
            }

            Exception exception;
            if (Encoding == Encoding.V11 && replyStatus != ReplyStatus.UserException)
            {
                exception = istr.ReadIce1SystemException(replyStatus);
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
