// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/AdapterI.h>
#include <IceGrid/AdapterFactory.h>
#include <IceGrid/TraceLevels.h>

using namespace std;
using namespace IceGrid;

StandaloneAdapterI::StandaloneAdapterI(const AdapterFactoryPtr& factory) :
    _factory(factory)
{
}

StandaloneAdapterI::StandaloneAdapterI()
{
}

void
StandaloneAdapterI::activate_async(const AMD_Adapter_activatePtr& cb, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);
    cb->ice_response(proxy);
}

Ice::ObjectPrx
StandaloneAdapterI::getDirectProxy(const Ice::Current&) const
{
    IceUtil::Mutex::Lock sync(*this);
    return proxy;
}

void
StandaloneAdapterI::setDirectProxy(const Ice::ObjectPrx& prx, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);
    proxy = prx;
}

void
StandaloneAdapterI::destroy(const Ice::Current& current)
{
    if(_factory)
    {
	_factory->destroy(current.id);
    }
}
