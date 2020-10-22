//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export(ICESTORM_API)]]
[[cpp:doxygen:include(IceStorm/IceStorm.h)]]
[[cpp:header-ext(h)]]
[[cpp:include(IceStorm/Config.h)]]

[[suppress-warning(reserved-identifier)]]
[[python:pkgdir(IceStorm)]]

#include <Ice/Metrics.ice>

[[java:package(com.zeroc)]]

[js:module(ice)]
[swift:module(IceStorm:MX)]
[cs:namespace(ZeroC)]
module IceMX
{
    /// Provides information on IceStorm topics.
    class TopicMetrics : Metrics
    {
        /// Number of events published on the topic by publishers.
        long published = 0;

        /// Number of events forwarded on the topic by IceStorm topic links.
        long forwarded = 0;
    }

    /// Provides information on IceStorm subscribers.
    class SubscriberMetrics : Metrics
    {
        /// Number of queued events.
        int queued = 0;

        /// Number of outstanding events.
        int outstanding = 0;

        /// Number of forwarded events.
        long delivered = 0;
    }
}
