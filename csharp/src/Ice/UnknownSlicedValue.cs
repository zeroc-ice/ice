// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents an instance of an unknown class.
/// </summary>
public sealed class UnknownSlicedValue : Value
{
    /// <summary>
    /// Initializes a new instance of the <see cref="UnknownSlicedValue" /> class.
    /// </summary>
    /// <param name="unknownTypeId">The Slice type ID of the unknown value, or the string form of the compact type
    /// ID (for example, "1") when the most-derived slice was marshaled with a compact type ID.</param>
    public UnknownSlicedValue(string unknownTypeId) => _unknownTypeId = unknownTypeId;

    /// <summary>
    /// Returns the Slice type ID associated with this instance.
    /// </summary>
    /// <returns>The type ID supplied to the constructor. It's the string form of the compact type ID (for example,
    /// "1") when the most-derived slice was marshaled with a compact type ID.</returns>
    public override string ice_id() => _unknownTypeId;

    private readonly string _unknownTypeId;
}
