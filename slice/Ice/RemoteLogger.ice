// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]
[["cpp:include:list"]]

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * An enumeration representing the different types of log messages.
 *
 **/
enum LogMessageType
{
    /**
     *
     * The {@link Logger} received a print message.
     *
     **/
    PrintMessage,

    /**
     *
     * The {@link Logger} received a trace message.
     *
     **/
    TraceMessage,

    /**
     *
     * The {@link Logger} received a warning message.
     *
     **/
    WarningMessage,

    /**
     *
     * The {@link Logger} received an error message.
     *
     **/
    ErrorMessage
};


/**
 *
 * A sequence of {@link LogMessageType}
 *
 **/
sequence<LogMessageType> LogMessageTypeSeq;


/**
 *
 * A complete log message.
 *
 **/
struct LogMessage
{
    /**
     *
     * The type of message sent to the {@link Logger}.
     *
     **/
    LogMessageType type;

    /**
     *
     * The date and time when the {@link Logger} received this message, expressed
     * as the number of microseconds since the Unix Epoch (00:00:00 UTC on 1 January 1970)
     *
     **/
    long timestamp;

    /**
     *
     * For a message of type trace, the trace category of this log message;
     * otherwise, the empty string.
     *
     **/
    string traceCategory;

     /**
     *
     * The log message itself.
     *
     **/
    string message;
};


/**
 *
 * A sequence of {@link LogMessage}.
 *
 **/
["cpp:type:std::list<LogMessage>"]
sequence<LogMessage> LogMessageSeq;


/**
 *
 * The Ice remote logger interface. An application can implement a
 * RemoteLogger to receive the log messages sent to the local {@link Logger}
 * of another Ice application.
 *
 **/
interface RemoteLogger
{
    /**
     *
     * init is called by {@link LoggerAdmin#attachRemoteLogger} when a
     * RemoteLogger proxy is attached.
     *
     * @param prefix The prefix of the associated local {@link Logger}
     *
     * @param logMessages Old log messages generated before "now".
     *
     * @see LoggerAdmin#attachRemoteLogger
     *
     *
     **/
    void init(string prefix, LogMessageSeq logMessages);


    /**
     *
     * Log a {@link LogMessage}. Note that log may be called by {@link LoggerAdmin}
     * before {@link #init}.
     *
     * @param message The message to log.
     *
     * @see Logger
     *
     **/
    void log(LogMessage message);
};


/**
 *
 * An exception thrown by {@link LoggerAdmin#attachRemoteLogger} to report
 * that the provided {@link RemoteLogger} was previously attached to this
 * {@link LoggerAdmin}.
 *
 **/
exception RemoteLoggerAlreadyAttachedException
{
};

/**
 *
 * The interface of the admin object that allows an Ice application the attach its
 * {@link RemoteLogger} to the {@link Logger} of this admin object's Ice communicator.
 *
 **/
interface LoggerAdmin
{
    /**
     *
     * attachRemoteLogger is called to attach a {@link RemoteLogger} object to
     * the local {@link Logger}.
     * attachRemoteLogger calls init on the provided {@link RemoteLogger} proxy.
     *
     * @param prx A proxy to the remote logger.
     *
     * @param messageTypes The list of message types that the remote logger wishes to receive.
     * An empty list means no filtering (send all message types).
     *
     * @param traceCategories The categories of traces that the remote logger wishes to receive.
     * This parameter is ignored if messageTypes is not empty and does not include trace.
     * An empty list means no filtering (send all trace categories).
     *
     * @param messageMax The maximum number of log messages (of all types) to be provided
     * to {@link RemoteLogger#init}. A negative value requests all messages available.
     *
     * @throws RemoteLoggerAlreadyAttachedException Raised if this remote logger is already
     * attached to this admin object.
     *
     **/
    void attachRemoteLogger(RemoteLogger* prx, LogMessageTypeSeq messageTypes, StringSeq traceCategories,
                            int messageMax)
        throws RemoteLoggerAlreadyAttachedException;

    /**
     *
     * detachRemoteLogger is called to detach a {@link RemoteLogger} object from
     * the local {@link Logger}.
     *
     * @param prx A proxy to the remote logger.
     *
     * @return True if the provided remote logger proxy was detached, and false otherwise.
     *
     **/
    bool detachRemoteLogger(RemoteLogger* prx);

    /**
     *
     * getLog retrieves log messages recently logged.
     *
     * @param messageTypes The list of message types that the caller wishes to receive.
     * An empty list means no filtering (send all message types).
     *
     * @param traceCategories The categories of traces that caller wish to receive.
     * This parameter is ignored if messageTypes is not empty and does not include trace.
     * An empty list means no filtering (send all trace categories).
     *
     * @param messageMax The maximum number of log messages (of all types) to be returned.
     * A negative value requests all messages available.
     *
     * @param prefix The prefix of the associated local {@link Logger}.
     *
     * @return The Log messages.
     *
     **/
    LogMessageSeq getLog(LogMessageTypeSeq messageTypes, StringSeq traceCategories, int messageMax, out string prefix);
};

};

