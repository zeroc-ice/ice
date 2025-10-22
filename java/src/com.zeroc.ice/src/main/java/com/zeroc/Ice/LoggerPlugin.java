// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * A special plug-in that installs a logger during a communicator's initialization.
 *
 * @see PluginFactory
 * @see InitializationData
 */
public class LoggerPlugin implements Plugin {
    /**
     * Constructs and installs a custom logger in the provided communicator.
     *
     * @param communicator the communicator in which to install the logger
     * @param logger the custom logger to be installed
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
