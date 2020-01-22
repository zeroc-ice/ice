//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace Ice
{
    public readonly struct SlicedData
    {
        /// <summary>
        /// The Ice encoding of the "unknown" slices held by this SlicedData. These slices can only be remarshaled
        /// with the same encoding.
        /// </summary>
        public readonly EncodingVersion Encoding { get; }

        /// <summary>
        /// The "unknown" or unreadable slices from a class or exception instance.
        /// </summary>
        public readonly IReadOnlyList<SliceInfo> Slices { get; }

        /// <summary>
        /// Constructs a new SlicedData.
        /// </summary>
        public SlicedData(EncodingVersion encoding, IReadOnlyList<SliceInfo> slices)
        {
            Encoding = encoding;
            Slices = slices;
        }
    }

    /// <summary>
    /// SliceInfo encapsulates the details of a slice for an unknown class or exception type.
    /// </summary>
    public sealed class SliceInfo
    {
        /// <summary>
        /// The Slice type ID for this slice. Can be null when CompactId is set and unresolved.
        /// </summary>
        public string? TypeId { get; }

        /// <summary>
        /// The Slice compact type ID for this slice.
        /// </summary>
        public int? CompactId { get; }

        /// <summary>
        /// The encoded bytes for this slice, including the leading size integer.
        /// </summary>
        // TODO: switch to IReadOnlyList or some other type once OutputStream supports it.
        public ReadOnlyCollection<byte> Bytes { get; }

        /// <summary>
        /// The class instances referenced by this slice.
        /// </summary>
        public IReadOnlyList<AnyClass> Instances { get; internal set; }

        /// <summary>
        /// Whether or not the slice contains optional members.
        /// </summary>
        public bool HasOptionalMembers { get; }

        /// <summary>
        /// Whether or not this is the last slice of the instance.
        /// </summary>
        public bool IsLastSlice { get; }

        public SliceInfo(string? typeId, int? compactId, ReadOnlyCollection<byte> bytes,
                         IReadOnlyList<AnyClass> instances, bool hasOptionalMembers, bool isLastSlice)
        {
            TypeId = typeId;
            CompactId = compactId;
            Bytes = bytes;
            Instances = instances;
            HasOptionalMembers = hasOptionalMembers;
            IsLastSlice = isLastSlice;
        }
    }
}
