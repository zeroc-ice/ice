// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Represents Ice's abstraction for logging and tracing.
 * Applications can provide their own logger by implementing this interface and setting a logger on the communicator.
 *
 * <p>The Ice runtime calls {@link #print}, {@link #trace}, {@link #warning} and {@link #error} while performing
 * internal operations, including from its last-resort exception handlers. Implementations of these methods must not
 * throw exceptions: a thrown exception can leave the Ice runtime in an inconsistent state or terminate an Ice
 * runtime thread.
 */
public interface Logger extends AutoCloseable {
    /**
     * Prints a message.
     * The message is printed literally, without any decorations such as executable name or timestamp.
     *
     * <p>An implementation of this method must not throw exceptions.
     *
     * @param message the message to log
     */
    void print(String message);

    /**
     * Logs a trace message.
     *
     * <p>An implementation of this method must not throw exceptions.
     *
     * @param category the trace category
     * @param message the trace message to log
     */
    void trace(String category, String message);

    /**
     * Logs a warning message.
     *
     * <p>An implementation of this method must not throw exceptions.
     *
     * @param message the warning message to log
     * @see #error
     */
    void warning(String message);

    /**
     * Logs an error message.
     *
     * <p>An implementation of this method must not throw exceptions.
     *
     * @param message the error message to log
     * @see #warning
     */
    void error(String message);

    /**
     * Returns this logger's prefix.
     *
     * @return the prefix
     */
    String getPrefix();

    /**
     * Returns a clone of the logger with a new prefix.
     *
     * @param prefix the new prefix for the logger
     * @return a new logger instance with the specified prefix
     */
    Logger cloneWithPrefix(String prefix);
}
