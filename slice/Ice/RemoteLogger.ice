// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:list"]]

[["js:module:@zeroc/ice"]]

[["python:pkgdir:Ice"]]

#include "BuiltinSequences.ice"

[["java:package:com.zeroc"]]

module Ice
{
    /// An enumeration representing the different types of log messages.
    enum LogMessageType
    {
        /// The {@link RemoteLogger} received a print message.
        ["swift:identifier:printMessage"]
        PrintMessage,

        /// The {@link RemoteLogger} received a trace message.
        ["swift:identifier:traceMessage"]
        TraceMessage,

        /// The {@link RemoteLogger} received a warning message.
        ["swift:identifier:warningMessage"]
        WarningMessage,

        /// The {@link RemoteLogger} received an error message.
        ["swift:identifier:errorMessage"]
        ErrorMessage
    }

    /// A sequence of {@link LogMessageType}.
    sequence<LogMessageType> LogMessageTypeSeq;

    /// A complete log message.
    struct LogMessage
    {
        /// The type of message sent to the {@link RemoteLogger}.
        LogMessageType type;

        /// The date and time when the {@link RemoteLogger} received this message, expressed as the number of microseconds
        /// since the Unix Epoch (00:00:00 UTC on 1 January 1970)
        long timestamp;

        /// For a message of type trace, the trace category of this log message; otherwise, the empty string.
        string traceCategory;

        /// The log message itself.
        string message;
    }

    /// A sequence of {@link LogMessage}.
    ["cpp:type:std::list<LogMessage>"]
    sequence<LogMessage> LogMessageSeq;

    /// The Ice remote logger interface. An application can implement a RemoteLogger to receive the log messages sent
    /// to the local {@link RemoteLogger} of another Ice application.
    interface RemoteLogger
    {
        /// init is called by attachRemoteLogger when a RemoteLogger proxy is attached.
        /// @param prefix The prefix of the associated local Logger.
        /// @param logMessages Old log messages generated before "now".
        ["swift:identifier:initialize"]
        void init(string prefix, LogMessageSeq logMessages);

        /// Log a LogMessage. Note that log may be called by LoggerAdmin before init.
        /// @param message The message to log.
        void log(LogMessage message);
    }

    /// Thrown when the provided RemoteLogger was previously attached to a LoggerAdmin.
    exception RemoteLoggerAlreadyAttachedException
    {
    }

    /// The interface of the admin object that allows an Ice application the attach its
    /// {@link RemoteLogger} to the {@link RemoteLogger} of this admin object's Ice communicator.
    ["cs:attribute:System.ComponentModel.EditorBrowsable(System.ComponentModel.EditorBrowsableState.Never)"]
    interface LoggerAdmin
    {
        /// Attaches a RemoteLogger object to the local logger. attachRemoteLogger calls init on the provided
        /// RemoteLogger proxy.
        /// @param prx A proxy to the remote logger.
        /// @param messageTypes The list of message types that the remote logger wishes to receive. An empty list means
        /// no filtering (send all message types).
        /// @param traceCategories The categories of traces that the remote logger wishes to receive. This parameter is
        /// ignored if messageTypes is not empty and does not include trace. An empty list means no filtering (send all
        /// trace categories).
        /// @param messageMax The maximum number of log messages (of all types) to be provided to init. A negative
        /// value requests all messages available.
        /// @throws RemoteLoggerAlreadyAttachedException Raised if this remote logger is already attached to this admin
        /// object.
        void attachRemoteLogger(RemoteLogger* prx, LogMessageTypeSeq messageTypes, StringSeq traceCategories,
                                int messageMax)
            throws RemoteLoggerAlreadyAttachedException;

        /// Detaches a RemoteLogger object from the local logger.
        /// @param prx A proxy to the remote logger.
        /// @return True if the provided remote logger proxy was detached, and false otherwise.
        bool detachRemoteLogger(RemoteLogger* prx);

        /// Retrieves log messages recently logged.
        /// @param messageTypes The list of message types that the caller wishes to receive. An empty list means no
        /// filtering (send all message types).
        /// @param traceCategories The categories of traces that caller wish to receive. This parameter is ignored if
        /// messageTypes is not empty and does not include trace. An empty list means no filtering (send all trace
        /// categories).
        /// @param messageMax The maximum number of log messages (of all types) to be returned. A negative value
        /// requests all messages available.
        /// @param prefix The prefix of the associated local logger.
        /// @return The Log messages.
        LogMessageSeq getLog(
            LogMessageTypeSeq messageTypes,
            StringSeq traceCategories,
            int messageMax,
            out string prefix);
    }
}
