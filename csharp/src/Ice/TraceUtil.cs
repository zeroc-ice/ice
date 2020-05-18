//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Globalization;
using System.IO;

namespace ZeroC.Ice
{
    internal static class TraceUtil
    {
        internal static void TraceSendRequest(Communicator communicator, OutgoingRequestFrame request, int size,
            int requestId, byte compress) =>
            TraceRequest("sending request", communicator, size, requestId, compress, request.Identity,
                request.Facet,
                request.Operation,
                request.IsIdempotent ? OperationMode.Idempotent : OperationMode.Normal,
                request.Context,
                request.Encoding);

        internal static void TraceReceivedRequest(Communicator communicator,
            IncomingRequestFrame request, int size, int requestId, byte compress) =>
            TraceRequest("received request", communicator, size, requestId, compress, request.Identity,
                request.Facet,
                request.Operation,
                request.IsIdempotent ? OperationMode.Idempotent : OperationMode.Normal,
                request.Context, request.Encoding);

        private static void TraceRequest(string heading, Communicator communicator, int size, int requestId,
            byte compress, Identity identity, string facet, string operation, OperationMode mode,
            IReadOnlyDictionary<string, string> context, Encoding encoding)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                using var s = new StringWriter(CultureInfo.CurrentCulture);
                s.Write(heading);
                PrintFrameHeader(Ice1Definitions.FrameType.Request, compress, size, s);
                s.Write("\nrequest id = " + requestId);
                if (requestId == 0)
                {
                    s.Write(" (oneway)");
                }

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

        internal static void TraceSendResponse(Communicator communicator, OutgoingResponseFrame response,
            int size, int requestId, byte compress) =>
            TraceResponse("sending reply", communicator, size, requestId, compress, response.ReplyStatus,
                response.Encoding);

        internal static void TraceReceivedResponse(Communicator communicator, IncomingResponseFrame response,
            int size, int requestId, byte compress) =>
            TraceResponse("received reply", communicator, size, requestId, compress, response.ReplyStatus,
                response.Encoding);

        internal static void TraceResponse(string heading, Communicator communicator, int size, int requestId,
            byte compress, ReplyStatus replyStatus, Encoding encoding)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                using var s = new StringWriter(CultureInfo.CurrentCulture);
                s.Write(heading);
                PrintFrameHeader(Ice1Definitions.FrameType.Reply, compress, size, s);
                s.Write("\nrequest id = ");
                s.Write(requestId);
                if (requestId == 0)
                {
                    s.Write(" (oneway)");
                }
                s.Write("\nreply status = ");
                s.Write(replyStatus);
                s.Write("\nencoding = ");
                s.Write(encoding.ToString());
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        internal static void TraceHeader(Communicator communicator, Ice1Definitions.FrameType type, byte compress,
            int size, string heading)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                using var s = new StringWriter(CultureInfo.CurrentCulture);
                s.Write(heading);
                s.Write(GetFrameTypeAsString(type));
                PrintFrameHeader(type, compress, size, s);
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCat, s.ToString());
            }
        }

        private static void PrintFrameHeader(Ice1Definitions.FrameType type, byte compress, int size, StringWriter s)
        {
            s.Write("\nmessage type = ");
            s.Write((byte)type);
            s.Write(type switch
            {
                Ice1Definitions.FrameType.Request => " (request)",
                Ice1Definitions.FrameType.RequestBatch => " (batch request)",
                Ice1Definitions.FrameType.Reply => " (reply)",
                Ice1Definitions.FrameType.CloseConnection => " (close connection)",
                Ice1Definitions.FrameType.ValidateConnection => " (validate connection)",
                _ => " (unknown)"
            });

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
            s.Write(size);
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
