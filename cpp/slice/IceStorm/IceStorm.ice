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
 * This dictionary represents Quality of service parameters.
 *
 * @see TopicManager::subscribe
 *
 */
dictionary<string, string> QoS;

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
     * Get the name of this topic.
     *
     * @return The name of the topic.
     *
     * @see TopicManager::create
     *
     **/
    nonmutating string getName();
    
    /**
     *
     * Get the type of this topic.
     *
     * @return The type of the topic.
     *
     * @see TopicManager::create
     *
     **/
    nonmutating string getType();
    
    /**
     *
     * Get a proxy to a publisher object for this topic. To publish
     * data to a topic, the publisher calls [getPublisher] and then
     * casts to the topic type. A checked cast may be used on the
     * publisher object if IceStorm is configured with
     * connection-oriented transports, otherwise an unchecked cast
     * must be used.
     *
     * @return A proxy to publish data on this topic.
     *
     **/
    nonmutating Object* getPublisher();

    /**
     *
     * Subscribe with the given [qos] to this topic. If the given
     * [subscriber] proxy has already been registered, it will be
     * replaced.
     *
     * @param qos The quality of service parameters for this
     * subscription.
     *
     * @param subscriber The subscriber's proxy, which must
     * implement the topic type.
     *
     * @see unsubscribe
     *
     **/
    void subscribe(QoS theQoS, Object* subscriber);

    /**
     *
     * Unsubscribe the given [subscriber].
     *
     * @param subscriber The proxy of an existing subscriber.
     *
     * @see subscribe
     *
     **/
    idempotent void unsubscribe(Object* subscriber);

    /**
     *
     * Create a link to the given topic. All events originating
     * on this topic will also be sent to [linkTo].
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
 * This exception indicates that a type ID does not have the
 * proper format.
 *
 **/
exception InvalidType
{
    /**
     *
     * The invalid type.
     *
     */
    string type;
};

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
     * Create a new topic. The topic name must be unique, otherwise
     * [TopicExists] is raised. The topic type identifies the
     * most-derived type of the topic and must be a fully-scoped
     * Slice type ID (such as <literal>::A::B</literal>). If the
     * format of the type ID is invalid, [InvalidType] is raised.
     *
     * @param name The name of the topic.
     *
     * @param type The type ID of the topic.
     *
     * @return A proxy to the topic instance.
     *
     * @throws TopicExists Raised if a topic with the same name already
     * exists.
     *
     **/
    Topic* create(string name, string type) throws TopicExists, InvalidType;

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
};

};

#endif
