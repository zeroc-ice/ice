//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    internal static class EncodingDefinitions
    {
        internal const byte EncodingMajor = 1;
        internal const byte EncodingMinor = 1;

        internal const byte OPTIONAL_END_MARKER = 0xFF;

        // TODO: replace by enum
        internal const byte FLAG_HAS_TYPE_ID_STRING = 1 << 0;
        internal const byte FLAG_HAS_TYPE_ID_INDEX = 1 << 1;
        internal const byte FLAG_HAS_TYPE_ID_COMPACT = (1 << 1) | (1 << 0);
        internal const byte FLAG_HAS_OPTIONAL_MEMBERS = 1 << 2;
        internal const byte FLAG_HAS_INDIRECTION_TABLE = 1 << 3;
        internal const byte FLAG_HAS_SLICE_SIZE = 1 << 4;
        internal const byte FLAG_IS_LAST_SLICE = 1 << 5;

        internal static void CheckSupportedEncoding(Encoding v)
        {
            if (v.Major != EncodingMajor || v.Minor > EncodingMinor)
            {
                throw new UnsupportedEncodingException("", v, Util.CurrentEncoding);
            }
        }

        //
        // Either return the given protocol if not compatible, or the greatest
        // supported protocol otherwise.
        //
        internal static Protocol GetCompatibleProtocol(Protocol v)
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

        //
        // Either return the given encoding if not compatible, or the greatest
        // supported encoding otherwise.
        //
        internal static Encoding GetCompatibleEncoding(Encoding v)
        {
            if (v.Major != Util.CurrentEncoding.Major)
            {
                return v; // Unsupported encoding, return as is.
            }
            else if (v.Minor < Util.CurrentEncoding.Minor)
            {
                return v; // Supported encoding.
            }
            else
            {
                //
                // Unsupported but compatible, use the currently supported
                // encoding, that's the best we can do.
                //
                return Util.CurrentEncoding;
            }
        }

        internal static bool IsSupported(Encoding version, Encoding supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;
    }
}
