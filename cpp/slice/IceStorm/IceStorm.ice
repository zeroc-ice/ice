// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STORM_ICE
#define ICE_STORM_ICE

/**
 *
 * The &Ice; module for publish/subscribe.
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
    Topic* topic;

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
 * A sequence of LinkInfo objects.
 *
 **/
sequence<LinkInfo> LinkInfoSeq;

/**
 *
 * Publishers publish information on a particular Topic. A topic
 * logically represents a type.
 *
 * @see TopicManager
 *
 **/
interface Topic
{
    /**
     *
     * Get the name of this Topic. The name of the Topic is for
     * administrative purposes. It is also important for subscribers
     * to form correct object-identity.
     *
     * @return The name of the topic.
     *
     * @see TopicManager::create
     * @see TopicManager::subscribe
     *
     **/
    ["nonmutating"] string getName();
    
    /**
     *
     * Get a proxy to a publisher object for this Topic.  To publish
     * data to a Topic the publisher calls [getPublisher] and then
     * casts to the appropriate type (at present the cast must be
     * unchecked since the type is not validated).
     *
     * @return A proxy to publish data on this Topic.
     *
     **/
    ["nonmutating"] Object* getPublisher();

    /**
     *
     * Create a link to the given Topic with the cost. All events
     * flowing through this topic will flow to the given Topic
     * [linkTo].
     *
     * @param linkTo The topic to link to.
     *
     * @param cost The cost to the linked topic.
     *
     **/
    void link(Topic* linkTo, int cost);

    /**
     *
     * Destroy the link from this topic to the given Topic [linkTo].
     *
     * @param link The topic to destroy the link to.
     *
     **/
    void unlink(Topic* linkTo);

    /**
     *
     * Retrieve information on the current links.
     *
     * @return A sequence of LinkInfo objects.
     *
     **/
    ["nonmutating"] LinkInfoSeq getLinkInfoSeq();

    /**
     *
     * Destroy the Topic.
     *
     **/
    void destroy();
};

/**
 *
 * Mapping of Topic name to Topic proxy.
 *
 **/
dictionary<string, Topic*> TopicDict;

/**
 *
 * This exception indicates that an attempt was made to create a Topic
 * that already exists.
 *
 **/
exception TopicExists
{
    /**
     *
     * The name of the Topic that already exists.
     *
     */
    string name;
};

/**
 *
 * This exception indicates that an attempt was made to retrieve a
 * Topic that doesn't exist.
 *
 **/
exception NoSuchTopic
{
    /**
     *
     * The name of the Topic that doesn't exist.
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
 * A TopicManager manages Topics, and subscribers to Topics.
 *
 * @see Topic
 *
 **/
interface TopicManager
{
    /**
     *
     * Create a Topic by name.
     *
     * @param name The name of the Topic.
     *
     * @return A proxy to the Topic instance.
     *
     * @throws TopicExists Raised if the Topic exists.
     *
     **/
    Topic* create(string name) throws TopicExists;

    /**
     *
     * Retrieve a Topic by name.
     *
     * @param name The name of the Topic.
     *
     * @return A proxy to the Topic instance.
     *
     * @throws NoSuchTopic Raised if the Topic doesn't exist.
     *
     **/
    ["nonmutating"] Topic* retrieve(string name) throws NoSuchTopic;

    /**
     *
     * Retrieve all Topics managed by this TopicManager.
     *
     * @return A dictionary of string, Topic proxy pairs.
     *
     **/
    ["nonmutating"] TopicDict retrieveAll();

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
     * @param tmpl The proxy to which to send events.
     *
     * @see unsubscribe
     *
     **/
    void subscribe(QoS qos, Object* subscriber) throws NoSuchTopic;

    /**
     *
     * Unsubscribe the given [subscriber].
     *
     * @param id The identity of the given subscriber.
     *
     * @param topics The topics to which to unsubscribe.
     *
     * @see subscribe
     *
     **/
    void unsubscribe(Object* subscriber);

    /**
     *
     * Shutdown the &IceStorm; instance.
     *
     **/
    void shutdown();
};

};

#endif
