// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/Session.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/RouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

class ClientLocator : public ServantLocator
{
public:

    ClientLocator(const SessionRouterIPtr& sessionRouter) :
	_sessionRouter(sessionRouter)
    {
    }
    
    virtual ObjectPtr
    locate(const Current& current, LocalObjectPtr&)
    {
	return _sessionRouter->getRouter(current.con)->getClientBlobject();
    }

    virtual void
    finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
    {
    }

    virtual void
    deactivate(const std::string&)
    {
    }

private:

    const SessionRouterIPtr _sessionRouter;
};

class ServerLocator : public ServantLocator
{
public:

    ServerLocator(const SessionRouterIPtr& sessionRouter) :
	_sessionRouter(sessionRouter)
    {
    }
    
    virtual ObjectPtr
    locate(const Current& current, LocalObjectPtr&)
    {
	return _sessionRouter->getRouter(current.id.category)->getServerBlobject();
    }

    virtual void
    finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
    {
    }

    virtual void
    deactivate(const std::string&)
    {
    }

private:

    const SessionRouterIPtr _sessionRouter;
};

Glacier2::SessionRouterI::SessionRouterI(const ObjectAdapterPtr& clientAdapter,
					 const ObjectAdapterPtr& serverAdapter,
					 const PermissionsVerifierPrx& verifier,
					 const SessionManagerPrx& sessionManager) :
    _properties(clientAdapter->getCommunicator()->getProperties()),
    _logger(clientAdapter->getCommunicator()->getLogger()),
    _traceLevel(_properties->getPropertyAsInt("Glacier2.Trace.Session")),
    _clientAdapter(clientAdapter),
    _serverAdapter(serverAdapter),
    _verifier(verifier),
    _sessionManager(sessionManager),
    _sessionTimeout(IceUtil::Time::seconds(_properties->getPropertyAsInt("Glacier2.SessionTimeout"))),
    _sessionThread(_sessionTimeout > IceUtil::Time() ? new SessionThread(this) : 0),
    _routersByConnectionHint(_routersByConnection.end()),
    _routersByCategoryHint(_routersByCategory.end()),
    _destroy(false)
{
    //
    // This session router is used directly as servant for the main
    // Glacier2 router Ice object.
    //
    const char* routerIdProperty = "Glacier2.RouterIdentity";
    Identity routerId = stringToIdentity(_properties->getPropertyWithDefault(routerIdProperty, "Glacier2/router"));
    _clientAdapter->add(this, routerId);

    //
    // All other calls on the client object adapter are dispatched to
    // a router servant based on connection information.
    //
    ServantLocatorPtr clientLocator = new ClientLocator(this);
    _clientAdapter->addServantLocator(clientLocator, "");

    //
    // If there is a server object adapter, all calls on this adapter
    // are dispatched to a router servant based on the category field
    // of the identity.
    //
    if(_serverAdapter)
    {
	ServantLocatorPtr serverLocator = new ServerLocator(this);
	_serverAdapter->addServantLocator(serverLocator, "");
    }

    if(_sessionThread)
    {
	_sessionThread->start();
    }
}

Glacier2::SessionRouterI::~SessionRouterI()
{
    assert(_destroy);
    assert(_routersByConnection.empty());
    assert(_routersByCategory.empty());
}

void
Glacier2::SessionRouterI::destroy()
{
    map<ConnectionPtr, RouterIPtr> routers;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	
	assert(!_destroy);
	
	_routersByConnection.swap(routers);
	_routersByConnectionHint = _routersByConnection.end();
	
	_routersByCategory.clear();
	_routersByCategoryHint = _routersByCategory.end();
	
	_destroy = true;
	notify();
    }

    //
    // We destroy the routers outside the thread synchronization, to
    // avoid deadlocks.
    //
    for_each(routers.begin(), routers.end(),
	     IceUtil::secondVoidMemFun<const ConnectionPtr, RouterI>(&RouterI::destroy));

    if(_sessionThread)
    {
	_sessionThread->getThreadControl().join();
    }
}

ObjectPrx
Glacier2::SessionRouterI::getClientProxy(const Current& current) const
{
    return getRouter(current.con)->getClientProxy(current); // Forward to the per-client router.
}

ObjectPrx
Glacier2::SessionRouterI::getServerProxy(const Current& current) const
{
    return getRouter(current.con)->getServerProxy(current); // Forward to the per-client router.
}

void
Glacier2::SessionRouterI::addProxy(const ObjectPrx& proxy, const Current& current)
{
    getRouter(current.con)->addProxy(proxy, current); // Forward to the per-client router.
}

SessionPrx
Glacier2::SessionRouterI::createSession(const std::string& userId, const std::string& password, const Current& current)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    if(_destroy)
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    //
    // Check the user-id and password.
    //
    string reason;
    if(!_verifier->checkPermissions(userId, password, reason))
    {
	PermissionDeniedException ex;
	ex.reason = reason;
	throw ex;
    }

    //
    // Check if a session already exists for the client.
    //
    map<ConnectionPtr, RouterIPtr>::iterator p;    

    if(_routersByConnectionHint != _routersByConnection.end() && _routersByConnectionHint->first == current.con)
    {
	p = _routersByConnectionHint;
    }
    else
    {
	p = _routersByConnection.find(current.con);
    }

    if(p != _routersByConnection.end())
    {
	SessionExistsException ex;
	ex.existingSession = p->second->getSession();
	throw ex;
    }

    //
    // If we have a session manager configured, we create a
    // client-visible session object.
    //
    SessionPrx session;
    if(_sessionManager)
    {
	session = _sessionManager->create(userId);
    }

    //
    // Add a new per-client router.
    //
    RouterIPtr router = new RouterI(_clientAdapter, _serverAdapter, current.con, userId, session);
    _routersByConnectionHint = _routersByConnection.insert(_routersByConnectionHint,
							   pair<const ConnectionPtr, RouterIPtr>(current.con, router));
    if(_serverAdapter)
    {
	string category = router->getServerProxy(current)->ice_getIdentity().category;
	assert(!category.empty());
	_routersByCategoryHint = _routersByCategory.insert(_routersByCategoryHint,
							   pair<string, RouterIPtr>(category, router));
    }
    
    if(_traceLevel >= 1)
    {
	Trace out(_logger, "Glacier2");
	out << "creating session\n";
	out << router->toString();
    }

    return session;
}

void
Glacier2::SessionRouterI::destroySession(const Current& current)
{
    RouterIPtr router;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	
	if(_destroy)
	{
	    throw ObjectNotExistException(__FILE__, __LINE__);
	}
	
	map<ConnectionPtr, RouterIPtr>::iterator p;    
	
	if(_routersByConnectionHint != _routersByConnection.end() && _routersByConnectionHint->first == current.con)
	{
	    p = _routersByConnectionHint;
	}
	else
	{
	    p = _routersByConnection.find(current.con);
	}
	
	if(p == _routersByConnection.end())
	{
	    SessionNotExistException ex;
	    throw ex;
	}
	
	router = p->second;

	_routersByConnection.erase(p++);
	_routersByConnectionHint = p;
	
	if(_serverAdapter)
	{
	    string category = router->getServerProxy(Current())->ice_getIdentity().category;
	    assert(!category.empty());
	    _routersByCategory.erase(category);
	    _routersByCategoryHint = _routersByCategory.end();
	}
	
	if(_traceLevel >= 1)
	{
	    Trace out(_logger, "Glacier2");
	    out << "destroying session\n";
	    out << router->toString();
	}
    }

    //
    // We destroy the router outside the thread synchronization, to
    // avoid deadlocks.
    //
    router->destroy();
}

RouterIPtr
Glacier2::SessionRouterI::getRouter(const ConnectionPtr& connection) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    if(_destroy)
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    map<ConnectionPtr, RouterIPtr>& routers = const_cast<map<ConnectionPtr, RouterIPtr>&>(_routersByConnection);

    if(_routersByConnectionHint != routers.end() && _routersByConnectionHint->first == connection)
    {
	return _routersByConnectionHint->second;
    }
    
    map<ConnectionPtr, RouterIPtr>::iterator p = routers.find(connection);

    if(p != routers.end())
    {
	_routersByConnectionHint = p;
	return p->second;
    }
    else
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }
}

RouterIPtr
Glacier2::SessionRouterI::getRouter(const string& category) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    if(_destroy)
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    map<string, RouterIPtr>& routers = const_cast<map<string, RouterIPtr>&>(_routersByCategory);

    if(_routersByCategoryHint != routers.end() && _routersByCategoryHint->first == category)
    {
	return _routersByCategoryHint->second;
    }
    
    map<string, RouterIPtr>::iterator p = routers.find(category);

    if(p != routers.end())
    {
	_routersByCategoryHint = p;
	return p->second;
    }
    else
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }
}

void
Glacier2::SessionRouterI::run()
{
    while(true)
    {
	vector<RouterIPtr> routers;

	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	    
	    if(_destroy)
	    {
		return;
	    }

	    assert(_sessionTimeout > IceUtil::Time());
	    timedWait(_sessionTimeout);

	    if(_destroy)
	    {
		return;
	    }

	    IceUtil::Time minTimestamp = IceUtil::Time::now() - _sessionTimeout;
	    
	    map<ConnectionPtr, RouterIPtr>::iterator p = _routersByConnection.begin();
	    
	    while(p != _routersByConnection.end())
	    {
		if(minTimestamp < p->second->getTimestamp())
		{
		    RouterIPtr router = p->second;
		    routers.push_back(router);

		    _routersByConnection.erase(p++);
		    _routersByConnectionHint = p;

		    if(_serverAdapter)
		    {
			string category = router->getServerProxy(Current())->ice_getIdentity().category;
			assert(!category.empty());
			_routersByCategory.erase(category);
			_routersByCategoryHint = _routersByCategory.end();
		    }

		    if(_traceLevel >= 1)
		    {
			Trace out(_logger, "Glacier2");
			out << "expiring session\n";
			out << router->toString();
		    }
		}
		else
		{
		    ++p;
		}
	    }
	}

	//
	// We destroy the expired routers outside the thread
	// synchronization, to avoid deadlocks.
	//
	for_each(routers.begin(), routers.end(), IceUtil::voidMemFun(&RouterI::destroy));
    }
}

Glacier2::SessionRouterI::SessionThread::SessionThread(const SessionRouterIPtr& sessionRouter) :
    _sessionRouter(sessionRouter)
{
}

void
Glacier2::SessionRouterI::SessionThread::run()
{
    _sessionRouter->run();
    _sessionRouter = 0; // Break cyclic dependencies.
}
