// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
StandaloneAdapterI::activate_async(const AMD_Adapter_activatePtr& cb, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);
    cb->ice_response(proxy);
}

Ice::ObjectPrx
IcePack::StandaloneAdapterI::getDirectProxy(const Ice::Current&) const
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
