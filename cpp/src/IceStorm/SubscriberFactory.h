// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SUBSCRIBER_FACTORY_H
#define SUBSCRIBER_FACTORY_H

#include <IceStorm/IceStormInternal.h> // For QoS, TopicLink
#include <IceStorm/QueuedProxy.h>
#include <IceUtil/RecMutex.h>
#include <map>

namespace IceStorm
{

//
// Forward declarations.
//
class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Subscriber;
typedef IceUtil::Handle<Subscriber> SubscriberPtr;

class Flusher;
typedef IceUtil::Handle<Flusher> FlusherPtr;

//
// Factory object that knows how to create various type of Subscriber
// objects.
//
class SubscriberFactory  : public IceUtil::Shared
{
public:

    SubscriberFactory(const Ice::CommunicatorPtr&, const TraceLevelsPtr&, const FlusherPtr&);

    //
    // Create a link subscriber (that is a subscriber that points to
    // another topic instance).
    //
    SubscriberPtr createLinkSubscriber(const TopicLinkPrx&, Ice::Int);

    //
    // Create a Subscriber with the given QoS.
    //
    SubscriberPtr createSubscriber(const QoS&, const Ice::ObjectPrx&);

    //
    // Increment the usage count of a queued proxy.
    //
    void incProxyUsageCount(const QueuedProxyPtr&);

    //
    // Decrement the usage count of a queued proxy.
    //
    void decProxyUsageCount(const QueuedProxyPtr&);

private:

    //
    // SubscriberFactory maps all subscriber proxies to queued proxies.
    // Only one queued proxy is created for a subscriber's proxy,
    // regardless of how many topics it subscribes to.
    //
    struct ProxyInfo
    {
        QueuedProxyPtr proxy;
        Ice::Int count;
    };

    Ice::CommunicatorPtr _communicator;
    TraceLevelsPtr _traceLevels;
    FlusherPtr _flusher;
    IceUtil::RecMutex _proxiesMutex;
    std::map<Ice::ObjectPrx, ProxyInfo> _proxies;
};

typedef IceUtil::Handle<SubscriberFactory> SubscriberFactoryPtr;

} // End namespace IceStorm

#endif
