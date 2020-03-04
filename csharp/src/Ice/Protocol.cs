//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Globalization;

namespace Ice
{
    /// <summary>Represents a version of the Ice protocol. A Protocol can hold a byte value that does not correspond
    /// to any of its enumerators, and such protocol is not supported by this Ice runtime. It is possible to marshal
    /// and unmarshal a proxy that uses an unsupported protocol, but it is not possible to call an operation with such
    /// a proxy.</summary>
    public enum Protocol : byte { Ice1 = 1, Ice2 = 2 }

    internal static class ProtocolExtensions
    {
        /// <summary>Checks if this protocol is supported by the Ice runtime. If not supported, throws
        /// UnsupportedProtocolException.</summary>
        /// <param name="protocol">The protocol.</param>
        internal static void CheckSupported(this Protocol protocol)
        {
            // For now, we support only ice1
            if (protocol != Protocol.Ice1)
            {
                throw new UnsupportedProtocolException("", protocol, Protocol.Ice1);
            }
        }

        /// <summary>Parses a protocol string in the stringified proxy format into a Protocol.</summary>
        /// <param name="str">The string to parse.</param>
        /// <returns>The parsed protocol, or throws an exception if the string cannot be parsed.</returns>
        internal static Protocol Parse(string str)
        {
            switch (str)
            {
                case "ice1":
                    return Protocol.Ice1;
                case "ice2":
                    return Protocol.Ice2;
                default:
                    (byte Major, byte Minor)? result = Util.ParseMajorMinorVersion(str, throwOnFailure: false);
                    if (result != null && result.Value.Minor == 0)
                    {
                        return (Protocol)result.Value.Major;
                    }
                    byte value = byte.Parse(str, CultureInfo.InvariantCulture);
                    return (Protocol)value;
            }
        }
    }
}
