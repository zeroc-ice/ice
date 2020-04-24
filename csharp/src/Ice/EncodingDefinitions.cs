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

        /// <summary>Each tagged parameter has a specific tag format. This tag format describes how the data is encoded
        /// and how it can be skipped by the unmarshaling code if the tagged parameter is present in the input stream
        /// but is not known to the receiver.</summary>
        internal enum TagFormat
        {
            F1 = 0,
            F2 = 1,
            F4 = 2,
            F8 = 3,
            Size = 4,
            VSize = 5,
            FSize = 6,
            Class = 7
        }
    }
}
