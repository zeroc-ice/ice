//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;

namespace ZeroC.Ice
{
    internal static class TraceUtil
    {
        internal static void TraceFrame(Communicator communicator, Span<byte> header, OutgoingRequestFrame frame) =>
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

        internal static void TraceFrame(Communicator communicator, Span<byte> header, OutgoingResponseFrame frame) =>
            TraceResponse(
                "sending response",
                communicator,
                header,
                frame.ReplyStatus,
                frame.Encoding);

        internal static void TraceFrame(Communicator communicator, Span<byte> header, IncomingRequestFrame frame) =>
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

        internal static void TraceFrame(Communicator communicator, Span<byte> header, IncomingResponseFrame frame) =>
            TraceResponse(
                "received response",
                communicator,
                header,
                frame.ReplyStatus,
                frame.Encoding);

        private static void TraceRequest(
            string heading,
            Communicator communicator,
            Span<byte> header,
            Identity identity,
            string facet,
            string operation,
            bool isIdempotent,
            IReadOnlyDictionary<string, string> context,
            Encoding encoding)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                using var s = new StringWriter(CultureInfo.CurrentCulture);
                s.Write(heading);
                PrintHeader(header, s);
                PrintRequestId(header, s);

                ToStringMode toStringMode = communicator.ToStringMode;
                s.Write("\nidentity = ");
                s.Write(identity.ToString(toStringMode));

                s.Write("\nfacet = ");
                if (facet.Length > 0)
                {
                    s.Write(IceUtilInternal.StringUtil.EscapeString(facet, "", toStringMode));
                }

                s.Write("\noperation = ");
                s.Write(operation);

                OperationMode mode = isIdempotent ? OperationMode.Idempotent : OperationMode.Normal;
                s.Write("\noperation mode = ");
                s.Write((byte)mode);
                s.Write(mode switch
                {
                    OperationMode.Normal => " (non-idempotent)",
                    _ => " (idempotent)",
                });

                int sz = context.Count;
                s.Write("\ncontext = ");
                foreach ((string key, string value) in context)
                {
                    s.Write(key);
                    s.Write('/');
                    s.Write(value);
                    if (--sz > 0)
                    {
                        s.Write(", ");
                    }
                }
                s.Write("\nencoding = ");
                s.Write(encoding.ToString());
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        private static void TraceResponse(
            string heading,
            Communicator communicator,
            Span<byte> header,
            ReplyStatus replyStatus,
            Encoding encoding)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                using var s = new StringWriter(CultureInfo.CurrentCulture);
                s.Write(heading);
                PrintHeader(header, s);
                PrintRequestId(header, s);
                s.Write("\nreply status = ");
                s.Write(replyStatus);
                s.Write("\nencoding = ");
                s.Write(encoding.ToString());
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        internal static void TraceSend(Communicator communicator, Span<byte> header) =>
            Trace("sending ", communicator, header);

        internal static void TraceReceived(Communicator communicator, Span<byte> header) =>
            Trace("received ", communicator, header);

        internal static void Trace(string heading, Communicator communicator, Span<byte> header)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                using var s = new StringWriter(CultureInfo.CurrentCulture);
                s.Write(heading);
                s.Write(GetFrameTypeAsString((Ice1Definitions.FrameType)header[8]));
                PrintHeader(header, s);
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        private static void PrintHeader(Span<byte> header, StringWriter s)
        {
            byte messageType = header[8];
            s.Write("\nmessage type = ");
            s.Write(messageType);
            s.Write((Ice1Definitions.FrameType)messageType switch
            {
                Ice1Definitions.FrameType.Request => " (request)",
                Ice1Definitions.FrameType.RequestBatch => " (batch request)",
                Ice1Definitions.FrameType.Reply => " (reply)",
                Ice1Definitions.FrameType.CloseConnection => " (close connection)",
                Ice1Definitions.FrameType.ValidateConnection => " (validate connection)",
                _ => " (unknown)"
            });

            byte compress = header[9];
            s.Write("\ncompression status = ");
            s.Write(compress);
            s.Write(compress switch
            {
                0 => " (not compressed; do not compress response, if any)",
                1 => " (not compressed; compress response, if any)",
                2 => " (compressed; compress response, if any)",
                _ => " (unknown)"
            });

            s.Write("\nmessage size = ");
            s.Write(InputStream.ReadInt(header.Slice(10, 4)));
        }

        private static void PrintRequestId(Span<byte> header, StringWriter s)
        {
            int requestId = InputStream.ReadInt(header.Slice(Ice1Definitions.HeaderSize, 4));
            s.Write("\nrequest id = ");
            s.Write(requestId);
            if (requestId == 0)
            {
                s.Write(" (oneway)");
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
