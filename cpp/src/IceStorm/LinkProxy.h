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

#ifndef LINK_PROXY_H
#define LINK_PROXY_H

#include <IceStorm/IceStormInternal.h>
#include <IceStorm/QueuedProxy.h>

namespace IceStorm
{

//
// LinkProxy subclasses QueuedProxy for topic links.
//
class LinkProxy : public QueuedProxy
{
public:

    LinkProxy(const TopicLinkPrx&);

    virtual Ice::ObjectPrx proxy() const;

protected:

    virtual void deliver(const EventPtr&);

private:

    TopicLinkPrx _obj;
};

}

#endif
