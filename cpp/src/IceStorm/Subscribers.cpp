// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/Subscribers.h>
#include <IceStorm/Instance.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/BatchFlusher.h>
#include <IceStorm/SubscriberPool.h>

#include <Ice/ObjectAdapter.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Connection.h>

#ifdef __BCPLUSPLUS__
#include <iterator>
#endif

using namespace std;
using namespace IceStorm;

//
// Per Subscriber object.
//
namespace
{

class PerSubscriberPublisherI : public Ice::BlobjectArray
{
public:

    PerSubscriberPublisherI(const InstancePtr& instance) :
	_instance(instance)
    {
    }

    ~PerSubscriberPublisherI()
    {
    }

    void
    setSubscriber(const SubscriberPtr& subscriber)
    {
	_subscriber = subscriber;
    }

    virtual bool
    ice_invoke(const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
	       vector<Ice::Byte>&,
	       const Ice::Current& current)
    {
	EventPtr event = new Event(
	    current.operation,
	    current.mode,
	    Ice::ByteSeq(),
	    current.ctx);

	//
	// COMPILERBUG: gcc 4.0.1 doesn't like this.
	//
	//event->data.swap(Ice::ByteSeq(inParams.first, inParams.second));
	Ice::ByteSeq data(inParams.first, inParams.second);
	event->data.swap(data);

	EventSeq e;
	e.push_back(event);
	Subscriber::QueueState state = _subscriber->queue(false, e);

	if(state == Subscriber::QueueStateFlush)
	{
	    list<SubscriberPtr> l;
	    l.push_back(_subscriber);
	    _instance->subscriberPool()->flush(l);
	}
	return true;
    }

private:

    const InstancePtr _instance;
    /*const*/ SubscriberPtr _subscriber;
};
typedef IceUtil::Handle<PerSubscriberPublisherI> PerSubscriberPublisherIPtr;

}
// Each of the various Subscriber types.
namespace
{

class SubscriberOneway : public Subscriber
{
public:

    SubscriberOneway(const InstancePtr&,
		     const Ice::ObjectPrx&,
		     const Ice::ObjectPrx&,
		     bool);
    //
    // Oneway
    //
    virtual bool flush();
    virtual void destroy();

private:
    const bool _batch;
    const Ice::ObjectPrx _obj;
    /*const*/ Ice::ObjectPrx _objBatch;
};

class SubscriberTwoway : public Subscriber
{
public:

    SubscriberTwoway(const InstancePtr&,
		     const Ice::ObjectPrx&,
		     const Ice::ObjectPrx&);

    virtual bool flush();

private:

    const Ice::ObjectPrx _obj;
};

//
// Twoway Ordered
//
class SubscriberTwowayOrdered : public Subscriber
{
public:

    SubscriberTwowayOrdered(const InstancePtr&,
			    const Ice::ObjectPrx&,
			    const Ice::ObjectPrx&);

    virtual bool flush();
    void response();

private:

    const Ice::ObjectPrx _obj;
};
typedef IceUtil::Handle<SubscriberTwowayOrdered> SubscriberTwowayOrderedPtr;

class SubscriberLink : public Subscriber
{
public:

    SubscriberLink(const InstancePtr&,
		   const TopicLinkPrx&,
		   int);

    virtual QueueState queue(bool, const std::vector<EventPtr>&);
    virtual bool flush();
    void response();

private:

    const TopicLinkPrx _obj;
    const int _cost;
};
typedef IceUtil::Handle<SubscriberLink> SubscriberLinkPtr;

}


SubscriberOneway::SubscriberOneway(
    const InstancePtr& instance,
    const Ice::ObjectPrx& proxy,
    const Ice::ObjectPrx& obj,
    bool batch) :
    Subscriber(instance, proxy, false, obj->ice_getIdentity()),
    _batch(batch),
    _obj(obj)
{
    _objBatch = obj->ice_isDatagram() ? _obj->ice_batchDatagram() : _obj->ice_batchOneway();

    if(batch)
    {
	_instance->batchFlusher()->add(_obj);
    }
}

bool
SubscriberOneway::flush()
{
    IceUtil::Mutex::Lock sync(_mutex);
    
    //
    // If the subscriber errored out then we're done.
    //
    if(_state != StateActive)
    {
	_busy = false;
	return false;
    }
    assert(!_events.empty());
    assert(_busy);

    try
    {
	//
	// Get the current set of events, but release the lock before
	// attempting to deliver the events. This allows other threads
	// to add events in case we block (such as during connection
	// establishment).
	//
	EventSeq v;
	v.swap(_events);
	sync.release();
	
	// XXX:
	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(_obj->ice_getIdentity().name.substr(0, 4) == "slow")
	{
	    //Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
	    //out << "deliberately stalling";
	    sleep(2);
	}
	if(_obj->ice_getIdentity().name.substr(0, 5) == "block")
	{
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
		out << "-> stall for 100s";
	    }
	    sleep(100);
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
		out << "<- stall for 100s";
	    }
	}

	//
	// Deliver the events without holding the lock.
	//
	// If there are more than one event queued and we are not in
	// batch sending mode then send the events as a batch and then
	// flush immediately, otherwise send one at a time.
	//
	vector<Ice::Byte> dummy;
	if(v.size() > 1 && !_batch)
	{
	    for(EventSeq::const_iterator p = v.begin(); p != v.end(); ++p)
	    {
		_objBatch->ice_invoke((*p)->op, (*p)->mode, (*p)->data, dummy, (*p)->context);
	    }
	    Ice::ConnectionPtr conn = _objBatch->ice_getCachedConnection();
	    assert(conn);
	    conn->flushBatchRequests();
	}
	else
	{
	    for(EventSeq::const_iterator p = v.begin(); p != v.end(); ++p)
	    {
		_obj->ice_invoke((*p)->op, (*p)->mode, (*p)->data, dummy, (*p)->context);
	    }
	}
	
	//
	// Reacquire the lock before we check the queue again.
	//
	sync.acquire();

	//
	// If there have been more events queued in the meantime then
	// we are still busy.
	//
	_busy = !_events.empty();
    }
    catch(const Ice::LocalException& ex)
    {
	assert(!sync.acquired());
	// setError will re-acquire and release the lock.
	setError(ex);
	return false;
    }

    return _busy;
}

void
SubscriberOneway::destroy()
{
    if(_batch)
    {
	_instance->batchFlusher()->remove(_obj);
    }
    Subscriber::destroy();
}

namespace
{

class TwowayInvokeI : public Ice::AMI_Object_ice_invoke
{
public:

    TwowayInvokeI(const SubscriberPtr& subscriber) :
	_subscriber(subscriber)
    {
    }

    virtual void
    ice_response(bool, const std::vector<Ice::Byte>&)
    {
    }

    virtual void
    ice_exception(const Ice::Exception& e)
    {
	_subscriber->setError(e);
    }

private:

    const SubscriberPtr _subscriber;
};

}

SubscriberTwoway::SubscriberTwoway(
    const InstancePtr& instance,
    const Ice::ObjectPrx& proxy,
    const Ice::ObjectPrx& obj) :
    Subscriber(instance, proxy, false, obj->ice_getIdentity()),
    _obj(obj)
{
}

bool
SubscriberTwoway::flush()
{
    IceUtil::Mutex::Lock sync(_mutex);

    //
    // If the subscriber errored out then we're done.
    //
    if(_state != StateActive)
    {
	_busy = false;
	return false;
    }
    assert(!_events.empty());
    assert(_busy);

    //
    // Get the current set of events, but release the lock before
    // attempting to deliver the events. This allows other threads
    // to add events in case we block (such as during connection
    // establishment).
    //
    EventSeq v;
    v.swap(_events);
    sync.release();

    //
    // Deliver the events without holding the lock.
    //
    for(EventSeq::const_iterator p = v.begin(); p != v.end(); ++p)
    {
	_obj->ice_invoke_async(new TwowayInvokeI(this), (*p)->op, (*p)->mode, (*p)->data, (*p)->context);
    }

    //
    // Reacquire the lock before we check the queue again.
    //
    sync.acquire();

    //
    // If there have been more events queued in the meantime then
    // we are still busy.
    //
    _busy = !_events.empty();

    return _busy;
}

namespace
{

class TwowayOrderedInvokeI : public Ice::AMI_Object_ice_invoke
{
public:

    TwowayOrderedInvokeI(const SubscriberTwowayOrderedPtr& subscriber) :
	_subscriber(subscriber)
    {
    }

    virtual void
    ice_response(bool, const std::vector<Ice::Byte>&)
    {
	_subscriber->response();
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
	_subscriber->setError(ex);
    }

private:

    const SubscriberTwowayOrderedPtr _subscriber;
};

}

SubscriberTwowayOrdered::SubscriberTwowayOrdered(
    const InstancePtr& instance,
    const Ice::ObjectPrx& proxy,
    const Ice::ObjectPrx& obj) :
    Subscriber(instance, proxy, false, obj->ice_getIdentity()),
    _obj(obj)
{
}

bool
SubscriberTwowayOrdered::flush()
{
    EventPtr e;
    {
	IceUtil::Mutex::Lock sync(_mutex);
	
	//
	// If the subscriber errored out then we're done.
	//
	if(_state != StateActive)
	{
	    _busy = false;
	    return false;
	}
	assert(!_events.empty());
	assert(_busy);
	
	e = _events.front();
	_events.erase(_events.begin());
    }
    
    _obj->ice_invoke_async(new TwowayOrderedInvokeI(this), e->op, e->mode, e->data, e->context);

    return false;
}

void
SubscriberTwowayOrdered::response()
{
    EventPtr e;
    {
	IceUtil::Mutex::Lock sync(_mutex);
	
	assert(_state == StateActive && _busy);
	
	if(_events.empty())
	{
	    _busy = false;
	    return;
	}
	
	e = _events.front();
	_events.erase(_events.begin());
    }

    _obj->ice_invoke_async(new TwowayOrderedInvokeI(this), e->op, e->mode, e->data, e->context);
}

namespace
{

class Topiclink_forwardI : public IceStorm::AMI_TopicLink_forward
{
public:

    Topiclink_forwardI(const SubscriberLinkPtr& subscriber) : _subscriber(subscriber)
    {
    }

    virtual void
    ice_response()
    {
	_subscriber->response();
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
	}
	catch(const Ice::ObjectNotExistException& ex)
	{
	    _subscriber->setError(ex);
	}
	catch(const Ice::LocalException& ex)
	{
	    _subscriber->setUnreachable(ex);
	}
    }

private:

    const SubscriberLinkPtr _subscriber;
};

}

SubscriberLink::SubscriberLink(
    const InstancePtr& instance,
    const TopicLinkPrx& obj,
    int cost) :
    Subscriber(instance, 0, true, obj->ice_getIdentity()),
    _obj(TopicLinkPrx::uncheckedCast(obj->ice_collocationOptimized(false))),
    _cost(cost)
{
}

Subscriber::QueueState
SubscriberLink::queue(bool forwarded, const EventSeq& events)
{
    if(forwarded)
    {
	return QueueStateNoFlush;
    }

    //
    // Don't propagate a message that has already been forwarded.
    // Also, if this link has a non-zero cost, then don't propagate
    // a message whose cost exceeds the link cost.
    //

    IceUtil::Mutex::Lock sync(_mutex);
    
    if(_state != StateActive)
    {
	//
	// Either the state is error here or the link is inactive.
	//
	return (_state == StateError) ? QueueStateError : QueueStateNoFlush;
    }
    
    size_t s = _events.size();
    for(EventSeq::const_iterator p = events.begin(); p != events.end(); ++p)
    {
	if(_cost != 0)
	{
	    //
	    // Note that we could calculate this cost once and
	    // cache it in a private form of the event to avoid
	    // this if this really is a performance problem.
	    //
	    int cost = 0;
	    Ice::Context::const_iterator q = (*p)->context.find("cost");
	    if(q != (*p)->context.end())
	    {
		cost = atoi(q->second.c_str());
	    }
	    if(cost > _cost)
	    {
		continue;
	    }
	}
	_events.push_back(*p);
    }

    //
    // If no event was queued, or we're busy then the subscriber
    // doesn't need to be flushed, otherwise it must be.
    //
    if(_busy || s == _events.size())
    {
	return QueueStateNoFlush;
    }

    _busy = true;
    return QueueStateFlush;
}

bool
SubscriberLink::flush()
{
    EventSeq v;
    {
	IceUtil::Mutex::Lock sync(_mutex);
	
	//
	// If the subscriber errored out then we're done.
	//
	if(_state != StateActive)
	{
	    _busy = false;
	    return false;
	}
	assert(!_events.empty());
	assert(_busy);
	
	v.swap(_events);
    }

    _obj->forward_async(new Topiclink_forwardI(this), v);

    return false;
}

void
SubscriberLink::response()
{
    EventSeq v;
    {
	IceUtil::Mutex::Lock sync(_mutex);
	
	assert(_state == StateActive && _busy);
	
	if(_events.empty())
	{
	    _busy = false;
	    return;
	}

	v.swap(_events);
    }

    _obj->forward_async(new Topiclink_forwardI(this), v);
}

SubscriberPtr
Subscriber::create(
    const InstancePtr& instance,
    const Ice::ObjectPrx& obj,
    const IceStorm::QoS& qos)
{
    PerSubscriberPublisherIPtr per = new PerSubscriberPublisherI(instance);
    Ice::ObjectPrx proxy = instance->objectAdapter()->addWithUUID(per);
    TraceLevelsPtr traceLevels = instance->traceLevels();
    SubscriberPtr subscriber;

    try
    {
	string reliability = "oneway";
	QoS::const_iterator p = qos.find("reliability");
	if(p != qos.end())
	{
	    reliability = p->second;
	}
	Ice::ObjectPrx newObj;

	if(reliability == "batch")
	{
	    if(obj->ice_isDatagram())
	    {
		newObj = obj->ice_batchDatagram();
	    }
	    else
	    {
		newObj = obj->ice_batchOneway();
	    }
	    subscriber = new SubscriberOneway(instance, proxy, newObj, true);
	}
	else if(reliability == "twoway")
	{
	    newObj = obj->ice_twoway();
	    subscriber = new SubscriberTwoway(instance, proxy, newObj);
	}
	else if(reliability == "twoway ordered")
	{
	    newObj = obj->ice_twoway();
	    subscriber = new SubscriberTwowayOrdered(instance, proxy, newObj);
	}
	else // reliability == "oneway"
	{
	    if(reliability != "oneway" && traceLevels->subscriber > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
		out << reliability <<" mode not understood.";
	    }
	    if(obj->ice_isDatagram())
	    {
		newObj = obj;
	    }
	    else
	    {
		newObj = obj->ice_oneway();
	    }
	    subscriber = new SubscriberOneway(instance, proxy, newObj, false);
	}
	per->setSubscriber(subscriber);
    }
    catch(const Ice::Exception&)
    {
	instance->objectAdapter()->remove(proxy->ice_getIdentity());
	throw;
    }
    
    return subscriber;
}

SubscriberPtr
Subscriber::create(
    const InstancePtr& instance,
    const TopicLinkPrx& link,
    int cost)
{
    return new SubscriberLink(instance, link, cost);
}

Subscriber::~Subscriber()
{
}

void
Subscriber::reachable()
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_state == StateUnreachable)
    {
	_state = StateActive;

	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(traceLevels->subscriber > 0)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
	    out << "Reachable " << _instance->communicator()->identityToString(id());
	}
    }
}

Ice::ObjectPrx
Subscriber::proxy() const
{
    return _proxy;
}

Ice::Identity
Subscriber::id() const
{
    return _id;
}

bool
Subscriber::persistent() const
{
    return _persistent;
}

Subscriber::QueueState
Subscriber::queue(bool, const EventSeq& events)
{
    IceUtil::Mutex::Lock sync(_mutex);
    
    if(_state != StateActive)
    {
	return QueueStateError;
    }

    copy(events.begin(), events.end(), back_inserter(_events));

    //
    // If another thread is busy delivering events then the subscriber
    // does not need to be flushed.
    //
    if(_busy)
    {
	return QueueStateNoFlush;
    }
    _busy = true;
    return QueueStateFlush;
}

void
Subscriber::destroy()
{
    //
    // Clear the per-subscriber object if it exists.
    //
    if(_proxy)
    {
	try
	{
	    _instance->objectAdapter()->remove(_proxy->ice_getIdentity());
	}
	catch(const Ice::NotRegisteredException&)
	{
	    // Ignore
	}
    }
}

void
Subscriber::flushTime(const IceUtil::Time& interval)
{
    if(_resetMax || interval > _maxSend)
    {
	assert(interval != IceUtil::Time());
	_resetMax = false;
	_maxSend = interval;
    }
}

IceUtil::Time
Subscriber::pollMaxFlushTime(const IceUtil::Time& now)
{
    //IceUtil::Time max = _maxSend;
    //_maxSend = _maxSend * 0.95;
    //return max;

    // The next call to flushTime can reset the max time.
    _resetMax = true;
    return _maxSend;
}

void
Subscriber::setError(const Ice::Exception& e)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_state != StateError)
    {
	_state = StateError;
	_busy = false;
	
	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(traceLevels->subscriber > 0)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
	    out << _instance->communicator()->identityToString(_id) << ": topic publish failed: " << e;
	}
    }
}

void
Subscriber::setUnreachable(const Ice::Exception& e)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_state != StateUnreachable)
    {
	_state = StateUnreachable;
	_busy = false;
	
	TraceLevelsPtr traceLevels = _instance->traceLevels();
	if(traceLevels->subscriber > 0)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
	    out << _instance->communicator()->identityToString(_id) << ": link publish unreachable: " << e;
	}
    }
}

Subscriber::Subscriber(
    const InstancePtr& instance,
    const Ice::ObjectPrx& proxy,
    bool persistent,
    const Ice::Identity& id) :
    _instance(instance),
    _id(id),
    _persistent(persistent),
    _proxy(proxy),
    _state(StateActive),
    _busy(false),
    _resetMax(true),
    _maxSend(IceUtil::Time::seconds(60*24)) // A long time
{
}

bool
IceStorm::operator==(const SubscriberPtr& subscriber, const Ice::Identity& id)
{
    return subscriber->id() == id;
}

