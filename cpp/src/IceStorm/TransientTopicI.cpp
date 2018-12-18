// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceStorm/TransientTopicI.h>
#include <IceStorm/Instance.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Util.h>

#include <Ice/Ice.h>

#include <list>
#include <algorithm>

using namespace IceStorm;
using namespace std;

namespace
{

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from Publishers.
//
class TransientPublisherI : public Ice::BlobjectArray
{
public:

    TransientPublisherI(const TransientTopicImplPtr& impl) :
        _impl(impl)
    {
    }

    virtual bool
    ice_invoke(const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
               Ice::ByteSeq&,
               const Ice::Current& current)
    {
        // Use cached reads.
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

        EventDataSeq v;
        v.push_back(event);
        _impl->publish(false, v);

        return true;
    }

private:

    const TransientTopicImplPtr _impl;
};

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from linked Topics.
//
class TransientTopicLinkI : public TopicLink
{
public:

    TransientTopicLinkI(const TransientTopicImplPtr& impl) :
        _impl(impl)
    {
    }

    virtual void
    forward(const EventDataSeq& v, const Ice::Current& /*current*/)
    {
        _impl->publish(true, v);
    }

private:

    const TransientTopicImplPtr _impl;
};

}

TransientTopicImpl::TransientTopicImpl(
    const InstancePtr& instance,
    const string& name,
    const Ice::Identity& id) :
    _instance(instance),
    _name(name),
    _id(id),
    _destroyed(false)
{
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

    _publisherPrx = _instance->publishAdapter()->add(new TransientPublisherI(this), pubid);
    _linkPrx = TopicLinkPrx::uncheckedCast(_instance->publishAdapter()->add(new TransientTopicLinkI(this), linkid));
}

TransientTopicImpl::~TransientTopicImpl()
{
}

string
TransientTopicImpl::getName(const Ice::Current&) const
{
    // Immutable
    return _name;
}

Ice::ObjectPrx
TransientTopicImpl::getPublisher(const Ice::Current&) const
{
    // Immutable
    return _publisherPrx;
}

Ice::ObjectPrx
TransientTopicImpl::getNonReplicatedPublisher(const Ice::Current&) const
{
    // Immutable
    return _publisherPrx;
}

void
TransientTopicImpl::subscribe(const QoS& origQoS, const Ice::ObjectPrx& obj, const Ice::Current&)
{
    if(!obj)
    {
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": subscribe: null proxy";
        }
        throw InvalidSubscriber("subscriber is a null proxy");
    }
    Ice::Identity id = obj->ice_getIdentity();
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    QoS qos = origQoS;
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": subscribe: " << _instance->communicator()->identityToString(id);

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
                out << '[' << p->first << "," << p->second << ']';
            }
        }
    }

    string reliability = "oneway";
    {
        QoS::iterator p = qos.find("reliability");
        if(p != qos.end())
        {
            reliability = p->second;
            qos.erase(p);
        }
    }

    Ice::ObjectPrx newObj = obj;
    if(reliability == "batch")
    {
        if(newObj->ice_isDatagram())
        {
            newObj = newObj->ice_batchDatagram();
        }
        else
        {
            newObj = newObj->ice_batchOneway();
        }
    }
    else if(reliability == "twoway")
    {
        newObj = newObj->ice_twoway();
    }
    else if(reliability == "twoway ordered")
    {
        qos["reliability"] = "ordered";
        newObj = newObj->ice_twoway();
    }
    else // reliability == "oneway"
    {
        if(reliability != "oneway" && traceLevels->subscriber > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->subscriberCat);
            out << reliability <<" mode not understood.";
        }
        if(!newObj->ice_isDatagram())
        {
            newObj = newObj->ice_oneway();
        }
    }

    Lock sync(*this);
    SubscriberRecord record;
    record.id = id;
    record.obj = newObj;
    record.theQoS = qos;
    record.topicName = _name;
    record.link = false;
    record.cost = 0;

    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), record.id);
    if(p != _subscribers.end())
    {
        // If we already have this subscriber remove it from our
        // subscriber list and remove it from the database.
        (*p)->destroy();
        _subscribers.erase(p);
    }

    SubscriberPtr subscriber = Subscriber::create(_instance, record);
    _subscribers.push_back(subscriber);
}

Ice::ObjectPrx
TransientTopicImpl::subscribeAndGetPublisher(const QoS& qos, const Ice::ObjectPrx& obj, const Ice::Current&)
{
    if(!obj)
    {
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": subscribe: null proxy";
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
        }
    }

    Lock sync(*this);

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

    SubscriberPtr subscriber = Subscriber::create(_instance, record);
    _subscribers.push_back(subscriber);

    return subscriber->proxy();
}

void
TransientTopicImpl::unsubscribe(const Ice::ObjectPrx& subscriber, const Ice::Current&)
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
        }
    }

    Lock sync(*this);
    // First remove the subscriber from the subscribers list. Note
    // that its possible that the subscriber isn't in the list, but is
    // in the database if the subscriber was locally reaped.
    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p != _subscribers.end())
    {
        (*p)->destroy();
        _subscribers.erase(p);
    }
}

TopicLinkPrx
TransientTopicImpl::getLinkProxy(const Ice::Current&)
{
    // immutable
    return _linkPrx;
}

void
TransientTopicImpl::link(const TopicPrx& topic, Ice::Int cost, const Ice::Current&)
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

    Lock sync(*this);

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
        throw LinkExists(IceStormInternal::identityToTopicName(id));
    }

    SubscriberPtr subscriber = Subscriber::create(_instance, record);
    _subscribers.push_back(subscriber);
}

void
TransientTopicImpl::unlink(const TopicPrx& topic, const Ice::Current&)
{
    Lock sync(*this);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    Ice::Identity id = topic->ice_getIdentity();

    vector<SubscriberPtr>::iterator p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p == _subscribers.end())
    {
        string name = IceStormInternal::identityToTopicName(id);
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": unlink " << name << " failed - not linked";
        }
        throw NoSuchLink(name);
    }

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << " unlink " << _instance->communicator()->identityToString(id);
    }

    // Remove the subscriber from the subscribers list. Note
    // that its possible that the subscriber isn't in the list, but is
    // in the database if the subscriber was locally reaped.
    p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p != _subscribers.end())
    {
        (*p)->destroy();
        _subscribers.erase(p);
    }
}

LinkInfoSeq
TransientTopicImpl::getLinkInfoSeq(const Ice::Current&) const
{
    Lock sync(*this);
    LinkInfoSeq seq;
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        SubscriberRecord record = (*p)->record();
        if(record.link && !(*p)->errored())
        {
            LinkInfo info;
            info.name = IceStormInternal::identityToTopicName(record.theTopic->ice_getIdentity());
            info.cost = record.cost;
            info.theTopic = record.theTopic;
            seq.push_back(info);
        }
    }
    return seq;
}

Ice::IdentitySeq
TransientTopicImpl::getSubscribers(const Ice::Current&) const
{
    IceUtil::Mutex::Lock sync(*this);

    Ice::IdentitySeq subscribers;
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        subscribers.push_back((*p)->id());
    }
    return subscribers;
}

void
TransientTopicImpl::destroy(const Ice::Current&)
{
    Lock sync(*this);

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

    try
    {
        _instance->publishAdapter()->remove(_linkPrx->ice_getIdentity());
        _instance->publishAdapter()->remove(_publisherPrx->ice_getIdentity());
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // Ignore -- this could occur on shutdown.
    }

    // Destroy all of the subscribers.
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        (*p)->destroy();
    }
    _subscribers.clear();
}

void
TransientTopicImpl::reap(const Ice::IdentitySeq&, const Ice::Current&)
{
}

bool
TransientTopicImpl::destroyed() const
{
    Lock sync(*this);
    return _destroyed;
}

Ice::Identity
TransientTopicImpl::id() const
{
    // immutable
    return _id;
}

void
TransientTopicImpl::publish(bool forwarded, const EventDataSeq& events)
{
    //
    // Copy of the subscriber list so that event publishing can occur
    // in parallel.
    //
    vector<SubscriberPtr> copy;
    {
        Lock sync(*this);
        copy = _subscribers;
    }

    //
    // Queue each event, gathering a list of those subscribers that
    // must be reaped.
    //
    vector<Ice::Identity> e;
    for(vector<SubscriberPtr>::const_iterator p = copy.begin(); p != copy.end(); ++p)
    {
        if(!(*p)->queue(forwarded, events) && (*p)->reap())
        {
            e.push_back((*p)->id());
        }
    }

    //
    // Run through the error list removing those subscribers that are
    // in error from the subscriber list.
    //
    if(!e.empty())
    {
        Lock sync(*this);
        for(vector<Ice::Identity>::const_iterator ep = e.begin(); ep != e.end(); ++ep)
        {
            //
            // Its possible for the subscriber to already have been
            // removed since the copy is iterated over outside of
            // mutex protection.
            //
            // Note that although this could be quicker if we used a
            // map, the most optimal case should be pushing around
            // events not searching for a particular subscriber.
            //
            // The subscriber is immediately destroyed & removed from
            // the _subscribers list. Add the subscriber to a list of
            // error'd subscribers and remove it from the database on
            // the next reap.
            //
            vector<SubscriberPtr>::iterator q = find(_subscribers.begin(), _subscribers.end(), *ep);
            if(q != _subscribers.end())
            {
                SubscriberPtr subscriber = *q;
                //
                // Destroy the subscriber.
                //
                subscriber->destroy();
                _subscribers.erase(q);
            }
        }
    }
}

void
TransientTopicImpl::shutdown()
{
    Lock sync(*this);

    // Shutdown each subscriber. This waits for the event queues to drain.
    for(vector<SubscriberPtr>::const_iterator p = _subscribers.begin(); p != _subscribers.end(); ++p)
    {
        (*p)->shutdown();
    }
}
