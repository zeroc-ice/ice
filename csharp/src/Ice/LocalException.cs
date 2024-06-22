// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base class for Ice run-time exceptions.
/// </summary>
public class LocalException : Exception
{
    /// <summary>
    /// Constructs a LocalException.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public LocalException(string? message = null, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <summary>
    /// Constructs a LocalException.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    public LocalException(System.Exception innerException)
        : this(message: null, innerException)
    {
    }

    public override string ice_id() => "::Ice::LocalException";
}
