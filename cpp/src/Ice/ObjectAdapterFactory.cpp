// **********************************************************************
//
// Copyright (c) 2002
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

#include <Ice/ObjectAdapterFactory.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ObjectAdapterFactory* p) { p->__incRef(); }
void IceInternal::decRef(ObjectAdapterFactory* p) { p->__decRef(); }

void
IceInternal::ObjectAdapterFactory::shutdown()
{
    IceUtil::Mutex::Lock sync(*this);

    for_each(_adapters.begin(), _adapters.end(),
	     Ice::secondVoidMemFun<string, ObjectAdapter>(&ObjectAdapter::deactivate));

    _adapters.clear();
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::createObjectAdapter(const string& name, const string& endpts, const string& id)
{
    IceUtil::Mutex::Lock sync(*this);

    map<string, ObjectAdapterIPtr>::iterator p = _adapters.find(name);
    if(p != _adapters.end())
    {
	return p->second;
    }

    ObjectAdapterIPtr adapter = new ObjectAdapterI(_instance, _communicator, name, endpts, id);
    _adapters.insert(make_pair(name, adapter));
    return adapter;
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::findObjectAdapter(const ObjectPrx& proxy)
{
    IceUtil::Mutex::Lock sync(*this);

    for(map<string, ObjectAdapterIPtr>::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	if(p->second->isLocal(proxy))
	{
	    return p->second;
	}
    }

    return 0;
}

IceInternal::ObjectAdapterFactory::ObjectAdapterFactory(const InstancePtr& instance,
							const CommunicatorPtr& communicator) :
    _instance(instance),
    _communicator(communicator)
{
}
