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

#ifndef QUEUED_PROXY_H
#define QUEUED_PROXY_H

#include <Ice/Proxy.h>
#include <IceStorm/Event.h>

namespace IceStorm
{

//
// QueuedProxy encapsulates a subscriber proxy in order to maintain
// a queue of events to be delivered to the subscriber. QueuedProxy
// manages the event queue, but delegates delivery to subsclasses.
//
class QueuedProxy : public IceUtil::Shared
{
public:

    QueuedProxy();

    void publish(const EventPtr&);

    virtual Ice::ObjectPrx proxy() const = 0;

protected:

    virtual void deliver(const EventPtr&) = 0;

private:

    IceUtil::Mutex _mutex;
    bool _busy;
    std::auto_ptr<Ice::LocalException> _exception;
    std::vector<EventPtr> _events;
};

typedef IceUtil::Handle<QueuedProxy> QueuedProxyPtr;

}

#endif
