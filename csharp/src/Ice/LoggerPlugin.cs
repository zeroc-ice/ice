// Copyright (c) ZeroC, Inc.

namespace Ice;

/// <summary>
/// Class to support custom loggers. Applications using a custom logger
/// instantiate a LoggerPlugin with a custom logger and
/// return the instance from their PluginFactory implementation.
/// </summary>
public class LoggerPlugin : Plugin
{
    /// <summary>
    /// Initializes a new instance of the <see cref="LoggerPlugin" /> class. This constructor installs a custom
    /// logger to the given communicator.
    /// </summary>
    /// <param name="communicator">The communicator using the custom logger.</param>
    /// <param name="logger">The custom logger for the communicator.</param>
    public
    LoggerPlugin(Communicator communicator, Logger logger)
    {
        if (communicator == null)
        {
            throw new PluginInitializationException("Communicator cannot be null.");
        }

        if (logger is null)
        {
            throw new PluginInitializationException("Logger cannot be null.");
        }

        Ice.Internal.Instance instance = communicator.instance;
        instance.setLogger(logger);
    }

    /// <summary>
    /// Called by the Ice run time during communicator initialization. The derived class
    /// can override this method to perform any initialization that might be required
    /// by a custom logger.
    /// </summary>
    public void
    initialize()
    {
    }

    /// <summary>
    /// Called by the Ice run time when the communicator is destroyed. The derived class
    /// can override this method to perform any finalization that might be required
    /// by a custom logger.
    /// </summary>
    public void
    destroy()
    {
    }
}
