// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/LinkSubscriber.h>
#include <IceStorm/SubscriberFactory.h>
#include <IceStorm/TraceLevels.h>

using namespace IceStorm;
using namespace std;

LinkSubscriber::LinkSubscriber(const SubscriberFactoryPtr& factory, const TraceLevelsPtr& traceLevels,
                               const QueuedProxyPtr& obj, Ice::Int cost) :
    Subscriber(traceLevels, obj->proxy()->ice_getIdentity()),
    _factory(factory), _obj(obj), _cost(cost)
{
    _factory->incProxyUsageCount(_obj);
}

LinkSubscriber::~LinkSubscriber()
{
    _factory->decProxyUsageCount(_obj);
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
LinkSubscriber::unsubscribe()
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    _state = StateUnsubscribed;

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Unsubscribe " << id();
    }
}

void
LinkSubscriber::replace()
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    _state = StateReplaced;

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Replace " << id();
    }
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
	    out << id() << ": link topic publish failed: " << e;
	}
    }
    catch(const Ice::LocalException& e)
    {
	if(_traceLevels->subscriber > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	    out << id() << ": link topic publish failed: " << e;
	}
    }
}

void
LinkSubscriber::flush()
{
    try
    {
	_obj->proxy()->ice_flush();
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
	    out << id() << ": link topic flush failed: " << e;
	}
    }
    catch(const Ice::LocalException& e)
    {
	if(_traceLevels->subscriber > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	    out << id() << ": link topic flush failed: " << e;
	}
    }
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
