// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * A special plug-in that installs a logger during a communicator's initialization.
 * Both initialize and destroy are no-op.
 */
public class LoggerPlugin implements Plugin {
    /**
     * Constructs a logger plugin. This constructor installs a custom logger in the communicator. The communicator takes
     * ownership of the logger and is responsible for closing it when the communicator is destroyed.
     *
     * @param communicator the communicator in which to install the logger
     * @param logger the logger to install
     */
    public LoggerPlugin(Communicator communicator, Logger logger) {
        if (communicator == null) {
            throw new PluginInitializationException("Communicator cannot be null.");
        }

        if (logger == null) {
            throw new PluginInitializationException("Logger cannot be null.");
        }

        Instance instance = communicator.getInstance();
        instance.setLogger(logger);
    }

    /** This method is no-op. */
    @Override
    public void initialize() {}

    /** This method is no-op. */
    @Override
    public void destroy() {}
}
