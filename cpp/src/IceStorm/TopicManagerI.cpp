//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TopicI.h>
#include <IceStorm/TraceLevels.h>
#include <IceStorm/Instance.h>
#include <IceStorm/NodeI.h>
#include <IceStorm/Observers.h>
#include <IceStorm/Subscriber.h>
#include <IceStorm/Util.h>

#include <functional>

using namespace std;
using namespace IceStorm;
using namespace IceStormElection;
using namespace IceStormInternal;

namespace
{

void
logError(const shared_ptr<Ice::Communicator>& com, const IceDB::LMDBException& ex)
{
    Ice::Error error(com->getLogger());
    error << "LMDB error: " << ex;
}

class TopicManagerI final : public TopicManagerInternal
{
public:

    TopicManagerI(shared_ptr<PersistentInstance> instance, shared_ptr<TopicManagerImpl> impl) :
        _instance(move(instance)), _impl(move(impl))
    {
    }

    shared_ptr<TopicPrx> create(string id, const Ice::Current&) override
    {
        while(true)
        {
            long long generation;
            auto master = getMaster(generation, __FILE__, __LINE__);
            if(master)
            {
                try
                {
                    return master->create(move(id));
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
                return _impl->create(move(id));
            }
        }
    }

    shared_ptr<TopicPrx> retrieve(string id, const Ice::Current&) override
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->retrieve(move(id));
    }

    TopicDict retrieveAll(const Ice::Current&) override
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->retrieveAll();
    }

    shared_ptr<NodePrx> getReplicaNode(const Ice::Current&) const override
    {
        // This doesn't require the replication to be running.
        return _instance->nodeProxy();
    }

private:

    shared_ptr<TopicManagerPrx> getMaster(long long& generation, const char* file, int line) const
    {
        auto node = _instance->node();
        if(node)
        {
            return Ice::uncheckedCast<TopicManagerPrx>(node->startUpdate(generation, file, line));
        }
        else
        {
            return nullptr;
        }
    }

    const shared_ptr<PersistentInstance> _instance;
    const shared_ptr<TopicManagerImpl> _impl;
};

class ReplicaObserverI final : public ReplicaObserver
{
public:

    ReplicaObserverI(shared_ptr<PersistentInstance> instance, shared_ptr<TopicManagerImpl> impl) :
        _instance(move(instance)),
        _impl(move(impl))
    {
    }

    void init(LogUpdate llu, TopicContentSeq content, const Ice::Current&) override
    {
        auto node = _instance->node();
        if(node)
        {
            node->checkObserverInit(llu.generation);
        }
        _impl->observerInit(move(llu), move(content));
    }

    void createTopic(LogUpdate llu, string name, const Ice::Current&) override
    {
        try
        {
            ObserverUpdateHelper unlock(_instance->node(), llu.generation, __FILE__, __LINE__);
            _impl->observerCreateTopic(llu, move(name));
        }
        catch(const ObserverInconsistencyException& e)
        {
            Ice::Warning warn(_instance->traceLevels()->logger);
            warn << "ReplicaObserverI::create: ObserverInconsistencyException: " << e.reason;
            _instance->node()->recovery(llu.generation);
            throw;
        }
    }

    void destroyTopic(LogUpdate llu, string name, const Ice::Current&) override
    {
        try
        {
            ObserverUpdateHelper unlock(_instance->node(), llu.generation, __FILE__, __LINE__);
            _impl->observerDestroyTopic(llu, move(name));
        }
        catch(const ObserverInconsistencyException& e)
        {
            Ice::Warning warn(_instance->traceLevels()->logger);
            warn << "ReplicaObserverI::destroy: ObserverInconsistencyException: " << e.reason;
            _instance->node()->recovery(llu.generation);
            throw;
        }
    }

    void addSubscriber(LogUpdate llu, string name, SubscriberRecord rec, const Ice::Current&) override
    {
        try
        {
            ObserverUpdateHelper unlock(_instance->node(), llu.generation, __FILE__, __LINE__);
            _impl->observerAddSubscriber(llu, move(name), move(rec));
        }
        catch(const ObserverInconsistencyException& e)
        {
            Ice::Warning warn(_instance->traceLevels()->logger);
            warn << "ReplicaObserverI::add: ObserverInconsistencyException: " << e.reason;
            _instance->node()->recovery(llu.generation);
            throw;
        }
    }

    void removeSubscriber(LogUpdate llu, string name, Ice::IdentitySeq id, const Ice::Current&) override
    {
        try
        {
            ObserverUpdateHelper unlock(_instance->node(), llu.generation, __FILE__, __LINE__);
            _impl->observerRemoveSubscriber(llu, move(name), move(id));
        }
        catch(const ObserverInconsistencyException& e)
        {
            Ice::Warning warn(_instance->traceLevels()->logger);
            warn << "ReplicaObserverI::remove: ObserverInconsistencyException: " << e.reason;
            _instance->node()->recovery(llu.generation);
            throw;
        }
    }

private:

    const shared_ptr<PersistentInstance> _instance;
    const shared_ptr<TopicManagerImpl> _impl;
};

class TopicManagerSyncI final : public TopicManagerSync
{
public:

    TopicManagerSyncI(shared_ptr<TopicManagerImpl> impl) :
        _impl(move(impl))
    {
    }

    void getContent(LogUpdate& llu, TopicContentSeq& content, const Ice::Current&) override
    {
        _impl->getContent(llu, content);
    }

private:

    const shared_ptr<TopicManagerImpl> _impl;
};

}

shared_ptr<TopicManagerImpl>
TopicManagerImpl::create(const std::shared_ptr<PersistentInstance>& instance)
{
    shared_ptr<TopicManagerImpl> manager(new TopicManagerImpl(instance));

    if(instance->observer())
    {
        instance->observer()->setObserverUpdater(manager);
    }

    manager->_managerImpl = make_shared<TopicManagerI>(instance, manager);

    // If there is no node adapter we don't need to start the
    // observer, nor sync since we're not replicating.
    if(instance->nodeAdapter())
    {
        auto observerImpl = make_shared<ReplicaObserverI>(instance, manager);
        manager->_observer = instance->nodeAdapter()->addWithUUID(observerImpl);
        auto syncImpl = make_shared<TopicManagerSyncI>(manager);
        manager->_sync = instance->nodeAdapter()->addWithUUID(syncImpl);
    }

    return manager;
}

TopicManagerImpl::TopicManagerImpl(shared_ptr<PersistentInstance> instance) :
    _instance(move(instance)),
    _lluMap(_instance->lluMap()),
    _subscriberMap(_instance->subscriberMap())
{
    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        // Ensure that the llu counter is present in the log.
        LogUpdate empty = {0, 0};
        _instance->lluMap().put(txn, lluDbKey, empty);

        // Recreate each of the topics.
        SubscriberRecordKey k;
        SubscriberRecord v;

        SubscriberMapRWCursor cursor(_subscriberMap, txn);
        if(cursor.get(k, v, MDB_FIRST))
        {
            bool moreTopics = false;
            do
            {
                // This record has to be a place holder record, otherwise
                // there is a database bug.
                assert(k.id.name.empty() && k.id.category.empty());

                Ice::Identity topic = k.topic;

                SubscriberRecordSeq content;
                while((moreTopics = cursor.get(k, v, MDB_NEXT)) == true && k.topic == topic)
                {
                    content.push_back(v);
                }

                string name = identityToTopicName(topic);
                installTopic(name, topic, false, content);
            } while(moreTopics);
        }

        txn.commit();
    }
    catch(const std::exception&)
    {
        shutdown();
        throw;
    }
}

shared_ptr<TopicPrx>
TopicManagerImpl::create(const string& name)
{
    lock_guard<recursive_mutex> lg(_mutex);

    reap();
    if(_topics.find(name) != _topics.end())
    {
        throw TopicExists(name);
    }

    // Identity is <instanceName>/topic.<topicname>
    Ice::Identity id = nameToIdentity(_instance, name);

    LogUpdate llu;
    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        SubscriberRecordKey key;
        key.topic = id;
        SubscriberRecord rec;
        rec.link = false;
        rec.cost = 0;

        _subscriberMap.put(txn, key, rec);

        llu = getIncrementedLLU(txn, _lluMap);

        txn.commit();
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    _instance->observers()->createTopic(llu, name);
    return installTopic(name, id, true);
}

shared_ptr<TopicPrx>
TopicManagerImpl::retrieve(const string& name)
{
    lock_guard<recursive_mutex> lg(_mutex);

    reap();

    auto p = _topics.find(name);
    if(p == _topics.end())
    {
        throw NoSuchTopic(name);
    }

    return p->second->proxy();
}

TopicDict
TopicManagerImpl::retrieveAll()
{
    lock_guard<recursive_mutex> lg(_mutex);

    reap();

    TopicDict all;
    for(auto p = _topics.begin(); p != _topics.end(); ++p)
    {
        all.insert({ p->first, p->second->proxy() });
    }

    return all;
}

void
TopicManagerImpl::observerInit(const LogUpdate& llu, const TopicContentSeq& content)
{
    lock_guard<recursive_mutex> lg(_mutex);

    auto traceLevels = _instance->traceLevels();
    if(traceLevels->topicMgr > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
        out << "init";
        for(const auto& c : content)
        {
            out << " topic: " << _instance->communicator()->identityToString(c.id) << " subscribers: ";
            for(auto q = c.records.cbegin(); q != c.records.cend(); ++q)
            {
                if(q != c.records.begin())
                {
                    out << ",";
                }
                out << _instance->communicator()->identityToString(q->id);
                if(traceLevels->topicMgr > 1)
                {
                    out << " endpoints: " << IceStormInternal::describeEndpoints(q->obj);
                }
            }
        }
    }

    // First we update the database state, and then we update our
    // internal state.
    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        _lluMap.put(txn, lluDbKey, llu);

        _subscriberMap.clear(txn);

        for(TopicContentSeq::const_iterator p = content.begin(); p != content.end(); ++p)
        for(const auto& c : content)
        {
            SubscriberRecordKey srkey;
            srkey.topic = c.id;
            SubscriberRecord rec;
            rec.link = false;
            rec.cost = 0;

            _subscriberMap.put(txn, srkey, rec);

            for(const auto& record : c.records)
            {
                SubscriberRecordKey key;
                key.topic = c.id;
                key.id = record.id;

                _subscriberMap.put(txn, key, record);
            }
        }
        txn.commit();
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    // We do this with two scans. The first runs through the topics
    // that we have and removes those not in the init list. The second
    // runs through the init list and either adds the ones that don't
    // exist, or updates those that do.

    auto p = _topics.begin();
    while(p != _topics.end())
    {
        TopicContentSeq::const_iterator q;
        for(q = content.begin(); q != content.end(); ++q)
        {
            if(q->id == p->second->id())
            {
                break;
            }
        }

        if(q == content.end())
        {
            // Note that this destroy should not remove anything from
            // the database since we've already synced up the db
            // state.
            //
            // TODO: We could short circuit the database operations in
            // the topic by calling a third form of destroy.
            p->second->observerDestroyTopic(llu);
            _topics.erase(p++);
        }
        else
        {
            ++p;
        }
    }

    // Now run through the contents updating the topics that do exist,
    // and creating those that do not.
    for(auto q = content.cbegin(); q != content.cend(); ++q)
    {
        string name = identityToTopicName(q->id);
        auto r = _topics.find(name);
        if(r == _topics.end())
        {
            installTopic(name, q->id, true, q->records);
        }
        else
        {
            r->second->update(q->records);
        }
    }
    // Clear the set of observers.
    _instance->observers()->clear();
}

void
TopicManagerImpl::observerCreateTopic(const LogUpdate& llu, const string& name)
{
    lock_guard<recursive_mutex> lg(_mutex);
    Ice::Identity id = nameToIdentity(_instance, name);

    try
    {
        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        SubscriberRecordKey key;
        key.topic = id;
        SubscriberRecord rec;
        rec.link = false;
        rec.cost = 0;

        if(_subscriberMap.find(txn, key))
        {
            throw ObserverInconsistencyException("topic exists: " + name);
        }
        _subscriberMap.put(txn, key, rec);

        _lluMap.put(txn, lluDbKey, llu);

        txn.commit();
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    installTopic(name, id, true);
}

void
TopicManagerImpl::observerDestroyTopic(const LogUpdate& llu, const string& name)
{
    lock_guard<recursive_mutex> lg(_mutex);

    auto q = _topics.find(name);
    if(q == _topics.end())
    {
        throw ObserverInconsistencyException("no topic: " + name);
    }
    q->second->observerDestroyTopic(llu);

    _topics.erase(q);
}

void
TopicManagerImpl::observerAddSubscriber(const LogUpdate& llu, const string& name, const SubscriberRecord& record)
{
    shared_ptr<TopicImpl> topic;
    {
        lock_guard<recursive_mutex> lg(_mutex);

        auto q = _topics.find(name);
        if(q == _topics.end())
        {
            throw ObserverInconsistencyException("no topic: " + name);
        }
        assert(q != _topics.end());
        topic = q->second;
    }
    topic->observerAddSubscriber(llu, record);
}

void
TopicManagerImpl::observerRemoveSubscriber(const LogUpdate& llu, const string& name, const Ice::IdentitySeq& id)
{
    shared_ptr<TopicImpl> topic;
    {
        lock_guard<recursive_mutex> lg(_mutex);

        auto q = _topics.find(name);
        if(q == _topics.end())
        {
            throw ObserverInconsistencyException("no topic: " + name);
        }
        assert(q != _topics.end());
        topic = q->second;
    }
    topic->observerRemoveSubscriber(llu, id);
}

void
TopicManagerImpl::getContent(LogUpdate& llu, TopicContentSeq& content)
{
    {
        lock_guard<recursive_mutex> lg(_mutex);
        reap();
    }

    try
    {
        content.clear();
        for(const auto& topic : _topics)
        {
            TopicContent rec = topic.second->getContent();
            content.push_back(rec);
        }

        IceDB::ReadOnlyTxn txn(_instance->dbEnv());
        _lluMap.get(txn, lluDbKey, llu);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }
}

LogUpdate
TopicManagerImpl::getLastLogUpdate() const
{
    LogUpdate llu;
    try
    {
        IceDB::ReadOnlyTxn txn(_instance->dbEnv());
        _lluMap.get(txn, lluDbKey, llu);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    return llu;
}

void
TopicManagerImpl::sync(const shared_ptr<Ice::ObjectPrx>& master)
{
    auto sync = Ice::uncheckedCast<TopicManagerSyncPrx>(master);

    LogUpdate llu;
    TopicContentSeq content;
    sync->getContent(llu, content);

    observerInit(llu, content);
}

void
TopicManagerImpl::initMaster(const set<GroupNodeInfo>& slaves, const LogUpdate& llu)
{
    lock_guard<recursive_mutex> lg(_mutex);

    reap();

    TopicContentSeq content;

    // Update the database llu. This prevents the following case:
    //
    // Three replicas 1, 2, 3. 3 is the master. It accepts a change
    // (say A=10, old value 9), writes to disk and then crashes. Now 2
    // becomes the master. The client can ask this master for A and it
    // returns 9. Now 3 comes back online, it has the last database
    // state, so it syncs this state with 1, 2. The client will now
    // magically get A==10. The solution here is when a new master is
    // elected and gets the latest database state it immediately
    // updates the llu stamp.
    //
    try
    {
        content.clear();

        IceDB::ReadWriteTxn txn(_instance->dbEnv());

        for(const auto& topic : _topics)
        {
            TopicContent rec = topic.second->getContent();
            content.push_back(rec);
        }

        _lluMap.put(txn, lluDbKey, llu);

        txn.commit();
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_instance->communicator(), ex);
        throw; // will become UnknownException in caller
    }

    // Now initialize the observers.
    _instance->observers()->init(slaves, llu, content);
}

shared_ptr<Ice::ObjectPrx>
TopicManagerImpl::getObserver() const
{
    return _observer;
}

shared_ptr<Ice::ObjectPrx>
TopicManagerImpl::getSync() const
{
    return _sync;
}

void
TopicManagerImpl::reap()
{
    //
    // Always called with mutex locked.
    //
    vector<string> reaped = _instance->topicReaper()->consumeReapedTopics();
    for(const auto& topic : reaped)
    {
        auto q = _topics.find(topic);
        if(q != _topics.end() && q->second->destroyed())
        {
            _topics.erase(q);
        }
    }
}

void
TopicManagerImpl::shutdown()
{
    lock_guard<recursive_mutex> lg(_mutex);

    for(const auto& topic : _topics)
    {
        topic.second->shutdown();
    }
    _topics.clear();

    _managerImpl = nullptr;
}

shared_ptr<Ice::Object>
TopicManagerImpl::getServant() const
{
    return _managerImpl;
}

void
TopicManagerImpl::updateTopicObservers()
{
    lock_guard<recursive_mutex> lg(_mutex);

    for(const auto& topic : _topics)
    {
        topic.second->updateObserver();
    }
}

void
TopicManagerImpl::updateSubscriberObservers()
{
    lock_guard<recursive_mutex> lg(_mutex);

    for(const auto& topic : _topics)
    {
        topic.second->updateSubscriberObservers();
    }
}

shared_ptr<TopicPrx>
TopicManagerImpl::installTopic(const string& name, const Ice::Identity& id, bool create,
                               const IceStorm::SubscriberRecordSeq& subscribers)
{
    //
    // Called by constructor or with 'this' mutex locked.
    //
    auto traceLevels = _instance->traceLevels();
    if(traceLevels->topicMgr > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
        if(create)
        {
            out << "creating new topic \"" << name << "\". id: "
                << _instance->communicator()->identityToString(id)
                << " subscribers: ";
            for(auto q = subscribers.cbegin(); q != subscribers.cend(); ++q)
            {
                if(q != subscribers.begin())
                {
                    out << ",";
                }
                if(traceLevels->topicMgr > 1)
                {
                    out << _instance->communicator()->identityToString(q->id)
                        << " endpoints: " << IceStormInternal::describeEndpoints(q->obj);
                }
            }
        }
        else
        {
            out << "loading topic \"" << name << "\" from database. id: "
                << _instance->communicator()->identityToString(id)
                << " subscribers: ";
            for(auto q = subscribers.cbegin(); q != subscribers.cend(); ++q)
            {
                if(q != subscribers.begin())
                {
                    out << ",";
                }
                if(traceLevels->topicMgr > 1)
                {
                    out << _instance->communicator()->identityToString(q->id)
                        << " endpoints: " << IceStormInternal::describeEndpoints(q->obj);
                }
            }
        }
    }

    // Create topic implementation
    auto topicImpl = TopicImpl::create(_instance, name, id, subscribers);

    // The identity is the name of the Topic.
    _topics.insert({ name, topicImpl });
    _instance->topicAdapter()->add(topicImpl->getServant(), id);
    return topicImpl->proxy();
}
