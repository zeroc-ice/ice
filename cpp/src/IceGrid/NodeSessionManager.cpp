// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/NodeSessionManager.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/NodeI.h>

using namespace std;
using namespace IceGrid;

NodeSessionKeepAliveThread::NodeSessionKeepAliveThread(const InternalRegistryPrx& registry,
                                                       const NodeIPtr& node,
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

NodeSessionPrx
NodeSessionKeepAliveThread::createSession(InternalRegistryPrx& registry, IceUtil::Time& timeout)
{
    NodeSessionPrx session;
    IceInternal::UniquePtr<Ice::Exception> exception;
    TraceLevelsPtr traceLevels = _node->getTraceLevels();
    try
    {
        if(traceLevels && traceLevels->replica > 1)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
            out << "trying to establish session with replica `" << _replicaName << "'";
        }

        set<InternalRegistryPrx> used;
        if(!registry->ice_getEndpoints().empty())
        {
            try
            {
                session = createSessionImpl(registry, timeout);
            }
            catch(const Ice::LocalException& ex)
            {
                exception.reset(ex.ice_clone());
                used.insert(registry);
                registry = InternalRegistryPrx::uncheckedCast(registry->ice_endpoints(Ice::EndpointSeq()));
            }
        }

        if(!session)
        {
            vector<Ice::AsyncResultPtr> results;
            vector<QueryPrx> queryObjects = _manager.getQueryObjects();
            for(vector<QueryPrx>::const_iterator q = queryObjects.begin(); q != queryObjects.end(); ++q)
            {
                results.push_back((*q)->begin_findObjectById(registry->ice_getIdentity()));
            }

            for(vector<Ice::AsyncResultPtr>::const_iterator p = results.begin(); p != results.end(); ++p)
            {
                QueryPrx query = QueryPrx::uncheckedCast((*p)->getProxy());
                if(isDestroyed())
                {
                    break;
                }

                InternalRegistryPrx newRegistry;
                try
                {
                    Ice::ObjectPrx obj = query->end_findObjectById(*p);
                    newRegistry = InternalRegistryPrx::uncheckedCast(obj);
                    if(newRegistry && used.find(newRegistry) == used.end())
                    {
                        session = createSessionImpl(newRegistry, timeout);
                        registry = newRegistry;
                        break;
                    }
                }
                catch(const Ice::LocalException& ex)
                {
                    exception.reset(ex.ice_clone());
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
        exception.reset(ex.ice_clone());
    }
    catch(const PermissionDeniedException& ex)
    {
        if(traceLevels)
        {
            traceLevels->logger->error("connection to the registry `" + _replicaName + "' was denied:\n" + ex.reason);
        }
        exception.reset(ex.ice_clone());
    }
    catch(const Ice::Exception& ex)
    {
        exception.reset(ex.ice_clone());
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
            if(exception.get())
            {
                out << *exception.get();
            }
            else
            {
                out << "failed to get replica proxy";
            }
        }
    }

    return session;
}

NodeSessionPrx
NodeSessionKeepAliveThread::createSessionImpl(const InternalRegistryPrx& registry, IceUtil::Time& timeout)
{
    NodeSessionPrx session;
    try
    {
        session = _node->registerWithRegistry(registry);
        int t = session->getTimeout();
        if(t > 0)
        {
            timeout = IceUtil::Time::seconds(t / 2);
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
NodeSessionKeepAliveThread::destroySession(const NodeSessionPrx& session)
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
NodeSessionKeepAliveThread::keepAlive(const NodeSessionPrx& session)
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

NodeSessionManager::NodeSessionManager(const Ice::CommunicatorPtr& communicator, const string& instanceName) :
    SessionManager(communicator, instanceName),
    _destroyed(false),
    _activated(false)
{
}

void
NodeSessionManager::create(const NodeIPtr& node)
{
    {
        Lock sync(*this);
        assert(!_node);
        const_cast<NodeIPtr&>(_node) = node;
        _thread = new Thread(*this);
        _thread->start();
    }

    //
    // Try to create the session. It's important that we wait for the
    // creation of the session as this will also try to create sessions
    // with replicas (see createdSession below) and this must be done
    // before the node is activated.
    //
    _thread->tryCreateSession();
    _thread->waitTryCreateSession(IceUtil::Time::seconds(3));
}

void
NodeSessionManager::create(const InternalRegistryPrx& replica)
{
    assert(_thread);
    NodeSessionKeepAliveThreadPtr thread;
    if(replica->ice_getIdentity() == _master->ice_getIdentity())
    {
        thread = _thread;
        thread->setRegistry(replica);
    }
    else
    {
        Lock sync(*this);
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
        Lock sync(*this);
        _activated = true;
    }

    //
    // Get the master session, if it's not created, try to create it
    // again and make sure that the servers are synchronized and the
    // replica observer is set on the session.
    //
    NodeSessionPrx session = _thread->getSession();
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
}

void
NodeSessionManager::destroy()
{
    NodeSessionMap sessions;
    {
        Lock sync(*this);
        if(_destroyed)
        {
            return;
        }
        _destroyed = true;
        _sessions.swap(sessions);
        notifyAll();
    }

    if(_thread)
    {
        _thread->terminate();
    }

    for(NodeSessionMap::const_iterator p = sessions.begin(); p != sessions.end(); ++p)
    {
        p->second->terminate();
    }

    if(_thread)
    {
        _thread->getThreadControl().join();
    }
    for(NodeSessionMap::const_iterator p = sessions.begin(); p != sessions.end(); ++p)
    {
        p->second->getThreadControl().join();
    }
}

void
NodeSessionManager::replicaInit(const InternalRegistryPrxSeq& replicas)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return;
    }

    //
    // Initialize the set of replicas known by the master.
    //
    _replicas.clear();
    for(InternalRegistryPrxSeq::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
    {
        _replicas.insert((*p)->ice_getIdentity());
        addReplicaSession(*p)->tryCreateSession();
    }
}

void
NodeSessionManager::replicaAdded(const InternalRegistryPrx& replica)
{
    Lock sync(*this);
    if(_destroyed)
    {
        return;
    }
    _replicas.insert(replica->ice_getIdentity());
    addReplicaSession(replica)->tryCreateSession();
}

void
NodeSessionManager::replicaRemoved(const InternalRegistryPrx& replica)
{
    {
        Lock sync(*this);
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

NodeSessionKeepAliveThreadPtr
NodeSessionManager::addReplicaSession(const InternalRegistryPrx& replica)
{
    assert(!_destroyed);
    NodeSessionMap::const_iterator p = _sessions.find(replica->ice_getIdentity());
    NodeSessionKeepAliveThreadPtr thread;
    if(p != _sessions.end())
    {
        thread = p->second;
        thread->setRegistry(replica);
    }
    else
    {
        thread = new NodeSessionKeepAliveThread(replica, _node, *this);
        _sessions.insert(make_pair(replica->ice_getIdentity(), thread));
        thread->start();
    }
    return thread;
}

void
NodeSessionManager::reapReplicas()
{
    vector<NodeSessionKeepAliveThreadPtr> reap;
    {
        Lock sync(*this);
        if(_destroyed)
        {
            return;
        }

        NodeSessionMap::iterator q = _sessions.begin();
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

    for(vector<NodeSessionKeepAliveThreadPtr>::const_iterator p = reap.begin(); p != reap.end(); ++p)
    {
        (*p)->getThreadControl().join();
    }
}

void
NodeSessionManager::syncServers(const NodeSessionPrx& session)
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
NodeSessionManager::createdSession(const NodeSessionPrx& session)
{
    bool activated;
    {
        Lock sync(*this);
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
        vector<Ice::AsyncResultPtr> results1;
        vector<Ice::AsyncResultPtr> results2;
        vector<QueryPrx> queryObjects = findAllQueryObjects(false);

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

        for(vector<QueryPrx>::const_iterator q = queryObjects.begin(); q != queryObjects.end(); ++q)
        {
            results1.push_back((*q)->begin_findAllObjectsByType(InternalRegistry::ice_staticId()));
        }
        for(vector<QueryPrx>::const_iterator q = queryObjects.begin(); q != queryObjects.end(); ++q)
        {
            results2.push_back((*q)->begin_findAllObjectsByType(Registry::ice_staticId()));
        }

        map<Ice::Identity, Ice::ObjectPrx> proxies;
        for(vector<Ice::AsyncResultPtr>::const_iterator p = results1.begin(); p != results1.end(); ++p)
        {
            QueryPrx query = QueryPrx::uncheckedCast((*p)->getProxy());
            if(isDestroyed())
            {
                return;
            }

            try
            {
                Ice::ObjectProxySeq prxs = query->end_findAllObjectsByType(*p);
                for(Ice::ObjectProxySeq::const_iterator q = prxs.begin(); q != prxs.end(); ++q)
                {
                    proxies[(*q)->ice_getIdentity()] = *q;
                }
            }
            catch(const Ice::LocalException&)
            {
                // Ignore.
            }
        }
        for(vector<Ice::AsyncResultPtr>::const_iterator p = results2.begin(); p != results2.end(); ++p)
        {
            QueryPrx query = QueryPrx::uncheckedCast((*p)->getProxy());
            if(isDestroyed())
            {
                return;
            }

            try
            {
                Ice::ObjectProxySeq prxs = query->end_findAllObjectsByType(*p);
                for(Ice::ObjectProxySeq::const_iterator q = prxs.begin(); q != prxs.end(); ++q)
                {
                    Ice::Identity id = (*q)->ice_getIdentity();
                    const string prefix("Registry-");
                    string::size_type pos = id.name.find(prefix);
                    if(pos == string::npos)
                    {
                        continue; // Ignore the master registry proxy.
                    }
                    id.name = "InternalRegistry-" + id.name.substr(prefix.size());

                    Ice::ObjectPrx prx = (*q)->ice_identity(id)->ice_endpoints(Ice::EndpointSeq());
                    id.name = "Locator";
                    prx = prx->ice_locator(Ice::LocatorPrx::uncheckedCast((*q)->ice_identity(id)));
                    proxies[id] = prx;
                }
            }
            catch(const Ice::LocalException&)
            {
                // Ignore
            }
        }

        for(map<Ice::Identity, Ice::ObjectPrx>::const_iterator q = proxies.begin(); q != proxies.end(); ++q)
        {
            replicas.push_back(InternalRegistryPrx::uncheckedCast(q->second));
        }
    }

    vector<NodeSessionKeepAliveThreadPtr> sessions;
    {
        Lock sync(*this);
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
            for(InternalRegistryPrxSeq::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
            {
                if((*p)->ice_getIdentity() != _master->ice_getIdentity())
                {
                    _replicas.insert((*p)->ice_getIdentity());

                    if(_sessions.find((*p)->ice_getIdentity()) == _sessions.end())
                    {
                        NodeSessionKeepAliveThreadPtr thread = addReplicaSession(*p);
                        thread->tryCreateSession();
                        sessions.push_back(thread);
                    }
                    else
                    {
                        addReplicaSession(*p); // Update the session
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
    IceUtil::Time before = IceUtil::Time::now();
    for(vector<NodeSessionKeepAliveThreadPtr>::const_iterator p = sessions.begin(); p != sessions.end(); ++p)
    {
        if(isDestroyed())
        {
            return;
        }
        IceUtil::Time timeout = IceUtil::Time::seconds(5) - (IceUtil::Time::now() - before);
        if(timeout <= IceUtil::Time())
        {
            break;
        }
        (*p)->waitTryCreateSession(timeout);
    }
}
