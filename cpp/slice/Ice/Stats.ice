// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_STATS_ICE
#define ICE_STATS_ICE

module Ice
{

/**
 *
 * An interface &Ice; uses to report statistics, such as how much data
 * is sent or received.  Applications must provide their own [Stats]
 * by implementing this interface and installing it with with a
 * communicator.
 *
 **/
local interface Stats
{
    /**
     *
     * Callback to report that data has been sent.
     *
     * @param protocol The protocol over which data has been sent (for
     * example "tcp", "udp", or "ssl").
     *
     * @param num How many bytes have been sent.
     *
     **/
    void bytesSent(string protocol, int num);

    /**
     *
     * Callback to report that data has been received.
     *
     * @param protocol The protocol over which data has been received
     * (for example "tcp", "udp", or "ssl").
     *
     * @param num How many bytes have been received.
     *
     **/
    void bytesReceived(string protocol, int num);
};

};

#endif
