// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/OutgoingAsync.h> // for Ice::AMI_Object_ice_invoke
#include <IceStorm/TwowayProxy.h>

using namespace std;

IceStorm::TwowayProxy::TwowayProxy(const Ice::ObjectPrx& obj) :
    _obj(obj)
{
}

Ice::ObjectPrx
IceStorm::TwowayProxy::proxy() const
{
    return _obj;
}

class UnorderedInvokeCB : public Ice::AMI_Object_ice_invoke
{
public:

    UnorderedInvokeCB(const IceStorm::UnorderedTwowayProxyPtr& proxy) : _proxy(proxy)
    {
    }

    ~UnorderedInvokeCB()
    {
    }
    
    virtual void
    ice_response(bool, const std::vector<Ice::Byte>&)
    {
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

    const IceStorm::UnorderedTwowayProxyPtr _proxy;
};

IceStorm::UnorderedTwowayProxy::UnorderedTwowayProxy(const Ice::ObjectPrx& obj) : 
    TwowayProxy(obj)
{
}

void
IceStorm::UnorderedTwowayProxy::exception(const Ice::LocalException& ex)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
}

void
IceStorm::UnorderedTwowayProxy::deliver(const vector<EventPtr>& v)
{
    //
    // TODO: Use a buffer of AMI callback objects to eliminate the dynamic memory allocation?
    //
    for(vector<EventPtr>::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	_obj->ice_invoke_async(new UnorderedInvokeCB(this), (*p)->op, (*p)->mode, (*p)->data, (*p)->context);
    }
}

class OrderedInvokeCB : public Ice::AMI_Object_ice_invoke
{
public:

    OrderedInvokeCB(const IceStorm::OrderedTwowayProxyPtr& proxy) : _proxy(proxy)
    {
    }

    ~OrderedInvokeCB()
    {
    }
    
    virtual void
    ice_response(bool, const std::vector<Ice::Byte>&)
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

    const IceStorm::OrderedTwowayProxyPtr _proxy;
};

IceStorm::OrderedTwowayProxy::OrderedTwowayProxy(const Ice::ObjectPrx& obj) : 
    TwowayProxy(obj)
{
}

void
IceStorm::OrderedTwowayProxy::publish(const EventPtr& event)
{
    EventPtr e;
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

	e = _events.front();
	_events.erase(_events.begin());
	_busy = true;
    }

    try
    {
	assert(e);
	vector<EventPtr> events;
	events.push_back(e);
	deliver(events);
    }
    catch(const Ice::LocalException& ex)
    {
	exception(ex);
	throw;
    }
}

void
IceStorm::OrderedTwowayProxy::exception(const Ice::LocalException& ex)
{
    IceUtil::Mutex::Lock sync(_mutex);
    assert(!_exception.get() && _busy);
    _busy = false;
    _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
}

void
IceStorm::OrderedTwowayProxy::response()
{
    EventPtr event;
    {
	IceUtil::Mutex::Lock sync(_mutex);
	
	assert(!_exception.get() && _busy);
	
	if(_events.empty())
	{
	    _busy = false;
	    return;
	}

	event = _events.front();
	_events.erase(_events.begin());
    }

    try
    {
	assert(event);
	vector<EventPtr> events;
	events.push_back(event);
	deliver(events);
    }
    catch(const Ice::LocalException& ex)
    {
	exception(ex);
    }
}

void
IceStorm::OrderedTwowayProxy::deliver(const vector<EventPtr>& v)
{
    //
    // Should only ever be called with a queue of length 1 (should
    // only be called by methods local to OrderedTwowayProxy).
    //
    assert(v.size() == 1);

    //
    // TODO: Use a buffer of AMI callback objects to eliminate the
    // dynamic memory allocation? (we could actually use only 2 AMI
    // callback objects there.)
    //
    for(vector<EventPtr>::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	_obj->ice_invoke_async(new OrderedInvokeCB(this), (*p)->op, (*p)->mode, (*p)->data, (*p)->context);
    }
}

