// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// A special plug-in that installs a logger during a communicator's initialization.
/// Both initialize and destroy are no-op.
/// </summary>
public class LoggerPlugin : Plugin
{
    /// <summary>
    /// Initializes a new instance of the <see cref="LoggerPlugin" /> class. This constructor installs a custom logger
    /// in the provided <paramref name="communicator" />. The communicator takes ownership of <paramref name="logger" />
    /// and is responsible for disposing it when the communicator is destroyed.
    /// </summary>
    /// <param name="communicator">The communicator in which to install the logger.</param>
    /// <param name="logger">The logger to install.</param>
    public
    LoggerPlugin(Communicator communicator, Logger logger) => communicator.instance.setLogger(logger);

    /// <summary>
    /// This method is no-op.
    /// </summary>
    public void initialize()
    {
    }

    /// <summary>
    /// This method is no-op.
    /// </summary>
    public void destroy()
    {
    }
}
