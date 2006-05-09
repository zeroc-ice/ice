// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceStorm/OnewayBatchSubscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Flusher.h>

using namespace IceStorm;
using namespace std;

OnewayBatchSubscriber::OnewayBatchSubscriber(const SubscriberFactoryPtr& factory,
	                                     const Ice::CommunicatorPtr& communicator,
					     const TraceLevelsPtr& traceLevels,
                                             const FlusherPtr& flusher,
					     const QueuedProxyPtr& obj) :
    OnewaySubscriber(factory, communicator, traceLevels, obj),
    _flusher(flusher)
{
}

OnewayBatchSubscriber::~OnewayBatchSubscriber()
{
}

void
OnewayBatchSubscriber::activate()
{
    OnewaySubscriber::activate();

    //
    // Delay adding this object to the flusher until it is activated. This ensures
    // that any subscriber with the same identity is first removed from the flusher.
    //
    _flusher->add(this);
}

void
OnewayBatchSubscriber::unsubscribe()
{
    {
	IceUtil::Mutex::Lock sync(_stateMutex);
	_state = StateUnsubscribed;
    }

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Unsubscribe " << _communicator->identityToString(id());
    }

    //
    // If this subscriber has been registered with the flusher then
    // remove it.
    //
    _flusher->remove(this);
}

void
OnewayBatchSubscriber::replace()
{
    {
	IceUtil::Mutex::Lock sync(_stateMutex);
	_state = StateReplaced;
    }

    if(_traceLevels->subscriber > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->subscriberCat);
	out << "Replace " << _communicator->identityToString(id());
    }

    //
    // If this subscriber has been registered with the flusher then
    // remove it.
    //
    _flusher->remove(this);
}

bool
OnewayBatchSubscriber::inactive() const
{
    return OnewaySubscriber::inactive();
}

void
OnewayBatchSubscriber::flush()
{
    _communicator->flushBatchRequests();
}

bool
OnewayBatchSubscriber::operator==(const Flushable& therhs) const
{
    const OnewayBatchSubscriber* rhs = dynamic_cast<const OnewayBatchSubscriber*>(&therhs);
    if(rhs != 0)
    {
	return id() == rhs->id();
    }
    return false;
}
