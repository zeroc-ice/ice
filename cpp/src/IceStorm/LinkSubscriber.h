// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef LINK_SUBSCRIBER_H
#define LINK_SUBSCRIBER_H

#include <IceStorm/Flushable.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/IceStormInternal.h> // For TopicLink

namespace IceStorm
{

class LinkSubscriber : public Subscriber, public Flushable
{
public:

    LinkSubscriber(const TraceLevelsPtr&, const TopicLinkPrx&, Ice::Int);
    ~LinkSubscriber();

    virtual bool persistent() const;
    virtual bool inactive() const;
    virtual void unsubscribe();
    virtual void replace();
    virtual void publish(const Event&);

    virtual void flush();
    virtual bool operator==(const Flushable&) const;

private:

    // Immutable
    TopicLinkPrx _obj;
    Ice::Int _cost;
};

} // End namespace IceStorm

#endif

