// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    // Definitions for the ice2 protocol.

    internal static class Ice2Definitions
    {
        internal static readonly Encoding Encoding = Encoding.V20;

        // ice2 frame types:
        internal enum FrameType : byte
        {
            Request = 0,
            Response = 2,
        }

        /// <summary>Helper method to write the request header body without constructing an Ice2RequestHeaderBody
        /// struct.</summary>
        internal static void WriteIce2RequestHeaderBody(
            this OutputStream ostr,
            Identity identity,
            string facet,
            IReadOnlyList<string> location,
            string operation,
            bool idempotent)
        {
            Debug.Assert(ostr.Encoding == Encoding);
            BitSequence bitSequence = ostr.WriteBitSequence(3); // bit set to true (set) by default

            identity.IceWrite(ostr);
            if (facet.Length > 0)
            {
                ostr.WriteString(facet);
            }
            else
            {
                bitSequence[0] = false;
            }

            if (location.Count > 0)
            {
                ostr.WriteSequence(location, OutputStream.IceWriterFromString);
            }
            else
            {
                bitSequence[1] = false;
            }

            ostr.WriteString(operation);
            ostr.WriteBool(idempotent);
            bitSequence[2] = false; // TODO: source for priority.
        }
    }
}
