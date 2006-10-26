// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>

#include <IceStorm/LinkProxy.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Event.h>

class LinkOrderedInvokeCB : public IceStorm::AMI_TopicLink_forward
{
public:

    LinkOrderedInvokeCB(const IceStorm::LinkProxyPtr& proxy) : _proxy(proxy)
    {
    }

    virtual void
    ice_response()
    {
	_proxy->response();
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
	}
	catch(const Ice::LocalException& ex)
	{
	    _proxy->exception(ex);
	}
    }

private:

    const IceStorm::LinkProxyPtr _proxy;
};

using namespace std;

IceStorm::LinkProxy::LinkProxy(const TraceLevelsPtr& traceLevels, const string& id, const TopicLinkPrx& obj) :
    _traceLevels(traceLevels),
    _id(id),
    _obj(obj),
    //
    // We'll use this proxy with AMI invocations later, so therefore
    // we must turn off the collocation optimization. I don't want to
    // store this in _obj as this proxy is used for comparison
    // purposes with proxies loaded from the Freeze database which
    // don't retain the collocation optimization flag (however, this
    // flag is used for proxy comparison purposes).
    //
    _objAMI(TopicLinkPrx::uncheckedCast(obj->ice_collocationOptimized(false)))
{
}

IceStorm::LinkProxy::~LinkProxy()
{
    assert(!_busy);
}

Ice::ObjectPrx
IceStorm::LinkProxy::proxy() const
{
    return _obj;
}

void
IceStorm::LinkProxy::publish(const EventPtr& event)
{
    vector<EventPtr> v;
    {
	IceUtil::Mutex::Lock sync(_mutex);
	
	if(_exception.get())
	{
	    _exception->ice_throw();
	}
	
	_events.push_back(event);
	
	if(_busy)
	{
	    return;
	}

	v.swap(_events);
	_busy = true;
    }

    try
    {
	deliver(v);
    }
    catch(const Ice::LocalException& ex)
    {
	exception(ex);
	throw;
    }
}

void
IceStorm::LinkProxy::deliver(const vector<EventPtr>& v)
{
    //
    // TODO: It would be nice to avoid all this copying. To do this we
    // could change EventData from a struct to a class and rename to
    // Event, and then remove EventPtr. However, this means EventData
    // would have some data that doesn't need to be passed over the
    // wire (forward/cost), and the marshaling of the data would be
    // marginally slower.
    //
    vector<EventData> events;
    for(vector<EventPtr>::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	EventData e;
	e.op = (*p)->op;
	e.mode = (*p)->mode;
	e.data = (*p)->data;
	e.context = (*p)->context;
	events.push_back(e);
    }
    _objAMI->forward_async(new LinkOrderedInvokeCB(this), events);
}

void
IceStorm::LinkProxy::response()
{
    vector<EventPtr> v;
    {
	IceUtil::Mutex::Lock sync(_mutex);
	
	assert(!_exception.get() && _busy);
	
	if(_events.empty())
	{
	    _busy = false;
	    return;
	}

	v.swap(_events);
    }

    try
    {
	deliver(v);
    }
    catch(const Ice::LocalException& ex)
    {
	exception(ex);
    }
}

void
IceStorm::LinkProxy::exception(const Ice::LocalException& ex)
{
    IceUtil::Mutex::Lock sync(_mutex);
    assert(!_exception.get() && _busy);
    _busy = false;
    _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
}
