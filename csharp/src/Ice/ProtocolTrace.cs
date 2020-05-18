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
                header,
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
                header,
                frame.ReplyStatus,
                frame.Encoding);

        internal static void TraceFrame(Communicator communicator, ReadOnlySpan<byte> header,
            IncomingRequestFrame frame) =>
            TraceRequest(
                "received request",
                communicator,
                header,
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
                header,
                frame.ReplyStatus,
                frame.Encoding);

        private static void TraceRequest(
            string traceMessagePrefix,
            Communicator communicator,
            ReadOnlySpan<byte> header,
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
                PrintHeader(header, s);
                PrintRequestId(header, s);

                ToStringMode toStringMode = communicator.ToStringMode;
                s.Append("\nidentity = ");
                s.Append(identity.ToString(toStringMode));

                s.Append("\nfacet = ");
                if (facet.Length > 0)
                {
                    s.Append(IceUtilInternal.StringUtil.EscapeString(facet, "", toStringMode));
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
            ReadOnlySpan<byte> header,
            ReplyStatus replyStatus,
            Encoding encoding)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                var s = new StringBuilder();
                s.Append(traceMessagePrefix);
                PrintHeader(header, s);
                PrintRequestId(header, s);
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
                PrintHeader(header, s);
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        private static void PrintHeader(ReadOnlySpan<byte> header, StringBuilder s)
        {
            byte messageType = header[8];
            s.Append("\nmessage type = ");
            s.Append(messageType);
            s.Append(" ");
            s.Append(GetFrameTypeAsString((Ice1Definitions.FrameType)messageType));

            byte compress = header[9];
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
            s.Append(InputStream.ReadInt(header.Slice(10, 4)));
        }

        private static void PrintRequestId(ReadOnlySpan<byte> header, StringBuilder s)
        {
            int requestId = InputStream.ReadInt(header.Slice(Ice1Definitions.HeaderSize, 4));
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
                Ice1Definitions.FrameType.CloseConnection => "close connection",
                Ice1Definitions.FrameType.ValidateConnection => "validate connection",
                _ => "unknown",
            };
        }
    }
}
