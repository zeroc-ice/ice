// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/OnewayProxy.h>

using namespace std;

IceStorm::OnewayProxy::OnewayProxy(const Ice::ObjectPrx& obj) :
    _obj(obj)
{
}

Ice::ObjectPrx
IceStorm::OnewayProxy::proxy() const
{
    return _obj;
}

void
IceStorm::OnewayProxy::deliver(const EventPtr& event)
{
    vector<Ice::Byte> dummy;
    _obj->ice_invoke(event->op, event->mode, event->data, dummy, event->context);
}
