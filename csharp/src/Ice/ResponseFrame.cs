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

        /// <summary>Creates a new outgoing response frame with the given reply status and payload.</summary>
        /// <param name="replyStatus">The reply status.</param>
        /// <param name="payload">The payload for this response frame.</param>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        // TODO: add parameter such as "bool assumeOwnership" once we add memory pooling.
        public static OutgoingResponseFrame Create(ReplyStatus replyStatus, ArraySegment<byte> payload, Current current)
        {
            var frame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId, replyStatus);

            if (payload.Count == 0)
            {
                frame.WriteEmptyEncapsulation(current.Encoding);
            }
            else
            {
                // TODO: this works only because we know this segment matches the array (for now). OutputStream should
                // provide an efficient API to build such a response frame.
                frame.WriteBlob(payload.Array);
            }
            frame.IsSealed = true;
            return frame;
        }

        /// <summary>Creates a new outgoing response frame with a OK reply status and an empty payload.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame Empty(Current current)
            => Create(ReplyStatus.OK, ArraySegment<byte>.Empty, current);

        /// <summary>Creates a response frame that represents "failure" and contains an exception. The returned frame is
        /// complete and no additional data can be included in its payload.</summary>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame CreateFailure(System.Exception exception, Ice.Current current)
        {
            OutgoingResponseFrame responseFrame;

            try
            {
                throw exception;
            }
            catch (Ice.RequestFailedException ex)
            {
                if (ex.Id.Name == null || ex.Id.Name.Length == 0)
                {
                    ex.Id = current.Id;
                }

                if (ex.Facet == null || ex.Facet.Length == 0)
                {
                    ex.Facet = current.Facet;
                }

                if (ex.Operation == null || ex.Operation.Length == 0)
                {
                    ex.Operation = current.Operation;
                }

                ReplyStatus replyStatus = default;

                if (ex is Ice.ObjectNotExistException)
                {
                    replyStatus = ReplyStatus.ObjectNotExistException;
                }
                else if (ex is Ice.FacetNotExistException)
                {
                    replyStatus = ReplyStatus.FacetNotExistException;
                }
                else if (ex is Ice.OperationNotExistException)
                {
                    replyStatus = ReplyStatus.OperationNotExistException;
                }
                else
                {
                    Debug.Assert(false);
                }

                responseFrame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId, replyStatus);
                ex.Id.IceWrite(responseFrame);

                // For compatibility with the old FacetPath.
                if (ex.Facet == null || ex.Facet.Length == 0)
                {
                    responseFrame.WriteStringSeq(Array.Empty<string>());
                }
                else
                {
                    string[] facetPath = { ex.Facet };
                    responseFrame.WriteStringSeq(facetPath);
                }
                responseFrame.WriteString(ex.Operation);
            }
            catch (Ice.UnknownLocalException ex)
            {
                responseFrame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId,
                    ReplyStatus.UnknownLocalException);
                responseFrame.WriteString(ex.Unknown);
            }
            catch (Ice.UnknownUserException ex)
            {
                responseFrame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId,
                    ReplyStatus.UnknownUserException);
                responseFrame.WriteString(ex.Unknown);
            }
            catch (Ice.UnknownException ex)
            {
               responseFrame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId,
                    ReplyStatus.UnknownException);
                responseFrame.WriteString(ex.Unknown);
            }
            catch (Ice.UserException ex)
            {
                responseFrame = StartFailure(current);
                responseFrame.WriteException(ex);
                responseFrame.EndPayload();
            }
            catch (Ice.Exception ex)
            {
                responseFrame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId,
                    ReplyStatus.UnknownLocalException);
                responseFrame.WriteString(ex.ice_id() + "\n" + ex.StackTrace);
            }
            catch (System.Exception ex)
            {
                responseFrame = new OutgoingResponseFrame(current.Adapter.Communicator, current.RequestId,
                    ReplyStatus.UnknownException);
                responseFrame.WriteString(ex.ToString());
            }
            return responseFrame;
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
