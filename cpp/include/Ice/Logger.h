// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOGGER_H
#define ICE_LOGGER_H

#include "Config.h"

#include <memory>
#include <string>

namespace Ice
{
    class Logger;

    /// A shared pointer to a Logger.
    using LoggerPtr = std::shared_ptr<Logger>;

    /// Represents Ice's abstraction for logging and tracing. Applications can provide their own logger by
    /// implementing this abstraction and setting a logger on the communicator.
    /// @remark The Ice runtime calls #print, #trace, #warning and #error from contexts where exceptions cannot be
    /// handled, such as destructors and `noexcept` functions. Implementations of these functions must not throw
    /// exceptions: a thrown exception can terminate the process.
    /// @see InitializationData
    /// @headerfile Ice/Ice.h
    class ICE_API Logger
    {
    public:
        virtual ~Logger();

        // We use const std::string& and not std::string_view for the log messages because implementations commonly
        // send the message to C APIs that require null-terminated strings.
        // The message itself is also often constructed from a string produced by an ostringstream.

        /// Prints a message.
        /// The message is printed literally, without any decorations such as executable name or timestamp.
        /// @param message The message to log.
        /// @remark An implementation of this function must not throw exceptions.
        virtual void print(const std::string& message) = 0;

        /// Logs a trace message.
        /// @param category The trace category.
        /// @param message The trace message to log.
        /// @remark An implementation of this function must not throw exceptions.
        virtual void trace(const std::string& category, const std::string& message) = 0;

        /// Logs a warning message.
        /// @param message The warning message to log.
        /// @remark An implementation of this function must not throw exceptions.
        /// @see #error
        virtual void warning(const std::string& message) = 0;

        /// Logs an error message.
        /// @param message The error message to log.
        /// @remark An implementation of this function must not throw exceptions.
        /// @see #warning
        virtual void error(const std::string& message) = 0;

        /// Returns this logger's prefix.
        /// @return The prefix.
        virtual std::string getPrefix() = 0;

        /// Returns a clone of the logger with a new prefix.
        /// @param prefix The new prefix for the logger.
        /// @return A new logger instance with the specified prefix.
        virtual LoggerPtr cloneWithPrefix(std::string prefix) = 0;
    };
}

#endif
