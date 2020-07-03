//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Globalization;

namespace ZeroC.Ice
{
    /// <summary>Represents a version of the Ice protocol. A Protocol can hold a byte value that does not correspond
    /// to any of its enumerators, and such protocol is not supported by this Ice runtime. It is possible to marshal
    /// and unmarshal a proxy that uses an unsupported protocol, but it is not possible to call an operation with such
    /// a proxy.</summary>
    public enum Protocol : byte
    {
        /// <summary>The ice1 protocol supported by all Ice versions since Ice 1.0.</summary>
        Ice1 = 1,
        /// <summary>The ice2 protocol introduced in Ice 4.0.</summary>
        Ice2 = 2
    }

    public static class ProtocolExtensions
    {
        /// <summary>Returns the Ice encoding that this protocol uses for its headers.</summary>
        /// <param name="protocol">The protocol.</param>
        public static Encoding GetEncoding(this Protocol protocol) =>
            protocol switch
            {
                Protocol.Ice1 => Encoding.V1_1,
                Protocol.Ice2 => Encoding.V2_0,
                _ => throw new NotSupportedException(@$"Ice protocol `{protocol.GetName()
                    }' is not supported by this Ice runtime ({Runtime.StringVersion})")
            };

        /// <summary>Returns the name of this protocol in lowercase, e.g. "ice1" or "ice2".</summary>
        public static string GetName(this Protocol protocol) =>
            protocol.ToString().ToLowerInvariant();

        /// <summary>Checks if this protocol is supported by the Ice runtime. If not supported, throws
        /// NotSupportedException.</summary>
        /// <param name="protocol">The protocol.</param>
        internal static void CheckSupported(this Protocol protocol)
        {
            if (protocol != Protocol.Ice1 && protocol != Protocol.Ice2)
            {
                throw new NotSupportedException(@$"Ice protocol `{protocol.GetName()
                    }' is not supported by this Ice runtime ({Runtime.StringVersion})");
            }
        }

        /// <summary>Parses a protocol string in the stringified proxy format into a Protocol.</summary>
        /// <param name="str">The string to parse.</param>
        /// <returns>The parsed protocol, or throws an exception if the string cannot be parsed.</returns>
        internal static Protocol Parse(string str)
        {
            return str switch
            {
                "ice1" => Protocol.Ice1,
                "ice2" => Protocol.Ice2,
                // Parse as a Major.Minor encoding:
                _ => Encoding.TryParse(str, out Encoding encoding) && encoding.Minor == 0 ?
                    (Protocol)encoding.Major : (Protocol)byte.Parse(str, CultureInfo.InvariantCulture)
            };
        }
    }
}
