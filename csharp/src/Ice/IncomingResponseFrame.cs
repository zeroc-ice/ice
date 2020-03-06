//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using Context = System.Collections.Generic.Dictionary<string, string>;

namespace Ice
{
    public enum ResultType : byte { Success, Failure };

    /// <summary>Represents a response protocol frame received by the application.</summary>
    public sealed class IncomingResponseFrame
    {
        public InputStream InputStream { get; }

        public ReplyStatus ReplyStatus { get; }

        /// <summary>The response context. Always null with Ice1.</summary>
        public Context? Context { get; }

        /// <summary>The payload of this response frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        public ArraySegment<byte> Payload
        {
            get
            {
                if (_payload == null)
                {
                    // TODO, it should never be empty, but currently it is when fulfilled by Sent
                    if (InputStream.Size == 0)
                    {
                        _payload = ArraySegment<byte>.Empty;
                    }
                    else
                    {
                        // TODO: for other reply status,return the non-encaps payload.
                        Debug.Assert(ReplyStatus == ReplyStatus.OK || ReplyStatus == ReplyStatus.UserException);

                        // TODO: works only when Payload called first before reading anything. Need a better version!
                        // TODO: not efficient to create an array here
                        // TODO: provide Encoding property
                        _payload = new ArraySegment<byte>(InputStream.ReadEncapsulation(out Encoding _));
                    }
                }
                return _payload.Value;
            }
        }

        private ArraySegment<byte>? _payload;

        /// <summary>Take the payload from this response frame. After calling this method, the payload can no longer
        /// be read.</summary>
        public ArraySegment<byte> TakePayload()
        {
            // TODO: make this method destructive with memory pooling.
            return Payload;
        }

        /// <summary>Starts reading the result carried by this response frame.</summary>
        /// <returns>The type of the result, either Success or Failure, and an InputStream iterator over this result.
        /// </returns>
        public (ResultType ResultType, InputStream InputStream) ReadResult()
        {
            InputStream.StartEncapsulation();
            switch (ReplyStatus)
            {
                case ReplyStatus.OK:
                    return (ResultType.Success, InputStream);
                case ReplyStatus.UserException:
                    return (ResultType.Failure, InputStream);
                default:
                    throw new InvalidOperationException(); // TODO: better exception and message
            }
        }

        /// <summary>Starts reading the return value carried by this response frame. If the response frame carries
        /// a failure, ReadReturnValue reads and throws this exception.</summary>
        /// <returns>An InputStream iterator over this return value.</returns>
        public InputStream ReadReturnValue()
        {
            if (ReadResult().ResultType == ResultType.Failure)
            {
                // TODO: would be nicer to read then EndEncaps then throw the exception
                InputStream.ThrowException();
            }
            return InputStream;
        }

        /// <summary>Reads an empty return value from the response frame.</summary>
        public void ReadVoidReturnValue()
        {
            InputStream istr = ReadReturnValue();
            istr.EndEncapsulation();
        }

        internal IncomingResponseFrame(ReplyStatus replyStatus, InputStream inputStream)
        {
            InputStream = inputStream;
            ReplyStatus = replyStatus;
        }
    }
}
