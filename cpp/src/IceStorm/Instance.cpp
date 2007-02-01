// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/Instance.h>
#include <IceStorm/BatchFlusher.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/SubscriberPool.h>

#include <Ice/Communicator.h>
#include <Ice/Properties.h>

using namespace IceStorm;
using namespace std;

Instance::Instance(
    const string& instanceName,
    const string& name,
    const Ice::CommunicatorPtr& communicator,
    const Ice::ObjectAdapterPtr& adapter) :
    _instanceName(instanceName),
    _communicator(communicator),
    _adapter(adapter),
    _traceLevels(new TraceLevels(name, communicator->getProperties(), communicator->getLogger())),
    _discardInterval(IceUtil::Time::seconds(communicator->getProperties()->getPropertyAsIntWithDefault(
                                                "IceStorm.Discard.Interval", 60))), // default one minute.
    // default one minute.
    _sendTimeout(communicator->getProperties()->getPropertyAsIntWithDefault("IceStorm.Send.Timeout", 60 * 1000))
{
    try
    {
        __setNoDelete(true);

        _batchFlusher = new BatchFlusher(this);
        _subscriberPool = new SubscriberPool(this);
    }
    catch(...)
    {
        shutdown();
        __setNoDelete(false);

        throw;
    }
    __setNoDelete(false);
}

Instance::~Instance()
{
}

string
Instance::instanceName() const
{
    return _instanceName;
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

IceUtil::Time
Instance::discardInterval() const
{
    return _discardInterval;
}

int
Instance::sendTimeout() const
{
    return _sendTimeout;
}

void
Instance::shutdown()
{
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
