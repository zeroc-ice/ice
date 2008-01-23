// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/Instance.h>

using namespace std;
using namespace Glacier2;

static const string serverSleepTime = "Glacier2.Server.SleepTime";
static const string clientSleepTime = "Glacier2.Client.SleepTime";
static const string serverBuffered = "Glacier2.Server.Buffered";
static const string clientBuffered = "Glacier2.Client.Buffered";

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
        _serverRequestQueueThread->start();
    }

    if(_properties->getPropertyAsIntWithDefault(clientBuffered, 1) > 0)
    {
        IceUtil::Time sleepTime = IceUtil::Time::milliSeconds(_properties->getPropertyAsInt(clientSleepTime));
        const_cast<RequestQueueThreadPtr&>(_clientRequestQueueThread) = new RequestQueueThread(sleepTime);
        _clientRequestQueueThread->start();
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
}
