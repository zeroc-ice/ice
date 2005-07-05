// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ObjectAdapterFactory.h>
#include <IceE/ObjectAdapter.h>
#include <IceE/LocalException.h>
#include <IceE/Functional.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(ObjectAdapterFactory* p) { p->__incRef(); }
void IceEInternal::decRef(ObjectAdapterFactory* p) { p->__decRef(); }

void
IceEInternal::ObjectAdapterFactory::shutdown()
{
    map<string, ObjectAdapterPtr> adapters;

    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	
	//
	// Ignore shutdown requests if the object adapter factory has
	// already been shut down.
	//
	if(!_instance)
	{
	    return;
	}
	
	adapters = _adapters;
	
	_instance = 0;
	_communicator = 0;
	
	notifyAll();
    }
    
    //
    // Deactivate outside the thread synchronization, to avoid
    // deadlocks.
    //
    //for_each(adapters.begin(), adapters.end(),
	     //IceE::secondVoidMemFun<const string, ObjectAdapter>(&ObjectAdapter::deactivate));
    for(map<string, ObjectAdapterPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	p->second->deactivate();
    }

}

void
IceEInternal::ObjectAdapterFactory::waitForShutdown()
{
    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	
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
    //for_each(_adapters.begin(), _adapters.end(),
	     //IceE::secondVoidMemFun<const string, ObjectAdapter>(&ObjectAdapter::waitForDeactivate));
    for(map<string, ObjectAdapterPtr>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	p->second->waitForDeactivate();
    }
    
    //
    // We're done, now we can throw away the object adapters.
    //
    _adapters.clear();

    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);

	//
	// Signal that waiting is complete.
	//
	_waitForShutdown = false;
	notifyAll();
    }
}

ObjectAdapterPtr
IceEInternal::ObjectAdapterFactory::createObjectAdapter(const string& name)
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);

    if(!_instance)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    map<string, ObjectAdapterPtr>::iterator p = _adapters.find(name);
    if(p != _adapters.end())
    {
	return p->second;
    }

    ObjectAdapterPtr adapter = new ObjectAdapter(_instance, _communicator, name);
    _adapters.insert(make_pair(name, adapter));
    return adapter;
}

ObjectAdapterPtr
IceEInternal::ObjectAdapterFactory::findObjectAdapter(const ObjectPrx& proxy)
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);

    if(!_instance)
    {
	return 0;
    }

    for(map<string, ObjectAdapterPtr>::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
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

namespace IceEInternal {

struct FlushAdapter
{
    void operator() (ObjectAdapterPtr p)
    {
	p->flushBatchRequests();
    }
};

}

void
IceEInternal::ObjectAdapterFactory::flushBatchRequests() const
{
    list<ObjectAdapterPtr> a;
    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);

	for(map<string, ObjectAdapterPtr>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
	{
	    a.push_back(p->second);
	}
    }
    for_each(a.begin(), a.end(), FlushAdapter());
}

IceEInternal::ObjectAdapterFactory::ObjectAdapterFactory(const InstancePtr& instance,
							const CommunicatorPtr& communicator) :
    _instance(instance),
    _communicator(communicator),
    _waitForShutdown(false)
{
}

IceEInternal::ObjectAdapterFactory::~ObjectAdapterFactory()
{
    assert(!_instance);
    assert(!_communicator);
    assert(_adapters.empty());
    assert(!_waitForShutdown);
}
