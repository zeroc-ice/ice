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
#include <Ice/LocalException.h>
#include <Ice/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(ObjectAdapterFactory* p) { p->__incRef(); }
void IceInternal::decRef(ObjectAdapterFactory* p) { p->__decRef(); }

void
IceInternal::ObjectAdapterFactory::shutdown()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // Ignore shutdown requests if the object adapter factory has
    // already been shut down.
    //
    if(!_instance)
    {
	return;
    }
    
    for_each(_adapters.begin(), _adapters.end(),
	     Ice::secondVoidMemFun<string, ObjectAdapter>(&ObjectAdapter::deactivate));
    
    _instance = 0;
    _communicator = 0;

    notifyAll();
}

void
IceInternal::ObjectAdapterFactory::waitForShutdown()
{
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
	//
	// First we wait for the shutdown of the factory itself.
	//
	while(_instance)
	{
	    wait();
	}

	//
	// If some other thread is currently shutting down, we wait
	// until this thread is finished.
	//
	while(_waitForShutdown)
	{
	    wait();
	}
	_waitForShutdown = true;
    }
    
    //
    // Now we wait for deactivation of each object adapter.
    //
    for_each(_adapters.begin(), _adapters.end(),
	     Ice::secondVoidMemFun<string, ObjectAdapter>(&ObjectAdapter::waitForDeactivate));
    
    //
    // We're done, now we can throw away the object adapters.
    //
    _adapters.clear();

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	//
	// Signal that waiting is complete.
	//
	_waitForShutdown = false;
	notifyAll();
    }
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::createObjectAdapter(const string& name, const string& endpts, const string& id)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

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
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    for(map<string, ObjectAdapterIPtr>::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	try
	{
	    if(p->second->isLocal(proxy))
	    {
		return p->second;
	    }
	}
	catch(const ObjectAdapterDeactivatedException&)
	{
	    // Ignore.
	}
    }

    return 0;
}

IceInternal::ObjectAdapterFactory::ObjectAdapterFactory(const InstancePtr& instance,
							const CommunicatorPtr& communicator) :
    _instance(instance),
    _communicator(communicator),
    _waitForShutdown(false)
{
}

IceInternal::ObjectAdapterFactory::~ObjectAdapterFactory()
{
    assert(!_instance);
    assert(!_communicator);
    assert(_adapters.empty());
    assert(!_waitForShutdown);
}
