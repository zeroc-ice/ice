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
						       const vector<QueryPrx>& queryObjects) : 
    SessionKeepAliveThread<NodeSessionPrx>(registry),
    _node(node),
    _queryObjects(queryObjects)
{
    assert(registry && node && !_queryObjects.empty());
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
NodeSessionKeepAliveThread::createSession(InternalRegistryPrx& registry, IceUtil::Time& timeout)
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
	    for(vector<QueryPrx>::const_iterator p = _queryObjects.begin(); p != _queryObjects.end(); ++p)
	    {
		InternalRegistryPrx newRegistry;
		try
		{
		    Ice::ObjectPrx obj = (*p)->findObjectById(registry->ice_getIdentity());
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
    _node->addObserver(session, session->getObserver());
    return session;
}

void 
NodeSessionKeepAliveThread::destroySession(const NodeSessionPrx& session)
{
    _node->removeObserver(session);

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
	_node->removeObserver(session);
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

    //
    // Initialize the IceGrid::Query objects. The IceGrid::Query
    // interface is used to lookup the registry proxy in case it
    // becomes unavailable. Since replicas might not always have
    // an up to date registry proxy, we need to query all the
    // replicas.
    //
    Ice::EndpointSeq endpoints = communicator->getDefaultLocator()->ice_getEndpoints();
    id.name = "Query";
    QueryPrx query = QueryPrx::uncheckedCast(communicator->stringToProxy(communicator->identityToString(id)));
    for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
	Ice::EndpointSeq singleEndpoint;
	singleEndpoint.push_back(*p);
	_queryObjects.push_back(QueryPrx::uncheckedCast(query->ice_endpoints(singleEndpoint)));
    }

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
    _thread->tryCreateSession(false);
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
	thread->tryCreateSession();
    }
    else
    {
	replicaAdded(replica, true);
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

void
NodeSessionManager::replicaAdded(const InternalRegistryPrx& replica, bool waitTryCreateSession)
{
    Lock sync(*this);
    if(_destroyed)
    {
	return;
    }

    ++_serial;
    NodeSessionMap::const_iterator p = _sessions.find(replica->ice_getIdentity());
    NodeSessionKeepAliveThreadPtr thread;
    if(p != _sessions.end())
    {
	thread = p->second;
	thread->setRegistry(replica);
    }
    else
    {
	thread = new NodeSessionKeepAliveThread(replica, _node, _queryObjects);
	_sessions.insert(make_pair(replica->ice_getIdentity(), thread));
	thread->start();
    }
    thread->tryCreateSession(waitTryCreateSession);
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

	++_serial;
	NodeSessionMap::iterator p = _sessions.find(replica->ice_getIdentity());
	if(p != _sessions.end())
	{
	    thread = p->second;
	    _sessions.erase(p);
	}
    }
    if(thread)
    {
	_node->removeObserver(thread->getSession()); // Needs to be done here because we don't destroy the session.
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
	    thread = new NodeSessionKeepAliveThread(*p, _node, _queryObjects);
	    thread->start();
	    thread->tryCreateSession(false);
	}
	_sessions.insert(make_pair((*p)->ice_getIdentity(), thread));
    }

    NodeSessionMap::iterator q = sessions.begin();
    while(q != sessions.end())
    {
	if(q->second->getSession()) // Don't destroy sessions which are still alive!
	{
	    _sessions.insert(make_pair(q->first, q->second));
	    sessions.erase(q++);
	}
	else
	{
	    ++q;
	}
    }
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
		map<Ice::Identity, Ice::ObjectPrx> proxies;
		for(vector<QueryPrx>::const_iterator p = _queryObjects.begin(); p != _queryObjects.end(); ++p)
		{
		    try
		    {
			Ice::ObjectProxySeq prxs = (*p)->findAllObjectsByType(InternalRegistry::ice_staticId());
			for(Ice::ObjectProxySeq::const_iterator q = prxs.begin(); q != prxs.end(); ++q)
			{
			    //
			    // NOTE: We might override a good proxy
			    // here! We could improve this to make
			    // sure that we don't override the proxy
			    // for replica N if that proxy was
			    // obtained from replica N.
			    //
			    proxies[(*q)->ice_getIdentity()] = *q;
			}
		    }
		    catch(const Ice::LocalException&)
		    {
			// IGNORE
		    }
		}
		for(map<Ice::Identity, Ice::ObjectPrx>::const_iterator q = proxies.begin(); q != proxies.end(); ++q)
		{
		    replicas.push_back(InternalRegistryPrx::uncheckedCast(q->second));
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
	    _node->checkConsistency(session);
	}
    }
    catch(const Ice::LocalException&)
    {
	// IGNORE
    }
}

