//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    internal static class Protocol
    {
        internal static void CheckSupportedProtocol(ProtocolVersion v)
        {
            if (v.Major != Ice1Definitions.ProtocolMajor || v.Minor > Ice1Definitions.ProtocolMinor)
            {
                throw new UnsupportedProtocolException("", v, Util.CurrentProtocol);
            }
        }

        internal static void CheckSupportedProtocolEncoding(Encoding v)
        {
            if (v.Major != Ice1Definitions.ProtocolEncodingMajor || v.Minor > Ice1Definitions.ProtocolEncodingMinor)
            {
                throw new UnsupportedEncodingException("", v, Util.CurrentProtocolEncoding);
            }
        }

        //
        // Either return the given protocol if not compatible, or the greatest
        // supported protocol otherwise.
        //
        internal static ProtocolVersion GetCompatibleProtocol(ProtocolVersion v)
        {
            if (v.Major != Util.CurrentProtocol.Major)
            {
                return v; // Unsupported protocol, return as is.
            }
            else if (v.Minor < Util.CurrentProtocol.Minor)
            {
                return v; // Supported protocol.
            }
            else
            {
                //
                // Unsupported but compatible, use the currently supported
                // protocol, that's the best we can do.
                //
                return Util.CurrentProtocol;
            }
        }

        internal static bool IsSupported(ProtocolVersion version, ProtocolVersion supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;
    }
}
