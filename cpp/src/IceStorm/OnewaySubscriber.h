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

    OnewaySubscriber(const SubscriberFactoryPtr&, const TraceLevelsPtr&, const QueuedProxyPtr&);
    ~OnewaySubscriber();

    virtual bool persistent() const;
    virtual void unsubscribe();
    virtual void replace();
    virtual void publish(const EventPtr&);

protected:

    // Immutable
    SubscriberFactoryPtr _factory;
    QueuedProxyPtr _obj;
};

} // End namespace IceStorm

#endif
