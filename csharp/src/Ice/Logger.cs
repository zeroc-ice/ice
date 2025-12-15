// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents Ice's abstraction for logging and tracing. Applications can provide their own logger by
/// implementing this interface and setting a logger on the communicator.
/// </summary>
public interface Logger : IDisposable
{
    /// <summary>
    /// Prints a message. The message is printed literally, without any decorations such as executable name or
    /// timestamp.
    /// </summary>
    /// <param name="message">The message to log.</param>
    void print(string message);

    /// <summary>
    /// Logs a trace message.
    /// </summary>
    /// <param name="category">The trace category.</param>
    /// <param name="message">The trace message to log.</param>
    void trace(string category, string message);

    /// <summary>
    /// Logs a warning message.
    /// </summary>
    /// <param name="message">The warning message to log.</param>
    void warning(string message);

    /// <summary>
    /// Logs an error message.
    /// </summary>
    /// <param name="message">The error message to log.</param>
    void error(string message);

    /// <summary>
    /// Returns this logger's prefix.
    /// </summary>
    /// <returns>The prefix.</returns>
    string getPrefix();

    /// <summary>
    /// Returns a clone of the logger with a new prefix.
    /// </summary>
    /// <param name="prefix">The new prefix for the logger.</param>
    /// <returns>A logger instance.</returns>
    /// <remarks>The caller is responsible for disposing the returned logger.</remarks>
    Logger cloneWithPrefix(string prefix);
}
