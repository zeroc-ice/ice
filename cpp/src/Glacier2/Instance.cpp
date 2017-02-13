// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/SessionRouterI.h>
#include <Glacier2/Instance.h>
#include <Glacier2/InstrumentationI.h>
#include <Ice/InstrumentationI.h>

using namespace std;
using namespace Glacier2;

namespace
{

const string serverSleepTime = "Glacier2.Server.SleepTime";
const string clientSleepTime = "Glacier2.Client.SleepTime";
const string serverBuffered = "Glacier2.Server.Buffered";
const string clientBuffered = "Glacier2.Client.Buffered";

}

Glacier2::Instance::Instance(const Ice::CommunicatorPtr& communicator, const Ice::ObjectAdapterPtr& clientAdapter, 
                             const Ice::ObjectAdapterPtr& serverAdapter) :
    _communicator(communicator),
    _properties(communicator->getProperties()),
    _logger(communicator->getLogger()),
    _clientAdapter(clientAdapter),
    _serverAdapter(serverAdapter)
{
    if(_properties->getPropertyAsIntWithDefault(serverBuffered, 1) > 0)
    {
        IceUtil::Time sleepTime = IceUtil::Time::milliSeconds(_properties->getPropertyAsInt(serverSleepTime));
        const_cast<RequestQueueThreadPtr&>(_serverRequestQueueThread) = new RequestQueueThread(sleepTime);
        try
        {
            _serverRequestQueueThread->start();
        }
        catch(const IceUtil::Exception&)
        {
            _serverRequestQueueThread->destroy();
            throw;
        }
    }

    if(_properties->getPropertyAsIntWithDefault(clientBuffered, 1) > 0)
    {
        IceUtil::Time sleepTime = IceUtil::Time::milliSeconds(_properties->getPropertyAsInt(clientSleepTime));
        const_cast<RequestQueueThreadPtr&>(_clientRequestQueueThread) = new RequestQueueThread(sleepTime);
        try
        {
            _clientRequestQueueThread->start();
        }
        catch(const IceUtil::Exception&)
        {
            _clientRequestQueueThread->destroy();
            throw;
        }
    }

    const_cast<ProxyVerifierPtr&>(_proxyVerifier) = new ProxyVerifier(communicator);

    //
    // If an Ice metrics observer is setup on the communicator, also
    // enable metrics for IceStorm.
    //
    IceInternal::CommunicatorObserverIPtr o = 
        IceInternal::CommunicatorObserverIPtr::dynamicCast(communicator->getObserver());
    if(o)
    {
        const_cast<Glacier2::Instrumentation::RouterObserverPtr&>(_observer) = 
            new RouterObserverI(o->getFacet(), 
                                _properties->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2"));
    }
}

Glacier2::Instance::~Instance()
{
}

void
Glacier2::Instance::destroy()
{
    if(_clientRequestQueueThread)
    {
        _clientRequestQueueThread->destroy();
    }
    
    if(_serverRequestQueueThread)
    {
        _serverRequestQueueThread->destroy();
    }

    const_cast<SessionRouterIPtr&>(_sessionRouter) = 0;
}

void
Glacier2::Instance::setSessionRouter(const SessionRouterIPtr& sessionRouter)
{
    const_cast<SessionRouterIPtr&>(_sessionRouter) = sessionRouter;
}
