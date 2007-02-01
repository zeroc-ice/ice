// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/Session.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/FilterManager.h>
#include <Glacier2/RouterI.h>

#include <IceUtil/UUID.h>

#include <IceSSL/Plugin.h>
#include <Ice/Network.h>

using namespace std;
using namespace Ice;

namespace Glacier2
{

class SessionControlI : public SessionControl
{
public:

    SessionControlI(const SessionRouterIPtr& sessionRouter, const ConnectionPtr& connection,
                    const FilterManagerPtr& filterManager, IceUtil::Time timeout) :
        _sessionRouter(sessionRouter),
        _connection(connection),
        _filters(filterManager),
        _timeout(static_cast<int>(timeout.toSeconds()))
    {
    }

    virtual StringSetPrx
    categories(const Current&)
    {
        return _filters->categoriesPrx();
    }

    virtual StringSetPrx
    adapterIds(const Current&)
    {
        return _filters->adapterIdsPrx();
    }

    virtual IdentitySetPrx
    identities(const Current&)
    {
        return _filters->identitiesPrx(); 
    }

    virtual int
    getSessionTimeout(const Current&)
    {
        return _timeout;
    }
    
    virtual void
    destroy(const Current&)
    {
        _sessionRouter->destroySession(_connection);
        _filters->destroy();
    }

private:

    const SessionRouterIPtr _sessionRouter;
    const ConnectionPtr _connection;
    const FilterManagerPtr _filters;
    const int _timeout;
};

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

class UserPasswordAuthorizer : public Authorizer
{
public:

    UserPasswordAuthorizer(const PermissionsVerifierPrx& verifier, const string& user, const string& password) :
        _verifier(verifier), _user(user), _password(password)
    {
    }

    virtual bool
    authorize(string& reason, const Ice::Context& ctx)
    {
        if(_verifier)
        {
            return _verifier->checkPermissions(_user, _password, reason, ctx);
        }
        reason = "No PermissionsVerifier is available";
        return false;
    }

private:

    const PermissionsVerifierPrx _verifier;
    const string _user;
    const string _password;
};

class SSLPasswordAuthorizer : public Authorizer
{
public:

    SSLPasswordAuthorizer(const SSLPermissionsVerifierPrx& verifier, const SSLInfo& info) :
        _verifier(verifier), _info(info)
    {
    }

    virtual bool
    authorize(string& reason, const Ice::Context& ctx)
    {
        if(_verifier)
        {
            return _verifier->authorize(_info, reason, ctx);
        }

        reason = "No SSLPermissionsVerifier is available";
        return false;
    }

private:

    const SSLPermissionsVerifierPrx _verifier;
    const SSLInfo _info;
};

class UserSessionFactory : public SessionFactory
{
public:

    UserSessionFactory(const SessionManagerPrx& manager, const string& user) :
        _manager(manager), _user(user)
    {
    }

    virtual SessionPrx
    create(const SessionControlPrx& control, const Ice::Context& ctx)
    {
        return _manager->create(_user, control, ctx);
    }

private:

    const SessionManagerPrx _manager;
    const string _user;
};

class SSLSessionFactory : public SessionFactory
{
public:

    SSLSessionFactory(const SSLSessionManagerPrx& manager, const SSLInfo& info) :
        _manager(manager), _info(info)
    {
    }

    virtual SessionPrx
    create(const SessionControlPrx& control, const Ice::Context& ctx)
    {
        return _manager->create(_info, control, ctx);
    }

private:

    const SSLSessionManagerPrx _manager;
    const SSLInfo _info;
};

}

using namespace Glacier2;

Glacier2::SessionRouterI::SessionRouterI(const ObjectAdapterPtr& clientAdapter,
                                         const ObjectAdapterPtr& serverAdapter,
                                         const PermissionsVerifierPrx& verifier,
                                         const SessionManagerPrx& sessionManager,
                                         const SSLPermissionsVerifierPrx& sslVerifier,
                                         const SSLSessionManagerPrx& sslSessionManager) :
    _properties(clientAdapter->getCommunicator()->getProperties()),
    _logger(clientAdapter->getCommunicator()->getLogger()),
    _sessionTraceLevel(_properties->getPropertyAsInt("Glacier2.Trace.Session")),
    _rejectTraceLevel(_properties->getPropertyAsInt("Glacier2.Client.Trace.Reject")),
    _clientAdapter(clientAdapter),
    _serverAdapter(serverAdapter),
    _verifier(verifier),
    _sessionManager(sessionManager),
    _sslVerifier(sslVerifier),
    _sslSessionManager(sslSessionManager),
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
    Identity routerId;
    routerId.category = _properties->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2");
    routerId.name = "router";
    _clientAdapter->add(this, routerId);

    //
    // All other calls on the client object adapter are dispatched to
    // a router servant based on connection information.
    //
    _clientAdapter->addServantLocator(new ClientLocator(this), "");

    //
    // If there is a server object adapter, all calls on this adapter
    // are dispatched to a router servant based on the category field
    // of the identity.
    //
    if(_serverAdapter)
    {
        _serverAdapter->addServantLocator(new ServerLocator(this), "");
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
    ObjectProxySeq seq;
    seq.push_back(proxy);
    addProxies(seq, current);
}

ObjectProxySeq
Glacier2::SessionRouterI::addProxies(const ObjectProxySeq& proxies, const Current& current)
{
    //
    // Forward to the per-client router.
    //
    return getRouter(current.con, current.id)->getClientBlobject()->add(proxies, current); 
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
    SessionFactoryPtr factory;
    if(_sessionManager)
    {
        factory = new UserSessionFactory(_sessionManager, userId);
    }
    return createSessionInternal(userId, true, new UserPasswordAuthorizer(_verifier, userId, password), factory,
                                 Ice::Context(), current);
}

SessionPrx
Glacier2::SessionRouterI::createSessionFromSecureConnection(const Current& current)
{
    if(!_sslVerifier)
    {
        PermissionDeniedException exc;
        exc.reason = "no configured ssl permissions verifier";
        throw exc;
    }

    string userDN;
    SSLInfo sslinfo;
    Ice::Context sslCtx;

    //
    // Populate the SSL context information.
    //
    try
    {
        IceSSL::ConnectionInfo info = IceSSL::getConnectionInfo(current.con);
        sslinfo.remotePort = ntohs(info.remoteAddr.sin_port);
        sslinfo.remoteHost = IceInternal::inetAddrToString(info.remoteAddr.sin_addr);
        sslinfo.localPort = ntohs(info.localAddr.sin_port);
        sslinfo.localHost = IceInternal::inetAddrToString(info.localAddr.sin_addr);

        sslinfo.cipher = info.cipher;

        if(info.certs.size() > 0)
        {
            sslinfo.certs.resize(info.certs.size());
            for(unsigned int i = 0; i < info.certs.size(); ++i)
            {
                sslinfo.certs[i] = info.certs[i]->encode();
            }
            userDN = info.certs[0]->getSubjectDN();
        }

        if(_properties->getPropertyAsInt("Glacier2.AddSSLContext") > 0)
        {
            sslCtx["SSL.Active"] = "1";
            sslCtx["SSL.Cipher"] = sslinfo.cipher;
            ostringstream os;
            os << sslinfo.remotePort;
            sslCtx["SSL.Remote.Port"] = os.str();
            sslCtx["SSL.Remote.Host"] = sslinfo.remoteHost;
            os.str("");
            os << ntohs(info.localAddr.sin_port);
            sslCtx["SSL.Local.Port"] = os.str();
            sslCtx["SSL.Local.Host"] = sslinfo.localHost;
            if(info.certs.size() > 0)
            {
                sslCtx["SSL.PeerCert"] = info.certs[0]->encode();
            }
        }
    }
    catch(const IceSSL::ConnectionInvalidException&)
    {
        PermissionDeniedException exc;
        exc.reason = "not ssl connection";
        throw exc;
    }
    catch(const IceSSL::CertificateEncodingException&)
    {
        PermissionDeniedException exc;
        exc.reason = "certificate encoding exception";
        throw exc;
    }

    SessionFactoryPtr factory;
    if(_sslSessionManager)
    {
        factory = new SSLSessionFactory(_sslSessionManager, sslinfo);
    }
    return createSessionInternal(userDN, false, new SSLPasswordAuthorizer(_sslVerifier, sslinfo), factory, sslCtx, 
                                 current);
}

void
Glacier2::SessionRouterI::destroySession(const Current& current)
{
    destroySession(current.con);
}

void
Glacier2::SessionRouterI::destroySession(const ConnectionPtr& connection)
{
    RouterIPtr router;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
        
        if(_destroy)
        {
            connection->close(true);
            throw ObjectNotExistException(__FILE__, __LINE__);
        }
        
        map<ConnectionPtr, RouterIPtr>::iterator p;    
        
        if(_routersByConnectionHint != _routersByConnection.end() && _routersByConnectionHint->first == connection)
        {
            p = _routersByConnectionHint;
        }
        else
        {
            p = _routersByConnection.find(connection);
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

Ice::Long
Glacier2::SessionRouterI::getSessionTimeout(const Ice::Current&) const
{
    return _sessionTimeout.toSeconds();
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
            out << "identity: " << _clientAdapter->getCommunicator()->identityToString(id);
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

SessionPrx
Glacier2::SessionRouterI::createSessionInternal(const string& userId, bool allowAddUserMode,
                                                const AuthorizerPtr& authorizer, const SessionFactoryPtr& factory,
                                                const Ice::Context& sslContext, const Current& current)
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
        // Authorize.
        //
        string reason;
        bool ok;

        try
        {
            ok = authorizer->authorize(reason, current.ctx);
        }
        catch(const Exception& ex)
        {
            if(_sessionTraceLevel >= 1)
            {
                Trace out(_logger, "Glacier2");
                out << "exception while verifying password:\n" << ex;
            }
            
            PermissionDeniedException exc;
            exc.reason = "internal server error";
            throw exc;
        }
        
        if(!ok)
        {
            PermissionDeniedException exc;
            if(reason.empty())
            {
                exc.reason = "permission denied";
            }
            else
            {
                exc.reason = reason;
            }
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
    Identity controlId;
    RouterIPtr router;

    try
    {
        //
        // The client blobject requires direct access to the full filter
        // servant, but proliferating the implementation of the servant
        // throughout the router code is undesirable. To avoid lots of
        // physical interdependencies, we create the filters and
        // clientblobject together and pass the clientblobject to the
        // router. We create the clientblobject here since it is
        // responsible for creating the filters and we want them to be
        // accessible during session creation.
        //
        FilterManagerPtr filterManager = FilterManager::create(_clientAdapter->getCommunicator(), _serverAdapter, 
                                                               userId, allowAddUserMode);

        //
        // If we have a session manager configured, we create a
        // client-visible session object.
        //
        if(factory)
        {
            SessionControlPrx control;
            if(_serverAdapter)
            {
                control = SessionControlPrx::uncheckedCast(
                    _serverAdapter->addWithUUID(
                        new SessionControlI(this, current.con, filterManager, _sessionTimeout)));
                controlId = control->ice_getIdentity();
            }
            session = factory->create(control, current.ctx);
        }
    
        //
        // Add a new per-client router.
        //
        router = new RouterI(_clientAdapter, _serverAdapter, current.con, userId, 
                             session, controlId, filterManager, sslContext);
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
            if(_serverAdapter)
            {
                try
                {
                    _serverAdapter->remove(controlId);
                }
                catch(const Exception&)
                {
                    // Ignore all exceptions here.
                }
            }
            try
            {
                session->destroy();
            }
            catch(const Exception&)
            {
                // Ignore all exceptions here.
            }
        }
        
        try
        {
            ex.ice_throw();
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
            throw;
        }
        catch(const Exception&)
        {
            if(_sessionTraceLevel >= 1)
            {
                Trace out(_logger, "Glacier2");
                out << "exception while creating session with session manager:\n" << ex;
            }
            
            CannotCreateSessionException exc;
            exc.reason = "internal server error";
            throw exc;
        }
    }

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

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
            pair<map<string, RouterIPtr>::iterator, bool> rc = 
                _routersByCategory.insert(pair<const string, RouterIPtr>(category, router));
            assert(rc.second);
            _routersByCategoryHint = rc.first;
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
