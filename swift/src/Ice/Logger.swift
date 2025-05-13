// Copyright (c) ZeroC, Inc.

import Foundation

/// The Ice message logger. Applications can provide their own logger by implementing this interface and installing it
/// in a communicator.
public protocol Logger: AnyObject {
    /// Print a message. The message is printed literally, without any decorations such as executable name or time
    /// stamp.
    ///
    /// - parameter message: `String` The message to log.
    func print(_ message: String)

    /// Log a trace message.
    ///
    /// - parameter category: `String` The trace category.
    ///
    /// - parameter message: `String` The trace message to log.
    func trace(category: String, message: String)

    /// Log a warning message.
    ///
    /// - parameter message: `String` The warning message to log.
    func warning(_ message: String)

    /// Log an error message.
    ///
    /// - parameter message: `String` The error message to log.
    func error(_ message: String)

    /// Returns this logger's prefix.
    ///
    /// - returns: `String` - The prefix.
    func getPrefix() -> String

    /// Returns a clone of the logger with a new prefix.
    ///
    /// - parameter prefix: `String` The new prefix for the logger.
    ///
    /// - returns: `Logger` - A logger instance.
    func cloneWithPrefix(_ prefix: String) -> Logger
}
