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

        /// <summary>Creates a new outgoing request frame with an OK reply status and a void return value.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        public static OutgoingResponseFrame WithVoidReturnValue(Encoding encoding)
            => new OutgoingResponseFrame(encoding, true);

        public static OutgoingResponseFrame WithReturnValue<T>(Encoding encoding, FormatType? format,
            in T value, OutputStreamWriter<T> writer)
        {
            var response = new OutgoingResponseFrame(encoding, false);
            byte[] buffer = new byte[256];
            buffer[0] = (byte)ReplyStatus.OK;
            response.Data.Add(buffer);
            var ostr = new OutputStream(encoding, response.Data, new OutputStream.Position(0, 1), format);
            writer(ostr, value);
            ostr.Save();
            response.Finish();
            return response;
        }

        public static OutgoingResponseFrame WithReturnValue<T>(Encoding encoding, FormatType? format, in T value,
            OutputStreamStructWriter<T> writer) where T : struct
        {
            var response = new OutgoingResponseFrame(encoding, false);
            byte[] buffer = new byte[256];
            buffer[0] = (byte)ReplyStatus.OK;
            response.Data.Add(buffer);
            var ostr = new OutputStream(encoding, response.Data, new OutputStream.Position(0, 1), format);
            writer(ostr, value);
            ostr.Save();
            response.Finish();
            return response;
        }

        /// <summary>Creates a new outgoing response frame with the given reply status and payload.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        /// <param name="payload">The payload for this response frame.</param>
        // TODO: add parameter such as "bool assumeOwnership" once we add memory pooling.
        public OutgoingResponseFrame(Encoding encoding, ArraySegment<byte> payload) :
            this(encoding, writeEmptyPayload: false)
        {
            if (payload[0] == (byte)ReplyStatus.OK || payload[0] == (byte)ReplyStatus.UserException)
            {
                // The minimum size for the payload is 7 bytes, the reply status byte plus 6 bytes of an
                // empty encapsulation.
                if (payload.Count < 7)
                {
                    throw new ArgumentException(
                        $"the response payload should contain at least 7 bytes, but it contains `{payload.Count}' bytes",
                        nameof(payload));
                }

                int size = InputStream.ReadInt(payload.AsSpan(1, 4));
                if (size != payload.Count - 1)
                {
                    throw new ArgumentException($"invalid payload size `{size}' expected `{payload.Count}'",
                        nameof(payload));
                }

                if (payload[5] != Encoding.Major || payload[6] != Encoding.Minor)
                {
                    throw new ArgumentException(@$"the payload encoding `{payload[5]}.{payload[6]}' must be the same
                                                   as the frame encoding `{Encoding.Major}.{Encoding.Minor}'",
                        nameof(payload));
                }
            }

            Data.Add(payload);
            Size = Data.GetByteCount();
            IsSealed = true;
        }

        /// <summary>Creates a response frame that represents "failure" and contains an exception.</summary>
        /// <param name="current">The current parameter holds decoded header data and other information about the
        /// request for which this constructor creates a response.</param>
        /// <param name="exception">The exception to store into the frame's payload.</param>
        public OutgoingResponseFrame(Current current, RemoteException exception)
            : this(current.Encoding, writeEmptyPayload: false)
        {
            OutputStream ostr;
            if (exception is RequestFailedException requestFailedException)
            {
                ostr = new OutputStream(Encoding, Data, new OutputStream.Position(0, 0));
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
            }
            else
            {
                byte[] buffer = new byte[256];
                buffer[0] = (byte)ReplyStatus.UserException;
                Data.Add(buffer);
                ostr = new OutputStream(Encoding, Data, new OutputStream.Position(0, 1), FormatType.SlicedFormat);
                ostr.WriteException(exception);
            }

            ostr.Save();
            Size = Data.GetByteCount();
            IsSealed = true;
        }

        private OutgoingResponseFrame(Encoding encoding, bool writeEmptyPayload)
        {
            Encoding = encoding;
            Data = new List<ArraySegment<byte>>();
            if (writeEmptyPayload)
            {
                Encoding.CheckSupported();
                byte[] buffer = new byte[256];
                int pos = 0;
                buffer[pos++] = (byte)ReplyStatus.OK;
                OutputStream.WriteInt(6, buffer.AsSpan(pos, 4));
                pos += 4;
                buffer[pos++] = encoding.Major;
                buffer[pos++] = encoding.Minor;
                Data.Add(new ArraySegment<byte>(buffer, 0, pos));
                Size = Data.GetByteCount();
                IsSealed = true;
            }
        }

        private void Finish()
        {
            Size = Data.GetByteCount();
            IsSealed = true;
        }
    }
}
