// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObjectAdapterFactory.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/LocalException.h>
#include <Ice/Functional.h>
#include <IceUtil/UUID.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ObjectAdapterFactory* p) { return p; }

void
IceInternal::ObjectAdapterFactory::shutdown()
{
    map<string, ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        
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
    for_each(adapters.begin(), adapters.end(),
             IceUtil::secondVoidMemFun<const string, ObjectAdapterI>(&ObjectAdapter::deactivate));
}

void
IceInternal::ObjectAdapterFactory::waitForShutdown()
{
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        
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
             IceUtil::secondVoidMemFun<const string, ObjectAdapterI>(&ObjectAdapter::waitForDeactivate));
    
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

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
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    return _instance == 0;
}

void
IceInternal::ObjectAdapterFactory::destroy()
{
    //
    // First wait for shutdown to finish.
    //
    waitForShutdown();

    map<string, ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        adapters = _adapters;
        _adapters.clear();
    }

    //
    // Now we destroy each object adapter.
    //
    for_each(adapters.begin(), adapters.end(),
             IceUtil::secondVoidMemFun<const string, ObjectAdapterI>(&ObjectAdapter::destroy));
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::createObjectAdapter(const string& name, const string& endpoints, 
                                                       const RouterPrx& router)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(!_instance)
    {
        throw ObjectAdapterDeactivatedException(__FILE__, __LINE__);
    }

    map<string, ObjectAdapterIPtr>::iterator p = _adapters.find(name);
    if(p != _adapters.end())
    {
        throw AlreadyRegisteredException(__FILE__, __LINE__, "object adapter", name);
    }

    if(name.empty() && (!endpoints.empty() || router != 0))
    {
        InitializationException ex(__FILE__, __LINE__);
        ex.reason = "Cannot configure endpoints or router with nameless object adapter";
        throw ex;
    }

    ObjectAdapterIPtr adapter;
    if(name.empty())
    {
        string uuid = IceUtil::generateUUID();
        adapter = new ObjectAdapterI(_instance, _communicator, this, uuid, "", 0, true);
        _adapters.insert(make_pair(uuid, adapter));
    }
    else
    {
        adapter = new ObjectAdapterI(_instance, _communicator, this, name, endpoints, router, false);
        _adapters.insert(make_pair(name, adapter));
    }
    return adapter;
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::findObjectAdapter(const ObjectPrx& proxy)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(!_instance)
    {
        return 0;
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

void
IceInternal::ObjectAdapterFactory::removeObjectAdapter(const string& name)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(_waitForShutdown)
    {
        return;
    }

    _adapters.erase(name);
}

namespace IceInternal {

struct FlushAdapter
{
    void operator() (ObjectAdapterIPtr p)
    {
        p->flushBatchRequests();
    }
};

}

void
IceInternal::ObjectAdapterFactory::flushBatchRequests() const
{
    list<ObjectAdapterIPtr> a;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        for(map<string, ObjectAdapterIPtr>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
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
