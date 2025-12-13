// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base class for all Ice exceptions not defined in Slice.
/// </summary>
public class LocalException : Ice.Exception
{
    /// <summary>
    /// Initializes a new instance of the <see cref="LocalException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public LocalException(string? message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::Ice::LocalException";
}
