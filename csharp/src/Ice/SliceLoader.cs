// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>Creates class and exception instances from Slice type IDs.</summary>
public interface SliceLoader
{
    /// <summary>Creates an instance of a class mapped from a Slice class or exception based on a Slice type ID.
    /// </summary>
    /// <param name="typeId">The Slice type ID or compact type ID.</param>
    /// <returns>A new instance of the class or exception identified by <paramref name="typeId"/>, or <c>null</c> if the
    /// implementation cannot find the corresponding class.</returns>
    /// <exception cref="MarshalException">Thrown when the corresponding class was found but its instantiation failed.
    /// </exception>
    object? createInstance(string typeId);
}
