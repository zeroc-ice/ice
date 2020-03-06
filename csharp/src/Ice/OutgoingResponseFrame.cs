//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using Context = System.Collections.Generic.Dictionary<string, string>;

namespace Ice
{
    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame : OutputStream
    {
        /// <summary>The ID for this request. With Ice1, a 0 value corresponds to a oneway request.</summary>
        public int RequestId { get; }

        /// <summary>The Ice1 reply status. Only meaningful for the Ice1 protocol, always set to OK with Ice2.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The response context. Always null with Ice1.</summary>
        public Context? Context { get; }

        private readonly Encoding _payloadEncoding; // TODO: move to OutputStream

        /// <summary>Creates a new outgoing request frame with an OK reply status and a void return value.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        public static OutgoingResponseFrame Empty(Current current)
            => new OutgoingResponseFrame(current, ReplyStatus.OK, ArraySegment<byte>.Empty);

        /// <summary>Creates a new outgoing response frame. It's a partial frame, and its payload needs to be
        /// written later on with StartReturnValue/EndReturnValue.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        public OutgoingResponseFrame(Current current) : base(current.Adapter.Communicator, Ice1Definitions.Encoding)
        {
            RequestId = current.RequestId;
            _payloadEncoding = current.Encoding;

            WriteSpan(Ice1Definitions.ReplyHeader.AsSpan());
            WriteInt(RequestId);
        }

        /// <summary>Creates a new outgoing response frame with the given reply status and payload.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this method creates a response.</param>
        /// <param name="replyStatus">The reply status.</param>
        /// <param name="payload">The payload for this response frame.</param>
        // TODO: add parameter such as "bool assumeOwnership" once we add memory pooling.
        public OutgoingResponseFrame(Current current, ReplyStatus replyStatus, ArraySegment<byte> payload)
            : this(current)
        {
            WriteByte((byte)replyStatus);

            if (payload.Count == 0 && (replyStatus == ReplyStatus.OK || replyStatus == ReplyStatus.UserException))
            {
                WriteEmptyEncapsulation(current.Encoding);
            }
            else
            {
                WritePayload(payload);
            }
        }

        /// <summary>Creates a response frame that represents "failure" and contains an exception.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(Current current, System.Exception exception)
            : this(current)
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

        /// <summary>Starts writing the return value for a successful response.</summary>
        /// <param name="format">The format for the return value, null (meaning keep communicator's setting),
        /// SlicedFormat or CompactFormat.</param>
        public void StartReturnValue(FormatType? format = null)
        {
            WriteByte((byte)ReplyStatus.OK);
            StartEncapsulation(_payloadEncoding, format);
        }

        /// <summary>Marks the end of the return value.</summary>
        public void EndReturnValue() => EndEncapsulation();
    }
}
