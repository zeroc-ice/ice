// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/TopicI.h>
#include <IceStorm/Instance.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/NodeI.h>
#include <IceStorm/Observers.h>
#include <IceStorm/SubscriberMap.h>
#include <IceStorm/LLUMap.h>
#include <IceStorm/Util.h>
#include <Ice/LoggerUtil.h>
#include <algorithm>

using namespace std;
using namespace IceStorm;
using namespace IceStormElection;
using namespace IceStormInternal;

using namespace Freeze;

namespace
{

const string subscriberDbName = "subscribers";

void
halt(const Ice::CommunicatorPtr& com, const DatabaseException& ex)
{
    {
        Ice::Error error(com->getLogger());
        error << "fatal exception: " << ex << "\n*** Aborting application ***";
    }

    abort();
}

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from Publishers.
//
class PublisherI : public Ice::BlobjectArray
{
public:

    PublisherI(const TopicImplPtr& topic, const InstancePtr& instance) :
        _topic(topic), _instance(instance)
    {
    }

    virtual bool
    ice_invoke(const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
               Ice::ByteSeq&,
               const Ice::Current& current)
    {
        // The publish call does a cached read.
        EventDataPtr event = new EventData(current.operation, current.mode, Ice::ByteSeq(), current.ctx);

        //
        // COMPILERBUG: gcc 4.0.1 doesn't like this.
        //
        //event->data.swap(Ice::ByteSeq(inParams.first, inParams.second));
        Ice::ByteSeq data(inParams.first, inParams.second);
        event->data.swap(data);

        EventDataSeq v;
        v.push_back(event);
        _topic->publish(false, v);

        return true;
    }

private:

    const TopicImplPtr _topic;
    const InstancePtr _instance;
};

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from linked Topics.
//
class TopicLinkI : public TopicLink
{
public:

    TopicLinkI(const TopicImplPtr& impl, const InstancePtr& instance) :
        _impl(impl), _instance(instance)
    {
    }

    virtual void
    forward(const EventDataSeq& v, const Ice::Current& /*current*/)
    {
        // The publish call does a cached read.
        _impl->publish(true, v);
    }

private:

    const TopicImplPtr _impl;
    const InstancePtr _instance;
};

class TopicI : public TopicInternal
{
public:

    TopicI(const TopicImplPtr& impl, const InstancePtr& instance) :
        _impl(impl), _instance(instance)
    {
    }

    virtual string getName(const Ice::Current&) const
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->getName();
    }

    virtual Ice::ObjectPrx getPublisher(const Ice::Current&) const
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->getPublisher();
    }

    virtual Ice::ObjectPrx getNonReplicatedPublisher(const Ice::Current&) const
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->getNonReplicatedPublisher();
    }

    virtual Ice::ObjectPrx subscribeAndGetPublisher(const QoS& qos, const Ice::ObjectPrx& obj,
                                                    const Ice::Current& current)
    {
        while(true)
        {
            Ice::Long generation = -1;
            TopicPrx master = getMasterFor(current, generation, __FILE__, __LINE__);
            if(master)
            {
                try
                {
                    return master->subscribeAndGetPublisher(qos, obj);
                }
                catch(const Ice::ConnectFailedException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
                catch(const Ice::TimeoutException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
            }
            else
            {
                FinishUpdateHelper unlock(_instance->node());
                return _impl->subscribeAndGetPublisher(qos, obj);
            }
        }
    }

    virtual void unsubscribe(const Ice::ObjectPrx& subscriber, const Ice::Current& current)
    {
        while(true)
        {
            Ice::Long generation = -1;
            TopicPrx master = getMasterFor(current, generation, __FILE__, __LINE__);
            if(master)
            {
                try
                {
                    master->unsubscribe(subscriber);
                }
                catch(const Ice::ConnectFailedException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
                catch(const Ice::TimeoutException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
            }
            else
            {
                FinishUpdateHelper unlock(_instance->node());
                _impl->unsubscribe(subscriber);
            }
            break;
        }
    }

    virtual TopicLinkPrx getLinkProxy(const Ice::Current&)
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->getLinkProxy();
    }

    virtual void reap(const Ice::IdentitySeq& ids, const Ice::Current& /*current*/)
    {
        NodeIPtr node = _instance->node();
        if(!node->updateMaster(__FILE__, __LINE__))
        {
            throw ReapWouldBlock();
        }
        FinishUpdateHelper unlock(node);
        _impl->reap(ids);
    }

    virtual void link(const TopicPrx& topic, Ice::Int cost, const Ice::Current& current)
    {
        while(true)
        {
            Ice::Long generation = -1;
            TopicPrx master = getMasterFor(current, generation, __FILE__, __LINE__);
            if(master)
            {
                try
                {
                    master->link(topic, cost);
                }
                catch(const Ice::ConnectFailedException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
                catch(const Ice::TimeoutException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
            }
            else
            {
                FinishUpdateHelper unlock(_instance->node());
                _impl->link(topic, cost);
            }
            break;
        }
    }

    virtual void unlink(const TopicPrx& topic, const Ice::Current& current)
    {
        while(true)
        {
            Ice::Long generation = -1;
            TopicPrx master = getMasterFor(current, generation, __FILE__, __LINE__);
            if(master)
            {
                try
                {
                    master->unlink(topic);
                }
                catch(const Ice::ConnectFailedException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
                catch(const Ice::TimeoutException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
            }
            else
            {
                FinishUpdateHelper unlock(_instance->node());
                _impl->unlink(topic);
            }
            break;
        }
    }

    virtual LinkInfoSeq getLinkInfoSeq(const Ice::Current&) const
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->getLinkInfoSeq();
    }

    virtual Ice::IdentitySeq getSubscribers(const Ice::Current&) const
    {
        return _impl->getSubscribers();
    }

    virtual void destroy(const Ice::Current& current)
    {
        while(true)
        {
            Ice::Long generation = -1;
            TopicPrx master = getMasterFor(current, generation, __FILE__, __LINE__);
            if(master)
            {
                try
                {
                    master->destroy();
                }
                catch(const Ice::ConnectFailedException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
                catch(const Ice::TimeoutException&)
                {
                    _instance->node()->recovery(generation);
                    continue;
                }
            }
            else
            {
                FinishUpdateHelper unlock(_instance->node());
                _impl->destroy();
            }
            break;
        }
    }

private:

    TopicPrx getMasterFor(const Ice::Current& cur, Ice::Long& generation, const char* file, int line) const
    {
        NodeIPtr node = _instance->node();
        Ice::ObjectPrx master;
        if(node)
        {
            master = _instance->node()->startUpdate(generation, file, line);
        }
        return (master) ? TopicPrx::uncheckedCast(master->ice_identity(cur.id)) : TopicPrx();
    }

    const TopicImplPtr _impl;
    const InstancePtr _instance;
};

}

namespace IceStorm
{
extern string identityToTopicName(const Ice::Identity& id);
}

TopicImpl::TopicImpl(
    const InstancePtr& instance,
    const string& name,
    const Ice::Identity& id,
    const SubscriberRecordSeq& subscribers) :
    _instance(instance),
    _connection(Freeze::createConnection(instance->communicator(), instance->serviceName())),
    _name(name),
    _id(id),
    _destroyed(false)
{
    try
    {
        __setNoDelete(true);

        // TODO: If we want to improve the performance of the
        // non-replicated case we could allocate a null-topic impl here.
        _servant = new TopicI(this, instance);

        //
        // Create a servant per topic to receive event data. If the
        // category is empty then we are in backwards compatibility
        // mode. In this case the servant's identity is
        // category=<topicname>, name=publish, otherwise the name is
        // <instancename>/<topicname>.publish. The same applies to the
        // link proxy.
        //
        // Activate the object and save a reference to give to publishers.
        //
        Ice::Identity pubid;
        Ice::Identity linkid;
        if(id.category.empty())
        {
            pubid.category = _name;
            pubid.name = "publish";
            linkid.category = _name;
            linkid.name = "link";
        }
        else
        {
            pubid.category = id.category;
            pubid.name = _name + ".publish";
            linkid.category = id.category;
            linkid.name = _name + ".link";
        }

        _publisherPrx = _instance->publishAdapter()->add(new PublisherI(this, instance), pubid);
        _linkPrx = TopicLinkPrx::uncheckedCast(
            _instance->publishAdapter()->add(new TopicLinkI(this, instance), linkid));

        //
        // Re-establish subscribers.
        //
        for(SubscriberRecordSeq::const_iterator p = subscribers.begin(); p != subscribers.end(); ++p)
        {
            Ice::Identity id = p->obj->ice_getIdentity();
            TraceLevelsPtr traceLevels = _instance->traceLevels();
            if(traceLevels->topic > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
                out << _name << " recreate " << _instance->communicator()->identityToString(id);
                if(traceLevels->topic > 1)
                {
                    out << " endpoints: " << IceStormInternal::describeEndpoints(p->obj);
                }
            }

            try
            {
                //
                // Create the subscriber object add it to the set of
                // subscribers.
                //
                SubscriberPtr subscriber = Subscriber::create(_instance, *p);
                _subscribers.push_back(subscriber);
            }
            catch(const Ice::Exception& ex)
            {
                Ice::Warning out(traceLevels->logger);
                out << _name << " recreate " << _instance->communicator()->identityToString(id);
                if(traceLevels->topic > 1)
                {
                    out << " endpoints: " << IceStormInternal::describeEndpoints(p->obj);
                }
                out << " failed: " << ex;
            }
        }

        if(_instance->observer())
        {
            _observer.attach(_instance->observer()->getTopicObserver(_instance->serviceName(), _name, 0));
        }
    }
    catch(...)
    {
        shutdown();
        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
}

TopicImpl::~TopicImpl()
{
    //cout << "~TopicImpl" << endl;
}

string
TopicImpl::getName() const
{
    // Immutable
    return _name;
}

Ice::ObjectPrx
TopicImpl::getPublisher() const
{
    // Immutable
    if(_instance->publisherReplicaProxy())
    {
        return _instance->publisherReplicaProxy()->ice_identity(_publisherPrx->ice_getIdentity());
    }
    return _publisherPrx;
}

Ice::ObjectPrx
TopicImpl::getNonReplicatedPublisher() const
{
    // If there is an adapter id configured then we're using icegrid
    // so create an indirect proxy, otherwise create a direct proxy.
    if(!_publisherPrx->ice_getAdapterId().empty())
    {
        return _instance->publishAdapter()->createIndirectProxy(_publisherPrx->ice_getIdentity());
    }
    else
    {
        return _instance->publishAdapter()->createDirectProxy(_publisherPrx->ice_getIdentity());
    }
}

namespace
{
void
trace(Ice::Trace& out, const InstancePtr& instance, const vector<SubscriberPtr>& s)
{
    out << '[';
    for(vector<SubscriberPtr>::const_iterator p = s.begin(); p != s.end(); ++p)
    {
        if(p != s.begin())
        {
            out << ",";
        }
        out << instance->communicator()->identityToString((*p)->id());
    }
    out << "]";
}
}

Ice::ObjectPrx
TopicImpl::subscribeAndGetPublisher(const QoS& qos, const Ice::ObjectPrx& obj)
{
    if(!obj)
    {
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": subscribeAndGetPublisher: null proxy";
        }
        throw InvalidSubscriber("subscriber is a null proxy");
    }
    Ice::Identity id = obj->ice_getIdentity();

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": subscribeAndGetPublisher: " << _instance->communicator()->identityToString(id);

        if(traceLevels->topic > 1)
        {
            out << " endpoints: " << IceStormInternal::describeEndpoints(obj)
                << " QoS: ";
            for(QoS::const_iterator p = qos.begin(); p != qos.end() ; ++p)
            {
                if(p != qos.begin())
                {
                    out << ',';
                }

            }
            out << " subscriptions: ";
            trace(out, _instance, _subscribers);
        }
    }

    IceUtil::Mutex::Lock sync(_subscribersMutex);

    SubscriberRecord record;
    record.id = id;
    record.obj = obj;
    record.theQoS = qos;
    record.topicName = _name;
    record.link = false;
    record.cost = 0;

    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), record.id);
    if(p != _subscribers.end())
    {
        throw AlreadySubscribed();
    }

    LogUpdate llu;

    SubscriberPtr subscriber = Subscriber::create(_instance, record);
    for(;;)
    {
        try
        {
            TransactionHolder txn(_connection);

            SubscriberRecordKey key;
            key.topic = _id;
            key.id = subscriber->id();

            SubscriberMap subscriberMap(_connection, subscriberDbName);
            subscriberMap.put(SubscriberMap::value_type(key, record));

            llu = getLLU(_connection);
            llu.iteration++;
            putLLU(_connection, llu);

            txn.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_instance->communicator(), ex);
        }
    }

    _subscribers.push_back(subscriber);

    _instance->observers()->addSubscriber(llu, _name, record);

    return subscriber->proxy();
}

void
TopicImpl::unsubscribe(const Ice::ObjectPrx& subscriber)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(!subscriber)
    {
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": unsubscribe: null proxy";
        }
        throw InvalidSubscriber("subscriber is a null proxy");
    }

    Ice::Identity id = subscriber->ice_getIdentity();

    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": unsubscribe: " << _instance->communicator()->identityToString(id);

        if(traceLevels->topic > 1)
        {
            out << " endpoints: " << IceStormInternal::describeEndpoints(subscriber);
            trace(out, _instance, _subscribers);
        }
    }

    IceUtil::Mutex::Lock sync(_subscribersMutex);
    Ice::IdentitySeq ids;
    ids.push_back(id);
    removeSubscribers(ids);
}

TopicLinkPrx
TopicImpl::getLinkProxy()
{
    // immutable
    if(_instance->publisherReplicaProxy())
    {
        return TopicLinkPrx::uncheckedCast(_instance->publisherReplicaProxy()->ice_identity(
                                               _linkPrx->ice_getIdentity()));
    }
    return _linkPrx;
}

void
TopicImpl::link(const TopicPrx& topic, Ice::Int cost)
{
    TopicInternalPrx internal = TopicInternalPrx::uncheckedCast(topic);
    TopicLinkPrx link = internal->getLinkProxy();

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": link " << _instance->communicator()->identityToString(topic->ice_getIdentity())
            << " cost " << cost;
    }

    IceUtil::Mutex::Lock sync(_subscribersMutex);

    Ice::Identity id = topic->ice_getIdentity();

    SubscriberRecord record;
    record.id = id;
    record.obj = link;
    record.theTopic = topic;
    record.topicName = _name;
    record.link = true;
    record.cost = cost;

    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), record.id);
    if(p != _subscribers.end())
    {
        string name = identityToTopicName(id);
        LinkExists ex;
        ex.name = name;
        throw ex;
    }

    LogUpdate llu;

    SubscriberPtr subscriber = Subscriber::create(_instance, record);

    for(;;)
    {
        try
        {
            TransactionHolder txn(_connection);

            SubscriberRecordKey key;
            key.topic = _id;
            key.id = id;

            SubscriberMap subscriberMap(_connection, subscriberDbName);
            subscriberMap.put(SubscriberMap::value_type(key, record));

            llu = getLLU(_connection);
            llu.iteration++;
            putLLU(_connection, llu);

            txn.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_instance->communicator(), ex);
        }
    }

    _subscribers.push_back(subscriber);

    _instance->observers()->addSubscriber(llu, _name, record);
}

void
TopicImpl::unlink(const TopicPrx& topic)
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    Ice::Identity id = topic->ice_getIdentity();

    vector<SubscriberPtr>::const_iterator p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p == _subscribers.end())
    {
        string name = identityToTopicName(id);
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": unlink " << name << " failed - not linked";
        }

        NoSuchLink ex;
        ex.name = name;
        throw ex;
    }

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << " unlink " << _instance->communicator()->identityToString(id);
    }

    Ice::IdentitySeq ids;
    ids.push_back(id);
    removeSubscribers(ids);
}

void
TopicImpl::reap(const Ice::IdentitySeq& ids)
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": reap ";
        for(Ice::IdentitySeq::const_iterator p = ids.begin(); p != ids.end() ; ++p)
        {
            if(p != ids.begin())
            {
                out << ",";
            }
            out << _instance->communicator()->identityToString(*p);
        }
    }

    removeSubscribers(ids);
}

void
TopicImpl::shutdown()
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    _servant = 0;

    // Shutdown each subscriber. This waits for the event queues to drain.
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        (*p)->shutdown();
    }

    _observer.detach();
}

LinkInfoSeq
TopicImpl::getLinkInfoSeq() const
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);

    LinkInfoSeq seq;
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        SubscriberRecord record = (*p)->record();
        if(record.link && !(*p)->errored())
        {
            LinkInfo info;
            info.name = identityToTopicName(record.theTopic->ice_getIdentity());
            info.cost = record.cost;
            info.theTopic = record.theTopic;
            seq.push_back(info);
        }
    }
    return seq;
}

Ice::IdentitySeq
TopicImpl::getSubscribers() const
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);

    Ice::IdentitySeq subscribers;
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        subscribers.push_back((*p)->id());
    }
    return subscribers;
}

void
TopicImpl::destroy()
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    _destroyed = true;

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": destroy";
    }

    // destroyInternal clears out the topic content.
    LogUpdate llu = {0,0};
    _instance->observers()->destroyTopic(destroyInternal(llu, true), _name);

    _observer.detach();
}

TopicContent
TopicImpl::getContent() const
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);

    TopicContent content;
    content.id = _id;
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        // Don't return errored subscribers (subscribers that have
        // errored out, but not reaped due to a failure with the
        // master). This means we can avoid the reaping step later.
        if(!(*p)->errored())
        {
            content.records.push_back((*p)->record());
        }
    }
    return content;
}

void
TopicImpl::update(const SubscriberRecordSeq& records)
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);

    // We do this with two scans. The first runs through the subscribers
    // that we have and removes those not in the init list. The second
    // runs through the init list and add the ones that don't
    // exist.

    {
        vector<SubscriberPtr>::iterator p = _subscribers.begin();
        while(p != _subscribers.end())
        {
            SubscriberRecordSeq::const_iterator q;
            for(q = records.begin(); q != records.end(); ++q)
            {
                if((*p)->id() == q->id)
                {
                    break;
                }
            }
            // The subscriber doesn't exist in the incoming subscriber
            // set so destroy it.
            if(q == records.end())
            {
                (*p)->destroy();
                p = _subscribers.erase(p);
            }
            else
            {
                // Otherwise reset the reaped status if necessary.
                (*p)->resetIfReaped();
                ++p;
            }
        }
    }

    for(SubscriberRecordSeq::const_iterator p = records.begin(); p != records.end(); ++p)
    {
        vector<SubscriberPtr>::iterator q;
        for(q = _subscribers.begin(); q != _subscribers.end(); ++q)
        {
            if((*q)->id() == p->id)
            {
                break;
            }
        }
        if(q == _subscribers.end())
        {
            SubscriberPtr subscriber = Subscriber::create(_instance, *p);
            _subscribers.push_back(subscriber);
        }
    }
}

bool
TopicImpl::destroyed() const
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    return _destroyed;
}

Ice::Identity
TopicImpl::id() const
{
    // immutable
    return _id;
}

TopicPrx
TopicImpl::proxy() const
{
    // immutable
    Ice::ObjectPrx prx;
    if(_instance->topicReplicaProxy())
    {
        prx = _instance->topicReplicaProxy()->ice_identity(_id);
    }
    else
    {
        prx = _instance->topicAdapter()->createProxy(_id);
    }
    return TopicPrx::uncheckedCast(prx);
}

namespace
{

class TopicInternalReapCB : public IceUtil::Shared
{
public:

    TopicInternalReapCB(const InstancePtr& instance, Ice::Long generation) :
        _instance(instance), _generation(generation)
    {
    }

    virtual void exception(const Ice::Exception& ex)
    {
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << "exception when calling `reap' on the master replica: " << ex;
        }
        _instance->node()->recovery(_generation);
    }

private:

    const InstancePtr _instance;
    const Ice::Long _generation;
};

}

void
TopicImpl::publish(bool forwarded, const EventDataSeq& events)
{
    TopicInternalPrx masterInternal;
    Ice::Long generation = -1;
    Ice::IdentitySeq reap;
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);

        //
        // Copy of the subscriber list so that event publishing can occur
        // in parallel.
        //
        vector<SubscriberPtr> copy;
        {
            IceUtil::Mutex::Lock sync(_subscribersMutex);
            if(_observer)
            {
                if(forwarded)
                {
                    _observer->forwarded();
                }
                else
                {
                    _observer->published();
                }
            }
            copy = _subscribers;
        }

        //
        // Queue each event, gathering a list of those subscribers that
        // must be reaped.
        //
        for(vector<SubscriberPtr>::const_iterator p = copy.begin(); p != copy.end(); ++p)
        {
            if(!(*p)->queue(forwarded, events) && (*p)->reap())
            {
                reap.push_back((*p)->id());
            }
        }

        // If there are no subscribers in error then we're done.
        if(reap.empty())
        {
            return;
        }
        if(!unlock.getMaster())
        {
            IceUtil::Mutex::Lock sync(_subscribersMutex);
            removeSubscribers(reap);
            return;
        }
        masterInternal = TopicInternalPrx::uncheckedCast(unlock.getMaster()->ice_identity(_id));
        generation = unlock.generation();
    }


    // Tell the master to reap this set of subscribers. This is an
    // AMI invocation so it shouldn't block the caller (in the
    // typical case) we do it outside of the mutex lock for
    // performance reasons.
    //
    // We must release the cached lock before calling this as the AMI
    // call may raise an exception in the caller (that is directly
    // call ice_exception) which calls recover() on the node which
    // would result in a deadlock since the node is locked.
    masterInternal->begin_reap(reap, newCallback_TopicInternal_reap(new TopicInternalReapCB(_instance, generation),
                                                                    &TopicInternalReapCB::exception));
}

void
TopicImpl::observerAddSubscriber(const LogUpdate& llu, const SubscriberRecord& record)
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": add replica observer: " << _instance->communicator()->identityToString(record.id);

        if(traceLevels->topic > 1)
        {
            out << " endpoints: " << IceStormInternal::describeEndpoints(record.obj)
                << " QoS: ";
            for(QoS::const_iterator p = record.theQoS.begin(); p != record.theQoS.end() ; ++p)
            {
                if(p != record.theQoS.begin())
                {
                    out << ',';
                }
                out << '[' << p->first << "," << p->second << ']';
            }
        }
        out << " llu: " << llu.generation << "/" << llu.iteration;
    }

    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), record.id);
    if(p != _subscribers.end())
    {
        // If the subscriber is already in the database display a
        // diagnostic.
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _instance->communicator()->identityToString(record.id) << ": already subscribed";
        }
        return;
    }

    SubscriberPtr subscriber = Subscriber::create(_instance, record);
    for(;;)
    {
        try
        {
            TransactionHolder txn(_connection);

            SubscriberRecordKey key;
            key.topic = _id;
            key.id = subscriber->id();

            SubscriberMap subscriberMap(_connection, subscriberDbName);
            subscriberMap.put(SubscriberMap::value_type(key, record));

            // Update the LLU.
            putLLU(_connection, llu);

            txn.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_instance->communicator(), ex);
        }
    }

    _subscribers.push_back(subscriber);
}

void
TopicImpl::observerRemoveSubscriber(const LogUpdate& llu, const Ice::IdentitySeq& ids)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": remove replica observer: ";
        for(Ice::IdentitySeq::const_iterator id = ids.begin(); id != ids.end(); ++id)
        {
            if(id != ids.begin())
            {
                out << ",";
            }
            out << _instance->communicator()->identityToString(*id);
        }
        out << " llu: " << llu.generation << "/" << llu.iteration;
    }

    IceUtil::Mutex::Lock sync(_subscribersMutex);

    // Remove the subscriber from the subscribers list. If the
    // subscriber had a local failure and was removed from the
    // subscriber list it could already be gone. That's not a problem.
    for(Ice::IdentitySeq::const_iterator id = ids.begin(); id != ids.end(); ++id)
    {
        vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), *id);
        if(p != _subscribers.end())
        {
            (*p)->destroy();
            _subscribers.erase(p);
        }
    }

    // Next remove from the database.
    for(;;)
    {
        try
        {
            TransactionHolder txn(_connection);

            for(Ice::IdentitySeq::const_iterator id = ids.begin(); id != ids.end(); ++id)
            {
                SubscriberRecordKey key;
                key.topic = _id;
                key.id = *id;

                SubscriberMap subscriberMap(_connection, subscriberDbName);
                subscriberMap.erase(key);
            }
            putLLU(_connection, llu);
            txn.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_instance->communicator(), ex);
        }
    }
}

void
TopicImpl::observerDestroyTopic(const LogUpdate& llu)
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);

    if(_destroyed)
    {
        return;
    }
    _destroyed = true;

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": destroyed";
        out << " llu: " << llu.generation << "/" << llu.iteration;
    }
    destroyInternal(llu, false);
}

Ice::ObjectPtr
TopicImpl::getServant() const
{
    return _servant;
}

void
TopicImpl::updateObserver()
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    if(_instance->observer())
    {
        _observer.attach(_instance->observer()->getTopicObserver(_instance->serviceName(), _name, _observer.get()));
    }
}

void
TopicImpl::updateSubscriberObservers()
{
    IceUtil::Mutex::Lock sync(_subscribersMutex);
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        (*p)->updateObserver();
    }
}

LogUpdate
TopicImpl::destroyInternal(const LogUpdate& origLLU, bool master)
{
    _instance->publishAdapter()->remove(_linkPrx->ice_getIdentity());
    _instance->publishAdapter()->remove(_publisherPrx->ice_getIdentity());
    _instance->topicReaper()->add(_name);

    // Destroy each of the subscribers.
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        (*p)->destroy();
    }
    _subscribers.clear();

    // Clear out the database records related to this topic.
    LogUpdate llu;
    for(;;)
    {
        try
        {
            TransactionHolder txn(_connection);

            // Erase all subscriber records and the topic record.
            SubscriberMap subscriberMap(_connection, subscriberDbName);

            IceStorm::SubscriberRecordKey key;
            key.topic = _id;
            SubscriberMap::iterator p = subscriberMap.find(key);
            while(p != subscriberMap.end() && p->first.topic == key.topic)
            {
                subscriberMap.erase(p++);
            }

            // Update the LLU.
            if(master)
            {
                llu = getLLU(_connection);
                llu.iteration++;
            }
            else
            {
                llu = origLLU;
            }
            putLLU(_connection, llu);

            txn.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_instance->communicator(), ex);
        }
    }

    _instance->topicAdapter()->remove(_id);

    _servant = 0;

    return llu;
}

void
TopicImpl::removeSubscribers(const Ice::IdentitySeq& ids)
{
    Ice::IdentitySeq removed;

    // First remove the subscriber from the subscribers list. Its
    // possible that some of these subscribers have already been
    // removed (consider, for example, a concurrent reap call from two
    // replicas on the same subscriber). To avoid sending unnecessary
    // observer updates keep track of the observers that are actually
    // removed.
    for(Ice::IdentitySeq::const_iterator id = ids.begin(); id != ids.end(); ++id)
    {
        vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), *id);
        if(p != _subscribers.end())
        {
            (*p)->destroy();
            _subscribers.erase(p);
            removed.push_back(*id);
        }
    }

    // If there is no further work to do we are done.
    if(removed.empty())
    {
        return;
    }

    // Next update the database and send the notification to any
    // slaves.
    LogUpdate llu;
    for(;;)
    {
        try
        {
            TransactionHolder txn(_connection);

            for(Ice::IdentitySeq::const_iterator id = ids.begin(); id != ids.end(); ++id)
            {
                SubscriberRecordKey key;
                key.topic = _id;
                key.id = *id;

                SubscriberMap subscriberMap(_connection, subscriberDbName);
                subscriberMap.erase(key);
            }

            llu = getLLU(_connection);
            llu.iteration++;
            putLLU(_connection, llu);

            txn.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_instance->communicator(), ex);
        }
    }

    _instance->observers()->removeSubscriber(llu, _name, ids);
}
