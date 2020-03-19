//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    public abstract class OutgoingFrame
    {
        /// <summary>The encoding of the frame payload</summary>
        public Encoding Encoding { get; }
        /// <summary>True for a sealed frame, false otherwise, a sealed frame does not change its contents.</summary>
        public bool IsSealed { get; protected set; }
        /// <summary>The frame byte count.</summary>
        public int Size { get; protected set; }

        /// <summary>Returns a list of array segments with the contents of the frame payload.</summary>
        public abstract IList<ArraySegment<byte>> Payload { get; }

        // Position of the start of the payload.
        protected OutputStream.Position PayloadStart;

        // Position of the end of the payload, for Ice1 this is always the frame end.
        protected OutputStream.Position? PayloadEnd;

        // Contents of the Frame
        internal List<ArraySegment<byte>> Data { get; private set; }

        protected OutgoingFrame(Encoding encoding)
        {
            Encoding = encoding;
            Encoding.CheckSupported();
            Data = new List<ArraySegment<byte>>();
        }

        /// <summary>Creates and returns an OutputStream that can be used to write the payload of this frame,
        /// once the caller finish writing the payload it must call SavePayload and passing the returned
        /// stream as argument.</summary>
        /// <param name="format">The format type for the payload.</param>
        /// <returns>An OutputStream instance that can be used to write the payload of this frame.</returns>
        public abstract OutputStream WritePayload(FormatType? format = null);

        // Call by OutputStream.Save to inform the frame that the payload is ready.
        internal void PayloadReady(OutputStream ostr)
        {
            if (PayloadEnd != null)
            {
                throw new InvalidOperationException("the frame already contains a payload");
            }

            OutputStream.Position payloadEnd = ostr.Finish();
            Size = Data.GetByteCount();
            PayloadEnd = payloadEnd;
            IsSealed = true;
        }
    }
}
