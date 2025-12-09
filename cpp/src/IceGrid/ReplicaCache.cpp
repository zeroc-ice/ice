// Copyright (c) ZeroC, Inc.

#include "ReplicaCache.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "ReplicaSessionI.h"
#include "Topics.h"

using namespace std;
using namespace IceGrid;

namespace
{
    IceStorm::TopicPrx createOrRetrieveReplicaObserverTopic(const IceStorm::TopicManagerPrx& topicManager)
    {
        optional<IceStorm::TopicPrx> topic = topicManager->createOrRetrieve("ReplicaObserverTopic");
        return topic->ice_endpoints(Ice::EndpointSeq());
    }
}

ReplicaCache::ReplicaCache(
    const shared_ptr<Ice::Communicator>& communicator,
    const IceStorm::TopicManagerPrx& topicManager)
    : _communicator(communicator),
      _topic(createOrRetrieveReplicaObserverTopic(topicManager)),
      _observers(
          Ice::uncheckedCast<ReplicaObserverPrx>(_topic->getPublisher().value()->ice_endpoints(Ice::EndpointSeq())))
{
}

shared_ptr<ReplicaEntry>
ReplicaCache::add(const string& name, const shared_ptr<ReplicaSessionI>& session)
{
    unique_lock lock(_mutex);

    shared_ptr<ReplicaEntry> entry;
    while ((entry = getImpl(name)))
    {
        auto s = entry->getSession();
        if (s->isDestroyed())
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
            catch (const Ice::LocalException&)
            {
                try
                {
                    s->destroy();
                }
                catch (const Ice::LocalException&)
                {
                }
            }
            lock.lock();
        }
    }

    if (_traceLevels && _traceLevels->replica > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
        out << "replica '" << name << "' up";
    }

    try
    {
        _observers->replicaAdded(session->getInternalRegistry());
    }
    catch (const Ice::NoEndpointException&)
    {
        // Expected if the replica is being shutdown.
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
        // Expected if the replica is being shutdown.
    }
    catch (const Ice::LocalException& ex)
    {
        auto traceLevels = getTraceLevels();
        if (traceLevels)
        {
            Ice::Warning out(traceLevels->logger);
            out << "unexpected exception while publishing 'replicaAdded' update:\n" << ex;
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

    if (_traceLevels && _traceLevels->replica > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
        out << "replica '" << name << "' down";
    }

    if (!shutdown)
    {
        try
        {
            _observers->replicaRemoved(entry->getProxy());
        }
        catch (const Ice::NoEndpointException&)
        {
            // Expected if the replica is being shutdown.
        }
        catch (const Ice::ObjectAdapterDestroyedException&)
        {
            // Expected if the replica is being shutdown.
        }
        catch (const Ice::LocalException& ex)
        {
            auto traceLevels = getTraceLevels();
            if (traceLevels)
            {
                Ice::Warning out(traceLevels->logger);
                out << "unexpected exception while publishing 'replicaRemoved' update:\n" << ex;
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
    if (!entry)
    {
        throw RegistryNotExistException(name);
    }
    return entry;
}

void
ReplicaCache::subscribe(const ReplicaObserverPrx& observer)
{
    try
    {
        lock_guard lock(_mutex);
        InternalRegistryPrxSeq replicas;
        for (const auto& entry : _entries)
        {
            replicas.emplace_back(entry.second->getProxy());
        }

        IceStorm::QoS qos;
        qos["reliability"] = "ordered";
        auto publisher = _topic->subscribeAndGetPublisher(qos, observer->ice_twoway());
        if (!publisher)
        {
            ostringstream os;
            os << "topic: '" << _topic->ice_toString() << "' returned null publisher proxy";
            throw Ice::MarshalException{__FILE__, __LINE__, os.str()};
        }
        Ice::uncheckedCast<ReplicaObserverPrx>(*publisher)->replicaInit(replicas);
    }
    catch (const Ice::NoEndpointException&)
    {
        // The replica is being shutdown.
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
        // Expected if the replica is being shutdown.
    }
    catch (const Ice::LocalException& ex)
    {
        auto traceLevels = getTraceLevels();
        if (traceLevels)
        {
            Ice::Warning out(traceLevels->logger);
            out << "unexpected exception while subscribing observer from replica observer topic:\n" << ex;
        }
    }
}

void
ReplicaCache::unsubscribe(const ReplicaObserverPrx& observer)
{
    try
    {
        _topic->unsubscribe(observer);
    }
    catch (const Ice::ObjectAdapterDestroyedException&)
    {
        // The replica is being shutdown.
    }
    catch (const Ice::NoEndpointException&)
    {
        // The replica is being shutdown.
    }
    catch (const Ice::LocalException& ex)
    {
        auto traceLevels = getTraceLevels();
        if (traceLevels)
        {
            Ice::Warning out(traceLevels->logger);
            out << "unexpected exception while unsubscribing observer from replica observer topic:\n" << ex;
        }
    }
}

Ice::ObjectPrx
ReplicaCache::getEndpoints(const string& name, const optional<Ice::ObjectPrx>& proxy) const
{
    Ice::EndpointSeq endpoints;

    if (proxy)
    {
        Ice::EndpointSeq endpts = proxy->ice_getEndpoints();
        endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
    }

    lock_guard lock(_mutex);
    for (const auto& entry : _entries)
    {
        auto prx = entry.second->getSession()->getEndpoint(name);
        if (prx)
        {
            Ice::EndpointSeq endpts = prx->ice_getEndpoints();
            endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
        }
    }

    return Ice::ObjectPrx(_communicator, "dummy")->ice_endpoints(endpoints);
}

void
ReplicaCache::setInternalRegistry(InternalRegistryPrx proxy)
{
    // Setup this replica internal registry proxy.
    _self = std::optional<InternalRegistryPrx>(std::move(proxy));
}

InternalRegistryPrx
ReplicaCache::getInternalRegistry() const
{
    // This replica internal registry proxy.
    assert(_self);
    return *_self;
}

ReplicaEntry::ReplicaEntry(std::string name, const shared_ptr<ReplicaSessionI>& session)
    : _name(std::move(name)),
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

InternalRegistryPrx
ReplicaEntry::getProxy() const
{
    return _session->getInternalRegistry();
}

Ice::ObjectPrx
ReplicaEntry::getAdminProxy() const
{
    InternalRegistryPrx prx = getProxy();
    return prx->ice_identity(Ice::Identity{"RegistryAdmin-" + _name, prx->ice_getIdentity().category});
}
