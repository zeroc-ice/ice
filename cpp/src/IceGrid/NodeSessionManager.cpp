//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/NodeSessionManager.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/NodeI.h>

using namespace std;
using namespace IceGrid;

NodeSessionKeepAliveThread::NodeSessionKeepAliveThread(const shared_ptr<InternalRegistryPrx>& registry,
                                                       const shared_ptr<NodeI>& node,
                                                       NodeSessionManager& manager) :
    SessionKeepAliveThread<NodeSessionPrx>(registry, node->getTraceLevels()->logger), _node(node), _manager(manager)
{
    assert(registry && node);
    string name = registry->ice_getIdentity().name;
    const string prefix("InternalRegistry-");
    string::size_type pos = name.find(prefix);
    if(pos != string::npos)
    {
        name = name.substr(prefix.size());
    }
    const_cast<string&>(_replicaName) = name;
}

shared_ptr<NodeSessionPrx>
NodeSessionKeepAliveThread::createSession(shared_ptr<InternalRegistryPrx>& registry, chrono::seconds& timeout)
{
    shared_ptr<NodeSessionPrx> session;
    string exceptionMessage;
    auto traceLevels = _node->getTraceLevels();
    try
    {
        if(traceLevels && traceLevels->replica > 1)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
            out << "trying to establish session with replica `" << _replicaName << "'";
        }

        set<shared_ptr<InternalRegistryPrx>> used;
        if(!registry->ice_getEndpoints().empty())
        {
            try
            {
                session = createSessionImpl(registry, timeout);
            }
            catch(const Ice::LocalException& ex)
            {
                exceptionMessage = ex.what();
                used.insert(registry);
                registry = Ice::uncheckedCast<InternalRegistryPrx>(registry->ice_endpoints(Ice::EndpointSeq()));
            }
        }

        if(!session)
        {
            vector<future<shared_ptr<Ice::ObjectPrx>>> results;
            auto queryObjects = _manager.getQueryObjects();
            for(const auto& object : queryObjects)
            {
                results.push_back(object->findObjectByIdAsync(registry->ice_getIdentity()));
            }

            for(auto& result : results)
            {
                if(isDestroyed())
                {
                    break;
                }

                shared_ptr<InternalRegistryPrx> newRegistry;
                try
                {
                    auto obj = result.get();
                    newRegistry = Ice::uncheckedCast<InternalRegistryPrx>(obj);
                    if(newRegistry && used.find(newRegistry) == used.end())
                    {
                        session = createSessionImpl(newRegistry, timeout);
                        registry = newRegistry;
                        break;
                    }
                }
                catch(const Ice::LocalException& ex)
                {
                    exceptionMessage = ex.what();
                    if(newRegistry)
                    {
                        used.insert(newRegistry);
                    }
                }
            }
        }
    }
    catch(const NodeActiveException& ex)
    {
        if(traceLevels)
        {
            traceLevels->logger->error("a node with the same name is already active with the replica `" +
                                       _replicaName + "'");
        }
        exceptionMessage = ex.what();
    }
    catch(const PermissionDeniedException& ex)
    {
        if(traceLevels)
        {
            traceLevels->logger->error("connection to the registry `" + _replicaName + "' was denied:\n" + ex.reason);
        }
        exceptionMessage = ex.what();
    }
    catch(const Ice::Exception& ex)
    {
        exceptionMessage = ex.what();
    }

    if(session)
    {
        if(traceLevels && traceLevels->replica > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
            out << "established session with replica `" << _replicaName << "'";
        }
    }
    else
    {
        if(traceLevels && traceLevels->replica > 1)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
            out << "failed to establish session with replica `" << _replicaName << "':\n";
            if(!exceptionMessage.empty())
            {
                out << exceptionMessage;
            }
            else
            {
                out << "failed to get replica proxy";
            }
        }
    }

    return session;
}

shared_ptr<NodeSessionPrx>
NodeSessionKeepAliveThread::createSessionImpl(const shared_ptr<InternalRegistryPrx>& registry, chrono::seconds& timeout)
{
    shared_ptr<NodeSessionPrx> session;
    try
    {
        session = _node->registerWithRegistry(registry);
        auto t = session->getTimeout();
        if(t > 0)
        {
            // Timeout is used to configure the delay to wait between two keep alives
            // If we used t directly, a delayed keep alive could kill the session
            timeout = chrono::seconds(t / 2);
        }
        _node->addObserver(session, session->getObserver());
        return session;
    }
    catch(const Ice::LocalException&)
    {
        destroySession(session);
        throw;
    }
}

void
NodeSessionKeepAliveThread::destroySession(const shared_ptr<NodeSessionPrx>& session)
{
    _node->removeObserver(session);

    if(session)
    {
        try
        {
            session->destroy();

            if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 0)
            {
                Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
                out << "destroyed replica `" << _replicaName << "' session";
            }
        }
        catch(const Ice::LocalException& ex)
        {
            if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 1)
            {
                Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
                out << "couldn't destroy replica `" << _replicaName << "' session:\n" << ex;
            }
        }
    }
}

bool
NodeSessionKeepAliveThread::keepAlive(const shared_ptr<NodeSessionPrx>& session)
{
    if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 2)
    {
        Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
        out << "sending keep alive message to replica `" << _replicaName << "'";
    }

    try
    {
        session->keepAlive(_node->getPlatformInfo().getLoadInfo());
        return true;
    }
    catch(const Ice::LocalException& ex)
    {
        _node->removeObserver(session);
        if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 0)
        {
            Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
            out << "lost session with replica `" << _replicaName << "':\n" << ex;
        }
        return false;
    }
}

NodeSessionManager::NodeSessionManager(const shared_ptr<Ice::Communicator>& communicator, const string& instanceName) :
    SessionManager(communicator, instanceName),
    _destroyed(false),
    _activated(false)
{
}

void
NodeSessionManager::create(const shared_ptr<NodeI>& node)
{
    {
        lock_guard lock(_mutex);
        assert(!_node);
        const_cast<shared_ptr<NodeI>&>(_node) = node;
        _thread = make_shared<Thread>(*this);
    }

    //
    // Try to create the session. It's important that we wait for the
    // creation of the session as this will also try to create sessions
    // with replicas (see createdSession below) and this must be done
    // before the node is activated.
    //
    _thread->tryCreateSession();
    _thread->waitTryCreateSession(3s);
}

void
NodeSessionManager::create(const shared_ptr<InternalRegistryPrx>& replica)
{
    assert(_thread);
    shared_ptr<NodeSessionKeepAliveThread> thread;
    if(replica->ice_getIdentity() == _master->ice_getIdentity())
    {
        thread = _thread;
        thread->setRegistry(replica);
    }
    else
    {
        lock_guard lock(_mutex);
        thread = addReplicaSession(replica);
    }

    if(thread)
    {
        thread->tryCreateSession();
        thread->waitTryCreateSession();
    }
}

void
NodeSessionManager::activate()
{
    {
        lock_guard lock(_mutex);
        _activated = true;
    }

    //
    // Get the master session, if it's not created, try to create it
    // again and make sure that the servers are synchronized and the
    // replica observer is set on the session.
    //
    auto session = _thread->getSession();
    if(session)
    {
        try
        {
            session->setReplicaObserver(_node->getProxy());
            syncServers(session);
        }
        catch(const Ice::LocalException& ex)
        {
            Ice::Warning out(_node->getTraceLevels()->logger);
            out << "failed to set replica observer:\n" << ex;
        }
    }
}

bool
NodeSessionManager::isWaitingForCreate()
{
    return _thread->isWaitingForCreate();
}

bool
NodeSessionManager::waitForCreate()
{
    assert(_thread);
    return _thread->waitForCreate();
}

void
NodeSessionManager::terminate()
{
    assert(_thread);
    _thread->terminate();
    _thread->join();
}

void
NodeSessionManager::destroy()
{
    NodeSessionMap sessions;
    {
        lock_guard lock(_mutex);
        if(_destroyed)
        {
            return;
        }
        _destroyed = true;
        _sessions.swap(sessions);
        _condVar.notify_all();
    }

    if(_thread)
    {
        _thread->terminate();
    }

    for(const auto& session : sessions)
    {
        session.second->terminate();
    }

    if(_thread)
    {
        _thread->join();
    }

    for(const auto& session : sessions)
    {
        session.second->join();
    }

}

void
NodeSessionManager::replicaInit(const InternalRegistryPrxSeq& replicas)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        return;
    }

    //
    // Initialize the set of replicas known by the master.
    //
    _replicas.clear();
    for(const auto& replica : replicas)
    {
        _replicas.insert(replica->ice_getIdentity());
        addReplicaSession(replica)->tryCreateSession();
    }
}

void
NodeSessionManager::replicaAdded(const shared_ptr<InternalRegistryPrx>& replica)
{
    lock_guard lock(_mutex);
    if(_destroyed)
    {
        return;
    }
    _replicas.insert(replica->ice_getIdentity());
    addReplicaSession(replica)->tryCreateSession();
}

void
NodeSessionManager::replicaRemoved(const shared_ptr<InternalRegistryPrx>& replica)
{
    {
        lock_guard lock(_mutex);
        if(_destroyed)
        {
            return;
        }
        _replicas.erase(replica->ice_getIdentity());
    }

    //
    // We don't remove the session here. It will eventually be reaped
    // by reapReplicas() if the session is dead.
    //
}

shared_ptr<NodeSessionKeepAliveThread>
NodeSessionManager::addReplicaSession(const shared_ptr<InternalRegistryPrx>& replica)
{
    assert(!_destroyed);
    auto p = _sessions.find(replica->ice_getIdentity());
    shared_ptr<NodeSessionKeepAliveThread> thread;
    if(p != _sessions.end())
    {
        thread = p->second;
        thread->setRegistry(replica);
    }
    else
    {
        thread = make_shared<NodeSessionKeepAliveThread>(replica, _node, *this);
        _sessions.insert(make_pair(replica->ice_getIdentity(), thread));
    }
    return thread;
}

void
NodeSessionManager::reapReplicas()
{
    // NodeSessionKeepAliveThread's destructor will join its thread on destruction
    // Keep this vector do the destruction outside the lock
    vector<shared_ptr<NodeSessionKeepAliveThread>> reap;
    {
        lock_guard lock(_mutex);
        if(_destroyed)
        {
            return;
        }

        auto q = _sessions.begin();
        while(q != _sessions.end())
        {
            if(_replicas.find(q->first) == _replicas.end() && q->second->terminateIfDisconnected())
            {
                _node->removeObserver(q->second->getSession());
                reap.push_back(q->second);
                _sessions.erase(q++);
            }
            else
            {
                ++q;
            }
        }
    }

    for(const auto& r : reap)
    {
        r->join();
    }
}

void
NodeSessionManager::syncServers(const shared_ptr<NodeSessionPrx>& session)
{
    //
    // Ask the session to load the servers on the node. Once this is
    // done we check the consistency of the node to make sure old
    // servers are removed.
    //
    // NOTE: it's important for this to be done after trying to
    // register with the replicas. When the master loads the server
    // some server might get activated and it's better if at that time
    // the registry replicas (at least the ones which are up) have all
    // established their session with the node.
    //
    assert(session);
    _node->checkConsistency(session);
    session->loadServers();
}

void
NodeSessionManager::createdSession(const shared_ptr<NodeSessionPrx>& session)
{
    bool activated;
    {
        lock_guard lock(_mutex);
        activated = _activated;
    }

    //
    // Synchronize the servers if the session is active and if the
    // node adapter has been activated (otherwise, the servers will be
    // synced after the node adapter activation, see activate()).
    //
    // We also set the replica observer to receive notifications of
    // replica addition/removal.
    //
    if(session && activated)
    {
        try
        {
            session->setReplicaObserver(_node->getProxy());
            syncServers(session);
        }
        catch(const Ice::LocalException& ex)
        {
            Ice::Warning out(_node->getTraceLevels()->logger);
            out << "failed to set replica observer:\n" << ex;
        }
        return;
    }

    //
    // If there's no master session or if the node adapter isn't
    // activated yet, we retrieve a list of the replicas either from
    // the master or from the known replicas (the ones configured with
    // Ice.Default.Locator) and we try to establish connections to
    // each of the replicas.
    //

    InternalRegistryPrxSeq replicas;
    if(session)
    {
        assert(!activated); // The node adapter isn't activated yet so
                            // we're not subscribed yet to the replica
                            // observer topic.
        try
        {
            replicas = _thread->getRegistry()->getReplicas();
        }
        catch(const Ice::LocalException&)
        {
        }
    }
    else
    {

        vector<future<Ice::ObjectProxySeq>> results1;
        vector<future<Ice::ObjectProxySeq>> results2;
        auto queryObjects = findAllQueryObjects(false);

        //
        // Below we try to retrieve internal registry proxies either
        // directly by querying for the internal registry type or
        // indirectly by querying registry proxies.
        //
        // IceGrid registries <= 3.5.0 kept internal registry proxies
        // while earlier version now keep registry proxies. Registry
        // proxies have fixed endpoints (no random port) so they are
        // more reliable.
        //

        for(const auto& object : queryObjects)
        {
            results1.push_back(object->findAllObjectsByTypeAsync(InternalRegistry::ice_staticId()));
        }
        for(const auto& object : queryObjects)
        {
            results2.push_back(object->findAllObjectsByTypeAsync(Registry::ice_staticId()));
        }

        map<Ice::Identity, shared_ptr<Ice::ObjectPrx>> proxies;

        for(auto& result : results1)
        {
            if(isDestroyed())
            {
                return;
            }

            try
            {
                auto prxs = result.get();
                for(const auto& prx : prxs)
                {
                    proxies[prx->ice_getIdentity()] = prx;
                }
            }
            catch(const Ice::LocalException&)
            {
                // Ignore.
            }
        }
        for(auto& result : results2)
        {
            if(isDestroyed())
            {
                return;
            }

            try
            {
                auto prxs = result.get();
                for(auto prx : prxs)
                {
                    Ice::Identity id = prx->ice_getIdentity();
                    const string prefix("Registry-");
                    string::size_type pos = id.name.find(prefix);
                    if(pos == string::npos)
                    {
                        continue; // Ignore the master registry proxy.
                    }

                    id.name = "InternalRegistry-" + id.name.substr(prefix.size());
                    prx = prx->ice_identity(id)->ice_endpoints(Ice::EndpointSeq());

                    id.name = "Locator";
                    prx = prx->ice_locator(Ice::uncheckedCast<Ice::LocatorPrx>(prx->ice_identity(id)));

                    proxies[id] = move(prx);
                }
            }
            catch(const Ice::LocalException&)
            {
                // Ignore
            }
        }

        for(const auto& prx : proxies)
        {
            replicas.push_back(Ice::uncheckedCast<InternalRegistryPrx>(prx.second));
        }
    }

    vector<shared_ptr<NodeSessionKeepAliveThread>> sessions;
    {
        lock_guard lock(_mutex);
        if(_destroyed)
        {
            return;
        }

        //
        // If the node adapter was activated since we last check, we don't need
        // to initialize the replicas here, it will be done by replicaInit().
        //
        if(!session || !_activated)
        {
            _replicas.clear();
            for(const auto& replica : replicas)
            {
                if(replica->ice_getIdentity() != _master->ice_getIdentity())
                {
                    _replicas.insert(replica->ice_getIdentity());

                    if(_sessions.find(replica->ice_getIdentity()) == _sessions.end())
                    {
                        auto thread = addReplicaSession(replica);
                        thread->tryCreateSession();
                        sessions.push_back(thread);
                    }
                    else
                    {
                        addReplicaSession(replica); // Update the session
                    }
                }
            }
        }
    }

    //
    // Wait for the creation. It's important to wait to ensure that
    // the replica sessions are created before the node adapter is
    // activated.
    //
    auto before = chrono::system_clock::now();
    for(const auto& s : sessions)
    {
        if(isDestroyed())
        {
            return;
        }
        auto timeout = 5s - (chrono::system_clock::now() - before);
        if(timeout <= 0s)
        {
            break;
        }
        s->waitTryCreateSession(chrono::duration_cast<chrono::seconds>(timeout));
    }
}
