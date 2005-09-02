// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef QUEUED_PROXY_H
#define QUEUED_PROXY_H

#include <Ice/Proxy.h>
#include <IceStorm/Event.h>

namespace IceStorm
{

//
// QueuedProxy encapsulates a subscriber proxy in order to maintain a
// queue of events to be delivered to the subscriber. QueuedProxy
// manages the event queue, but delegates delivery to subclasses.
//
class QueuedProxy : public IceUtil::Shared
{
public:

    QueuedProxy();

    virtual void publish(const EventPtr&);

    virtual Ice::ObjectPrx proxy() const = 0;

protected:

    virtual void deliver(const std::vector<EventPtr>&) = 0;

    IceUtil::Mutex _mutex;
    std::auto_ptr<Ice::LocalException> _exception;
    bool _busy;
    std::vector<EventPtr> _events;
};

typedef IceUtil::Handle<QueuedProxy> QueuedProxyPtr;

}

#endif
