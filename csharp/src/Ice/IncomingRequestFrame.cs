//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    /// <summary>Represents a request protocol frame received by the application.</summary>
    // TODO: IncomingRequestFrame should derive from InputStream
    public sealed class IncomingRequestFrame
    {
        public InputStream InputStream { get; }

        // TODO: combine Current and RequestFrameHeader in some way?
        public Current Current { get; }

        /// <summary>The payload of this request frame. The bytes inside the payload should not be written to;
        /// they are writable because of the <see cref="System.Net.Sockets.Socket"/> methods for sending.</summary>
        // TODO: describe how long this payload remains valid once we add memory pooling.
        public ArraySegment<byte> Payload { get; }

        /// <summary>Take the payload from this response frame. After calling this method, the payload can no longer
        /// be read.</summary>
        public ArraySegment<byte> TakePayload()
        {
            // TODO: make this method destructive with memory pooling.
            return Payload;
        }

        public IncomingRequestFrame(InputStream inputStream, Current current)
        {
            InputStream = inputStream;
            Current = current;

            // TODO: works only when Payload called first before reading anything. Need a better version!
            // TODO: not efficient to create an array here
            InputStream.Pos -= 6;
            Payload = new ArraySegment<byte>(InputStream.ReadEncapsulation(out Encoding _));
        }
    }
}
