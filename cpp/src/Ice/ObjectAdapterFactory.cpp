// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/ObjectAdapterFactory.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/Functional.h>


#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ObjectAdapterFactory* p) { p->__incRef(); }
void IceInternal::decRef(ObjectAdapterFactory* p) { p->__decRef(); }

void
IceInternal::ObjectAdapterFactory::shutdown()
{
    JTCSyncT<JTCMutex> sync(*this);
    for_each(_adapters.begin(), _adapters.end(),
	     ::IceInternal::secondVoidMemFun<string, ObjectAdapter>(&ObjectAdapter::deactivate));
    _adapters.clear();
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::createObjectAdapter(const string& name, const string& endpts)
{
    JTCSyncT<JTCMutex> sync(*this);
    map<string, ObjectAdapterPtr>::iterator p = _adapters.find(name);
    if (p != _adapters.end())
    {
	return p->second;
    }

    ObjectAdapterPtr adapter = new ObjectAdapterI(_instance, name, endpts);
    _adapters.insert(make_pair(name, adapter));
    return adapter;
}

ObjectPtr
IceInternal::ObjectAdapterFactory::proxyToObject(const ObjectPrx& proxy)
{
    map<string, ObjectAdapterPtr>::iterator p;
    for (p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	try
	{
	    return p->second->proxyToObject(proxy);
	}
	catch(const WrongObjectAdapterException&)
	{
	}
    }

    return 0;
}

IceInternal::ObjectAdapterFactory::ObjectAdapterFactory(const InstancePtr& instance) :
    _instance(instance)
{
}
