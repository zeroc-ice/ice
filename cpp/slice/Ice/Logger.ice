// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_LOGGER_ICE
#define ICE_LOGGER_ICE

module Ice
{

/**
 *
 * The &Ice; message logger. Applications can provide their own logger
 * by implementing this interface and installing it in a communicator.
 *
 **/
local interface Logger
{
    /**
     *
     * Log trace messages.
     *
     * @param category The trace category.
     *
     * @param message The trace message to log.
     *
     **/
    void trace(string category, string message);

    /**
     *
     * Log warning messages.
     *
     * @param message The warning message to log.
     *
     * @see error
     *
     **/
    void warning(string message);

    /**
     *
     * Log error messages.
     *
     * @param message The error message to log.
     *
     * @see warning
     *
     **/
    void error(string message);
};

};

#endif
