// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/Instance.h>
#include <IceStorm/BatchFlusher.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/SubscriberPool.h>
#include <IceStorm/KeepAliveThread.h>

#include <Ice/Communicator.h>

using namespace IceStorm;
using namespace std;

Instance::Instance(
    const string& name,
    const Ice::CommunicatorPtr& communicator,
    const Ice::ObjectAdapterPtr& adapter) :
    _communicator(communicator),
    _adapter(adapter),
    _traceLevels(new TraceLevels(name, communicator->getProperties(), communicator->getLogger()))
{
    try
    {
	__setNoDelete(true);

	_batchFlusher = new BatchFlusher(this);
	_subscriberPool = new SubscriberPool(this);
	_keepAlive = new KeepAliveThread(this);
    }
    catch(...)
    {
	shutdown();
	destroy();
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
}

Instance::~Instance()
{
}

Ice::CommunicatorPtr
Instance::communicator() const
{
    return _communicator;
}

Ice::PropertiesPtr
Instance::properties() const
{
    return _communicator->getProperties();
}

Ice::ObjectAdapterPtr
Instance::objectAdapter() const
{
    return _adapter;
}

TraceLevelsPtr
Instance::traceLevels() const
{
    return _traceLevels;
}

KeepAliveThreadPtr
Instance::keepAlive() const
{
    return _keepAlive;
}

BatchFlusherPtr
Instance::batchFlusher() const
{
    return _batchFlusher;
}

SubscriberPoolPtr
Instance::subscriberPool() const
{
    return _subscriberPool;
}

void
Instance::shutdown()
{
    if(_keepAlive)
    {
	_keepAlive->destroy();
	_keepAlive->getThreadControl().join();
    }

    if(_batchFlusher)
    {
	_batchFlusher->destroy();
	_batchFlusher->getThreadControl().join();
    }

    if(_subscriberPool)
    {
	_subscriberPool->destroy();
    }
}

void
Instance::destroy()
{
    _keepAlive = 0;
    _batchFlusher = 0;
    _subscriberPool = 0;
}
