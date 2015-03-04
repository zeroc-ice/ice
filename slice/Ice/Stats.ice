// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h"]]

module Ice
{

/**
 *
 * An interface Ice uses to report statistics, such as how much data
 * is sent or received. Applications must provide their own {@link Stats}
 * by implementing this interface and installing it in a communicator.
 *
 * <p class="Deprecated">This local interface is deprecated as of Ice 3.5.
 *
 **/
["deprecate:Stats is deprecated; you should use instead Ice Metrics or Ice Instrumentation"]
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

