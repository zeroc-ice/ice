//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Text;

namespace ZeroC.Ice
{
    internal static class ProtocolTrace
    {
        internal static void TraceFrame(Communicator communicator, ReadOnlySpan<byte> header,
            OutgoingRequestFrame frame) =>
            TraceRequest(
                "sending request",
                communicator,
                header[8], // Message Type
                header[9], // Compression Status
                InputStream.ReadInt(header.Slice(10, 4)), // Request size
                InputStream.ReadInt(header.Slice(14, 4)), // Request-Id
                frame.Identity,
                frame.Facet,
                frame.Operation,
                frame.IsIdempotent,
                frame.Context,
                frame.Encoding);

        internal static void TraceFrame(Communicator communicator, ReadOnlySpan<byte> header,
            OutgoingResponseFrame frame) =>
            TraceResponse(
                "sending response",
                communicator,
                header[8], // Message Type
                header[9], // Compression Status
                InputStream.ReadInt(header.Slice(10, 4)), // Request size
                InputStream.ReadInt(header.Slice(14, 4)), // Request-Id,
                frame.ReplyStatus,
                frame.Encoding);

        internal static void TraceFrame(Communicator communicator, ReadOnlySpan<byte> header,
            IncomingRequestFrame frame) =>
            TraceRequest(
                "received request",
                communicator,
                header[8], // Message Type
                header[9], // Compression Status
                InputStream.ReadInt(header.Slice(10, 4)), // Request size
                InputStream.ReadInt(header.Slice(14, 4)), // Request-Id,
                frame.Identity,
                frame.Facet,
                frame.Operation,
                frame.IsIdempotent,
                frame.Context,
                frame.Encoding);

        internal static void TraceFrame(Communicator communicator, ReadOnlySpan<byte> header,
            IncomingResponseFrame frame) =>
            TraceResponse(
                "received response",
                communicator,
                header[8], // Message Type
                header[9], // Compression Status
                InputStream.ReadInt(header.Slice(10, 4)), // Request size
                InputStream.ReadInt(header.Slice(14, 4)), // Request-Id,
                frame.ReplyStatus,
                frame.Encoding);

        internal static void TraceCollocatedFrame(Communicator communicator, byte messageType, int requestId,
            OutgoingRequestFrame frame) =>
            TraceRequest(
                "sending request",
                communicator,
                messageType,
                0,
                frame.Size + Ice1Definitions.HeaderSize + 4,
                requestId,
                frame.Identity,
                frame.Facet,
                frame.Operation,
                frame.IsIdempotent,
                frame.Context,
                frame.Encoding);

        internal static void TraceCollocatedFrame(Communicator communicator, byte messageType, int requestId,
            IncomingResponseFrame frame) =>
            TraceResponse(
                "received response",
                communicator,
                messageType,
                0,
                frame.Size + Ice1Definitions.HeaderSize + 4,
                requestId,
                frame.ReplyStatus,
                frame.Encoding);

        private static void TraceRequest(
            string traceMessagePrefix,
            Communicator communicator,
            byte messageType,
            byte compress,
            int size,
            int requestId,
            Identity identity,
            string facet,
            string operation,
            bool isIdempotent,
            IReadOnlyDictionary<string, string> context,
            Encoding encoding)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                var s = new StringBuilder();
                s.Append(traceMessagePrefix);
                PrintHeader(messageType, compress, size, s);
                PrintRequestId(requestId, s);

                ToStringMode toStringMode = communicator.ToStringMode;
                s.Append("\nidentity = ");
                s.Append(identity.ToString(toStringMode));

                s.Append("\nfacet = ");
                if (facet.Length > 0)
                {
                    s.Append(StringUtil.EscapeString(facet, "", toStringMode));
                }

                s.Append("\noperation = ");
                s.Append(operation);

                OperationMode mode = isIdempotent ? OperationMode.Idempotent : OperationMode.Normal;
                s.Append("\noperation mode = ");
                s.Append((byte)mode);
                s.Append(mode switch
                {
                    OperationMode.Normal => " (non-idempotent)",
                    _ => " (idempotent)",
                });

                int sz = context.Count;
                s.Append("\ncontext = ");
                foreach ((string key, string value) in context)
                {
                    s.Append(key);
                    s.Append('/');
                    s.Append(value);
                    if (--sz > 0)
                    {
                        s.Append(", ");
                    }
                }
                s.Append("\nencoding = ");
                s.Append(encoding.ToString());
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        private static void TraceResponse(
            string traceMessagePrefix,
            Communicator communicator,
            byte messageType,
            byte compress,
            int size,
            int requestId,
            ReplyStatus replyStatus,
            Encoding encoding)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                var s = new StringBuilder();
                s.Append(traceMessagePrefix);
                PrintHeader(messageType, compress, size, s);
                PrintRequestId(requestId, s);
                s.Append("\nreply status = ");
                s.Append(replyStatus);
                s.Append("\nencoding = ");
                s.Append(encoding.ToString());
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        internal static void TraceSend(Communicator communicator, ReadOnlySpan<byte> header) =>
            Trace("sending ", communicator, header);

        internal static void TraceReceived(Communicator communicator, ReadOnlySpan<byte> header) =>
            Trace("received ", communicator, header);

        internal static void Trace(string traceMessagePrefix, Communicator communicator, ReadOnlySpan<byte> header)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                var s = new StringBuilder();
                s.Append(traceMessagePrefix);
                s.Append(GetFrameTypeAsString((Ice1Definitions.FrameType)header[8]));
                PrintHeader(header[8], header[9], InputStream.ReadInt(header.Slice(10, 4)), s);
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        private static void PrintHeader(byte messageType, byte compress, int size, StringBuilder s)
        {
            s.Append("\nmessage type = ");
            s.Append(GetFrameTypeAsString((Ice1Definitions.FrameType)messageType));

            s.Append("\ncompression status = ");
            s.Append(compress);
            s.Append(compress switch
            {
                0 => " (not compressed; do not compress response, if any)",
                1 => " (not compressed; compress response, if any)",
                2 => " (compressed; compress response, if any)",
                _ => " (unknown)"
            });

            s.Append("\nmessage size = ");
            s.Append(size);
        }

        private static void PrintRequestId(int requestId, StringBuilder s)
        {
            s.Append("\nrequest id = ");
            s.Append(requestId);
            if (requestId == 0)
            {
                s.Append(" (oneway)");
            }
        }

        private static string GetFrameTypeAsString(Ice1Definitions.FrameType type)
        {
            return type switch
            {
                Ice1Definitions.FrameType.Request => "request",
                Ice1Definitions.FrameType.RequestBatch => "batch request",
                Ice1Definitions.FrameType.Reply => "reply",
                Ice1Definitions.FrameType.ValidateConnection => "validate connection",
                Ice1Definitions.FrameType.CloseConnection => "close connection",
                _ => "unknown",
            };
        }
    }
}
