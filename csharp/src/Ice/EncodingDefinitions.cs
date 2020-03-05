//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    internal static class EncodingDefinitions
    {
        internal const byte OPTIONAL_END_MARKER = 0xFF;

        // TODO: replace by enum
        internal const byte FLAG_HAS_TYPE_ID_STRING = 1 << 0;
        internal const byte FLAG_HAS_TYPE_ID_INDEX = 1 << 1;
        internal const byte FLAG_HAS_TYPE_ID_COMPACT = (1 << 1) | (1 << 0);
        internal const byte FLAG_HAS_OPTIONAL_MEMBERS = 1 << 2;
        internal const byte FLAG_HAS_INDIRECTION_TABLE = 1 << 3;
        internal const byte FLAG_HAS_SLICE_SIZE = 1 << 4;
        internal const byte FLAG_IS_LAST_SLICE = 1 << 5;

        internal static bool IsSupported(Encoding version, Encoding supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;
    }
}
