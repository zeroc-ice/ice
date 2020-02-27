//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    using Context = Dictionary<string, string>;

    /// <summary>Represents a response protocol frame received by the application.</summary>
    // TODO: IncomingResponseFrame should derive from InputStream
    public sealed class IncomingResponseFrame
    {
        // TODO: missing RequestId, Connection, Encoding?

        public InputStream InputStream { get; }

        /// <summary>The Ice1 reply status. Only meaningful for the Ice1 protocol, always set to OK with Ice2.</summary>
        // TODO: currently it's only OK or UserException as we "throw" other responses.
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
                    if (InputStream.IsEmpty)
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
                        _payload = new ArraySegment<byte>(InputStream.ReadEncapsulation(out EncodingVersion _));
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

        internal IncomingResponseFrame(ReplyStatus replyStatus, InputStream inputStream)
        {
            ReplyStatus = replyStatus;
            InputStream = inputStream;
        }
    }
}
