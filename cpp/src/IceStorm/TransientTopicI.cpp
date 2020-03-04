//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    TransientPublisherI(shared_ptr<TransientTopicImpl> impl) :
        _impl(move(impl))
    {
    }

    bool
    ice_invoke(pair<const Ice::Byte*, const Ice::Byte*> inParams, Ice::ByteSeq&, const Ice::Current& current) override
    {
        // Use cached reads.
        EventData event = { current.operation, current.mode, Ice::ByteSeq(), current.ctx };

        Ice::ByteSeq data(inParams.first, inParams.second);
        event.data.swap(data);

        EventDataSeq v;
        v.push_back(move(event));
        _impl->publish(false, v);

        return true;
    }

private:

    const shared_ptr<TransientTopicImpl> _impl;
};

//
// The servant has a 1-1 association with a topic. It is used to
// receive events from linked Topics.
//
class TransientTopicLinkI : public TopicLink
{
public:

    TransientTopicLinkI(shared_ptr<TransientTopicImpl> impl) :
        _impl(move(impl))
    {
    }

    void
    forward(EventDataSeq v, const Ice::Current&) override
    {
        _impl->publish(true, move(v));
    }

private:

    const shared_ptr<TransientTopicImpl> _impl;
};

}

shared_ptr<TransientTopicImpl>
TransientTopicImpl::create(const shared_ptr<Instance>& instance, const std::string& name, const Ice::Identity& id)
{
    shared_ptr<TransientTopicImpl> topicImpl(new TransientTopicImpl(instance, name, id));

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
        pubid.category = name;
        pubid.name = "publish";
        linkid.category = name;
        linkid.name = "link";
    }
    else
    {
        pubid.category = id.category;
        pubid.name = name + ".publish";
        linkid.category = id.category;
        linkid.name = name + ".link";
    }

    auto publisher = make_shared<TransientPublisherI>(topicImpl);
    topicImpl->_publisherPrx = instance->publishAdapter()->add(publisher, pubid);
    auto topicLink = make_shared<TransientTopicLinkI>(topicImpl);
    topicImpl->_linkPrx = Ice::uncheckedCast<TopicLinkPrx>(instance->publishAdapter()->add(topicLink, linkid));

    return topicImpl;
}

TransientTopicImpl::TransientTopicImpl(shared_ptr<Instance> instance,
                                       const std::string& name,
                                       const Ice::Identity& id) :
    _instance(move(instance)),
    _name(name),
    _id(id),
    _destroyed(false)
{
}

string
TransientTopicImpl::getName(const Ice::Current&) const
{
    // Immutable
    return _name;
}

shared_ptr<Ice::ObjectPrx>
TransientTopicImpl::getPublisher(const Ice::Current&) const
{
    // Immutable
    return _publisherPrx;
}

shared_ptr<Ice::ObjectPrx>
TransientTopicImpl::getNonReplicatedPublisher(const Ice::Current&) const
{
    // Immutable
    return _publisherPrx;
}

shared_ptr<Ice::ObjectPrx>
TransientTopicImpl::subscribeAndGetPublisher(QoS qos, shared_ptr<Ice::ObjectPrx> obj, const Ice::Current&)
{
    if(!obj)
    {
        auto traceLevels = _instance->traceLevels();
        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": subscribe: null proxy";
        }
        throw InvalidSubscriber("subscriber is a null proxy");
    }
    Ice::Identity id = obj->ice_getIdentity();

    auto traceLevels = _instance->traceLevels();
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

    lock_guard<mutex> lg(_mutex);

    SubscriberRecord record;
    record.id = id;
    record.obj = obj;
    record.theQoS = qos;
    record.topicName = _name;
    record.link = false;
    record.cost = 0;

    if(find(_subscribers.begin(), _subscribers.end(), record.id) != _subscribers.end())
    {
        throw AlreadySubscribed();
    }

    auto subscriber = Subscriber::create(_instance, record);
    _subscribers.push_back(subscriber);

    return subscriber->proxy();
}

void
TransientTopicImpl::unsubscribe(shared_ptr<Ice::ObjectPrx> subscriber, const Ice::Current&)
{
    auto traceLevels = _instance->traceLevels();
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

    lock_guard<mutex> lg(_mutex);

    // First remove the subscriber from the subscribers list. Note
    // that its possible that the subscriber isn't in the list, but is
    // in the database if the subscriber was locally reaped.
    auto p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p != _subscribers.end())
    {
        (*p)->destroy();
        _subscribers.erase(p);
    }
}

shared_ptr<TopicLinkPrx>
TransientTopicImpl::getLinkProxy(const Ice::Current&)
{
    // immutable
    return _linkPrx;
}

void
TransientTopicImpl::link(shared_ptr<TopicPrx> topic, int cost, const Ice::Current&)
{
    auto internal = Ice::uncheckedCast<TopicInternalPrx>(topic);
    auto link = internal->getLinkProxy();

    auto traceLevels = _instance->traceLevels();
    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": link " << _instance->communicator()->identityToString(topic->ice_getIdentity())
            << " cost " << cost;
    }

    lock_guard<mutex> lg(_mutex);

    auto id = topic->ice_getIdentity();

    SubscriberRecord record;
    record.id = id;
    record.obj = link;
    record.theTopic = topic;
    record.topicName = _name;
    record.link = true;
    record.cost = cost;

    if(find(_subscribers.begin(), _subscribers.end(), record.id) != _subscribers.end())
    {
        throw LinkExists(IceStormInternal::identityToTopicName(id));
    }

    auto subscriber = Subscriber::create(_instance, record);
    _subscribers.push_back(subscriber);
}

void
TransientTopicImpl::unlink(shared_ptr<TopicPrx> topic, const Ice::Current&)
{
    lock_guard<mutex> lg(_mutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    auto id = topic->ice_getIdentity();
    auto traceLevels = _instance->traceLevels();

    if(find(_subscribers.begin(), _subscribers.end(), id) == _subscribers.end())
    {
        string name = IceStormInternal::identityToTopicName(id);

        if(traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": unlink " << name << " failed - not linked";
        }
        throw NoSuchLink(name);
    }

    if(traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << " unlink " << _instance->communicator()->identityToString(id);
    }

    // Remove the subscriber from the subscribers list. Note
    // that its possible that the subscriber isn't in the list, but is
    // in the database if the subscriber was locally reaped.
    auto p = find(_subscribers.begin(), _subscribers.end(), id);
    if(p != _subscribers.end())
    {
        (*p)->destroy();
        _subscribers.erase(p);
    }
}

LinkInfoSeq
TransientTopicImpl::getLinkInfoSeq(const Ice::Current&) const
{
    lock_guard<mutex> lg(_mutex);

    LinkInfoSeq seq;
    for(const auto& subscriber : _subscribers)
    {
        SubscriberRecord record = subscriber->record();
        if(record.link && !subscriber->errored())
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
    lock_guard<mutex> lg(_mutex);

    Ice::IdentitySeq subscribers;
    for(const auto& subscriber : _subscribers)
    {
        subscribers.push_back(subscriber->id());
    }
    return subscribers;
}

void
TransientTopicImpl::destroy(const Ice::Current&)
{
    lock_guard<mutex> lg(_mutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    _destroyed = true;

    auto traceLevels = _instance->traceLevels();
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
    for(const auto& subscriber : _subscribers)
    {
        subscriber->destroy();
    }
    _subscribers.clear();
}

void
TransientTopicImpl::reap(Ice::IdentitySeq, const Ice::Current&)
{
}

bool
TransientTopicImpl::destroyed() const
{
    lock_guard<mutex> lg(_mutex);
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
    vector<shared_ptr<Subscriber>> copy;
    {
        lock_guard<mutex> lg(_mutex);
        copy = _subscribers;
    }

    //
    // Queue each event, gathering a list of those subscribers that
    // must be reaped.
    //
    vector<Ice::Identity> ids;
    for(const auto& subscriber : copy)
    {
        if(!subscriber->queue(forwarded, events) && subscriber->reap())
        {
            ids.push_back(subscriber->id());
        }
    }

    //
    // Run through the error list removing those subscribers that are
    // in error from the subscriber list.
    //
    if(!ids.empty())
    {
        lock_guard<mutex> lg(_mutex);
        for(const auto& id : ids)
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
            auto q = find(_subscribers.begin(), _subscribers.end(), id);
            if(q != _subscribers.end())
            {
                //
                // Destroy the subscriber.
                //
                (*q)->destroy();
                _subscribers.erase(q);
            }
        }
    }
}

void
TransientTopicImpl::shutdown()
{
    lock_guard<mutex> lg(_mutex);

    // Shutdown each subscriber. This waits for the event queues to drain.
    for(const auto& subscriber : _subscribers)
    {
        subscriber->shutdown();
    }
}
