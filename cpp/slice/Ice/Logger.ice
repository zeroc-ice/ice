// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_LOGGER_ICE
#define ICE_LOGGER_ICE

module Ice
{

/**
 *
 * The &Ice; message logger. Applications can provide their own Logger
 * by implementing this interface and installing it with with a
 * Communicator.
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
