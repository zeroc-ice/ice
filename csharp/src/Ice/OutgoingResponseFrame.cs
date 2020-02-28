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

    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame : OutputStream
    {
        /// <summary>The ID for this request. With Ice1, a 0 value corresponds to a oneway request.</summary>
        public int RequestId { get; }

        /// <summary>The Ice1 reply status. Only meaningful for the Ice1 protocol, always set to OK with Ice2.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The response context. Always null with Ice1.</summary>
        public Context? Context { get; }

        /// <summary>Creates a new "success" outgoing response frame.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <param name="format">The Slice format (Compact or Sliced) used by the encapsulation.</param>
        /// <param name="payloadWriter">An action that writes the contents of the payload.</param>
        public OutgoingResponseFrame(Current current, FormatType? format, Action<OutputStream> payloadWriter)
            : this(current.Adapter.Communicator, current.RequestId)
        {
            WriteByte((byte)ReplyStatus.OK);
            StartEncapsulation(current.Encoding, format);
            payloadWriter(this);
            EndEncapsulation();
        }

        /// <summary>Creates a new "success" outgoing response frame with a null format</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <param name="payloadWriter">An action that writes the contents of the payload.</param>
        public OutgoingResponseFrame(Current current, Action<OutputStream> payloadWriter)
            : this(current, format: null, payloadWriter)
        {
        }

        /// <summary>Creates a new outgoing response frame with the given reply status and payload.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <param name="replyStatus">The reply status.</param>
        /// <param name="payload">The payload for this response frame.</param>
        // TODO: add parameter such as "bool assumeOwnership" once we add memory pooling.
        public OutgoingResponseFrame(Current current, ReplyStatus replyStatus, ArraySegment<byte> payload)
            : this(current.Adapter.Communicator, current.RequestId)

        {
            WriteByte((byte)replyStatus);

            if (payload.Count == 0 && (replyStatus == ReplyStatus.OK || replyStatus == ReplyStatus.UserException))
            {
                WriteEmptyEncapsulation(current.Encoding);
            }
            else
            {
                // TODO: this works only because we know this segment matches the array (for now). OutputStream should
                // provide an efficient API to build such a response frame.
                WriteBlob(payload.Array);
            }
        }

        /// <summary>Creates a new outgoing response frame with the given payload and reply status OK.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <param name="payload">The payload for this response frame. Null is equivalent to an empty payload</param>
        public OutgoingResponseFrame(Current current, ArraySegment<byte>? payload = null)
            : this(current, ReplyStatus.OK, payload ?? ArraySegment<byte>.Empty)
        {
        }

        /// <summary>Creates a response frame that represents "failure" and contains an exception.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(Current current, System.Exception exception)
            : this(current.Adapter.Communicator, current.RequestId)
        {
            try
            {
                throw exception;
            }
            catch (RequestFailedException ex)
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

                if (ex is ObjectNotExistException)
                {
                    replyStatus = ReplyStatus.ObjectNotExistException;
                }
                else if (ex is FacetNotExistException)
                {
                    replyStatus = ReplyStatus.FacetNotExistException;
                }
                else if (ex is OperationNotExistException)
                {
                    replyStatus = ReplyStatus.OperationNotExistException;
                }
                else
                {
                    Debug.Assert(false);
                }

                WriteByte((byte)replyStatus);
                ex.Id.IceWrite(this);

                // For compatibility with the old FacetPath.
                if (ex.Facet == null || ex.Facet.Length == 0)
                {
                    WriteStringSeq(Array.Empty<string>());
                }
                else
                {
                    WriteStringSeq(new string[]{ ex.Facet });
                }
                WriteString(ex.Operation);
            }
            catch (UnknownLocalException ex)
            {
                WriteByte((byte)ReplyStatus.UnknownLocalException);
                WriteString(ex.Unknown);
            }
            catch (UnknownUserException ex)
            {
                WriteByte((byte)ReplyStatus.UnknownUserException);
                WriteString(ex.Unknown);
            }
            catch (UnknownException ex)
            {
                WriteByte((byte)ReplyStatus.UnknownException);
                WriteString(ex.Unknown);
            }
            catch (UserException ex)
            {
                WriteByte((byte)ReplyStatus.UserException);
                StartEncapsulation(current.Encoding, FormatType.SlicedFormat);
                WriteException(ex);
                EndEncapsulation();
            }
            catch (Ice.Exception ex)
            {
                WriteByte((byte)ReplyStatus.UnknownLocalException);
                WriteString(ex.ice_id() + "\n" + ex.StackTrace);
            }
            catch (System.Exception ex)
            {
                WriteByte((byte)ReplyStatus.UnknownException);
                WriteString(ex.ToString());
            }
        }

        private OutgoingResponseFrame(Communicator communicator, int requestId)
            : base(communicator, Util.CurrentProtocolEncoding)
        {
            RequestId = requestId;

            WriteBlob(Protocol.replyHdr);
            WriteInt(requestId);
        }
    }
}
