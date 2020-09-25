// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>SlicedData holds the sliced-off unknown slices of a class or remote exception. Each SlicedData value
    /// holds at least one slice.</summary>
    public readonly struct SlicedData : IEquatable<SlicedData>
    {
        /// <summary>The Ice encoding of the "unknown" slices held by this SlicedData. These slices can only be
        /// remarshaled with the same encoding.</summary>
        public readonly Encoding Encoding { get; }

        /// <summary>The "unknown" or unreadable slices from a class or remote exception instance.</summary>
        public readonly IReadOnlyList<SliceInfo> Slices { get; }

        /// <inheritdoc/>
        public bool Equals(SlicedData other) =>
            Encoding == other.Encoding &&
            Slices == other.Slices;

        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is SlicedData value && Equals(value);

        /// <inheritdoc/>
        public override int GetHashCode() =>
            HashCode.Combine(Encoding, Slices);

        /// <summary>The equality operator == returns true if its operands are equal, false otherwise.</summary>
        public static bool operator ==(SlicedData lhs, SlicedData rhs) => lhs.Equals(rhs);

        /// <summary>The inequality operator != returns true if its operands are not equal, false otherwise.</summary>
        public static bool operator !=(SlicedData lhs, SlicedData rhs) => !(lhs == rhs);

        internal SlicedData(Encoding encoding, IReadOnlyList<SliceInfo> slices)
        {
            Debug.Assert(slices.Count >= 1);
            Encoding = encoding;
            Slices = slices;
        }
    }

    /// <summary>SliceInfo encapsulates the details of a slice for an unknown class or remote exception.</summary>
    public sealed class SliceInfo
    {
        /// <summary>The Slice type ID for this slice.</summary>
        public string TypeId { get; }

        /// <summary>The Slice compact type ID for this slice. Only applicable to classes with the 1.1 encoding;
        /// otherwise, always null.</summary>
        public int? CompactId { get; }

        /// <summary>The encoded bytes for this slice, including the leading size integer.</summary>
        public ReadOnlyMemory<byte> Bytes { get; }

        /// <summary>The class instances referenced by this slice.</summary>
        public IReadOnlyList<AnyClass> Instances { get; internal set; }

        /// <summary>Whether or not the slice contains tagged members.</summary>
        public bool HasTaggedMembers { get; }

        internal SliceInfo(
            string typeId,
            int? compactId,
            ReadOnlyMemory<byte> bytes,
            IReadOnlyList<AnyClass> instances,
            bool hasTaggedlMembers)
        {
            TypeId = typeId;
            CompactId = compactId;
            Bytes = bytes;
            Instances = instances;
            HasTaggedMembers = hasTaggedlMembers;
        }
    }
}
