// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Represents Ice's abstraction for logging and tracing. Applications can provide their own logger by implementing this
 * abstraction and setting a logger on the communicator.
 */
public interface Logger extends AutoCloseable {
    /**
     * Prints a message. The message is printed literally, without any decorations such as executable
     * name or time stamp.
     *
     * @param message The message to log.
     */
    void print(String message);

    /**
     * Logs a trace message.
     *
     * @param category The trace category.
     * @param message The trace message to log.
     */
    void trace(String category, String message);

    /**
     * Logs a warning message.
     *
     * @param message The warning message to log.
     * @see #error
     */
    void warning(String message);

    /**
     * Logs an error message.
     *
     * @param message The error message to log.
     * @see #warning
     */
    void error(String message);

    /**
     * Returns this logger's prefix.
     *
     * @return The prefix.
     */
    String getPrefix();

    /**
     * Returns a clone of the logger with a new prefix.
     *
     * @param prefix The new prefix for the logger.
     * @return A logger instance.
     */
    Logger cloneWithPrefix(String prefix);
}
