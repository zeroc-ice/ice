// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>

#include <IceStorm/OnewaySubscriber.h>
#include <IceStorm/TraceLevels.h>

using namespace IceStorm;
using namespace std;

OnewaySubscriber::OnewaySubscriber(const TraceLevelsPtr& traceLevels, const Ice::ObjectPrx& obj) :
    Subscriber(traceLevels, obj->ice_getIdentity()),
    _obj(obj)
{
}

OnewaySubscriber::~OnewaySubscriber()
{
}

bool
OnewaySubscriber::persistent() const
{
    return false;
}

void
OnewaySubscriber::unsubscribe()
{
    IceUtil::Mutex::Lock sync(_stateMutex);
    _state = StateUnsubscribed;

    if (_traceLevels->subscriber > 0)
    {
	ostringstream s;
	s << "Unsubscribe " << _obj->ice_getIdentity();
	_traceLevels->logger->trace(_traceLevels->subscriberCat, s.str());
    }
}

void
OnewaySubscriber::publish(const Event& event)
{
    try
    {
	std::vector< ::Ice::Byte> dummy;
	_obj->ice_invoke(event.op, event.nonmutating, event.data, dummy, event.context);
    }
    catch(const Ice::LocalException& e)
    {
	IceUtil::Mutex::Lock sync(_stateMutex);
	//
	// It's possible that the subscriber was unsubscribed, or
	// marked invalid by another thread. Don't display a
	// diagnostic in this case.
	//
	if (_state == StateActive)
	{
	    if (_traceLevels->subscriber > 0)
	    {
		ostringstream s;
		s << _obj->ice_getIdentity() << ": publish failed: " << e;
		_traceLevels->logger->trace(_traceLevels->subscriberCat, s.str());
	    }
	    _state = StateError;
	}
    }

}
