//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["ice-prefix", "cpp:header-ext:h"]]

#include <Ice/Instrumentation.ice>
#include <IceStorm/IceStorm.ice>

module IceStorm
{

module Instrumentation
{

local interface TopicObserver extends Ice::Instrumentation::Observer
{
    /**
     *
     * Notification of an event published on the topic by a publisher.
     *
     **/
    void published();

    /**
     *
     * Notification of an event forwared on the topic by another topic.
     *
     **/
    void forwarded();
}

local interface SubscriberObserver extends Ice::Instrumentation::Observer
{
    /**
     *
     * Notification of some events being queued.
     *
     **/
    void queued(int count);

    /**
     *
     * Notification of a some events being sent.
     *
     **/
    void outstanding(int count);

    /**
     *
     * Notification of some events being delivered.
     *
     **/
    void delivered(int count);
}

/**
 *
 * The ObserverUpdater interface is implemented by IceStorm and an
 * instance of this interface is provided on initialization to the
 * TopicManagerObserver object.
 *
 * This interface can be used by add-ins imlementing the
 * TopicManagerObserver interface to update the obsevers of observed
 * objects.
 *
 **/
local interface ObserverUpdater
{
    /**
     *
     * Update topic observers associated with each topics.
     *
     * When called, this method goes through all the topics and for
     * each topic TopicManagerObserver::getTopicObserver is
     * called. The implementation of getTopicObserver has the
     * possibility to return an updated observer if necessary.
     *
     **/
    void updateTopicObservers();

    /**
     *
     * Update subscriber observers associated with each subscriber.
     *
     * When called, this method goes through all the subscribers and
     * for each subscriber TopicManagerObserver::getSubscriberObserver
     * is called. The implementation of getSubscriberObserver has the
     * possibility to return an updated observer if necessary.
     *
     **/
    void updateSubscriberObservers();
}

enum SubscriberState
{
    /**
     *
     * Online waiting to send events.
     *
     **/
    SubscriberStateOnline,

    /**
     *
     * Offline, retrying.
     *
     **/
    SubscriberStateOffline,

    /**
     *
     * Error state, awaiting to be destroyed.
     *
     **/
    SubscriberStateError
}

/**
 *
 * The topic manager observer interface used by the Ice run-time to
 * obtain and update observers for its observeable objects. This
 * interface should be implemented by add-ins that wish to observe
 * IceStorm objects in order to collect statistics.
 *
 **/
local interface TopicManagerObserver
{
    /**
     *
     * This method should return an observer for the given topic.
     *
     * @param svc The service name.
     *
     * @param name The topic name.
     *
     * @param old The previous observer, only set when updating an
     * existing observer.
     *
     **/
    TopicObserver getTopicObserver(string svc, string name, TopicObserver old);

    /**
     *
     * This method should return an observer for the given subscriber.
     *
     * @param topic The name of the topic subscribed.
     *
     * @param subscriber The proxy of the subscriber.
     *
     * @param qos The QoS configured for the subscriber.
     *
     * @param link The proxy of the linked topic if this subscriber
     * forwards events to a linked topic.
     *
     * @param old The previous observer, only set when updating an
     * existing observer.
     *
     **/
    SubscriberObserver getSubscriberObserver(string svc, string topic, Object* prx, QoS q, IceStorm::Topic* link,
                                             SubscriberState s, SubscriberObserver old);

    /**
     *
     * IceStorm calls this method on initialization. The add-in
     * implementing this interface can use this object to get IceStorm
     * to re-obtain observers for topics and subscribers.
     *
     * @param updater The observer updater object.
     *
     **/
    void setObserverUpdater(ObserverUpdater updater);
}

}

}
