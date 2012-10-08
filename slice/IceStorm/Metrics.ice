// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Metrics.ice>

module IceMX
{

/**
 *
 * Topic metrics.
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
};

/**
 *
 * Subscriber metrics.
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
};

};
