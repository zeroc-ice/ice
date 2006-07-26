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
    _registry(registry),
    _node(node), 
    _shutdown(false)
{
    string name = _registry->ice_getIdentity().name;
    const string prefix("InternalRegistry-");
    string::size_type pos = name.find(prefix);
    if(pos != string::npos)
    {
	name = name.substr(prefix.size());
    }
    const_cast<string&>(_name) = name;
    const_cast<InternalRegistryPrx&>(_registry) = 
	InternalRegistryPrx::uncheckedCast(
	    _registry->ice_getCommunicator()->stringToProxy(
		_registry->ice_getCommunicator()->identityToString(_registry->ice_getIdentity())));
}

void
NodeSessionKeepAliveThread::run()
{
    //
    // Keep alive the session.
    //
    NodeSessionPrx session;
    IceUtil::Time timeout = IceUtil::Time::seconds(5); 
    TraceLevelsPtr traceLevels = _node->getTraceLevels();
    while(true)
    {
	//
	// Send a keep alive message to the session.
	//
	if(session)
	{
	    if(traceLevels && traceLevels->replica > 2)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		out << "sending keep alive message to replica `" << _name << "'";
	    }

	    try
	    {
		session->keepAlive(_node->getPlatformInfo().getLoadInfo());
	    }
	    catch(const Ice::LocalException& ex)
	    {
		if(traceLevels && traceLevels->replica > 0)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "lost session with replica `" << _name << "':\n" << ex;
		}
		session = 0;
	    }
	}

	//
	// If the session isn't established yet, try to create a new
	// session.
	//
	if(!session)
	{
	    try
	    {
		if(traceLevels && traceLevels->replica > 1)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "trying to establish session with replica `" << _name << "'";
		}

		session = _node->registerWithRegistry(_registry);
		{
		    Lock sync(*this);
		    _session = session;
		    notifyAll();
		}

		int t = session->getTimeout();
		if(t > 0)
		{
		    timeout = IceUtil::Time::seconds(t);
		}
	
		if(traceLevels && traceLevels->replica > 0)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "established session with replica `" << _name << "'";
		}
	    }
	    catch(const NodeActiveException&)
	    {
		if(traceLevels)
		{
		    traceLevels->logger->error("a node with the same name is already registered and active");
		}
	    }
	    catch(const Ice::LocalException& ex)
	    {
		if(traceLevels && traceLevels->replica > 1)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "failed to establish session with replica `" << _name << "':\n" << ex;
		}
	    }
	}

	//
	// Wait for the configured timeout duration.
	//
	{
	    Lock sync(*this);
	    if(!_shutdown)
	    {
		timedWait(timeout);
	    }
	    if(_shutdown)
	    {
		break;
	    }
	}
    }

    //
    // Destroy the session.
    //
    if(session)
    {
	try
	{
	    session->destroy();

	    if(traceLevels && traceLevels->replica > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		out << "destroyed replica `" << _name << "' session";
	    }
	}
	catch(const Ice::LocalException& ex)
	{
	    if(traceLevels && traceLevels->replica > 1)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		out << "couldn't destroy replica `" << _name << "' session:\n" << ex;
	    }
	}
    }
}

bool
NodeSessionKeepAliveThread::waitForCreate()
{
    Lock sync(*this);
    while(!_session && !_shutdown)
    {
	wait();
    }
    return !_shutdown;
}

void
NodeSessionKeepAliveThread::terminate()
{
    Lock sync(*this);
    _shutdown = true;
    notifyAll();
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

    _thread = new Thread(*this);
    _thread->start();
}

void
NodeSessionManager::run()
{
    NodeSessionPrx session;
    TraceLevelsPtr traceLevels = _node->getTraceLevels();
    IceUtil::Time timeout = IceUtil::Time::seconds(5); 
    while(true)
    {
	if(session)
	{
	    if(traceLevels && traceLevels->replica > 2)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		out << "sending keep alive message to master replica";
	    }

	    try
	    {
		session->keepAlive(_node->getPlatformInfo().getLoadInfo());
	    }
	    catch(const Ice::LocalException& ex)
	    {
		if(traceLevels && traceLevels->replica > 0)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "lost session with master replica:\n" << ex;
		}
		session = 0;
		_node->setObserver(0);
		{
		    Lock sync(*this);
		    _masterSession = session;
		    notifyAll();
		}
	    }
	}

	if(!session)
	{
	    //
	    // Establish a session with the master IceGrid registry.
	    //
	    try
	    {
		if(traceLevels && traceLevels->replica > 1)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "trying to establish session with master replica";
		}

		session = _node->registerWithRegistry(_master);

		_node->setObserver(session->getObserver());

		//
		// We only check consistency with the master registry.
		//
		_node->checkConsistency(session);
		
		{
		    Lock sync(*this);
		    _masterSession = session;
		    notifyAll();
		}

		int t = session->getTimeout();
		if(t > 0)
		{
		    timeout = IceUtil::Time::seconds(t);
		}

		if(traceLevels && traceLevels->replica > 0)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "established session with master replica";
		}
	    }
	    catch(const NodeActiveException&)
	    {
		if(traceLevels)
		{
		    traceLevels->logger->error("a node with the same name is already registered and active");
		}
	    }
	    catch(const Ice::LocalException& ex)
	    {
		if(traceLevels && traceLevels->replica > 1)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "failed to establish session with master replica:\n" << ex;
		}
	    }

	    //
	    // Get the list of replicas (either with the master
	    // session or the IceGrid::Query interface) and make sure
	    // we have sessions opened to these replicas.
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
			    _serial = 1;
			    syncReplicas(replicas);
			    break;
			}
			serial = _serial;
		    }
		    
		    if(session)
		    {
			replicas = session->getReplicas();
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
	}

	{
	    Lock sync(*this);	    
	    if(!_destroyed)
	    {
		timedWait(timeout);
	    }
	    if(_destroyed)
	    {
		break;
	    }
	}
    }
    
    //
    // Destroy the session.
    //
    if(session)
    {
	try
	{
	    session->destroy();

	    if(traceLevels && traceLevels->replica > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		out << "destroyed master replica session";
	    }
	}
	catch(const Ice::LocalException& ex)
	{
	    if(traceLevels && traceLevels->replica > 1)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		out << "couldn't destroy master replica session:\n" << ex;
	    }
	}
    }
}

void
NodeSessionManager::waitForCreate()
{
    //
    // Wait for the node to establish the session with the master or
    // at least one replica registry.
    //
    while(true)
    {
	NodeSessionKeepAliveThreadPtr thread;
	{
	    Lock sync(*this);
	    while(!_masterSession && _sessions.empty() && !_destroyed)
	    {
		wait();
	    }

	    if(_masterSession || _destroyed)
	    {
		return;
	    }
	    else
	    {
		thread = _sessions.begin()->second;
	    }
	}
	if(thread->waitForCreate())
	{
	    break;
	}
    }
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
NodeSessionManager::replicaAdded(const InternalRegistryPrx& replica)
{
    Lock sync(*this);
    if(_destroyed)
    {
	return;
    }

    ++_serial;
    if(_sessions.find(replica->ice_getIdentity()) != _sessions.end())
    {
	return;
    }

    NodeSessionKeepAliveThreadPtr thread = new NodeSessionKeepAliveThread(replica, _node);
    thread->start();
    _sessions.insert(make_pair(replica->ice_getIdentity(), thread));
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

