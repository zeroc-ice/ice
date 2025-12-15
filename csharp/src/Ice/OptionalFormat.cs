// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// The optional format, used for marshaling optional fields and arguments.
/// It describes how the data is marshaled and how it can be skipped by the unmarshaling code
/// if the optional isn't known to the receiver.
/// </summary>
public enum OptionalFormat
{
    /// <summary>
    /// Fixed 1-byte encoding.
    /// </summary>
    F1 = 0,

    /// <summary>
    /// Fixed 2-byte encoding.
    /// </summary>
    F2 = 1,

    /// <summary>
    /// Fixed 4-byte encoding.
    /// </summary>
    F4 = 2,

    /// <summary>
    /// Fixed 8-byte encoding.
    /// </summary>
    F8 = 3,

    /// <summary>
    /// "Size encoding" using either 1 or 5 bytes. Used by enums, class identifiers, etc.
    /// </summary>
    Size = 4,

    /// <summary>
    /// Variable "size encoding" using either 1 or 5 bytes followed by data.
    /// Used by strings, fixed size structs, and containers whose size can be computed prior to marshaling.
    /// </summary>
    VSize = 5,

    /// <summary>
    /// Fixed "size encoding" using 4 bytes followed by data.
    /// Used by variable-size structs, and containers whose sizes can't be computed prior to unmarshaling.
    /// </summary>
    FSize = 6,

    /// <summary>
    /// Class instance. No longer supported.
    /// </summary>
    Class = 7
}
