//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace ZeroC.Ice
{
    internal static class ProtocolTrace
    {
        internal static void TraceFrame(
            Communicator communicator,
            long streamId,
            object frame,
            byte compress = 0)
        {
            if (communicator.TraceLevels.Protocol >= 1)
            {
                string framePrefix;
                string frameType;
                Protocol protocol;
                Encoding encoding;
                int frameSize;

                if (frame is OutgoingFrame outgoingFrame)
                {
                    framePrefix = "sending";
                    protocol = outgoingFrame.Protocol;
                    encoding = outgoingFrame.Encoding;
                    frameType = frame is OutgoingRequestFrame ? "request" : "response";
                    frameSize = outgoingFrame.Size;
                }
                else if (frame is IncomingFrame incomingFrame)
                {
                    framePrefix = "received";
                    protocol = incomingFrame.Protocol;
                    encoding = incomingFrame.Encoding;
                    frameType = frame is IncomingRequestFrame ? "request" : "response";
                    frameSize = incomingFrame.Size;
                }
                else
                {
                    Debug.Assert(false);
                    return;
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
                else
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
                    s.Append((int)streamId);
                    if (streamId == 0)
                    {
                        s.Append(" (oneway)");
                    }
                }

                if (frame is OutgoingRequestFrame || frame is IncomingRequestFrame)
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
                else
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
                    else
                    {
                        Debug.Assert(false);
                    }
                }

                communicator.Logger.Trace(communicator.TraceLevels.ProtocolCategory, s.ToString());
            }
        }
    }
}
