//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    /// <summary>Represents a response protocol frame sent by the application.</summary>
    public sealed class OutgoingResponseFrame
    {
        /// <summary>The encoding of the frame payload</summary>
        public Encoding Encoding { get; }
        /// <summary>True for a sealed frame, false otherwise, a sealed frame does not change its contents.</summary>
        public bool IsSealed { get; private set; }

        /// <summary>Returns a list of array segments with the contents of the frame payload.</summary>
        public IList<ArraySegment<byte>> Payload => Data;

        /// <summary>The frame byte count.</summary>
        public int Size { get; private set; }

        // Contents of the Frame
        internal List<ArraySegment<byte>> Data { get; private set; }

        // Position of the end of the payload, for Ice1 this is always the frame end.
        private OutputStream.Position? _payloadEnd;

        /// <summary>Creates a new outgoing request frame with an OK reply status and a void return value.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        public static OutgoingResponseFrame Empty(Encoding encoding)
            => new OutgoingResponseFrame(encoding, ArraySegment<byte>.Empty);

        /// <summary>Creates a new outgoing response frame. It's a partial frame, and its payload needs to be
        /// written later on with WritePayload.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        public OutgoingResponseFrame(Encoding encoding)
        {
            Encoding = encoding;
            Encoding.CheckSupported();
            Data = new List<ArraySegment<byte>>();
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
                _payloadEnd = new OutputStream.Position(0, payload.Count);
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
            if (exception is RequestFailedException requestFailedException)
            {
                var ostr = new OutputStream(Encoding, Data, new OutputStream.Position(0, 0));
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
                    ostr.WriteString(requestFailedException.Message);
                }
                _payloadEnd = ostr.Save();
                Size = Data.GetByteCount();
                IsSealed = true;
            }
            else
            {
                OutputStream ostr = WritePayload(ReplyStatus.UserException, FormatType.SlicedFormat);
                ostr.WriteException(exception);
                ostr.Save();
            }
        }

        /// <summary>Starts writing the return value for a successful response.</summary>
        /// <param name="replyStatus">The response reply status.</param>
        /// <param name="format">The format for the return value, null (meaning keep communicator's setting),
        /// SlicedFormat or CompactFormat.</param>
        public OutputStream WritePayload(ReplyStatus replyStatus, FormatType? format = null)
        {
            if (_payloadEnd != null)
            {
                throw new InvalidOperationException("the frame already contains a payload");
            }
            Debug.Assert(Data.Count == 0);
            byte[] buffer = new byte[256];
            buffer[0] = (byte)replyStatus;
            Data.Add(buffer);
            return new OutputStream(Encoding, Data, new OutputStream.Position(0, 1), true, format,
                payloadEnd => PayloadReady(this, payloadEnd));
        }

        internal static void PayloadReady(OutgoingResponseFrame frame, OutputStream.Position payloadEnd)
        {
            if (frame._payloadEnd != null)
            {
                throw new InvalidOperationException("the frame already contains a payload");
            }

            frame.Size = frame.Data.GetByteCount();
            frame._payloadEnd = payloadEnd;
            frame.IsSealed = true;
        }
    }
}
