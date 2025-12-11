// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base class for all Ice exceptions.
/// It has only two derived classes: <see cref="LocalException" /> and <see cref="UserException" />.
/// </summary>
public abstract class Exception : System.Exception
{
    /// <summary>
    /// Returns the type ID of this exception. This corresponds to the Slice type ID for Slice-defined exceptions,
    /// and to a similar fully scoped name for other exceptions.
    /// </summary>
    /// <returns>The type ID of this exception.</returns>
    public abstract string ice_id();

    /// <summary>
    /// Initializes a new instance of the <see cref="Exception" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    protected Exception(string? message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }
}
