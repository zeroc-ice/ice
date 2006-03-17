// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/Session.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/RouterI.h>

#include <IceUtil/UUID.h>

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
	return _sessionRouter->getRouter(current.con, current.id)->getClientBlobject();
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
    _sessionTraceLevel(_properties->getPropertyAsInt("Glacier2.Trace.Session")),
    _rejectTraceLevel(_properties->getPropertyAsInt("Glacier2.Client.Trace.Reject")),
    _clientAdapter(clientAdapter),
    _serverAdapter(serverAdapter),
    _verifier(verifier),
    _sessionManager(sessionManager),
    _sessionCloseCountDefault(_properties->getPropertyAsIntWithDefault("Glacier2.SessionManager.CloseCount", 0)),
    _sessionCloseCount(0),
    _sessionTimeout(IceUtil::Time::seconds(_properties->getPropertyAsInt("Glacier2.SessionTimeout"))),
    _sessionThread(_sessionTimeout > IceUtil::Time() ? new SessionThread(this, _sessionTimeout) : 0),
    _routersByConnectionHint(_routersByConnection.end()),
    _routersByCategoryHint(_routersByCategory.end()),
    _destroy(false)
{
    //
    // This session router is used directly as servant for the main
    // Glacier2 router Ice object.
    //
    const char* routerIdProperty = "Glacier2.RouterIdentity";
    string routerId = _properties->getProperty(routerIdProperty);
    if(routerId.empty())
    {
        const char* instanceNameProperty = "Glacier2.InstanceName";
        routerId = _properties->getPropertyWithDefault(instanceNameProperty, "Glacier2") + "/router";
    }
    Identity id = stringToIdentity(routerId);
    _clientAdapter->add(this, id);

    //
    // If we have a session manager configured, and a the property to
    // close the connection set then we want to set a unique
    // connection on the session manager so that we don't close the
    // connections to the sessions all the time.
    //
    if(_sessionManager && _sessionCloseCountDefault > 0)
    {
	_sessionCloseCount = _sessionCloseCountDefault;
	_sessionManager = SessionManagerPrx::uncheckedCast(_sessionManager->ice_connectionId(IceUtil::generateUUID()));
    }

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
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    assert(_destroy);
    assert(_routersByConnection.empty());
    assert(_routersByCategory.empty());
    assert(_pending.empty());
    assert(!_sessionThread);
}

void
Glacier2::SessionRouterI::destroy()
{
    map<ConnectionPtr, RouterIPtr> routers;
    SessionThreadPtr sessionThread;

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	
	assert(!_destroy);
	_destroy = true;
	notify();
	
	_routersByConnection.swap(routers);
	_routersByConnectionHint = _routersByConnection.end();
	
	_routersByCategory.clear();
	_routersByCategoryHint = _routersByCategory.end();
	
	sessionThread = _sessionThread;
	_sessionThread = 0;
    }

    //
    // We destroy the routers outside the thread synchronization, to
    // avoid deadlocks.
    //
    for(map<ConnectionPtr, RouterIPtr>::iterator p = routers.begin(); p != routers.end(); ++p)
    {
	RouterIPtr router = p->second;
	
	try
	{
	    router->destroy();
	}
	catch(const Exception& ex)
	{
	    if(_sessionTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "exception while destroying session\n" << ex;
	    }
	}
    }

    if(sessionThread)
    {
	sessionThread->destroy();
	sessionThread->getThreadControl().join();
    }
}

ObjectPrx
Glacier2::SessionRouterI::getClientProxy(const Current& current) const
{
    return getRouter(current.con, current.id)->getClientProxy(current); // Forward to the per-client router.
}

ObjectPrx
Glacier2::SessionRouterI::getServerProxy(const Current& current) const
{
    return getRouter(current.con, current.id)->getServerProxy(current); // Forward to the per-client router.
}

void
Glacier2::SessionRouterI::addProxy(const ObjectPrx& proxy, const Current& current)
{
    getRouter(current.con, current.id)->addProxy(proxy, current); // Forward to the per-client router.
}

string
Glacier2::SessionRouterI::getCategoryForClient(const Ice::Current& current) const
{
    // Forward to the per-client router.
    return getRouter(current.con, current.id)->getServerProxy(current)->ice_getIdentity().category;
}

SessionPrx
Glacier2::SessionRouterI::createSession(const std::string& userId, const std::string& password, const Current& current)
{
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	
	if(_destroy)
	{
	    current.con->close(true);
	    throw ObjectNotExistException(__FILE__, __LINE__);
	}

	//
	// Check whether a session already exists for the connection.
	//
	{
	    map<ConnectionPtr, RouterIPtr>::iterator p;    
	    if(_routersByConnectionHint != _routersByConnection.end() &&
	       _routersByConnectionHint->first == current.con)
	    {
		p = _routersByConnectionHint;
	    }
	    else
	    {
		p = _routersByConnection.find(current.con);
	    }
	    
	    if(p != _routersByConnection.end())
	    {
		CannotCreateSessionException exc;
		exc.reason = "session exists";
		throw exc;
	    }
	}
	
	//
	// If some other thread is currently trying to create a
	// session, we wait until this thread is finished.
	//
	bool searchAgain = false;
	while(_pending.find(current.con) != _pending.end())
	{
	    wait();
	    
	    if(_destroy)
	    {
		current.con->close(true);
		throw ObjectNotExistException(__FILE__, __LINE__);
	    }

	    searchAgain = true;
	}

	//
	// Check for existing sessions again if we waited above, as
	// new sessions have been added in the meantime.
	//
	if(searchAgain)
	{
	    map<ConnectionPtr, RouterIPtr>::iterator p;    
	    if(_routersByConnectionHint != _routersByConnection.end() &&
	       _routersByConnectionHint->first == current.con)
	    {
		p = _routersByConnectionHint;
	    }
	    else
	    {
		p = _routersByConnection.find(current.con);
	    }
	    
	    if(p != _routersByConnection.end())
	    {
		CannotCreateSessionException exc;
		exc.reason = "session exists";
		throw exc;
	    }
	}

	//
	// No session exists yet, so we will try to create one. To
	// avoid that other threads try to create sessions for the
	// same connection, we add our endpoints to _pending.
	//
	_pending.insert(current.con);
    }

    try
    {
	//
	// Check the user-id and password.
	//
	string reason;
	bool ok;
	
	try
	{
	    ok = _verifier->checkPermissions(userId, password, reason, current.ctx);
	}
	catch(const Exception& ex)
	{
	    if(_sessionTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "exception while verifying password\n" << ex;
	    }
	    
	    PermissionDeniedException exc;
	    exc.reason = "internal server error";
	    throw exc;
	}
	
	if(!ok)
	{
	    PermissionDeniedException exc;
	    exc.reason = reason;
	    throw exc;
	}
    }
    catch(const Exception& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

	//
	// Signal other threads that we are done with trying to
	// establish a session for our connection;
	//
	_pending.erase(current.con);
	notify();

	ex.ice_throw();
    }


    SessionPrx session;
    RouterIPtr router;

    try
    {
        //
	// If we have a session manager configured, we create a
	// client-visible session object.
	//
	if(_sessionManager)
	{
	    session = _sessionManager->create(userId, current.ctx);
	}
    
	//
	// Add a new per-client router.
	//
	router = new RouterI(_clientAdapter, _serverAdapter, current.con, userId, session);
    }
    catch(const Exception& ex)
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

	//
	// Signal other threads that we are done with trying to
	// establish a session for our connection;
	//
	_pending.erase(current.con);
	notify();
	
	assert(!router);
	
	if(session)
	{
	    try
	    {
		session->destroy();
	    }
	    catch(const Exception&)
	    {
		// Ignore all exceptions here.
	    }
	}

	ex.ice_throw();
    }

    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

	//
	// Determine whether we should close the connection to the
	// session manager.
	//
	if(_sessionCloseCount > 0)
	{
	    assert(_sessionManager);
	    --_sessionCloseCount;
	    if(_sessionCloseCount == 0)
	    {
		_sessionCloseCount = _sessionCloseCountDefault;
		try
		{
		    Ice::ConnectionPtr connection = _sessionManager->ice_connection();
		    if(connection)
		    {
			connection->close(false);
		    }
		}
		catch(const Exception&)
		{
		    // Ignore all exceptions here.
		}
	    }
	}
	
	//
	// Signal other threads that we are done with trying to
	// establish a session for our connection;
	//
	_pending.erase(current.con);
	notify();
	
	if(_destroy)
	{
	    try
	    {
		router->destroy();
	    }
	    catch(const Exception&)
	    {
		// Ignore all exceptions here.
	    }
	    
	    current.con->close(true);
	    throw ObjectNotExistException(__FILE__, __LINE__);
	}
	
	_routersByConnectionHint = _routersByConnection.insert(
	    _routersByConnectionHint, pair<const ConnectionPtr, RouterIPtr>(current.con, router));
	
	if(_serverAdapter)
	{
	    string category = router->getServerProxy(current)->ice_getIdentity().category;
	    assert(!category.empty());
	    _routersByCategoryHint = _routersByCategory.insert(
		_routersByCategoryHint, pair<const string, RouterIPtr>(category, router));
	}
	
	if(_sessionTraceLevel >= 1)
	{
	    Trace out(_logger, "Glacier2");
	    out << "created session\n";
	    out << router->toString();
	}
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
	    current.con->close(true);
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
	    SessionNotExistException exc;
	    throw exc;
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
    }

    //
    // We destroy the router outside the thread synchronization, to
    // avoid deadlocks.
    //
    try
    {
	if(_sessionTraceLevel >= 1)
	{
	    Trace out(_logger, "Glacier2");
	    out << "destroying session\n";
	    out << router->toString();
	}

	router->destroy();
    }
    catch(const Exception& ex)
    {
	if(_sessionTraceLevel >= 1)
	{
	    Trace out(_logger, "Glacier2");
	    out << "exception while destroying session\n" << ex;
	}
    }
}

RouterIPtr
Glacier2::SessionRouterI::getRouter(const ConnectionPtr& connection, const Ice::Identity& id) const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    if(_destroy)
    {
	connection->close(true);
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
	if(_rejectTraceLevel >= 1)
	{
	    Trace out(_logger, "Glacier2");
	    out << "rejecting request. no session is associated with the connection.\n";
	    out << "identity: " << identityToString(id);
	}
	connection->close(true);
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
Glacier2::SessionRouterI::expireSessions()
{
    vector<RouterIPtr> routers;
    
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	
	if(_destroy)
	{
	    return;
	}
	
	assert(_sessionTimeout > IceUtil::Time());
	IceUtil::Time minTimestamp = IceUtil::Time::now() - _sessionTimeout;
	
	map<ConnectionPtr, RouterIPtr>::iterator p = _routersByConnection.begin();
	
	while(p != _routersByConnection.end())
	{
	    if(p->second->getTimestamp() < minTimestamp)
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
    for(vector<RouterIPtr>::iterator p = routers.begin(); p != routers.end(); ++p)
    {
	RouterIPtr router = *p;
	
	try
	{
	    if(_sessionTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "expiring session\n";
		out << router->toString();
	    }
	    
	    router->destroy();
	}
	catch(const Exception& ex)
	{
	    if(_sessionTraceLevel >= 1)
	    {
		Trace out(_logger, "Glacier2");
		out << "exception while expiring session\n" << ex;
	    }
	}
    }
}

Glacier2::SessionRouterI::SessionThread::SessionThread(const SessionRouterIPtr& sessionRouter,
						       const IceUtil::Time& sessionTimeout) :
    _sessionRouter(sessionRouter),
    _sessionTimeout(sessionTimeout)
{
}

Glacier2::SessionRouterI::SessionThread::~SessionThread()
{
    assert(!_sessionRouter);
}

void
Glacier2::SessionRouterI::SessionThread::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    _sessionRouter = 0;
    notify();
}

void
Glacier2::SessionRouterI::SessionThread::run()
{
    while(true)
    {
	SessionRouterIPtr sessionRouter;

	{
	    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

	    if(!_sessionRouter)
	    {
		return;
	    }
	    
	    assert(_sessionTimeout > IceUtil::Time());
	    timedWait(_sessionTimeout / 4);

	    if(!_sessionRouter)
	    {
		return;
	    }

	    sessionRouter = _sessionRouter;
	}

	sessionRouter->expireSessions();
    }
}
