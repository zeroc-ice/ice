// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

namespace ZeroC.Ice
{
    /// <summary>Provides public extensions methods for <see cref="Protocol"/>.</summary>
    public static class ProtocolExtensions
    {
        /// <summary>Returns the Ice encoding that this protocol uses for its headers.</summary>
        /// <param name="protocol">The protocol.</param>
        public static Encoding GetEncoding(this Protocol protocol) =>
            protocol switch
            {
                Protocol.Ice1 => Encoding.V11,
                Protocol.Ice2 => Encoding.V20,
                _ => throw new NotSupportedException(
                    @$"Ice protocol `{protocol.GetName()}' is not supported by this Ice runtime ({Runtime.StringVersion
                    })")
            };

        /// <summary>Returns the name of this protocol in lowercase, e.g. "ice1" or "ice2".</summary>
        public static string GetName(this Protocol protocol) =>
            protocol.ToString().ToLowerInvariant();

        /// <summary>Parses a protocol string in the stringified proxy format into a Protocol.</summary>
        /// <param name="str">The string to parse.</param>
        /// <returns>The parsed protocol, or throws an exception if the string cannot be parsed.</returns>
        public static Protocol Parse(string str)
        {
            switch (str)
            {
                case "ice1":
                    return Protocol.Ice1;
                case "ice2":
                    return Protocol.Ice2;
                default:
                    if (str.EndsWith(".0", StringComparison.InvariantCulture))
                    {
                        str = str[0..^2];
                    }
                    if (byte.TryParse(str, out byte value))
                    {
                        return value > 0 ? (Protocol)value : throw new FormatException("invalid protocol 0");
                    }
                    else
                    {
                        throw new FormatException($"invalid protocol `{str}'");
                    }
            }
        }

        /// <summary>Checks if this protocol is supported by the Ice runtime. If not supported, throws
        /// NotSupportedException.</summary>
        /// <param name="protocol">The protocol.</param>
        internal static void CheckSupported(this Protocol protocol)
        {
            if (!protocol.IsSupported())
            {
                throw new NotSupportedException(
                    @$"Ice protocol `{protocol.GetName()}' is not supported by this Ice runtime ({Runtime.StringVersion
                    })");
            }
        }

        internal static ArraySegment<byte> GetEmptyArgsPayload(this Protocol protocol, Encoding encoding) =>
            protocol switch
            {
                Protocol.Ice1 => Ice1Definitions.GetEmptyArgsPayload(encoding),
                Protocol.Ice2 => Ice2Definitions.GetEmptyArgsPayload(encoding),
                _ => throw new NotSupportedException(
                    @$"Ice protocol `{protocol.GetName()}' is not supported by this Ice runtime ({Runtime.StringVersion
                    })"),
            };

        internal static ArraySegment<byte> GetVoidReturnPayload(this Protocol protocol, Encoding encoding) =>
            protocol switch
            {
                Protocol.Ice1 => Ice1Definitions.GetVoidReturnValuePayload(encoding),
                Protocol.Ice2 => Ice2Definitions.GetVoidReturnValuePayload(encoding),
                _ => throw new NotSupportedException(
                    @$"Ice protocol `{protocol.GetName()}' is not supported by this Ice runtime ({Runtime.StringVersion
                    })"),
            };

        internal static bool IsSupported(this Protocol protocol) =>
            protocol == Protocol.Ice1 || protocol == Protocol.Ice2;
    }
}
