// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LINK_SUBSCRIBER_H
#define LINK_SUBSCRIBER_H

#include <IceStorm/Flushable.h>
#include <IceStorm/Subscriber.h>

namespace IceStorm
{

class LinkSubscriber : public Subscriber, public Flushable
{
public:

    LinkSubscriber(const SubscriberFactoryPtr&, const Ice::CommunicatorPtr&,
	           const TraceLevelsPtr&, const QueuedProxyPtr&, Ice::Int);
    ~LinkSubscriber();

    virtual bool persistent() const;
    virtual bool inactive() const;
    virtual void publish(const EventPtr&);
    virtual Ice::ObjectPrx proxy() const;

    virtual void flush();
    virtual bool operator==(const Flushable&) const;

private:

    // Immutable
    const Ice::CommunicatorPtr _communicator;
    const Ice::Int _cost;
};

} // End namespace IceStorm

#endif

