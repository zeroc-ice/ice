//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * The Ice message logger. Applications can provide their own logger
 * by implementing this interface and installing it in a communicator.
 **/
public interface Logger
{
    /**
     * Print a message. The message is printed literally, without
     * any decorations such as executable name or time stamp.
     * @param message The message to log.
     **/
    void print(String message);

    /**
     * Log a trace message.
     * @param category The trace category.
     * @param message The trace message to log.
     **/
    void trace(String category, String message);

    /**
     * Log a warning message.
     * @param message The warning message to log.
     *
     * @see #error
     **/
    void warning(String message);

    /**
     * Log an error message.
     * @param message The error message to log.
     *
     * @see #warning
     **/
    void error(String message);

    /**
     * Returns this logger's prefix.
     * @return The prefix.
     **/
    String getPrefix();

    /**
     * Returns a clone of the logger with a new prefix.
     * @param prefix The new prefix for the logger.
     * @return A logger instance.
     **/
    Logger cloneWithPrefix(String prefix);
}
