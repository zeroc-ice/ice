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

#ifndef ICE_STORM_ICE
#define ICE_STORM_ICE

/**
 *
 * A messaging service with support for federation. In contrast to
 * most other messaging or event services, &IceStorm; supports typed
 * events, meaning that broadcasting a message over a federation is as
 * easy as invoking a method on an interface.
 *
 **/
module IceStorm
{

interface Topic;

/**
 *
 * Information on the topic links.
 *
 **/
struct LinkInfo
{
    /**
     *
     * The linked topic.
     *
     **/
    Topic* theTopic;

    /**
     *
     * The name of the linked topic.
     *
     **/
    string name;

    /**
     *
     * The cost of traversing this link.
     *
     **/
    int cost;
};

/**
 *
 * A sequence of [LinkInfo] objects.
 *
 **/
sequence<LinkInfo> LinkInfoSeq;

/**
 *
 * Publishers publish information on a particular topic. A topic
 * logically represents a type.
 *
 * @see TopicManager
 *
 **/
interface Topic
{
    /**
     *
     * Get the name of this topic. The name of the topic is for
     * administrative purposes. It is also important for subscribers
     * to form correct object-identity.
     *
     * @return The name of the topic.
     *
     * @see TopicManager::create
     * @see TopicManager::subscribe
     *
     **/
    nonmutating string getName();
    
    /**
     *
     * Get a proxy to a publisher object for this topic.  To publish
     * data to a topic the publisher calls [getPublisher] and then
     * casts to the appropriate type (at present the cast must be
     * unchecked since the type is not validated).
     *
     * @return A proxy to publish data on this topic.
     *
     **/
    nonmutating Object* getPublisher();

    /**
     *
     * Create a link to the given topic with the cost. All events
     * flowing through this topic will flow to the given topic
     * [linkTo].
     *
     * @param linkTo The topic to link to.
     *
     * @param cost The cost to the linked topic.
     *
     **/
    idempotent void link(Topic* linkTo, int cost);

    /**
     *
     * Destroy the link from this topic to the given topic [linkTo].
     *
     * @param link The topic to destroy the link to.
     *
     **/
    idempotent void unlink(Topic* linkTo);

    /**
     *
     * Retrieve information on the current links.
     *
     * @return A sequence of LinkInfo objects.
     *
     **/
    nonmutating LinkInfoSeq getLinkInfoSeq();

    /**
     *
     * Destroy the topic.
     *
     **/
    void destroy();
};

/**
 *
 * Mapping of topic name to topic proxy.
 *
 **/
dictionary<string, Topic*> TopicDict;

/**
 *
 * This exception indicates that an attempt was made to create a topic
 * that already exists.
 *
 **/
exception TopicExists
{
    /**
     *
     * The name of the topic that already exists.
     *
     */
    string name;
};

/**
 *
 * This exception indicates that an attempt was made to retrieve a
 * topic that does not exist.
 *
 **/
exception NoSuchTopic
{
    /**
     *
     * The name of the topic that does not exist.
     *
     */
    string name;
};

/**
 *
 * This dictionary represents Quality of service parameters.
 *
 * @see TopicManager::subscribe
 *
 */
dictionary<string, string> QoS;

/**
 *
 * A topic manager manages topics, and subscribers to topics.
 *
 * @see Topic
 *
 **/
interface TopicManager
{
    /**
     *
     * Create a topic by name.
     *
     * @param name The name of the topic.
     *
     * @return A proxy to the topic instance.
     *
     * @throws TopicExists Raised if the topic exists.
     *
     **/
    Topic* create(string name) throws TopicExists;

    /**
     *
     * Retrieve a topic by name.
     *
     * @param name The name of the topic.
     *
     * @return A proxy to the topic instance.
     *
     * @throws NoSuchTopic Raised if the topic does not exist.
     *
     **/
    nonmutating Topic* retrieve(string name) throws NoSuchTopic;

    /**
     *
     * Retrieve all topics managed by this topic manager.
     *
     * @return A dictionary of string, topic proxy pairs.
     *
     **/
    nonmutating TopicDict retrieveAll();

    /**
     *
     * Subscribe with the given [qos] to the topic identified in the
     * [subscribers] identity category. If a subscriber already exists
     * with the given identity, then it will be replaced by this
     * subscriber.
     *
     * @param qos The quality of service parameters for this
     * subscription. The only currently supported QoS is
     * "reliability", which can be either "oneway" or "batch".
     *
     * @param subscriber The proxy to which to send events.
     *
     * @see unsubscribe
     *
     **/
    void subscribe(QoS theQoS, Object* subscriber) throws NoSuchTopic;

    /**
     *
     * Unsubscribe the given [subscriber].
     *
     * @param subscriber The proxy of the given subscriber.
     *
     * @see subscribe
     *
     **/
    idempotent void unsubscribe(Object* subscriber);

    /**
     *
     * Shutdown the &IceStorm; instance.
     *
     **/
    idempotent void shutdown();
};

};

#endif
