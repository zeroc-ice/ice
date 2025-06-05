// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:GLACIER2_API"]]
[["cpp:doxygen:include:Glacier2/Glacier2.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:Glacier2/Config.h"]]

[["js:module:@zeroc/ice"]]
[["python:pkgdir:Glacier2"]]

#include "Ice/Metrics.ice"

["java:identifier:com.zeroc.IceMX"]
["swift:identifier:Glacier2"]
module IceMX
{
    /// Provides information about Glacier2 sessions.
    class SessionMetrics extends Metrics
    {
        /// The number of client requests forwarded.
        int forwardedClient = 0;

        /// The number of server requests forwarded.
        int forwardedServer = 0;

        /// The size of the routing table.
        int routingTableSize = 0;

        /// The number of client requests queued.
        ["deprecated:As of Ice 3.8, the Glacier2 router no longer queues requests."]
        int queuedClient = 0;

        /// The number of server requests queued.
        ["deprecated:As of Ice 3.8, the Glacier2 router no longer queues requests."]
        int queuedServer = 0;

        /// The number of client requests overridden.
        ["deprecated:As of Ice 3.8, the Glacier2 router no longer queues requests."]
        int overriddenClient = 0;

        /// The number of server requests overridden.
        ["deprecated:As of Ice 3.8, the Glacier2 router no longer queues requests."]
        int overriddenServer = 0;
    }
}
