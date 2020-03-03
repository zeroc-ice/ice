//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace IceInternal
{
    public static class Protocol
    {
        internal static void CheckSupportedProtocol(Ice.ProtocolVersion v)
        {
            if (v.Major != Ice1Definitions.ProtocolMajor || v.Minor > Ice1Definitions.ProtocolMinor)
            {
                throw new Ice.UnsupportedProtocolException("", v, Ice.Util.CurrentProtocol);
            }
        }

        public static void CheckSupportedProtocolEncoding(Ice.Encoding v)
        {
            if (v.Major != Ice1Definitions.ProtocolEncodingMajor || v.Minor > Ice1Definitions.ProtocolEncodingMinor)
            {
                throw new Ice.UnsupportedEncodingException("", v, Ice.Util.CurrentProtocolEncoding);
            }
        }

        //
        // Either return the given protocol if not compatible, or the greatest
        // supported protocol otherwise.
        //
        internal static Ice.ProtocolVersion GetCompatibleProtocol(Ice.ProtocolVersion v)
        {
            if (v.Major != Ice.Util.CurrentProtocol.Major)
            {
                return v; // Unsupported protocol, return as is.
            }
            else if (v.Minor < Ice.Util.CurrentProtocol.Minor)
            {
                return v; // Supported protocol.
            }
            else
            {
                //
                // Unsupported but compatible, use the currently supported
                // protocol, that's the best we can do.
                //
                return Ice.Util.CurrentProtocol;
            }
        }

        internal static bool IsSupported(Ice.ProtocolVersion version, Ice.ProtocolVersion supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;
    }
}
