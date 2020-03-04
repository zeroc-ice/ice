//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <IceGrid/ReplicaCache.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/Topics.h>

using namespace std;
using namespace IceGrid;

ReplicaCache::ReplicaCache(const shared_ptr<Ice::Communicator>& communicator,
                           const shared_ptr<IceStorm::TopicManagerPrx>& topicManager) :
    _communicator(communicator)
{
    shared_ptr<IceStorm::TopicPrx> t;
    try
    {
        t = topicManager->create("ReplicaObserverTopic");
    }
    catch(const IceStorm::TopicExists&)
    {
        t = topicManager->retrieve("ReplicaObserverTopic");
    }

    const_cast<shared_ptr<IceStorm::TopicPrx>&>(_topic) =
        Ice::uncheckedCast<IceStorm::TopicPrx>(t->ice_endpoints(Ice::EndpointSeq()));
    const_cast<shared_ptr<ReplicaObserverPrx>&>(_observers) =
        Ice::uncheckedCast<ReplicaObserverPrx>(_topic->getPublisher()->ice_endpoints(Ice::EndpointSeq()));
}

shared_ptr<ReplicaEntry>
ReplicaCache::add(const string& name, const shared_ptr<ReplicaSessionI>& session)
{
    unique_lock lock(_mutex);

    shared_ptr<ReplicaEntry> entry;
    while((entry = getImpl(name)))
    {
        auto s = entry->getSession();
        if(s->isDestroyed())
        {
            // Wait for the session to be removed.
            _condVar.wait(lock);
        }
        else
        {
            //
            // Check if the replica is still reachable, if not, we
            // destroy its session.
            //
            lock.unlock();
            try
            {
                s->getInternalRegistry()->ice_ping();
                throw ReplicaActiveException();
            }
            catch(const Ice::LocalException&)
            {
                try
                {
                    s->destroy(Ice::emptyCurrent);
                }
                catch(const Ice::LocalException&)
                {
                }
            }
            lock.lock();
        }
    }

    if(_traceLevels && _traceLevels->replica > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
        out << "replica `" << name << "' up";
    }

    try
    {
        _observers->replicaAdded(session->getInternalRegistry());
    }
    catch(const Ice::NoEndpointException&)
    {
        // Expected if the replica is being shutdown.
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // Expected if the replica is being shutdown.
    }
    catch(const Ice::LocalException& ex)
    {
        auto traceLevels = getTraceLevels();
        if(traceLevels)
        {
            Ice::Warning out(traceLevels->logger);
            out << "unexpected exception while publishing `replicaAdded' update:\n" << ex;
        }
    }

    return addImpl(name, make_shared<ReplicaEntry>(name, session));
}

shared_ptr<ReplicaEntry>
ReplicaCache::remove(const string& name, bool shutdown)
{
    lock_guard lock(_mutex);

    auto entry = getImpl(name);
    assert(entry);
    removeImpl(name);
    _condVar.notify_all();

    if(_traceLevels && _traceLevels->replica > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
        out << "replica `" << name << "' down";
    }

    if(!shutdown)
    {
        try
        {
            _observers->replicaRemoved(entry->getProxy());
        }
        catch(const Ice::NoEndpointException&)
        {
            // Expected if the replica is being shutdown.
        }
        catch(const Ice::ObjectAdapterDeactivatedException&)
        {
            // Expected if the replica is being shutdown.
        }
        catch(const Ice::LocalException& ex)
        {
            auto traceLevels = getTraceLevels();
            if(traceLevels)
            {
                Ice::Warning out(traceLevels->logger);
                out << "unexpected exception while publishing `replicaRemoved' update:\n" << ex;
            }
        }
    }

    return entry;
}

shared_ptr<ReplicaEntry>
ReplicaCache::get(const string& name) const
{
    lock_guard lock(_mutex);
    auto entry = getImpl(name);
    if(!entry)
    {
        throw RegistryNotExistException(name);
    }
    return entry;
}

void
ReplicaCache::subscribe(const shared_ptr<ReplicaObserverPrx>& observer)
{
    try
    {
        lock_guard lock(_mutex);
        InternalRegistryPrxSeq replicas;
        for(const auto& entry : _entries)
        {
            replicas.push_back(entry.second->getProxy());
        }

        IceStorm::QoS qos;
        qos["reliability"] = "ordered";
        auto publisher = _topic->subscribeAndGetPublisher(qos, observer->ice_twoway());
        Ice::uncheckedCast<ReplicaObserverPrx>(publisher)->replicaInit(replicas);
    }
    catch(const Ice::NoEndpointException&)
    {
        // The replica is being shutdown.
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // Expected if the replica is being shutdown.
    }
    catch(const Ice::LocalException& ex)
    {
        auto traceLevels = getTraceLevels();
        if(traceLevels)
        {
            Ice::Warning out(traceLevels->logger);
            out << "unexpected exception while subscribing observer from replica observer topic:\n" << ex;
        }
    }
}

void
ReplicaCache::unsubscribe(const shared_ptr<ReplicaObserverPrx>& observer)
{
    try
    {
        _topic->unsubscribe(observer);
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // The replica is being shutdown.
    }
    catch(const Ice::NoEndpointException&)
    {
        // The replica is being shutdown.
    }
    catch(const Ice::LocalException& ex)
    {
        auto traceLevels = getTraceLevels();
        if(traceLevels)
        {
            Ice::Warning out(traceLevels->logger);
            out << "unexpected exception while unsubscribing observer from replica observer topic:\n" << ex;
        }
    }
}

shared_ptr<Ice::ObjectPrx>
ReplicaCache::getEndpoints(const string& name, const shared_ptr<Ice::ObjectPrx>& proxy) const
{
    Ice::EndpointSeq endpoints;

    if(proxy)
    {
        Ice::EndpointSeq endpts = proxy->ice_getEndpoints();
        endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
    }

    lock_guard lock(_mutex);
    for(const auto& entry : _entries)
    {
        auto prx = entry.second->getSession()->getEndpoint(name);
        if(prx)
        {
            Ice::EndpointSeq endpts = prx->ice_getEndpoints();
            endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
        }
    }

    return _communicator->stringToProxy("dummy")->ice_endpoints(endpoints);
}

void
ReplicaCache::setInternalRegistry(const shared_ptr<InternalRegistryPrx>& proxy)
{
    //
    // Setup this replica internal registry proxy.
    //
    _self = proxy;
}

shared_ptr<InternalRegistryPrx>
ReplicaCache::getInternalRegistry() const
{
    //
    // This replica internal registry proxy.
    //
    return _self;
}

ReplicaEntry::ReplicaEntry(const std::string& name, const shared_ptr<ReplicaSessionI>& session) :
    _name(name),
    _session(session)
{
}

const shared_ptr<ReplicaSessionI>&
ReplicaEntry::getSession() const
{
    return _session;
}

shared_ptr<InternalReplicaInfo>
ReplicaEntry::getInfo() const
{
    return _session->getInfo();
}

shared_ptr<InternalRegistryPrx>
ReplicaEntry::getProxy() const
{
    return _session->getInternalRegistry();
}

shared_ptr<Ice::ObjectPrx>
ReplicaEntry::getAdminProxy() const
{
    auto prx = getProxy();
    assert(prx);
    Ice::Identity adminId = { "RegistryAdmin-" + _name, prx->ice_getIdentity().category };
    return prx->ice_identity(move(adminId));
}
