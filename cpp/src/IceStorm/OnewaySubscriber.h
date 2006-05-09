// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ONEWAY_SUBSCRIBER_H
#define ONEWAY_SUBSCRIBER_H

#include <IceStorm/Subscriber.h>
#include <IceStorm/QueuedProxy.h>

namespace IceStorm
{

class SubscriberFactory;
typedef IceUtil::Handle<SubscriberFactory> SubscriberFactoryPtr;

class OnewaySubscriber : public Subscriber
{
public:

    OnewaySubscriber(const SubscriberFactoryPtr&, const Ice::CommunicatorPtr&, const TraceLevelsPtr&, 
    		     const QueuedProxyPtr&);
    ~OnewaySubscriber();

    virtual bool persistent() const;
    virtual void activate();
    virtual void unsubscribe();
    virtual void replace();
    virtual void publish(const EventPtr&);

protected:

    // Immutable
    Ice::CommunicatorPtr _communicator;
    SubscriberFactoryPtr _factory;
    QueuedProxyPtr _obj;
};

} // End namespace IceStorm

#endif
