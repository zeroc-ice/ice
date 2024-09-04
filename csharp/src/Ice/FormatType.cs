// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// This enumeration describes the possible formats for classes and exceptions.
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
    /// The Sliced format allows the receiver to slice off unknown slices. If an application receives a derived
    /// class it does not know, it can create a base class while preserving the unknown derived slices.
    /// </summary>
    SlicedFormat
}
