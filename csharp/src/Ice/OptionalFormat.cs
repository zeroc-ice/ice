// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// The optional format.
///
/// An optional value is encoded with a specific optional format. This optional
/// format describes how the data is encoded and how it can be skipped by the
/// unmarshaling code if the optional is not known to the receiver.
/// </summary>
public enum OptionalFormat
{
    /// <summary>
    /// A fixed size numeric encoded on 1 byte.
    /// </summary>
    F1 = 0,

    /// <summary>
    /// A fixed size numeric encoded on 2 byte.
    /// </summary>
    F2 = 1,

    /// <summary>
    /// A fixed size numeric encoded on 4 byte.
    /// </summary>
    F4 = 2,

    /// <summary>
    /// A fixed size numeric encoded on 8 byte.
    /// </summary>
    F8 = 3,

    /// <summary>
    /// A variable-length size encoded on 1 or 5 bytes.
    /// </summary>
    Size = 4,

    /// <summary>
    /// A variable-length size followed by size bytes.
    /// </summary>
    VSize = 5,

    /// <summary>
    /// A fixed length size (encoded on 4 bytes) followed by size bytes.
    /// </summary>
    FSize = 6,

    /// <summary>
    /// Represents a class, but is no longer encoded or decoded.
    /// </summary>
    Class = 7
}
