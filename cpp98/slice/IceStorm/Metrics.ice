//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICESTORM_API"]]
[["cpp:doxygen:include:IceStorm/IceStorm.h"]]
[["cpp:header-ext:h"]]
[["cpp:include:IceStorm/Config.h"]]

[["ice-prefix"]]

[["objc:dll-export:ICESTORM_API"]]
[["objc:header-dir:objc"]]

[["python:pkgdir:IceStorm"]]

#include <Ice/Metrics.ice>

#ifndef __SLICE2JAVA_COMPAT__
[["java:package:com.zeroc"]]
#endif

["objc:prefix:ICEMX", "js:module:ice", "swift:module:IceStorm:MX"]
module IceMX
{

/**
 *
 * Provides information on IceStorm topics.
 *
 **/
class TopicMetrics extends Metrics
{
    /**
     *
     * Number of events published on the topic by publishers.
     *
     **/
    long published = 0;

    /**
     *
     * Number of events forwarded on the topic by IceStorm topic links.
     *
     **/
    long forwarded = 0;
}

/**
 *
 * Provides information on IceStorm subscribers.
 *
 **/
class SubscriberMetrics extends Metrics
{
    /**
     *
     * Number of queued events.
     *
     **/
    int queued = 0;

    /**
     *
     * Number of outstanding events.
     *
     **/
    int outstanding = 0;

    /**
     *
     * Number of forwarded events.
     *
     **/
    long delivered = 0;
}

}
