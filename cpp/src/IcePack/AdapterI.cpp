// **********************************************************************
//
// Copyright (c) 2001
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

#include <Ice/Ice.h>
#include <IcePack/AdapterI.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

IcePack::StandaloneAdapterI::StandaloneAdapterI(const AdapterFactoryPtr& factory) :
    _factory(factory)
{
}

IcePack::StandaloneAdapterI::StandaloneAdapterI()
{
}

Ice::ObjectPrx
IcePack::StandaloneAdapterI::getDirectProxy(bool activate, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);
    return proxy;
}

void
IcePack::StandaloneAdapterI::setDirectProxy(const Ice::ObjectPrx& prx, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);
    proxy = prx;
}

void
IcePack::StandaloneAdapterI::destroy(const Ice::Current& current)
{
    if(_factory)
    {
	_factory->destroy(current.id);
    }
}
