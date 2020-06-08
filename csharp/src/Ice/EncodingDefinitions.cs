//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    internal static class EncodingDefinitions
    {
        internal const long VarLongMinValue = -2_305_843_009_213_693_952; // -2^61
        internal const long VarLongMaxValue = 2_305_843_009_213_693_951; // 2^61 - 1
        internal const ulong VarULongMinValue = 0;
        internal const ulong VarULongMaxValue = 4_611_686_018_427_387_903; // 2^62 - 1

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
            Class = 7,

            /// <summary>VInt is a special value that is never marshaled and that means "one of F1, F2, F4 or F8".
            /// </summary>
            VInt = 8
        }
    }
}
