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

void
IcePack::StandaloneAdapterI::getDirectProxy_async(const AMD_Adapter_getDirectProxyPtr& cb,
						  bool activate,
						  const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);
    cb->ice_response(proxy);
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
