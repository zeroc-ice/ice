// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Unknown sliced value holds an instance of an unknown Slice class type.
/// </summary>
public sealed class UnknownSlicedValue : Value
{
    /// <summary>
    /// Represents an instance of a Slice class type having the given Slice type.
    /// </summary>
    /// <param name="unknownTypeId">The Slice type ID of the unknown object.</param>
    public UnknownSlicedValue(string unknownTypeId)
    {
        _unknownTypeId = unknownTypeId;
    }

    /// <summary>
    /// Returns the Slice type ID associated with this object.
    /// </summary>
    /// <returns>The type ID.</returns>
    public override string ice_id()
    {
        return _unknownTypeId;
    }

    private string _unknownTypeId;
}
