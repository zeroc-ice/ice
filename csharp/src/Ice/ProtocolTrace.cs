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
        internal static void TraceFrame(
            Communicator communicator,
            ReadOnlySpan<byte> header,
            OutgoingRequestFrame frame) =>
            TraceRequest("sending request",
                         communicator,
                         frame.Protocol,
                         header.GetFrameType(), // Frame type
                         header.GetCompressionStatus(), // Compression Status
                         header.GetFrameSize(frame.Protocol), // Request size
                         header.GetRequestId(), // Request-Id
                         frame.Identity,
                         frame.Facet,
                         frame.Operation,
                         frame.IsIdempotent,
                         frame.Context,
                         frame.Encoding);

        internal static void TraceFrame(
            Communicator communicator,
            ReadOnlySpan<byte> header,
            OutgoingResponseFrame frame) =>
            TraceResponse("sending response",
                          communicator,
                          frame.Protocol,
                          header.GetFrameType(), // Frame type
                          header.GetCompressionStatus(), // Compression Status
                          header.GetFrameSize(frame.Protocol), // Response size
                          header.GetRequestId(), // Request-Id,
                          frame.ResultType,
                          frame.Encoding);

        internal static void TraceFrame(
            Communicator communicator,
            ReadOnlySpan<byte> header,
            IncomingRequestFrame frame) =>
            TraceRequest("received request",
                         communicator,
                         frame.Protocol,
                         header.GetFrameType(), // Frame type
                         header.GetCompressionStatus(), // Compression Status
                         header.GetFrameSize(frame.Protocol), // Request size
                         header.GetRequestId(), // Request-Id,
                         frame.Identity,
                         frame.Facet,
                         frame.Operation,
                         frame.IsIdempotent,
                         frame.Context,
                         frame.Encoding);

        internal static void TraceFrame(
            Communicator communicator,
            ReadOnlySpan<byte> header,
            IncomingResponseFrame frame) =>
            TraceResponse("received response",
                          communicator,
                          frame.Protocol,
                          header.GetFrameType(), // Frame type
                          header.GetCompressionStatus(), // Compression Status
                          header.GetFrameSize(frame.Protocol), // Request size
                          header.GetRequestId(), // Request-Id,
                          frame.ResultType,
                          frame.Encoding);

        internal static void TraceCollocatedFrame(
            Communicator communicator,
            byte frameType,
            int requestId,
            OutgoingRequestFrame frame) =>
            TraceRequest("sending request",
                         communicator,
                         frame.Protocol,
                         frameType,
                         0,
                         frame.GetFrameSize(),
                         requestId,
                         frame.Identity,
                         frame.Facet,
                         frame.Operation,
                         frame.IsIdempotent,
                         frame.Context,
                         frame.Encoding);

        internal static void TraceCollocatedFrame(
            Communicator communicator,
            byte frameType,
            int requestId,
            IncomingResponseFrame frame) =>
            TraceResponse("received response",
                          communicator,
                          frame.Protocol,
                          frameType,
                          0,
                          frame.GetFrameSize(),
                          requestId,
                          frame.ResultType,
                          frame.Encoding);

        private static void TraceRequest(
            string traceFramePrefix,
            Communicator communicator,
            Protocol protocol,
            byte frameType,
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
                s.Append(traceFramePrefix);
                PrintHeader(protocol, frameType, compress, size, s);
                PrintRequestId(requestId, s);

                ToStringMode toStringMode = communicator.ToStringMode;
                s.Append("\nidentity = ");
                s.Append(identity.ToString(toStringMode));

                s.Append("\nfacet = ");
                if (facet.Length > 0)
                {
                    s.Append(StringUtil.EscapeString(facet, toStringMode));
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
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCategory, s.ToString());
            }
        }

        private static void TraceResponse(
            string traceFramePrefix,
            Communicator communicator,
            Protocol protocol,
            byte frameType,
            byte compress,
            int size,
            int requestId,
            ResultType resultType,
            Encoding encoding)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                var s = new StringBuilder();
                s.Append(traceFramePrefix);
                PrintHeader(protocol, frameType, compress, size, s);
                PrintRequestId(requestId, s);
                s.Append("\nresult type = ");
                s.Append(resultType);
                s.Append("\nencoding = ");
                s.Append(encoding.ToString());
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCategory, s.ToString());
            }
        }

        internal static void TraceSend(Communicator communicator, Protocol protocol, ReadOnlySpan<byte> header) =>
            Trace("sending ", communicator, protocol, header);

        internal static void TraceReceived(Communicator communicator, Protocol protocol, ReadOnlySpan<byte> header) =>
            Trace("received ", communicator, protocol, header);

        internal static void Trace(
            string traceFramePrefix,
            Communicator communicator,
            Protocol protocol,
            ReadOnlySpan<byte> header)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                var s = new StringBuilder();
                s.Append(traceFramePrefix);
                s.Append(GetFrameTypeAsString(protocol, header.GetFrameType()));
                PrintHeader(protocol,
                            header.GetFrameType(),
                            header.GetCompressionStatus(),
                            header.GetFrameSize(protocol),
                            s);
                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCategory, s.ToString());
            }
        }

        private static void PrintHeader(Protocol protocol, byte frameType, byte compress, int size, StringBuilder s)
        {
            s.Append("\nprotocol = ");
            s.Append(protocol.GetName());
            s.Append("\nframe type = ");
            s.Append(GetFrameTypeAsString(protocol, frameType));
            s.Append("\ncompression status = ");
            s.Append(compress);
            s.Append(compress switch
            {
                0 => " (not compressed)",
                1 => " (not compressed)",
                2 => " (compressed)",
                _ => " (unknown)"
            });

            s.Append("\nframe size = ");
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

        private static string GetFrameTypeAsString(Protocol protocol, byte type)
        {
            if (protocol == Protocol.Ice2)
            {
                return (Ice2Definitions.FrameType)type switch
                    {
                        Ice2Definitions.FrameType.Request => "request",
                        Ice2Definitions.FrameType.Reply => "reply",
                        Ice2Definitions.FrameType.ValidateConnection => "validate connection",
                        Ice2Definitions.FrameType.CloseConnection => "close connection",
                        _ => "unknown",
                    };
            }
            else
            {
                return (Ice1Definitions.FrameType)type switch
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
}
