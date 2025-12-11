// Copyright (c) ZeroC, Inc.

#nullable enable

namespace IceBox;

/// <summary>
/// The exception that is thrown when an IceBox service fails to start.
/// </summary>
/// <remarks>You can throw any exception from your implementation of <see cref="Service.start"/>. This exception is
/// provided for backward compatibility with earlier versions of IceBox.</remarks>
public sealed class FailureException : Ice.LocalException
{
    public string reason => Message;

    public FailureException(string? message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::IceBox::FailureException";
}

/// <summary>
/// Represents an IceBox service that you implement and that the IceBox service manager starts and later stops.
/// The same service can be started and stopped multiple times.
/// </summary>
public interface Service
{
    /// <summary>
    /// Starts the service.
    /// </summary>
    /// <param name="name">The service's name, as specified in configuration.</param>
    /// <param name="communicator">A communicator for use by the service. The IceBox service manager creates this
    /// communicator when it starts, and destroys this communicator when it shuts down.</param>
    /// <param name="args">The service arguments that were not converted into properties of <paramref name="communicator"/>.</param>
    void start(string name, Ice.Communicator communicator, string[] args);

    /// <summary>
    /// Stops the service.
    /// </summary>
    void stop();
}
