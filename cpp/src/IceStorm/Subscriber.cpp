// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/Subscriber.h>
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
        EventDataPtr event = new EventData(
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

        EventDataSeq e;
        e.push_back(event);
        Subscriber::QueueState state = _subscriber->queue(false, e);

        if(state == Subscriber::QueueStateFlush)
        {
            _instance->subscriberPool()->flush(_subscriber);
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

    SubscriberOneway(const InstancePtr&, const Ice::ObjectPrx&, const Ice::ObjectPrx&);
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

    SubscriberTwoway(const InstancePtr&, const Ice::ObjectPrx&, const Ice::ObjectPrx&);

    virtual bool flush();
    void response();

private:

    const Ice::ObjectPrx _obj;
    const int _maxOutstanding;
    int _outstanding;
};
typedef IceUtil::Handle<SubscriberTwoway> SubscriberTwowayPtr;

//
// Twoway Ordered
//
class SubscriberTwowayOrdered : public Subscriber
{
public:

    SubscriberTwowayOrdered(const InstancePtr&, const Ice::ObjectPrx&, const Ice::ObjectPrx&);

    virtual bool flush();
    void response();

private:

    const Ice::ObjectPrx _obj;
};
typedef IceUtil::Handle<SubscriberTwowayOrdered> SubscriberTwowayOrderedPtr;

class SubscriberLink : public Subscriber
{
public:

    SubscriberLink(const InstancePtr&, const TopicLinkPrx&, int);

    virtual QueueState queue(bool, const std::vector<EventDataPtr>&);
    virtual bool flush();
    void response();

    void offline(const Ice::Exception&);

private:

    const TopicLinkPrx _obj;
    const int _cost;

    // The next to try sending a new event if we're offline.
    IceUtil::Time _next;
    bool _warn;
};
typedef IceUtil::Handle<SubscriberLink> SubscriberLinkPtr;

}

SubscriberOneway::SubscriberOneway(
    const InstancePtr& instance,
    const Ice::ObjectPrx& proxy,
    const Ice::ObjectPrx& obj) :
    Subscriber(instance, proxy, false, obj->ice_getIdentity()),
    _batch(obj->ice_isBatchDatagram() || obj->ice_isBatchOneway()),
    _obj(obj)
{
    //
    // COMPILERFIX: Initialized this way for Borland to compile.
    //
    if(obj->ice_isDatagram())
    {
        _objBatch = obj->ice_batchDatagram();
    }
    else
    {
        _objBatch = obj->ice_batchOneway();
    }

    if(_batch)
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
    if(_state == SubscriberStateError)
    {
        return false;
    }
    assert(_state == SubscriberStateFlushPending);
    assert(!_events.empty());

    try
    {
        //
        // Get the current set of events, but release the lock before
        // attempting to deliver the events. This allows other threads
        // to add events in case we block (such as during connection
        // establishment).
        //
        EventDataSeq v;
        v.swap(_events);
        sync.release();

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
            for(EventDataSeq::const_iterator p = v.begin(); p != v.end(); ++p)
            {
                _objBatch->ice_invoke((*p)->op, (*p)->mode, (*p)->data, dummy, (*p)->context);
            }
            Ice::ConnectionPtr conn = _objBatch->ice_getCachedConnection();
            assert(conn);
            conn->flushBatchRequests();
        }
        else
        {
            for(EventDataSeq::const_iterator p = v.begin(); p != v.end(); ++p)
            {
                _obj->ice_invoke((*p)->op, (*p)->mode, (*p)->data, dummy, (*p)->context);
            }
        }
        
        //
        // Reacquire the lock before we check the queue again.
        //
        sync.acquire();
    }
    catch(const Ice::LocalException& ex)
    {
        assert(!sync.acquired());
        // error will re-acquire and release the lock.
        error(ex);
        return false;
    }

    if(!_events.empty())
    {
        assert(_state == SubscriberStateFlushPending);
        return true;
    }
    _state = SubscriberStateOnline;
    return false;
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

    TwowayInvokeI(const SubscriberTwowayPtr& subscriber) :
        _subscriber(subscriber)
    {
    }

    virtual void
    ice_response(bool, const std::vector<Ice::Byte>&)
    {
        _subscriber->response();
    }

    virtual void
    ice_exception(const Ice::Exception& e)
    {
        _subscriber->error(e);
    }

private:

    const SubscriberTwowayPtr _subscriber;
};

}

SubscriberTwoway::SubscriberTwoway(
    const InstancePtr& instance,
    const Ice::ObjectPrx& proxy,
    const Ice::ObjectPrx& obj) :
    Subscriber(instance, proxy, false, obj->ice_getIdentity()),
    _obj(obj),
    _maxOutstanding(10),
    _outstanding(0)
{
}

bool
SubscriberTwoway::flush()
{
    EventDataPtr e;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        
        //
        // If the subscriber errored out then we're done.
        //
        if(_state == SubscriberStateError)
        {
            return false;
        }
        assert(_state == SubscriberStateFlushPending);
        assert(!_events.empty());

        //
        // If there are more than _maxOutstanding unanswered AMI
        // events we're also done. In this case the response to the
        // pending AMI requests will trigger another event to be sent.
        //
        if(_outstanding >= _maxOutstanding)
        {
            _state = SubscriberStateSending;
            return false;
        }
        
        //
        // Dequeue the head event, count one more outstanding AMI
        // request.
        //
        e = _events.front();
        _events.erase(_events.begin());
        _state = SubscriberStateSending;
        ++_outstanding;
    }

    _obj->ice_invoke_async(new TwowayInvokeI(this), e->op, e->mode, e->data, e->context);

    //
    // We process the subscriber state after the event send and not
    // before to prevent the subscriber from being requeued
    // concurrently.
    //
    {
        IceUtil::Mutex::Lock sync(_mutex);
        //
        // If the subscriber has already been requeued for a flush or
        // the subscriber errored out then we're done.
        //
        if(_state == SubscriberStateFlushPending || _state == SubscriberStateError)
        {
            return false;
        }

        //
        // If there are no events left in the queue transition back to
        // the online state, and return false to indicate to the
        // worker not to requeue.
        //
        if(_events.empty())
        {
            _state = SubscriberStateOnline;
            return false;
        }

        //
        // We must still be in sending state.
        //
        assert(_state == SubscriberStateSending);

        //
        // If we're below the outstanding limit then requeue,
        // otherwise the response callback will do so.
        //
        if(_outstanding < _maxOutstanding)
        {
            _state = SubscriberStateFlushPending;
        }

        return _state == SubscriberStateFlushPending;
    }
}

void
SubscriberTwoway::response()
{
    IceUtil::Mutex::Lock sync(_mutex);

    --_outstanding;

    //
    // Note that its possible for the _state to be error if there are
    // mutliple threads in the client side thread pool and response
    // and exception are called out of order.
    //
    assert(_outstanding >= 0 && _outstanding < _maxOutstanding);

    //
    // Unless we're in the sending state we do nothing.
    //
    if(_state == SubscriberStateSending)
    {
        //
        // If there are no more events then we transition back to
        // online.
        //
        if(_events.empty())
        {
            _state = SubscriberStateOnline;
        }
        //
        // Otherwise we re-add for a flush.
        //
        else
        {
            _state = SubscriberStateFlushPending;
            _instance->subscriberPool()->flush(this);
        }
    }
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
        _subscriber->error(ex);
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
    EventDataPtr e;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        
        //
        // If the subscriber errored out then we're done.
        //
        if(_state == SubscriberStateError)
        {
            return false;
        }
        assert(_state == SubscriberStateFlushPending);
        assert(!_events.empty());
        
        e = _events.front();
        _events.erase(_events.begin());
    }
    
    _obj->ice_invoke_async(new TwowayOrderedInvokeI(this), e->op, e->mode, e->data, e->context);

    return false;
}

void
SubscriberTwowayOrdered::response()
{
    IceUtil::Mutex::Lock sync(_mutex);
    
    assert(_state != SubscriberStateError);
    if(_events.empty())
    {
        _state = SubscriberStateOnline;
        return;
    }
    _instance->subscriberPool()->flush(this);
}

namespace
{

class Topiclink_forwardI : public IceStorm::AMI_TopicLink_forward
{
public:

    Topiclink_forwardI(const SubscriberLinkPtr& subscriber) :
        _subscriber(subscriber)
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
            _subscriber->error(ex);
        }
        catch(const Ice::LocalException& ex)
        {
            _subscriber->offline(ex);
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
    _cost(cost),
    _warn(true)
{
}

Subscriber::QueueState
SubscriberLink::queue(bool forwarded, const EventDataSeq& events)
{
    if(forwarded)
    {
        return QueueStateNoFlush;
    }

    //
    // Don't propagate a message that has already been forwarded.
    // Also, if this link has a non-zero cost, then don't propagate a
    // message whose cost exceeds the link cost.
    //

    IceUtil::Mutex::Lock sync(_mutex);

    if(_state == SubscriberStateError)
    {
        return QueueStateError;
    }

    //
    // If the proxy is offline and its time to send another event then
    // put us into retry state.
    //
    if(_state == SubscriberStateOffline)
    {
        //
        // If there are alot of subscribers offline then we will call
        // Time::now() alot, which could be costly. This could be
        // optimized to only one per event-batch by making the
        // forwarded argument an EventInfo thing where the queue-time
        // is lazy initialized.
        //
        if(IceUtil::Time::now() < _next)
        {
            return QueueStateNoFlush;
        }

        //
        // State transition to online.
        //
        _state = SubscriberStateOnline;
    }
    
    int queued = 0;
    for(EventDataSeq::const_iterator p = events.begin(); p != events.end(); ++p)
    {
        if(_cost != 0)
        {
            //
            // Note that we could calculate this cost once and cache
            // it in a private form of the event to avoid this if this
            // really is a performance problem (this could use the
            // EventInfo thing discussed above).
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
        ++queued;
        _events.push_back(*p);
    }

    if(_state == SubscriberStateFlushPending || queued == 0)
    {
        return QueueStateNoFlush;
    }
    _state = SubscriberStateFlushPending;
    return QueueStateFlush;
}

bool
SubscriberLink::flush()
{
    EventDataSeq v;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        
        //
        // If the subscriber errored out then we're done.
        //
        if(_state == SubscriberStateError)
        {
            return false;
        }

        assert(_state == SubscriberStateFlushPending);
        assert(!_events.empty());
        
        v.swap(_events);
    }

    _obj->forward_async(new Topiclink_forwardI(this), v);
    return false;
}

void
SubscriberLink::response()
{
    IceUtil::Mutex::Lock sync(_mutex);
    
    assert(_state != SubscriberStateError);
    
    //
    // A successful response means we're no longer retrying, we're
    // back active.
    //
    _warn = true;
    
    //
    // No more events, no need to requeue this subscriber.
    //
    if(_events.empty())
    {
        _state = SubscriberStateOnline;
        return;
    }
    _instance->subscriberPool()->flush(this);
}

void
SubscriberLink::offline(const Ice::Exception& e)
{
    IceUtil::Mutex::Lock sync(_mutex);
    assert(_state != SubscriberStateOffline);

    _next = IceUtil::Time::now() + _instance->discardInterval();

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(_warn)
    {
        Ice::Warning warn(traceLevels->logger);
        warn << traceLevels->subscriberCat << ":" << _instance->communicator()->identityToString(_id)
             << ": link offline: " << e;
    }
    else
    {
        if(traceLevels->subscriber > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
            out << _instance->communicator()->identityToString(_id) << ": link offline: " << e
                << " discarding events: " << _instance->discardInterval() << "s";
        }
    }

    _state = SubscriberStateOffline;
    _warn = false;

    //
    // Clear all queued events.
    //
    _events.clear();
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
        string reliability;
        QoS::const_iterator p = qos.find("reliability");
        if(p != qos.end())
        {
            reliability = p->second;
        }
        if(!reliability.empty() && reliability != "ordered")
        {
            throw BadQoS("invalid reliability: " + reliability);
        }

        //
        // Override the timeout.
        //
        Ice::ObjectPrx newObj;
        try
        {
            newObj = obj->ice_timeout(instance->sendTimeout());
        }
        catch(const Ice::FixedProxyException&)
        {
            //
            // In the event IceStorm is collocated this could be a
            // fixed proxy in which case its not possible to set the
            // timeout.
            //
            newObj = obj;
        }
        if(reliability == "ordered")
        {
            if(!newObj->ice_isTwoway())
            {
                throw BadQoS("ordered reliability requires a twoway proxy");
            }
            subscriber = new SubscriberTwowayOrdered(instance, proxy, newObj);
        }
        else if(newObj->ice_isOneway() || newObj->ice_isDatagram() ||
                newObj->ice_isBatchOneway() || newObj->ice_isBatchDatagram())
        {
            subscriber = new SubscriberOneway(instance, proxy, newObj);
        }
        else if(newObj->ice_isTwoway())
        {
            subscriber = new SubscriberTwoway(instance, proxy, newObj);
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
    return new SubscriberLink(
        instance,
        TopicLinkPrx::uncheckedCast(link->ice_timeout(instance->sendTimeout())),
        cost);
}

Subscriber::~Subscriber()
{
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
Subscriber::queue(bool, const EventDataSeq& events)
{
    IceUtil::Mutex::Lock sync(_mutex);
    
    if(_state == SubscriberStateError)
    {
        return QueueStateError;
    }

    copy(events.begin(), events.end(), back_inserter(_events));
    if(_state == SubscriberStateSending || _state == SubscriberStateFlushPending)
    {
        return QueueStateNoFlush;
    }

    _state = SubscriberStateFlushPending;
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
        catch(const Ice::ObjectAdapterDeactivatedException&)
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
        //
        // If is possible for the flush interval to be zero if the
        // timer resolution is sufficiently big. See
        // http://bugzilla.zeroc.com/bugzilla/show_bug.cgi?id=1739
        //
        //assert(interval != IceUtil::Time());
        _resetMax = false;
        _maxSend = interval;
    }
}

IceUtil::Time
Subscriber::pollMaxFlushTime(const IceUtil::Time& now)
{
    // The next call to flushTime can reset the max time.
    _resetMax = true;
    return _maxSend;
}

void
Subscriber::error(const Ice::Exception& e)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_state != SubscriberStateError)
    {
        _state = SubscriberStateError;
        _events.clear();
        
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->subscriber > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
            out << _instance->communicator()->identityToString(_id) << ": topic publish failed: " << e;
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
    _state(SubscriberStateOnline),
    _resetMax(true),
    _maxSend(IceUtil::Time::seconds(60*24)) // A long time
{
}

bool
IceStorm::operator==(const SubscriberPtr& subscriber, const Ice::Identity& id)
{
    return subscriber->id() == id;
}

bool
IceStorm::operator==(const Subscriber& s1, const Subscriber& s2)
{
    return &s1 == &s2;
}

bool
IceStorm::operator!=(const Subscriber& s1, const Subscriber& s2)
{
    return &s1 != &s2;
}

bool
IceStorm::operator<(const Subscriber& s1, const Subscriber& s2)
{
    return &s1 < &s2;
}
