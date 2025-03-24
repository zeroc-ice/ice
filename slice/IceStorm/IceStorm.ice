// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICESTORM_API"]]
[["cpp:doxygen:include:IceStorm/IceStorm.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceStorm/Config.h"]]

[["java:package:com.zeroc"]]
[["js:module:@zeroc/ice"]]
[["python:pkgdir:IceStorm"]]

#include "Ice/Identity.ice"
#include "Metrics.ice"

/// Lightweight publish/subscribe framework, available for all Ice language mappings.
module IceStorm
{
    interface Topic;

    /// Information on the topic links.
    struct LinkInfo
    {
        /// The linked topic. It is never null.
        Topic* theTopic;

        /// The name of the linked topic.
        string name;

        /// The cost of traversing this link.
        int cost;
    }

    /// A sequence of {@link LinkInfo} objects.
    sequence<LinkInfo> LinkInfoSeq;

    /// This dictionary represents quality of service parameters.
    /// @see Topic#subscribeAndGetPublisher
    dictionary<string, string> QoS;

    /// This exception indicates that an attempt was made to create a link that already exists.
    exception LinkExists
    {
        /// The name of the linked topic.
        string name;
    }

    /// This exception indicates that an attempt was made to remove a link that does not exist.
    exception NoSuchLink
    {
        /// The name of the link that does not exist.
        string name;
    }

    /// This exception indicates that an attempt was made to subscribe a proxy for which a subscription already exists.
    exception AlreadySubscribed
    {
    }

    /// This exception indicates that an attempt was made to subscribe a proxy that is null.
    exception InvalidSubscriber
    {
        /// The reason for the failure.
        string reason;
    }

    /// This exception indicates that a subscription failed due to an invalid QoS.
    exception BadQoS
    {
        /// The reason for the failure.
        string reason;
    }

    /// Publishers publish information on a particular topic. A topic logically represents a type.
    /// @see TopicManager
    interface Topic
    {
        /// Get the name of this topic.
        /// @return The name of the topic.
        /// @see TopicManager#create
        ["cpp:const"] idempotent string getName();

        /// Get a proxy to a publisher object for this topic. To publish data to a topic, the publisher calls
        /// {@link getPublisher} and then creates a proxy with the publisher type from this proxy. If a replicated
        /// IceStorm deployment is used this call may return a replicated proxy. The returned proxy is never null.
        /// @return A proxy to publish data on this topic.
        ["cpp:const"] idempotent Object* getPublisher();

        /// Get a non-replicated proxy to a publisher object for this topic. To publish data to a topic, the publisher
        /// calls getPublisher and then creates a proxy with the publisher type from this proxy. The returned proxy is
        /// never null.
        /// @return A proxy to publish data on this topic.
        ["cpp:const"] idempotent Object* getNonReplicatedPublisher();

        /// Subscribe with the given <code>qos</code> to this topic. A per-subscriber publisher object is returned.
        /// @param theQoS The quality of service parameters for this subscription.
        /// @param subscriber The subscriber's proxy. This proxy is never null.
        /// @return The per-subscriber publisher object. The returned object is never null.
        /// @throws AlreadySubscribed Raised if the subscriber object is already subscribed.
        /// @throws InvalidSubscriber Raised if the subscriber object is null.
        /// @throws BadQoS Raised if the requested quality of service is unavailable or invalid.
        /// @see #unsubscribe
        Object* subscribeAndGetPublisher(QoS theQoS, Object* subscriber)
            throws AlreadySubscribed, InvalidSubscriber, BadQoS;

        /// Unsubscribe the given <code>subscriber</code>.
        /// @param subscriber The proxy of an existing subscriber. This proxy is never null.
        /// @see #subscribeAndGetPublisher
        idempotent void unsubscribe(Object* subscriber);

        /// Create a link to the given topic. All events originating on this topic will also be sent to
        /// <code>linkTo</code>.
        /// @param linkTo The topic to link to. This proxy is never null.
        /// @param cost The cost to the linked topic.
        /// @throws LinkExists Raised if a link to the same topic already exists.
        void link(Topic* linkTo, int cost) throws LinkExists;

        /// Destroy the link from this topic to the given topic <code>linkTo</code>.
        /// @param linkTo The topic to destroy the link to. This proxy is never null.
        /// @throws NoSuchLink Raised if a link to the topic does not exist.
        void unlink(Topic* linkTo) throws NoSuchLink;

        /// Retrieve information on the current links.
        /// @return A sequence of LinkInfo objects.
        ["cpp:const"] idempotent LinkInfoSeq getLinkInfoSeq();

        /// Retrieve the list of subscribers for this topic.
        /// @return The sequence of Ice identities for the subscriber objects.
        ["cpp:const"] Ice::IdentitySeq getSubscribers();

        /// Destroy the topic.
        void destroy();
    }

    /// Mapping of topic name to topic proxy.
    dictionary<string, Topic*> TopicDict;

    /// This exception indicates that an attempt was made to create a topic that already exists.
    exception TopicExists
    {
        /// The name of the topic that already exists.
        string name;
    }

    /// This exception indicates that an attempt was made to retrieve a topic that does not exist.
    exception NoSuchTopic
    {
        /// The name of the topic that does not exist.
        string name;
    }

    /// A topic manager manages topics, and subscribers to topics.
    /// @see Topic
    interface TopicManager
    {
        /// Create a new topic. The topic name must be unique.
        /// @param name The name of the topic.
        /// @return A proxy to the topic instance. The returned proxy is never null.
        /// @throws TopicExists Raised if a topic with the same name already exists.
        Topic* create(string name) throws TopicExists;

        /// Retrieve a topic by name.
        /// @param name The name of the topic.
        /// @return A proxy to the topic instance. The returned proxy is never null.
        /// @throws NoSuchTopic Raised if the topic does not exist.
        idempotent Topic* retrieve(string name) throws NoSuchTopic;

        /// Retrieve all topics managed by this topic manager.
        /// @return A dictionary of string, topic proxy pairs.
        idempotent TopicDict retrieveAll();
    }

    /// This interface is advertised by the IceStorm service through the Ice object with the identity 'IceStorm/Finder'.
    /// This allows clients to retrieve the topic manager with just the endpoint information of the IceStorm service.
    interface Finder
    {
        /// Get the topic manager proxy. The proxy might point to several replicas.
        /// @return The topic manager proxy. The returned proxy is never null.
        TopicManager* getTopicManager();
    }
}
