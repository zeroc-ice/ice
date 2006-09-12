// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/LinkSubscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Event.h>
#include <IceStorm/QueuedProxy.h>

#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/Communicator.h>

using namespace IceStorm;
using namespace std;

LinkSubscriber::LinkSubscriber(const SubscriberFactoryPtr& factory, const Ice::CommunicatorPtr& communicator,
			       const TraceLevelsPtr& traceLevels, const QueuedProxyPtr& obj, Ice::Int cost) :
    Subscriber(factory, communicator, traceLevels, obj),
    _communicator(communicator),
    _cost(cost)
{
}

LinkSubscriber::~LinkSubscriber()
{
}

bool
LinkSubscriber::persistent() const
{
    return true;
}

bool
LinkSubscriber::inactive() const
{
    return Subscriber::inactive();
}

void
LinkSubscriber::publish(const EventPtr& event)
{
    //
    // Don't propagate a message that has already been forwarded.
    // Also, if this link has a non-zero cost, then don't propagate
    // a message whose cost exceeds the link cost.
    //
    if(event->forwarded || (_cost > 0 && event->cost > _cost))
    {
	return;
    }

    try
    {
	_obj->publish(event);
    }
    catch(const Ice::ObjectNotExistException& e)
    {
	//
	// ObjectNotExist causes the link to be removed.
	//
	IceUtil::Mutex::Lock sync(_stateMutex);
	_state = StateError;

	if(_traceLevels->subscriber > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	    out << _desc << ": link topic publish failed: " << e;
	}
    }
    catch(const Ice::LocalException& e)
    {
	if(_traceLevels->subscriber > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	    out << _desc << ": link topic publish failed: " << e;
	}
    }
}

Ice::ObjectPrx
LinkSubscriber::proxy() const
{
    return Ice::ObjectPrx();
}

void
LinkSubscriber::flush()
{
    _communicator->flushBatchRequests();
}

bool
LinkSubscriber::operator==(const Flushable& therhs) const
{
    const LinkSubscriber* rhs = dynamic_cast<const LinkSubscriber*>(&therhs);
    if(rhs != 0)
    {
	return id() == rhs->id();
    }
    return false;
}
