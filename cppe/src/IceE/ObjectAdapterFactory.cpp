// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ObjectAdapterFactory.h>
#include <IceE/ObjectAdapter.h>
#include <IceE/LocalException.h>
#include <IceE/Functional.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ObjectAdapterFactory* p) { return p; }

void
IceInternal::ObjectAdapterFactory::shutdown()
{
    map<string, ObjectAdapterPtr> adapters;

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
	     //Ice::secondVoidMemFun<const string, ObjectAdapter>(&ObjectAdapter::deactivate));
    for(map<string, ObjectAdapterPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	p->second->deactivate();
    }

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
    for(map<string, ObjectAdapterPtr>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	p->second->waitForDeactivate();
    }
    
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	//
	// Signal that waiting is complete.
	//
	_waitForShutdown = false;
	notifyAll();
    }
}

bool
IceInternal::ObjectAdapterFactory::isShutdown() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    return _instance == 0;
}

void
IceInternal::ObjectAdapterFactory::destroy()
{
    //
    // First wait for shutdown to finish.
    //
    waitForShutdown();

    map<string, ObjectAdapterPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        adapters = _adapters;
        _adapters.clear();
    }

    //
    // Now we destroy each object adapter.
    //
    for(map<string, ObjectAdapterPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	p->second->destroy();
    }
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::createObjectAdapter(const string& name, const string& endpoints
#ifdef ICEE_HAS_ROUTER
						       , const RouterPrx& router
#endif
						      )
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(!_instance)
    {
	throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    map<string, ObjectAdapterPtr>::iterator p = _adapters.find(name);
    if(p != _adapters.end())
    {
	throw AlreadyRegisteredException(__FILE__, __LINE__, "object adapter", name);
    }

    ObjectAdapterPtr adapter = new ObjectAdapter(_instance, _communicator, this, name, endpoints
#ifdef ICEE_HAS_ROUTER
						 , router
#endif
    						);
    _adapters.insert(make_pair(name, adapter));
    return adapter;
}

void
IceInternal::ObjectAdapterFactory::removeObjectAdapter(const string& name)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_waitForShutdown)
    {
        return;
    }

    _adapters.erase(name);
}

namespace IceInternal {

struct FlushAdapter
{
    void operator() (ObjectAdapterPtr p)
    {
	p->flushBatchRequests();
    }
};

}

void
IceInternal::ObjectAdapterFactory::flushBatchRequests() const
{
    list<ObjectAdapterPtr> a;
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

	for(map<string, ObjectAdapterPtr>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
	{
	    a.push_back(p->second);
	}
    }
    for_each(a.begin(), a.end(), FlushAdapter());
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
