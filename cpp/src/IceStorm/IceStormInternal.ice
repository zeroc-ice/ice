// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STORM_INTERNAL_ICE
#define ICE_STORM_INTERNAL_ICE

#include <IceStorm/IceStorm.ice>
#include <IceStorm/Event.ice>

module IceStorm
{

/**
 *
 * The TopicLink interface. This is used to forward events between
 * federated Topic instances.
 *
 * @see TopicInternal
 *
 **/
interface TopicLink
{
    /**
     *
     * Forward a sequence of events.
     *
     * @param events The events to forward.
     *
     **/
    ["ami"] void forward(EventSeq events);
};

/**
 *
 * The TopicUpstreamLink interface. This is used to inform an unstream
 * topic that a federated downstream topic is alive.
 *
 * @see TopicInternal
 *
 **/
interface TopicUpstreamLink
{
    /**
     *
     *  Inform the upstream topic that a federated downstream topic is alive.
     *
     **/
    void keepAlive();
};

/**
 *
 * Internal operations for a topic.
 *
 * @see Topic
 *
 **/
interface TopicInternal extends Topic
{
    /**
     *
     * Retrieve a proxy to the TopicLink interface.
     *
     * @return The TopicLink for the Topic.
     *
     **/
    idempotent TopicLink* getLinkProxy();

    /**
     *
     * Send a link notification to the downstream topic.
     *
     * @param name The topic name.
     *
     * @param upstream The upstream topic.
     * 
     **/
    void linkNotification(string name, TopicUpstreamLink* upstream);

    /**
     *
     * Send an unlink notification to the downstream topic.
     *
     * @param name The topic name.
     *
     * @param upstream The upstream topic.
     * 
     **/
    void unlinkNotification(string name, TopicUpstreamLink* upstream);
};

}; // End module IceStorm

#endif
