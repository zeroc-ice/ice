// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
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
            Initialize = 0,
            Request = 1,
            Response = 2,
            GoAway = 3
        }

        /// <summary>Writes a request header body without constructing an Ice2RequestHeaderBody instance. This
        /// implementation is slightly more efficient than the generated code because it avoids the allocation of a
        /// string[] to write the location.</summary>
        internal static void WriteIce2RequestHeaderBody(
            this OutputStream ostr,
            Identity identity,
            string facet,
            IReadOnlyList<string> location,
            string operation,
            bool idempotent,
            DateTime deadline)
        {
            Debug.Assert(ostr.Encoding == Encoding);
            BitSequence bitSequence = ostr.WriteBitSequence(4); // bit set to true (set) by default

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

            if (idempotent)
            {
                ostr.WriteBool(true);
            }
            else
            {
                bitSequence[2] = false;
            }

            bitSequence[3] = false; // TODO: source for priority.

            // DateTime.MaxValue represents an infinite deadline and it is encoded as -1
            ostr.WriteVarLong(
                deadline == DateTime.MaxValue ? -1 : (long)(deadline - DateTime.UnixEpoch).TotalMilliseconds);
        }
    }
}
