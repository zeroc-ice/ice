// Copyright (c) ZeroC, Inc.

#nullable enable

namespace IceBox;

/// <summary>
/// This exception is a general failure notification.
/// It is thrown for errors such as a service encountering an error during initialization, or the service manager
/// being unable to load a service executable.
/// </summary>
public sealed class FailureException : Ice.LocalException
{
    public string reason => Message;

    public FailureException(string? message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::IceBox::FailureException";
}

public interface Service
{
    /// <summary>
    /// Start the service.
    /// The given communicator is created by the ServiceManager for use by the service. This
    ///  communicator may also be used by other services, depending on the service configuration.
    ///  &lt;p class="Note"&gt;The ServiceManager owns this communicator, and is responsible for destroying it.
    /// </summary>
    ///  <param name="name">The service's name, as determined by the configuration.
    ///  </param>
    /// <param name="communicator">A communicator for use by the service.
    ///  </param>
    /// <param name="args">The service arguments that were not converted into properties.
    ///  </param>
    /// <exception name="FailureException">Raised if start failed.</exception>
    void start(string name, Ice.Communicator communicator, string[] args);

    /// <summary>
    /// Stop the service.
    /// </summary>
    void stop();
}
