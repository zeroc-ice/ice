// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TWOWAY_PROXY_H
#define TWOWAY_PROXY_H

#include <IceStorm/QueuedProxy.h>

namespace IceStorm
{

//
// TwowayProxy subclasses QueuedProxy for topic subscribers.
//
class TwowayProxy : public QueuedProxy
{
public:

    TwowayProxy(const Ice::ObjectPrx&);

    virtual Ice::ObjectPrx proxy() const;

protected:

    const Ice::ObjectPrx _obj;
};

//
// Ordered two way proxy, events are guaranted to be delivered in the
// order they were received.
//
class OrderedTwowayProxy : public TwowayProxy
{
public:

    OrderedTwowayProxy(const Ice::ObjectPrx&);

    virtual void publish(const EventPtr&);
    void response();
    void exception(const Ice::LocalException&);
    
protected:

    virtual void deliver(const EventPtr&);
    void send(IceUtil::Mutex::Lock&);

};

typedef IceUtil::Handle<OrderedTwowayProxy> OrderedTwowayProxyPtr;

//
// Un-ordered two way proxy. There's not guarantees on the order of
// delivery of the events.
//
class UnorderedTwowayProxy : public TwowayProxy
{
public:

    UnorderedTwowayProxy(const Ice::ObjectPrx&);

    void exception(const Ice::LocalException&);

protected:

    virtual void deliver(const EventPtr&);
};

typedef IceUtil::Handle<UnorderedTwowayProxy> UnorderedTwowayProxyPtr;

}

#endif
