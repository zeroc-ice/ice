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

        private static readonly byte[] _voidReturnValuePayload11 = new byte[] { 0, 8, 1, 1 }; // size 2 encoded as 2 * 4
        private static readonly byte[] _voidReturnValuePayload20 = new byte[] { 0, 12, 2, 0, 0 };

        /// <summary>Returns the payload of an ice2 request frame for an operation with no argument.</summary>
        /// <param name="encoding">The encoding of this empty args payload. The header of this payload is always encoded
        /// using ice2's header encoding (2.0).</param>
        /// <returns>The payload.</returns>
        /// <remarks>The 2.0 encoding has an extra byte for the compression status.</remarks>
        internal static ArraySegment<byte> GetEmptyArgsPayload(Encoding encoding) =>
            GetVoidReturnValuePayload(encoding).Slice(1);

        /// <summary>Returns the payload of an ice2 response frame for an operation returning void.</summary>
        /// <param name="encoding">The encoding of this void return. The header of this payload is always encoded
        /// using ice2's header encoding (2.0).</param>
        /// <returns>The payload.</returns>
        internal static ArraySegment<byte> GetVoidReturnValuePayload(Encoding encoding)
        {
            encoding.CheckSupported();
            return encoding == Encoding.V11 ? _voidReturnValuePayload11 : _voidReturnValuePayload20;
        }

        /// <summary>Writes a request header body This implementation is slightly more efficient than the generated code
        /// because it avoids the allocation of a string[] to write the location.</summary>
        internal static void WriteIce2RequestHeaderBody(
            this OutputStream ostr,
            Identity identity,
            string facet,
            IReadOnlyList<string> location,
            string operation,
            bool idempotent,
            DateTime deadline,
            IReadOnlyDictionary<string, string> context)
        {
            Debug.Assert(ostr.Encoding == Encoding);

            // All bits are set to true by default, and true means the corresponding value is set.
            BitSequence bitSequence = ostr.WriteBitSequence(5);

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

            if (context.Count > 0)
            {
                ostr.WriteDictionary(context, OutputStream.IceWriterFromString, OutputStream.IceWriterFromString);
            }
            else
            {
                bitSequence[4] = false;
            }
        }
    }
}
