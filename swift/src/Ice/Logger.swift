// Copyright (c) ZeroC, Inc.

import Foundation

/// Represents Ice's abstraction for logging and tracing.
/// Applications can provide their own logger by implementing this interface and setting a logger on the communicator.
public protocol Logger: AnyObject {
    /// Prints a message. The message is printed literally, without any decorations such as executable name or time
    /// stamp.
    ///
    /// - Parameter message: The message to log.
    func print(_ message: String)

    /// Logs a trace message.
    ///
    /// - Parameters:
    ///   - category: The trace category.
    ///   - message: The trace message to log.
    func trace(category: String, message: String)

    /// Logs a warning message.
    ///
    /// - Parameter message: The warning message to log.
    func warning(_ message: String)

    /// Logs an error message.
    ///
    /// - Parameter message: The error message to log.
    func error(_ message: String)

    /// Returns this logger's prefix.
    ///
    /// - Returns: The prefix.
    func getPrefix() -> String

    /// Returns a clone of the logger with a new prefix.
    ///
    /// - Parameter prefix: The new prefix for the logger.
    /// - Returns: A logger instance.
    func cloneWithPrefix(_ prefix: String) -> Logger
}
