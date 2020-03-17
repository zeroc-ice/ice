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
            => new OutgoingResponseFrame(encoding, ReplyStatus.OK, ArraySegment<byte>.Empty);

        /// <summary>Creates a new outgoing response frame. It's a partial frame, and its payload needs to be
        /// written later on with WritePayload/SavePayload.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        public OutgoingResponseFrame(Encoding encoding) : base(encoding)
        {
        }

        /// <summary>Creates a new outgoing response frame with the given reply status and payload.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        /// <param name="replyStatus">The reply status.</param>
        /// <param name="payload">The payload for this response frame.</param>
        // TODO: add parameter such as "bool assumeOwnership" once we add memory pooling.
        public OutgoingResponseFrame(Encoding encoding, ReplyStatus replyStatus, ArraySegment<byte> payload)
            : this(encoding)
        {
            ReplyStatus = replyStatus;
            var ostr = new OutputStream(Encoding, Data, PayloadStart);
            ostr.WriteByte((byte)replyStatus);
            if (payload.Count == 0 && (replyStatus == ReplyStatus.OK || replyStatus == ReplyStatus.UserException))
            {
                ostr.WriteEmptyEncapsulation(Encoding);
            }
            else
            {
                ostr.WritePayload(payload);
            }
            OutputStream.Position payloadEnd = ostr.Tail;
            ostr.Finish();
            Size = Ice1Definitions.HeaderSize + 4 + Data.GetByteCount();
            PayloadSize = Data.GetByteCount();
            PayloadEnd = payloadEnd;
            IsSealed = true;
        }

        /// <summary>Creates a response frame that represents "failure" and contains an exception.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(Current current, RemoteException exception)
            : this(current.Encoding)
        {
            var ostr = new OutputStream(Encoding, Data, PayloadStart);
            if (exception is RequestFailedException requestFailedException)
            {
                if (requestFailedException is DispatchException dispatchException)
                {
                    // TODO: the null checks are necessary due to the way we unmarshal the exception through reflection.

                    if (dispatchException.Id.Name == null || dispatchException.Id.Name.Length == 0)
                    {
                        dispatchException.Id = current.Id;
                    }

                    if (dispatchException.Facet == null || dispatchException.Facet.Length == 0)
                    {
                        dispatchException.Facet = current.Facet;
                    }

                    if (dispatchException.Operation == null || dispatchException.Operation.Length == 0)
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
                    if (dispatchException.Facet == null || dispatchException.Facet.Length == 0)
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
                ostr.StartEncapsulation(Encoding, FormatType.SlicedFormat);
                ostr.WriteException(exception);
                ostr.EndEncapsulation();
            }
            OutputStream.Position payloadEnd = ostr.Tail;
            ostr.Finish();
            Size = Ice1Definitions.HeaderSize + 4 + Data.GetByteCount();
            PayloadSize = OutputStream.Distance(Data, PayloadStart, payloadEnd);
            PayloadEnd = payloadEnd;
            IsSealed = true;
        }

        /// <summary>Starts writing the return value for a successful response.</summary>
        /// <param name="format">The format for the return value, null (meaning keep communicator's setting),
        /// SlicedFormat or CompactFormat.</param>
        public override OutputStream WritePayload(FormatType? format = null)
        {
            if (Ostr != null)
            {
                throw new InvalidOperationException("the frame already start writting the payload");
            }

            if (PayloadEnd != null)
            {
                throw new InvalidOperationException("the frame already contains a paylod");
            }
            Ostr = new OutputStream(Encoding, Data, PayloadStart);
            Ostr.WriteByte((byte)ReplyStatus.OK);
            Ostr.StartEncapsulation(Encoding, format);
            return Ostr;
        }

        private ArraySegment<byte> CreateResponseHeader(int requestId)
        {
            ArraySegment<byte> requestHeader = new byte[Ice1Definitions.HeaderSize + 4];
            int pos = 0;
            Ice1Definitions.Magic.CopyTo(requestHeader.AsSpan(pos, Ice1Definitions.Magic.Length));
            pos += Ice1Definitions.Magic.Length;
            Ice1Definitions.ProtocolBytes.CopyTo(requestHeader.AsSpan(pos, Ice1Definitions.ProtocolBytes.Length));
            pos += Ice1Definitions.ProtocolBytes.Length;
            requestHeader[pos++] = (byte)Ice1Definitions.MessageType.ReplyMessage;
            pos++; // compression plabceholder
            OutputStream.WriteInt(Size, requestHeader.AsSpan(pos, 4));
            pos += 4;
            OutputStream.WriteInt(requestId, requestHeader.AsSpan(pos, 4));

            return requestHeader;
        }

        internal List<ArraySegment<byte>> GetResponseData(int requestId)
        {
            var data = new List<ArraySegment<byte>>(Data.Count + 1)
            {
                CreateResponseHeader(requestId)
            };
            data.AddRange(Data);
            return data;
        }
    }
}
