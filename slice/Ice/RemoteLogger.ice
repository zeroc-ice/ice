// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:list"]]

[["js:module:@zeroc/ice"]]

#include "BuiltinSequences.ice"

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Represents the different types of log messages.
    enum LogMessageType
    {
        /// A print message.
        ["swift:identifier:printMessage"]
        PrintMessage,

        /// A trace message.
        ["swift:identifier:traceMessage"]
        TraceMessage,

        /// A warning message.
        ["swift:identifier:warningMessage"]
        WarningMessage,

        /// An error message.
        ["swift:identifier:errorMessage"]
        ErrorMessage
    }

    /// A sequence of {@link LogMessageType}.
    sequence<LogMessageType> LogMessageTypeSeq;

    /// Represents a full log message.
    struct LogMessage
    {
        /// The type of this log message.
        LogMessageType type;

        /// The date and time when the application logged this message, expressed as the number of microseconds since
        /// the Unix Epoch (00:00:00 UTC on 1 January 1970).
        long timestamp;

        /// For a message of type 'trace', the trace category of this log message; otherwise, the empty string.
        string traceCategory;

        /// The log message itself.
        string message;
    }

    /// A sequence of {@link LogMessage}.
    ["cpp:type:std::list<LogMessage>"]
    sequence<LogMessage> LogMessageSeq;

    /// Represents an Ice object that receives the log messages of a communicator's logger. {@link LoggerAdmin} calls
    /// this object after {@link LoggerAdmin::attachRemoteLogger} attaches it.
    interface RemoteLogger
    {
        /// Receives the prefix of the local logger and the log messages it logged before this remote logger was
        /// attached. {@link LoggerAdmin} calls this operation once, after {@link LoggerAdmin::attachRemoteLogger}
        /// attaches this remote logger.
        /// @param prefix The prefix of the local logger.
        /// @param logMessages The most recent log messages cached by {@link LoggerAdmin} that match the filters given
        /// to {@link LoggerAdmin::attachRemoteLogger}, oldest first, up to its `messageMax`. This sequence is empty
        /// when `messageMax` is 0.
        ["swift:identifier:initialize"]
        void init(string prefix, LogMessageSeq logMessages);

        /// Receives a log message that matches the filters given to {@link LoggerAdmin::attachRemoteLogger}.
        /// @param message The log message.
        /// @remark {@link LoggerAdmin} can call {@link log} before {@link init}, even though the log messages passed
        /// to {@link init} are older.
        void log(LogMessage message);
    }

    /// Thrown when a {@link RemoteLogger} with the same identity is already attached to a {@link LoggerAdmin}.
    exception RemoteLoggerAlreadyAttachedException
    {
    }

    /// Represents the admin facet that allows an Ice application to attach its {@link RemoteLogger} to the local
    /// logger of an Ice communicator.
    interface LoggerAdmin
    {
        /// Attaches a {@link RemoteLogger} object to the local logger, then calls {@link RemoteLogger::init} on
        /// @p prx. This admin object detaches the remote logger when a call to it fails.
        /// @param prx A proxy to the remote logger.
        /// @param messageTypes The list of message types that the remote logger wishes to receive. An empty list means
        /// no filtering (send all message types).
        /// @param traceCategories The categories of traces that the remote logger wishes to receive. This parameter is
        /// ignored if @p messageTypes is not empty and does not include trace. An empty list means no filtering
        /// (send all trace categories).
        /// @param messageMax The maximum number of cached log messages (of all types) to be provided to
        /// {@link RemoteLogger::init}. A negative value requests all the cached messages, and 0 requests none. This
        /// parameter does not limit the log messages sent to {@link RemoteLogger::log}.
        /// @throws RemoteLoggerAlreadyAttachedException Thrown when a remote logger with the identity of @p prx is
        /// already attached to this admin object.
        void attachRemoteLogger(
            RemoteLogger* prx,
            LogMessageTypeSeq messageTypes,
            StringSeq traceCategories,
            int messageMax)
            throws RemoteLoggerAlreadyAttachedException;

        /// Detaches a {@link RemoteLogger} object from the local logger.
        /// @param prx A proxy to the remote logger. This admin object uses only the identity of this proxy.
        /// @return `true` if a remote logger with this identity was attached and is now detached, and `false`
        /// otherwise.
        bool detachRemoteLogger(RemoteLogger* prx);

        /// Retrieves the most recent log messages cached by this admin object, oldest first. The properties
        /// `Ice.Admin.Logger.KeepLogs` and `Ice.Admin.Logger.KeepTraces` set how many log messages it caches.
        /// @param messageTypes The list of message types that the caller wishes to receive. An empty list means no
        /// filtering (send all message types).
        /// @param traceCategories The categories of traces that the caller wishes to receive. This parameter is
        /// ignored if @p messageTypes is not empty and does not include trace. An empty list means no filtering (send
        /// all trace categories).
        /// @param messageMax The maximum number of log messages (of all types) to be returned. A negative value
        /// requests all the cached messages, and 0 requests none.
        /// @param prefix The prefix of the associated local logger.
        /// @return The log messages.
        LogMessageSeq getLog(
            LogMessageTypeSeq messageTypes,
            StringSeq traceCategories,
            int messageMax,
            out string prefix);
    }
}
