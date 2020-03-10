//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    internal static class EncodingDefinitions
    {
        internal const byte TaggedEndMarker = 0xFF;

        [Flags]
        internal enum SliceFlags : byte
        {
            HasTypeIdString = 1,
            HasTypeIdIndex = 2,
            HasTypeIdCompact = HasTypeIdString | HasTypeIdIndex,
            HasTaggedMembers = 4,
            HasIndirectionTable = 8,
            HasSliceSize = 16,
            IsLastSlice = 32
        }
    }
}
