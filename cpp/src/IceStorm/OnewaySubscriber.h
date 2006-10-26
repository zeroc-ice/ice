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

namespace IceStorm
{

class OnewaySubscriber : public Subscriber
{
public:

    OnewaySubscriber(const SubscriberFactoryPtr&, const Ice::CommunicatorPtr&, const TraceLevelsPtr&, 
		     const Ice::ObjectAdapterPtr&, const QueuedProxyPtr&);
    ~OnewaySubscriber();

    virtual bool persistent() const;
    virtual void activate();
    virtual void unsubscribe();
    virtual void replace();
    virtual void reachable();
    virtual void publish(const EventPtr&);
    virtual Ice::ObjectPrx proxy() const;

protected:

    Ice::ObjectAdapterPtr _adapter;
    Ice::ObjectPrx _proxy;
};

} // End namespace IceStorm

#endif
