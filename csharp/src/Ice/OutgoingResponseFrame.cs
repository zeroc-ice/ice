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

        /// <summary>True for a sealed frame, false otherwise. Once sealed, a frame is read-only.</summary>
        public bool IsSealed { get; private set; }

        /// <summary>Returns a list of array segments with the contents of the frame payload.</summary>
        public IList<ArraySegment<byte>> Payload => Data;

        /// <summary>The frame byte count.</summary>
        public int Size { get; private set; }

        // Contents of the Frame
        internal List<ArraySegment<byte>> Data { get; private set; }

        /// <summary>Creates a new outgoing response frame with an OK reply status and a void return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithVoidReturnValue(Current current)
            => new OutgoingResponseFrame(current.Encoding, writeVoidReturnValue: true);

        /// <summary>Creates a new outgoing response frame with an OK reply status and a return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
        /// <param name="value">The return value to marshal.</param>
        /// <param name="writer">A delegate that must write the value to the frame.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(Current current,
                                                               FormatType? format,
                                                               T value,
                                                               OutputStreamWriter<T> writer)
        {
            var response = new OutgoingResponseFrame(current.Encoding);
            byte[] buffer = new byte[256];
            buffer[0] = (byte)ReplyStatus.OK;
            response.Data.Add(buffer);
            var ostr = new OutputStream(response.Encoding, response.Data, new OutputStream.Position(0, 1),
                 format ?? current.Adapter.Communicator.DefaultFormat);
            writer(ostr, value);
            ostr.Save();
            response.Finish();
            return response;
        }

        /// <summary>Creates a new outgoing response frame with an OK reply status and a return value.</summary>
        /// <param name="current">The Current object for the corresponding incoming request.</param>
        /// <param name="format">The format type used to marshal classes and exceptions, when this parameter is null
        /// the communicator's default format is used.</param>
        /// <param name="value">The return value to marshal, when the response frame contains multiple return
        /// values they must be passed in a tuple.</param>
        /// <param name="writer">A delegate that must write the value to the frame.</param>
        /// <returns>A new OutgoingResponseFrame.</returns>
        public static OutgoingResponseFrame WithReturnValue<T>(Current current,
                                                               FormatType? format,
                                                               in T value,
                                                               OutputStreamStructWriter<T> writer)
            where T : struct
        {
            var response = new OutgoingResponseFrame(current.Encoding);
            byte[] buffer = new byte[256];
            buffer[0] = (byte)ReplyStatus.OK;
            response.Data.Add(buffer);
            var ostr = new OutputStream(response.Encoding, response.Data, new OutputStream.Position(0, 1),
                format ?? current.Adapter.Communicator.DefaultFormat);
            writer(ostr, value);
            ostr.Save();
            response.Finish();
            return response;
        }

        /// <summary>Creates a new outgoing response frame with the given payload.</summary>
        /// <param name="encoding">The encoding for the frame payload.</param>
        /// <param name="payload">The payload for this response frame.</param>
        // TODO: add parameter such as "bool assumeOwnership" once we add memory pooling.
        // TODO: should we pass the payload as a list of segments, or maybe add a separate
        // ctor that accepts a list of segments instead of a single segment
        public OutgoingResponseFrame(Encoding encoding, ArraySegment<byte> payload)
            : this(encoding)
        {
            if (payload[0] == (byte)ReplyStatus.OK || payload[0] == (byte)ReplyStatus.UserException)
            {
                // The minimum size for the payload is 7 bytes, the reply status byte plus 6 bytes for an
                // empty encapsulation.
                if (payload.Count < 7)
                {
                    throw new ArgumentException(
                        $"{nameof(payload)} should contain at least 7 bytes, but it contains `{payload.Count}' bytes",
                        nameof(payload));
                }

                int size = InputStream.ReadInt(payload.AsSpan(1, 4));
                if (size != payload.Count - 1)
                {
                    throw new ArgumentException($"invalid payload size `{size}'; expected `{payload.Count}'",
                        nameof(payload));
                }

                if (payload[5] != Encoding.Major || payload[6] != Encoding.Minor)
                {
                    throw new ArgumentException(@$"the payload encoding `{payload[5]}.{payload[6]
                        }' must be the same as the supplied encoding `{Encoding.Major}.{Encoding.Minor}'",
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
            : this(current.Encoding)
        {
            OutputStream ostr;
            if (exception is DispatchException dispatchException)
            {
                ostr = new OutputStream(Encoding, Data, new OutputStream.Position(0, 0));
                if (dispatchException is PreExecutionException preExecutionException)
                {
                    ReplyStatus replyStatus = preExecutionException switch
                    {
                        ObjectNotExistException _ => ReplyStatus.ObjectNotExistException,
                        OperationNotExistException _ => ReplyStatus.OperationNotExistException,
                        _ => throw new ArgumentException("unknown PreExecutionException", nameof(exception))
                    };

                    ostr.WriteByte((byte)replyStatus);
                    preExecutionException.Id.IceWrite(ostr);
                    ostr.WriteFacet(preExecutionException.Facet);
                    ostr.WriteString(preExecutionException.Operation);
                }
                else
                {
                    ostr.WriteByte((byte)ReplyStatus.UnknownLocalException);
                    ostr.WriteString(dispatchException.Message);
                }
            }
            else
            {
                byte[] buffer = new byte[256];
                buffer[0] = (byte)ReplyStatus.UserException;
                Data.Add(buffer);
                ostr = new OutputStream(Encoding, Data, new OutputStream.Position(0, 1), FormatType.Sliced);
                ostr.WriteException(exception);
            }

            ostr.Save();
            Size = Data.GetByteCount();
            IsSealed = true;
        }

        private OutgoingResponseFrame(Encoding encoding, bool writeVoidReturnValue = false)
        {
            Encoding = encoding;
            Data = new List<ArraySegment<byte>>();
            if (writeVoidReturnValue)
            {
                Encoding.CheckSupported();
                Data.Add(Ice1Definitions.EmptyResponsePayload);
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
