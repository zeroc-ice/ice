// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:header-ext:h"]]

#include "Ice/Identity.ice"
#include "IceStorm/IceStorm.ice"

module IceStorm
{
    /// The key for persistent subscribers, or topics.
    /// If the subscriber identity is empty then the record is used as a place holder for the creation of a topic,
    /// otherwise the record holds a subscription record.
    struct SubscriberRecordKey
    {
        /// The topic identity.
        Ice::Identity topic;

        /// The identity of the subscriber. If this is empty then the key is a placeholder for a topic.
        Ice::Identity id;
    }

    /// Used to store persistent information for persistent subscribers.
    struct SubscriberRecord
    {
        /// The name of the topic.
        string topicName;

        /// The subscriber identity.
        Ice::Identity id;

        /// Is this a link record, or a subscriber record?
        bool link;

        /// The subscriber object.
        Object* obj;

        // We could simulate a union here with a class object.

        // Persistent subscriber data.
        /// The QoS.
        QoS theQoS;

        // Link subscriber data.
        /// The cost.
        int cost;
        /// The linked topic.
        Topic* theTopic;
    }

    sequence<SubscriberRecord> SubscriberRecordSeq;
}
