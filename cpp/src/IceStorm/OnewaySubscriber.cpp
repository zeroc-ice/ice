// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/OnewaySubscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Event.h>
#include <IceStorm/QueuedProxy.h>

#include <Ice/ObjectAdapter.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>

using namespace IceStorm;
using namespace std;

namespace IceStorm
{

class PerSubscriberPublisherProxyI : public Ice::BlobjectArray
{
public:

    PerSubscriberPublisherProxyI(const SubscriberPtr& subscriber) :
	_subscriber(subscriber)
    {
    }

    ~PerSubscriberPublisherProxyI()
    {
    }

    virtual bool
    ice_invoke(const pair<const Ice::Byte*, const Ice::Byte*>& inParams, vector< Ice::Byte>& outParam,
	       const Ice::Current& current)
    {
	const Ice::Context& context = current.ctx;

	EventPtr event = new Event;
	event->forwarded = false;
	Ice::Context::const_iterator p = context.find("cost");
	if(p != context.end())
	{
	    event->cost = atoi(p->second.c_str());
	}
	else
	{
	    event->cost = 0; // TODO: Default comes from property?
	}
	event->op = current.operation;
	event->mode = current.mode;
	vector<Ice::Byte>(inParams.first, inParams.second).swap(event->data);
	event->context = context;

	_subscriber->publish(event);

	return true;
    }

private:

    //
    // Set of associated subscribers
    //
    const SubscriberPtr _subscriber;
};

}

OnewaySubscriber::OnewaySubscriber(const SubscriberFactoryPtr& factory, const Ice::CommunicatorPtr& communicator,
				   const TraceLevelsPtr& traceLevels, const Ice::ObjectAdapterPtr& adapter,
				   const QueuedProxyPtr& obj) :
    Subscriber(factory, communicator, traceLevels, obj),
    _adapter(adapter)
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
OnewaySubscriber::activate()
{
    _proxy = _adapter->addWithUUID(new PerSubscriberPublisherProxyI(this));
}

void
OnewaySubscriber::unsubscribe()
{
    Subscriber::unsubscribe();

    //
    // Clear the per-subscriber object.
    //
    try
    {
	_adapter->remove(_proxy->ice_getIdentity());
    }
    catch(const Ice::NotRegisteredException&)
    {
	// Ignore
    }
}

void
OnewaySubscriber::replace()
{
    Subscriber::replace();

    //
    // Clear the per-subscriber object.
    //
    try
    {
	_adapter->remove(_proxy->ice_getIdentity());
    }
    catch(const Ice::NotRegisteredException&)
    {
	// Ignore
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
                out << _desc << ": publish failed: " << e;
            }
            _state = StateError;

	    //
	    // Clear the per-subscriber object.
	    //
	    try
	    {
		_adapter->remove(_proxy->ice_getIdentity());
	    }
	    catch(const Ice::NotRegisteredException&)
	    {
		// Ignore
	    }
        }
    }
}

Ice::ObjectPrx
OnewaySubscriber::proxy() const
{
    assert(_proxy);
    return _proxy;
}
