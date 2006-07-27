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

NodeSessionKeepAliveThread::NodeSessionKeepAliveThread(const InternalRegistryPrx& registry, const NodeIPtr& node) : 
    SessionKeepAliveThread<NodeSessionPrx, InternalRegistryPrx>(registry),
    _node(node)
{
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
NodeSessionKeepAliveThread::createSession(const InternalRegistryPrx& registry, IceUtil::Time& timeout) const
{
    try
    {
	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "trying to establish session with replica `" << _name << "'";
	}

	NodeSessionPrx session = _node->registerWithRegistry(registry);

	int t = session->getTimeout();
	if(t > 0)
	{
	    timeout = IceUtil::Time::seconds(t);
	}
	
	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 0)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "established session with replica `" << _name << "'";
	}
	
	return session;
    }
    catch(const NodeActiveException&)
    {
	if(_node->getTraceLevels())
	{
	    _node->getTraceLevels()->logger->error("a node with the same name is already registered and active");
	}
	return 0;
    }
    catch(const Ice::LocalException& ex)
    {
	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "failed to establish session with replica `" << _name << "':\n" << ex;
	}
	return 0;
    }
}

void 
NodeSessionKeepAliveThread::destroySession(const NodeSessionPrx& session) const
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
NodeSessionKeepAliveThread::keepAlive(const NodeSessionPrx& session) const
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

    id.name = "InternalRegistry";
    _master = InternalRegistryPrx::uncheckedCast(communicator->stringToProxy(communicator->identityToString(id)));

    _thread = new Thread(*this, _master);
    _thread->start();
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

    InternalRegistryPrx registry = 
	InternalRegistryPrx::uncheckedCast(
	    replica->ice_getCommunicator()->stringToProxy(
		replica->ice_getCommunicator()->identityToString(replica->ice_getIdentity())));

    NodeSessionKeepAliveThreadPtr thread = new NodeSessionKeepAliveThread(registry, _node);
    thread->start();
    _sessions.insert(make_pair(replica->ice_getIdentity(), thread));
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
	thread->terminate();
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
	    thread = new NodeSessionKeepAliveThread(*p, _node);
	    thread->start();
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

NodeSessionPrx
NodeSessionManager::createSession(const InternalRegistryPrx& registry, IceUtil::Time& timeout) const
{
    //
    // Establish a session with the master IceGrid registry.
    //
    NodeSessionPrx session;
    try
    {
	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "trying to establish session with master replica";
	}

	session = _node->registerWithRegistry(registry);

	int t = session->getTimeout();
	if(t > 0)
	{
	    timeout = IceUtil::Time::seconds(t);
	}

	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 0)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "established session with master replica";
	}	
    }
    catch(const NodeActiveException&)
    {
	if(_node->getTraceLevels())
	{
	    _node->getTraceLevels()->logger->error("a node with the same name is already registered and active");
	}
    }
    catch(const Ice::LocalException& ex)
    {
	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "failed to establish session with master replica:\n" << ex;
	}
    }

    if(session)
    {
	_node->setObserver(session->getObserver());
	_node->checkConsistency(session);
    }
    else
    {
	_node->setObserver(0);
    }

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
		replicas = registry->getReplicas();
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

    return session;
}

bool
NodeSessionManager::keepAlive(const NodeSessionPrx& session) const
{
    if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 2)
    {
	Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	out << "sending keep alive message to master replica";
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
	    out << "lost session with master replica:\n" << ex;
	}
	_node->setObserver(0);
	return false;
    }
}

void
NodeSessionManager::destroySession(const NodeSessionPrx& session) const
{
    try
    {
	session->destroy();

	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 0)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "destroyed master replica session";
	}
    }
    catch(const Ice::LocalException& ex)
    {
	if(_node->getTraceLevels() && _node->getTraceLevels()->replica > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->replicaCat);
	    out << "couldn't destroy master replica session:\n" << ex;
	}
    }
}

