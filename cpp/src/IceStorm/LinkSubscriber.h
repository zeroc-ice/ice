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

#ifndef LINK_SUBSCRIBER_H
#define LINK_SUBSCRIBER_H

#include <IceStorm/Flushable.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/IceStormInternal.h> // For TopicLink
#include <IceStorm/QueuedProxy.h>

namespace IceStorm
{

class SubscriberFactory;
typedef IceUtil::Handle<SubscriberFactory> SubscriberFactoryPtr;

class LinkSubscriber : public Subscriber, public Flushable
{
public:

    LinkSubscriber(const SubscriberFactoryPtr&, const TraceLevelsPtr&, const QueuedProxyPtr&, Ice::Int);
    ~LinkSubscriber();

    virtual bool persistent() const;
    virtual bool inactive() const;
    virtual void unsubscribe();
    virtual void replace();
    virtual void publish(const EventPtr&);

    virtual void flush();
    virtual bool operator==(const Flushable&) const;

private:

    // Immutable
    SubscriberFactoryPtr _factory;
    QueuedProxyPtr _obj;
    Ice::Int _cost;
};

} // End namespace IceStorm

#endif

