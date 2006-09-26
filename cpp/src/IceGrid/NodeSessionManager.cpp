// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/NodeSessionManager.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Query.h>

using namespace std;
using namespace IceGrid;

NodeSessionKeepAliveThread::NodeSessionKeepAliveThread(const InternalRegistryPrx& registry,
						       const NodeIPtr& node,
						       const IceGrid::QueryPrx& query) : 
    SessionKeepAliveThread<NodeSessionPrx>(registry),
    _node(node),
    _query(query)
{
    assert(registry && node && query);
    string name = registry->ice_getIdentity().name;
    const string prefix("InternalRegistry-");
    string::size_type pos = name.find(prefix);
    if(pos != string::npos)
    {
	name = name.substr(prefix.size());
    }
    const_cast<string&>(_name) = name;
}

NodeSessionPrx
NodeSessionKeepAliveThread::createSession(const InternalRegistryPrx& registry, IceUtil::Time& timeout)
{
    NodeSessionPrx session;
    auto_ptr<Ice::Exception> exception;
    TraceLevelsPtr traceLevels = _node->getTraceLevels();
    try
    {
	if(traceLevels && traceLevels->replica > 1)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
	    out << "trying to establish session with replica `" << _name << "'";
	}

	if(!registry->ice_getEndpoints().empty())
	{
	    try
	    {
		session = createSessionImpl(registry, timeout);
	    }
	    catch(const Ice::LocalException& ex)
	    {
		exception.reset(ex.ice_clone());
		setRegistry(InternalRegistryPrx::uncheckedCast(registry->ice_endpoints(Ice::EndpointSeq())));
	    }
	}

	if(!session)
	{
	    try
	    {
		Ice::ObjectPrx obj = _query->findObjectById(registry->ice_getIdentity());
		InternalRegistryPrx newRegistry = InternalRegistryPrx::uncheckedCast(obj);
		if(newRegistry && newRegistry != registry)
		{
		    session = createSessionImpl(newRegistry, timeout);
		    setRegistry(newRegistry);
		}
	    }
	    catch(const Ice::LocalException& ex)
	    {
		exception.reset(ex.ice_clone());
	    }
	}
    }
    catch(const NodeActiveException& ex)
    {
	if(traceLevels)
	{
	    traceLevels->logger->error("a node with the same name is already active with the replica `" + _name + "'");
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
	    out << "established session with replica `" << _name << "'";
	}	
    }
    else
    {
	if(traceLevels && traceLevels->replica > 1)
	{
	    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
	    out << "failed to establish session with replica `" << _name << "':\n";
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
    NodeSessionPrx session = _node->registerWithRegistry(registry);
    int t = session->getTimeout();
    if(t > 0)
    {
	timeout = IceUtil::Time::seconds(t / 2);
    }
    return session;
}

void 
NodeSessionKeepAliveThread::destroySession(const NodeSessionPrx& session)
{
    try
    {
	session->destroy();

	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 0)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "destroyed replica `" << _name << "' session";
	}
    }
    catch(const Ice::LocalException& ex)
    {
	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "couldn't destroy replica `" << _name << "' session:\n" << ex;
	}
    }
}

bool 
NodeSessionKeepAliveThread::keepAlive(const NodeSessionPrx& session)
{
    if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 2)
    {
	Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	out << "sending keep alive message to replica `" << _name << "'";
    }

    try
    {
	session->keepAlive(_node->getPlatformInfo().getLoadInfo());
	return true;
    }
    catch(const Ice::LocalException& ex)
    {
	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 0)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "lost session with replica `" << _name << "':\n" << ex;
	}
	return false;
    }
}

NodeSessionManager::NodeSessionManager() : 
    _serial(1), 
    _destroyed(false)
{
}

void
NodeSessionManager::create(const NodeIPtr& node)
{
    assert(!_node);

    const_cast<NodeIPtr&>(_node) = node;

    Ice::CommunicatorPtr communicator = _node->getCommunicator();
    assert(communicator->getDefaultLocator());
    Ice::Identity id = communicator->getDefaultLocator()->ice_getIdentity();

    id.name = "Query";
    _query = QueryPrx::uncheckedCast(communicator->stringToProxy(communicator->identityToString(id)));

    id.name = "InternalRegistry-Master";
    _master = InternalRegistryPrx::uncheckedCast(communicator->stringToProxy(communicator->identityToString(id)));

    _thread = new Thread(*this);
    _thread->start();
    
    //
    // We can't wait for the session to be created here as the node
    // adapter isn't activated yet and the registry would hang trying
    // to load the servers on the node (when createSession invokes
    // loadServers() on the session).
    //
    //_thread->tryCreateSession(_master);
}

void
NodeSessionManager::create(const InternalRegistryPrx& replica)
{
    assert(_thread);
    if(replica->ice_getIdentity() == _master->ice_getIdentity())
    {
	_thread->tryCreateSession(replica);
    }
    else
    {
	NodeSessionKeepAliveThreadPtr thread = replicaAdded(replica);
	if(thread)
	{
	    thread->tryCreateSession(replica);
	}
    }
}

void
NodeSessionManager::waitForCreate()
{
    assert(_thread);
    _thread->waitForCreate();
}

void
NodeSessionManager::destroy()
{
    NodeSessionMap sessions;
    {
	Lock sync(*this);
	_destroyed = true;
	_sessions.swap(sessions);
	notifyAll();
    }

    _thread->terminate();
    NodeSessionMap::const_iterator p;
    for(p = sessions.begin(); p != sessions.end(); ++p)
    {
	p->second->terminate();
    }

    _thread->getThreadControl().join();
    for(p = sessions.begin(); p != sessions.end(); ++p)
    {
	p->second->getThreadControl().join();
    }
}

NodeSessionKeepAliveThreadPtr
NodeSessionManager::replicaAdded(const InternalRegistryPrx& replica)
{
    Lock sync(*this);
    if(_destroyed)
    {
	return 0;
    }

    ++_serial;
    NodeSessionMap::const_iterator p = _sessions.find(replica->ice_getIdentity());
    if(p != _sessions.end())
    {
	return p->second;
    }

    NodeSessionKeepAliveThreadPtr thread = new NodeSessionKeepAliveThread(replica, _node, _query);
    _sessions.insert(make_pair(replica->ice_getIdentity(), thread));
    thread->start();
    return thread;
}

void
NodeSessionManager::replicaRemoved(const InternalRegistryPrx& replica)
{
    NodeSessionKeepAliveThreadPtr thread;
    {
	Lock sync(*this);
	if(_destroyed)
	{
	    return;
	}

	--_serial;
	NodeSessionMap::iterator p = _sessions.find(replica->ice_getIdentity());
	if(p != _sessions.end())
	{
	    thread = p->second;
	    _sessions.erase(p);
	}
    }
    if(thread)
    {
	thread->terminate(false); // Don't destroy the session, the replica is being shutdown!
	thread->getThreadControl().join();
    }
}

void
NodeSessionManager::syncReplicas(const InternalRegistryPrxSeq& replicas)
{ 
    NodeSessionMap sessions;
    _sessions.swap(sessions);

    NodeSessionKeepAliveThreadPtr thread;
    for(InternalRegistryPrxSeq::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
    {
	if((*p)->ice_getIdentity() == _master->ice_getIdentity())
	{
	    continue;
	}
	NodeSessionMap::const_iterator q = sessions.find((*p)->ice_getIdentity());
	if(q != sessions.end())
	{
	    thread = q->second;
	    sessions.erase((*p)->ice_getIdentity());
	}
	else
	{
	    thread = new NodeSessionKeepAliveThread(*p, _node, _query);
	    thread->start();
	    thread->tryCreateSession(*p);
	}
	_sessions.insert(make_pair((*p)->ice_getIdentity(), thread));
    }

    NodeSessionMap::const_iterator q;
    for(q = sessions.begin(); q != sessions.end(); ++q)
    {
	q->second->terminate();
    }
    for(q = sessions.begin(); q != sessions.end(); ++q)
    {
	q->second->getThreadControl().join();
    }
}

void
NodeSessionManager::createdSession(const NodeSessionPrx& session)
{
    //
    // Get the list of replicas (either with the master session or the
    // IceGrid::Query interface) and make sure we have sessions opened
    // to these replicas.
    //
    try
    {
	unsigned long serial = 0;
	InternalRegistryPrxSeq replicas;
	while(true)
	{
	    {
		Lock sync(*this);
		if(serial == _serial)
		{
		    NodeSessionManager& self = const_cast<NodeSessionManager&>(*this);
		    self._serial = 1;
		    self.syncReplicas(replicas);
		    break;
		}
		serial = _serial;
	    }
		    
	    if(session)
	    {
		replicas = _thread->getRegistry()->getReplicas();
	    }
	    else
	    {
		replicas.clear();
		Ice::ObjectProxySeq proxies = _query->findAllObjectsByType(InternalRegistry::ice_staticId());
		for(Ice::ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
		{
		    replicas.push_back(InternalRegistryPrx::uncheckedCast(*p));
		}
	    }
	}
    }
    catch(const Ice::LocalException&)
    {
	// IGNORE
    }

    //
    // Ask the master to load the servers on the node. Once this is
    // done we check the consistency of the node to make sure old
    // servers are removed.
    //
    // NOTE: it's important for this to be done after trying to
    // register with the replicas. When the master loads the server
    // some server might get activated and it's better if at that time
    // the registry replicas (at least the ones which are up) have all
    // established their session with the node.
    //
    try
    {
	if(session)
	{
	    session->loadServers();
	    _node->setObserver(session->getObserver());
	    _node->checkConsistency(session);
	}
	else
	{
	    _node->setObserver(0);
	}
    }
    catch(const Ice::LocalException&)
    {
	// IGNORE
    }
}

