
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/SessionRouterI.h>
#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/FilterManager.h>
#include <Glacier2/RouterI.h>

#include <Ice/UUID.h>

#include <IceSSL/IceSSL.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace
{

shared_ptr<IPConnectionInfo>
getIPConnectionInfo(const shared_ptr<ConnectionInfo>& info)
{
    for(shared_ptr<ConnectionInfo> p = info; p; p = p->underlying)
    {
        auto ipInfo = dynamic_pointer_cast<IPConnectionInfo>(p);
        if(ipInfo)
        {
            return ipInfo;
        }
    }

    return nullptr;
}

}

namespace Glacier2
{

class SessionControlI final : public SessionControl
{
public:

    SessionControlI(shared_ptr<SessionRouterI> sessionRouter, shared_ptr<Connection> connection,
                    shared_ptr<FilterManager> filterManager) :
        _sessionRouter(move(sessionRouter)),
        _connection(move(connection)),
        _filters(move(filterManager))
    {
    }

    shared_ptr<StringSetPrx>
    categories(const Current&) override
    {
        return _filters->categoriesPrx();
    }

    shared_ptr<StringSetPrx>
    adapterIds(const Current&) override
    {
        return _filters->adapterIdsPrx();
    }

    shared_ptr<IdentitySetPrx>
    identities(const Current&) override
    {
        return _filters->identitiesPrx();
    }

    int
    getSessionTimeout(const Current& current) override
    {
        return static_cast<int>(_sessionRouter->getSessionTimeout(current));
    }

    void
    destroy(const Current&) override
    {
        _sessionRouter->destroySession(_connection);
        _filters->destroy();
    }

private:

    const shared_ptr<SessionRouterI> _sessionRouter;
    const shared_ptr<Connection> _connection;
    const shared_ptr<FilterManager> _filters;
};

class UserPasswordCreateSession final : public CreateSession
{
public:

    UserPasswordCreateSession(function<void(const shared_ptr<SessionPrx>&)> response,
                              function<void(exception_ptr)> exception,
                              const string& user, const string& password,
                              const Ice::Current& current, const shared_ptr<SessionRouterI>& sessionRouter) :
        CreateSession(sessionRouter, user, current),
        _response(move(response)),
        _exception(move(exception)),
        _password(password)
    {
    }

    void
    checkPermissionsResponse(bool ok, const string& reason)
    {
        if(ok)
        {
            authorized(_sessionRouter->_sessionManager != nullptr);
        }
        else
        {
            exception(make_exception_ptr(PermissionDeniedException(reason.empty() ? string("permission denied") : reason)));
        }
    }

    void
    checkPermissionsException(exception_ptr ex)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch(const PermissionDeniedException&)
        {
            exception(ex);
        }
        catch(...)
        {
            unexpectedAuthorizeException(ex);
        }
    }

    void
    authorize() override
    {
        assert(_sessionRouter->_verifier);

        auto ctx = _current.ctx;
        ctx.insert(_context.begin(), _context.end());
        auto self = static_pointer_cast<UserPasswordCreateSession>(shared_from_this());
        _sessionRouter->_verifier->checkPermissionsAsync(_user, _password,
                                                         [self](bool ok, const string& reason)
                                                         {
                                                             self->checkPermissionsResponse(ok, reason);
                                                         },
                                                         [self](exception_ptr e)
                                                         {
                                                             self->checkPermissionsException(e);
                                                         },
                                                         nullptr,
                                                         ctx);
    }

    shared_ptr<FilterManager>
    createFilterManager() override
    {
        return FilterManager::create(_instance, _user, true);
    }

    void
    createSession() override
    {
        auto ctx = _current.ctx;
        ctx.insert(_context.begin(), _context.end());
        auto self = shared_from_this();
        _sessionRouter->_sessionManager->createAsync(_user, _control,
                                                     [self](shared_ptr<SessionPrx> response)
                                                     {
                                                         self->sessionCreated(move(response));
                                                     },
                                                     [self](exception_ptr e)
                                                     {
                                                         self->createException(e);
                                                     },
                                                     nullptr,
                                                     ctx);

    }

    void
    finished(const shared_ptr<SessionPrx>& session) override
    {
        _response(session);
    }

    void
    finished(exception_ptr ex) override
    {
        _exception(ex);
    }

private:

    const function<void(shared_ptr<SessionPrx>)> _response;
    const function<void(exception_ptr)> _exception;
    const string _password;
};

class SSLCreateSession final : public CreateSession
{
public:

    SSLCreateSession(function<void(const shared_ptr<SessionPrx>& returnValue)> response,
                     function<void(exception_ptr)> exception,
                     const string& user,
                     const SSLInfo& sslInfo, const Ice::Current& current, const shared_ptr<SessionRouterI>& sessionRouter) :
        CreateSession(sessionRouter, user, current),
        _response(move(response)),
        _exception(move(exception)),
        _sslInfo(sslInfo)
    {
    }

    void
    authorizeResponse(bool ok, const string& reason)
    {
        if(ok)
        {
            authorized(_sessionRouter->_sslSessionManager != nullptr);
        }
        else
        {
            exception(make_exception_ptr(PermissionDeniedException(reason.empty() ? string("permission denied") : reason)));
        }
    }

    void
    authorizeException(exception_ptr ex)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch(const PermissionDeniedException&)
        {
            exception(ex);
        }
        catch(...)
        {
            unexpectedAuthorizeException(ex);
        }
    }

    void
    authorize() override
    {
        assert(_sessionRouter->_sslVerifier);

        auto ctx = _current.ctx;
        ctx.insert(_context.begin(), _context.end());

        auto self = static_pointer_cast<SSLCreateSession>(shared_from_this());
        _sessionRouter->_sslVerifier->authorizeAsync(_sslInfo,
                                                     [self](bool ok, const string& reason)
                                                     {
                                                         self->authorizeResponse(ok, reason);
                                                     },
                                                     [self](exception_ptr e)
                                                     {
                                                         self->authorizeException(e);
                                                     },
                                                     nullptr,
                                                     ctx);
    }

    shared_ptr<FilterManager>
    createFilterManager() override
    {
        return FilterManager::create(_instance, _user, false);
    }

    void
    createSession() override
    {
        auto ctx = _current.ctx;
        ctx.insert(_context.begin(), _context.end());

        auto self = static_pointer_cast<SSLCreateSession>(shared_from_this());
        _sessionRouter->_sslSessionManager->createAsync(_sslInfo, _control,
                                                        [self](shared_ptr<SessionPrx> response)
                                                        {
                                                            self->sessionCreated(move(response));
                                                        },
                                                        [self](exception_ptr e)
                                                        {
                                                            self->createException(e);
                                                        },
                                                        nullptr,
                                                        ctx);
    }

    void
    finished(const shared_ptr<SessionPrx>& session) override
    {
        _response(session);
    }

    void
    finished(exception_ptr ex) override
    {
        _exception(ex);
    }

private:

    const function<void(const shared_ptr<SessionPrx>)> _response;
    const function<void(exception_ptr)> _exception;
    const SSLInfo _sslInfo;
};

}

CreateSession::CreateSession(shared_ptr<SessionRouterI> sessionRouter, const string& user, const Ice::Current& current) :
    _instance(sessionRouter->_instance),
    _sessionRouter(move(sessionRouter)),
    _user(user),
    _current(current)
{
    // Clear reserved contexts potentially set by client
    auto ctx = _current.ctx;
    ctx.erase("_con.type");
    ctx.erase("_con.remotePort");
    ctx.erase("_con.remoteAddress");
    ctx.erase("_con.localPort");
    ctx.erase("_con.localAddress");
    ctx.erase("_con.cipher");
    ctx.erase("_con.peerCert");
    const_cast<Ice::Current&>(_current).ctx = ctx;

    if(_instance->properties()->getPropertyAsInt("Glacier2.AddConnectionContext") > 0)
    {
        _context["_con.type"] = current.con->type();
        {
            auto info = getIPConnectionInfo(current.con->getInfo());
            if(info)
            {
                ostringstream os;
                os << info->remotePort;
                _context["_con.remotePort"] = os.str();
                _context["_con.remoteAddress"] = info->remoteAddress;
                os.str("");
                os << info->localPort;
                _context["_con.localPort"] = os.str();
                _context["_con.localAddress"] = info->localAddress;            }
        }
        {
            auto info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(current.con->getInfo());
            if(info)
            {
                _context["_con.cipher"] = info->cipher;
                if(info->certs.size() > 0)
                {
                    _context["_con.peerCert"] = info->certs[0]->encode();
                }
            }
        }
    }
}

void
CreateSession::create()
{
    try
    {
        if(_sessionRouter->startCreateSession(shared_from_this(), _current.con))
        {
            authorize();
        }
    }
    catch(const Ice::Exception&)
    {
        finished(current_exception());
    }
}

void
CreateSession::addPendingCallback(shared_ptr<CreateSession> callback)
{
    _pendingCallbacks.push_back(move(callback));
}

void
CreateSession::authorized(bool createSession)
{
    //
    // Create the filter manager now as it's required for the session control object.
    //
    _filterManager = createFilterManager();

    //
    // If we have a session manager configured, we create a client-visible session object,
    // otherwise, we return a null session proxy.
    //
    if(createSession)
    {
        if(_instance->serverObjectAdapter())
        {
            auto obj = make_shared<SessionControlI>(_sessionRouter, _current.con, _filterManager);
            _control = uncheckedCast<SessionControlPrx>(_instance->serverObjectAdapter()->addWithUUID(obj));
        }
        this->createSession();
    }
    else
    {
        sessionCreated(nullptr);
    }
}

void
CreateSession::unexpectedAuthorizeException(exception_ptr ex)
{

    if(_sessionRouter->sessionTraceLevel() >= 1)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch(const Ice::Exception& e)
        {
            Warning out(_instance->logger());
            out << "exception while verifying permissions:\n" << e;
        }
    }

    exception(make_exception_ptr(PermissionDeniedException("internal server error")));

}

void
CreateSession::createException(exception_ptr sex)
{
    try
    {
        rethrow_exception(sex);
    }
    catch(const CannotCreateSessionException&)
    {
        exception(current_exception());
    }
    catch(const Ice::Exception&)
    {
        unexpectedCreateSessionException(current_exception());
    }
}

void
CreateSession::sessionCreated(const shared_ptr<SessionPrx>& session)
{
    //
    // Create the session router object.
    //
    shared_ptr<RouterI> router;
    try
    {
        Ice::Identity ident;
        if(_control)
        {
            ident = _control->ice_getIdentity();
        }

        if(_instance->properties()->getPropertyAsInt("Glacier2.AddConnectionContext") == 1)
        {
            router = make_shared<RouterI>(_instance, _current.con, _user, session, ident, _filterManager, _context);
        }
        else
        {
            router = make_shared<RouterI>(_instance, _current.con, _user, session, ident, _filterManager, Ice::Context());
        }
    }
    catch(const Ice::Exception&)
    {
        if(session)
        {
            session->destroyAsync();
        }
        unexpectedCreateSessionException(current_exception());
        return;
    }

    //
    // Notify the router that the creation is finished.
    //
    try
    {
        _sessionRouter->finishCreateSession(_current.con, router);
        finished(session);
    }
    catch(const Ice::Exception&)
    {
        finished(current_exception());
    }

    for(const auto& callback : _pendingCallbacks)
    {
        callback->create();
    }
}

void
CreateSession::unexpectedCreateSessionException(exception_ptr ex)
{
    if(_sessionRouter->sessionTraceLevel() >= 1)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch(const Ice::Exception& e)
        {
            Trace out(_instance->logger(), "Glacier2");
            out << "exception while creating session with session manager:\n" << e;
        }
    }
    exception(make_exception_ptr(CannotCreateSessionException("internal server error")));
}

void
CreateSession::exception(exception_ptr ex)
{
    try
    {
        _sessionRouter->finishCreateSession(_current.con, nullptr);
    }
    catch(const Ice::Exception&)
    {
    }

    finished(ex);

    if(_control)
    {
        try
        {
            _instance->serverObjectAdapter()->remove(_control->ice_getIdentity());
        }
        catch(const Exception&)
        {
        }
    }

    for(const auto& callback : _pendingCallbacks)
    {
        callback->create();
    }
}

SessionRouterI::SessionRouterI(shared_ptr<Instance> instance,
                               shared_ptr<PermissionsVerifierPrx> verifier,
                               shared_ptr<SessionManagerPrx> sessionManager,
                               shared_ptr<SSLPermissionsVerifierPrx> sslVerifier,
                               shared_ptr<SSLSessionManagerPrx> sslSessionManager) :
    _instance(move(instance)),
    _sessionTraceLevel(_instance->properties()->getPropertyAsInt("Glacier2.Trace.Session")),
    _rejectTraceLevel(_instance->properties()->getPropertyAsInt("Glacier2.Client.Trace.Reject")),
    _verifier(move(verifier)),
    _sessionManager(move(sessionManager)),
    _sslVerifier(move(sslVerifier)),
    _sslSessionManager(move(sslSessionManager)),
    _routersByConnectionHint(_routersByConnection.cend()),
    _routersByCategoryHint(_routersByCategory.cend()),
    _destroy(false)
{
}

SessionRouterI::~SessionRouterI()
{
    lock_guard<mutex> lg(_mutex);

    assert(_destroy);
    assert(_routersByConnection.empty());
    assert(_routersByCategory.empty());
    assert(_pending.empty());
}

void
SessionRouterI::destroy()
{
    map<shared_ptr<Connection>, shared_ptr<RouterI>> routers;
    {
        lock_guard<mutex> lg(_mutex);

        assert(!_destroy);
        _destroy = true;

        _routersByConnection.swap(routers);
        _routersByConnectionHint = _routersByConnection.cend();

        _routersByCategory.clear();
        _routersByCategoryHint = _routersByCategory.cend();
    }

    //
    // We destroy the routers outside the thread synchronization to
    // avoid deadlocks.
    //
    for(auto& router : routers)
    {
        router.second->destroy([self = shared_from_this()](exception_ptr e) { self->sessionDestroyException(e); });
    }
}

shared_ptr<ObjectPrx>
SessionRouterI::getClientProxy(Ice::optional<bool>& hasRoutingTable, const Current& current) const
{
    return getRouter(current.con, current.id)->getClientProxy(hasRoutingTable, current); // Forward to the per-client router.
}

shared_ptr<ObjectPrx>
SessionRouterI::getServerProxy(const Current& current) const
{
    return getRouter(current.con, current.id)->getServerProxy(current); // Forward to the per-client router.
}

ObjectProxySeq
SessionRouterI::addProxies(ObjectProxySeq proxies, const Current& current)
{
    //
    // Forward to the per-client router.
    //
    return getRouter(current.con, current.id)->addProxies(move(proxies), current);
}

string
SessionRouterI::getCategoryForClient(const Ice::Current& current) const
{
    // Forward to the per-client router.
    if(_instance->serverObjectAdapter())
    {
        return getRouter(current.con, current.id)->getServerProxy(current)->ice_getIdentity().category;
    }
    else
    {
        return "";
    }
}

void
SessionRouterI::createSessionAsync(string userId, string password,
                                   function<void(const shared_ptr<SessionPrx>&)> response,
                                   function<void(exception_ptr)> exception,
                                   const Current& current)
{
    if(!_verifier)
    {
        exception(make_exception_ptr(PermissionDeniedException("no configured permissions verifier")));
        return;
    }

    auto session = make_shared<UserPasswordCreateSession>(move(response),
                                                          move(exception),
                                                          move(userId),
                                                          move(password),
                                                          current,
                                                          shared_from_this());
    session->create();
}

void
SessionRouterI::createSessionFromSecureConnectionAsync(function<void(const std::shared_ptr<SessionPrx>&)> response,
                                                       function<void(std::exception_ptr)> exception,
                                                       const Current& current)
{
    if(!_sslVerifier)
    {
        exception(make_exception_ptr(PermissionDeniedException("no configured ssl permissions verifier")));
        return;
    }

    string userDN;
    SSLInfo sslinfo;

    //
    // Populate the SSL context information.
    //
    try
    {
        auto info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(current.con->getInfo());
        if(!info)
        {
            exception(make_exception_ptr(PermissionDeniedException("not ssl connection")));
            return;
        }

        auto ipInfo = getIPConnectionInfo(info);
        sslinfo.remotePort = ipInfo->remotePort;
        sslinfo.remoteHost = ipInfo->remoteAddress;
        sslinfo.localPort = ipInfo->localPort;
        sslinfo.localHost = ipInfo->localAddress;
        sslinfo.cipher = info->cipher;

        for(const auto& cert : info->certs)
        {
            sslinfo.certs.push_back(cert->encode());
        }
        if(info->certs.size() > 0)
        {
            userDN = info->certs[0]->getSubjectDN();
        }
    }
    catch(const IceSSL::CertificateEncodingException&)
    {
        exception(make_exception_ptr(PermissionDeniedException("certificate encoding exception")));
        return;
    }
    catch(const Ice::LocalException&)
    {
        exception(make_exception_ptr(PermissionDeniedException("connection exception")));
        return;
    }

    auto session = make_shared<SSLCreateSession>(move(response), move(exception), userDN, sslinfo, current, shared_from_this());
    session->create();
}

void
SessionRouterI::destroySession(const Current& current)
{
    destroySession(current.con);
}

void
SessionRouterI::refreshSessionAsync(function<void()> response, function<void(exception_ptr)> exception,
                                    const Ice::Current& current)
{
    shared_ptr<RouterI> router;
    {
        lock_guard<mutex> lg(_mutex);
        router = getRouterImpl(current.con, current.id, false); // getRouter updates the session timestamp.
        if(!router)
        {
            exception(make_exception_ptr(SessionNotExistException()));
            return;
        }
    }

    auto session = router->getSession();
    if(session)
    {
        //
        // Ping the session to ensure it does not timeout.
        //

        session->ice_pingAsync([responseCb = move(response)]
                               {
                                   responseCb();
                               },
                               [exceptionCb = move(exception),
                                sessionRouter = shared_from_this(),
                                connection = current.con](exception_ptr e)
                               {
                                   exceptionCb(e);
                                   sessionRouter->destroySession(connection);
                               });
    }
    else
    {
        response();
    }
}

void
SessionRouterI::refreshSession(const shared_ptr<Connection>& con)
{
    shared_ptr<RouterI> router;
    {
        lock_guard<mutex> lg(_mutex);
        router = getRouterImpl(con, Ice::Identity(), false); // getRouter updates the session timestamp.
        if(!router)
        {
            //
            // Close the connection otherwise the peer has no way to know that the
            // session has gone.
            //
            con->close(ConnectionClose::Forcefully);
            throw SessionNotExistException();
        }
    }

    auto session = router->getSession();
    if(session)
    {
        //
        // Ping the session to ensure it does not timeout.
        //
        session->ice_pingAsync(nullptr, [sessionRouter = shared_from_this(), con](exception_ptr)
            {
                //
                // Close the connection otherwise the peer has no way to know that
                // the session has gone.
                //
                con->close(ConnectionClose::Forcefully);
                sessionRouter->destroySession(con);
            });
    }
}

void
SessionRouterI::destroySession(const shared_ptr<Connection>& connection)
{
    shared_ptr<RouterI> router;

    {
        lock_guard<mutex> lg(_mutex);

        if(_destroy)
        {
            throw ObjectNotExistException(__FILE__, __LINE__);
        }

        map<shared_ptr<Connection>, shared_ptr<RouterI>>::const_iterator p;

        if(_routersByConnectionHint != _routersByConnection.cend() && _routersByConnectionHint->first == connection)
        {
            p = _routersByConnectionHint;
        }
        else
        {
            p = _routersByConnection.find(connection);
        }

        if(p == _routersByConnection.cend())
        {
            throw SessionNotExistException();
        }

        router = p->second;

        _routersByConnection.erase(p++);
        _routersByConnectionHint = p;

        if(_instance->serverObjectAdapter())
        {
            string category = router->getServerProxy(Current())->ice_getIdentity().category;
            assert(!category.empty());
            _routersByCategory.erase(category);
            _routersByCategoryHint = _routersByCategory.cend();
        }
    }

    //
    // We destroy the router outside the thread synchronization, to
    // avoid deadlocks.
    //
    if(_sessionTraceLevel >= 1)
    {
        Trace out(_instance->logger(), "Glacier2");
        out << "destroying session\n" << router->toString();
    }
    router->destroy([self = shared_from_this()](exception_ptr e) { self->sessionDestroyException(e); });
}

long long int
SessionRouterI::getSessionTimeout(const Ice::Current& current) const
{
    return current.con->getACM().timeout;
}

int
SessionRouterI::getACMTimeout(const Ice::Current& current) const
{
    return current.con->getACM().timeout;
}

void
SessionRouterI::updateSessionObservers()
{
    lock_guard<mutex> lg(_mutex);

    const auto& observer = _instance->getObserver();
    assert(observer);

    for(auto& router : _routersByConnection)
    {
        router.second->updateObserver(observer);
    }
}

shared_ptr<RouterI>
SessionRouterI::getRouter(const shared_ptr<Connection>& connection, const Ice::Identity& id, bool close) const
{
    lock_guard<mutex> lg(_mutex);
    return getRouterImpl(connection, id, close);
}

shared_ptr<Object>
SessionRouterI::getClientBlobject(const shared_ptr<Connection>& connection, const Ice::Identity& id) const
{
    lock_guard<mutex> lg(_mutex);
    return getRouterImpl(connection, id, true)->getClientBlobject();
}

shared_ptr<Object>
SessionRouterI::getServerBlobject(const string& category) const
{
    lock_guard<mutex> lg(_mutex);

    if(_destroy)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    if(_routersByCategoryHint != _routersByCategory.cend() && _routersByCategoryHint->first == category)
    {
        return _routersByCategoryHint->second->getServerBlobject();
    }

    auto p = _routersByCategory.find(category);

    if(p != _routersByCategory.cend())
    {
        _routersByCategoryHint = p;
        return p->second->getServerBlobject();
    }
    else
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }
}

shared_ptr<RouterI>
SessionRouterI::getRouterImpl(const shared_ptr<Connection>& connection, const Ice::Identity& id, bool close) const
{
    //
    // The connection can be null if the client tries to forward requests to
    // a proxy which points to the client endpoints (in which case the request
    // is forwarded with collocation optimization).
    //
    if(_destroy || !connection)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    if(_routersByConnectionHint != _routersByConnection.cend() && _routersByConnectionHint->first == connection)
    {
        _routersByConnectionHint->second->updateTimestamp();
        return _routersByConnectionHint->second;
    }

    auto p = _routersByConnection.find(connection);

    if(p != _routersByConnection.cend())
    {
        _routersByConnectionHint = p;
        p->second->updateTimestamp();
        return p->second;
    }
    else if(close)
    {
        if(_rejectTraceLevel >= 1)
        {
            Trace out(_instance->logger(), "Glacier2");
            out << "rejecting request, no session is associated with the connection.\n";
            out << "identity: " << identityToString(id);
        }
        connection->close(ConnectionClose::Forcefully);
        throw ObjectNotExistException(__FILE__, __LINE__);
    }
    return nullptr;
}

void
SessionRouterI::sessionDestroyException(exception_ptr ex)
{
    if(_sessionTraceLevel > 0)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch(const Ice::Exception& e)
        {
            Trace out(_instance->logger(), "Glacier2");
            out << "exception while destroying session\n" << e;
        }
    }
}

bool
SessionRouterI::startCreateSession(const shared_ptr<CreateSession>& cb, const shared_ptr<Connection>& connection)
{
    lock_guard<mutex> lg(_mutex);

    if(_destroy)
    {
        throw CannotCreateSessionException("router is shutting down");
    }

    //
    // Check whether a session already exists for the connection.
    //
    {
        map<shared_ptr<Connection>, shared_ptr<RouterI>>::const_iterator p;
        if(_routersByConnectionHint != _routersByConnection.cend() &&
           _routersByConnectionHint->first == connection)
        {
            p = _routersByConnectionHint;
        }
        else
        {
            p = _routersByConnection.find(connection);
        }

        if(p != _routersByConnection.cend())
        {
            throw CannotCreateSessionException("session exists");
        }
    }

    auto p = _pending.find(connection);
    if(p != _pending.end())
    {
        //
        // If some other thread is currently trying to create a
        // session, we wait until this thread is finished.
        //
        p->second->addPendingCallback(cb);
        return false;
    }
    else
    {
        //
        // No session exists yet, so we will try to create one. To
        // avoid that other threads try to create sessions for the
        // same connection, we add our endpoints to _pending.
        //
        _pending.insert(make_pair(connection, cb));
        return true;
    }
}

void
SessionRouterI::finishCreateSession(const shared_ptr<Connection>& connection, const shared_ptr<RouterI>& router)
{
    lock_guard<mutex> lg(_mutex);

    //
    // Signal other threads that we are done with trying to
    // establish a session for our connection;
    //
    _pending.erase(connection);

    if(!router)
    {
        return;
    }

    if(_destroy)
    {
        router->destroy([self = shared_from_this()](exception_ptr e) { self->sessionDestroyException(e); });

        throw CannotCreateSessionException("router is shutting down");
    }

    _routersByConnectionHint = _routersByConnection.insert(_routersByConnectionHint, {connection, router});

    if(_instance->serverObjectAdapter())
    {
        string category = router->getServerProxy(Ice::emptyCurrent)->ice_getIdentity().category;
        assert(!category.empty());
        auto rc = _routersByCategory.insert({category, router});
        assert(rc.second);
        _routersByCategoryHint = rc.first;
    }

    connection->setCloseCallback([self = shared_from_this()](const shared_ptr<Connection>& c)
        {
            try
            {
                self->destroySession(c);
            }
            catch(const std::exception&)
            {
            }

        });

    connection->setHeartbeatCallback([self = shared_from_this()](const shared_ptr<Connection>& c)
        {
            try
            {
                self->refreshSession(c);
            }
            catch(const std::exception&)
            {
            }
        });

    if(_sessionTraceLevel >= 1)
    {
        Trace out(_instance->logger(), "Glacier2");
        out << "created session\n" << router->toString();
    }
}
