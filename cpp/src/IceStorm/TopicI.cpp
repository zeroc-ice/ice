// Copyright (c) ZeroC, Inc.

#include "TopicI.h"
#include "Ice/LoggerUtil.h"
#include "Instance.h"
#include "NodeI.h"
#include "Observers.h"
#include "Subscriber.h"
#include "TraceLevels.h"
#include "Util.h"

#include <algorithm>

using namespace std;
using namespace IceStorm;
using namespace IceStormElection;
using namespace IceStormInternal;

namespace
{
    void logError(const shared_ptr<Ice::Communicator>& com, const IceDB::LMDBException& ex)
    {
        Ice::Error error(com->getLogger());
        error << "LMDB error: " << ex;
    }

    //
    // The servant has a 1-1 association with a topic. It is used to
    // receive events from Publishers.
    //
    class PublisherI : public Ice::BlobjectArray
    {
    public:
        PublisherI(shared_ptr<TopicImpl> topic, shared_ptr<PersistentInstance> instance)
            : _topic(std::move(topic)),
              _instance(std::move(instance))
        {
        }

        bool ice_invoke(pair<const byte*, const byte*> inParams, Ice::ByteSeq&, const Ice::Current& current) override
        {
            // The publish call does a cached read.
            EventData event = {current.operation, current.mode, Ice::ByteSeq(), current.ctx};

            Ice::ByteSeq data(inParams.first, inParams.second);
            event.data = std::move(data);

            EventDataSeq v;
            v.push_back(std::move(event));
            _topic->publish(false, v);

            return true;
        }

    private:
        const shared_ptr<TopicImpl> _topic;
        const shared_ptr<PersistentInstance> _instance;
    };

    //
    // The servant has a 1-1 association with a topic. It is used to
    // receive events from linked Topics.
    //
    class TopicLinkI : public TopicLink
    {
    public:
        TopicLinkI(shared_ptr<TopicImpl> impl, shared_ptr<PersistentInstance> instance)
            : _impl(std::move(impl)),
              _instance(std::move(instance))
        {
        }

        void forward(EventDataSeq v, const Ice::Current&) override
        {
            // The publish call does a cached read.
            _impl->publish(true, v);
        }

    private:
        const shared_ptr<TopicImpl> _impl;
        const shared_ptr<PersistentInstance> _instance;
    };

    class TopicI : public TopicInternal
    {
    public:
        TopicI(shared_ptr<TopicImpl> impl, shared_ptr<PersistentInstance> instance)
            : _impl(std::move(impl)),
              _instance(std::move(instance))
        {
        }

        [[nodiscard]] string getName(const Ice::Current&) const override
        {
            // Use cached reads.
            CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
            return _impl->getName();
        }

        [[nodiscard]] optional<Ice::ObjectPrx> getPublisher(const Ice::Current&) const override
        {
            // Use cached reads.
            CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
            return _impl->getPublisher();
        }

        [[nodiscard]] optional<Ice::ObjectPrx> getNonReplicatedPublisher(const Ice::Current&) const override
        {
            // Use cached reads.
            CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
            return _impl->getNonReplicatedPublisher();
        }

        optional<Ice::ObjectPrx>
        subscribeAndGetPublisher(QoS qos, optional<Ice::ObjectPrx> obj, const Ice::Current& current) override
        {
            checkNotNull(obj, __FILE__, __LINE__, current);
            while (true)
            {
                int64_t generation = -1;
                auto master = getMasterFor(current, generation, __FILE__, __LINE__);
                if (master)
                {
                    try
                    {
                        // Don't move the parameters as we may need to retry.
                        return master->subscribeAndGetPublisher(qos, *obj);
                    }
                    catch (const Ice::ConnectFailedException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                    catch (const Ice::TimeoutException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                }
                else
                {
                    FinishUpdateHelper unlock(_instance->node());
                    return _impl->subscribeAndGetPublisher(std::move(qos), std::move(*obj));
                }
            }
        }

        void unsubscribe(optional<Ice::ObjectPrx> subscriber, const Ice::Current& current) override
        {
            checkNotNull(subscriber, __FILE__, __LINE__, current);
            while (true)
            {
                int64_t generation = -1;
                auto master = getMasterFor(current, generation, __FILE__, __LINE__);
                if (master)
                {
                    try
                    {
                        // Don't move the parameters as we may need to retry.
                        master->unsubscribe(*subscriber);
                    }
                    catch (const Ice::ConnectFailedException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                    catch (const Ice::TimeoutException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                }
                else
                {
                    FinishUpdateHelper unlock(_instance->node());
                    _impl->unsubscribe(*subscriber);
                }
                break;
            }
        }

        optional<TopicLinkPrx> getLinkProxy(const Ice::Current&) override
        {
            // Use cached reads.
            CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
            return _impl->getLinkProxy();
        }

        void reap(Ice::IdentitySeq ids, const Ice::Current&) override
        {
            auto node = _instance->node();
            if (!node->updateMaster(__FILE__, __LINE__))
            {
                throw ReapWouldBlock();
            }
            FinishUpdateHelper unlock(node);
            _impl->reap(ids);
        }

        void link(optional<TopicPrx> topic, int cost, const Ice::Current& current) override
        {
            while (true)
            {
                int64_t generation = -1;
                auto master = getMasterFor(current, generation, __FILE__, __LINE__);
                if (master)
                {
                    try
                    {
                        master->link(topic, cost);
                    }
                    catch (const Ice::ConnectFailedException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                    catch (const Ice::TimeoutException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                }
                else
                {
                    FinishUpdateHelper unlock(_instance->node());
                    _impl->link(topic.value(), cost);
                }
                break;
            }
        }

        void unlink(optional<TopicPrx> topic, const Ice::Current& current) override
        {
            while (true)
            {
                int64_t generation = -1;
                auto master = getMasterFor(current, generation, __FILE__, __LINE__);
                if (master)
                {
                    try
                    {
                        master->unlink(topic);
                    }
                    catch (const Ice::ConnectFailedException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                    catch (const Ice::TimeoutException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                }
                else
                {
                    FinishUpdateHelper unlock(_instance->node());
                    _impl->unlink(topic.value());
                }
                break;
            }
        }

        [[nodiscard]] LinkInfoSeq getLinkInfoSeq(const Ice::Current&) const override
        {
            // Use cached reads.
            CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
            return _impl->getLinkInfoSeq();
        }

        [[nodiscard]] Ice::IdentitySeq getSubscribers(const Ice::Current&) const override
        {
            return _impl->getSubscribers();
        }

        void destroy(const Ice::Current& current) override
        {
            while (true)
            {
                int64_t generation = -1;
                auto master = getMasterFor(current, generation, __FILE__, __LINE__);
                if (master)
                {
                    try
                    {
                        master->destroy();
                    }
                    catch (const Ice::ConnectFailedException&)
                    {
                        _instance->node()->recovery(generation);
                        continue;
                    }
                    catch (const Ice::TimeoutException&)
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
        optional<TopicPrx>
        getMasterFor(const Ice::Current& current, int64_t& generation, const char* file, int line) const
        {
            auto node = _instance->node();
            optional<Ice::ObjectPrx> master;
            if (node)
            {
                master = node->startUpdate(generation, file, line);
            }

            if (master)
            {
                return master->ice_identity<TopicPrx>(current.id);
            }
            else
            {
                return nullopt;
            }
        }

        const shared_ptr<TopicImpl> _impl;
        const shared_ptr<PersistentInstance> _instance;
    };
}

shared_ptr<TopicImpl>
TopicImpl::create(
    const shared_ptr<PersistentInstance>& instance,
    const string& name,
    const Ice::Identity& id,
    const SubscriberRecordSeq& subscribers)
{
    shared_ptr<TopicImpl> topicImpl(new TopicImpl(instance, name, id, subscribers));

    topicImpl->_servant = make_shared<TopicI>(topicImpl, instance);
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
    if (id.category.empty())
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

    auto publisher = make_shared<PublisherI>(topicImpl, instance);
    topicImpl->_publisherPrx = instance->publishAdapter()->add(publisher, pubid);
    auto topicLink = make_shared<TopicLinkI>(topicImpl, instance);
    topicImpl->_linkPrx = instance->publishAdapter()->add<TopicLinkPrx>(topicLink, linkid);

    return topicImpl;
}

TopicImpl::TopicImpl(
    shared_ptr<PersistentInstance> instance,
    string name,
    Ice::Identity id,
    const SubscriberRecordSeq& subscribers)
    : _instance(std::move(instance)),
      _name(std::move(name)),
      _id(std::move(id)),
      _lluMap(_instance->lluMap()),
      _subscriberMap(_instance->subscriberMap())
{
    try
    {
        //
        // Re-establish subscribers.
        //
        for (const auto& subscriber : subscribers)
        {
            Ice::Identity ident = subscriber.obj->ice_getIdentity();
            auto traceLevels = _instance->traceLevels();
            if (traceLevels->topic > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
                out << _name << " recreate " << _instance->communicator()->identityToString(ident);
                if (traceLevels->topic > 1)
                {
                    out << " endpoints: " << IceStormInternal::describeEndpoints(subscriber.obj);
                }
            }

            try
            {
                //
                // Create the subscriber object add it to the set of
                // subscribers.
                //
                _subscribers.push_back(Subscriber::create(_instance, subscriber));
            }
            catch (const Ice::Exception& ex)
            {
                Ice::Warning out(traceLevels->logger);
                out << _name << " recreate " << _instance->communicator()->identityToString(ident);
                if (traceLevels->topic > 1)
                {
                    out << " endpoints: " << IceStormInternal::describeEndpoints(subscriber.obj);
                }
                out << " failed: " << ex;
            }
        }

        if (_instance->observer())
        {
            _observer.attach(_instance->observer()->getTopicObserver(_instance->serviceName(), _name, nullptr));
        }
    }
    catch (const std::exception&)
    {
        shutdown();
        throw;
    }
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
    assert(_publisherPrx);
    if (_instance->publisherReplicaProxy())
    {
        return _instance->publisherReplicaProxy()->ice_identity(_publisherPrx->ice_getIdentity());
    }
    return *_publisherPrx;
}

Ice::ObjectPrx
TopicImpl::getNonReplicatedPublisher() const
{
    assert(_publisherPrx);
    // If there is an adapter id configured then we're using icegrid so create an indirect proxy, otherwise create a
    // direct proxy.
    if (!_publisherPrx->ice_getAdapterId().empty())
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
    void trace(Ice::Trace& out, const shared_ptr<PersistentInstance>& instance, const vector<shared_ptr<Subscriber>>& s)
    {
        out << '[';
        for (auto p = s.cbegin(); p != s.cend(); ++p)
        {
            if (p != s.begin())
            {
                out << ",";
            }
            out << instance->communicator()->identityToString((*p)->id());
        }
        out << "]";
    }
}

optional<Ice::ObjectPrx>
TopicImpl::subscribeAndGetPublisher(QoS qos, Ice::ObjectPrx obj)
{
    auto id = obj->ice_getIdentity();
    auto traceLevels = _instance->traceLevels();
    lock_guard lock(_subscribersMutex);
    if (traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": subscribeAndGetPublisher: " << _instance->communicator()->identityToString(id);

        if (traceLevels->topic > 1)
        {
            out << " endpoints: " << IceStormInternal::describeEndpoints(obj) << " QoS: ";
            for (auto p = qos.begin(); p != qos.end(); ++p)
            {
                if (p != qos.begin())
                {
                    out << ',';
                }
            }
            out << " subscriptions: ";
            trace(out, _instance, _subscribers);
        }
    }

    SubscriberRecord record;
    record.id = id;
    record.obj = obj;
    record.theQoS = qos;
    record.topicName = _name;
    record.link = false;
    record.cost = 0;

    if (find(_subscribers.begin(), _subscribers.end(), record.id) != _subscribers.end())
    {
        throw AlreadySubscribed();
    }

    LogUpdate llu;

    auto subscriber = Subscriber::create(_instance, record);
    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        SubscriberRecordKey key;
        key.topic = _id;
        key.id = subscriber->id();

        _subscriberMap.put(txn, key, record);

        llu = getIncrementedLLU(txn, _lluMap);

        txn.commit();
    }
    catch (const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    _subscribers.push_back(subscriber);

    _instance->observers()->addSubscriber(llu, _name, record);

    auto publisher = subscriber->proxy();
    assert(publisher); // The publisher is always non-null when the subscriber record link is false.
    return *publisher;
}

void
TopicImpl::unsubscribe(const Ice::ObjectPrx& subscriber)
{
    auto traceLevels = _instance->traceLevels();
    Ice::Identity id = subscriber->ice_getIdentity();

    lock_guard lock(_subscribersMutex);
    if (traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": unsubscribe: " << _instance->communicator()->identityToString(id);

        if (traceLevels->topic > 1)
        {
            out << " endpoints: " << IceStormInternal::describeEndpoints(subscriber);
            trace(out, _instance, _subscribers);
        }
    }
    removeSubscribers(Ice::IdentitySeq{id});
}

TopicLinkPrx
TopicImpl::getLinkProxy()
{
    // immutable
    assert(_linkPrx);
    optional<Ice::ObjectPrx> replica = _instance->publisherReplicaProxy();
    if (replica)
    {
        return replica->ice_identity<TopicLinkPrx>(_linkPrx->ice_getIdentity());
    }
    return *_linkPrx;
}

void
TopicImpl::link(const TopicPrx& topic, int cost)
{
    auto internal = Ice::uncheckedCast<TopicInternalPrx>(topic);
    optional<TopicLinkPrx> link = internal->getLinkProxy();
    assert(link);

    auto traceLevels = _instance->traceLevels();
    if (traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": link " << _instance->communicator()->identityToString(topic->ice_getIdentity()) << " cost "
            << cost;
    }

    lock_guard lock(_subscribersMutex);

    Ice::Identity id = topic->ice_getIdentity();

    SubscriberRecord record;
    record.id = id;
    record.obj = link;
    record.theTopic = topic;
    record.topicName = _name;
    record.link = true;
    record.cost = cost;

    if (find(_subscribers.begin(), _subscribers.end(), record.id) != _subscribers.end())
    {
        string name = IceStormInternal::identityToTopicName(id);
        throw LinkExists(name);
    }

    LogUpdate llu;

    auto subscriber = Subscriber::create(_instance, record);

    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        SubscriberRecordKey key;
        key.topic = _id;
        key.id = id;

        _subscriberMap.put(txn, key, record);

        llu = getIncrementedLLU(txn, _lluMap);

        txn.commit();
    }
    catch (const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    _subscribers.push_back(subscriber);

    _instance->observers()->addSubscriber(llu, _name, record);
}

void
TopicImpl::unlink(const TopicPrx& topic)
{
    lock_guard lock(_subscribersMutex);

    if (_destroyed)
    {
        throw Ice::ObjectNotExistException{__FILE__, __LINE__};
    }

    Ice::Identity id = topic->ice_getIdentity();

    auto traceLevels = _instance->traceLevels();

    if (find(_subscribers.begin(), _subscribers.end(), id) == _subscribers.end())
    {
        string name = IceStormInternal::identityToTopicName(id);

        if (traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _name << ": unlink " << name << " failed - not linked";
        }

        throw NoSuchLink(name);
    }

    if (traceLevels->topic > 0)
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
    lock_guard lock(_subscribersMutex);

    auto traceLevels = _instance->traceLevels();
    if (traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": reap ";
        for (auto p = ids.begin(); p != ids.end(); ++p)
        {
            if (p != ids.begin())
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
    lock_guard lock(_subscribersMutex);

    _servant = nullptr;

    // Shutdown each subscriber. This waits for the event queues to drain.
    for (const auto& subscriber : _subscribers)
    {
        subscriber->shutdown();
    }

    _observer.detach();
}

LinkInfoSeq
TopicImpl::getLinkInfoSeq() const
{
    lock_guard lock(_subscribersMutex);

    LinkInfoSeq seq;
    for (const auto& subscriber : _subscribers)
    {
        SubscriberRecord record = subscriber->record();
        if (record.link && !subscriber->errored())
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
TopicImpl::getSubscribers() const
{
    lock_guard lock(_subscribersMutex);

    Ice::IdentitySeq subscribers;
    for (const auto& subscriber : _subscribers)
    {
        subscribers.push_back(subscriber->id());
    }
    return subscribers;
}

void
TopicImpl::destroy()
{
    lock_guard lock(_subscribersMutex);

    if (_destroyed)
    {
        throw Ice::ObjectNotExistException{__FILE__, __LINE__};
    }
    _destroyed = true;

    auto traceLevels = _instance->traceLevels();
    if (traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": destroy";
    }

    // destroyInternal clears out the topic content.
    LogUpdate llu = {0, 0};
    _instance->observers()->destroyTopic(destroyInternal(llu, true), _name);

    _observer.detach();
}

TopicContent
TopicImpl::getContent() const
{
    lock_guard lock(_subscribersMutex);

    TopicContent content;
    content.id = _id;
    for (const auto& subscriber : _subscribers)
    {
        // Don't return errored subscribers (subscribers that have
        // errored out, but not reaped due to a failure with the
        // master). This means we can avoid the reaping step later.
        if (!subscriber->errored())
        {
            content.records.push_back(subscriber->record());
        }
    }
    return content;
}

void
TopicImpl::update(const SubscriberRecordSeq& records)
{
    lock_guard lock(_subscribersMutex);

    // We do this with two scans. The first runs through the subscribers
    // that we have and removes those not in the init list. The second
    // runs through the init list and add the ones that don't
    // exist.

    {
        auto p = _subscribers.begin();
        while (p != _subscribers.end())
        {
            SubscriberRecordSeq::const_iterator q;
            for (q = records.begin(); q != records.end(); ++q)
            {
                if ((*p)->id() == q->id)
                {
                    break;
                }
            }
            // The subscriber doesn't exist in the incoming subscriber
            // set so destroy it.
            if (q == records.end())
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

    for (const auto& record : records)
    {
        vector<shared_ptr<Subscriber>>::iterator q;
        for (q = _subscribers.begin(); q != _subscribers.end(); ++q)
        {
            if ((*q)->id() == record.id)
            {
                break;
            }
        }
        if (q == _subscribers.end())
        {
            auto subscriber = Subscriber::create(_instance, record);
            _subscribers.push_back(subscriber);
        }
    }
}

bool
TopicImpl::destroyed() const
{
    lock_guard lock(_subscribersMutex);

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
    optional<Ice::ObjectPrx> prx = _instance->topicReplicaProxy();
    if (prx)
    {
        return prx->ice_identity<TopicPrx>(_id);
    }
    else
    {
        return _instance->topicAdapter()->createProxy<TopicPrx>(_id);
    }
}

void
TopicImpl::publish(bool forwarded, const EventDataSeq& events)
{
    optional<TopicInternalPrx> masterInternal;
    int64_t generation = -1;
    Ice::IdentitySeq reap;
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);

        //
        // Copy of the subscriber list so that event publishing can occur
        // in parallel.
        //
        vector<shared_ptr<Subscriber>> copy;
        {
            lock_guard lock(_subscribersMutex);

            if (_observer)
            {
                if (forwarded)
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
        for (const auto& subscriber : copy)
        {
            if (!subscriber->queue(forwarded, events) && subscriber->reap())
            {
                reap.push_back(subscriber->id());
            }
        }

        // If there are no subscribers in error then we're done.
        if (reap.empty())
        {
            return;
        }
        if (!unlock.getMaster())
        {
            lock_guard lock(_subscribersMutex);
            removeSubscribers(reap);
            return;
        }
        masterInternal = unlock.getMaster()->ice_identity<TopicInternalPrx>(_id);
        generation = unlock.generation();
    }

    // Tell the master to reap this set of subscribers. This is an AMI invocation so it shouldn't block the caller (in
    // the typical case) we do it outside of the mutex lock for performance reasons.
    //
    // We must release the cached lock before calling this as the AMI call may raise an exception in the caller (that
    // is directly call ice_exception) which calls recover() on the node which would result in a deadlock since the
    // node is locked.

    masterInternal->reapAsync(
        reap,
        nullptr,
        [instance = _instance, generation](exception_ptr ex)
        {
            auto traceLevels = instance->traceLevels();
            if (traceLevels->topic > 0)
            {
                try
                {
                    rethrow_exception(ex);
                }
                catch (const std::exception& e)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
                    out << "exception when calling `reap' on the master replica: " << e;
                }
            }
            instance->node()->recovery(generation);
        });
}

void
TopicImpl::observerAddSubscriber(const LogUpdate& llu, const SubscriberRecord& record)
{
    lock_guard lock(_subscribersMutex);

    auto traceLevels = _instance->traceLevels();
    if (traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": add replica observer: " << _instance->communicator()->identityToString(record.id);

        if (traceLevels->topic > 1)
        {
            out << " endpoints: " << IceStormInternal::describeEndpoints(record.obj) << " QoS: ";
            for (auto p = record.theQoS.begin(); p != record.theQoS.end(); ++p)
            {
                if (p != record.theQoS.begin())
                {
                    out << ',';
                }
                out << '[' << p->first << "," << p->second << ']';
            }
        }
        out << " llu: " << llu.generation << "/" << llu.iteration;
    }

    if (find(_subscribers.begin(), _subscribers.end(), record.id) != _subscribers.end())
    {
        // If the subscriber is already in the database display a
        // diagnostic.
        if (traceLevels->topic > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
            out << _instance->communicator()->identityToString(record.id) << ": already subscribed";
        }
        return;
    }

    auto subscriber = Subscriber::create(_instance, record);
    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        SubscriberRecordKey key;
        key.topic = _id;
        key.id = subscriber->id();

        _subscriberMap.put(txn, key, record);

        // Update the LLU.
        _lluMap.put(txn, lluDbKey, llu);

        txn.commit();
    }
    catch (const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    _subscribers.push_back(subscriber);
}

void
TopicImpl::observerRemoveSubscriber(const LogUpdate& llu, const Ice::IdentitySeq& ids)
{
    auto traceLevels = _instance->traceLevels();
    if (traceLevels->topic > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicCat);
        out << _name << ": remove replica observer: ";
        for (auto id = ids.begin(); id != ids.end(); ++id)
        {
            if (id != ids.begin())
            {
                out << ",";
            }
            out << _instance->communicator()->identityToString(*id);
        }
        out << " llu: " << llu.generation << "/" << llu.iteration;
    }

    lock_guard lock(_subscribersMutex);

    // First remove from the database.
    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        for (const auto& id : ids)
        {
            SubscriberRecordKey key = {_id, id};
            _subscriberMap.del(txn, key);
        }

        _lluMap.put(txn, lluDbKey, llu);

        txn.commit();
    }
    catch (const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    // Then remove the subscriber from the subscribers list. If the
    // subscriber had a local failure and was removed from the
    // subscriber list it could already be gone. That's not a problem.
    for (const auto& id : ids)
    {
        auto p = find(_subscribers.begin(), _subscribers.end(), id);
        if (p != _subscribers.end())
        {
            (*p)->destroy();
            _subscribers.erase(p);
        }
    }
}

void
TopicImpl::observerDestroyTopic(const LogUpdate& llu)
{
    lock_guard lock(_subscribersMutex);

    if (_destroyed)
    {
        return;
    }
    _destroyed = true;

    auto traceLevels = _instance->traceLevels();
    if (traceLevels->topic > 0)
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
    lock_guard lock(_subscribersMutex);

    if (_instance->observer())
    {
        _observer.attach(_instance->observer()->getTopicObserver(_instance->serviceName(), _name, _observer.get()));
    }
}

void
TopicImpl::updateSubscriberObservers()
{
    lock_guard lock(_subscribersMutex);

    for (const auto& subscriber : _subscribers)
    {
        subscriber->updateObserver();
    }
}

LogUpdate
TopicImpl::destroyInternal(const LogUpdate& origLLU, bool master)
{
    // Clear out the database records related to this topic.
    LogUpdate llu;
    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        // Erase all subscriber records and the topic record.
        SubscriberRecordKey key;
        key.topic = _id;

        SubscriberMapRWCursor cursor(_subscriberMap, txn);
        if (cursor.find(key))
        {
            _subscriberMap.del(txn, key);

            SubscriberRecordKey k;
            SubscriberRecord v;
            while (cursor.get(k, v, MDB_NEXT) && k.topic == key.topic)
            {
                _subscriberMap.del(txn, k);
            }
        }

        // Update the LLU.
        if (master)
        {
            llu = getIncrementedLLU(txn, _lluMap);
        }
        else
        {
            llu = origLLU;
            _lluMap.put(txn, lluDbKey, llu);
        }

        txn.commit();
    }
    catch (const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    assert(_linkPrx);
    _instance->publishAdapter()->remove(_linkPrx->ice_getIdentity());

    assert(_publisherPrx);
    _instance->publishAdapter()->remove(_publisherPrx->ice_getIdentity());

    _instance->topicReaper()->add(_name);

    // Destroy each of the subscribers.
    for (const auto& subscriber : _subscribers)
    {
        subscriber->destroy();
    }
    _subscribers.clear();

    _instance->topicAdapter()->remove(_id);

    _servant = nullptr;

    return llu;
}

void
TopicImpl::removeSubscribers(const Ice::IdentitySeq& ids)
{
    // First update the database

    LogUpdate llu;
    bool found = false;
    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        for (const auto& id : ids)
        {
            SubscriberRecordKey key = {_id, id};

            if (_subscriberMap.del(txn, key))
            {
                found = true;
            }
        }

        if (found)
        {
            llu = getIncrementedLLU(txn, _lluMap);
            txn.commit();
        }
        else
        {
            txn.rollback();
        }
    }
    catch (const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    if (found)
    {
        // Then remove the subscriber from the subscribers list. Its
        // possible that some of these subscribers have already been
        // removed (consider, for example, a concurrent reap call from two
        // replicas on the same subscriber). To avoid sending unnecessary
        // observer updates keep track of the observers that are actually
        // removed.
        for (const auto& id : ids)
        {
            auto p = find(_subscribers.begin(), _subscribers.end(), id);
            if (p != _subscribers.end())
            {
                (*p)->destroy();
                _subscribers.erase(p);
            }
        }

        _instance->observers()->removeSubscriber(llu, _name, ids);
    }
}
