// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICESTORM_API"]]
[["cpp:doxygen:include:IceStorm/IceStorm.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceStorm/Config.h"]]

[["js:module:@zeroc/ice"]]

#include "Ice/Metrics.ice"

["java:identifier:com.zeroc.IceMX"]
["swift:identifier:IceStorm"]
module IceMX
{
    /// Provides information about one or more IceStorm topics.
    class TopicMetrics extends Metrics
    {
        /// The number of events published on the topic(s) by publishers.
        long published = 0;

        /// The number of events forwarded on the topic(s) by IceStorm topic links.
        long forwarded = 0;
    }

    /// Provides information about IceStorm subscribers.
    class SubscriberMetrics extends Metrics
    {
        /// The number of queued events.
        int queued = 0;

        /// The number of outstanding events.
        int outstanding = 0;

        /// The number of forwarded events.
        long delivered = 0;
    }
}
