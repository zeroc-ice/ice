// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Holds class slices that cannot be unmarshaled because their types are not known locally.
/// </summary>
/// <param name="slices">The details of each slice, in order of most-derived to least-derived.</param>
public sealed record class SlicedData(SliceInfo[] slices);

/// <summary>
/// Encapsulates the details of a slice with an unknown type.
/// </summary>
/// <param name="typeId">The Slice type ID or compact type ID for this slice.</param>
/// <param name="bytes">The encoded bytes for this slice, including the leading size integer.</param>
/// <param name="hasOptionalMembers">Whether or not the slice contains optional members.</param>
/// <param name="isLastSlice">Whether or not this is the last slice.</param>
public sealed record class SliceInfo(
    string typeId,
    byte[] bytes,
    bool hasOptionalMembers,
    bool isLastSlice)
{
    /// <summary>
    /// Gets or sets the class instances referenced by this slice.
    /// </summary>
    public Value[] instances { get; set; } = [];
}
