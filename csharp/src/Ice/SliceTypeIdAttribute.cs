// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>Assigns a Slice type ID to a class or interface.</summary>
/// <remarks>The Slice compiler assigns Slice type IDs to classes and interfaces it generates from Slice classes,
/// interfaces, and exceptions.</remarks>
// Corresponds to IceRPC's ZeroC.Slice.SliceTypeIdAttribute.
[AttributeUsage(AttributeTargets.Class | AttributeTargets.Interface, Inherited = false)]
public sealed class SliceTypeIdAttribute : Attribute
{
    /// <summary>Gets the Slice type ID.</summary>
    /// <value>The Slice type ID string.</value>
    public string Value { get; }

    /// <summary>Initializes a new instance of the <see cref="SliceTypeIdAttribute" /> class.</summary>
    /// <param name="value">The Slice type ID.</param>>
    public SliceTypeIdAttribute(string value) => Value = value;
}

/// <summary>Assigns a compact Slice type ID to a class.</summary>
/// <remarks>The Slice compiler assigns both a Slice type ID and a compact Slice type ID to the mapped class of a Slice
/// class that specifies a compact type ID.</remarks>
[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class CompactSliceTypeIdAttribute : Attribute
{
    /// <summary>Gets the compact Slice type ID.</summary>
    /// <value>The compact Slice type ID numeric value.</value>
    public int Value { get; }

    /// <summary>Initializes a new instance of the <see cref="CompactSliceTypeIdAttribute" /> class.</summary>
    /// <param name="value">The compact type ID.</param>>
    public CompactSliceTypeIdAttribute(int value) => Value = value;
}
