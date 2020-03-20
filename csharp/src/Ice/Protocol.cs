//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Globalization;

namespace Ice
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

    internal static class ProtocolExtensions
    {
        /// <summary>Checks if this protocol is supported by the Ice runtime. If not supported, throws
        /// NotSupportedException.</summary>
        /// <param name="protocol">The protocol.</param>
        internal static void CheckSupported(this Protocol protocol)
        {
            // For now, we support only ice1
            if (protocol != Protocol.Ice1)
            {
                throw new System.NotSupportedException(
                    $"Ice protocol `{protocol.ToString().ToLower()}' is not supported by this Ice runtime");
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
                    // Parse as a Major.Minor encoding:
                    if (Encoding.TryParse(str, out Encoding encoding) && encoding.Minor == 0)
                    {
                        return (Protocol)encoding.Major;
                    }
                    byte value = byte.Parse(str, CultureInfo.InvariantCulture);
                    return (Protocol)value;
            }
        }
    }
}
