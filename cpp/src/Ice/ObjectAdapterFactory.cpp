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
	     secondVoidMemFun<string, ObjectAdapter>(&ObjectAdapter::deactivate));
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
IceInternal::ObjectAdapterFactory::proxyToServant(const ObjectPrx& proxy)
{
    if (_adapters.empty())
    {
	//
	// If there are no adapters at all, no endpoint can ever be
	// local, and no object can exist locally. We indicate this by
	// returning null to the caller.
	//
	return 0;	 
    }

    bool allEndpointsLocal = true;

    for (map<string, ObjectAdapterPtr>::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	try
	{
	    ObjectPtr servant = p->second->proxyToServant(proxy);
	    if (servant)
	    {
		//
		// Servant found, return it to the caller
		//
		return servant;
	    }
	}
	catch(const WrongObjectAdapterException&)
	{
	    //
	    // A WrongObjectAdapter exception indicates that there is
	    // no endpoint from the proxy that matches at least one
	    // endpoint from the object adapter. That means that we
	    // have at least one non-local endpoint in the proxy.
	    //
	    allEndpointsLocal = false;
	}
    }

    if (allEndpointsLocal)
    {
	//
	// If we didn't find a servant even though all endpoints are
	// local, we throw an ObjectNotExistException.
	//
	throw ObjectNotExistException(__FILE__, __LINE__);
    }
    else
    {
	//
	// If at least one of the endpoints from the proxy are not
	// local, it might be possible that the object exists
	// remotely. We indicate this by returning null to the caller.
	//
	return 0;
    }
}

IceInternal::ObjectAdapterFactory::ObjectAdapterFactory(const InstancePtr& instance) :
    _instance(instance)
{
}
