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
    _obj->ice_invoke(event->op, Ice::Idempotent, event->data, dummy, event->context);
}
