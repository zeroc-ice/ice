// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICESTORM_API"]]
[["cpp:doxygen:include:IceStorm/IceStorm.h"]]
[["cpp:header-ext:h"]]

[["cpp:include:IceStorm/Config.h"]]

[["js:module:@zeroc/ice"]]

#include "Ice/Identity.ice"
#include "Metrics.ice"

/// Lightweight publish/subscribe framework, available for all Ice language mappings.
["java:identifier:com.zeroc.IceStorm"]
module IceStorm
{
    interface Topic;

    /// Information about a topic link.
    struct LinkInfo
    {
        /// The linked topic proxy. This proxy is never null.
        Topic* theTopic;

        /// The name of the linked topic.
        string name;

        /// The cost of traversing this link.
        int cost;
    }

    /// A sequence of {@link LinkInfo} objects.
    sequence<LinkInfo> LinkInfoSeq;

    /// Quality of service parameters.
    /// @see Topic#subscribeAndGetPublisher
    dictionary<string, string> QoS;

    /// The exception that is thrown when attempting to create a link that already exists.
    exception LinkExists
    {
        /// The name of the linked topic.
        string name;
    }

    /// The exception that is thrown when attempting to remove a link that does not exist.
    exception NoSuchLink
    {
        /// The name of the link that does not exist.
        string name;
    }

    /// The exception that is thrown when attempting to subscribe a proxy for which a subscription already exists.
    exception AlreadySubscribed
    {
    }

    /// The exception that is thrown when attempting to subscribe with an invalid {@link QoS}.
    exception BadQoS
    {
        /// The reason for the failure.
        string reason;
    }

    /// Represents an IceStorm topic. Publishers publish data to a topic (via the topic's publisher object), and
    /// subscribers subscribe to a topic.
    /// @see TopicManager
    interface Topic
    {
        /// Gets the name of this topic.
        /// @return The name of the topic.
        /// @see TopicManager#create
        ["cpp:const"]
        idempotent string getName();

        /// Gets a proxy to a publisher object for this topic. To publish data to a topic, a publisher calls this
        /// operation and then creates a proxy with the publisher type from this proxy. If a replicated IceStorm
        /// deployment is used, this call may return a replicated proxy.
        /// @return A proxy to publish data on this topic. This proxy is never null.
        ["cpp:const"]
        idempotent Object* getPublisher();

        /// Gets a non-replicated proxy to a publisher object for this topic. To publish data to a topic, a publisher
        /// calls this operation and then creates a proxy with the publisher type from this proxy.
        /// @return A proxy to publish data on this topic. This proxy is never null.
        ["cpp:const"]
        idempotent Object* getNonReplicatedPublisher();

        /// Subscribes to this topic.
        /// @param theQoS The quality of service parameters for this subscription.
        /// @param subscriber The subscriber's proxy. This proxy cannot be null.
        /// @return The per-subscriber publisher proxy. This proxy is never null.
        /// @throws AlreadySubscribed Thrown when @p subscriber is already subscribed.
        /// @throws BadQoS Thrown when @p theQoS is unavailable or invalid.
        /// @see #unsubscribe
        Object* subscribeAndGetPublisher(QoS theQoS, Object* subscriber)
            throws AlreadySubscribed, BadQoS;

        /// Unsubscribes the provided @p subscriber from this topic.
        /// @param subscriber A proxy to an existing subscriber. This proxy is never null.
        /// @see #subscribeAndGetPublisher
        idempotent void unsubscribe(Object* subscriber);

        /// Creates a link to another topic. All events originating on this topic will also be sent to the other topic.
        /// @param linkTo The topic to link to. This proxy cannot be null.
        /// @param cost The cost of the link.
        /// @throws LinkExists Thrown when a link to @p linkTo already exists.
        void link(Topic* linkTo, int cost) throws LinkExists;

        /// Destroys a link from this topic to the provided topic.
        /// @param linkTo The topic to destroy the link to. This proxy cannot be null.
        /// @throws NoSuchLink Thrown when a link to @p linkTo does not exist.
        void unlink(Topic* linkTo) throws NoSuchLink;

        /// Gets information on the current links.
        /// @return A sequence of LinkInfo objects.
        ["cpp:const"]
        idempotent LinkInfoSeq getLinkInfoSeq();

        /// Gets the list of subscribers for this topic.
        /// @return The sequence of Ice identities for the subscriber objects.
        ["cpp:const"] Ice::IdentitySeq getSubscribers();

        /// Destroys this topic.
        void destroy();
    }

    /// A dictionary of topic name to topic proxy.
    dictionary<string, Topic*> TopicDict;

    /// The exception that is thrown when attempting to create a topic that already exists.
    exception TopicExists
    {
        /// The name of the topic that already exists.
        string name;
    }

    /// The exception that is thrown when attempting to retrieve a topic that does not exist.
    exception NoSuchTopic
    {
        /// The name of the topic that does not exist.
        string name;
    }

    /// Represents an object that manages topics.
    /// @see Topic
    interface TopicManager
    {
        /// Creates a new topic.
        /// @param name The name of the topic.
        /// @return A proxy to the new topic object. The returned proxy is never null.
        /// @throws TopicExists Thrown when a topic with the same @p name already exists.
        Topic* create(string name) throws TopicExists;

        /// Retrieves a topic by name.
        /// @param name The name of the topic.
        /// @return A proxy to the topic object. The returned proxy is never null.
        /// @throws NoSuchTopic Thrown when there is no topic named @p name.
        idempotent Topic* retrieve(string name) throws NoSuchTopic;

        /// Retrieves all topics managed by this topic manager.
        /// @return A dictionary of string, topic proxy pairs.
        idempotent TopicDict retrieveAll();
    }

    /// Provides access to a {@link TopicManager} object via a fixed identity.
    /// An IceStorm Finder is always registered with identity `IceStorm/Finder`. This allows clients to obtain the
    /// associated TopicManager proxy with just the endpoint information of the object. For example, you can use the
    /// Finder proxy `IceStorm/Finder:tcp -h somehost -p 4061` to get the TopicManager proxy
    /// `MyIceStorm/TopicManager:tcp -h somehost -p 4061`.
    interface Finder
    {
        /// Gets a proxy to the associated {@link TopicManager}. The proxy might point to several replicas.
        /// @return The topic manager proxy. This proxy is never null.
        TopicManager* getTopicManager();
    }
}
