// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Specifies the format for marshaling classes and exceptions with the Slice 1.1 encoding.
/// </summary>
public enum FormatType
{
    /// <summary>
    /// The Compact format assumes the sender and receiver have the same Slice definitions for classes. If an
    /// application receives a derived class it does not know, it is not capable of decoding it into a known
    /// base class because there is not enough information in the encoded payload. The compact format is more
    /// space-efficient on the wire.
    /// </summary>
    CompactFormat,

    /// <summary>
    /// Allows slicing-off unknown slices during unmarshaling, at the cost of some extra space in the marshaled data.
    /// If an application receives a derived class it does not know, it can create a base class while preserving the
    /// unknown derived slices.
    /// </summary>
    SlicedFormat
}
