//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace ZeroC.Ice
{
    internal static class ProtocolTrace
    {
        internal static void TraceFrame(
            Endpoint endpoint,
            long streamId,
            object frame,
            byte type = 0,
            byte compress = 0)
        {
            Communicator communicator = endpoint.Communicator;
            Protocol protocol = endpoint.Protocol;
            if (communicator.TraceLevels.Protocol >= 1)
            {
                string framePrefix;
                string frameType;
                Encoding encoding;
                int frameSize;
                ArraySegment<byte> data = ArraySegment<byte>.Empty;

                if (frame is OutgoingFrame outgoingFrame)
                {
                    framePrefix = "sent";
                    encoding = outgoingFrame.Encoding;
                    frameType = frame is OutgoingRequestFrame ? "request" : "response";
                    frameSize = outgoingFrame.Size;
                }
                else if (frame is IncomingFrame incomingFrame)
                {
                    framePrefix = "received";
                    encoding = incomingFrame.Encoding;
                    frameType = frame is IncomingRequestFrame ? "request" : "response";
                    frameSize = incomingFrame.Size;
                }
                else
                {
                    if (frame is IList<ArraySegment<byte>> sendBuffer)
                    {
                        framePrefix = "sending";
                        data = sendBuffer.Count > 0 ? sendBuffer[0] : ArraySegment<byte>.Empty;
                    }
                    else if (frame is ArraySegment<byte> readBuffer)
                    {
                        framePrefix = "received";
                        data = readBuffer;
                    }
                    else
                    {
                        Debug.Assert(false);
                        return;
                    }

                    if (protocol == Protocol.Ice2)
                    {
                        frameType = (Ice2Definitions.FrameType)type switch
                        {
                            Ice2Definitions.FrameType.Initialize => "initialize",
                            Ice2Definitions.FrameType.Close => "close",
                            _ => "unknown"
                        };
                        encoding = Ice2Definitions.Encoding;
                        frameSize = 0;
                    }
                    else
                    {
                        frameType = (Ice1Definitions.FrameType)type switch
                        {
                            Ice1Definitions.FrameType.ValidateConnection => "validate",
                            Ice1Definitions.FrameType.CloseConnection => "close",
                            Ice1Definitions.FrameType.RequestBatch => "batch request",
                            _ => "unknown"
                        };
                        encoding = Ice1Definitions.Encoding;
                        frameSize = 0;
                    }
                }

                var s = new StringBuilder();
                s.Append(framePrefix);
                s.Append(' ');
                s.Append(frameType);

                s.Append("\nprotocol = ");
                s.Append(protocol.GetName());
                s.Append("\nencoding = ");
                s.Append(encoding.ToString());

                s.Append("\nframe size = ");
                s.Append(frameSize);

                if (protocol == Protocol.Ice2)
                {
                    s.Append("\nstream ID = ");
                    s.Append(streamId);
                }
                else if (frameType == "request" || frameType == "response")
                {
                    s.Append("\ncompression status = ");
                    s.Append(compress);
                    s.Append(compress switch
                    {
                        0 => " (not compressed; do not compress response, if any)",
                        1 => " (not compressed; compress response, if any)",
                        2 => " (compressed; compress response, if any)",
                        _ => " (unknown)"
                    });

                    s.Append("\nrequest ID = ");
                    int requestId = streamId % 4 < 2 ? (int)(streamId >> 2) + 1 : 0;
                    s.Append((int)requestId);
                    if (requestId == 0)
                    {
                        s.Append(" (oneway)");
                    }
                }

                if (frameType == "request")
                {
                    Identity identity;
                    string facet;
                    string operation;
                    bool isIdempotent;
                    IReadOnlyDictionary<string, string> context;
                    if (frame is OutgoingRequestFrame outgoingRequest)
                    {
                        identity = outgoingRequest.Identity;
                        facet = outgoingRequest.Facet;
                        operation = outgoingRequest.Operation;
                        isIdempotent = outgoingRequest.IsIdempotent;
                        context = outgoingRequest.Context;
                    }
                    else if (frame is IncomingRequestFrame incomingRequest)
                    {
                        Debug.Assert(incomingRequest != null);
                        identity = incomingRequest.Identity;
                        facet = incomingRequest.Facet;
                        operation = incomingRequest.Operation;
                        isIdempotent = incomingRequest.IsIdempotent;
                        context = incomingRequest.Context;
                    }
                    else
                    {
                        Debug.Assert(false);
                        return;
                    }

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

                    s.Append($"\nidempotent = ");
                    s.Append(isIdempotent.ToString().ToLower());

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
                }
                else if (frameType == "response")
                {
                    s.Append("\nresult type = ");
                    if (frame is IncomingResponseFrame incomingResponseFrame)
                    {
                        s.Append(incomingResponseFrame.ResultType);
                    }
                    else if (frame is OutgoingResponseFrame outgoingResponseFrame)
                    {
                        s.Append(outgoingResponseFrame.ResultType);
                    }
                }
                else if (frameType == "batch request")
                {
                    s.Append("\nnumber of requests = ");
                    s.Append(data.AsReadOnlySpan().ReadInt());
                }
                else if (protocol == Protocol.Ice2 && frameType == "close")
                {
                    var istr = new InputStream(data, encoding);
                    s.Append("\nlast stream ID = ");
                    s.Append(istr.ReadVarLong());
                    s.Append("\nreason = ");
                    s.Append(istr.ReadString());
                }

                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCategory, s.ToString());
            }
        }
    }
}
