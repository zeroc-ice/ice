// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObjectAdapterFactory.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/Object.h>
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
    list<ObjectAdapterIPtr> adapters;

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
    for_each(adapters.begin(), adapters.end(), IceUtil::voidMemFun(&ObjectAdapter::deactivate));
}

void
IceInternal::ObjectAdapterFactory::waitForShutdown()
{
    list<ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        //
        // First we wait for the shutdown of the factory itself.
        //
        while(_instance)
        {
            wait();
        }

        adapters = _adapters;
    }

    //
    // Now we wait for deactivation of each object adapter.
    //
    for_each(adapters.begin(), adapters.end(), IceUtil::voidMemFun(&ObjectAdapter::waitForDeactivate));
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

    list<ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        adapters = _adapters;
    }

    //
    // Now we destroy each object adapter.
    //
    for_each(adapters.begin(), adapters.end(), IceUtil::voidMemFun(&ObjectAdapter::destroy));

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        _adapters.clear();
    }
}

void
IceInternal::ObjectAdapterFactory::updateObservers(void (ObjectAdapterI::*fn)())
{
    list<ObjectAdapterIPtr> adapters;

    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);
        adapters = _adapters;
    }

    for_each(adapters.begin(), adapters.end(), IceUtil::voidMemFun(fn));
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::createObjectAdapter(const string& name, const RouterPrx& router)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(!_instance)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    ObjectAdapterIPtr adapter;
    if(name.empty())
    {
        string uuid = IceUtil::generateUUID();
        adapter = new ObjectAdapterI(_instance, _communicator, this, uuid, true);
        adapter->initialize(0);
    }
    else
    {
        if(_adapterNamesInUse.find(name) != _adapterNamesInUse.end())
        {
            throw AlreadyRegisteredException(__FILE__, __LINE__, "object adapter", name);
        }
        adapter = new ObjectAdapterI(_instance, _communicator, this, name, false);
        adapter->initialize(router);
        _adapterNamesInUse.insert(name);
    }

    _adapters.push_back(adapter);
    return adapter;
}

ObjectAdapterPtr
IceInternal::ObjectAdapterFactory::findObjectAdapter(const ObjectPrx& proxy)
{
    list<ObjectAdapterIPtr> adapters;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        if(!_instance)
        {
            return 0;
        }

        adapters = _adapters;
    }

    for(list<ObjectAdapterIPtr>::iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        try
        {
            if((*p)->isLocal(proxy))
            {
                return *p;
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
IceInternal::ObjectAdapterFactory::removeObjectAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

    if(!_instance)
    {
        return;
    }

    for(list<ObjectAdapterIPtr>::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
        if(*p == adapter)
        {
            _adapters.erase(p);
            break;
        }
    }
    _adapterNamesInUse.erase(adapter->getName());
}

void
IceInternal::ObjectAdapterFactory::flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr& outAsync) const
{
    list<ObjectAdapterIPtr> adapters;
    {
        IceUtil::Monitor<IceUtil::RecMutex>::Lock sync(*this);

        adapters = _adapters;
    }

    for(list<ObjectAdapterIPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        (*p)->flushAsyncBatchRequests(outAsync);
    }
}

IceInternal::ObjectAdapterFactory::ObjectAdapterFactory(const InstancePtr& instance,
                                                        const CommunicatorPtr& communicator) :
    _instance(instance),
    _communicator(communicator)
{
}

IceInternal::ObjectAdapterFactory::~ObjectAdapterFactory()
{
    assert(!_instance);
    assert(!_communicator);
    assert(_adapters.empty());
}
