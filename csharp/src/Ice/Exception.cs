// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base class for all Ice exceptions.
/// </summary>
public abstract class Exception : System.Exception
{
    /// <summary>
    /// Returns the type ID of this exception.
    /// </summary>
    /// <returns>The type ID of this exception.</returns>
    public abstract string ice_id();

    /// <summary>
    /// Constructs an Ice exception.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    protected Exception(string? message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }
}
