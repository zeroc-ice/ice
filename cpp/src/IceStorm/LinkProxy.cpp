// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
