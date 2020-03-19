//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame : OutgoingFrame
    {
        /// <summary>The Ice1 reply status. Only meaningful for the Ice1 protocol, always set to OK with Ice2.</summary>
        public ReplyStatus ReplyStatus { get; }

        /// <summary>The response context. Always null with ice1.</summary>
        public Dictionary<string, string>? Context { get; }

        /// <summary>Creates a new outgoing request frame with an OK reply status and a void return value.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        public static OutgoingResponseFrame Empty(Encoding encoding)
            => new OutgoingResponseFrame(encoding, ArraySegment<byte>.Empty);

        /// <summary>Creates a new outgoing response frame. It's a partial frame, and its payload needs to be
        /// written later on with WritePayload.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        public OutgoingResponseFrame(Encoding encoding) : base(encoding)
        {
        }

        /// <summary>Creates a new outgoing response frame with the given reply status and payload.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        /// <param name="payload">The payload for this response frame.</param>
        // TODO: add parameter such as "bool assumeOwnership" once we add memory pooling.
        public OutgoingResponseFrame(Encoding encoding, ArraySegment<byte> payload)
            : this(encoding)
        {
            if (payload.Count == 0)
            {
                byte[] buffer = new byte[256];
                int pos = 0;
                buffer[pos++] = (byte)ReplyStatus.OK;
                OutputStream.WriteInt(6, buffer.AsSpan(pos, 4));
                pos += 4;
                buffer[pos++] = encoding.Major;
                buffer[pos++] = encoding.Minor;
                Data.Add(new ArraySegment<byte>(buffer, 0, pos));
            }
            else
            {
                Data.Add(payload);
                PayloadEnd = new OutputStream.Position(0, payload.Count);
            }
            Size = Data.GetByteCount();
            IsSealed = true;
        }

        /// <summary>Creates a response frame that represents "failure" and contains an exception.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(Current current, RemoteException exception)
            : this(current.Encoding)
        {
            var ostr = new OutputStream(this, PayloadStart);
            if (exception is RequestFailedException requestFailedException)
            {
                if (requestFailedException is DispatchException dispatchException)
                {
                    // TODO: the null checks are necessary due to the way we unmarshal the exception through reflection.

                    if (string.IsNullOrEmpty(dispatchException.Id.Name))
                    {
                        dispatchException.Id = current.Id;
                    }

                    if (string.IsNullOrEmpty(dispatchException.Facet))
                    {
                        dispatchException.Facet = current.Facet;
                    }

                    if (string.IsNullOrEmpty(dispatchException.Operation))
                    {
                        dispatchException.Operation = current.Operation;
                    }

                    ReplyStatus replyStatus = default;

                    if (dispatchException is ObjectNotExistException)
                    {
                        replyStatus = ReplyStatus.ObjectNotExistException;
                    }
                    else if (dispatchException is OperationNotExistException)
                    {
                        replyStatus = ReplyStatus.OperationNotExistException;
                    }
                    else
                    {
                        Debug.Assert(false);
                    }

                    ostr.WriteByte((byte)replyStatus);
                    dispatchException.Id.IceWrite(ostr);

                    // For compatibility with the old FacetPath.
                    if (string.IsNullOrEmpty(dispatchException.Facet))
                    {
                        ostr.WriteStringSeq(Array.Empty<string>());
                    }
                    else
                    {
                        ostr.WriteStringSeq(new string[] { dispatchException.Facet });
                    }
                    ostr.WriteString(dispatchException.Operation);
                }
                else
                {
                    ostr.WriteByte((byte)ReplyStatus.UnknownLocalException);
                    if (requestFailedException.IceMessage.Length > 0)
                    {
                        ostr.WriteString(requestFailedException.IceMessage);
                    }
                    else
                    {
                        ostr.WriteString(requestFailedException.ToString());
                    }
                }
            }
            else
            {
                ostr.WriteByte((byte)ReplyStatus.UserException);
                ostr.StartEncapsulation(FormatType.SlicedFormat);
                ostr.WriteException(exception);
                ostr.EndEncapsulation();
            }
            PayloadEnd = ostr.Finish();
            Size = Data.GetByteCount();
            IsSealed = true;
        }

        /// <summary>Starts writing the return value for a successful response.</summary>
        /// <param name="format">The format for the return value, null (meaning keep communicator's setting),
        /// SlicedFormat or CompactFormat.</param>
        public override OutputStream WritePayload(FormatType? format = null)
        {
            if (PayloadEnd != null)
            {
                throw new InvalidOperationException("the frame already contains a payload");
            }
            var ostr = new OutputStream(this, PayloadStart);
            ostr.WriteByte((byte)ReplyStatus.OK);
            ostr.StartEncapsulation(format);
            return ostr;
        }
    }
}
