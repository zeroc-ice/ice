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
						       const NodeIPtr& node) : 
    _registry(InternalRegistryPrx::uncheckedCast(registry->ice_adapterId(""))),
    _node(node), 
    _timeout(IceUtil::Time::seconds(5)), 
    _shutdown(false)
{
}

void
NodeSessionKeepAliveThread::run()
{
    //
    // Keep alive the session.
    //
    NodeSessionPrx session;
    while(true)
    {
	keepAlive(session);

	{
	    Lock sync(*this);

	    session = _session;	    

	    if(!_shutdown)
	    {
		timedWait(_timeout);
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
	}
	catch(const Ice::LocalException& ex)
	{
	    //
	    // TODO: XXX: TRACE?
	    //
//	    ostringstream os;
//	    os << "couldn't contact the IceGrid registry to destroy the node session:\n" << ex;
//	    _node->getTraceLevels()->logger->warning(os.str());
	}
    }
}

void
NodeSessionKeepAliveThread::waitForCreate()
{
    Lock sync(*this);
    while(!_session)
    {
	wait();
    }
}

void
NodeSessionKeepAliveThread::terminate()
{
    Lock sync(*this);
    _shutdown = true;
    notifyAll();
}

void
NodeSessionKeepAliveThread::keepAlive(const NodeSessionPrx& session)
{
    if(session)
    {
	try
	{
	    session->keepAlive(_node->getPlatformInfo().getLoadInfo());
	    return; // We're done!
	}
	catch(const Ice::LocalException&)
	{
	}
    }

    try
    {
	NodeSessionPrx newSession = _node->registerWithRegistry(_registry);
	int timeout = newSession->getTimeout();
	{
	    Lock sync(*this);
	    if(timeout > 0)
	    {
		_timeout = IceUtil::Time::seconds(timeout);
	    }
	    _session = newSession;
	    notifyAll();
	}
    }
    catch(const NodeActiveException&)
    {
	_node->getTraceLevels()->logger->error("a node with the same name is already registered and active");
    }
    catch(const Ice::LocalException& ex)
    {
	//
	// TODO: FIX THIS SHOULD BE A TRACE
	//
	ostringstream os;
	os << "couldn't contact the IceGrid registry:\n" << ex;
	_node->getTraceLevels()->logger->warning(os.str());
    }
}

NodeSessionManager::NodeSessionManager()
{
}

void
NodeSessionManager::create(const NodeIPtr& node)
{
    assert(!_node);
    const_cast<NodeIPtr&>(_node) = node;

    Ice::CommunicatorPtr communicator = _node->getCommunicator();
    Ice::PropertiesPtr properties = communicator->getProperties();

    Ice::LocatorPrx locator = communicator->getDefaultLocator();
    assert(locator);
    string instanceName = locator->ice_getIdentity().category;

    QueryPrx query = QueryPrx::uncheckedCast(communicator->stringToProxy(instanceName + "/Query"));
    Ice::ObjectProxySeq proxies = query->findAllObjectsByType(InternalRegistry::ice_staticId());
    NodeSessionKeepAliveThreadPtr thread;

    Lock sync(*this);
    for(Ice::ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
	thread = new NodeSessionKeepAliveThread(InternalRegistryPrx::uncheckedCast(*p), _node);
	thread->start();
	_sessions.insert(make_pair((*p)->ice_getIdentity(), thread));
    }
}

void
NodeSessionManager::waitForCreate()
{
    Lock sync(*this);
    for(NodeSessionMap::const_iterator p = _sessions.begin(); p != _sessions.end(); ++p)
    {
	p->second->waitForCreate();
    }
}

void
NodeSessionManager::destroy()
{
    Lock sync(*this);
    for(NodeSessionMap::const_iterator p = _sessions.begin(); p != _sessions.end(); ++p)
    {
	p->second->terminate();
    }
    for(NodeSessionMap::const_iterator p = _sessions.begin(); p != _sessions.end(); ++p)
    {
	p->second->getThreadControl().join();
    }
}

void
NodeSessionManager::replicaAdded(const InternalRegistryPrx& replica)
{
    Lock sync(*this);
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

