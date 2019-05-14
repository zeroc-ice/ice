//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:GLACIER2_API"]]
[["cpp:doxygen:include:Glacier2/Glacier2.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:Glacier2/Config.h"]]

[["ice-prefix"]]

[["js:module:ice"]]

[["objc:dll-export:GLACIER2_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:Glacier2"]]

#include <Ice/Metrics.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICEMX", "swift:module:Glacier2:MX"]
module IceMX
{

/**
 *
 * Provides information on Glacier2 sessions.
 *
 **/
class SessionMetrics extends Metrics
{
    /**
     *
     * Number of client requests forwared.
     *
     **/
    int forwardedClient = 0;

    /**
     *
     * Number of server requests forwared.
     *
     **/
    int forwardedServer = 0;

    /**
     *
     * The size of the routing table.
     *
     **/
    int routingTableSize = 0;

    /**
     *
     * Number of client requests queued.
     *
     **/
    int queuedClient = 0;

    /**
     *
     * Number of server requests queued.
     *
     **/
    int queuedServer = 0;

    /**
     *
     * Number of client requests overridden.
     *
     **/
    int overriddenClient = 0;

    /**
     *
     * Number of server requests overridden.
     *
     **/
    int overriddenServer = 0;
}

}
