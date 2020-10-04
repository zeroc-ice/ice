// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Extension class for OutputStream to marshal ProxyData11 and ProxyData20 without creating a
    /// ProxyData11/20 instance.</summary>
    internal static class ProxyDataExtensions
    {
        internal static void WriteProxyData11(
            this OutputStream ostr,
            string facet,
            InvocationMode invocationMode,
            Protocol protocol,
            Encoding encoding)
        {
            Debug.Assert(ostr.Encoding == Encoding.V11);
            ostr.WriteFacet11(facet);
            ostr.Write(invocationMode);
            ostr.WriteBool(false); // "secure"
            ostr.Write(protocol);
            ostr.WriteByte(0); // protocol minor
            encoding.IceWrite(ostr);
        }

        internal static void WriteProxyData20(
            this OutputStream ostr,
            Identity identity,
            Protocol protocol,
            Encoding encoding,
            IReadOnlyList<string> location,
            InvocationMode invocationMode,
            string facet)
        {
            Debug.Assert(ostr.Encoding == Encoding.V20);

            BitSequence bitSequence = ostr.WriteBitSequence(5);
            identity.IceWrite(ostr);

            if (protocol != Protocol.Ice2)
            {
                ostr.Write(protocol);
            }
            else
            {
                bitSequence[0] = false;
            }

            if (encoding != Encoding.V20)
            {
                encoding.IceWrite(ostr);
            }
            else
            {
                bitSequence[1] = false;
            }

            if (location.Count > 0)
            {
                ostr.WriteSequence(location, OutputStream.IceWriterFromString);
            }
            else
            {
                bitSequence[2] = false;
            }

            // We only write a non-null invocation mode for ice1.
            if (protocol == Protocol.Ice1 && invocationMode != InvocationMode.Twoway)
            {
                ostr.Write(invocationMode);
            }
            else
            {
                bitSequence[3] = false;
            }

            if (facet.Length > 0)
            {
                ostr.WriteString(facet);
            }
            else
            {
                bitSequence[4] = false;
            }
        }
    }
}
