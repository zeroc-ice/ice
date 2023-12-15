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
logError(const Ice::CommunicatorPtr& com, const IceDB::LMDBException& ex)
{
    Ice::Error error(com->getLogger());
    error << "LMDB error: " << ex;
}

class TopicManagerI : public TopicManagerInternal
{
public:

    TopicManagerI(const PersistentInstancePtr& instance, const TopicManagerImplPtr& impl) :
        _instance(instance), _impl(impl)
    {
    }

    virtual TopicPrx create(const string& id, const Ice::Current&)
    {
        while(true)
        {
            Ice::Long generation;
            TopicManagerPrx master = getMaster(generation, __FILE__, __LINE__);
            if(master)
            {
                try
                {
                    return master->create(id);
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
                return _impl->create(id);
            }
        }
    }

    virtual TopicPrx retrieve(const string& id, const Ice::Current&) const
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->retrieve(id);
    }

    virtual TopicDict retrieveAll(const Ice::Current&) const
    {
        // Use cached reads.
        CachedReadHelper unlock(_instance->node(), __FILE__, __LINE__);
        return _impl->retrieveAll();
    }

    virtual NodePrx getReplicaNode(const Ice::Current&) const
    {
        // This doesn't require the replication to be running.
        return _instance->nodeProxy();
    }

private:

    TopicManagerPrx getMaster(Ice::Long& generation, const char* file, int line) const
    {
        NodeIPtr node = _instance->node();
        if(node)
        {
            return TopicManagerPrx::uncheckedCast(node->startUpdate(generation, file, line));
        }
        else
        {
            return TopicManagerPrx();
        }
    }

    const PersistentInstancePtr _instance;
    const TopicManagerImplPtr _impl;
};

class ReplicaObserverI : public ReplicaObserver
{
public:

    ReplicaObserverI(const PersistentInstancePtr& instance, const TopicManagerImplPtr& impl) :
        _instance(instance),
        _impl(impl)
    {
    }

    virtual void init(const LogUpdate& llu, const TopicContentSeq& content, const Ice::Current&)
    {
        NodeIPtr node = _instance->node();
        if(node)
        {
            node->checkObserverInit(llu.generation);
        }
        _impl->observerInit(llu, content);
    }

    virtual void createTopic(const LogUpdate& llu, const string& name, const Ice::Current&)
    {
        try
        {
            ObserverUpdateHelper unlock(_instance->node(), llu.generation, __FILE__, __LINE__);
            _impl->observerCreateTopic(llu, name);
        }
        catch(const ObserverInconsistencyException& e)
        {
            Ice::Warning warn(_instance->traceLevels()->logger);
            warn << "ReplicaObserverI::create: ObserverInconsistencyException: " << e.reason;
            _instance->node()->recovery(llu.generation);
            throw;
        }
    }

    virtual void destroyTopic(const LogUpdate& llu, const string& name, const Ice::Current&)
    {
        try
        {
            ObserverUpdateHelper unlock(_instance->node(), llu.generation, __FILE__, __LINE__);
            _impl->observerDestroyTopic(llu, name);
        }
        catch(const ObserverInconsistencyException& e)
        {
            Ice::Warning warn(_instance->traceLevels()->logger);
            warn << "ReplicaObserverI::destroy: ObserverInconsistencyException: " << e.reason;
            _instance->node()->recovery(llu.generation);
            throw;
        }
    }

    virtual void addSubscriber(const LogUpdate& llu, const string& name, const SubscriberRecord& rec,
                               const Ice::Current&)
    {
        try
        {
            ObserverUpdateHelper unlock(_instance->node(), llu.generation, __FILE__, __LINE__);
            _impl->observerAddSubscriber(llu, name, rec);
        }
        catch(const ObserverInconsistencyException& e)
        {
            Ice::Warning warn(_instance->traceLevels()->logger);
            warn << "ReplicaObserverI::add: ObserverInconsistencyException: " << e.reason;
            _instance->node()->recovery(llu.generation);
            throw;
        }
    }

    virtual void removeSubscriber(const LogUpdate& llu, const string& name, const Ice::IdentitySeq& id,
                                  const Ice::Current&)
    {
        try
        {
            ObserverUpdateHelper unlock(_instance->node(), llu.generation, __FILE__, __LINE__);
            _impl->observerRemoveSubscriber(llu, name, id);
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

    const PersistentInstancePtr _instance;
    const TopicManagerImplPtr _impl;
};

class TopicManagerSyncI : public TopicManagerSync
{
public:

    TopicManagerSyncI(const TopicManagerImplPtr& impl) :
        _impl(impl)
    {
    }

    virtual void getContent(LogUpdate& llu, TopicContentSeq& content, const Ice::Current&)
    {
        _impl->getContent(llu, content);
    }

private:

    const TopicManagerImplPtr _impl;
};

}

TopicManagerImpl::TopicManagerImpl(const PersistentInstancePtr& instance) :
    _instance(instance),
    _lluMap(instance->lluMap()),
    _subscriberMap(instance->subscriberMap())
{
    try
    {
        __setNoDelete(true);

        if(_instance->observer())
        {
            _instance->observer()->setObserverUpdater(this);
        }

        // TODO: If we want to improve the performance of the
        // non-replicated case we could allocate a null-topic manager impl
        // here.
        _managerImpl = new TopicManagerI(instance, this);

        // If there is no node adapter we don't need to start the
        // observer, nor sync since we're not replicating.
        if(_instance->nodeAdapter())
        {
            _observerImpl = new ReplicaObserverI(instance, this);
            _observer = _instance->nodeAdapter()->addWithUUID(_observerImpl);
            _syncImpl = new TopicManagerSyncI(this);
            _sync = _instance->nodeAdapter()->addWithUUID(_syncImpl);
        }

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
    }
    catch(...)
    {
        shutdown();
        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
}

TopicPrx
TopicManagerImpl::create(const string& name)
{
    Lock sync(*this);

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

TopicPrx
TopicManagerImpl::retrieve(const string& name) const
{
    Lock sync(*this);

    TopicManagerImpl* This = const_cast<TopicManagerImpl*>(this);
    This->reap();

    map<string, TopicImplPtr>::const_iterator p = _topics.find(name);
    if(p == _topics.end())
    {
        throw NoSuchTopic(name);
    }

    return p->second->proxy();
}

TopicDict
TopicManagerImpl::retrieveAll() const
{
    Lock sync(*this);

    TopicManagerImpl* This = const_cast<TopicManagerImpl*>(this);
    This->reap();

    TopicDict all;
    for(map<string, TopicImplPtr>::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
    {
        all.insert(TopicDict::value_type(p->first, p->second->proxy()));
    }

    return all;
}

void
TopicManagerImpl::observerInit(const LogUpdate& llu, const TopicContentSeq& content)
{
    Lock sync(*this);

    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topicMgr > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
        out << "init";
        for(TopicContentSeq::const_iterator p = content.begin(); p != content.end(); ++p)
        {
            out << " topic: " << _instance->communicator()->identityToString(p->id) << " subscribers: ";
            for(SubscriberRecordSeq::const_iterator q = p->records.begin(); q != p->records.end(); ++q)
            {
                if(q != p->records.begin())
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
        {
            SubscriberRecordKey srkey;
            srkey.topic = p->id;
            SubscriberRecord rec;
            rec.link = false;
            rec.cost = 0;

            _subscriberMap.put(txn, srkey, rec);

            for(SubscriberRecordSeq::const_iterator q = p->records.begin(); q != p->records.end(); ++q)
            {
                SubscriberRecordKey key;
                key.topic = p->id;
                key.id = q->id;

                _subscriberMap.put(txn, key, *q);
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

    map<string, TopicImplPtr>::iterator p = _topics.begin();
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
    for(TopicContentSeq::const_iterator q = content.begin(); q != content.end(); ++q)
    {
        string name = identityToTopicName(q->id);
        map<string, TopicImplPtr>::const_iterator r = _topics.find(name);
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
    Lock sync(*this);
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
    Lock sync(*this);

    map<string, TopicImplPtr>::iterator q = _topics.find(name);
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
    TopicImplPtr topic;
    {
        Lock sync(*this);

        map<string, TopicImplPtr>::iterator q = _topics.find(name);
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
    TopicImplPtr topic;
    {
        Lock sync(*this);

        map<string, TopicImplPtr>::iterator q = _topics.find(name);
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
        Lock sync(*this);
        reap();
    }

    try
    {
        content.clear();
        for(map<string, TopicImplPtr>::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
        {
            TopicContent rec = p->second->getContent();
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
TopicManagerImpl::sync(const Ice::ObjectPrx& master)
{
    TopicManagerSyncPrx sync = TopicManagerSyncPrx::uncheckedCast(master);

    LogUpdate llu;
    TopicContentSeq content;
    sync->getContent(llu, content);

    observerInit(llu, content);
}

void
TopicManagerImpl::initMaster(const set<GroupNodeInfo>& slaves, const LogUpdate& llu)
{
    Lock sync(*this);

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

        for(map<string, TopicImplPtr>::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
        {
            TopicContent rec = p->second->getContent();
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

Ice::ObjectPrx
TopicManagerImpl::getObserver() const
{
    return _observer;
}

Ice::ObjectPrx
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
    // Lock sync(*this);
    //
    vector<string> reaped = _instance->topicReaper()->consumeReapedTopics();
    for(vector<string>::const_iterator p = reaped.begin(); p != reaped.end(); ++p)
    {
        map<string, TopicImplPtr>::iterator q = _topics.find(*p);
        if(q != _topics.end() && q->second->destroyed())
        {
            _topics.erase(q);
        }
    }
}

void
TopicManagerImpl::shutdown()
{
    Lock sync(*this);

    for(map<string, TopicImplPtr>::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
    {
        p->second->shutdown();
    }
    _topics.clear();

    _observerImpl = 0;
    _syncImpl = 0;
    _managerImpl = 0;
}

Ice::ObjectPtr
TopicManagerImpl::getServant() const
{
    return _managerImpl;
}

void
TopicManagerImpl::updateTopicObservers()
{
    Lock sync(*this);
    for(map<string, TopicImplPtr>::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
    {
        p->second->updateObserver();
    }
}

void
TopicManagerImpl::updateSubscriberObservers()
{
    Lock sync(*this);
    for(map<string, TopicImplPtr>::const_iterator p = _topics.begin(); p != _topics.end(); ++p)
    {
        p->second->updateSubscriberObservers();
    }
}

TopicPrx
TopicManagerImpl::installTopic(const string& name, const Ice::Identity& id, bool create,
                               const IceStorm::SubscriberRecordSeq& subscribers)
{
    //
    // Called by constructor or with 'this' mutex locked.
    //
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->topicMgr > 0)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->topicMgrCat);
        if(create)
        {
            out << "creating new topic \"" << name << "\". id: "
                << _instance->communicator()->identityToString(id)
                << " subscribers: ";
            for(SubscriberRecordSeq::const_iterator q = subscribers.begin(); q != subscribers.end(); ++q)
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
            for(SubscriberRecordSeq::const_iterator q = subscribers.begin(); q != subscribers.end(); ++q)
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
    TopicImplPtr topicImpl = new TopicImpl(_instance, name, id, subscribers);

    // The identity is the name of the Topic.
    _topics.insert(map<string, TopicImplPtr>::value_type(name, topicImpl));
    _instance->topicAdapter()->add(topicImpl->getServant(), id);
    return topicImpl->proxy();
}
