// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************


#include <IceStorm/LinkSubscriber.h>
#include <IceStorm/TraceLevels.h>
#include <Ice/Ice.h>


using namespace IceStorm;
using namespace std;

LinkSubscriber::LinkSubscriber(const TraceLevelsPtr& traceLevels, const TopicLinkPrx& obj, Ice::Int cost) :
    Subscriber(traceLevels, obj->ice_getIdentity()),
    _obj(TopicLinkPrx::checkedCast(obj->ice_batchOneway())),
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
    JTCSyncT<JTCMutex> sync(_stateMutex);
    _state = StateUnsubscribed;

    if (_traceLevels->subscriber > 0)
    {
	ostringstream s;
	s << "Unsubscribe " << _obj->ice_getIdentity();
	_traceLevels->logger->trace(_traceLevels->subscriberCat, s.str());
    }
}

void
LinkSubscriber::publish(const Event& event)
{
    //
    // Don't forward forwarded, or more costly events.
    //
    if (event.forwarded || event.cost > _cost)
    {
	return;
    }

    try
    {
	_obj->forward(event.op, event.nonmutating, event.data, event.context);
    }
    catch(const Ice::ObjectNotExistException& e)
    {
	//
	// ObjectNotExist causes the link to be removed.
	//
	JTCSyncT<JTCMutex> sync(_stateMutex);
	_state = StateError;

	if (_traceLevels->subscriber > 0)
	{
	    ostringstream s;
	    s << _obj->ice_getIdentity() << ": link topic publish failed: " << e;
	    _traceLevels->logger->trace(_traceLevels->subscriberCat, s.str());
	}
    }
    catch(const Ice::LocalException& e)
    {
	if (_traceLevels->subscriber > 0)
	{
	    ostringstream s;
	    s << _obj->ice_getIdentity() << ": link topic publish failed: " << e;
	    _traceLevels->logger->trace(_traceLevels->subscriberCat, s.str());
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
	JTCSyncT<JTCMutex> sync(_stateMutex);
	_state = StateError;

	if (_traceLevels->subscriber > 0)
	{
	    ostringstream s;
	    s << _obj->ice_getIdentity() << ": link topic flush failed: " << e;
	    _traceLevels->logger->trace(_traceLevels->subscriberCat, s.str());
	}
    }
    catch(const Ice::LocalException& e)
    {
	if (_traceLevels->subscriber > 0)
	{
	    ostringstream s;
	    s << _obj->ice_getIdentity() << ": link topic flush failed: " << e;
	    _traceLevels->logger->trace(_traceLevels->subscriberCat, s.str());
	}
    }
}

bool
LinkSubscriber::operator==(const Flushable& therhs) const
{
    const LinkSubscriber* rhs = dynamic_cast<const LinkSubscriber*>(&therhs);
    if (rhs != 0)
    {
	return id() == rhs->id();
    }
    return false;
}
