// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/SubscriberFactory.h>
#include <IceStorm/QueuedProxy.h>

#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>

using namespace IceStorm;
using namespace std;

Subscriber::Subscriber( const SubscriberFactoryPtr& factory,
			const Ice::CommunicatorPtr& communicator,
			const TraceLevelsPtr& traceLevels,
			const QueuedProxyPtr& obj) :
    _factory(factory),
    _desc(communicator->identityToString(obj->proxy()->ice_getIdentity())),
    _traceLevels(traceLevels),
    _obj(obj),
    _state(StateActive)
{
    _factory->incProxyUsageCount(_obj);
}

Subscriber::~Subscriber()
{
    _factory->decProxyUsageCount(_obj);
}

//
// A subscriber is inactive if the state is not active and its not
// unreachable. Link subscribers go into the unreachable state if they
// are temporarily unreachable.
//
bool
Subscriber::inactive() const
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    return _state != StateActive && _state != StateUnreachable;
}

void
Subscriber::activate()
{
}

void
Subscriber::unsubscribe()
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    _state = StateUnsubscribed;

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Unsubscribe " << _desc;
    }
}

void
Subscriber::replace()
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    _state = StateReplaced;

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Replace " << _desc;
    }
}

bool
Subscriber::error() const
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    return _state == StateError;
}

Ice::Identity
Subscriber::id() const
{
    return _obj->proxy()->ice_getIdentity();
}
