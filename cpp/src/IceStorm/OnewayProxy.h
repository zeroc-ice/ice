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

    virtual void deliver(const EventPtr&);

private:

    Ice::ObjectPrx _obj;
};

}

#endif
