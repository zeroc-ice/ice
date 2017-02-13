// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h", "cpp:dll-export:GLACIER2_API", "objc:header-dir:objc", "objc:dll-export:GLACIER2_API", "js:ice-build"]]
[["cpp:include:Glacier2/Config.h"]]

#include <Ice/Metrics.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICEMX"]
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
};

};
