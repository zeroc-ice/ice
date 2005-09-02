// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ONEWAY_PROXY_H
#define ONEWAY_PROXY_H

#include <IceStorm/QueuedProxy.h>

namespace IceStorm
{

//
// OnewayProxy subclasses QueuedProxy for topic subscribers.
//
class OnewayProxy : public QueuedProxy
{
public:

    OnewayProxy(const Ice::ObjectPrx&);

    virtual Ice::ObjectPrx proxy() const;

protected:

    virtual void deliver(const std::vector<EventPtr>&);

private:

    Ice::ObjectPrx _obj;
};

}

#endif
