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

#include <IceStorm/LinkProxy.h>

using namespace std;

IceStorm::LinkProxy::LinkProxy(const TopicLinkPrx& obj) :
    _obj(obj)
{
}

Ice::ObjectPrx
IceStorm::LinkProxy::proxy() const
{
    return _obj;
}

void
IceStorm::LinkProxy::deliver(const EventPtr& event)
{
    _obj->forward(event->op, event->mode, event->data, event->context);
}
