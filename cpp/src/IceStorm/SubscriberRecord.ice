// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["ice-prefix", "cpp:header-ext:h"]]

#include <Ice/Identity.ice>
#include <IceStorm/IceStorm.ice>

module IceStorm
{

/**
 *
 * The key for persistent subscribers, or topics.
 *
 * If the subscriber identity is empty then the record is used as a
 * place holder for the creation of a topic, otherwise the record
 * holds a subscription record.
 *
 **/
struct SubscriberRecordKey
{
    // The topic identity.
    Ice::Identity topic;

    // The identity of the subscriber. If this is empty then the key
    // is a placeholder for a topic.
    Ice::Identity id;
}

/**
 *
 * Used to store persistent information for persistent subscribers.
 *
 **/
struct SubscriberRecord
{
    string topicName; // The name of the topic.

    Ice::Identity id; // The subscriber identity.

    // Is this a link record, or a subscriber record?
    bool link;

    Object* obj; // The subscriber object.

    // We could simulate a union here with a class object.

    // Persistent subscriber data.
    QoS theQoS; // The QoS.

    // Link subscriber data.
    int cost; // The cost.
    Topic* theTopic;  // The linked topic.
}

sequence<SubscriberRecord> SubscriberRecordSeq;

} // End module IceStorm
