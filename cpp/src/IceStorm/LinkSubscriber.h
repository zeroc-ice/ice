// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    LinkSubscriber(const SubscriberFactoryPtr&, const Ice::CommunicatorPtr&,
	           const TraceLevelsPtr&, const QueuedProxyPtr&, Ice::Int);
    ~LinkSubscriber();

    virtual bool persistent() const;
    virtual bool inactive() const;
    virtual void activate();
    virtual void unsubscribe();
    virtual void replace();
    virtual void publish(const EventPtr&);

    virtual void flush();
    virtual bool operator==(const Flushable&) const;

private:

    // Immutable
    SubscriberFactoryPtr _factory;
    Ice::CommunicatorPtr _communicator;
    QueuedProxyPtr _obj;
    Ice::Int _cost;
};

} // End namespace IceStorm

#endif

