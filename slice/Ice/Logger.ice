// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

["objc:prefix:ICE"]
module Ice
{

/**
 *
 * The Ice message logger. Applications can provide their own logger
 * by implementing this interface and installing it in a communicator.
 *
 **/
local interface Logger
{
    /**
     *
     * Print a message. The message is printed literally, without
     * any decorations such as executable name or time stamp.
     *
     **/
    void print(string message);

    /**
     *
     * Log a trace message.
     *
     * @param category The trace category.
     *
     * @param message The trace message to log.
     *
     **/
    void trace(string category, string message);

    /**
     *
     * Log a warning message.
     *
     * @param message The warning message to log.
     *
     * @see #error
     *
     **/
    void warning(string message);

    /**
     *
     * Log an error message.
     *
     * @param message The error message to log.
     *
     * @see #warning
     *
     **/
    void error(string message);
    
    /**
     *
     * Returns this logger's prefix.
     *
     * @return The prefix.
     *
     **/
    string getPrefix();

    /**
     *
     * Returns a clone of the logger with a new prefix.
     *
     * @param prefix The new prefix for the logger.
     *
     */
    Logger cloneWithPrefix(string prefix);
};

};

