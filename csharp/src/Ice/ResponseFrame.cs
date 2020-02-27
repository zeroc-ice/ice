//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    using Context = Dictionary<string, string>;

    /// <summary>The reply status used in Ice1 response frames (known as reply messages).</summary>
    public enum ReplyStatus : byte
    {
        OK = 0,
        UserException = 1,
        ObjectNotExistException = 2,
        FacetNotExistException = 3,
        OperationNotExistException = 4,
        UnknownLocalException = 5,
        UnknownUserException = 6,
        UnknownException = 7
    }

    /// <summary>Represents a response protocol frame received by the application.</summary>
    // TODO: IncomingResponseFrame should derive from InputStream
    public sealed class IncomingResponseFrame
    {
        // TODO: missing RequestId ?

        public InputStream InputStream { get; }

        /// <summary>The Ice1 reply status. Only meaningful for the Ice1 protocol, always set to OK with Ice2.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The response context. Always null with Ice1.</summary>
        public Context? Context { get; }

        internal IncomingResponseFrame(InputStream inputStream, ReplyStatus replyStatus)
        {
            InputStream = inputStream;
            ReplyStatus = replyStatus;
        }
    }

    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame : OutputStream
    {
        /// <summary>An OutputStream is sealed when it can no longer be written into, in particular the payload
        /// is complete and the context (if available) is written.</summary>
        public bool IsSealed { get; private set; } = false; // TODO: move to OutputStream with protected set

        /// <summary>The ID for this request. With Ice1, a 0 value corresponds to a oneway request.</summary>
        public int RequestId { get; }

        /// <summary>The Ice1 reply status. Only meaningful for the Ice1 protocol, always set to OK with Ice2.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The response context. Always null with Ice1.</summary>
        public Context? Context { get; }

        /// <summary>Creates a new outgoing response frame and starts the payload (encapsulation) that will hold the
        /// return value in that frame. This frame encodes a "success" response.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <param name="format">The Slice format (Compact or Sliced) used by the encapsulation.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame Start(Current current, FormatType? format = null)
        {
            var frame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId,
                ReplyStatus.OK);
            frame.StartEncapsulation(current.Encoding, format);
            return frame;
        }

        /// <summary>Creates a new outgoing response frame and the payload (encapsulation) that will hold the exception
        /// in that frame. This frame encodes a "failure" response.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame StartFailure(Current current)
        {
            var frame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId,
                ReplyStatus.UserException);
            frame.StartEncapsulation(current.Encoding, FormatType.SlicedFormat);
            return frame;
        }

        /// <summary>Creates a new outgoing response frame with the reply status and payload of an incoming response
        /// frame.</summary>
        /// <param name="incoming">An incoming response frame.</param>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame FromIncoming(IncomingResponseFrame incoming, Current current)
        {
            var frame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId,
                incoming.ReplyStatus);
            frame.WriteBlob(incoming.InputStream.ReadEncapsulation(out EncodingVersion _));
            frame.IsSealed = true;
            return frame;
        }

        /// <summary>Marks the end of the payload of this outgoing response frame.</summary>
        public void EndPayload()
        {
            Debug.Assert(!IsSealed);
            EndEncapsulation();
            IsSealed = true;
        }

        private OutgoingResponseFrame(Communicator communicator, int requestId, ReplyStatus replyStatus)
            : base(communicator, Ice.Util.CurrentProtocolEncoding)
        {
            RequestId = requestId;
            ReplyStatus = replyStatus;

            WriteBlob(Protocol.replyHdr);
            WriteInt(requestId);
            WriteByte((byte)replyStatus);
        }
    }
}
