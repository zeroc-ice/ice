// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <IceGrid/ReplicaCache.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/Topics.h>

using namespace std;
using namespace IceGrid;

ReplicaCache::ReplicaCache(const Ice::CommunicatorPtr& communicator, const IceStorm::TopicManagerPrx& topicManager) :
    _communicator(communicator)
{
    IceStorm::TopicPrx t;
    try
    {
        t = topicManager->create("ReplicaObserverTopic");
    }
    catch(const IceStorm::TopicExists&)
    {
        t = topicManager->retrieve("ReplicaObserverTopic");
    }

    const_cast<IceStorm::TopicPrx&>(_topic) = IceStorm::TopicPrx::uncheckedCast(t->ice_endpoints(Ice::EndpointSeq()));
    const_cast<ReplicaObserverPrx&>(_observers) = ReplicaObserverPrx::uncheckedCast(_topic->getPublisher()->ice_endpoints(Ice::EndpointSeq()));
}

ReplicaEntryPtr
ReplicaCache::add(const string& name, const ReplicaSessionIPtr& session)
{
    Lock sync(*this);

    ReplicaEntryPtr entry;
    while((entry = getImpl(name)))
    {
        ReplicaSessionIPtr session = entry->getSession();
        if(session->isDestroyed())
        {
            wait(); // Wait for the session to be removed.
        }
        else
        {
            //
            // Check if the replica is still reachable, if not, we
            // destroy its session.
            //
            sync.release();
            try
            {
                session->getInternalRegistry()->ice_ping();
                throw ReplicaActiveException();
            }
            catch(const Ice::LocalException&)
            {
                try
                {
                    session->destroy(Ice::emptyCurrent);
                }
                catch(const Ice::LocalException&)
                {
                }
            }
            sync.acquire();
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
        TraceLevelsPtr traceLevels = getTraceLevels();
        if(traceLevels)
        {
            Ice::Warning out(traceLevels->logger);
            out << "unexpected exception while publishing `replicaAdded' update:\n" << ex;
        }
    }

    return addImpl(name, new ReplicaEntry(name, session));
}

ReplicaEntryPtr
ReplicaCache::remove(const string& name, bool shutdown)
{
    Lock sync(*this);

    ReplicaEntryPtr entry = getImpl(name);
    assert(entry);
    removeImpl(name);
    notifyAll();

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
            TraceLevelsPtr traceLevels = getTraceLevels();
            if(traceLevels)
            {
                Ice::Warning out(traceLevels->logger);
                out << "unexpected exception while publishing `replicaRemoved' update:\n" << ex;
            }
        }
    }

    return entry;
}

ReplicaEntryPtr
ReplicaCache::get(const string& name) const
{
    Lock sync(*this);
    ReplicaEntryPtr entry = getImpl(name);
    if(!entry)
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
        Lock sync(*this);
        InternalRegistryPrxSeq replicas;
        for(map<string, ReplicaEntryPtr>::const_iterator p = _entries.begin(); p != _entries.end(); ++p)
        {
            replicas.push_back(p->second->getProxy());
        }

        IceStorm::QoS qos;
        qos["reliability"] = "ordered";
        Ice::ObjectPrx publisher = _topic->subscribeAndGetPublisher(qos, observer->ice_twoway());
        ReplicaObserverPrx::uncheckedCast(publisher)->replicaInit(replicas);
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
        TraceLevelsPtr traceLevels = getTraceLevels();
        if(traceLevels)
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
        TraceLevelsPtr traceLevels = getTraceLevels();
        if(traceLevels)
        {
            Ice::Warning out(traceLevels->logger);
            out << "unexpected exception while unsubscribing observer from replica observer topic:\n" << ex;
        }
    }
}

Ice::ObjectPrx
ReplicaCache::getEndpoints(const string& name, const Ice::ObjectPrx& proxy) const
{
    Ice::EndpointSeq endpoints;

    if(proxy)
    {
        Ice::EndpointSeq endpts = proxy->ice_getEndpoints();
        endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
    }

    Lock sync(*this);
    for(map<string, ReplicaEntryPtr>::const_iterator p = _entries.begin(); p != _entries.end(); ++p)
    {
        Ice::ObjectPrx prx = p->second->getSession()->getEndpoint(name);
        if(prx)
        {
            Ice::EndpointSeq endpts = prx->ice_getEndpoints();
            endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
        }
    }

    return _communicator->stringToProxy("dummy")->ice_endpoints(endpoints);
}

void
ReplicaCache::setInternalRegistry(const InternalRegistryPrx& proxy)
{
    //
    // Setup this replica internal registry proxy.
    //
    _self = proxy;
}

InternalRegistryPrx
ReplicaCache::getInternalRegistry() const
{
    //
    // This replica internal registry proxy.
    //
    return _self;
}

ReplicaEntry::ReplicaEntry(const std::string& name, const ReplicaSessionIPtr& session) :
    _name(name),
    _session(session)
{
}

ReplicaEntry::~ReplicaEntry()
{
}

const ReplicaSessionIPtr&
ReplicaEntry::getSession() const
{
    return _session;
}

InternalReplicaInfoPtr
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
    Ice::ObjectPrx prx = getProxy();
    assert(prx);
    Ice::Identity adminId;
    adminId.name = "RegistryAdmin-" + _name;
    adminId.category = prx->ice_getIdentity().category;
    return prx->ice_identity(adminId);
}
