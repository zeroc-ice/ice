// **********************************************************************
//
// Copyright (c) 2001
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
#include <IceStorm/TraceLevels.h>

using namespace IceStorm;
using namespace std;

LinkSubscriber::LinkSubscriber(const TraceLevelsPtr& traceLevels, const TopicLinkPrx& obj, Ice::Int cost) :
    Subscriber(traceLevels, obj->ice_getIdentity()),
    _obj(TopicLinkPrx::uncheckedCast(obj->ice_batchOneway())),
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
LinkSubscriber::unsubscribe()
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    _state = StateUnsubscribed;

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Unsubscribe " << _obj->ice_getIdentity();
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
	out << "Replace " << _obj->ice_getIdentity();
    }
}

void
LinkSubscriber::publish(const Event& event)
{
    //
    // Don't forward forwarded, or more costly events.
    //
    if(event.forwarded || event.cost > _cost)
    {
	return;
    }

    try
    {
	_obj->forward(event.op, event.mode, event.data, event.context);
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
	    out << _obj->ice_getIdentity() << ": link topic publish failed: " << e;
	}
    }
    catch(const Ice::LocalException& e)
    {
	if(_traceLevels->subscriber > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	    out << _obj->ice_getIdentity() << ": link topic publish failed: " << e;
	}
    }
}

void
LinkSubscriber::flush()
{
    try
    {
	_obj->ice_flush();
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
	    out << _obj->ice_getIdentity() << ": link topic flush failed: " << e;
	}
    }
    catch(const Ice::LocalException& e)
    {
	if(_traceLevels->subscriber > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	    out << _obj->ice_getIdentity() << ": link topic flush failed: " << e;
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
