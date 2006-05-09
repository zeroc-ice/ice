// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/OnewaySubscriber.h>
#include <IceStorm/SubscriberFactory.h>
#include <IceStorm/TraceLevels.h>

using namespace IceStorm;
using namespace std;

OnewaySubscriber::OnewaySubscriber(const SubscriberFactoryPtr& factory, const Ice::CommunicatorPtr& communicator,
				   const TraceLevelsPtr& traceLevels, const QueuedProxyPtr& obj) :
    Subscriber(traceLevels, obj->proxy()->ice_getIdentity()),
    _communicator(communicator),
    _factory(factory),
    _obj(obj)
{
    _factory->incProxyUsageCount(_obj);
}

OnewaySubscriber::~OnewaySubscriber()
{
    _factory->decProxyUsageCount(_obj);
}

bool
OnewaySubscriber::persistent() const
{
    return false;
}

void
OnewaySubscriber::activate()
{
    // Nothing to do
}

void
OnewaySubscriber::unsubscribe()
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    _state = StateUnsubscribed;

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Unsubscribe " << _communicator->identityToString(id());
    }
}

void
OnewaySubscriber::replace()
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    _state = StateReplaced;

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Replace " << _communicator->identityToString(id());
    }
}

void
OnewaySubscriber::publish(const EventPtr& event)
{
    try
    {
        _obj->publish(event);
    }
    catch(const Ice::LocalException& e)
    {
	IceUtil::Mutex::Lock sync(_stateMutex);

        //
        // It's possible that the subscriber was unsubscribed, or
        // marked invalid by another thread. Don't display a
        // diagnostic in this case.
        //
        if(_state == StateActive)
        {
            if(_traceLevels->subscriber > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
                out << _communicator->identityToString(id()) << ": publish failed: " << e;
            }
            _state = StateError;
        }
    }
}
